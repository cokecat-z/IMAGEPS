#include <QDebug>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QApplication>
#include <QElapsedTimer>
#include <cmath>

// GDAL headers - moved from header to avoid Qt Designer crash
#include "gdal_priv.h"

#include "GLDisplayWidget.h"
#include "IMAGEPS.h"

GLDisplayWidget::GLDisplayWidget(QWidget* parent) : QOpenGLWidget(parent) {
	if (!QCoreApplication::applicationFilePath().contains("designer", Qt::CaseInsensitive)) {
		GDALAllRegister();
	}
	setFocusPolicy(Qt::StrongFocus);
	m_imagePS = nullptr;
}

GLDisplayWidget::~GLDisplayWidget() {
	makeCurrent();
	for (auto& img : m_images) {
		if (img.texture) {
			img.texture->destroy();
			delete img.texture;
		}
	}
	doneCurrent();
}

// 设置 IMAGEPS 实例 
void GLDisplayWidget::setImagePS(IMAGEPS* imagePS)
{
	m_imagePS = imagePS;
}

void GLDisplayWidget::initializeGL() {
	initializeOpenGLFunctions();
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void GLDisplayWidget::loadImage(const QString& path) {
	makeCurrent();

	// 清理现有资源 
	for (auto& img : m_images) {
		if (img.texture) {
			img.texture->destroy();
			delete img.texture;
		}
	}
	m_images.clear();

	GDALDataset* dataset = (GDALDataset*)GDALOpen(path.toUtf8(), GA_ReadOnly);
	if (!dataset) {
		qWarning() << "Failed to open image:" << path;
		doneCurrent();
		return;
	}

	// 初始化图像数据 
	ImageData newImage;
	m_currentImagePath = path;
	int bandCount = dataset->GetRasterCount();
	GDALRasterBand* firstBand = dataset->GetRasterBand(1);
	newImage.dataType = firstBand->GetRasterDataType();

	ImageGeoMetadata* metadata = m_imagePS->getImageMetadata(path);

	double geoTransform[6];
	bool hasGeoTransform = false;
	if (metadata) {
		geoTransform[0] = metadata->geoTransform[0];
		geoTransform[1] = metadata->geoTransform[1];
		geoTransform[2] = metadata->geoTransform[2];
		geoTransform[3] = metadata->geoTransform[3];
		geoTransform[4] = metadata->geoTransform[4];
		geoTransform[5] = metadata->geoTransform[5];
	}
	else if (dataset) {
		hasGeoTransform = (dataset->GetGeoTransform(geoTransform) == CE_None);
	}

	if (hasGeoTransform || metadata) {
		newImage.minX = geoTransform[0];
		newImage.maxX = geoTransform[0] + geoTransform[1] * dataset->GetRasterXSize();
		newImage.minY = geoTransform[3] + geoTransform[5] * dataset->GetRasterYSize();
		newImage.maxY = geoTransform[3];
	}
	else {
		newImage.minX = 0;
		newImage.maxX = dataset->GetRasterXSize();
		newImage.minY = 0;
		newImage.maxY = dataset->GetRasterYSize();
	}

	// 创建纹理 
	newImage.texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	newImage.texture->create();

	// 根据波段数设置纹理格式 
	if (bandCount == 1) {
		// 单波段处理 
		newImage.texture->setFormat(QOpenGLTexture::R16_UNorm);
		newImage.texture->setSize(dataset->GetRasterXSize(), dataset->GetRasterYSize());
		newImage.texture->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt16);
	}
	else {
		// 多波段处理 
		newImage.texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
		newImage.texture->setSize(dataset->GetRasterXSize(), dataset->GetRasterYSize());
		newImage.texture->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
	}

	// 设置初始视图 
	m_viewCenter = QPointF((newImage.minX + newImage.maxX) / 2.0,
		(newImage.minY + newImage.maxY) / 2.0);
	float widthRatio = width() / (float)(newImage.maxX - newImage.minX);
	float heightRatio = height() / (float)(newImage.maxY - newImage.minY);
	m_viewScale = qMin(widthRatio, heightRatio) * 0.9f;

	m_images.append(newImage);
	GDALClose(dataset);

	// 加载初始数据 
	updateTextureForCurrentLevel();
	doneCurrent();
	update();
}

void GLDisplayWidget::loadMultispectralImage(const QString& path) {
	makeCurrent();

	// 清理现有纹理 
	for (auto& img : m_images) {
		if (img.texture) {
			img.texture->destroy();
			delete img.texture;
		}
	}
	m_images.clear();

	GDALDataset* dataset = (GDALDataset*)GDALOpen(path.toUtf8(), GA_ReadOnly);
	if (!dataset) {
		qWarning() << "Failed to open image:" << path;
		doneCurrent();
		return;
	}

	// 初始化图像数据结构 
	ImageData newImage;
	m_currentImagePath = path;
	newImage.dataType = dataset->GetRasterBand(1)->GetRasterDataType();

	ImageGeoMetadata* metadata = m_imagePS->getImageMetadata(path);

	double geoTransform[6];
	bool hasGeoTransform = false;
	if (metadata) {
		geoTransform[0] = metadata->geoTransform[0];
		geoTransform[1] = metadata->geoTransform[1];
		geoTransform[2] = metadata->geoTransform[2];
		geoTransform[3] = metadata->geoTransform[3];
		geoTransform[4] = metadata->geoTransform[4];
		geoTransform[5] = metadata->geoTransform[5];
	}
	else if (dataset) {
		hasGeoTransform = (dataset->GetGeoTransform(geoTransform) == CE_None);
	}

	if (hasGeoTransform || metadata) {
		newImage.minX = geoTransform[0];
		newImage.maxX = geoTransform[0] + geoTransform[1] * dataset->GetRasterXSize();
		newImage.minY = geoTransform[3] + geoTransform[5] * dataset->GetRasterYSize();
		newImage.maxY = geoTransform[3];
	}
	else {
		newImage.minX = 0;
		newImage.maxX = dataset->GetRasterXSize();
		newImage.minY = 0;
		newImage.maxY = dataset->GetRasterYSize();
	}

	// 创建纹理 
	newImage.texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	newImage.texture->create();
	newImage.texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
	newImage.texture->setSize(dataset->GetRasterXSize(), dataset->GetRasterYSize());
	newImage.texture->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);

	// 初始视图设置 
	m_viewCenter = QPointF((newImage.minX + newImage.maxX) / 2,
		(newImage.minY + newImage.maxY) / 2);
	m_viewScale = qMin(width() / (newImage.maxX - newImage.minX),
		height() / (newImage.maxY - newImage.minY))  * 0.9f;

	m_images.append(newImage);
	GDALClose(dataset);

	// 加载初始金字塔层级 
	updateTextureForCurrentLevel();
	doneCurrent();
}

