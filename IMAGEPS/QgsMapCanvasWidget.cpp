#include <cmath>
#include <algorithm>
#include <QCoreApplication>
#include <QFileInfo>

// QGIS headers - moved from header to avoid Qt Designer crash
#include <qgis.h>
#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>
#include <qgsmaptoolpan.h>
#include <qgsmaptoolzoom.h>
#include <qgsrasterdataprovider.h>
#include <qgsrasterrenderer.h>
#include <qgssinglebandgrayrenderer.h>
#include <qgsmultibandcolorrenderer.h>
#include <qgscontrastenhancement.h>
#include <qgsrastertransparency.h>
#include <qgsrectangle.h>
#include <qgsvectorlayer.h>
#include <qgsfeature.h>
#include <qgsgeometry.h>
#include <qgssymbol.h>
#include <qgsrenderer.h>
#include <qgslayertree.h>
#include <qgslayertreemodel.h>
#include <qgslayertreelayer.h>
#include <qgsproject.h>
#include <qgscoordinatereferencesystem.h>
#include <qgscoordinatetransform.h>
#include <qgsmarkersymbol.h>
#include <qgslinesymbol.h>
#include <qgsfillsymbol.h>
#include <qgssinglesymbolrenderer.h>

// GDAL headers - moved from header to avoid Qt Designer crash
#include "gdal_priv.h"
#include "cpl_progress.h"

#include "QgsMapCanvasWidget.h"

QgsMapCanvasWidget::QgsMapCanvasWidget(QWidget* parent)
    : QWidget(parent)
    , m_mapCanvas(nullptr)
    , m_panTool(nullptr)
    , m_zoomInTool(nullptr)
    , m_zoomOutTool(nullptr)
    , m_layerTree(nullptr)
    , m_layerTreeModel(nullptr)
    , m_currentRasterLayer(nullptr)
    , m_progressDialog(nullptr)
    , m_overviewWatcher(nullptr)
    , m_buildingOverviews(false)
    , m_loadingImage(false)
    , m_rasterOnTop(false)
    , m_vectorLayerColorIndex(0)
    , m_rasterLayerVisible(true)
{
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    this->setPalette(palette);

    if (!QCoreApplication::applicationFilePath().contains("designer", Qt::CaseInsensitive)) {
        initCanvas();
    }
}

QgsMapCanvasWidget::~QgsMapCanvasWidget()
{
    if (m_overviewWatcher) {
        m_overviewWatcher->cancel();
        delete m_overviewWatcher;
    }
    if (m_progressDialog) {
        delete m_progressDialog;
    }
    clearImage();
    clearVectorLayers();
    
    delete m_layerTreeModel;
    delete m_layerTree;
    delete m_panTool;
    delete m_zoomInTool;
    delete m_zoomOutTool;
    delete m_mapCanvas;
}

bool QgsMapCanvasWidget::hasRasterLayer() const
{
    return m_currentRasterLayer != nullptr && m_currentRasterLayer->isValid();
}

void QgsMapCanvasWidget::initCanvas()
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_mapCanvas = new QgsMapCanvas(this);
    m_mapCanvas->setCanvasColor(QColor(255, 255, 255));
    m_mapCanvas->setVisible(true);
    m_mapCanvas->setFocusPolicy(Qt::StrongFocus);
    
    // 启用并行渲染，提高渲染性能
    m_mapCanvas->setParallelRenderingEnabled(true);
    
    // 设置渲染优化参数（兼容QGIS LTR版本）
    m_mapCanvas->setAutoFillBackground(true);
    m_mapCanvas->setStyleSheet("background-color: rgb(255, 255, 255);");
    
    QPalette canvasPalette = m_mapCanvas->palette();
    canvasPalette.setColor(QPalette::Window, Qt::white);
    m_mapCanvas->setPalette(canvasPalette);

    m_panTool = new QgsMapToolPan(m_mapCanvas);
    m_zoomInTool = new QgsMapToolZoom(m_mapCanvas, false);
    m_zoomOutTool = new QgsMapToolZoom(m_mapCanvas, true);

    m_mapCanvas->setMapTool(m_panTool);

    layout->addWidget(m_mapCanvas);

    setLayout(layout);
}

void QgsMapCanvasWidget::loadImage(const QString& filePath)
{
    if (m_loadingImage) {
        emit imageLoaded(false, QStringLiteral("正在加载其他影像，请等待完成"));
        return;
    }
    
    m_loadingImage = true;
    emit loadingStarted(QStringLiteral("开始加载影像..."));
    
    bool invoked = QMetaObject::invokeMethod(this, "loadImageAsync", Qt::QueuedConnection,
                                             Q_ARG(QString, filePath));
    if (!invoked) {
        m_loadingImage = false;
        emit imageLoaded(false, QStringLiteral("错误: 无法启动异步加载"));
        emit loadingFinished(false, QStringLiteral("错误: 无法启动异步加载"));
    }
}

