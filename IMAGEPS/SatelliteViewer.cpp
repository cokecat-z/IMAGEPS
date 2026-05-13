#include "SatelliteViewer.h"
#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QElapsedTimer>
#include <cmath>
#include "IMAGEPS.h"

SatelliteViewer::SatelliteViewer(QWidget* parent)
	: QGraphicsView(parent)
	//m_scene(new QGraphicsScene(this)),
	//m_pixmapItem(nullptr) {
{
	GDALAllRegister();
	//setScene(m_scene);
	setDragMode(QGraphicsView::ScrollHandDrag);
	setRenderHint(QPainter::Antialiasing);
	setRenderHint(QPainter::SmoothPixmapTransform);
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	setResizeAnchor(QGraphicsView::AnchorUnderMouse);
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

SatelliteViewer::~SatelliteViewer() {
	// GDAL会自动清理资源 
}
//
//void SatelliteViewer::loadImage(const QString& path) {
//	m_images.clear();
//	m_currentImagePath = path;
//
//	GDALDataset* dataset = (GDALDataset*)GDALOpen(path.toUtf8(), GA_ReadOnly);
//	if (!dataset) {
//		qWarning() << "Failed to open image:" << path;
//		return;
//	}
//
//	// 初始化图像数据 
//	ImageData newImage;
//	int bandCount = dataset->GetRasterCount();
//	GDALRasterBand* firstBand = dataset->GetRasterBand(1);
//	newImage.dataType = firstBand->GetRasterDataType();
//
//	// 获取地理坐标信息 
//	double geoTransform[6];
//	bool hasGeoTransform = (dataset->GetGeoTransform(geoTransform) == CE_None);
//
//	if (hasGeoTransform) {
//		newImage.minX = geoTransform[0];
//		newImage.maxX = geoTransform[0] + geoTransform[1] * dataset->GetRasterXSize();
//		newImage.minY = geoTransform[3] + geoTransform[5] * dataset->GetRasterYSize();
//		newImage.maxY = geoTransform[3];
//	}
//	else {
//		newImage.minX = 0;
//		newImage.maxX = dataset->GetRasterXSize();
//		newImage.minY = 0;
//		newImage.maxY = dataset->GetRasterYSize();
//	}
//
//	// 设置初始视图 
//	m_viewCenter = QPointF((newImage.minX + newImage.maxX) / 2.0,
//		(newImage.minY + newImage.maxY) / 2.0);
//	float widthRatio = width() / (float)(newImage.maxX - newImage.minX);
//	float heightRatio = height() / (float)(newImage.maxY - newImage.minY);
//	m_viewScale = qMin(widthRatio, heightRatio) * 0.9f;
//
//	m_images.append(newImage);
//	GDALClose(dataset);
//
//	// 加载初始数据 
//	updateImageForCurrentLevel();
//}
//
//void SatelliteViewer::updateImageForCurrentLevel() {
//	if (m_images.isEmpty())  return;
//
//	int overviewLevel = calculateOverviewLevel(m_viewScale);
//	m_images[0].currentOverviewLevel = overviewLevel;
//
//	QImage img = readOverview(overviewLevel);
//	if (img.isNull())  return;
//
//	// 更新或创建pixmap项 
//	if (!m_pixmapItem) {
//		m_pixmapItem = m_scene->addPixmap(QPixmap::fromImage(img));
//		m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
//	}
//	else {
//		m_pixmapItem->setPixmap(QPixmap::fromImage(img));
//	}
//
//	// 更新场景范围 
//	const ImageData& imgData = m_images.first();
//	m_scene->setSceneRect(imgData.minX, imgData.minY,
//		imgData.maxX - imgData.minX,
//		imgData.maxY - imgData.minY);
//
//	updateViewTransform();
//}
//
//void SatelliteViewer::updateViewTransform() {
//	if (m_images.isEmpty())  return;
//
//	const ImageData& img = m_images.first();
//	resetTransform();
//
//	// 计算缩放因子 
//	float scaleX = width() / (img.maxX - img.minX)  * m_viewScale;
//	float scaleY = height() / (img.maxY - img.minY)  * m_viewScale;
//	float scale1 = qMin(scaleX, scaleY);
//
//	// 应用变换 
//	scale(scale1, scale1);
//	centerOn(m_viewCenter);
//}
//
//// 其他方法实现与原始代码类似，主要修改：
//// 1. 移除所有OpenGL相关代码 
//// 2. 将纹理操作改为QPixmap操作 
//// 3. 修改鼠标事件处理以适应QGraphicsView 
//
//void SatelliteViewer::wheelEvent(QWheelEvent* event) {
//	if (m_images.isEmpty()) {
//		QGraphicsView::wheelEvent(event);
//		return;
//	}
//
//	// 计算缩放因子 
//	float scaleFactor = pow(1.2, event->angleDelta().y() / 120.0f);
//	float newScale = qBound(0.01f, m_viewScale * scaleFactor, 100.0f);
//
//	// 更新视图 
//	m_viewScale = newScale;
//	updateImageForCurrentLevel();
//	event->accept();
//}
//
//void SatelliteViewer::mousePressEvent(QMouseEvent* event) {
//	if (event->button() == Qt::LeftButton && !m_images.isEmpty()) {
//		m_lastDragPos = event->pos();
//		m_dragStartViewCenter = m_viewCenter;
//		setCursor(Qt::ClosedHandCursor);
//		event->accept();
//	}
//	else {
//		QGraphicsView::mousePressEvent(event);
//	}
//}
//
//void SatelliteViewer::mouseMoveEvent(QMouseEvent* event) {
//	if (!m_images.isEmpty() && event->buttons() & Qt::LeftButton) {
//		QPoint delta = event->pos() - m_lastDragPos;
//		m_lastDragPos = event->pos();
//
//		// 计算视图中心移动量 
//		QPointF viewDelta = mapToScene(delta) - mapToScene(QPoint(0, 0));
//		m_viewCenter = m_dragStartViewCenter - viewDelta;
//
//		clampViewCenter();
//		updateViewTransform();
//		event->accept();
//	}
//	else {
//		QGraphicsView::mouseMoveEvent(event);
//	}
//}
//
//// 其他方法（如processMultiBand, calculateOverviewLevel等）保持与原始代码相同，
//// 只是移除了OpenGL相关部分 
//
//void SatelliteViewer::clampViewCenter() {
//	if (m_images.isEmpty())  return;
//
//	const ImageData& img = m_images.first();
//	float halfW = width() / (2.0f * m_viewScale);
//	float halfH = height() / (2.0f * m_viewScale);
//
//	// 修改前（错误）：
//	// m_viewCenter.setX(qBound(img.minX  + halfW, m_viewCenter.x(), img.maxX  - halfW));
//	// m_viewCenter.setY(qBound(img.minY  + halfH, m_viewCenter.y(), img.maxY  - halfH));
//
//	// 修改后（正确）：
//	m_viewCenter.setX(qBound(img.minX + halfW, m_viewCenter.x(), img.maxX - halfW));
//	m_viewCenter.setY(qBound(img.minY + halfH, m_viewCenter.y(), img.maxY - halfH));
//
//	// 添加边界情况处理 
//	if (halfW * 2 > (img.maxX - img.minX)) {
//		m_viewCenter.setX((img.minX + img.maxX) / 2.0f);
//	}
//	if (halfH * 2 > (img.maxY - img.minY)) {
//		m_viewCenter.setY((img.minY + img.maxY) / 2.0f);
//	}
//}
//
//QImage SatelliteViewer::processMultispectralBlock(GDALDataset* dataset, int overviewLevel,
//	int x, int y, int width, int height) {
//	// 查找RGB波段（支持自动波段识别）
//	int redBandIdx = 1, greenBandIdx = 2, blueBandIdx = 3;
//	for (int i = 1; i <= dataset->GetRasterCount(); i++) {
//		GDALColorInterp colorType = dataset->GetRasterBand(i)->GetColorInterpretation();
//		if (colorType == GCI_RedBand) redBandIdx = i;
//		else if (colorType == GCI_GreenBand) greenBandIdx = i;
//		else if (colorType == GCI_BlueBand) blueBandIdx = i;
//	}
//
//	// 获取金字塔层级波段 
//	GDALRasterBand* redBand = dataset->GetRasterBand(redBandIdx)->GetOverview(overviewLevel);
//	GDALRasterBand* greenBand = dataset->GetRasterBand(greenBandIdx)->GetOverview(overviewLevel);
//	GDALRasterBand* blueBand = dataset->GetRasterBand(blueBandIdx)->GetOverview(overviewLevel);
//
//	// 读取16位数据 
//	uint16_t* redBuf = new uint16_t[width * height];
//	uint16_t* greenBuf = new uint16_t[width * height];
//	uint16_t* blueBuf = new uint16_t[width * height];
//
//	redBand->RasterIO(GF_Read, x, y, width, height,
//		redBuf, width, height, GDT_UInt16, 0, 0);
//	greenBand->RasterIO(GF_Read, x, y, width, height,
//		greenBuf, width, height, GDT_UInt16, 0, 0);
//	blueBand->RasterIO(GF_Read, x, y, width, height,
//		blueBuf, width, height, GDT_UInt16, 0, 0);
//
//	// 自适应拉伸和gamma校正 
//	auto stretchValues = [](uint16_t* data, int size) {
//		uint16_t minVal = 65535, maxVal = 0;
//		for (int i = 0; i < size; i++) {
//			if (data[i] > 0) { // 忽略0值 
//				minVal = qMin(minVal, data[i]);
//				maxVal = qMax(maxVal, data[i]);
//			}
//		}
//		return qMakePair(minVal, maxVal);
//	};
//
//	auto rRange = stretchValues(redBuf, width * height);
//	auto gRange = stretchValues(greenBuf, width * height);
//	auto bRange = stretchValues(blueBuf, width * height);
//
//	// 转换为QImage 
//	QImage img(width, height, QImage::Format_RGB888);
//	const float gamma = 0.6f;
//	for (int y = 0; y < height; ++y) {
//		QRgb* scanLine = reinterpret_cast<QRgb*>(img.scanLine(y));
//		for (int x = 0; x < width; ++x) {
//			int idx = y * width + x;
//
//			float r = (rRange.second > rRange.first) ?
//				pow((redBuf[idx] - rRange.first) / float(rRange.second - rRange.first), gamma) : 0;
//			float g = (gRange.second > gRange.first) ?
//				pow((greenBuf[idx] - gRange.first) / float(gRange.second - gRange.first), gamma) : 0;
//			float b = (bRange.second > bRange.first) ?
//				pow((blueBuf[idx] - bRange.first) / float(bRange.second - bRange.first), gamma) : 0;
//
//			scanLine[x] = qRgb(
//				static_cast<int>(qBound(0.0f, r, 1.0f) * 255),
//				static_cast<int>(qBound(0.0f, g, 1.0f) * 255),
//				static_cast<int>(qBound(0.0f, b, 1.0f) * 255)
//			);
//		}
//	}
//
//	delete[] redBuf;
//	delete[] greenBuf;
//	delete[] blueBuf;
//	return img;
//}
//
//QImage SatelliteViewer::processPanchromaticBlock(GDALRasterBand* band,
//	int x, int y,
//	int width, int height) {
//	// 读取16位数据 
//	uint16_t* buffer = new uint16_t[width * height];
//	band->RasterIO(GF_Read, x, y, width, height,
//		buffer, width, height, GDT_UInt16, 0, 0);
//
//	// 修复问题2：改进统计值计算（排除极端值）
//	uint16_t minVal = 65535, maxVal = 0;
//	const float percentile = 0.02f; // 排除2%的极端值 
//
//	// 计算直方图 
//	int hist[65536] = { 0 };
//	for (int i = 0; i < width * height; ++i) {
//		if (buffer[i] > 0) hist[buffer[i]]++;
//	}
//
//	// 计算百分比阈值 
//	int totalPixels = width * height;
//	int count = 0;
//	int lowThreshold = 0, highThreshold = 65535;
//
//	for (int i = 0; i < 65536; ++i) {
//		count += hist[i];
//		if (count >= totalPixels * percentile) {
//			lowThreshold = i;
//			break;
//		}
//	}
//
//	count = 0;
//	for (int i = 65535; i >= 0; --i) {
//		count += hist[i];
//		if (count >= totalPixels * percentile) {
//			highThreshold = i;
//			break;
//		}
//	}
//
//	// 使用改进后的范围 
//	minVal = lowThreshold;
//	maxVal = highThreshold;
//
//	// 创建图像（使用gamma校正提升亮度）
//	QImage img(width, height, QImage::Format_Grayscale8);
//	const float gamma = 0.6f; // gamma值可调整 
//
//	for (int y = 0; y < height; ++y) {
//		uchar* scanLine = img.scanLine(y);
//		for (int x = 0; x < width; ++x) {
//			int idx = y * width + x;
//			float normalized = (buffer[idx] - minVal) / float(maxVal - minVal);
//			normalized = qBound(0.0f, normalized, 1.0f);
//			scanLine[x] = static_cast<uchar>(pow(normalized, gamma) * 255);
//		}
//	}
//
//	delete[] buffer;
//	return img;
//}
//
//int SatelliteViewer::calculateOverviewLevel(float viewScale) const {
//	if (m_images.isEmpty()) {
//		qWarning() << "No image loaded";
//		return 0;
//	}
//
//	GDALDataset* dataset = (GDALDataset*)GDALOpen(m_currentImagePath.toUtf8(), GA_ReadOnly);
//	if (!dataset) {
//		qWarning() << "Failed to open dataset for overview calculation";
//		return 0;
//	}
//
//	// 获取基础信息 
//	GDALRasterBand* firstBand = dataset->GetRasterBand(1);
//	int maxOverview = firstBand->GetOverviewCount();
//
//	//// 如果没有金字塔则直接返回0级（全分辨率）
//	//if (maxOverview <= 0) {
//	//	GDALClose(dataset);
//	//	return 0;
//	//}
//
//		// 如果没有金字塔则创建金字塔
//	if (maxOverview <= 0) {
//		GDALClose(dataset); // 先关闭当前数据集 
//
//		// 调用金字塔创建函数 
//		SystemConfig* sysConfig = IMAGEPS::instance->getSystemConfig();
//		if (sysConfig) {
//			QStringList fileList;
//			fileList << m_currentImagePath;
//
//			// 创建金字塔（阻塞方式）
//			sysConfig->acceptModule(QString::fromLocal8Bit("金字塔创建"), fileList);
//
//			// 重新打开数据集检查金字塔
//			dataset = (GDALDataset*)GDALOpen(m_currentImagePath.toUtf8(), GA_ReadOnly);
//			if (dataset) {
//				firstBand = dataset->GetRasterBand(1);
//				maxOverview = firstBand->GetOverviewCount();
//			}
//		}
//
//		// 如果还是没有金字塔则返回0级 
//		if (maxOverview <= 0) {
//			if (dataset) GDALClose(dataset);
//			return 0;
//		}
//	}
//
//	// 计算图像分辨率（单位：像素/米）
//	ImageGeoMetadata* metadata = IMAGEPS::getImageMetadata(m_currentImagePath);
//
//	double geoTransform[6];
//	bool hasGeoTransform = false;
//	//dataset->GetGeoTransform(geoTransform);
//	if (metadata) {
//		geoTransform[0] = metadata->geoTransform[0];
//		geoTransform[1] = metadata->geoTransform[1];
//		geoTransform[2] = metadata->geoTransform[2];
//		geoTransform[3] = metadata->geoTransform[3];
//		geoTransform[4] = metadata->geoTransform[4];
//		geoTransform[5] = metadata->geoTransform[5];
//	}
//	else if (dataset) {
//		hasGeoTransform = (dataset->GetGeoTransform(geoTransform) == CE_None);
//	}
//	float imageResolution = hasGeoTransform ? qAbs(geoTransform[1]) : 1.0f;
//
//	// 计算显示分辨率（单位：像素/米）
//	// viewScale表示1个世界单位对应多少屏幕像素 
//	// devicePixelRatio考虑高DPI屏幕 
//	float displayResolution = viewScale * devicePixelRatioF();
//
//	// 计算分辨率比率（显示分辨率 / 图像分辨率）
//	// 比率>1表示需要缩小显示（使用金字塔）
//	// 比率<1表示需要放大显示（使用更精细的金字塔层级）
//	float ratio = displayResolution / imageResolution;
//
//	// 根据比率选择最优金字塔层级 
//	int optimalLevel = 0;
//	if (maxOverview >= 4 && ratio <= 0.0625f) {  // 1:16 
//		optimalLevel = 4;
//	}
//	else if (ratio <= 0.125f && maxOverview >= 3) {  // 1:8缩小 
//		optimalLevel = 3;
//	}
//	else if (ratio <= 0.25f && maxOverview >= 2) {  // 1:4缩小 
//		optimalLevel = 2;
//	}
//	else if (ratio <= 0.5f && maxOverview >= 1) {  // 1:2缩小 
//		optimalLevel = 1;
//	}
//	else {  // 全分辨率 
//		optimalLevel = 0;
//	}
//
//	// 验证选择的金字塔层级是否有效 
//	optimalLevel = qBound(0, optimalLevel, maxOverview - 1);
//
//	//// 调试信息 
//	//qDebug() << "Overview selection:"
//	//	<< "ImageRes=" << imageResolution
//	//	<< "DisplayRes=" << displayResolution
//	//	<< "Ratio=" << ratio
//	//	<< "SelectedLevel=" << optimalLevel
//	//	<< "MaxAvailable=" << maxOverview;
//
//	GDALClose(dataset);
//	return optimalLevel;
//}
//
//QImage SatelliteViewer::processFloat32Block(GDALRasterBand* band,
//	int x, int y,
//	int width, int height) {
//	// 分配32位浮点缓冲区 
//	float* buffer = new float[width * height];
//
//	// 读取数据 
//	CPLErr err = band->RasterIO(GF_Read, x, y, width, height,
//		buffer, width, height, GDT_Float32, 0, 0);
//	if (err != CE_None) {
//		qWarning() << "Failed to read 32-bit float data";
//		delete[] buffer;
//		return QImage();
//	}
//
//	// 计算有效数据范围（排除NaN和无穷大）
//	float minVal = std::numeric_limits<float>::max();
//	float maxVal = -std::numeric_limits<float>::max();
//	int validCount = 0;
//
//	for (int i = 0; i < width * height; ++i) {
//		if (std::isfinite(buffer[i])) {
//			minVal = qMin(minVal, buffer[i]);
//			maxVal = qMax(maxVal, buffer[i]);
//			validCount++;
//		}
//	}
//
//	// 处理全无效数据情况 
//	if (validCount == 0 || minVal >= maxVal) {
//		minVal = 0;
//		maxVal = 1;
//	}
//
//	// 创建8位灰度图 
//	QImage img(width, height, QImage::Format_Grayscale8);
//	float range = maxVal - minVal;
//	range = (range > 0) ? range : 1.0f;
//
//	for (int row = 0; row < height; ++row) {
//		uchar* scanLine = img.scanLine(row);
//		for (int col = 0; col < width; ++col) {
//			int idx = row * width + col;
//			if (std::isfinite(buffer[idx])) {
//				float normalized = (buffer[idx] - minVal) / range;
//				scanLine[col] = static_cast<uchar>(qBound(0.0f, normalized, 1.0f) * 255);
//			}
//			else {
//				scanLine[col] = 0;
//			}
//		}
//	}
//
//	delete[] buffer;
//	return img;
//}
//
//QImage SatelliteViewer::processSingleBand(GDALRasterBand* band,
//	int x, int y,
//	int width, int height) {
//	// 读取数据 
//	uint8_t* buffer = new uint8_t[width * height];
//	band->RasterIO(GF_Read, x, y, width, height,
//		buffer, width, height, GDT_Byte, 0, 0);
//
//	// 创建灰度图像 
//	QImage img(width, height, QImage::Format_Grayscale8);
//
//	// 直接复制数据（8位无需拉伸）
//	for (int y = 0; y < height; ++y) {
//		uchar* scanLine = img.scanLine(y);
//		memcpy(scanLine, buffer + y * width, width);
//	}
//
//	delete[] buffer;
//	return img;
//}
//
//QImage SatelliteViewer::processSingleBand16(GDALRasterBand* band,
//	int x, int y,
//	int width, int height) {
//	// 读取16位数据 
//	uint16_t* buffer = new uint16_t[width * height];
//	band->RasterIO(GF_Read, x, y, width, height,
//		buffer, width, height, GDT_UInt16, 0, 0);
//
//	// 改进的统计值计算（排除极端值）
//	uint16_t minVal = 65535, maxVal = 0;
//	const float excludePercent = 0.02f; // 排除2%极端值 
//
//	// 计算直方图 
//	int hist[65536] = { 0 };
//	for (int i = 0; i < width * height; ++i) {
//		if (buffer[i] > 0) hist[buffer[i]]++;
//	}
//
//	// 计算最小阈值 
//	int total = width * height;
//	int count = 0;
//	for (int i = 0; i < 65536; ++i) {
//		count += hist[i];
//		if (count >= total * excludePercent) {
//			minVal = i;
//			break;
//		}
//	}
//
//	// 计算最大阈值 
//	count = 0;
//	for (int i = 65535; i >= 0; --i) {
//		count += hist[i];
//		if (count >= total * excludePercent) {
//			maxVal = i;
//			break;
//		}
//	}
//
//	// 创建图像（带gamma校正提升亮度）
//	QImage img(width, height, QImage::Format_Grayscale8);
//	const float gamma = 0.7f; // 可调整的gamma值 
//
//	for (int y = 0; y < height; ++y) {
//		uchar* scanLine = img.scanLine(y);
//		for (int x = 0; x < width; ++x) {
//			int idx = y * width + x;
//			float val = (buffer[idx] - minVal) / float(maxVal - minVal);
//			val = qBound(0.0f, val, 1.0f);
//			scanLine[x] = static_cast<uchar>(pow(val, gamma) * 255);
//		}
//	}
//
//	delete[] buffer;
//	return img;
//}
//
//QImage SatelliteViewer::process8BitMultiBand(GDALRasterBand* bands[3], const QRect& region) {
//	std::unique_ptr<uint8_t[]> buffers[3];
//
//	try {
//		// 分配内存 
//		for (int i = 0; i < 3; ++i) {
//			buffers[i].reset(new uint8_t[region.width() * region.height()]);
//		}
//
//		// 读取数据 
//		for (int i = 0; i < 3; ++i) {
//			CPLErr err = bands[i]->RasterIO(
//				GF_Read,
//				region.x(), region.y(),
//				region.width(), region.height(),
//				buffers[i].get(),
//				region.width(), region.height(),
//				GDT_Byte, 0, 0
//			);
//
//			if (err != CE_None) {
//				qWarning() << "RasterIO failed for band" << i;
//				return QImage();
//			}
//		}
//
//		// 创建图像 
//		QImage img(region.size(), QImage::Format_RGB888);
//
//		for (int y = 0; y < region.height(); ++y) {
//			uchar* scanLine = img.scanLine(y);
//			for (int x = 0; x < region.width(); ++x) {
//				int idx = y * region.width() + x;
//				scanLine[x * 3 + 0] = buffers[0][idx]; // R 
//				scanLine[x * 3 + 1] = buffers[1][idx]; // G 
//				scanLine[x * 3 + 2] = buffers[2][idx]; // B 
//			}
//		}
//
//		return img;
//	}
//	catch (const std::bad_alloc&) {
//		qCritical() << "Memory allocation failed";
//		return QImage();
//	}
//}
//
//QImage SatelliteViewer::process16BitMultiBand(GDALRasterBand* bands[3], const QRect& region) {
//	std::unique_ptr<uint16_t[]> buffers[3];
//
//	try {
//		// 分配内存 
//		const int pixelCount = region.width()  * region.height();
//		for (int i = 0; i < 3; ++i) {
//			buffers[i].reset(new uint16_t[pixelCount]);
//		}
//
//		// 读取数据（使用进度提示）
//		//qDebug() << "Loading 16-bit multi-band data...";
//		QElapsedTimer timer;
//		timer.start();
//
//		for (int i = 0; i < 3; ++i) {
//			CPLErr err = bands[i]->RasterIO(
//				GF_Read,
//				region.x(), region.y(),
//				region.width(), region.height(),
//				buffers[i].get(),
//				region.width(), region.height(),
//				GDT_UInt16, 0, 0
//			);
//
//			if (err != CE_None) {
//				qWarning() << "RasterIO failed for band" << i;
//				return QImage();
//			}
//		}
//
//		//qDebug() << "Data loaded in" << timer.elapsed() << "ms";
//
//		// 改进的统计值计算（排除极端值）
//		uint16_t minVals[3] = { 65535, 65535, 65535 };
//		uint16_t maxVals[3] = { 0, 0, 0 };
//		const float excludePercent = 0.01f; // 更严格的排除比例 
//
//		// 并行计算统计值（使用OpenMP）
//#pragma omp parallel for 
//		for (int i = 0; i < 3; ++i) {
//			// 计算直方图 
//			int hist[65536] = { 0 };
//			for (int j = 0; j < pixelCount; ++j) {
//				if (buffers[i][j] > 0) hist[buffers[i][j]]++;
//			}
//
//			// 计算有效范围（改进算法）
//			int totalValid = 0;
//			for (int j = 0; j < 65536; ++j) {
//				totalValid += hist[j];
//			}
//
//			// 计算最小阈值（排除暗部噪声）
//			int count = 0;
//			int lowThreshold = 0;
//			for (int j = 0; j < 65536; ++j) {
//				count += hist[j];
//				if (count >= totalValid * excludePercent) {
//					lowThreshold = j;
//					break;
//				}
//			}
//
//			// 计算最大阈值（排除高光溢出）
//			count = 0;
//			int highThreshold = 65535;
//			for (int j = 65535; j >= 0; --j) {
//				count += hist[j];
//				if (count >= totalValid * excludePercent) {
//					highThreshold = j;
//					break;
//				}
//			}
//
//			minVals[i] = lowThreshold;
//			maxVals[i] = highThreshold;
//
//			//qDebug() << "Band" << i << "range:" << minVals[i] << "-" << maxVals[i];
//		}
//
//		// 创建图像（改进的色调映射）
//		QImage img(region.size(), QImage::Format_RGB888);
//		const float gamma = 0.8f; // 调整gamma值 
//		const float contrast = 1.2f; // 对比度增强 
//
//		for (int y = 0; y < region.height(); ++y) {
//			uchar* scanLine = img.scanLine(y);
//			for (int x = 0; x < region.width(); ++x) {
//				int idx = y * region.width() + x;
//
//				for (int i = 0; i < 3; ++i) {
//					// 改进的色调映射算法 
//					float normalized = (buffers[i][idx] - minVals[i]) /
//						float(maxVals[i] - minVals[i]);
//					normalized = qBound(0.0f, normalized, 1.0f);
//
//					// 对比度增强 
//					normalized = (normalized - 0.5f) * contrast + 0.5f;
//					normalized = qBound(0.0f, normalized, 1.0f);
//
//					// Gamma校正 
//					scanLine[x * 3 + i] = static_cast<uchar>(pow(normalized, gamma) * 255);
//				}
//			}
//		}
//
//		// 锐化处理（可选）
//		if (region.width() > 512) { // 对大图像进行锐化 
//			QImage sharpened = img;
//			QPainter painter(&sharpened);
//			painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//			painter.drawImage(0, 0, img);
//			painter.end();
//			return sharpened;
//		}
//
//		return img;
//	}
//	catch (const std::bad_alloc&) {
//		qCritical() << "Memory allocation failed";
//		return QImage();
//	}
//}
//
//QImage SatelliteViewer::processMultiBand(GDALDataset* dataset,
//	int overviewLevel,
//	const QRect& region) {
//	// 输入验证 
//	if (!dataset) {
//		qCritical() << "Null dataset pointer";
//		return QImage();
//	}
//
//	try {
//		// 获取波段信息 
//		const int bandCount = dataset->GetRasterCount();
//		if (bandCount < 3) {
//			qWarning() << "Insufficient bands:" << bandCount;
//			return QImage();
//		}
//
//		// 自动识别RGB波段（带保护）
//		int bands[3] = { 1, 2, 3 }; // 默认RGB顺序 
//		bool hasColorInfo = false;
//
//		for (int i = 1; i <= qMin(bandCount, 10); i++) { // 限制检查前10个波段 
//			GDALRasterBand* band = dataset->GetRasterBand(i);
//			if (!band) continue;
//
//			GDALColorInterp colorType = band->GetColorInterpretation();
//			switch (colorType) {
//			case GCI_RedBand:    bands[0] = i; hasColorInfo = true; break;
//			case GCI_GreenBand:  bands[1] = i; hasColorInfo = true; break;
//			case GCI_BlueBand:   bands[2] = i; hasColorInfo = true; break;
//			default: break;
//			}
//		}
//
//		// 获取金字塔层级（带范围检查）
//		GDALRasterBand* redBand = dataset->GetRasterBand(bands[0]);
//		if (!redBand) {
//			qCritical() << "Invalid red band";
//			return QImage();
//		}
//
//		const int overviewCount = redBand->GetOverviewCount();
//		overviewLevel = qBound(0, overviewLevel, qMax(0, overviewCount - 1));
//
//		// 获取金字塔波段（带空指针检查）
//		GDALRasterBand* ovrBands[3] = {
//			redBand->GetOverview(overviewLevel),
//			dataset->GetRasterBand(bands[1])->GetOverview(overviewLevel),
//			dataset->GetRasterBand(bands[2])->GetOverview(overviewLevel)
//		};
//
//		for (int i = 0; i < 3; ++i) {
//			if (!ovrBands[i]) {
//				qCritical() << "Null overview band:" << i;
//				return QImage();
//			}
//		}
//
//		// 确定处理区域 
//		const int width = ovrBands[0]->GetXSize();
//		const int height = ovrBands[0]->GetYSize();
//
//		QRect safeRegion = region.isEmpty() ? QRect(0, 0, width, height) : region;
//		safeRegion = safeRegion.intersected(QRect(0, 0, width, height));
//
//		if (safeRegion.isEmpty()) {
//			qWarning() << "Empty region after intersection";
//			return QImage();
//		}
//
//		// 根据数据类型分派处理 
//		const GDALDataType dataType = ovrBands[0]->GetRasterDataType();
//
//		if (dataType == GDT_Byte) {
//			return process8BitMultiBand(ovrBands, safeRegion);
//		}
//		else if (dataType == GDT_UInt16) {
//			return process16BitMultiBand(ovrBands, safeRegion);
//		}
//		else {
//			qWarning() << "Unsupported data type:" << dataType;
//			return QImage();
//		}
//	}
//	catch (const std::exception& e) {
//		qCritical() << "Exception in processMultiBand:" << e.what();
//		return QImage();
//	}
//}
//
//QPointF SatelliteViewer::screenToWorld(const QPoint& screenPos) const
//{
//	if (m_images.isEmpty())
//		return QPointF();
//
//	const ImageData& img = m_images.first();
//
//	// 计算视口范围 
//	float halfW = width() / (2.0f * m_viewScale);
//	float halfH = height() / (2.0f * m_viewScale);
//
//	// 转换为世界坐标 
//	double worldX = m_viewCenter.x() + (screenPos.x() - width() / 2.0) / m_viewScale;
//	double worldY = m_viewCenter.y() - (screenPos.y() - height() / 2.0) / m_viewScale;
//
//	return QPointF(worldX, worldY);
//}