void GLDisplayWidget::loadPanchromaticImage(const QString& path) {
	makeCurrent();

	// 清理现有纹理 
	for (auto& img : m_images) {
		if (img.texture) {
			img.texture->destroy();
			delete img.texture;
		}
	}
	m_images.clear();

	GDALDataset* dataset = (GDALDataset*)GDALOpen(path.toUtf8(), GA_ReadOnly);
	if (!dataset) {
		qWarning() << "Failed to open image:" << path;
		doneCurrent();
		return;
	}

	// 初始化图像数据结构 
	ImageData newImage;
	m_currentImagePath = path;
	newImage.dataType = dataset->GetRasterBand(1)->GetRasterDataType();

	ImageGeoMetadata* metadata = m_imagePS->getImageMetadata(path);

	double geoTransform[6];
	bool hasGeoTransform = false;
	if (metadata) {
		geoTransform[0] = metadata->geoTransform[0];
		geoTransform[1] = metadata->geoTransform[1];
		geoTransform[2] = metadata->geoTransform[2];
		geoTransform[3] = metadata->geoTransform[3];
		geoTransform[4] = metadata->geoTransform[4];
		geoTransform[5] = metadata->geoTransform[5];
	}
	else if (dataset) {
		hasGeoTransform = (dataset->GetGeoTransform(geoTransform) == CE_None);
	}

	if (hasGeoTransform || metadata) {
		newImage.minX = geoTransform[0];
		newImage.maxX = geoTransform[0] + geoTransform[1] * dataset->GetRasterXSize();
		newImage.minY = geoTransform[3] + geoTransform[5] * dataset->GetRasterYSize();
		newImage.maxY = geoTransform[3];
	}

	newImage.texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	newImage.texture->create();
	newImage.texture->setFormat(QOpenGLTexture::R16_UNorm);
	newImage.texture->setSize(dataset->GetRasterXSize(), dataset->GetRasterYSize());

	newImage.texture->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt16);

	// 初始视图设置 
	m_viewCenter = QPointF((newImage.minX + newImage.maxX) / 2,
		(newImage.minY + newImage.maxY) / 2);
	m_viewScale = qMin(width() / (newImage.maxX - newImage.minX),
		height() / (newImage.maxY - newImage.minY))  * 0.9f;

	m_images.append(newImage);
	GDALClose(dataset);

	// 加载初始金字塔层级 
	updateTextureForCurrentLevel();
	doneCurrent();
}