void QgsMapCanvasWidget::loadImageAsync(const QString& filePath)
{
    try {
        emit loadingProgress(5, QStringLiteral("初始化GDAL..."));
        GDALAllRegister();
        
        if (filePath.isEmpty()) {
            emit imageLoaded(false, QStringLiteral("错误: 文件路径为空"));
            m_loadingImage = false;
            emit loadingFinished(false, QStringLiteral("错误: 文件路径为空"));
            return;
        }

        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            emit imageLoaded(false, QStringLiteral("错误: 文件不存在 - ") + filePath);
            m_loadingImage = false;
            emit loadingFinished(false, QStringLiteral("错误: 文件不存在"));
            return;
        }

        if (!fileInfo.isFile()) {
            emit imageLoaded(false, QStringLiteral("错误: 路径不是有效的文件 - ") + filePath);
            m_loadingImage = false;
            emit loadingFinished(false, QStringLiteral("错误: 路径不是有效的文件"));
            return;
        }

        QString suffix = fileInfo.suffix().toLower();
        QStringList supportedFormats = {"tif", "tiff", "img", "jpg", "jpeg", "png", "bmp", "gif"};
        if (!supportedFormats.contains(suffix)) {
            emit imageLoaded(false, QStringLiteral("错误: 不支持的文件格式 '%1'，支持的格式: tif, tiff, img, jpg, jpeg, png, bmp, gif").arg(suffix));
            m_loadingImage = false;
            emit loadingFinished(false, QStringLiteral("错误: 不支持的文件格式"));
            return;
        }

        qint64 fileSize = fileInfo.size();
        if (fileSize == 0) {
            emit imageLoaded(false, QStringLiteral("错误: 文件为空 - ") + filePath);
            m_loadingImage = false;
            emit loadingFinished(false, QStringLiteral("错误: 文件为空"));
            return;
        }

        if (fileSize > 5 * 1024 * 1024 * 1024LL) {
            emit imageLoaded(false, QStringLiteral("错误: 文件过大（超过5GB），无法加载"));
            m_loadingImage = false;
            emit loadingFinished(false, QStringLiteral("错误: 文件过大"));
            return;
        }

        emit loadingProgress(10, QStringLiteral("检查金字塔文件..."));
        if (!checkAndBuildOverviews(filePath)) {
            m_loadingImage = false;
            emit imageLoaded(false, QStringLiteral("金字塔构建被取消或失败"));
            emit loadingFinished(false, QStringLiteral("金字塔构建被取消或失败"));
            return;
        }

        emit loadingProgress(60, QStringLiteral("准备加载影像..."));
        clearImage();
        m_currentImagePath = filePath;

        emit loadingProgress(70, QStringLiteral("创建栅格图层..."));
        addRasterLayer(filePath);
    }
    catch (const std::exception& e) {
        emit imageLoaded(false, QStringLiteral("异常: ") + QString::fromStdString(e.what()));
        m_loadingImage = false;
        emit loadingFinished(false, QStringLiteral("异常: ") + QString::fromStdString(e.what()));
    }
    catch (...) {
        emit imageLoaded(false, QStringLiteral("未知异常: 影像加载失败"));
        m_loadingImage = false;
        emit loadingFinished(false, QStringLiteral("未知异常: 影像加载失败"));
    }
}

void QgsMapCanvasWidget::clearImage()
{
    // 取消正在进行的金字塔构建
    if (m_overviewWatcher) {
        m_overviewWatcher->cancel();
        delete m_overviewWatcher;
        m_overviewWatcher = nullptr;
    }
    
    // 停止加载状态
    m_loadingImage = false;
    m_buildingOverviews = false;
    
    // 移除图层并清理
    if (m_currentRasterLayer) {
        m_mapCanvas->setLayers(QList<QgsMapLayer*>());
        m_currentRasterLayer->deleteLater();
        m_currentRasterLayer = nullptr;
    }
    
    // 清除画布缓存和历史
    m_mapCanvas->clearCache();
    m_mapCanvas->freeze(false);
    
    // 重置画布范围到默认状态
    m_mapCanvas->setExtent(QgsRectangle());
    
    // 清空路径并刷新
    m_currentImagePath.clear();
    m_mapCanvas->refresh();
    
    // 强制更新显示
    update();
}

bool QgsMapCanvasWidget::hasExistingOverviews(const QString& filePath)
{
    return checkHasExistingOverviews(filePath);
}

bool QgsMapCanvasWidget::checkHasExistingOverviews(const QString& filePath)
{
    GDALDataset* dataset = (GDALDataset*)GDALOpen(filePath.toUtf8(), GA_ReadOnly);
    if (!dataset) {
        return false;
    }

    int bandCount = dataset->GetRasterCount();
    for (int i = 1; i <= bandCount; ++i) {
        GDALRasterBand* band = dataset->GetRasterBand(i);
        if (band && band->GetOverviewCount() > 0) {
            GDALClose(dataset);
            return true;
        }
    }

    GDALClose(dataset);
    
    QFileInfo fileInfo(filePath);
    
    QString ovrPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".ovr";
    if (QFile::exists(ovrPath)) {
        return true;
    }
    
    QString auxPath = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".aux";
    if (QFile::exists(auxPath)) {
        return true;
    }
    
    QString auxXmlPath = filePath + ".aux.xml";
    if (QFile::exists(auxXmlPath)) {
        return true;
    }

    return false;
}

