# QGIS 影像显示模块 - 问题记录与功能需求文档

## 注:Qt版本使用:D:\test\OSGeo4W\apps\Qt5, QGIS版本使用:D:\test\OSGeo4W

## 目录
1. [问题记录](#问题记录)
2. [功能需求](#功能需求)
3. [解决方案](#解决方案)
4. [注意事项](#注意事项)

---

## 问题记录

### 问题1：影像颜色呈现异常

**描述**：
- 影像显示颜色与 ArcMap 预期视觉效果不符
- 原始代码对4波段影像错误使用假彩色合成（红=4、绿=3、蓝=2），导致植被呈红色
- 缺少Gamma校正，导致对比度和亮度与ArcMap不一致

**根本原因**：
- 波段顺序设置错误，与ArcMap标准RGB合成（红=1、绿=2、蓝=3）不符
- 缺少Gamma校正（ArcMap默认Gamma值为1.55595）

**解决方案**：
```cpp
// 正确的波段顺序（标准RGB合成，与ArcMap一致）
int redBand = 1;    // 第 1 波段 → 红色通道
int greenBand = 2;  // 第 2 波段 → 绿色通道
int blueBand = 3;   // 第 3 波段 → 蓝色通道

// 设置Gamma校正
renderer->setRedGamma(1.55595);
renderer->setGreenGamma(1.55595);
renderer->setBlueGamma(1.55595);
```

**特殊处理**：
- GF7 和 BWDMUX 格式影像使用假彩色合成（红=4、绿=3、蓝=2）

---

### 问题2：影像加载速度显著下降

**描述**：
- 加载多光谱影像时性能降低多倍
- 每次加载都对每个波段调用 `bandStatistics()`，触发全图统计计算

**根本原因**：
- 对每个波段重复调用统计函数，导致性能下降

**解决方案**：
- 优化统计数据获取方式，只请求必要的统计值（Min/Max）
- 减少不必要的计算和对象创建

---

### 问题3：NoData值透明显示问题

**描述**：
- 影像边缘区域（NoData值）显示为黑色而不是透明
- 黑边问题持续存在

**根本原因**：
- 部分影像的边缘区域使用黑色（0,0,0）而不是标准NoData值
- 单波段NoData设置可能不足以处理多波段影像

**解决方案**：
```cpp
// 设置单波段NoData值透明
QVector<QgsRasterTransparency::TransparentSingleValuePixel> singleValuePixels;
for (int i = 1; i <= bandCount; ++i) {
    double noDataValue = provider->sourceNoDataValue(i);
    if (noDataValue != std::numeric_limits<double>::max()) {
        // opacity参数范围：0.0（完全透明）到 1.0（完全不透明）
        QgsRasterTransparency::TransparentSingleValuePixel pixel(noDataValue, noDataValue, 0.0);
        singleValuePixels.append(pixel);
    }
}
if (!singleValuePixels.isEmpty()) {
    transparency->setTransparentSingleValuePixelList(singleValuePixels);
}

// 使用三波段透明处理黑色像素（处理黑边）
// 将RGB值为(0,0,0)的像素设置为完全透明
QVector<QgsRasterTransparency::TransparentThreeValuePixel> threeValuePixels;
QgsRasterTransparency::TransparentThreeValuePixel blackPixel(0.0, 0.0, 0.0, 0.0);
threeValuePixels.append(blackPixel);
transparency->setTransparentThreeValuePixelList(threeValuePixels);

// 设置图层混合模式
layer->setBlendMode(QPainter::CompositionMode_SourceOver);
```

**重要说明**：
1. **opacity参数范围是0.0到1.0**，其中：
   - `0.0` 表示完全透明
   - `1.0` 表示完全不透明
   - **错误用法**：使用 `opacity = 100`（这是错误的！）
2. 当前QGIS LTR版本不支持 `TransparentGradientPixel`，因此使用 `TransparentThreeValuePixel` 替代
3. 画布背景色必须设置为白色 `QColor(255, 255, 255)`，才能正确显示透明效果
4. 单波段渲染器也需要设置透明度处理（使用 `TransparentSingleValuePixel`）

**状态**：已修复

---

### 问题6：查看影像时概率性崩溃

**描述**：
- 在界面中点击"查看影像"按钮时，会有概率在 `IMAGEPS.cpp` 第17787行附近崩溃
- 崩溃发生在进度对话框清理代码中

**根本原因**：
1. **成员变量未初始化**：`m_imageLoadingProgressDialog` 在构造函数中没有初始化为 `nullptr`，导致第一次调用时可能包含随机垃圾值
2. **信号断开不彻底**：使用 `disconnect(..., nullptr)` 无法正确断开 lambda 连接
3. **Qt父子机制冲突**：`QProgressDialog` 创建时指定了父对象 `this`，Qt父子机制和手动 `deleteLater()` 可能导致重复删除

**解决方案**：

**1. 在构造函数初始化列表中初始化指针**：
```cpp
IMAGEPS::IMAGEPS(QWidget* parent)
    : QMainWindow(parent)
    , m_imageLoadingProgressDialog(nullptr)
```

**2. 优化清理逻辑**：
```cpp
if (m_imageLoadingProgressDialog) {
    QProgressDialog* oldDialog = m_imageLoadingProgressDialog;
    m_imageLoadingProgressDialog = nullptr;
    disconnect(ui.openGLWidget, nullptr, this, nullptr);
    oldDialog->setParent(nullptr);
    oldDialog->deleteLater();
}
```

**3. 创建对话框时不指定父对象**：
```cpp
m_imageLoadingProgressDialog = new QProgressDialog("正在加载影像...", "取消", 0, 100);
```

**状态**：已修复

---

### 问题4：影像加载时界面卡顿无响应

**描述**：
- 用户点击"查看影像"按钮后，界面立即切换到影像显示标签页
- 但影像长时间无法显示，界面处于无响应状态
- 缺少加载进度反馈，用户无法感知系统正在工作

**根本原因**：
- `GDALBuildOverviews()` 在主线程中同步执行，长时间阻塞UI事件循环
- 虽然声明了 `QFutureWatcher`，但未实际使用
- 加载过程中没有任何进度反馈

**解决方案**：

**1. QgsMapCanvasWidget.h - 添加进度信号**：
```cpp
signals:
    void loadingProgress(int percent, const QString& message);
    void loadingStarted(const QString& message);
    void loadingFinished(bool success, const QString& message);
```

**2. QgsMapCanvasWidget.cpp - 添加进度反馈**：
```cpp
// 在关键阶段发出进度信号
emit loadingProgress(5, QStringLiteral("初始化GDAL..."));
emit loadingProgress(10, QStringLiteral("检查金字塔文件..."));
emit loadingProgress(15, QStringLiteral("开始构建金字塔..."));
emit loadingProgress(70, QStringLiteral("创建栅格图层..."));
emit loadingProgress(95, QStringLiteral("刷新画布..."));
```

**3. IMAGEPS.cpp - 显示进度对话框**：
```cpp
void IMAGEPS::on_actionOpenImageShow(QString filenamePATH)
{
    // 立即切换标签页
    ui.tabWidget->setCurrentIndex(1);
    
    // 创建进度对话框
    m_imageLoadingProgressDialog = new QProgressDialog(...);
    m_imageLoadingProgressDialog->show();
    
    // 连接进度信号
    connect(ui.openGLWidget, &QgsMapCanvasWidget::loadingProgress, 
            this, [this](int percent, const QString& msg) {
        m_imageLoadingProgressDialog->setValue(percent);
        m_imageLoadingProgressDialog->setLabelText(msg);
        QCoreApplication::processEvents();
    });
    
    // 开始异步加载
    ui.openGLWidget->loadImageWithOverviewCheck(filenamePATH);
}
```

**状态**：已修复

---

### 问题5：`on_actionOpenImageShow` 函数重复调用问题

**描述**：
- 在表格控件中选中整行时，会选中该行的所有单元格
- 原代码在遍历选中项时，同一行的第一列值会被重复添加到列表中
- 导致 `on_actionOpenImageShow(filenamePATH)` 函数被同一影像路径多次调用

**根本原因**：
- 使用 `QStringList` 存储文件名，没有去重机制
- 遍历所有选中单元格时，每行被多次处理

**解决方案**：
- 将 `QStringList firstColumnValues` 替换为 `QSet<QString> firstColumnValues`
- 将 `.append()` 方法替换为 `.insert()` 方法，利用 `QSet` 的自动去重特性
- 在多个表格控件的"查看影像"、"卸载影像"等功能中应用此修复

**修改的代码位置**：
- `ui.sateImageDataList_TableW` 的"查看影像"、"卸载影像"、"设置纠正成果属性信息"操作
- `ui.PyramidDataList_TableW` 的"查看影像"、"卸载影像"操作
- `ui.imageInterList_TableW` 的"查看影像"操作
- `ui.AbsPositPrecCheckDataList_TableW` 的"查看影像"操作
- `ui.VectorfileDataList_TableW` 的"卸载矢量"操作
- `ui.CloudDetectionDataList_TableW` 的"查看影像"、"卸载影像"操作
- `unloadImages()` 和 `viewImages()` 通用函数

**状态**：已修复

---

### 问题8：QtVS Tools Designer打开UI文件崩溃

**描述**：
- 在Visual Studio中使用QtVS Tools打开项目的UI文件时，VS会自动崩溃
- 报错提示"无法打开文件"
- 工程编译不通过

**根本原因**：
- QtVS Tools在打开UI文件时，会启动Qt Designer进程解析关联的头文件，以获取自定义Widget的类型信息
- `QgsMapCanvasWidget.h`直接包含了QGIS头文件（如`qgis.h`、`qgsmapcanvas.h`等）
- `ImageInfoShow.h`、`MeasurementareaShow.h`、`GLDisplayWidget.h`、`SatelliteViewer.h`直接包含了GDAL头文件
- QGIS和GDAL头文件需要相应的运行时环境支持，但Designer进程是独立的Qt应用，没有加载这些依赖库和环境变量
- 导致Designer解析头文件时发生崩溃

**工作流程示意**：
```
打开UI文件 → QtVS Tools启动Designer → Designer解析头文件(IMAGEPS.h)
         → 包含QgsMapCanvasWidget.h → 遇到QGIS/GDAL头文件 → 缺少运行时环境 → 崩溃
```

**解决方案**：
1. 将QGIS头文件从头文件(.h)移到源文件(.cpp)中，在头文件中使用前向声明
2. 将GDAL头文件从被UI引用的头文件移到源文件中
3. 设置注册表让Designer以独立进程模式运行

**修改文件清单**：

| 文件 | 修改内容 |
|------|----------|
| QgsMapCanvasWidget.h | 移除QGIS和GDAL头文件，添加31个前向声明 |
| QgsMapCanvasWidget.cpp | 添加完整的QGIS和GDAL头文件包含 |
| ImageInfoShow.h | 移除GDAL头文件，添加2个前向声明 |
| ImageInfoShow.cpp | 添加完整的GDAL头文件包含 |
| MeasurementareaShow.h | 移除GDAL头文件，添加2个前向声明 |
| MeasurementareaShow.cpp | 添加完整的GDAL头文件包含 |
| GLDisplayWidget.h | 移除GDAL头文件，添加2个前向声明，修改dataType类型 |
| GLDisplayWidget.cpp | 添加完整的GDAL头文件包含 |
| SatelliteViewer.h | 移除未使用的GDAL头文件 |
| SatelliteViewer.cpp | 添加GDAL头文件包含 |

**修改前 - QgsMapCanvasWidget.h**：
```cpp
// QGIS headers - 直接包含会导致Designer崩溃
#include <qgis.h>
#include <qgsmapcanvas.h>
#include <qgsrasterlayer.h>
#include <qgslayertree.h>
// ... 更多QGIS头文件
```

**修改后 - QgsMapCanvasWidget.h**：
```cpp
// QGIS classes - 使用前向声明，避免Designer解析
class QgsMapCanvas;
class QgsRasterLayer;
class QgsVectorLayer;
class QgsLayerTree;
class QgsLayerTreeModel;
class QgsLayerTreeLayer;
class QgsMapToolPan;
class QgsMapToolZoom;
class QgsProject;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransform;
class QgsRasterDataProvider;
class QgsRasterRenderer;
class QgsSingleBandGrayRenderer;
class QgsMultiBandColorRenderer;
class QgsContrastEnhancement;
class QgsRasterTransparency;
class QgsRectangle;
class QgsFeature;
class QgsGeometry;
class QgsSymbol;
class QgsRenderer;
class QgsMarkerSymbol;
class QgsLineSymbol;
class QgsFillSymbol;
class QgsSingleSymbolRenderer;
class QgsMapLayer;
```

**修改 - QgsMapCanvasWidget.cpp**：
```cpp
// 在源文件开头添加完整的QGIS头文件包含
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
```

**注意事项**：
1. 前向声明只能用于指针类型和引用类型，不能用于值类型
2. 类的成员函数定义必须在源文件中，因为头文件中只有前向声明
3. 如果需要在头文件中使用类的完整定义（如继承），需要保持该头文件包含

**第二阶段修复 - 构造函数运行时检测**：

前向声明只解决了编译期头文件解析问题，但Designer打开UI文件时会实例化自定义Widget，此时构造函数中调用QGIS/GDAL初始化代码会导致运行时崩溃。需要在构造函数中检测是否在Qt Designer中运行。

**检测原理**：
```
Qt Designer进程的可执行文件路径包含 "designer"
通过 QCoreApplication::applicationFilePath() 判断当前运行环境
（比 applicationName() 更可靠，因为QtVS Tools启动的Designer进程名可能不是"designer"）
```

**修改示例 - QgsMapCanvasWidget.cpp**：
```cpp
QgsMapCanvasWidget::QgsMapCanvasWidget(QWidget* parent)
    : QWidget(parent)
    // ... 成员初始化 ...
{
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    this->setPalette(palette);

    // 在Qt Designer中不初始化QGIS组件
    if (!QCoreApplication::applicationFilePath().contains("designer", Qt::CaseInsensitive)) {
        initCanvas();  // 创建QgsMapCanvas等QGIS对象
    }
}
```

**修改示例 - ImageInfoShow.cpp**：
```cpp
ImageInfoShow::ImageInfoShow(QWidget* parent)
    : QCustomPlot(parent)
{
    m_imagePS = nullptr;

    // 在Qt Designer中不初始化GDAL/PROJ
    if (!QCoreApplication::applicationFilePath().contains("designer", Qt::CaseInsensitive)) {
        GDALAllRegister();
        PJ_CONTEXT* ctx = proj_context_create();
        // ... PROJ初始化 ...
    }
}
```

**第二阶段修改文件清单**：

| 文件 | 修改内容 |
|------|----------|
| QgsMapCanvasWidget.cpp | 构造函数中检测Designer，跳过initCanvas() |
| ImageInfoShow.cpp | 构造函数中检测Designer，跳过GDAL/PROJ初始化 |
| MeasurementareaShow.cpp | 构造函数中检测Designer，跳过GDAL/PROJ初始化 |
| GLDisplayWidget.cpp | 构造函数中检测Designer，跳过GDAL初始化 |
| SatelliteViewer.cpp | 构造函数中检测Designer，跳过GDAL初始化 |

**工作流程示意（修复后）**：
```
打开UI文件 → QtVS Tools启动Designer → Designer解析头文件(前向声明，安全)
         → 实例化自定义Widget → 构造函数通过applicationFilePath检测到"designer"进程
         → 跳过QGIS/GDAL初始化 → Designer正常显示占位Widget
```

**状态**：已修复

---

### 问题7：智能镶嵌文件路径不存在问题

**描述**：
- 在执行智能镶嵌功能时，`SmartMosaicFilePath` 中的文件路径可能不存在
- 原因可能是原始文件被删除、移动或格式转换（如从 .pix 转为 .tif）
- 直接传入不存在的路径会导致后续处理失败

**根本原因**：
- 文件路径列表在保存后，实际文件可能发生变化
- 缺少文件存在性检查和同名文件查找机制

**解决方案**：
在调用 `SmartMosaic` 函数前，添加文件存在性检查逻辑：
1. 遍历 `SmartMosaicFilePath` 中的每个路径
2. 检查文件是否存在
3. 如果不存在，获取文件所在目录和基础文件名（不含扩展名）
4. 在该目录下查找同名的 .tif/.tiff/.img 文件（不区分大小写）
5. 如果找到则替换原路径，如果找不到则记录警告日志

**实现代码**：
```cpp
// 检查文件是否存在，如果不存在则查找同名的tif/img文件
for (int i = 0; i < SmartMosaicFilePath.size(); ++i)
{
    QString& filePath = SmartMosaicFilePath[i];
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists())
    {
        QString dirPath = fileInfo.absolutePath();
        QString baseName = fileInfo.completeBaseName();
        QDir dir(dirPath);

        // 查找同名的tif/img文件
        QStringList filters;
        filters << baseName + ".tif" << baseName + ".TIF"
                << baseName + ".tiff" << baseName + ".TIFF"
                << baseName + ".img" << baseName + ".IMG";

        QStringList foundFiles = dir.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);

        if (!foundFiles.isEmpty())
        {
            // 找到同名文件，替换路径
            QString newPath = dir.absoluteFilePath(foundFiles.first());
            SmartMosaicFilePath[i] = newPath;
            PROJECT_LOG_INFO(this->CurrentConfig,
                QString::fromLocal8Bit("智能镶嵌: 文件不存在，已替换为同名文件: %1 -> %2")
                .arg(filePath).arg(newPath));
        }
        else
        {
            // 未找到同名文件，记录警告
            PROJECT_LOG_WARNING(this->CurrentConfig,
                QString::fromLocal8Bit("智能镶嵌: 文件不存在且未找到同名tif/img文件: %1")
                .arg(filePath));
        }
    }
}
```

**修改位置**：
- `IMAGEPS.cpp` 第7498-7537行：`SmartMosaicactionSlot()` 函数
- `IMAGEPS.cpp` 第13295-13334行：树形菜单"智能镶嵌"节点处理

**状态**：已修复

---

## 功能需求

### 需求1：实现 ArcMap 风格的百分比截断拉伸

**ArcMap 默认设置**：
- 拉伸类型：百分比截断
- 最小值截断：0.25%
- 最大值截断：0.25%

**实现代码**：
```cpp
double range = maxValue - minValue;
double lowerValue = minValue + range * 0.0025;  // 底部截断 0.25%
double upperValue = maxValue - range * 0.0025;  // 顶部截断 0.25%
ce->setMinimumValue(lowerValue);
ce->setMaximumValue(upperValue);
```

---

### 需求2：设置 NoData 值为无颜色

**ArcMap 设置**：
- "将 NoData 显示为" 设置为透明/无颜色

**实现代码**：
```cpp
double noDataValue = provider->sourceNoDataValue(1);
if (noDataValue != std::numeric_limits<double>::max()) {
    renderer->setNoDataValue(noDataValue);
}
```

---

### 需求3：Gamma 校正（待实现）

**ArcMap 默认设置**：
- Gamma 值：1.55595（对所有通道）

**当前状态**：
- 当前使用的QGIS LTR版本（D:\test\OSGeo4W）不支持 `setGamma()` 方法
- `QgsSingleBandGrayRenderer` 和 `QgsMultiBandColorRenderer` 没有Gamma校正API
- 需要升级到更新的QGIS版本或使用自定义渲染逻辑实现Gamma校正

**替代方案**：
- 使用百分比截断拉伸（0.25%）来调整对比度
- 通过对比度增强算法模拟部分 Gamma 效果

---

### 需求 4：图层切换功能（已实现）

**功能描述**：
- 在 tab_16（影像显示标签页）中添加图层切换工具栏
- 支持影像和矢量图层之间的快速切换
- 提供图层显示/隐藏控制
- 支持调整图层叠放顺序

**UI 组件**：
- **影像在上按钮**：将影像图层置于顶层
- **矢量在上按钮**：将矢量图层置于顶层
- **图层选择下拉框**：显示所有可用图层及其可见性状态
- **显示/隐藏按钮**：切换选中图层的可见性

**实现细节**：
- 利用 `QgsMapCanvasWidget` 已有的图层管理接口
- 在影像/矢量加载成功后自动更新图层下拉框
- 使用 `setLayerOrder()` 控制图层叠放顺序
- 使用 `switchToLayer()` 切换到指定图层
- 使用 `toggleLayerVisibility()` 切换图层可见性
- 使用 `getAllLayerStatus()` 获取所有图层状态

**测试场景**：
| 操作 | 预期结果 |
|------|----------|
| 加载影像后 | 图层下拉框显示影像图层 |
| 加载矢量后 | 图层下拉框显示影像和矢量图层 |
| 点击"影像在上" | 影像图层覆盖在矢量图层之上 |
| 点击"矢量在上" | 矢量图层覆盖在影像图层之上 |
| 选择图层并点击"显示/隐藏" | 图层可见性切换，下拉框文本更新 |

---

## 解决方案汇总

### 关键修改文件

| 文件 | 修改内容 |
|------|----------|
| `QgsMapCanvasWidget.cpp` | 修复波段顺序、实现百分比截断拉伸、设置 NoData |
| `QgsMapCanvasWidget.h` | 添加必要的头文件包含 |

### 核心函数

**`configureRasterRenderer()`** - 配置栅格渲染器：
1. 根据波段数量选择渲染器类型
2. 设置正确的波段顺序（红=3、绿=2、蓝=1）
3. 应用百分比截断拉伸（0.25%）
4. 设置 NoData 值

---

## 注意事项

### API 兼容性警告

1. **QGIS 版本差异**：
   - `QgsSingleBandGrayRenderer` 和 `QgsMultiBandColorRenderer` 支持 `setGamma()` 方法
   - `QgsRasterTransparency` 结构体成员名称可能因版本而异

2. **避免的错误**：
   - 不要使用 `setNoDataColor()` 方法（不存在）
   - 不要使用 `setNoDataValue()` 方法（不存在）
   - 不要使用 `QList` 作为透明像素列表（应使用 `QVector`）

### 性能优化建议

1. **统计数据缓存**：
   - 避免重复调用 `bandStatistics()`
   - 考虑缓存统计结果供后续使用

2. **金字塔文件**：
   - 确保影像有金字塔文件（.ovr）以提升加载性能
   - 在加载前检查并提示用户创建金字塔

### 测试用例

| 测试场景 | 预期结果 |
|----------|----------|
| 加载16位4波段多光谱影像 | 颜色与ArcMap一致，植被呈绿色（标准RGB合成） |
| 加载16位3波段影像 | 颜色与ArcMap一致 |
| 加载8位4波段影像 | 颜色与ArcMap一致 |
| 加载8位3波段影像 | 颜色与ArcMap一致 |
| 加载灰度图 | 对比度与ArcMap一致，Gamma=1.55595 |
| 加载 GF7 格式影像 | 使用假彩色合成（红=4、绿=3、蓝=2），植被呈红色 |
| 影像边缘区域 | NoData 值显示为透明 |
| 加载大文件（600MB+） | 加载时间<10秒，不阻塞UI |

---

## 修改历史

| 日期 | 修改内容 | 作者 |
|------|----------|------|
| 2026-05-18 | 修复波段顺序、实现百分比截断、设置 NoData | System |
| 2026-05-19 | 修正波段顺序为标准RGB（红=1、绿=2、蓝=3）、添加Gamma校正（1.55595） | System |
| 2026-05-20 | **修复黑边问题**：修正`opacity`参数错误（从100改为0.0），为单波段渲染器添加透明度设置 | System |
| 2026-05-20 | **优化影像加载体验**：添加进度信号机制，在`QgsMapCanvasWidget`中实现加载进度反馈，在`IMAGEPS`中显示进度对话框，避免界面无响应 | System |
| 2026-05-20 | **修复进度条重复弹出与崩溃问题**：添加`Qt::UniqueConnection`标志防止信号重复连接，在删除对话框前先断开信号连接，使用`deleteLater()`替代`delete`确保安全删除 | System |
| 2026-05-21 | **添加 ShpFilteringDataList_TableW 右键菜单功能**：实现加载矢量(文件)、加载矢量(文件夹)、卸载矢量、查看矢量功能 | System |
| 2026-05-21 | **添加矢量数据列表视图功能**：在视图菜单中添加矢量数据列表视图，为 ShpDataList_TableW 设置表格属性（选择行为、编辑触发器、选择模式、拉伸最后一列） | System |
| 2026-05-22 | **集成矢量数据查看功能**：扩展`QgsMapCanvasWidget`支持矢量图层加载、渲染、坐标转换、显示控制（可见性/透明度）、图层顺序控制等功能 | System |
| 2026-05-23 | **实现统一的矢量查看体验**：创建`on_actionOpenVectorShow`函数，与`on_actionOpenImageShow`保持一致的界面交互逻辑 | System |
| 2026-05-23 | **实现图层切换功能模块**：添加图层信息查询、图层切换、可见性控制等接口 | System |
| 2026-05-24 | **修复查看影像概率性崩溃问题**：1)在构造函数中初始化`m_imageLoadingProgressDialog`为`nullptr`；2)优化清理逻辑，使用`disconnect(ui.openGLWidget, nullptr, this, nullptr)`断开所有信号；3)创建`QProgressDialog`时不指定父对象，避免Qt父子机制冲突 | System |
| 2026-05-25 | **添加 ShpDataList_TableW 保存和读取逻辑**：在工程加载时调用`loadImageData(ui.ShpDataList_TableW, ShpFilteringFilePath, "ShpDataList.xml")`，在工程保存时调用`writeImageXml(ui.ShpDataList_TableW, ShpFilteringFilePath, "ShpDataList.xml")`，实现与其他表格一致的保存和读取功能 | System |
| 2026-05-26 | **在 tab_16 中添加图层切换功能**：1) 在 UI 中添加图层切换工具栏（包含"影像在上"、"矢量在上"按钮，图层选择下拉框，"显示/隐藏"按钮）；2) 实现`onRasterOnTop()`、`onVectorOnTop()`、`onLayerSwitchComboChanged()`、`onToggleLayerVisibility()`、`updateLayerSwitchCombo()`、`extractLayerName()`、`getFullLayerName()` 槽函数；3) 在影像/矢量加载成功后自动更新图层下拉框；4) 利用 QgsMapCanvasWidget 已有的`setLayerOrder()`、`switchToLayer()`、`toggleLayerVisibility()`、`getAllLayerStatus()` 接口实现图层管理功能 | System |
| 2026-05-27 | **修复图层切换功能的同名图层问题**：1) 修改`switchToLayer()`和`toggleLayerVisibility()`支持带类型前缀的图层名称（如"[影像] xxx"、"[矢量] xxx"），确保同名影像和矢量图层能正确区分；2) 修改`updateLayerSwitchCombo()`使用完整图层标识（类型+名称）来恢复选中状态 | System |
| 2026-05-27 | **添加矢量图层颜色区分功能**：1) 在 QgsMapCanvasWidget 中添加`m_vectorLayerColorIndex`成员变量；2) 修改`configureVectorRenderer()`使用8种预定义颜色循环分配给不同矢量图层；3) 在`addVectorLayer()`中添加颜色索引递增逻辑，确保每个新加载的矢量图层都有不同颜色 | System |
| 2026-05-28 | **添加影像图层隐藏和视角切换功能**：1) 在 QgsMapCanvasWidget 中添加`m_rasterLayerVisible`成员变量；2) 修改`getAllLayerStatus()`返回影像图层可见性；3) 修改`updateMapCanvasLayers()`支持影像图层隐藏；4) 修改`toggleLayerVisibility()`支持影像图层可见性切换；5) 修改`switchToLayer()`对影像图层也设置视图范围 | System |
| 2026-07-08 | **修复QtVS Tools Designer打开UI文件崩溃问题**：1) 将`QgsMapCanvasWidget.h`中所有QGIS头文件替换为前向声明；2) 在`QgsMapCanvasWidget.cpp`开头添加完整的QGIS头文件包含；3) 确保所有QGIS类指针和引用在头文件中正确声明，避免Designer进程解析QGIS头文件时因缺少运行时环境而崩溃 | System |