QImage GLDisplayWidget::readOverview(int overviewLevel, const QRect& region) {
	GDALDataset* dataset = (GDALDataset*)GDALOpen(m_currentImagePath.toUtf8(), GA_ReadOnly);
	if (!dataset) return QImage();

	const int bandCount = dataset->GetRasterCount();
	GDALRasterBand* firstBand = dataset->GetRasterBand(1);

	// 验证金字塔层级有效性 
	const int maxOverview = firstBand->GetOverviewCount();
	overviewLevel = qBound(0, overviewLevel, qMax(0, maxOverview - 1));

	if (bandCount == 1) {
		// 单波段处理（使用修改后的processPanchromaticBlock）
		GDALRasterBand* band = firstBand->GetOverview(overviewLevel);
		int width = band->GetXSize();
		int height = band->GetYSize();
		QRect readRegion = region.isEmpty() ? QRect(0, 0, width, height) : region;
		return processPanchromaticBlock(band, readRegion.x(), readRegion.y(),
			readRegion.width(), readRegion.height());
	}
	else if (bandCount >= 3) {
		// 查找RGB波段（支持自动波段识别）
		int redBandIdx = 1, greenBandIdx = 2, blueBandIdx = 3;
		for (int i = 1; i <= bandCount; i++) {
			GDALColorInterp colorType = dataset->GetRasterBand(i)->GetColorInterpretation();
			if (colorType == GCI_RedBand) redBandIdx = i;
			else if (colorType == GCI_GreenBand) greenBandIdx = i;
			else if (colorType == GCI_BlueBand) blueBandIdx = i;
		}

		GDALRasterBand* redBand = dataset->GetRasterBand(redBandIdx)->GetOverview(overviewLevel);
		int width = redBand->GetXSize();
		int height = redBand->GetYSize();

		QRect readRegion = region.isEmpty() ? QRect(0, 0, width, height) : region;

		// 读取16位数据 
		uint16_t* redBuf = new uint16_t[readRegion.width() * readRegion.height()];
		uint16_t* greenBuf = new uint16_t[readRegion.width() * readRegion.height()];
		uint16_t* blueBuf = new uint16_t[readRegion.width() * readRegion.height()];

		redBand->RasterIO(GF_Read, readRegion.x(), readRegion.y(),
			readRegion.width(), readRegion.height(),
			redBuf, readRegion.width(), readRegion.height(),
			GDT_UInt16, 0, 0);

		dataset->GetRasterBand(greenBandIdx)->GetOverview(overviewLevel)
			->RasterIO(GF_Read, readRegion.x(), readRegion.y(),
				readRegion.width(), readRegion.height(),
				greenBuf, readRegion.width(), readRegion.height(),
				GDT_UInt16, 0, 0);

		dataset->GetRasterBand(blueBandIdx)->GetOverview(overviewLevel)
			->RasterIO(GF_Read, readRegion.x(), readRegion.y(),
				readRegion.width(), readRegion.height(),
				blueBuf, readRegion.width(), readRegion.height(),
				GDT_UInt16, 0, 0);

		// 自适应拉伸和gamma校正 
		auto stretchValues = [](uint16_t* data, int size) {
			uint16_t minVal = 65535, maxVal = 0;
			for (int i = 0; i < size; i++) {
				if (data[i] > 0) { // 忽略0值 
					minVal = qMin(minVal, data[i]);
					maxVal = qMax(maxVal, data[i]);
				}
			}
			return qMakePair(minVal, maxVal);
		};

		auto rRange = stretchValues(redBuf, readRegion.width()  * readRegion.height());
		auto gRange = stretchValues(greenBuf, readRegion.width()  * readRegion.height());
		auto bRange = stretchValues(blueBuf, readRegion.width()  * readRegion.height());

		// 转换为QImage（带自适应拉伸）
		QImage img(readRegion.size(), QImage::Format_RGB888);
		const float gamma = 0.6f;
		for (int y = 0; y < readRegion.height(); ++y) {
			for (int x = 0; x < readRegion.width(); ++x) {
				int idx = y * readRegion.width() + x;

				float r = (rRange.second > rRange.first) ?
					pow((redBuf[idx] - rRange.first) / float(rRange.second - rRange.first), gamma) : 0;
				float g = (gRange.second > gRange.first) ?
					pow((greenBuf[idx] - gRange.first) / float(gRange.second - gRange.first), gamma) : 0;
				float b = (bRange.second > bRange.first) ?
					pow((blueBuf[idx] - bRange.first) / float(bRange.second - bRange.first), gamma) : 0;

				img.setPixelColor(x, y, QColor(
					static_cast<int>(qBound(0.0f, r, 1.0f) * 255),
					static_cast<int>(qBound(0.0f, g, 1.0f) * 255),
					static_cast<int>(qBound(0.0f, b, 1.0f) * 255)
				));
			}
		}

		delete[] redBuf;
		delete[] greenBuf;
		delete[] blueBuf;
		GDALClose(dataset);
		return img;
	}
	// 全色影像处理（单波段16位）
	else {
		GDALRasterBand* band = firstBand->GetOverview(overviewLevel);
		int width = band->GetXSize();
		int height = band->GetYSize();

		QRect readRegion = region.isEmpty() ? QRect(0, 0, width, height) : region;

		// 读取16位数据 
		uint16_t* buffer = new uint16_t[readRegion.width() * readRegion.height()];
		band->RasterIO(GF_Read, readRegion.x(), readRegion.y(),
			readRegion.width(), readRegion.height(),
			buffer, readRegion.width(), readRegion.height(),
			GDT_UInt16, 0, 0);

		// 计算统计值（忽略0值）
		uint16_t minVal = 65535, maxVal = 0;
		for (int i = 0; i < readRegion.width() * readRegion.height(); ++i) {
			if (buffer[i] > 0) {
				minVal = qMin(minVal, buffer[i]);
				maxVal = qMax(maxVal, buffer[i]);
			}
		}

		// 转换为16位灰度图 
		QImage img(readRegion.size(), QImage::Format_Grayscale16);
		for (int y = 0; y < readRegion.height(); ++y) {
			uint16_t* scanLine = reinterpret_cast<uint16_t*>(img.scanLine(y));
			for (int x = 0; x < readRegion.width(); ++x) {
				int idx = y * readRegion.width() + x;
				scanLine[x] = buffer[idx] > 0 ?
					static_cast<uint16_t>((buffer[idx] - minVal) / float(maxVal - minVal) * 65535) : 0;
			}
		}

		delete[] buffer;
		GDALClose(dataset);
		return img;
	}
}

void GLDisplayWidget::updateTextureForCurrentLevel() {
	if (m_images.isEmpty())  return;

	makeCurrent();

	// 强制释放旧纹理 
	if (m_images[0].texture) {
		m_images[0].texture->destroy();
		delete m_images[0].texture;
		m_images[0].texture = nullptr;
	}

	GDALDataset* dataset = (GDALDataset*)GDALOpen(m_currentImagePath.toUtf8(), GA_ReadOnly);
	if (!dataset) {
		doneCurrent();
		return;
	}

	int bandCount = dataset->GetRasterCount();
	int overviewLevel = calculateOverviewLevel(m_viewScale);
	QImage img;

	if (bandCount == 1) {
		// 单波段处理 
		GDALRasterBand* band = dataset->GetRasterBand(1)->GetOverview(overviewLevel);
		if (!band) {
			GDALClose(dataset);
			doneCurrent();
			return;
		}

		if (dataset->GetRasterBand(1)->GetRasterDataType() == GDT_Byte) {
			img = processSingleBand(band, 0, 0, band->GetXSize(), band->GetYSize());
		}
		else {
			img = processSingleBand16(band, 0, 0, band->GetXSize(), band->GetYSize());
		}
	}
	else {
		//// 多波段处理 - 确保使用正确的波段顺序 
		//img = processMultiBand(dataset, overviewLevel,
		//	QRect(0, 0,
		//		dataset->GetRasterBand(1)->GetOverview(overviewLevel)->GetXSize(),
		//		dataset->GetRasterBand(1)->GetOverview(overviewLevel)->GetYSize()));

		// 多波段处理 - 确保使用正确的波段顺序 
		GDALRasterBand* band1 = dataset->GetRasterBand(1);
		GDALRasterBand* overviewBand1 = band1->GetOverview(overviewLevel);
		if (!overviewBand1) {
			// 如果没有概览，使用原始影像数据 
			overviewBand1 = band1;
		}

		img = processMultiBand(dataset, overviewLevel,
			QRect(0, 0,
				overviewBand1->GetXSize(),
				overviewBand1->GetYSize()));

		// 确保RGB顺序正确 
		img = ensureRGBOrder(img, dataset);
	}

	// 创建新纹理 
	m_images[0].texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	m_images[0].texture->create();

	if (bandCount == 1) {
		m_images[0].texture->setFormat(QOpenGLTexture::R8_UNorm);
		m_images[0].texture->setSize(img.width(), img.height());
		m_images[0].texture->allocateStorage(QOpenGLTexture::Red, QOpenGLTexture::UInt8);

		m_images[0].texture->bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			img.width(), img.height(),
			GL_RED, GL_UNSIGNED_BYTE,
			img.constBits());

		// 设置单通道显示为灰度 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
	}
	else {
		QImage glImage = img.convertToFormat(QImage::Format_RGBA8888);
		m_images[0].texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
		m_images[0].texture->setSize(glImage.width(), glImage.height());
		m_images[0].texture->allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);

		m_images[0].texture->bind();
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			glImage.width(), glImage.height(),
			GL_RGBA, GL_UNSIGNED_BYTE,
			glImage.constBits());
	}

	// 公共纹理参数 
	m_images[0].texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
	m_images[0].texture->setWrapMode(QOpenGLTexture::ClampToEdge);

	GDALClose(dataset);
	doneCurrent();
	update();
}

