#include <cmath>
#include <algorithm>
#include <QCoreApplication>
#include "QgsMapCanvasWidget.h"

QgsMapCanvasWidget::QgsMapCanvasWidget(QWidget* parent)
    : QWidget(parent)
    , m_mapCanvas(nullptr)
    , m_panTool(nullptr)
    , m_zoomInTool(nullptr)
    , m_zoomOutTool(nullptr)
    , m_currentRasterLayer(nullptr)
    , m_progressDialog(nullptr)
    , m_overviewWatcher(nullptr)
    , m_buildingOverviews(false)
    , m_loadingImage(false)
{
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    this->setPalette(palette);
    initCanvas();
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
    
    delete m_panTool;
    delete m_zoomInTool;
    delete m_zoomOutTool;
    delete m_mapCanvas;
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
    
    m_mapCanvas->setParallelRenderingEnabled(true);
    
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
    
    bool invoked = QMetaObject::invokeMethod(this, "loadImageAsync", Qt::QueuedConnection,
                                             Q_ARG(QString, filePath));
    if (!invoked) {
        m_loadingImage = false;
        emit imageLoaded(false, QStringLiteral("错误: 无法启动异步加载"));
    }
}

void QgsMapCanvasWidget::loadImageAsync(const QString& filePath)
{
    try {
        GDALAllRegister();
        
        if (filePath.isEmpty()) {
            emit imageLoaded(false, QStringLiteral("错误: 文件路径为空"));
            m_loadingImage = false;
            return;
        }

        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists()) {
            emit imageLoaded(false, QStringLiteral("错误: 文件不存在 - ") + filePath);
            m_loadingImage = false;
            return;
        }

        if (!fileInfo.isFile()) {
            emit imageLoaded(false, QStringLiteral("错误: 路径不是有效的文件 - ") + filePath);
            m_loadingImage = false;
            return;
        }

        QString suffix = fileInfo.suffix().toLower();
        QStringList supportedFormats = {"tif", "tiff", "img", "jpg", "jpeg", "png", "bmp", "gif"};
        if (!supportedFormats.contains(suffix)) {
            emit imageLoaded(false, QStringLiteral("错误: 不支持的文件格式 '%1'，支持的格式: tif, tiff, img, jpg, jpeg, png, bmp, gif").arg(suffix));
            m_loadingImage = false;
            return;
        }

        qint64 fileSize = fileInfo.size();
        if (fileSize == 0) {
            emit imageLoaded(false, QStringLiteral("错误: 文件为空 - ") + filePath);
            m_loadingImage = false;
            return;
        }

        if (fileSize > 5 * 1024 * 1024 * 1024LL) {
            emit imageLoaded(false, QStringLiteral("错误: 文件过大（超过5GB），无法加载"));
            m_loadingImage = false;
            return;
        }

        if (!checkAndBuildOverviews(filePath)) {
            m_loadingImage = false;
            emit imageLoaded(false, QStringLiteral("金字塔构建被取消或失败"));
            return;
        }

        clearImage();
        m_currentImagePath = filePath;

        addRasterLayer(filePath);
    }
    catch (const std::exception& e) {
        emit imageLoaded(false, QStringLiteral("异常: ") + QString::fromStdString(e.what()));
        m_loadingImage = false;
    }
    catch (...) {
        emit imageLoaded(false, QStringLiteral("未知异常: 影像加载失败"));
        m_loadingImage = false;
    }
}

void QgsMapCanvasWidget::clearImage()
{
    if (m_currentRasterLayer) {
        m_mapCanvas->setLayers(QList<QgsMapLayer*>());
        delete m_currentRasterLayer;
        m_currentRasterLayer = nullptr;
    }
    m_currentImagePath.clear();
    m_mapCanvas->refresh();
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
    msgBox.setText(QStringLiteral("当前影像没有金字塔文件，是否生成外部.ovr格式的金字塔?\n\n生成金字塔可以提高影像显示性能。"));
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
        QString errorMsg = QStringLiteral("金字塔构建完成但未生成.ovr文件") + QStringLiteral("\n") +
                           QStringLiteral("文件路径: ") + filePath + QStringLiteral("\n") +
                           QStringLiteral("可能原因: 驱动不支持外部金字塔");
        QMessageBox::warning(parent, QStringLiteral("警告"), errorMsg);
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

    return true;
}