---

## 矢量数据查看功能实现

### 功能概述

参考项目"E:\Project\0508test\0508test"实现矢量数据查看功能，包括：

1. **矢量数据与影像数据精确套合显示**：自动进行坐标系统检测和转换
2. **矢量图层加载**：支持Shapefile格式（.shp）
3. **矢量图层渲染**：根据几何类型（点、线、面）自动配置渲染样式
4. **坐标系统处理**：自动检测CRS不匹配并进行坐标转换
5. **图层控制**：支持显示/隐藏切换、透明度调整、图层顺序控制
6. **错误处理**：优雅处理加载失败和格式错误
7. **统一用户体验**：矢量查看与影像查看使用相同的界面交互逻辑
8. **图层切换**：支持在不同类型图层之间无缝切换

### 修改的文件

| 文件 | 修改内容 |
|------|----------|
| `QgsMapCanvasWidget.h` | 添加矢量图层相关的成员变量、公共接口、信号和辅助函数声明 |
| `QgsMapCanvasWidget.cpp` | 实现矢量图层加载、渲染、坐标转换、显示控制、图层切换等功能 |
| `IMAGEPS.h` | 添加矢量数据文件路径列表成员变量和`on_actionOpenVectorShow`函数声明 |
| `IMAGEPS.cpp` | 实现`on_actionOpenVectorShow`函数，修改查看矢量菜单调用此函数 |