QImage GLDisplayWidget::ensureRGBOrder(const QImage& inputImage, GDALDataset* dataset) {
	// 自动识别RGB波段 
	int redBand = 1, greenBand = 2, blueBand = 3;

	for (int i = 1; i <= dataset->GetRasterCount(); ++i) {
		GDALColorInterp colorType = dataset->GetRasterBand(i)->GetColorInterpretation();
		if (colorType == GCI_RedBand) redBand = i;
		else if (colorType == GCI_GreenBand) greenBand = i;
		else if (colorType == GCI_BlueBand) blueBand = i;
	}

	// 如果已经是RGB顺序则直接返回 
	if (redBand == 1 && greenBand == 2 && blueBand == 3) {
		return inputImage;
	}

	// 否则重新排列波段 
	QImage outputImage(inputImage.size(), QImage::Format_RGB888);
	for (int y = 0; y < inputImage.height(); ++y) {
		const uchar* inLine = inputImage.constScanLine(y);
		uchar* outLine = outputImage.scanLine(y);

		for (int x = 0; x < inputImage.width(); ++x) {
			outLine[x * 3 + 0] = inLine[x * 3 + (redBand - 1)];   // R 
			outLine[x * 3 + 1] = inLine[x * 3 + (greenBand - 1)]; // G 
			outLine[x * 3 + 2] = inLine[x * 3 + (blueBand - 1)];  // B 
		}
	}

	return outputImage;
}

void GLDisplayWidget::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (m_images.isEmpty())  return;

	const ImageData& img = m_images.first();
	if (!img.texture || !img.texture->isCreated())  return;

	// 设置投影矩阵
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float imageWidth = img.maxX - img.minX;
	float imageHeight = img.maxY - img.minY;
	float widgetAspectRatio = width() / (float)height();
	float imageAspectRatio = (img.maxX - img.minX) / (img.maxY - img.minY);

	float halfW, halfH;

	if (widgetAspectRatio > imageAspectRatio) {
		halfW = (img.maxX - img.minX)  * m_viewScale * widgetAspectRatio / 2.0f;
		halfH = (img.maxY - img.minY)  * m_viewScale / 2.0f;
	}
	else {
		halfW = (img.maxX - img.minX)  * m_viewScale / 2.0f;
		halfH = (img.maxY - img.minY)  * m_viewScale / (2.0f * widgetAspectRatio);
	}

	glOrtho(m_viewCenter.x() - halfW, m_viewCenter.x() + halfW,
		m_viewCenter.y() + halfH, m_viewCenter.y() - halfH, -1, 1);

	// 绘制纹理
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	img.texture->bind();

	// 绘制带有正确纹理坐标的图像 
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0); glVertex2f(img.minX, img.minY);
	glTexCoord2f(1, 0); glVertex2f(img.maxX, img.minY);
	glTexCoord2f(1, 1); glVertex2f(img.maxX, img.maxY);
	glTexCoord2f(0, 1); glVertex2f(img.minX, img.maxY);
	glEnd();
}

void GLDisplayWidget::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);
	if (!m_images.isEmpty()) {
		const ImageData& img = m_images.first();
		float imageAspectRatio = (img.maxX - img.minX) / (img.maxY - img.minY);
		float widgetAspectRatio = w / (float)h;

		if (widgetAspectRatio > imageAspectRatio) {
			m_viewScale = w / (img.maxX - img.minX)  * 0.9f;
		}
		else {
			m_viewScale = h / (img.maxY - img.minY)  * 0.9f;
		}
	}
}

void GLDisplayWidget::mousePressEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton && !m_images.isEmpty()) {
		m_lastDragPos = event->pos();
		m_dragStartViewCenter = m_viewCenter;
		setCursor(Qt::ClosedHandCursor);
		event->accept();
	}
	else {
		QOpenGLWidget::mousePressEvent(event);
	}
}

void GLDisplayWidget::mouseMoveEvent(QMouseEvent* event) {
	if (!m_images.isEmpty() && event->buttons() & Qt::LeftButton) {
		QPoint delta = event->pos() - m_lastDragPos;
		m_lastDragPos = event->pos();

		// 修改后（仅调整X轴方向）：
		m_viewCenter.rx() -= delta.x() / m_viewScale;  // X轴保持原样 
		m_viewCenter.ry() -= delta.y() / m_viewScale;  // Y轴改为+=

		clampViewCenter();
		update();
		event->accept();
	}
	else {
		QOpenGLWidget::mouseMoveEvent(event);
	}
}

void GLDisplayWidget::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton) {
		setCursor(Qt::ArrowCursor);
		event->accept();
	}
	else {
		QOpenGLWidget::mouseReleaseEvent(event);
	}
}

void GLDisplayWidget::wheelEvent(QWheelEvent* event) {
	if (m_images.isEmpty()) {
		QOpenGLWidget::wheelEvent(event);
		return;
	}

	// 计算缩放中心的世界坐标 
	QPointF mousePos = event->position();
	QPointF oldWorldPos(
		m_viewCenter.x() + (mousePos.x() - width() / 2.0f) / m_viewScale,
		m_viewCenter.y() - (mousePos.y() - height() / 2.0f) / m_viewScale
	);

	// 计算新缩放级别 
	float scaleFactor = pow(1.2, event->angleDelta().y() / 120.0f);
	float newScale = qBound(0.01f, m_viewScale * scaleFactor, 100.0f);

	// 更新视图中心 
	m_viewCenter.rx() = oldWorldPos.x() - (mousePos.x() - width() / 2.0f) / newScale;
	m_viewCenter.ry() = oldWorldPos.y() + (mousePos.y() - height() / 2.0f) / newScale;
	m_viewScale = newScale;
	clampViewCenter();

	// 异步更新纹理层级 
	QMetaObject::invokeMethod(this, [this]() {
		updateTextureForCurrentLevel();
	}, Qt::QueuedConnection);

	update();
	event->accept();
}