bool QgsMapCanvasWidget::loadImageWithOverviewCheck(const QString& filePath)
{
    if (checkHasExistingOverviews(filePath)) {
        m_lastDeclinedFile.clear();
        loadImage(filePath);
        return true;
    }

    if (m_lastDeclinedFile == filePath) {
        return false;
    }

    QMessageBox msgBox(this);
    msgBox.setWindowTitle(QStringLiteral("提示"));
    //msgBox.setText(QStringLiteral("当前影像没有金字塔文件，是否生成外部.ovr格式的金字塔?\n\n生成金字塔可以提高影像显示性能。"));
    msgBox.setText(QStringLiteral("当前影像没有金字塔文件，是否生成金字塔?\n\n生成金字塔可以提高影像显示性能。"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setButtonText(QMessageBox::Yes, QStringLiteral("是"));
    msgBox.setButtonText(QMessageBox::No, QStringLiteral("否"));
    msgBox.setDefaultButton(QMessageBox::No);

    int reply = msgBox.exec();

    if (reply != QMessageBox::Yes) {
        m_lastDeclinedFile = filePath;
        return false;
    }

    m_lastDeclinedFile.clear();
    
    if (buildExternalOverviewsStatic(filePath, this)) {
        loadImage(filePath);
        return true;
    }

    return false;
}

int QgsMapCanvasWidget::gdalProgressCallbackStatic(double dfComplete, const char* pszMessage, void* pProgressArg)
{
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(pProgressArg);
    
    if (progressDialog && progressDialog->wasCanceled()) {
        return 1;
    }

    int percent = static_cast<int>(dfComplete * 100);
    if (progressDialog) {
        progressDialog->setValue(percent);
        if (pszMessage && strlen(pszMessage) > 0) {
            progressDialog->setLabelText(QStringLiteral("正在构建金字塔...") + QString::fromUtf8(pszMessage));
        }
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }

    return 0;
}

bool QgsMapCanvasWidget::buildExternalOverviewsStatic(const QString& filePath, QWidget* parent)
{
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.isWritable()) {
        QString errorMsg = QStringLiteral("无法构建金字塔") + QStringLiteral("\n") +
                           QStringLiteral("原因: 文件不可写") + QStringLiteral("\n") +
                           QStringLiteral("文件路径: ") + filePath;
        QMessageBox::warning(parent, QStringLiteral("错误"), errorMsg);
        return false;
    }

    QString ovrPath = filePath + ".ovr";
    QFileInfo ovrInfo(ovrPath);
    if (ovrInfo.exists()) {
        QFile::remove(ovrPath);
    }

    CPLSetConfigOption("GDAL_PAM_ENABLED", "YES");
    CPLSetConfigOption("GDAL_DISABLE_READDIR_ON_OPEN", "YES");
    CPLSetConfigOption("USE_RRD", "YES");
    
    GDALDataset* dataset = (GDALDataset*)GDALOpen(filePath.toUtf8(), GA_Update);
    if (!dataset) {
        QString errorMsg = QStringLiteral("无法打开影像文件进行金字塔构建") + QStringLiteral("\n") + 
                           QStringLiteral("文件路径: ") + filePath + QStringLiteral("\n") +
                           QStringLiteral("错误信息: ") + QString::fromUtf8(CPLGetLastErrorMsg());
        QMessageBox::warning(parent, QStringLiteral("错误"), errorMsg);
        return false;
    }

    GDALDriver* driver = dataset->GetDriver();
    QString driverName = QString::fromUtf8(driver ? driver->GetDescription() : "Unknown");
    
    int overviewList[] = {2, 4, 8, 16, 32, 64};
    int overviewCount = sizeof(overviewList) / sizeof(overviewList[0]);

    CPLErr ret = GDALBuildOverviews(
        dataset,
        "NEAREST",
        overviewCount,
        overviewList,
        0,
        nullptr,
        nullptr,
        nullptr
    );

    GDALClose(dataset);

    if (ret != CE_None) {
        QString errorMsg = QStringLiteral("金字塔构建失败") + QStringLiteral("\n") +
                           QStringLiteral("文件路径: ") + filePath + QStringLiteral("\n") +
                           QStringLiteral("文件驱动: ") + driverName + QStringLiteral("\n") +
                           QStringLiteral("错误信息: ") + QString::fromUtf8(CPLGetLastErrorMsg());
        QMessageBox::warning(parent, QStringLiteral("错误"), errorMsg);
        return false;
    }

    QFileInfo checkOvr(ovrPath);
    if (checkOvr.exists()) {
        QMessageBox::information(parent, QStringLiteral("成功"), QStringLiteral("金字塔构建成功"));
    } else {
        //QString errorMsg = QStringLiteral("金字塔构建完成但未生成.ovr文件") + QStringLiteral("\n") +
        //                   QStringLiteral("文件路径: ") + filePath + QStringLiteral("\n") +
        //                   QStringLiteral("可能原因: 驱动不支持外部金字塔");
        //QMessageBox::warning(parent, QStringLiteral("警告"), errorMsg);
    }

    return true;
}

bool QgsMapCanvasWidget::checkAndBuildOverviews(const QString& filePath)
{
    if (hasExistingOverviews(filePath)) {
        return true;
    }

    if (m_lastDeclinedFile == filePath) {
        return false;
    }

    return buildExternalOverviews(filePath);
}

int QgsMapCanvasWidget::gdalProgressCallback(double dfComplete, const char* pszMessage, void* pProgressArg)
{
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(pProgressArg);
    
    if (progressDialog && progressDialog->wasCanceled()) {
        return 1;
    }

    int percent = static_cast<int>(dfComplete * 100);
    if (progressDialog) {
        progressDialog->setValue(percent);
        if (pszMessage && strlen(pszMessage) > 0) {
            progressDialog->setLabelText(QStringLiteral("正在构建金字塔...") + QString::fromUtf8(pszMessage));
        }
        QCoreApplication::processEvents();
    }

    return 0;
}

bool QgsMapCanvasWidget::buildExternalOverviews(const QString& filePath)
{
    emit loadingProgress(15, QStringLiteral("开始构建金字塔..."));
    
    m_progressDialog = new QProgressDialog(QStringLiteral("正在构建影像金字塔..."), QStringLiteral("取消"), 0, 100, this);
    m_progressDialog->setWindowModality(Qt::WindowModal);
    m_progressDialog->setMinimumDuration(0);
    m_progressDialog->show();

    GDALDataset* dataset = (GDALDataset*)GDALOpen(filePath.toUtf8(), GA_Update);
    if (!dataset) {
        delete m_progressDialog;
        m_progressDialog = nullptr;
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("无法打开影像文件进行金字塔构建"));
        return false;
    }

    int overviewList[] = {2, 4, 8, 16, 32, 64};
    int overviewCount = sizeof(overviewList) / sizeof(overviewList[0]);

    emit loadingProgress(20, QStringLiteral("执行GDALBuildOverviews..."));
    CPLErr ret = GDALBuildOverviews(
        dataset,
        "NEAREST",
        overviewCount,
        overviewList,
        0,
        nullptr,
        gdalProgressCallback,
        m_progressDialog
    );

    GDALClose(dataset);

    if (m_progressDialog->wasCanceled()) {
        delete m_progressDialog;
        m_progressDialog = nullptr;
        QMessageBox::information(this, QStringLiteral("提示"), QStringLiteral("金字塔构建已取消"));
        return false;
    }

    delete m_progressDialog;
    m_progressDialog = nullptr;

    if (ret != CE_None) {
        QMessageBox::warning(this, QStringLiteral("错误"), QStringLiteral("金字塔构建失败"));
        return false;
    }

    emit loadingProgress(55, QStringLiteral("金字塔构建完成"));
    return true;
}