void QgsMapCanvasWidget::addRasterLayer(const QString& filePath)
{
    try {
        QFileInfo fileInfo(filePath);
        QString baseName = fileInfo.completeBaseName();

        m_currentRasterLayer = new QgsRasterLayer(filePath, baseName, "gdal");

        if (!m_currentRasterLayer) {
            emit imageLoaded(false, QStringLiteral("错误: 创建栅格图层失败，内存不足或GDAL初始化失败"));
            m_loadingImage = false;
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
            return;
        }

        QgsRasterDataProvider* provider = m_currentRasterLayer->dataProvider();
        if (!provider) {
            QString errorMsg = QStringLiteral("错误: 无法获取栅格数据提供者 - ") + filePath;
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
            emit imageLoaded(false, errorMsg);
            m_loadingImage = false;
            return;
        }

        if (provider->bandCount() == 0) {
            QString errorMsg = QStringLiteral("错误: 影像文件没有波段数据 - ") + filePath;
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
            emit imageLoaded(false, errorMsg);
            m_loadingImage = false;
            return;
        }

        configureRasterRenderer(m_currentRasterLayer);

        QList<QgsMapLayer*> layers;
        layers << m_currentRasterLayer;
        m_mapCanvas->setLayers(layers);
        m_mapCanvas->setExtent(m_currentRasterLayer->extent());
        
        m_mapCanvas->setCanvasColor(QColor(255, 255, 255));
        
        m_mapCanvas->refresh();

        m_loadingImage = false;
        emit imageLoaded(true, QStringLiteral("影像加载成功"));
    }
    catch (const std::bad_alloc& e) {
        if (m_currentRasterLayer) {
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
        }
        emit imageLoaded(false, QStringLiteral("错误: 内存分配失败，无法加载影像"));
        m_loadingImage = false;
    }
    catch (const std::exception& e) {
        if (m_currentRasterLayer) {
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
        }
        emit imageLoaded(false, QStringLiteral("异常: ") + QString::fromStdString(e.what()));
        m_loadingImage = false;
    }
    catch (...) {
        if (m_currentRasterLayer) {
            delete m_currentRasterLayer;
            m_currentRasterLayer = nullptr;
        }
        emit imageLoaded(false, QStringLiteral("未知异常: 影像加载失败"));
        m_loadingImage = false;
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

    QgsRasterTransparency* transparency = new QgsRasterTransparency();
    
    QVector<QgsRasterTransparency::TransparentSingleValuePixel> singleValuePixels;
    for (int i = 1; i <= bandCount; ++i) {
        double noDataValue = provider->sourceNoDataValue(i);
        if (noDataValue != std::numeric_limits<double>::max()) {
            QgsRasterTransparency::TransparentSingleValuePixel pixel(noDataValue, noDataValue, 0.0);
            singleValuePixels.append(pixel);
        }
    }
    if (!singleValuePixels.isEmpty()) {
        transparency->setTransparentSingleValuePixelList(singleValuePixels);
    }
    
    QVector<QgsRasterTransparency::TransparentThreeValuePixel> threeValuePixels;
    QgsRasterTransparency::TransparentThreeValuePixel blackPixel(0.0, 0.0, 0.0, 0.0);
    threeValuePixels.append(blackPixel);
    transparency->setTransparentThreeValuePixelList(threeValuePixels);
    
    renderer->setRasterTransparency(transparency);
    layer->setRenderer(renderer);
    
    layer->setBlendMode(QPainter::CompositionMode_SourceOver);
    
    layer->triggerRepaint();
    m_mapCanvas->refresh();
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