void GLDisplayWidget::clampViewCenter() {
	if (m_images.isEmpty())  return;

	const ImageData& img = m_images.first();
	float halfW = width() / (2.0f * m_viewScale);
	float halfH = height() / (2.0f * m_viewScale);

	// 修改前（错误）：
	// m_viewCenter.setX(qBound(img.minX  + halfW, m_viewCenter.x(), img.maxX  - halfW));
	// m_viewCenter.setY(qBound(img.minY  + halfH, m_viewCenter.y(), img.maxY  - halfH));

	// 修改后（正确）：
	m_viewCenter.setX(qBound(img.minX + halfW, m_viewCenter.x(), img.maxX - halfW));
	m_viewCenter.setY(qBound(img.minY + halfH, m_viewCenter.y(), img.maxY - halfH));

	// 添加边界情况处理 
	if (halfW * 2 > (img.maxX - img.minX)) {
		m_viewCenter.setX((img.minX + img.maxX) / 2.0f);
	}
	if (halfH * 2 > (img.maxY - img.minY)) {
		m_viewCenter.setY((img.minY + img.maxY) / 2.0f);
	}
}

QImage GLDisplayWidget::processMultispectralBlock(GDALDataset* dataset, int overviewLevel,
	int x, int y, int width, int height) {
	// 查找RGB波段（支持自动波段识别）
	int redBandIdx = 1, greenBandIdx = 2, blueBandIdx = 3;
	for (int i = 1; i <= dataset->GetRasterCount(); i++) {
		GDALColorInterp colorType = dataset->GetRasterBand(i)->GetColorInterpretation();
		if (colorType == GCI_RedBand) redBandIdx = i;
		else if (colorType == GCI_GreenBand) greenBandIdx = i;
		else if (colorType == GCI_BlueBand) blueBandIdx = i;
	}

	// 获取金字塔层级波段 
	GDALRasterBand* redBand = dataset->GetRasterBand(redBandIdx)->GetOverview(overviewLevel);
	GDALRasterBand* greenBand = dataset->GetRasterBand(greenBandIdx)->GetOverview(overviewLevel);
	GDALRasterBand* blueBand = dataset->GetRasterBand(blueBandIdx)->GetOverview(overviewLevel);

	// 读取16位数据 
	uint16_t* redBuf = new uint16_t[width * height];
	uint16_t* greenBuf = new uint16_t[width * height];
	uint16_t* blueBuf = new uint16_t[width * height];

	redBand->RasterIO(GF_Read, x, y, width, height,
		redBuf, width, height, GDT_UInt16, 0, 0);
	greenBand->RasterIO(GF_Read, x, y, width, height,
		greenBuf, width, height, GDT_UInt16, 0, 0);
	blueBand->RasterIO(GF_Read, x, y, width, height,
		blueBuf, width, height, GDT_UInt16, 0, 0);

	// 自适应拉伸和gamma校正 
	auto stretchValues = [](uint16_t* data, int size) {
		uint16_t minVal = 65535, maxVal = 0;
		for (int i = 0; i < size; i++) {
			if (data[i] > 0) { // 忽略0值 
				minVal = qMin(minVal, data[i]);
				maxVal = qMax(maxVal, data[i]);
			}
		}
		return qMakePair(minVal, maxVal);
	};

	auto rRange = stretchValues(redBuf, width * height);
	auto gRange = stretchValues(greenBuf, width * height);
	auto bRange = stretchValues(blueBuf, width * height);

	// 转换为QImage 
	QImage img(width, height, QImage::Format_RGB888);
	const float gamma = 0.6f;
	for (int y = 0; y < height; ++y) {
		QRgb* scanLine = reinterpret_cast<QRgb*>(img.scanLine(y));
		for (int x = 0; x < width; ++x) {
			int idx = y * width + x;

			float r = (rRange.second > rRange.first) ?
				pow((redBuf[idx] - rRange.first) / float(rRange.second - rRange.first), gamma) : 0;
			float g = (gRange.second > gRange.first) ?
				pow((greenBuf[idx] - gRange.first) / float(gRange.second - gRange.first), gamma) : 0;
			float b = (bRange.second > bRange.first) ?
				pow((blueBuf[idx] - bRange.first) / float(bRange.second - bRange.first), gamma) : 0;

			scanLine[x] = qRgb(
				static_cast<int>(qBound(0.0f, r, 1.0f) * 255),
				static_cast<int>(qBound(0.0f, g, 1.0f) * 255),
				static_cast<int>(qBound(0.0f, b, 1.0f) * 255)
			);
		}
	}

	delete[] redBuf;
	delete[] greenBuf;
	delete[] blueBuf;
	return img;
}

QImage GLDisplayWidget::processPanchromaticBlock(GDALRasterBand* band,
	int x, int y,
	int width, int height) {
	// 读取16位数据 
	uint16_t* buffer = new uint16_t[width * height];
	band->RasterIO(GF_Read, x, y, width, height,
		buffer, width, height, GDT_UInt16, 0, 0);

	// 修复问题2：改进统计值计算（排除极端值）
	uint16_t minVal = 65535, maxVal = 0;
	const float percentile = 0.02f; // 排除2%的极端值 

	// 计算直方图 
	int hist[65536] = { 0 };
	for (int i = 0; i < width * height; ++i) {
		if (buffer[i] > 0) hist[buffer[i]]++;
	}

	// 计算百分比阈值 
	int totalPixels = width * height;
	int count = 0;
	int lowThreshold = 0, highThreshold = 65535;

	for (int i = 0; i < 65536; ++i) {
		count += hist[i];
		if (count >= totalPixels * percentile) {
			lowThreshold = i;
			break;
		}
	}

	count = 0;
	for (int i = 65535; i >= 0; --i) {
		count += hist[i];
		if (count >= totalPixels * percentile) {
			highThreshold = i;
			break;
		}
	}

	// 使用改进后的范围 
	minVal = lowThreshold;
	maxVal = highThreshold;

	// 创建图像（使用gamma校正提升亮度）
	QImage img(width, height, QImage::Format_Grayscale8);
	const float gamma = 0.6f; // gamma值可调整 

	for (int y = 0; y < height; ++y) {
		uchar* scanLine = img.scanLine(y);
		for (int x = 0; x < width; ++x) {
			int idx = y * width + x;
			float normalized = (buffer[idx] - minVal) / float(maxVal - minVal);
			normalized = qBound(0.0f, normalized, 1.0f);
			scanLine[x] = static_cast<uchar>(pow(normalized, gamma) * 255);
		}
	}

	delete[] buffer;
	return img;
}