void QgsMapCanvasWidget::addRasterLayer(const QString& filePath)
{
    try {
        emit loadingProgress(75, QStringLiteral("创建栅格图层..."));
        QFileInfo fileInfo(filePath);
        QString baseName = fileInfo.completeBaseName();

        m_currentRasterLayer = new QgsRasterLayer(filePath, baseName, "gdal");

        if (!m_currentRasterLayer) {
            emit imageLoaded(false, QStringLiteral("错误: 创建栅格图层失败，内存不足或GDAL初始化失败"));
            m_loadingImage = false;
            emit loadingFinished(false, QStringLiteral("错误: 创建栅格图层失败"));
            return;
        }

        if (!m_currentRasterLayer->isValid()) {
            QString errorMsg = QStringLiteral("错误: 无法加载影像文件 - ") + filePath;
            QString errorString = m_currentRasterLayer->error().message();
            if (!errorString.isEmpty()) {
                errorMsg += QStringLiteral("\nGDAL错误: ") + errorString;
            }
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
            emit imageLoaded(false, errorMsg);
            m_loadingImage = false;
            emit loadingFinished(false, errorMsg);
            return;
        }

        emit loadingProgress(80, QStringLiteral("获取数据提供者..."));
        QgsRasterDataProvider* provider = m_currentRasterLayer->dataProvider();
        if (!provider) {
            QString errorMsg = QStringLiteral("错误: 无法获取栅格数据提供者 - ") + filePath;
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
            emit imageLoaded(false, errorMsg);
            m_loadingImage = false;
            emit loadingFinished(false, errorMsg);
            return;
        }

        if (provider->bandCount() == 0) {
            QString errorMsg = QStringLiteral("错误: 影像文件没有波段数据 - ") + filePath;
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
            emit imageLoaded(false, errorMsg);
            m_loadingImage = false;
            emit loadingFinished(false, errorMsg);
            return;
        }

        emit loadingProgress(85, QStringLiteral("配置渲染器..."));
        configureRasterRenderer(m_currentRasterLayer);

        emit loadingProgress(90, QStringLiteral("设置图层..."));
        QList<QgsMapLayer*> layers;
        layers << m_currentRasterLayer;
        m_mapCanvas->setLayers(layers);
        
        m_mapCanvas->setExtent(m_currentRasterLayer->extent());
        m_mapCanvas->setCanvasColor(QColor(255, 255, 255));
        
        emit loadingProgress(95, QStringLiteral("刷新画布..."));
        m_mapCanvas->refresh();
        
        // 等待渲染完成，避免缩放时闪烁
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

        m_loadingImage = false;
        emit imageLoaded(true, QStringLiteral("影像加载成功"));
        emit loadingFinished(true, QStringLiteral("影像加载成功"));
    }
    catch (const std::bad_alloc& e) {
        if (m_currentRasterLayer) {
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
        }
        emit imageLoaded(false, QStringLiteral("错误: 内存分配失败，无法加载影像"));
        m_loadingImage = false;
        emit loadingFinished(false, QStringLiteral("错误: 内存分配失败"));
    }
    catch (const std::exception& e) {
        if (m_currentRasterLayer) {
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
        }
        emit imageLoaded(false, QStringLiteral("异常: ") + QString::fromStdString(e.what()));
        m_loadingImage = false;
        emit loadingFinished(false, QStringLiteral("异常: ") + QString::fromStdString(e.what()));
    }
    catch (...) {
        if (m_currentRasterLayer) {
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
        }
        emit imageLoaded(false, QStringLiteral("未知异常: 影像加载失败"));
        m_loadingImage = false;
        emit loadingFinished(false, QStringLiteral("未知异常: 影像加载失败"));
    }
}

void QgsMapCanvasWidget::configureRasterRenderer(QgsRasterLayer* layer)
{
    if (!layer || !layer->dataProvider())
        return;

    QgsRasterDataProvider* provider = layer->dataProvider();
    int bandCount = provider->bandCount();

    if (bandCount == 1) {
        configureSingleBandRenderer(layer, provider);
    }
    else if (bandCount >= 3) {
        configureMultiBandRenderer(layer, provider, bandCount);
    }
    else {
        QgsRasterRenderer* renderer = layer->renderer();
        if (renderer) {
            renderer->setInput(provider);
        }
    }

    layer->triggerRepaint();
}

