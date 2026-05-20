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
- 通过对比度增强算法模拟部分Gamma效果

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

---

*文档版本：v1.2*
*最后更新：2026-05-20*