int GLDisplayWidget::calculateOverviewLevel(float viewScale) const {
	if (m_images.isEmpty()) {
		qWarning() << "No image loaded";
		return 0;
	}

	GDALDataset* dataset = (GDALDataset*)GDALOpen(m_currentImagePath.toUtf8(), GA_ReadOnly);
	if (!dataset) {
		qWarning() << "Failed to open dataset for overview calculation";
		return 0;
	}

	// 获取基础信息 
	GDALRasterBand* firstBand = dataset->GetRasterBand(1);
	int maxOverview = firstBand->GetOverviewCount();

	//// 如果没有金字塔则直接返回0级（全分辨率）
	//if (maxOverview <= 0) {
	//	GDALClose(dataset);
	//	return 0;
	//}

		// 如果没有金字塔则创建金字塔
	if (maxOverview <= 0) {
		GDALClose(dataset); // 先关闭当前数据集 

		// 调用金字塔创建函数 
		//SystemConfig* sysConfig = IMAGEPS::instance->getSystemConfig();
		SystemConfig* sysConfig = m_imagePS->getSystemConfig();
		if (sysConfig) {
			QStringList fileList;
			fileList << m_currentImagePath;

			// 创建金字塔（阻塞方式）
			sysConfig->acceptModule(QString::fromLocal8Bit("金字塔创建"), fileList);

			// 重新打开数据集检查金字塔
			dataset = (GDALDataset*)GDALOpen(m_currentImagePath.toUtf8(), GA_ReadOnly);
			if (dataset) {
				firstBand = dataset->GetRasterBand(1);
				maxOverview = firstBand->GetOverviewCount();
			}
		}

		// 如果还是没有金字塔则返回0级 
		if (maxOverview <= 0) {
			if (dataset) GDALClose(dataset);
			return 0;
		}
	}

	// 计算图像分辨率（单位：像素/米）
	ImageGeoMetadata* metadata = m_imagePS->getImageMetadata(m_currentImagePath);

	double geoTransform[6];
	bool hasGeoTransform = false;
	//dataset->GetGeoTransform(geoTransform);
	if (metadata) {
		geoTransform[0] = metadata->geoTransform[0];
		geoTransform[1] = metadata->geoTransform[1];
		geoTransform[2] = metadata->geoTransform[2];
		geoTransform[3] = metadata->geoTransform[3];
		geoTransform[4] = metadata->geoTransform[4];
		geoTransform[5] = metadata->geoTransform[5];
	}
	else if (dataset) {
		hasGeoTransform = (dataset->GetGeoTransform(geoTransform) == CE_None);
	}
	float imageResolution = hasGeoTransform ? qAbs(geoTransform[1]) : 1.0f;

	// 计算显示分辨率（单位：像素/米）
	// viewScale表示1个世界单位对应多少屏幕像素 
	// devicePixelRatio考虑高DPI屏幕 
	float displayResolution = viewScale * devicePixelRatioF();

	// 计算分辨率比率（显示分辨率 / 图像分辨率）
	// 比率>1表示需要缩小显示（使用金字塔）
	// 比率<1表示需要放大显示（使用更精细的金字塔层级）
	float ratio = displayResolution / imageResolution;

	// 根据比率选择最优金字塔层级 
	int optimalLevel = 0;
	if (maxOverview >= 4 && ratio <= 0.0625f) {  // 1:16 
		optimalLevel = 4;
	}
	else if (ratio <= 0.125f && maxOverview >= 3) {  // 1:8缩小 
		optimalLevel = 3;
	}
	else if (ratio <= 0.25f && maxOverview >= 2) {  // 1:4缩小 
		optimalLevel = 2;
	}
	else if (ratio <= 0.5f && maxOverview >= 1) {  // 1:2缩小 
		optimalLevel = 1;
	}
	else {  // 全分辨率 
		optimalLevel = 0;
	}

	// 验证选择的金字塔层级是否有效 
	optimalLevel = qBound(0, optimalLevel, maxOverview - 1);

	//// 调试信息 
	//qDebug() << "Overview selection:"
	//	<< "ImageRes=" << imageResolution
	//	<< "DisplayRes=" << displayResolution
	//	<< "Ratio=" << ratio
	//	<< "SelectedLevel=" << optimalLevel
	//	<< "MaxAvailable=" << maxOverview;

	GDALClose(dataset);
	return optimalLevel;
}