void QgsMapCanvasWidget::configureSingleBandRenderer(QgsRasterLayer* layer, QgsRasterDataProvider* provider)
{
    QgsSingleBandGrayRenderer* renderer = new QgsSingleBandGrayRenderer(provider, 1);
    
    QgsContrastEnhancement* ce = new QgsContrastEnhancement(provider->dataType(1));
    ce->setContrastEnhancementAlgorithm(QgsContrastEnhancement::StretchToMinimumMaximum);
    
    QgsRasterBandStats stats = provider->bandStatistics(1, 
        Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max);
    
    if (stats.minimumValue != std::numeric_limits<double>::max() && 
        stats.maximumValue != -std::numeric_limits<double>::max()) {
        double minValue = stats.minimumValue;
        double maxValue = stats.maximumValue;
        double range = maxValue - minValue;
        
        if (range > 0) {
            double lowerValue = minValue + range * 0.0025;
            double upperValue = maxValue - range * 0.0025;
            ce->setMinimumValue(lowerValue);
            ce->setMaximumValue(upperValue);
        } else {
            ce->setMinimumValue(minValue);
            ce->setMaximumValue(maxValue);
        }
    } else {
        Qgis::DataType dtype = provider->dataType(1);
        if (dtype == Qgis::DataType::Float32 || dtype == Qgis::DataType::Float64) {
            ce->setMinimumValue(-1.0);
            ce->setMaximumValue(1.0);
        } else if (dtype == Qgis::DataType::UInt16 || dtype == Qgis::DataType::Int16) {
            ce->setMinimumValue(0);
            ce->setMaximumValue(65535);
        } else {
            ce->setMinimumValue(0);
            ce->setMaximumValue(255);
        }
    }
    
    renderer->setContrastEnhancement(ce);
    
    QgsRasterTransparency* transparency = new QgsRasterTransparency();
    QVector<QgsRasterTransparency::TransparentSingleValuePixel> singleValuePixels;
    
    double noDataValue = provider->sourceNoDataValue(1);
    if (noDataValue != std::numeric_limits<double>::max()) {
        QgsRasterTransparency::TransparentSingleValuePixel pixel(noDataValue, noDataValue, 0.0);
        singleValuePixels.append(pixel);
    }
    
    QgsRasterTransparency::TransparentSingleValuePixel blackPixel(0.0, 0.0, 0.0);
    singleValuePixels.append(blackPixel);
    
    transparency->setTransparentSingleValuePixelList(singleValuePixels);
    
    renderer->setRasterTransparency(transparency);
    layer->setRenderer(renderer);
}

void QgsMapCanvasWidget::configureMultiBandRenderer(QgsRasterLayer* layer, QgsRasterDataProvider* provider, int bandCount)
{
    int redBand = 1;
    int greenBand = 2;
    int blueBand = 3;

    QFileInfo fileInfo(layer->source());
    QString baseName = fileInfo.completeBaseName().toUpper();
    
    //if (baseName.contains("GF7") || baseName.contains("BWDMUX")) {
    //    redBand = 4;
    //    greenBand = 3;
    //    blueBand = 2;
    //}
    
    redBand = qMin(qMax(redBand, 1), bandCount);
    greenBand = qMin(qMax(greenBand, 1), bandCount);
    blueBand = qMin(qMax(blueBand, 1), bandCount);

    QgsMultiBandColorRenderer* renderer = new QgsMultiBandColorRenderer(provider, redBand, greenBand, blueBand);

    QgsContrastEnhancement* redCE = createContrastEnhancement(provider, redBand);
    QgsContrastEnhancement* greenCE = createContrastEnhancement(provider, greenBand);
    QgsContrastEnhancement* blueCE = createContrastEnhancement(provider, blueBand);

    renderer->setRedContrastEnhancement(redCE);
    renderer->setGreenContrastEnhancement(greenCE);
    renderer->setBlueContrastEnhancement(blueCE);

    // 优化透明度处理：减少缩放时的渲染延迟
    QgsRasterTransparency* transparency = createRasterTransparency(provider, bandCount);
    renderer->setRasterTransparency(transparency);
    
    // 设置图层属性
    layer->setRenderer(renderer);
    layer->setBlendMode(QPainter::CompositionMode_SourceOver);
    
    // 禁用图层的自动刷新，由外部统一控制
    layer->triggerRepaint();
}

QgsRasterTransparency* QgsMapCanvasWidget::createRasterTransparency(QgsRasterDataProvider* provider, int bandCount)
{
    QgsRasterTransparency* transparency = new QgsRasterTransparency();
    
    // 收集NoData值
    QVector<QgsRasterTransparency::TransparentSingleValuePixel> singleValuePixels;
    for (int i = 1; i <= bandCount; ++i) {
        double noDataValue = provider->sourceNoDataValue(i);
        if (noDataValue != std::numeric_limits<double>::max()) {
            // 使用较小的容差范围，提高透明度处理效率
            QgsRasterTransparency::TransparentSingleValuePixel pixel(noDataValue - 0.5, noDataValue + 0.5, 0.0);
            singleValuePixels.append(pixel);
        }
    }
    
    // 设置单波段透明度（主要用于NoData值）
    if (!singleValuePixels.isEmpty()) {
        transparency->setTransparentSingleValuePixelList(singleValuePixels);
    }
    
    // 设置三波段透明度（用于黑色像素，即已去除黑边的区域）
    // 优化：只在多波段影像时添加黑色透明处理
    if (bandCount >= 3) {
        QVector<QgsRasterTransparency::TransparentThreeValuePixel> threeValuePixels;
        // 使用容差范围，避免精确匹配带来的性能问题
        QgsRasterTransparency::TransparentThreeValuePixel blackPixel(0.0, 0.0, 0.0, 0.0);
        threeValuePixels.append(blackPixel);
        transparency->setTransparentThreeValuePixelList(threeValuePixels);
    }
    
    return transparency;
}

QgsContrastEnhancement* QgsMapCanvasWidget::createContrastEnhancement(QgsRasterDataProvider* provider, int band)
{
    QgsContrastEnhancement* ce = new QgsContrastEnhancement(provider->dataType(band));
    ce->setContrastEnhancementAlgorithm(QgsContrastEnhancement::StretchToMinimumMaximum);
    
    QgsRasterBandStats stats = provider->bandStatistics(band, 
        Qgis::RasterBandStatistic::Min | Qgis::RasterBandStatistic::Max);
    
    if (stats.minimumValue != std::numeric_limits<double>::max() && 
        stats.maximumValue != -std::numeric_limits<double>::max()) {
        double minValue = stats.minimumValue;
        double maxValue = stats.maximumValue;
        double range = maxValue - minValue;
        
        if (range > 0) {
            double lowerValue = minValue + range * 0.0025;
            double upperValue = maxValue - range * 0.0025;
            ce->setMinimumValue(lowerValue);
            ce->setMaximumValue(upperValue);
        } else {
            ce->setMinimumValue(minValue);
            ce->setMaximumValue(maxValue);
        }
    } else {
        Qgis::DataType dtype = provider->dataType(band);
        if (dtype == Qgis::DataType::Float32 || dtype == Qgis::DataType::Float64) {
            ce->setMinimumValue(-1.0);
            ce->setMaximumValue(1.0);
        } else if (dtype == Qgis::DataType::UInt16 || dtype == Qgis::DataType::Int16) {
            ce->setMinimumValue(0);
            ce->setMaximumValue(65535);
        } else {
            ce->setMinimumValue(0);
            ce->setMaximumValue(255);
        }
    }
    
    return ce;
}