### 核心API

**QgsMapCanvasWidget 矢量图层接口**：

| 函数 | 功能 |
|------|------|
| `addVectorLayer(filePath)` | 加载矢量图层并添加到画布 |
| `removeVectorLayer(layer)` | 移除指定矢量图层 |
| `clearVectorLayers()` | 清除所有矢量图层 |
| `setVectorLayerVisibility(layer, visible)` | 设置图层可见性 |
| `setVectorLayerOpacity(layer, opacity)` | 设置图层透明度 |
| `setLayerOrder(rasterOnTop)` | 设置图层顺序 |
| `checkSpatialReferenceMatch(layer)` | 检查坐标系统匹配 |

**QgsMapCanvasWidget 图层切换接口**：

| 函数 | 功能 |
|------|------|
| `hasRasterLayer()` | 检查是否有影像图层 |
| `getRasterLayerName()` | 获取影像图层名称 |
| `getVectorLayerNames()` | 获取所有矢量图层名称列表 |
| `switchToLayer(layerName)` | 切换到指定图层 |
| `toggleLayerVisibility(layerName)` | 切换图层可见性 |
| `getAllLayerStatus()` | 获取所有图层状态（名称+可见性） |

### 统一用户体验设计

**查看影像与查看矢量的一致性**：

| 行为 | 查看影像 | 查看矢量 |
|------|----------|----------|
| 标签页切换 | 切换到索引1的标签页 | 切换到索引1的标签页 |
| 标签页标题 | 显示文件名 | 显示文件名 |
| 显示模式设置 | 设置为影像显示模式 | 设置为影像显示模式 |
| 关闭逻辑 | 统一的标签页关闭处理 | 统一的标签页关闭处理 |

### 测试用例

| 测试场景 | 预期结果 |
|----------|----------|
| 加载单个Shapefile | 矢量图层成功加载并叠加显示在影像上 |
| 加载多个Shapefile | 多个矢量图层正确叠加显示 |
| CRS匹配的矢量与影像 | 矢量与影像精确套合 |
| CRS不匹配的矢量与影像 | 自动进行坐标转换后正确套合 |
| 点要素图层 | 红色圆点标记（半径3px） |
| 线要素图层 | 蓝色线条（宽度2px） |
| 面要素图层 | 绿色半透明填充（50%透明度） |
| 设置图层透明度 | 图层透明度正确变化 |
| 切换图层可见性 | 图层正确显示/隐藏 |
| 加载无效文件 | 显示警告对话框，不崩溃 |
| 查看矢量 | 切换到标签页1，显示矢量数据 |
| 图层切换 | 视图保持连续，数据显示准确 |

---

*文档版本：v1.7*
*最后更新：2026-05-23*