QImage GLDisplayWidget::processFloat32Block(GDALRasterBand* band,
                                          int x, int y,
                                          int width, int height) {
    // 分配32位浮点缓冲区 
    float* buffer = new float[width * height];
    
    // 读取数据 
    CPLErr err = band->RasterIO(GF_Read, x, y, width, height,
                              buffer, width, height, GDT_Float32, 0, 0);
    if (err != CE_None) {
        qWarning() << "Failed to read 32-bit float data";
        delete[] buffer;
        return QImage();
    }
 
    // 计算有效数据范围（排除NaN和无穷大）
    float minVal = std::numeric_limits<float>::max();
    float maxVal = -std::numeric_limits<float>::max();
    int validCount = 0;
    
    for (int i = 0; i < width * height; ++i) {
        if (std::isfinite(buffer[i])) {
            minVal = qMin(minVal, buffer[i]);
            maxVal = qMax(maxVal, buffer[i]);
            validCount++;
        }
    }
 
    // 处理全无效数据情况 
    if (validCount == 0 || minVal >= maxVal) {
        minVal = 0;
        maxVal = 1;
    }
 
    // 创建8位灰度图 
    QImage img(width, height, QImage::Format_Grayscale8);
    float range = maxVal - minVal;
    range = (range > 0) ? range : 1.0f;
    
    for (int row = 0; row < height; ++row) {
        uchar* scanLine = img.scanLine(row); 
        for (int col = 0; col < width; ++col) {
            int idx = row * width + col;
            if (std::isfinite(buffer[idx])) {
                float normalized = (buffer[idx] - minVal) / range;
                scanLine[col] = static_cast<uchar>(qBound(0.0f, normalized, 1.0f) * 255);
            } else {
                scanLine[col] = 0;
            }
        }
    }
 
    delete[] buffer;
    return img;
}

QImage GLDisplayWidget::processSingleBand(GDALRasterBand* band,
	int x, int y,
	int width, int height) {
	// 读取数据 
	uint8_t* buffer = new uint8_t[width * height];
	band->RasterIO(GF_Read, x, y, width, height,
		buffer, width, height, GDT_Byte, 0, 0);

	// 创建灰度图像 
	QImage img(width, height, QImage::Format_Grayscale8);

	// 直接复制数据（8位无需拉伸）
	for (int y = 0; y < height; ++y) {
		uchar* scanLine = img.scanLine(y);
		memcpy(scanLine, buffer + y * width, width);
	}

	delete[] buffer;
	return img;
}

QImage GLDisplayWidget::processSingleBand16(GDALRasterBand* band,
	int x, int y,
	int width, int height) {
	// 读取16位数据 
	uint16_t* buffer = new uint16_t[width * height];
	band->RasterIO(GF_Read, x, y, width, height,
		buffer, width, height, GDT_UInt16, 0, 0);

	// 改进的统计值计算（排除极端值）
	uint16_t minVal = 65535, maxVal = 0;
	const float excludePercent = 0.02f; // 排除2%极端值 

	// 计算直方图 
	int hist[65536] = { 0 };
	for (int i = 0; i < width * height; ++i) {
		if (buffer[i] > 0) hist[buffer[i]]++;
	}

	// 计算最小阈值 
	int total = width * height;
	int count = 0;
	for (int i = 0; i < 65536; ++i) {
		count += hist[i];
		if (count >= total * excludePercent) {
			minVal = i;
			break;
		}
	}

	// 计算最大阈值 
	count = 0;
	for (int i = 65535; i >= 0; --i) {
		count += hist[i];
		if (count >= total * excludePercent) {
			maxVal = i;
			break;
		}
	}

	// 创建图像（带gamma校正提升亮度）
	QImage img(width, height, QImage::Format_Grayscale8);
	const float gamma = 0.7f; // 可调整的gamma值 

	for (int y = 0; y < height; ++y) {
		uchar* scanLine = img.scanLine(y);
		for (int x = 0; x < width; ++x) {
			int idx = y * width + x;
			float val = (buffer[idx] - minVal) / float(maxVal - minVal);
			val = qBound(0.0f, val, 1.0f);
			scanLine[x] = static_cast<uchar>(pow(val, gamma) * 255);
		}
	}

	delete[] buffer;
	return img;
}

QImage GLDisplayWidget::process8BitMultiBand(GDALRasterBand* bands[3], const QRect& region) {
	std::unique_ptr<uint8_t[]> buffers[3];

	try {
		// 分配内存 
		for (int i = 0; i < 3; ++i) {
			buffers[i].reset(new uint8_t[region.width() * region.height()]);
		}

		// 读取数据 
		for (int i = 0; i < 3; ++i) {
			CPLErr err = bands[i]->RasterIO(
				GF_Read,
				region.x(), region.y(),
				region.width(), region.height(),
				buffers[i].get(),
				region.width(), region.height(),
				GDT_Byte, 0, 0
			);

			if (err != CE_None) {
				qWarning() << "RasterIO failed for band" << i;
				return QImage();
			}
		}

		// 创建图像 
		QImage img(region.size(), QImage::Format_RGB888);

		for (int y = 0; y < region.height(); ++y) {
			uchar* scanLine = img.scanLine(y);
			for (int x = 0; x < region.width(); ++x) {
				int idx = y * region.width() + x;
				scanLine[x * 3 + 0] = buffers[0][idx]; // R 
				scanLine[x * 3 + 1] = buffers[1][idx]; // G 
				scanLine[x * 3 + 2] = buffers[2][idx]; // B 
			}
		}

		return img;
	}
	catch (const std::bad_alloc&) {
		qCritical() << "Memory allocation failed";
		return QImage();
	}
}