void QgsMapCanvasWidget::onOverviewBuildFinished()
{
    if (m_progressDialog) {
        m_progressDialog->close();
        delete m_progressDialog;
        m_progressDialog = nullptr;
    }

    bool success = m_overviewWatcher->result();
    emit overviewBuildFinished(success);

    if (success && m_currentRasterLayer) {
        m_mapCanvas->refresh();
    }
}

// Vector layer implementation

bool QgsMapCanvasWidget::addVectorLayer(const QString& filePath)
{
    if (m_buildingOverviews) {
        emit vectorLayerAdded(nullptr, false);
        return false;
    }

    QString baseName = QFileInfo(filePath).baseName();
    QgsVectorLayer* vectorLayer = new QgsVectorLayer(filePath, baseName, "ogr");

    if (!vectorLayer->isValid()) {
        delete vectorLayer;
        emit vectorLayerAdded(nullptr, false);
        return false;
    }

    if (!vectorLayer->crs().isValid()) {
        // 警告用户但仍然允许加载
    }

    configureVectorRenderer(vectorLayer);
    m_vectorLayerColorIndex++;  // 为下一个矢量图层准备不同颜色

    // 检查并处理坐标系统不匹配
    if (m_currentRasterLayer && m_currentRasterLayer->isValid()) {
        const QgsCoordinateReferenceSystem& rasterCrs = m_currentRasterLayer->crs();
        const QgsCoordinateReferenceSystem& vectorCrs = vectorLayer->crs();

        if (rasterCrs.isValid() && vectorCrs.isValid() && !(rasterCrs == vectorCrs)) {
            // 自动进行坐标转换
            if (!transformVectorLayer(vectorLayer, vectorCrs, rasterCrs)) {
                // 转换失败，仍然添加图层但警告用户
            }
        }
    }

    m_vectorLayers.append(vectorLayer);
    updateMapCanvasLayers();

    if (m_currentRasterLayer && m_currentRasterLayer->isValid()) {
        m_mapCanvas->setExtent(m_currentRasterLayer->extent());
    } else if (!vectorLayer->extent().isEmpty()) {
        m_mapCanvas->setExtent(vectorLayer->extent());
    }

    m_mapCanvas->refresh();
    emit vectorLayerAdded(vectorLayer, true);
    return true;
}

bool QgsMapCanvasWidget::removeVectorLayer(QgsVectorLayer* layer)
{
    if (!layer)
        return false;

    int index = m_vectorLayers.indexOf(layer);
    if (index != -1) {
        m_vectorLayers.removeAt(index);
        delete layer;
        updateMapCanvasLayers();
        emit vectorLayerRemoved(m_vectorLayers.count());
        return true;
    }
    return false;
}

void QgsMapCanvasWidget::clearVectorLayers()
{
    for (QgsVectorLayer* layer : m_vectorLayers) {
        delete layer;
    }
    m_vectorLayers.clear();
    updateMapCanvasLayers();
    emit vectorLayerRemoved(0);
}

void QgsMapCanvasWidget::setVectorLayerVisibility(QgsVectorLayer* layer, bool visible)
{
    if (!layer)
        return;

    QgsLayerTreeLayer* node = m_layerTree ? m_layerTree->findLayer(layer) : nullptr;
    if (node) {
        node->setItemVisibilityChecked(visible);
        m_mapCanvas->refresh();
    }
}

void QgsMapCanvasWidget::setVectorLayerOpacity(QgsVectorLayer* layer, double opacity)
{
    if (!layer)
        return;

    layer->setOpacity(opacity);
    m_mapCanvas->refresh();
}

void QgsMapCanvasWidget::setLayerOrder(bool rasterOnTop)
{
    m_rasterOnTop = rasterOnTop;
    updateMapCanvasLayers();
}

bool QgsMapCanvasWidget::checkSpatialReferenceMatch(QgsVectorLayer* vectorLayer) const
{
    if (!m_currentRasterLayer || !vectorLayer)
        return false;

    const QgsCoordinateReferenceSystem& rasterCrs = m_currentRasterLayer->crs();
    const QgsCoordinateReferenceSystem& vectorCrs = vectorLayer->crs();

    if (!rasterCrs.isValid() || !vectorCrs.isValid())
        return false;

    return rasterCrs == vectorCrs;
}

void QgsMapCanvasWidget::onVectorLayerVisibilityChanged(bool visible)
{
    Q_UNUSED(visible);
    m_mapCanvas->refresh();
}