QImage GLDisplayWidget::process16BitMultiBand(GDALRasterBand* bands[3], const QRect& region) {
	std::unique_ptr<uint16_t[]> buffers[3];

	try {
		// 分配内存 
		const int pixelCount = region.width()  * region.height();
		for (int i = 0; i < 3; ++i) {
			buffers[i].reset(new uint16_t[pixelCount]);
		}

		// 读取数据（使用进度提示）
		//qDebug() << "Loading 16-bit multi-band data...";
		QElapsedTimer timer;
		timer.start();

		for (int i = 0; i < 3; ++i) {
			CPLErr err = bands[i]->RasterIO(
				GF_Read,
				region.x(), region.y(),
				region.width(), region.height(),
				buffers[i].get(),
				region.width(), region.height(),
				GDT_UInt16, 0, 0
			);

			if (err != CE_None) {
				qWarning() << "RasterIO failed for band" << i;
				return QImage();
			}
		}

		//qDebug() << "Data loaded in" << timer.elapsed() << "ms";

		// 改进的统计值计算（排除极端值）
		uint16_t minVals[3] = { 65535, 65535, 65535 };
		uint16_t maxVals[3] = { 0, 0, 0 };
		const float excludePercent = 0.01f; // 更严格的排除比例 

		// 并行计算统计值（使用OpenMP）
#pragma omp parallel for 
		for (int i = 0; i < 3; ++i) {
			// 计算直方图 
			int hist[65536] = { 0 };
			for (int j = 0; j < pixelCount; ++j) {
				if (buffers[i][j] > 0) hist[buffers[i][j]]++;
			}

			// 计算有效范围（改进算法）
			int totalValid = 0;
			for (int j = 0; j < 65536; ++j) {
				totalValid += hist[j];
			}

			// 计算最小阈值（排除暗部噪声）
			int count = 0;
			int lowThreshold = 0;
			for (int j = 0; j < 65536; ++j) {
				count += hist[j];
				if (count >= totalValid * excludePercent) {
					lowThreshold = j;
					break;
				}
			}

			// 计算最大阈值（排除高光溢出）
			count = 0;
			int highThreshold = 65535;
			for (int j = 65535; j >= 0; --j) {
				count += hist[j];
				if (count >= totalValid * excludePercent) {
					highThreshold = j;
					break;
				}
			}

			minVals[i] = lowThreshold;
			maxVals[i] = highThreshold;

			//qDebug() << "Band" << i << "range:" << minVals[i] << "-" << maxVals[i];
		}

		// 创建图像（改进的色调映射）
		QImage img(region.size(), QImage::Format_RGB888);
		const float gamma = 0.8f; // 调整gamma值 
		const float contrast = 1.2f; // 对比度增强 

		for (int y = 0; y < region.height(); ++y) {
			uchar* scanLine = img.scanLine(y);
			for (int x = 0; x < region.width(); ++x) {
				int idx = y * region.width() + x;

				for (int i = 0; i < 3; ++i) {
					// 改进的色调映射算法 
					float normalized = (buffers[i][idx] - minVals[i]) /
						float(maxVals[i] - minVals[i]);
					normalized = qBound(0.0f, normalized, 1.0f);

					// 对比度增强 
					normalized = (normalized - 0.5f) * contrast + 0.5f;
					normalized = qBound(0.0f, normalized, 1.0f);

					// Gamma校正 
					scanLine[x * 3 + i] = static_cast<uchar>(pow(normalized, gamma) * 255);
				}
			}
		}

		// 锐化处理（可选）
		if (region.width() > 512) { // 对大图像进行锐化 
			QImage sharpened = img;
			QPainter painter(&sharpened);
			painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
			painter.drawImage(0, 0, img);
			painter.end();
			return sharpened;
		}

		return img;
	}
	catch (const std::bad_alloc&) {
		qCritical() << "Memory allocation failed";
		return QImage();
	}
}

QImage GLDisplayWidget::processMultiBand(GDALDataset* dataset,
	int overviewLevel,
	const QRect& region) {
	// 输入验证 
	if (!dataset) {
		qCritical() << "Null dataset pointer";
		return QImage();
	}

	try {
		// 获取波段信息 
		const int bandCount = dataset->GetRasterCount();
		if (bandCount < 3) {
			qWarning() << "Insufficient bands:" << bandCount;
			return QImage();
		}

		// 自动识别RGB波段（带保护）
		int bands[3] = { 1, 2, 3 }; // 默认RGB顺序 
		bool hasColorInfo = false;

		for (int i = 1; i <= qMin(bandCount, 10); i++) { // 限制检查前10个波段 
			GDALRasterBand* band = dataset->GetRasterBand(i);
			if (!band) continue;

			GDALColorInterp colorType = band->GetColorInterpretation();
			switch (colorType) {
			case GCI_RedBand:    bands[0] = i; hasColorInfo = true; break;
			case GCI_GreenBand:  bands[1] = i; hasColorInfo = true; break;
			case GCI_BlueBand:   bands[2] = i; hasColorInfo = true; break;
			default: break;
			}
		}

		// 获取金字塔层级（带范围检查）
		GDALRasterBand* redBand = dataset->GetRasterBand(bands[0]);
		if (!redBand) {
			qCritical() << "Invalid red band";
			return QImage();
		}

		const int overviewCount = redBand->GetOverviewCount();
		overviewLevel = qBound(0, overviewLevel, qMax(0, overviewCount - 1));

		// 获取金字塔波段（带空指针检查）
		GDALRasterBand* ovrBands[3] = {
			redBand->GetOverview(overviewLevel),
			dataset->GetRasterBand(bands[1])->GetOverview(overviewLevel),
			dataset->GetRasterBand(bands[2])->GetOverview(overviewLevel)
		};

		for (int i = 0; i < 3; ++i) {
			if (!ovrBands[i]) {
				qCritical() << "Null overview band:" << i;
				return QImage();
			}
		}

		// 确定处理区域 
		const int width = ovrBands[0]->GetXSize();
		const int height = ovrBands[0]->GetYSize();

		QRect safeRegion = region.isEmpty() ? QRect(0, 0, width, height) : region;
		safeRegion = safeRegion.intersected(QRect(0, 0, width, height));

		if (safeRegion.isEmpty()) {
			qWarning() << "Empty region after intersection";
			return QImage();
		}

		// 根据数据类型分派处理 
		const GDALDataType dataType = ovrBands[0]->GetRasterDataType();

		if (dataType == GDT_Byte) {
			return process8BitMultiBand(ovrBands, safeRegion);
		}
		else if (dataType == GDT_UInt16) {
			return process16BitMultiBand(ovrBands, safeRegion);
		}
		else {
			qWarning() << "Unsupported data type:" << dataType;
			return QImage();
		}
	}
	catch (const std::exception& e) {
		qCritical() << "Exception in processMultiBand:" << e.what();
		return QImage();
	}
}

QPointF GLDisplayWidget::screenToWorld(const QPoint& screenPos) const
{
	if (m_images.isEmpty())
		return QPointF();

	const ImageData& img = m_images.first();

	// 计算视口范围 
	float halfW = width() / (2.0f * m_viewScale);
	float halfH = height() / (2.0f * m_viewScale);

	// 转换为世界坐标 
	double worldX = m_viewCenter.x() + (screenPos.x() - width() / 2.0) / m_viewScale;
	double worldY = m_viewCenter.y() - (screenPos.y() - height() / 2.0) / m_viewScale;

	return QPointF(worldX, worldY);
}