void QgsMapCanvasWidget::configureVectorRenderer(QgsVectorLayer* layer)
{
    if (!layer || !layer->isValid())
        return;

    // 预定义的颜色列表，用于区分不同矢量图层
    static const QStringList pointColors = {
        "#ff0000",  // 红色
        "#00ff00",  // 绿色
        "#0000ff",  // 蓝色
        "#ff00ff",  // 紫色
        "#00ffff",  // 青色
        "#ff8000",  // 橙色
        "#8000ff",  // 紫罗兰
        "#0080ff"   // 天蓝
    };
    
    static const QStringList lineColors = {
        "#0000ff",  // 蓝色
        "#ff0000",  // 红色
        "#00ff00",  // 绿色
        "#ff00ff",  // 紫色
        "#00ffff",  // 青色
        "#ff8000",  // 橙色
        "#8000ff",  // 紫罗兰
        "#0080ff"   // 天蓝
    };
    
    static const QList<QColor> fillColors = {
        QColor(255, 0, 0, 50),    // 红色半透明
        QColor(0, 255, 0, 50),    // 绿色半透明
        QColor(0, 0, 255, 50),    // 蓝色半透明
        QColor(255, 0, 255, 50),  // 紫色半透明
        QColor(0, 255, 255, 50),  // 青色半透明
        QColor(255, 128, 0, 50),  // 橙色半透明
        QColor(128, 0, 255, 50),  // 紫罗兰半透明
        QColor(0, 128, 255, 50)   // 天蓝半透明
    };
    
    static const QStringList outlineColors = {
        "#ff0000",  // 红色
        "#00ff00",  // 绿色
        "#0000ff",  // 蓝色
        "#ff00ff",  // 紫色
        "#00ffff",  // 青色
        "#ff8000",  // 橙色
        "#8000ff",  // 紫罗兰
        "#0080ff"   // 天蓝
    };

    Qgis::GeometryType geomType = layer->geometryType();
    int colorIndex = m_vectorLayerColorIndex % 8;  // 循环使用8种颜色

    switch (geomType) {
    case Qgis::GeometryType::Point: {
        QVariantMap properties;
        properties["color"] = pointColors[colorIndex];
        properties["size"] = "3";
        QgsMarkerSymbol* symbol = QgsMarkerSymbol::createSimple(properties);
        layer->setRenderer(new QgsSingleSymbolRenderer(symbol));
        break;
    }
    case Qgis::GeometryType::Line: {
        QVariantMap properties;
        properties["color"] = lineColors[colorIndex];
        properties["width"] = "2";
        QgsLineSymbol* symbol = QgsLineSymbol::createSimple(properties);
        layer->setRenderer(new QgsSingleSymbolRenderer(symbol));
        break;
    }
    case Qgis::GeometryType::Polygon: {
        QVariantMap properties;
        QColor fillColor = fillColors[colorIndex];
        properties["color"] = QString("%1,%2,%3,%4")
            .arg(fillColor.red())
            .arg(fillColor.green())
            .arg(fillColor.blue())
            .arg(fillColor.alpha());
        properties["outline_color"] = outlineColors[colorIndex];
        properties["outline_width"] = "1";
        QgsFillSymbol* symbol = QgsFillSymbol::createSimple(properties);
        layer->setRenderer(new QgsSingleSymbolRenderer(symbol));
        break;
    }
    default:
        break;
    }

    layer->triggerRepaint();
}

void QgsMapCanvasWidget::updateMapCanvasLayers()
{
    QList<QgsMapLayer*> layers;

    if (m_rasterOnTop) {
        for (QgsVectorLayer* layer : m_vectorLayers) {
            bool visible = m_vectorLayerVisibility.value(layer->name(), true);
            if (visible) {
                layers << layer;
            }
        }
        if (m_currentRasterLayer && m_rasterLayerVisible) {
            layers << m_currentRasterLayer;
        }
    } else {
        if (m_currentRasterLayer && m_rasterLayerVisible) {
            layers << m_currentRasterLayer;
        }
        for (QgsVectorLayer* layer : m_vectorLayers) {
            bool visible = m_vectorLayerVisibility.value(layer->name(), true);
            if (visible) {
                layers << layer;
            }
        }
    }

    m_mapCanvas->setLayers(layers);
    m_mapCanvas->refresh();
}

QString QgsMapCanvasWidget::analyzeCrsCompatibility(const QgsCoordinateReferenceSystem& rasterCrs, const QgsCoordinateReferenceSystem& vectorCrs) const
{
    QString info;
    info += QStringLiteral("【栅格图层 CRS】\n");
    info += QStringLiteral("描述: %1\n").arg(rasterCrs.description());
    info += QStringLiteral("认证ID: %1\n").arg(rasterCrs.authid());
    info += QStringLiteral("WKT: %1\n\n").arg(rasterCrs.toWkt());

    info += QStringLiteral("【矢量图层 CRS】\n");
    info += QStringLiteral("描述: %1\n").arg(vectorCrs.description());
    info += QStringLiteral("认证ID: %1\n").arg(vectorCrs.authid());
    info += QStringLiteral("WKT: %1\n\n").arg(vectorCrs.toWkt());

    if (rasterCrs.authid() == vectorCrs.authid()) {
        info += QStringLiteral("两个CRS使用相同的认证ID，可以直接叠加显示。");
    } else if (rasterCrs.isGeographic() && vectorCrs.isGeographic()) {
        info += QStringLiteral("两个CRS都是地理坐标系，但基准可能不同。");
    } else if (rasterCrs.isGeographic() && !vectorCrs.isGeographic()) {
        info += QStringLiteral("栅格为地理坐标系，矢量为投影坐标系，需要进行投影转换。");
    } else if (!rasterCrs.isGeographic() && vectorCrs.isGeographic()) {
        info += QStringLiteral("栅格为投影坐标系，矢量为地理坐标系，需要进行投影转换。");
    } else {
        info += QStringLiteral("两个CRS都是投影坐标系，可能需要进行投影转换。");
    }

    return info;
}

bool QgsMapCanvasWidget::transformVectorLayer(QgsVectorLayer* vectorLayer, const QgsCoordinateReferenceSystem& sourceCrs, const QgsCoordinateReferenceSystem& targetCrs)
{
    if (!vectorLayer || !sourceCrs.isValid() || !targetCrs.isValid())
        return false;

    try {
        QgsCoordinateTransform transform(sourceCrs, targetCrs, QgsProject::instance());

        QList<QgsFeature> updatedFeatures;

        QgsFeatureIterator iterator = vectorLayer->getFeatures();
        QgsFeature feature;

        while (iterator.nextFeature(feature)) {
            QgsGeometry geometry = feature.geometry();
            if (geometry.isNull())
                continue;

            geometry.transform(transform);
            feature.setGeometry(geometry);
            updatedFeatures.append(feature);
        }

        if (updatedFeatures.isEmpty())
            return false;

        vectorLayer->startEditing();
        vectorLayer->dataProvider()->truncate();
        vectorLayer->dataProvider()->addFeatures(updatedFeatures);
        vectorLayer->commitChanges();
        vectorLayer->setCrs(targetCrs);
        vectorLayer->updateExtents();

        return true;
    } catch (const std::exception&) {
        return false;
    }
}

// Layer switch implementation

QString QgsMapCanvasWidget::getRasterLayerName() const
{
    if (m_currentRasterLayer && m_currentRasterLayer->isValid()) {
        return m_currentRasterLayer->name();
    }
    return QString();
}

QStringList QgsMapCanvasWidget::getVectorLayerNames() const
{
    QStringList names;
    for (QgsVectorLayer* layer : m_vectorLayers) {
        if (layer && layer->isValid()) {
            names.append(layer->name());
        }
    }
    return names;
}

bool QgsMapCanvasWidget::switchToLayer(const QString& layerName)
{
    // 检查是否带有类型前缀
    QString actualName = layerName;
    bool isRaster = false;
    bool isVector = false;
    
    if (layerName.startsWith(QStringLiteral("[影像] "))) {
        actualName = layerName.mid(5);  // 移除 "[影像] " 前缀
        isRaster = true;
    } else if (layerName.startsWith(QStringLiteral("[矢量] "))) {
        actualName = layerName.mid(5);  // 移除 "[矢量] " 前缀
        isVector = true;
    }
    
    // 如果指定了类型，按类型查找
    if (isRaster) {
        if (m_currentRasterLayer && m_currentRasterLayer->isValid() && 
            m_currentRasterLayer->name() == actualName) {
            updateMapCanvasLayers();
            m_mapCanvas->setExtent(m_currentRasterLayer->extent());  // 设置视图范围
            m_mapCanvas->refresh();
            return true;
        }
        return false;
    }
    
    if (isVector) {
        for (QgsVectorLayer* layer : m_vectorLayers) {
            if (layer && layer->isValid() && layer->name() == actualName) {
                bool oldOrder = m_rasterOnTop;
                m_rasterOnTop = false;
                updateMapCanvasLayers();
                m_rasterOnTop = oldOrder;
                m_mapCanvas->setExtent(layer->extent());
                m_mapCanvas->refresh();
                return true;
            }
        }
        return false;
    }
    
    // 没有类型前缀，按原来的逻辑查找（优先影像）
    if (m_currentRasterLayer && m_currentRasterLayer->isValid() && 
        m_currentRasterLayer->name() == actualName) {
        updateMapCanvasLayers();
        m_mapCanvas->setExtent(m_currentRasterLayer->extent());  // 设置视图范围
        m_mapCanvas->refresh();
        return true;
    }

    for (QgsVectorLayer* layer : m_vectorLayers) {
        if (layer && layer->isValid() && layer->name() == actualName) {
            bool oldOrder = m_rasterOnTop;
            m_rasterOnTop = false;
            updateMapCanvasLayers();
            m_rasterOnTop = oldOrder;
            m_mapCanvas->setExtent(layer->extent());
            m_mapCanvas->refresh();
            return true;
        }
    }

    return false;
}

bool QgsMapCanvasWidget::toggleLayerVisibility(const QString& layerName)
{
    // 检查是否带有类型前缀
    QString actualName = layerName;
    bool isVector = false;
    bool isRaster = false;
    
    if (layerName.startsWith(QStringLiteral("[矢量] "))) {
        actualName = layerName.mid(5);  // 移除 "[矢量] " 前缀
        isVector = true;
    } else if (layerName.startsWith(QStringLiteral("[影像] "))) {
        actualName = layerName.mid(5);  // 移除 "[影像] " 前缀
        isRaster = true;
    }
    
    // 切换影像图层可见性
    if (isRaster) {
        if (m_currentRasterLayer && m_currentRasterLayer->isValid() && 
            m_currentRasterLayer->name() == actualName) {
            m_rasterLayerVisible = !m_rasterLayerVisible;
            updateMapCanvasLayers();
            return true;
        }
        return false;
    }
    
    // 切换矢量图层可见性
    if (isVector) {
        for (QgsVectorLayer* layer : m_vectorLayers) {
            if (layer && layer->isValid() && layer->name() == actualName) {
                bool currentVisible = m_vectorLayerVisibility.value(actualName, true);
                m_vectorLayerVisibility[actualName] = !currentVisible;
                updateMapCanvasLayers();
                return true;
            }
        }
        return false;
    }
    
    // 没有类型前缀，按原来的逻辑查找（优先影像）
    if (m_currentRasterLayer && m_currentRasterLayer->isValid() && 
        m_currentRasterLayer->name() == actualName) {
        m_rasterLayerVisible = !m_rasterLayerVisible;
        updateMapCanvasLayers();
        return true;
    }
    
    for (QgsVectorLayer* layer : m_vectorLayers) {
        if (layer && layer->isValid() && layer->name() == actualName) {
            bool currentVisible = m_vectorLayerVisibility.value(actualName, true);
            m_vectorLayerVisibility[actualName] = !currentVisible;
            updateMapCanvasLayers();
            return true;
        }
    }

    return false;
}

QList<QPair<QString, bool>> QgsMapCanvasWidget::getAllLayerStatus() const
{
    QList<QPair<QString, bool>> statusList;

    // Add raster layer status
    if (m_currentRasterLayer && m_currentRasterLayer->isValid()) {
        statusList.append(QPair<QString, bool>(QStringLiteral("[影像] ") + m_currentRasterLayer->name(), m_rasterLayerVisible));
    }

    // Add vector layer status
    for (QgsVectorLayer* layer : m_vectorLayers) {
        if (layer && layer->isValid()) {
            bool visible = m_vectorLayerVisibility.value(layer->name(), true);
            statusList.append(QPair<QString, bool>(QStringLiteral("[矢量] ") + layer->name(), visible));
        }
    }

    return statusList;
}