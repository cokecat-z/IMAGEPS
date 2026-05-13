#include "MeasurementareaShow.h"   
#include "IMAGEPS.h"  
#include <QDebug>   
#include <cmath>

//MeasurementareaShow* MeasurementareaShow::Measurementareainstance = nullptr;

MeasurementareaShow::MeasurementareaShow(IMAGEPS* parentImagePS, QWidget* parent)
	: QOpenGLWidget(parent)
	, m_imagePS(parentImagePS),
	m_worldMinX(0), m_worldMaxX(0), m_worldMinY(0), m_worldMaxY(0),
	m_viewCenterX(0), m_viewCenterY(0), m_viewScale(1.0),
	m_isPanning(false)
{
	if (!m_imagePS && parent) {
		m_imagePS = qobject_cast<IMAGEPS*>(parent);
	}
	//Measurementareainstance = this;

	// 初始化GDAL
	GDALAllRegister();

	// 检查PROJ是否可用 
	PJ_CONTEXT* ctx = proj_context_create();
	if (!ctx) {
		qWarning() << "Failed to create PROJ context";
	}
	else {
		qDebug() << "PROJ initialized successfully";
		proj_context_destroy(ctx);
	}

	m_currentImageTypeFilter = QString::fromLocal8Bit("所有类型");
}

MeasurementareaShow::~MeasurementareaShow()
{
	makeCurrent();
	for (GeoImage &image : m_images) {
		if (image.texture) {
			delete image.texture;  
			image.texture = nullptr;
		}
	}
	doneCurrent();

	// 释放地理变换参数内存 
	for (auto& transform : m_geoTransforms) {
		delete[] transform;
	}
	m_geoTransforms.clear();
}

void MeasurementareaShow::initializeGL()
{
	initializeOpenGLFunctions();
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	//if (this->objectName() == "glWidget2") {
	//	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//}
	//else {
	//	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//}
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// [新增] 深度测试 
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}

void MeasurementareaShow::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	//updateProjection();
}

void MeasurementareaShow::paintGL()
{
	// 设置正确的视图矩阵 
	//updateViewTransform();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. 再绘制边界框和连接点（屏幕坐标系）
	for (const GeoBoundary &boundary : m_boundaries) {
		drawBoundary(boundary);
	}

	if (m_showPoints) {
		drawPoints();
	}

	// 绘制选择框 
	if (m_isSelecting || !m_selectionRect.isNull()) {
		drawSelectionRect();
	}
}

// 判断影像是否为加带号的投影坐标系
bool MeasurementareaShow::isProjectedWithZone(const QString& imagePath, int* zone = nullptr) {
	// 注册GDAL驱动 
	GDALAllRegister();

	// 打开影像文件 
	GDALDataset* dataset = (GDALDataset*)GDALOpen(imagePath.toUtf8().constData(), GA_ReadOnly);
	if (!dataset) {
		//QMessageBox::critical(nullptr, u8"错误", u8"无法打开影像文件！");
		return false;
	}

	// 获取投影信息 
	const char* proj = dataset->GetProjectionRef();
	if (strlen(proj) == 0) {
		GDALClose(dataset);
		//QMessageBox::warning(nullptr, u8"警告", u8"该影像没有投影信息！");
		return false;
	}

	// 创建空间参考对象
	OGRSpatialReference srs;
	if (srs.importFromWkt(proj) != OGRERR_NONE) {
		GDALClose(dataset);
		//QMessageBox::critical(nullptr, u8"错误", u8"无法解析投影信息！");
		return false;
	}

	// 如果不是投影坐标系，直接返回false 
	if (!srs.IsProjected()) {
		GDALClose(dataset);
		return false;
	}

	// 检查常见带号投影类型
	const char* projName = srs.GetAttrValue("PROJCS");
	int detectedZone = 0;
	bool isZonedProjection = false;

	// 1. 检查UTM投影 
	if (projName && strstr(projName, "UTM")) {
		detectedZone = srs.GetUTMZone();
		if (detectedZone != 0) {
			isZonedProjection = true;
		}
	}
	// 2. 检查高斯克吕格投影
	else if (projName && (strstr(projName, "Gauss") || strstr(projName, "GK"))) {
		double centralMeridian = srs.GetProjParm(SRS_PP_CENTRAL_MERIDIAN, 0.0);
		if (centralMeridian != 0.0) {
			// 6度带计算
			detectedZone = static_cast<int>((centralMeridian + 3) / 6);
			isZonedProjection = true;

			// 或者3度带计算
			// detectedZone = static_cast<int>(centralMeridian / 3);
		}
	}
	// 3. 检查其他可能带号投影
	else {
		// 检查是否存在带号参数
		const char* zoneParam = srs.GetAttrValue("PROJCS|PARAMETER[\"zone\"");
		if (zoneParam) {
			detectedZone = atoi(zoneParam);
			isZonedProjection = (detectedZone != 0);
		}
	}

	// 如果传入了zone指针，返回带号 
	if (zone) {
		*zone = detectedZone;
	}

	GDALClose(dataset);
	return isZonedProjection;
}

bool MeasurementareaShow::calculateCornerCoordinates(const QString filePath, GeoBoundary& boundary)
{
	//if (currentDataset) {
	//	GDALClose(currentDataset);
	//	currentDataset = nullptr;
	//}

	currentDataset = (GDALDataset*)GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly);
	if (!currentDataset) return false;

	//// 清空原有数据 
	boundary.corners.clear();
	//boundary.filePath = filePath;

	// 获取地理变换参数 
	double geoTransform[6];
	if (currentDataset->GetGeoTransform(geoTransform) != CE_None) {
		//QMessageBox::warning(this, "警告", "该影像文件不包含地理参考信息！");
		return false;
	}

	// 获取投影信息 
	const char* projectionRef = currentDataset->GetProjectionRef();
	if (strlen(projectionRef) == 0) {
		//QMessageBox::warning(this, "警告", "该影像文件不包含投影信息！\n");
		return false;
	}

	int imageWidth = currentDataset->GetRasterXSize();
	int imageHeight = currentDataset->GetRasterYSize();

	// 创建坐标转换 
	OGRSpatialReference sourceSRS, targetSRS;
	if (sourceSRS.importFromWkt(projectionRef) != OGRERR_NONE) {
		//QMessageBox::critical(this, "错误", "无法解析投影信息！\n");
		return false;
	}

	targetSRS.SetWellKnownGeogCS("WGS84");  // 目标坐标系为WGS84 

	OGRCoordinateTransformation *coordTransform =
		OGRCreateCoordinateTransformation(&sourceSRS, &targetSRS);

	if (!coordTransform) {
		//QMessageBox::critical(this, "错误", "创建坐标转换失败！\n");
		return false;
	}

	// 定义四个角点（按parseXmlBoundary的顺序：左上、右上、右下、左下）
	struct ImageCorner {
		QString name;
		double pixelX, pixelY;
	};

	ImageCorner corners[4] = {
		{"左上角(UL)", 0, 0},
		{"右上角(UR)", static_cast<double>(imageWidth), 0},
		{"右下角(LR)", static_cast<double>(imageWidth), static_cast<double>(imageHeight)},
		{"左下角(LL)", 0, static_cast<double>(imageHeight)}
	};

	for (const auto& corner : corners) {
		// 计算投影坐标 
		double projX = geoTransform[0] + corner.pixelX  * geoTransform[1] + corner.pixelY  * geoTransform[2];
		double projY = geoTransform[3] + corner.pixelX  * geoTransform[4] + corner.pixelY  * geoTransform[5];

		// 转换为地理坐标 
		double lon = projX;
		double lat = projY;

		if (coordTransform->Transform(1, &lon, &lat)) {
			boundary.corners.append(QPointF(lat, lon));
		}
		else {
			OCTDestroyCoordinateTransformation(coordTransform);
			return false;
		}
	}

	// 清理资源 
	OCTDestroyCoordinateTransformation(coordTransform);
	return true;
}

bool MeasurementareaShow::parseXmlBoundary(const QString &xmlPath, GeoBoundary &boundary) {
	QFile file(xmlPath);
	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "无法打开XML文件:" << xmlPath;
		return false;
	}

	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		qWarning() << "XML解析失败:" << xmlPath;
		return false;
	}
	file.close();

	QDomElement root = doc.documentElement();
	boundary.corners.clear();
	// boundary.isGeoReferenced  = true;  // 原结构体中的标记，新结构体无此字段 

	// 按顺时针顺序获取四个角点
	boundary.corners.append(QPointF(
		root.firstChildElement("UpperLeftLong").text().toDouble(),
		root.firstChildElement("UpperLeftLat").text().toDouble()
	));
	boundary.corners.append(QPointF(
		root.firstChildElement("UpperRightLong").text().toDouble(),
		root.firstChildElement("UpperRightLat").text().toDouble()
	));
	boundary.corners.append(QPointF(
		root.firstChildElement("LowerRightLong").text().toDouble(),
		root.firstChildElement("LowerRightLat").text().toDouble()
	));
	boundary.corners.append(QPointF(
		root.firstChildElement("LowerLeftLong").text().toDouble(),
		root.firstChildElement("LowerLeftLat").text().toDouble()
	));

	return true;
}

bool MeasurementareaShow::containsBoundary(const QVector<GeoBoundary>& boundaries, const QString& filePath) {
	for (const auto& b : boundaries) {
		if (b.filePath == filePath) {
			return true; // 已存在相同项，无需添加
		}
	}
	return false;
}

void MeasurementareaShow::addGeoBoundary(const QString &filePath, const QColor &color) {

	GeoBoundary boundary;
	boundary.color = color;
	boundary.filePath = filePath;

	QFileInfo fileInfo(filePath);
	QString xmlPath = m_imagePS->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") +
		fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml");

	if (isProjectedWithZone(filePath))
	{
		calculateCornerCoordinates(filePath, boundary);
	}
	else {
		if (!parseXmlBoundary(xmlPath, boundary)) {
			qWarning() << "Failed to parse XML boundary for file:" << filePath;
			return;
		}
	}

	if (boundary.corners.size() != 4) {
		qWarning() << "Invalid boundary data for file:" << filePath;
		return;
	}

	// 判断当前文件是否为原始数据（DataModelPath中的文件）
	bool isOriginalData = m_imagePS->getDataModelPath().contains(filePath);

	// 判断当前文件是否为参考数据（DOMFilePath或DEMFilePath中的文件）
	bool isReferenceData = m_imagePS->getDOMFilePath().contains(filePath) ||
		m_imagePS->getDEMFilePath().contains(filePath);

	// 原始数据直接加载，不做过滤
	if (isOriginalData) {
		// 检查是否已存在相同 filePath 的边界 
		if (!containsBoundary(m_boundaries, filePath)) {
			m_boundaries.append(boundary);
		}
		calculateWorldExtent();
		zoomToFullExtent();
		update();
		return;
	}

	// 如果不是参考数据，也直接加载（保持原有兼容性）
	if (!isReferenceData) {
		if (!containsBoundary(m_boundaries, filePath)) {
			m_boundaries.append(boundary);
		}
		calculateWorldExtent();
		zoomToFullExtent();
		update();
		return;
	}

	// 以下是只对参考数据的处理逻辑 
	// 获取所有原始影像的边界框 
	QVector<QPolygonF> originalPolygons;
	for (const GeoBoundary& existing : m_boundaries) {
		if (m_imagePS->getDataModelPath().contains(existing.filePath)) {
			QPolygonF poly;
			for (const QPointF& point : existing.corners) {
				poly << point;
			}
			originalPolygons.append(poly);
		}
	}

	// 如果没有原始影像边界框，默认绘制所有参考数据
	bool shouldDraw = originalPolygons.isEmpty();
	
	// 检查参考数据边界框是否与任何原始影像边界框相交 
	if (!shouldDraw) {
		// 创建参考数据的多边形
		QPolygonF refPoly;
		for (const QPointF& point : boundary.corners) {
			refPoly << point;
		}

		// 检查与每个原始影像边界框的相交情况
		for (const QPolygonF& originalPoly : originalPolygons) {
			if (originalPoly.intersects(refPoly)) {
				shouldDraw = true;
				break;
			}
		}
	}

	if (shouldDraw) {
		if (!containsBoundary(m_boundaries, filePath)) {
			m_boundaries.append(boundary);
		}
		// 如果之前被排除过，现在又符合条件了，就从排除列表中移除
		if (m_excludedReferences.contains(filePath)) {
			m_excludedReferences.remove(filePath);
		}
		calculateWorldExtent();
		zoomToFullExtent();
		update();
	}
	else {
		qDebug() << "Reference boundary does not intersect with any original image, skipping:" << filePath;
		// 添加到排除列表
		if (!m_excludedReferences.contains(filePath)) {
			m_excludedReferences.insert(filePath);
		}
	}
}

// 获取文件列表
QStringList MeasurementareaShow::getExcludedReferences() const {
	return m_excludedReferences.toList();
}

//void MeasurementareaShow::addGeoBoundary(const QString &filePath, const QColor &color) {
//
//	GeoBoundary boundary;
//	boundary.color = color;
//	boundary.filePath = filePath;  // 存储完整文件路径
//	QFileInfo fileInfo(filePath);
//	if (parseXmlBoundary(m_imagePS->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml"), boundary)) {
//		m_boundaries.append(boundary);   // 直接追加，不清除已有边界 
//		update();
//	}
//
//	if (boundary.corners.size() != 4) {
//		qWarning() << "Failed to parse geographic coordinates for file:" << filePath;
//		return;
//	}
//
//	//m_boundaries.append(boundary);
//	calculateWorldExtent();
//	zoomToFullExtent();
//}

//void MeasurementareaShow::addGeoBoundary(const QString &filePath, const QColor &color) {
//	// 获取旋转后的四个角点坐标 
//	std::vector<QString> corners = getRotatedImageCorners(filePath);
//	if (corners.size() != 4 || corners[0] == "无地理参考信息") {
//		qWarning() << "Failed to get valid geographic coordinates for file:" << filePath;
//		return;
//	}
//
//	// 解析四个角点的坐标 
//	GeoBoundary boundary;
//	boundary.color = color;
//	boundary.filePath = filePath;
//
//	for (const QString& corner : corners) {
//		QStringList parts = corner.split(", ");
//		if (parts.size() == 2) {
//			boundary.corners.append(QPointF(parts[0].toDouble(), parts[1].toDouble()));
//		}
//	}
//	if (boundary.corners.size() != 4) {
//		qWarning() << "Failed to parse geographic coordinates for file:" << filePath;
//		return;
//	}
//
//	// 创建目标坐标系 
//	OGRSpatialReference* targetSRS = createTargetSRS();
//
//	// 执行坐标系转换 
//	reprojectBoundary(boundary, targetSRS);
//
//	delete targetSRS;
//
//	m_boundaries.append(boundary);
//	calculateWorldExtent();
//	zoomToFullExtent();
//}

void MeasurementareaShow::clearBoundaries()
{
	m_boundaries.clear();
	update();
}

void MeasurementareaShow::calculateWorldExtent() {
	if (m_boundaries.isEmpty() && m_images.isEmpty()) {
		qWarning() << "No boundaries or images available";
		m_worldMinX = m_worldMaxX = m_worldMinY = m_worldMaxY = 0.0;
		return;
	}

	// 初始化范围 
	bool initialized = false;

	// 首先尝试从边界框初始化 
	if (!m_boundaries.isEmpty()) {
		const auto& firstBoundary = m_boundaries.first();
		if (!firstBoundary.corners.isEmpty()) {
			m_worldMinX = m_worldMaxX = firstBoundary.corners.first().x();
			m_worldMinY = m_worldMaxY = firstBoundary.corners.first().y();
			initialized = true;
		}
	}

	if (!initialized) {
		qWarning() << "Failed to initialize world extent";
		m_worldMinX = m_worldMaxX = m_worldMinY = m_worldMaxY = 0.0;
		return;
	}

	// 更新所有边界框的范围 
	for (const auto& boundary : m_boundaries) {
		for (const auto& corner : boundary.corners) {
			m_worldMinX = qMin(m_worldMinX, corner.x());
			m_worldMaxX = qMax(m_worldMaxX, corner.x());
			m_worldMinY = qMin(m_worldMinY, corner.y());
			m_worldMaxY = qMax(m_worldMaxY, corner.y());
		}
	}

}

//void MeasurementareaShow::calculateWorldExtent()
//{
//	if (!Measurementareainstance) {
//		qWarning() << "Measurementareainstance is null";
//		m_worldMinX = m_worldMaxX = m_worldMinY = m_worldMaxY = 0.0;
//		return;
//	}
//
//	if (m_boundaries.isEmpty()) {
//		qWarning() << "No boundaries available";
//		m_worldMinX = m_worldMaxX = m_worldMinY = m_worldMaxY = 0.0;
//		return;
//	}
//
//	const auto& firstBoundary = m_boundaries.first();
//	if (firstBoundary.corners.isEmpty()) {
//		qWarning() << "First boundary has no corners";
//		m_worldMinX = m_worldMaxX = m_worldMinY = m_worldMaxY = 0.0;
//		return;
//	}
//
//	try {
//		m_worldMinX = m_worldMaxX = firstBoundary.corners.first().x();
//		m_worldMinY = m_worldMaxY = firstBoundary.corners.first().y();
//	}
//	catch (...) {
//		qCritical() << "Failed to access corner coordinates";
//		return;
//	}
//
//	for (const auto& boundary : m_boundaries) {
//		for (const auto& corner : boundary.corners) {
//			m_worldMinX = qMin(m_worldMinX, corner.x());
//			m_worldMaxX = qMax(m_worldMaxX, corner.x());
//			m_worldMinY = qMin(m_worldMinY, corner.y());
//			m_worldMaxY = qMax(m_worldMaxY, corner.y());
//		}
//	}
//}

void MeasurementareaShow::drawBoundary(const GeoBoundary &boundary) {
	if (boundary.corners.size() != 4) return;

	// 保存当前OpenGL状态 
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// 判断是否是高亮边界
	bool isHighlighted = m_highlightedFiles.contains(boundary.filePath);

	// 设置线条属性 
	if (isHighlighted) {
		glLineWidth(m_highlightWidth);
		glColor3f(m_highlightColor.redF(), m_highlightColor.greenF(), m_highlightColor.blueF());
	}
	else {
		glLineWidth(0.5f); // 普通线条宽度
		glColor3f(boundary.color.redF(), boundary.color.greenF(), boundary.color.blueF());
	}

	// 临时切换到屏幕坐标系
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width(), height(), 0, -1, 1);

	// 转换所有角点到屏幕坐标 
	QVector<QPoint> screenCorners;
	for (const QPointF& corner : boundary.corners) {
		screenCorners.append(worldToScreen(corner));
	}

	// 绘制边界框
	glBegin(GL_LINE_LOOP);
	for (const QPoint& corner : screenCorners) {
		glVertex2i(corner.x(), corner.y());
	}
	glEnd();

	// 如果是高亮状态，再绘制一次带透明度的粗线
	if (isHighlighted) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f(m_highlightColor.redF(), m_highlightColor.greenF(),
			m_highlightColor.blueF(), 0.1f); // 半透明 
		glLineWidth(m_highlightWidth * 2);
		glBegin(GL_LINE_LOOP);
		for (const QPoint& corner : screenCorners) {
			glVertex2i(corner.x(), corner.y());
		}
		glEnd();
		glDisable(GL_BLEND);
	}

	// 恢复世界坐标系 
	glPopMatrix();
	glPopAttrib();
}

// 交互控制函数实现   
void MeasurementareaShow::zoomIn()
{
	m_viewScale *= 1.2; // 放大20%
	update();
}

void MeasurementareaShow::zoomOut()
{
	m_viewScale /= 1.2; // 缩小20%
	update();
}

void MeasurementareaShow::zoomToFullExtent()
{
	if (m_boundaries.isEmpty() && m_conPoints.isEmpty() && m_colPoints.isEmpty()) {
		m_viewCenterX = 0;
		m_viewCenterY = 0;
		m_viewScale = 1.0;
		return;
	}

	// 初始化范围 
	bool hasValidRange = false;
	double minX = 0, maxX = 0, minY = 0, maxY = 0;

	// 首先从边界框获取范围 
	if (!m_boundaries.isEmpty()) {
		const auto& firstBoundary = m_boundaries.first();
		if (!firstBoundary.corners.isEmpty()) {
			minX = maxX = firstBoundary.corners.first().x();
			minY = maxY = firstBoundary.corners.first().y();
			hasValidRange = true;
		}
	}

	// 如果没有边界框，尝试从连接点获取范围 
	if (!hasValidRange && !m_conPoints.isEmpty()) {
		minX = maxX = m_conPoints.first().position.x();
		minY = maxY = m_conPoints.first().position.y();
		hasValidRange = true;
	}

	// 如果还是没有，尝试从控制点获取范围 
	if (!hasValidRange && !m_colPoints.isEmpty()) {
		minX = maxX = m_colPoints.first().position.x();
		minY = maxY = m_colPoints.first().position.y();
		hasValidRange = true;
	}

	if (!hasValidRange) {
		qWarning() << "No valid boundaries or points available";
		return;
	}

	// 更新所有边界框的范围 
	for (const auto& boundary : m_boundaries) {
		for (const auto& corner : boundary.corners) {
			minX = qMin(minX, corner.x());
			maxX = qMax(maxX, corner.x());
			minY = qMin(minY, corner.y());
			maxY = qMax(maxY, corner.y());
		}
	}

	// 更新所有连接点的范围 
	for (const auto& point : m_conPoints) {
		minX = qMin(minX, point.position.x());
		maxX = qMax(maxX, point.position.x());
		minY = qMin(minY, point.position.y());
		maxY = qMax(maxY, point.position.y());
	}

	// 更新所有控制点的范围 
	for (const auto& point : m_colPoints) {
		minX = qMin(minX, point.position.x());
		maxX = qMax(maxX, point.position.x());
		minY = qMin(minY, point.position.y());
		maxY = qMax(maxY, point.position.y());
	}

	// 计算中心点 
	m_viewCenterX = (minX + maxX) / 2.0;
	m_viewCenterY = (minY + maxY) / 2.0;

	// 计算合适的缩放比例，使整个范围可见
	double worldWidth = maxX - minX;
	double worldHeight = maxY - minY;
	double widgetAspect = static_cast<double>(width()) / height();
	double worldAspect = worldWidth / worldHeight;

	// 添加10%的边距 
	double marginScale = 1.1;

	if (widgetAspect > worldAspect) {
		// 以高度为基准
		m_viewScale = height() / (worldHeight * marginScale);
	}
	else {
		// 以宽度为基准 
		m_viewScale = width() / (worldWidth * marginScale);
	}

	update();
}

void MeasurementareaShow::pan(double dx, double dy)
{
	// 将屏幕坐标的平移量转换为世界坐标
	double worldDx = dx / m_viewScale;
	double worldDy = dy / m_viewScale;

	m_viewCenterX -= worldDx;
	m_viewCenterY += worldDy; // Y轴方向相反

	update();
}

// 修改鼠标事件处理 
void MeasurementareaShow::mousePressEvent(QMouseEvent *event) {
	if (m_useMiddleButtonForPan && event->button() == Qt::LeftButton) {
		// 选择模式下，左键开始选择 
		if (m_isPointSelecting) {
			// 点选择模式
			m_isSelecting = true;
			m_selectionStart = event->pos();
			m_selectionRect = QRect(m_selectionStart, QSize());
		}
		else {
			// 原有影像选择模式
			m_isSelecting = true;
			m_selectionStart = event->pos();
			m_selectionRect = QRect(m_selectionStart, QSize());
		}
	}
	else if ((!m_useMiddleButtonForPan && event->button() == Qt::LeftButton) ||
		(m_useMiddleButtonForPan && event->button() == Qt::MiddleButton)) {
		// 普通模式左键平移，或选择模式中键平移 
		m_isPanning = true;
		m_lastPanPos = event->pos();
	}
	QOpenGLWidget::mousePressEvent(event);
}

void MeasurementareaShow::mouseMoveEvent(QMouseEvent *event) {
	if (m_isSelecting) {
		// 更新选择框 
		m_selectionRect = QRect(m_selectionStart, event->pos()).normalized();
		update();
	}
	else if (m_isPanning) {
		QPoint delta = event->pos() - m_lastPanPos;
		pan(delta.x(), delta.y());
		m_lastPanPos = event->pos();
	}
	QOpenGLWidget::mouseMoveEvent(event);
}

void MeasurementareaShow::mouseReleaseEvent(QMouseEvent *event) {
	if (m_isSelecting && event->button() == Qt::LeftButton) {
		m_isSelecting = false;

		if (m_isPointSelecting) {
			clearPointHighlights();
			// 处理点选择 
			QPointF worldStart = screenToWorld(m_selectionRect.topLeft());
			QPointF worldEnd = screenToWorld(m_selectionRect.bottomRight());

			// 计算世界坐标范围 
			double minX = qMin(worldStart.x(), worldEnd.x());
			double maxX = qMax(worldStart.x(), worldEnd.x());
			double minY = qMin(worldStart.y(), worldEnd.y());
			double maxY = qMax(worldStart.y(), worldEnd.y());

			// 清空之前的选择
			if (!event->modifiers().testFlag(Qt::ControlModifier)) {
				m_selectedConPoints.clear();
				m_selectedColPoints.clear();
			}

			// 检查哪些点在选择范围内 
			if (m_currentPointTypeFilter == "con" || m_currentPointTypeFilter == "all") {
				for (const Point& point : m_conPoints) {
					if (point.position.x() >= minX && point.position.x() <= maxX &&
						point.position.y() >= minY && point.position.y() <= maxY) {
						m_selectedConPoints.insert(point.id);
					}
				}
			}

			if (m_currentPointTypeFilter == "col" || m_currentPointTypeFilter == "all") {
				for (const Point& point : m_colPoints) {
					if (point.position.x() >= minX && point.position.x() <= maxX &&
						point.position.y() >= minY && point.position.y() <= maxY) {
						m_selectedColPoints.insert(point.id);
					}
				}
			}

			// 高亮选中的点
			highlightPointsById(m_selectedColPoints, "col", true);
			highlightPointsById(m_selectedConPoints, "con", true);

			// 更新表格选中状态 
			emit pointsSelected(m_selectedConPoints, m_selectedColPoints);
		}
		else {
			// 转换选择框到世界坐标 
			QPointF worldStart = screenToWorld(m_selectionRect.topLeft());
			QPointF worldEnd = screenToWorld(m_selectionRect.bottomRight());

			// 计算选择矩形的世界坐标边界 
			double minX = qMin(worldStart.x(), worldEnd.x());
			double maxX = qMax(worldStart.x(), worldEnd.x());
			double minY = qMin(worldStart.y(), worldEnd.y());
			double maxY = qMax(worldStart.y(), worldEnd.y());

			// 创建选择区域的多边形（顺时针或逆时针均可）
			QPolygonF selectionPoly;
			selectionPoly << QPointF(minX, minY)
				<< QPointF(maxX, minY)
				<< QPointF(maxX, maxY)
				<< QPointF(minX, maxY);

			//// 清空之前的选择 
			//m_selectedFiles.clear();
			// 是否按下了 Ctrl 键 
			bool isCtrlPressed = event->modifiers().testFlag(Qt::ControlModifier);

			// 如果没有按下 Ctrl，则清除之前的高亮
			if (!isCtrlPressed) {
				m_selectedFiles.clear();
				//clearHighlights(); // 这会清空 m_highlightedFiles 并触发重绘
			}

			QStringList m_currentImageFilePath = m_imagePS->getSelectFilePath(m_currentImageTypeFilter);

			// 遍历所有边界框 
			for (const GeoBoundary &boundary : m_boundaries) {
				// 过滤文件类型 
				if (!m_currentImageTypeFilter.isEmpty() && !m_currentImageFilePath.contains(boundary.filePath)) {
					continue;
				}

				// 构造边界框的多边形 
				QPolygonF boundaryPoly;
				for (const QPointF &corner : boundary.corners) {
					boundaryPoly.append(corner);
				}
				// 确保闭合（可选，QPolygonF 自动处理）

				// 判断选择矩形是否与边界框的边线或角点相交 
				bool intersects = false;

				// 检查边线相交 
				for (int i = 0; i < boundaryPoly.size(); ++i) {
					QLineF edge(boundaryPoly[i], boundaryPoly[(i + 1) % boundaryPoly.size()]);
					QPointF intersectPoint;
					QLineF::IntersectType intersectType = edge.intersects(QLineF(selectionPoly[0], selectionPoly[1]), &intersectPoint);
					if (intersectType == QLineF::BoundedIntersection) {
						intersects = true;
						break;
					}
					intersectType = edge.intersects(QLineF(selectionPoly[1], selectionPoly[2]), &intersectPoint);
					if (intersectType == QLineF::BoundedIntersection) {
						intersects = true;
						break;
					}
					intersectType = edge.intersects(QLineF(selectionPoly[2], selectionPoly[3]), &intersectPoint);
					if (intersectType == QLineF::BoundedIntersection) {
						intersects = true;
						break;
					}
					intersectType = edge.intersects(QLineF(selectionPoly[3], selectionPoly[0]), &intersectPoint);
					if (intersectType == QLineF::BoundedIntersection) {
						intersects = true;
						break;
					}
				}

				// 检查角点相交 
				if (!intersects) {
					for (const QPointF &corner : boundary.corners) {
						if (selectionPoly.containsPoint(corner, Qt::OddEvenFill)) {
							intersects = true;
							break;
						}
					}
				}

				if (intersects) {
					//m_selectedFiles.insert(boundary.filePath);
					bool wasHighlighted = m_highlightedFiles.contains(boundary.filePath);
					
					if (wasHighlighted) {
						// 已高亮 → 取消高亮
						//m_highlightedFiles.remove(boundary.filePath);
						m_selectedFiles.remove(boundary.filePath);
						//highlightBoundary(boundary.filePath, false); // 触发视觉更新 
					}
					else {
						// 未高亮 → 设置高亮
						//m_highlightedFiles.insert(boundary.filePath);
						m_selectedFiles.insert(boundary.filePath);
						//highlightBoundary(boundary.filePath, true); // 触发视觉更新 
					}
				}
			}

			// 更新高亮状态 
			m_highlightedFiles = m_selectedFiles;
			update();

			// 发送信号 
			emit filesSelected(m_selectedFiles.toList());
		}

		update();
	}
	else if (m_isPanning && ((!m_useMiddleButtonForPan && event->button() == Qt::LeftButton) ||
		(m_useMiddleButtonForPan && event->button() == Qt::MiddleButton))) {
		m_isPanning = false;
	}
	QOpenGLWidget::mouseReleaseEvent(event);
}

//void MeasurementareaShow::mouseReleaseEvent(QMouseEvent *event) {
//	if (m_isSelecting && event->button() == Qt::LeftButton) {
//		m_isSelecting = false;
//
//		// 转换选择框到世界坐标 
//		QPointF worldStart = screenToWorld(m_selectionRect.topLeft());
//		QPointF worldEnd = screenToWorld(m_selectionRect.bottomRight());
//
//		// 计算世界坐标范围 
//		double minX = qMin(worldStart.x(), worldEnd.x());
//		double maxX = qMax(worldStart.x(), worldEnd.x());
//		double minY = qMin(worldStart.y(), worldEnd.y());
//		double maxY = qMax(worldStart.y(), worldEnd.y());
//
//		// 清空之前的选择 
//		m_selectedFiles.clear();
//		QStringList m_currentImageFilePath = m_imagePS->getSelectFilePath(m_currentImageTypeFilter);
//
//		// 检查哪些边界框在选择范围内 
//		for (const GeoBoundary &boundary : m_boundaries) {
//			// 首先检查文件是否在m_currentImageFilePath列表中
//			if (!m_currentImageTypeFilter.isEmpty() && !m_currentImageFilePath.contains(boundary.filePath)) {
//				continue;
//			}
//
//			//// 检查类型是否匹配
//			//if (!m_currentImageTypeFilter.isEmpty() && type != m_currentImageTypeFilter) {
//			//	continue;
//			//}
//
//			// 检查边界框是否在选择范围内 
//			bool inside = true;
//			for (const QPointF &corner : boundary.corners) {
//				if (corner.x() < minX || corner.x() > maxX ||
//					corner.y() < minY || corner.y() > maxY) {
//					inside = false;
//					break;
//				}
//			}
//
//			if (inside) {
//				m_selectedFiles.insert(boundary.filePath);
//			}
//		}
//
//		// 高亮选中的影像 
//		m_highlightedFiles = m_selectedFiles;
//		//qDebug() << "Highlighted files:" << m_highlightedFiles;
//		update();
//
//		// 发射信号通知外部选中了文件 
//		emit filesSelected(m_selectedFiles.toList());
//	}
//	else if ((!m_useMiddleButtonForPan && event->button() == Qt::LeftButton) ||
//		(m_useMiddleButtonForPan && event->button() == Qt::MiddleButton)) {
//		m_isPanning = false;
//	}
//	QOpenGLWidget::mouseReleaseEvent(event);
//}

void MeasurementareaShow::wheelEvent(QWheelEvent* event) {
	// 获取鼠标位置的世界坐标 
	QPointF mouseWorldPos = screenToWorld(event->position().toPoint());

	// 计算缩放因子 
	double zoomFactor = 1.1; // 默认放大10%
	if (event->angleDelta().y() < 0) {
		zoomFactor = 1.0 / zoomFactor; // 缩小
	}

	// 计算新的缩放比例
	double newScale = m_viewScale * zoomFactor;

	// 设置合理的缩放限制
	const double minScale = 0.001;  // 最小缩放级别 
	const double maxScale = 1000000000.0; // 最大缩放级别 
	newScale = qBound(minScale, newScale, maxScale);

	// 计算缩放后的鼠标位置 
	m_viewScale = newScale;
	QPointF newMouseWorldPos = screenToWorld(event->position().toPoint());

	// 调整视图中心保持鼠标位置稳定 
	m_viewCenterX += (mouseWorldPos.x() - newMouseWorldPos.x());
	m_viewCenterY += (mouseWorldPos.y() - newMouseWorldPos.y());

	update();
	event->accept();
}

// 坐标转换函数   
QPointF MeasurementareaShow::screenToWorld(const QPoint &screenPos) const {
	// 屏幕坐标到世界坐标的转换
	double x = (screenPos.x() - width() / 2.0) / m_viewScale + m_viewCenterX;
	double y = (height() / 2.0 - screenPos.y()) / m_viewScale + m_viewCenterY;
	return QPointF(x, y);
}

QPoint MeasurementareaShow::worldToScreen(const QPointF &worldPos) const {
	// 世界坐标到屏幕坐标的转换 
	int x = static_cast<int>((worldPos.x() - m_viewCenterX) * m_viewScale + width() / 2.0);
	int y = static_cast<int>((m_viewCenterY - worldPos.y()) * m_viewScale + height() / 2.0);
	return QPoint(x, y);
}

// 更新视图变换   
void MeasurementareaShow::updateViewTransform()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// 计算视图范围
	double halfWidth = width() / (2.0 * m_viewScale);
	double halfHeight = height() / (2.0 * m_viewScale);

	double left = m_viewCenterX - halfWidth;
	double right = m_viewCenterX + halfWidth;
	double bottom = m_viewCenterY - halfHeight;
	double top = m_viewCenterY + halfHeight;

	glOrtho(left, right, bottom, top, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

/////////////////////////////////////////////

// 辅助函数：格式化坐标到小数点后9位
QString MeasurementareaShow::formatGeoCoordinate(double x, double y) {
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(9) << x << ", " << y;
	return QString::fromStdString(oss.str());
}

// 主函数：获取影像旋转后的实际四顶点坐标
//std::vector<QString> MeasurementareaShow::getRotatedImageCorners(const QString& imagePath, const QString& rpcPath) {
//	GDALAllRegister();
//	std::vector<QString> corners;
//
//	GDALDataset* poDataset = (GDALDataset*)GDALOpen(imagePath.toUtf8().constData(), GA_ReadOnly);
//	if (!poDataset) {
//		corners.push_back(" 无法打开文件");
//		return corners;
//	}
//
//	// 获取源坐标系 
//	const char* srcWKT = poDataset->GetProjectionRef();
//	QString sourceCRS(srcWKT);
//
//	// 获取角点坐标 
//	double geoTransform[6];
//	if (poDataset->GetGeoTransform(geoTransform) == CE_None) {
//		// 使用double类型避免转换警告 
//		double xSize = static_cast<double>(poDataset->GetRasterXSize());
//		double ySize = static_cast<double>(poDataset->GetRasterYSize());
//
//		// 四个角点（左上、右上、右下、左下）
//		std::vector<QPointF> pixelCorners = {
//			QPointF(0.0, 0.0),
//			QPointF(xSize, 0.0),
//			QPointF(xSize, ySize),
//			QPointF(0.0, ySize)
//		};
//
//		for (const auto& corner : pixelCorners) {
//			double x = corner.x();
//			double y = corner.y();
//			double geoX, geoY;
//
//			GDALApplyGeoTransform(geoTransform, x, y, &geoX, &geoY);
//
//			// 转换到目标坐标系 
//			QPointF projectedPoint = convertToTargetCRS(QPointF(geoX, geoY), sourceCRS);
//			corners.push_back(formatGeoCoordinate(projectedPoint.x(), projectedPoint.y()));
//		}
//	}
//	else {
//		corners.push_back(" 无地理参考信息");
//	}
//
//	GDALClose(poDataset);
//	return corners;
//}

std::vector<QString> MeasurementareaShow::getRotatedImageCorners(const QString& imagePath, const QString& rpcPath) {
	GDALAllRegister();
	std::vector<QString> corners;

	// 1. 打开影像文件
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(imagePath.toUtf8().constData(), GA_ReadOnly);
	if (!poDataset) return corners;

	// 2. 判断是否使用RPC模型
	bool useRPC = false;
	GDALRPCInfo rpcInfo;
	void* hTransformArg = nullptr;
	ImageGeoMetadata* metadata = m_imagePS->getImageMetadata(imagePath);

	if (!rpcPath.isEmpty() || metadata) {
		// 加载RPC信息
		char** papszRPC = metadata->rpcMetadata;
		if (!rpcPath.isEmpty()) {
			papszRPC = GDALLoadRPCFile(rpcPath.toStdString());
		}
		else {
			papszRPC = metadata->rpcMetadata;
		}

		if (papszRPC && GDALExtractRPCInfo(papszRPC, &rpcInfo)) {
			hTransformArg = GDALCreateRPCTransformer(&rpcInfo, FALSE, 0, nullptr);
			useRPC = (hTransformArg != nullptr);
		}
	}

	// 3. 获取地理变换参数（非RPC时使用）
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
	else if (poDataset) {
		hasGeoTransform = (poDataset->GetGeoTransform(geoTransform) == CE_None);
	}
	// 4. 定义四角像素坐标（左上、右上、右下、左下）
	const double xSize = poDataset->GetRasterXSize();
	const double ySize = poDataset->GetRasterYSize();
	const std::vector<std::pair<double, double>> pixelCoords = {
		{0.0, 0.0},     // 左上角
		{xSize, 0.0},   // 右上角
		{xSize, ySize}, // 右下角
		{0.0, ySize}    // 左下角
	};

	// 5. 坐标转换逻辑
	for (const auto& coord : pixelCoords) {
		double x = coord.first;
		double y = coord.second;
		double geoX = 0.0, geoY = 0.0;

		//qDebug() << "坐标点x" << x << "坐标点y" << y;

		if (useRPC) {
			// RPC转换（包含旋转）
			double z = 0.0;
			int success = FALSE;
			GDALRPCTransform(hTransformArg, FALSE, 1, &x, &y, &z, &success);
			if (success) {
				//                QString geoCoord =formatDouble(x) + ", " + formatDouble(y);
				//                qDebug() << geoCoord;
				corners.push_back(formatGeoCoordinate(x, y));
				//qDebug() << corners;
			}
			else {
				corners.push_back("RPC转换失败");
			}
		}
		else if (metadata->geoTransform || hasGeoTransform) {
			// 地理变换矩阵转换（自动处理旋转）
			GDALApplyGeoTransform(geoTransform, x, y, &geoX, &geoY);
			corners.push_back(formatGeoCoordinate(geoX, geoY));
		}
		else {
			// 无地理信息
			corners.push_back("无地理参考信息");
		}
	}

	// 6. 清理资源
	if (hTransformArg) GDALDestroyTransformer(hTransformArg);
	GDALClose(poDataset);

	return corners;
}

void MeasurementareaShow::removeGeoBoundary(const QString &filePath) {
	// 直接通过文件路径匹配删除边界框 
	for (int i = 0; i < m_boundaries.size(); ++i) {
		if (m_boundaries[i].filePath == filePath) {
			m_boundaries.remove(i);
			--i; // 调整索引 
			//qDebug() << "Removed boundary for file:" << filePath;
		}
	}

	// 同时清除该文件的高亮状态 
	m_highlightedFiles.remove(filePath);

	// 更新世界范围并重绘 
	calculateWorldExtent();
	update();
}

//void MeasurementareaShow::removeGeoBoundary(const QString &filePath) {
//	// 获取旋转后的四个角点坐标作为比较基准
//	std::vector<QString> targetCorners = getRotatedImageCorners(filePath);
//	if (targetCorners.empty() || targetCorners[0] == "无地理参考信息") {
//		qWarning() << "无法获取有效地理坐标用于比较:" << filePath;
//		return;
//	}
//
//	// 转换为QPointF集合用于比较
//	QVector<QPointF> targetPoints;
//	for (const QString& corner : targetCorners) {
//		QStringList parts = corner.split(", ");
//		if (parts.size() == 2) {
//			targetPoints.append(QPointF(parts[0].toDouble(), parts[1].toDouble()));
//		}
//	}
//
//	// 遍历所有边界框，查找匹配项
//	for (int i = 0; i < m_boundaries.size(); ++i) {
//		const GeoBoundary& boundary = m_boundaries[i];
//		bool match = true;
//
//		// 比较四个角点是否匹配 
//		if (boundary.corners.size() == targetPoints.size()) {
//			for (int j = 0; j < boundary.corners.size(); ++j) {
//				if (boundary.corners[j] == targetPoints[j]) {
//					match = false;
//					break;
//				}
//			}
//
//			if (!match) {
//				m_boundaries.remove(i);
//				--i; // 调整索引 
//			}
//		}
//	}
//
//	// 更新世界范围并重绘 
//	calculateWorldExtent();
//	update();
//}

void MeasurementareaShow::loadPoints(const QStringList& filePathList, const QStringList filenames, const QString flag, const QList<QString> orderedPointIds)
{
	QMap<QString, QPointF> objPointMap;
	if ("con" == flag) {
		m_conPoints.clear();
		m_conpointIdToIndex.clear();

		// 从obj.txt 文件中读取第1,2,3列内容
		QString objFilePath = m_imagePS->projectdir + "SatTiePointMatch" + "/" + "obj.txt";
		QFile objFile(objFilePath);
		if (!objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qWarning() << "Cannot open obj.txt  file:" << objFilePath;
			return;
		}

		QTextStream objIn(&objFile);
		//QMap<int, QPointF> objPointMap; // 存储obj.txt 中的点(pointId -> worldPos)

		while (!objIn.atEnd()) {
			QString objLine = objIn.readLine().trimmed();
			if (objLine.isEmpty())  continue;

			QStringList objParts = objLine.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
			if (objParts.size() < 3) continue;

			bool ok;
			QString objPointId = objParts[0];
			//if (!ok){
			//	continue;
			//}

			double x = objParts[1].toDouble(&ok);
			if (!ok) {
				continue;
			}
			double y = objParts[2].toDouble(&ok);
			if (!ok) {
				continue;
			}
			objPointMap[objPointId] = QPointF(x, y);
		}
		objFile.close();
	}
	else if ("col" == flag) {
		m_colPoints.clear();
		m_colpointIdToIndex.clear();
	}

	int index = 0;
	for (const QString& filePath : filePathList) {
		QFileInfo fileInfo(filePath);
		QString suffix = fileInfo.suffix().toLower();

		QFile file(filePath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qWarning() << "Cannot open points file:" << filePath;
			continue;
		}

		QSet<QString> uniqueIds; // 去重容器 

		QTextStream in(&file);
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (line.isEmpty())   continue;

			QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);

			if ("con" == flag) {
				// 处理当前文件中的点 
				if (parts.size() < 4) continue;

				const QString pointId = parts[0];
				if (uniqueIds.contains(pointId)) {
					//qDebug() << "Skipping duplicate/invalid point ID:" << pointId;
					continue;
				}

				QString imageName = parts[1];
				QPointF pixelPos(parts[2].toDouble(), parts[3].toDouble());

				// 从objPointMap中获取对应的世界坐标
				QPointF worldPos;
				if (objPointMap.contains(pointId)) {
					worldPos = objPointMap[pointId];
				}
				else {
					continue;
				}

				uniqueIds.insert(pointId);
				m_conPoints.append({ pointId, imageName, worldPos, "tp" });

			}
			else if ("col" == flag) {
				// .txt 文件处理逻辑 - 使用orderedPointIds作为ID 
				if (parts.size() < 8) continue;

				// 检查orderedPointIds是否有效 
				if (index >= orderedPointIds.size()) {
					//qWarning() << "orderedPointIds size mismatch at index:" << index;
					continue;
				}

				QString pointId = orderedPointIds[index];
				if (uniqueIds.contains(pointId)) {
					//qDebug() << "Skipping duplicate/invalid point ID:" << pointId;
					continue;
				}

				QString imageName = fileInfo.completeBaseName();
				double x = parts[1].toDouble();
				double y = parts[2].toDouble();

				QPointF worldPos(x, y);
				uniqueIds.insert(pointId);

				int lastUnderscorePos = imageName.lastIndexOf('_');
				QString result = imageName.left(lastUnderscorePos);
				m_colPoints.append({ pointId, result, worldPos, "txt" });
				index++;
			}
			else {
				qWarning() << "Unsupported file format:" << filePath;
				break;
			}
		}
		file.close();
	}

	// 建立pointId到索引的映射 
	for (int i = 0; i < m_conPoints.size(); ++i) {
		m_conpointIdToIndex[m_conPoints[i].id] = i;
	}

	// 控制点的映射关系（使用数组索引作为key，pointId作为value）
	for (int i = 0; i < m_colPoints.size(); ++i) {
		m_colpointIdToIndex[i] = m_colPoints[i].id;
	}

	update();
}

//void MeasurementareaShow::loadPoints(const QStringList& filePathList, const QStringList filenames, const QString flag, const QList<QString> orderedPointIds)
//{
//	if ("con" == flag) {
//		m_conPoints.clear();
//		m_conpointIdToIndex.clear();
//	}
//	else if ("col" == flag) {
//		m_colPoints.clear();
//		m_colpointIdToIndex.clear();
//	}
//
//	int index = 0;
//	for (const QString& filePath : filePathList) {
//		QFileInfo fileInfo(filePath);
//		QString suffix = fileInfo.suffix().toLower();
//
//		QFile file(filePath);
//		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			//qWarning() << "Cannot open points file:" << filePath;
//			continue;
//		}
//
//		QSet<int> uniqueIds; // 去重容器 
//
//		QTextStream in(&file);
//		while (!in.atEnd()) {
//			QString line = in.readLine().trimmed();
//			if (line.isEmpty())  continue;
//
//			QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
//
//			if ("con" == flag) {
//				// .tp 文件处理逻辑（保持不变）
//				if (parts.size() < 4) continue;
//
//				bool ok;
//				const int pointId = parts[0].toInt(&ok);
//				if (!ok || uniqueIds.contains(pointId)) {
//					//qDebug() << "Skipping duplicate/invalid point ID:" << pointId;
//					continue;
//				}
//
//				QString imageName = parts[1];
//				QPointF pixelPos(parts[2].toDouble(), parts[3].toDouble());
//				QPointF worldPos = pixelToWorld(pixelPos, imageName, filenames);
//
//				uniqueIds.insert(pointId);
//				m_conPoints.append({ pointId, imageName, worldPos, "tp" });
//
//			}
//			else if ("col" == flag) {
//				// .txt 文件处理逻辑 - 使用orderedPointIds作为ID 
//				if (parts.size() < 8) continue;
//
//				// 检查orderedPointIds是否有效 
//				if (index >= orderedPointIds.size()) {
//					qWarning() << "orderedPointIds size mismatch at index:" << index;
//					continue;
//				}
//
//				bool ok;
//				int pointId = orderedPointIds[index].toInt(&ok);
//				if (!ok || uniqueIds.contains(pointId)) {
//					//qDebug() << "Skipping duplicate/invalid point ID:" << pointId;
//					continue;
//				}
//
//				QString imageName = fileInfo.completeBaseName();
//				double x = parts[1].toDouble();
//				double y = parts[2].toDouble();
//
//				//OGRSpatialReference sourceSRS;
//				//if (qAbs(x) <= 180 && qAbs(y) <= 90) {
//				//	sourceSRS.SetWellKnownGeogCS("WGS84");
//				//}
//				//else {
//				//	sourceSRS.SetFromUserInput("PROJCS[\"CGCS2000\",GEOGCS[\"GCS_China_Geodetic_Coordinate_System_2000\",DATUM[\"China_2000\",SPHEROID[\"CGCS2000\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",120],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1]]");
//				//}
//
//				//OGRSpatialReference* targetSRS = createTargetSRS();
//				//if (!transformCoordinates(&sourceSRS, targetSRS, x, y)) {
//				//	qWarning() << "Failed to transform coordinates for file:" << imageName;
//				//}
//				//delete targetSRS;
//
//				QPointF worldPos(x, y);
//				uniqueIds.insert(pointId);
//				m_colPoints.append({ pointId, imageName, worldPos, "txt" });
//				index++;
//			}
//			else {
//				qWarning() << "Unsupported file format:" << filePath;
//				break;
//			}
//		}
//		file.close();
//	}
//
//	// 建立pointId到索引的映射 
//	for (int i = 0; i < m_conPoints.size(); ++i) {
//		m_conpointIdToIndex[m_conPoints[i].id] = i;
//	}
//
//	// 控制点的映射关系（使用数组索引作为key，pointId作为value）
//	for (int i = 0; i < m_colPoints.size(); ++i) {
//		m_colpointIdToIndex[i] = m_colPoints[i].id;
//	}
//
//	update();
//}

// 根据pointId删除点 
void MeasurementareaShow::removePointById(QString pointId, const QString flag) {
	if ("con" == flag) {
		if (m_conpointIdToIndex.contains(pointId)) {
			int index = m_conpointIdToIndex[pointId];
			m_conPoints.remove(index);

			// 更新映射表 
			m_conpointIdToIndex.remove(pointId);
			for (auto& id : m_conpointIdToIndex.keys()) {
				if (m_conpointIdToIndex[id] > index) {
					m_conpointIdToIndex[id]--;
				}
			}
			update();
		}
	}
	else if ("col" == flag) {
		// 查找要删除的点的索引 
		int removeIndex = -1;
		for (int i = 0; i < m_colPoints.size(); ++i) {
			if (m_colPoints[i].id == pointId) {
				removeIndex = i;
				break;
			}
		}

		if (removeIndex != -1) {
			m_colPoints.remove(removeIndex);

			// 更新映射表 
			m_colpointIdToIndex.clear();
			for (int i = 0; i < m_colPoints.size(); ++i) {
				m_colpointIdToIndex[i] = m_colPoints[i].id;
			}
			update();
		}
	}
}

void MeasurementareaShow::removePointsByFilePath(const QString& filePath, const QString& flag)
{
	if ("con" == flag) {
		// 删除连接点
		QVector<Point> newConPoints;
		for (int i = 0; i < m_conPoints.size(); ++i) {
			if (m_conPoints[i].imageName != QFileInfo(filePath).completeBaseName()) {
				newConPoints.append(m_conPoints[i]);
			}
		}
		m_conPoints = newConPoints;

		// 重建映射表
		m_conpointIdToIndex.clear();
		for (int i = 0; i < m_conPoints.size(); ++i) {
			m_conpointIdToIndex[m_conPoints[i].id] = i;
		}
	}
	else if ("col" == flag) {
		// 删除控制点 
		QVector<Point> newColPoints;
		for (int i = 0; i < m_colPoints.size(); ++i) {
			if (m_colPoints[i].imageName != QFileInfo(filePath).completeBaseName()) {
				newColPoints.append(m_colPoints[i]);
			}
		}
		m_colPoints = newColPoints;

		// 重建映射表
		m_colpointIdToIndex.clear();
		for (int i = 0; i < m_colPoints.size(); ++i) {
			m_colpointIdToIndex[i] = m_colPoints[i].id;
		}
	}

	update();
}

//void MeasurementareaShow::highlightPointById(QString pointId, const QString flag, bool highlight) {
//	if ("con" == flag) {
//		if (m_conpointIdToIndex.contains(pointId)) {
//			if (highlight) {
//				m_highlightedconPointIds.insert(pointId);
//			}
//			else {
//				m_highlightedconPointIds.remove(pointId);
//			}
//			update();
//		}
//	}
//	else if ("col" == flag) {
//		// 检查pointId是否存在于控制点中 
//		bool found = false;
//		for (const auto& point : m_colPoints) {
//			if (point.id == pointId) {
//				found = true;
//				break;
//			}
//		}
//
//		if (found) {
//			if (highlight) {
//				m_highlightedcolPointIds.insert(pointId);
//			}
//			else {
//				m_highlightedcolPointIds.remove(pointId);
//			}
//			update();
//		}
//	}
//}

// 单点操作兼容版本
void MeasurementareaShow::highlightPointById(QString pointId,
	const QString& flag,
	bool highlight)
{
	highlightPointsById({ pointId }, flag, highlight);
}

/**
 * @brief 高亮/取消高亮多个点
 * @param pointIds 要操作的点ID集合（支持多个点）
 * @param flag 点类型标识（"con"或"col"）
 * @param highlight true=高亮，false=取消高亮
 */
void MeasurementareaShow::highlightPointsById(const QSet<QString>& pointIds,
	const QString& flag,
	bool highlight)
{
	bool needUpdate = false;

	if ("con" == flag) {
		for (const auto& id : pointIds) {
			if (m_conpointIdToIndex.contains(id)) {
				if (highlight) {
					m_highlightedconPointIds.insert(id);
				}
				else {
					m_highlightedconPointIds.remove(id);
				}
				needUpdate = true;
			}
		}
	}
	else if ("col" == flag) {
		//bool anyValid = std::any_of(pointIds.begin(), pointIds.end(), [this](const QString& id) {
		//	return std::any_of(m_colPoints.begin(), m_colPoints.end(),
		//		[&id](const auto& point) { return point.id == id; });
		//});

		//if (anyValid) {
			for (const auto& id : pointIds) {
				if (highlight) {
					m_highlightedcolPointIds.insert(id);
				}
				else {
					m_highlightedcolPointIds.remove(id);
				}
			}
			needUpdate = true;
		//}
	}

	if (needUpdate) {
		update();  // 统一触发界面刷新 
	}
}

// 清除所有高亮 
void MeasurementareaShow::clearPointHighlights() {
	m_highlightedconPointIds.clear();
	m_highlightedcolPointIds.clear();
	update();
}

void MeasurementareaShow::showPoints(bool show)
{
	m_showPoints = show;
	update();
}

void MeasurementareaShow::drawPoints() {
	if (!m_showPoints || (m_conPoints.isEmpty() && m_colPoints.isEmpty()))   return;

	// 保存OpenGL状态 
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	// 临时切换到屏幕坐标系 
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width(), height(), 0, -1, 1);

	// 绘制所有连接点 
	for (const Point& tp : m_conPoints) {
		// 检查点是否应该显示
		if (m_onlyShowVisibleIds && !m_visiblePointIds.contains(tp.id)) {
			continue;
		}

		// 检查点是否可见 
		if (!isConPointVisible(tp.id))  continue;

		QPoint screenPos = worldToScreen(tp.position);

		//高亮点加粗显示 
		if (m_highlightedconPointIds.contains(tp.id)) {
			glColor3f(1.0f, 1.0f, 0.0f); // 高亮显示为黄色 
			glLineWidth(3.0f);
		}
		else {
			glColor3f(0.0f, 1.0f, 0.0f); // .tp文件 - 绿色 
			glLineWidth(1.0f);
		}

		// 绘制十字标记 
		const float crossSize = 1.0f;
		glBegin(GL_LINES);
		glVertex2f(screenPos.x() - crossSize, screenPos.y());
		glVertex2f(screenPos.x() + crossSize, screenPos.y());
		glVertex2f(screenPos.x(), screenPos.y() - crossSize);
		glVertex2f(screenPos.x(), screenPos.y() + crossSize);
		glEnd();
	}

	// 绘制所有控制点 
	for (const Point& col : m_colPoints) {
		// 检查点是否应该显示 
		if (m_onlyShowVisibleIds && !m_visiblePointIds.contains(col.id)) {
			continue;
		}

		// 检查点是否可见 
		if (!isColPointVisible(col.id))  continue;

		QPoint screenPos = worldToScreen(col.position);

		//高亮点加粗显示 
		if (m_highlightedcolPointIds.contains(col.id)) {
			glColor3f(1.0f, 1.0f, 0.0f); // 高亮显示为黄色 
			glLineWidth(3.0f);
		}
		else
		{
			glColor3f(1.0f, 0.0f, 0.0f); // .txt文件 - 红色 
			glLineWidth(1.0f);
		}

		// 绘制十字标记 
		const float crossSize = 1.0f;
		glBegin(GL_LINES);
		glVertex2f(screenPos.x() - crossSize, screenPos.y());
		glVertex2f(screenPos.x() + crossSize, screenPos.y());
		glVertex2f(screenPos.x(), screenPos.y() - crossSize);
		glVertex2f(screenPos.x(), screenPos.y() + crossSize);
		glEnd();
	}

	// 恢复世界坐标系 
	glPopMatrix();
	glPopAttrib();
}

// 连接点显示/隐藏设置
void MeasurementareaShow::setConPointsVisible(bool visible, QString pointId)
{
	if (pointId == "-1") {
		// 整体设置 
		m_conVisibility.allVisible = visible;
		m_conVisibility.hiddenIds.clear();
	}
	else {
		// 单个点设置
		if (visible) {
			m_conVisibility.hiddenIds.remove(pointId);
		}
		else {
			m_conVisibility.hiddenIds.insert(pointId);
		}
	}
	update();
}

bool MeasurementareaShow::isConPointVisible(QString pointId) const
{
	if (!m_conVisibility.allVisible)  return false;
	return !m_conVisibility.hiddenIds.contains(pointId);
}

void MeasurementareaShow::showAllConPoints()
{
	setConPointsVisible(true);
}

void MeasurementareaShow::hideAllConPoints()
{
	setConPointsVisible(false);
}

// 控制点显示/隐藏设置
void MeasurementareaShow::setColPointsVisible(bool visible, QString pointId)
{
	if (pointId == "-1") {
		// 整体设置 
		m_colVisibility.allVisible = visible;
		m_colVisibility.hiddenIds.clear();
	}
	else {
		// 单个点设置
		if (visible) {
			m_colVisibility.hiddenIds.remove(pointId);
		}
		else {
			m_colVisibility.hiddenIds.insert(pointId);
		}
	}
	update();
}

bool MeasurementareaShow::isColPointVisible(QString pointId) const
{
	if (!m_colVisibility.allVisible)  return false;
	return !m_colVisibility.hiddenIds.contains(pointId);
}

void MeasurementareaShow::showAllColPoints()
{
	setColPointsVisible(true);
}

void MeasurementareaShow::hideAllColPoints()
{
	setColPointsVisible(false);
}

// 统一管理所有点 
void MeasurementareaShow::setAllPointsVisible(bool visible)
{
	setConPointsVisible(visible);
	setColPointsVisible(visible);
}



//////////////////////////////////////////////////
//QPointF MeasurementareaShow::pixelToWorld(const QPointF& pixelPos, QString& imageName)
//{
//	imageName = "E:\\JLtestData\\origin\\JL1GF02A_PMS1_20230618093553_200166837_102_0025_001_L1_MSS\\JL1GF02A_PMS1_20230618093553_200166837_102_0025_001_L1_MSS.tif";
//
//	// 使用与getRotatedImageCorners相同的GDAL转换方式 
//	GDALDataset* poDataset = (GDALDataset*)GDALOpen(imageName.toUtf8(), GA_ReadOnly);
//	if (!poDataset) {
//		qWarning() << u8"无法打开图像文件:" << imageName;
//		return QPointF(0, 0);
//	}
//
//	// 获取地理变换参数
//	double geoTransform[6];
//	if (poDataset->GetGeoTransform(geoTransform) != CE_None) {
//		qWarning() << u8"无法获取地理变换参数:" << imageName;
//		GDALClose(poDataset);
//		return QPointF(0, 0);
//	}
//
//	// 应用与边界框相同的坐标转换
//	double worldX, worldY;
//	GDALApplyGeoTransform(geoTransform, pixelPos.x(), pixelPos.y(), &worldX, &worldY);
//
//	// Y轴方向调整（如果需要）
//	worldY = -worldY; // 根据您的坐标系需求决定是否需要反转Y轴
//
//	GDALClose(poDataset);
//	return QPointF(worldX, worldY);
//}

QPointF MeasurementareaShow::pixelToWorld(const QPointF& pixelPos, QString& imageName, const QStringList filenames)
{
	QString rpcPath = "";
	QString filenametmp = "";
	for (auto data : filenames)
	{
		QFileInfo fileInfodata(data);
		if (fileInfodata.completeBaseName() == imageName)
		{
			imageName = data;
			rpcPath = fileInfodata.absolutePath() + "/" + fileInfodata.completeBaseName() + QString::fromLocal8Bit("_rpc.txt");
			break;
		}
	}

	GDALDataset* poDataset = (GDALDataset*)GDALOpen(imageName.toUtf8().constData(), GA_ReadOnly);
	if (!poDataset) {
		//qWarning() << "Failed to open image file:" << imageName;
		return QPointF(0, 0);
	}

	// 加载 RPC 文件 
	char **rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
	if (rpcMetadata == nullptr) {
		qWarning() << u8"Failed to load RPC file!";
		GDALClose(poDataset);
		return QPointF(0, 0);
	}

	// 写入 RPC 元数据
	//poDataset->SetMetadata(rpcMetadata, "RPC");

	// 使用转换后的屏幕坐标作为四角点
	double x[1], y[1], z[1] = { 0 };
	int success[1] = { 0 };

	x[0] = pixelPos.x();
	y[0] = pixelPos.y();
	z[0] = 0;

	// 从RPC元数据中获取高程信息
	double heightOffset = 0.0;
	double heightScale = 1.0;
	const char* pszHeightOffset = CSLFetchNameValue(rpcMetadata, "HEIGHT_OFF");
	const char* pszHeightScale = CSLFetchNameValue(rpcMetadata, "HEIGHT_SCALE");

	if (pszHeightOffset && pszHeightScale) {
		heightOffset = CPLAtof(pszHeightOffset);
		heightScale = CPLAtof(pszHeightScale);
	}
	else {
		qWarning() << u8"未找到RPC高程参数，使用默认高程0.0";
	}

	x[0] = pixelPos.x();
	y[0] = pixelPos.y();
	z[0] = heightOffset;

	// 创建 RPC 转换器
	GDALRPCInfoV2 rpcInfo = { 0 };
	GDALExtractRPCInfo(rpcMetadata, &rpcInfo);
	void *transformer = GDALCreateRPCTransformerV2(&rpcInfo, false, 0.1, nullptr);
	if (transformer == nullptr) {
		qWarning() << u8"Failed to create RPC transformer!";
		GDALClose(poDataset);
		return QPointF(0, 0);
	}

	// 执行 RPC 转换
	GDALRPCTransform(transformer, FALSE, 1, x, y, z, success);

	// 获取源坐标系 
	OGRSpatialReference sourceSRS;
	const char* pszProjection = poDataset->GetProjectionRef();
	if (pszProjection && strlen(pszProjection) > 0) {
		sourceSRS.importFromWkt(pszProjection);
	}
	else {
		// 根据坐标值判断坐标系类型 
		if (qAbs(x[0]) <= 180 && qAbs(y[0]) <= 90) {
			sourceSRS.SetWellKnownGeogCS("WGS84");
			//qDebug() << "Assuming WGS84 geographic coordinates for pixel position";
		}
		else {
			sourceSRS.SetFromUserInput("PROJCS[\"CGCS2000\",GEOGCS[\"GCS_China_Geodetic_Coordinate_System_2000\",DATUM[\"China_2000\",SPHEROID[\"CGCS2000\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",120],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1]]");
			//qDebug() << "Assuming CGCS2000 projected coordinates for pixel position";
		}
	}

	// 创建目标坐标系 
	OGRSpatialReference* targetSRS = createTargetSRS();

	// 执行坐标转换 
	if (!transformCoordinates(&sourceSRS, targetSRS, x[0], y[0])) {
		qWarning() << "Failed to transform coordinates for file:" << imageName;
	}

	delete targetSRS;
	GDALClose(poDataset);

	QPointF worldPos(x[0], y[0]);

	return worldPos;
}

// 创建目标坐标系(CGCS_2000_129)
OGRSpatialReference* MeasurementareaShow::createTargetSRS() {
	OGRSpatialReference* targetSRS = new OGRSpatialReference();
	//targetSRS->SetFromUserInput("PROJCS[\"CGCS_2000_129\",GEOGCS[\"GCS_CGCS_2000\",DATUM[\"China_2000\",SPHEROID[\"CGCS2000\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",129],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1]]");
	targetSRS->SetFromUserInput("+proj=tmerc +lat_0=0 +lon_0=129 +k=1 +x_0=500000 +y_0=0 +ellps=GRS80 +units=m +no_defs");
	return targetSRS;
}

// 坐标转换函数 
bool MeasurementareaShow::transformCoordinates(OGRSpatialReference* sourceSRS,
	OGRSpatialReference* targetSRS,
	double& x, double& y)
{
	if (!sourceSRS || !targetSRS) {
		return false;
	}

	// 检查源坐标系是否为WGS84地理坐标系
	bool isWGS84 = false;
	const char* wgs84WKT = "GEOGCS[\"WGS 84\"";
	char* sourceWKT = nullptr;
	sourceSRS->exportToWkt(&sourceWKT);
	if (sourceWKT && strstr(sourceWKT, wgs84WKT) != nullptr) {
		isWGS84 = true;
	}
	CPLFree(sourceWKT);

	// 创建坐标转换对象
	OGRCoordinateTransformation* poCT = OGRCreateCoordinateTransformation(sourceSRS, targetSRS);
	if (!poCT) {
		qWarning() << "Failed to create coordinate transformation";
		return false;
	}
	bool isGeographic = sourceSRS->IsGeographic();
	// 执行坐标转换，处理WGS84坐标顺序
	bool success;
	if (isGeographic) {
		success = poCT->Transform(1, &y, &x);
		double tmpx = fabs(x);
		x = fabs(y);
		y = tmpx;
	}
	else {
		success = poCT->Transform(1, &x, &y);
		x = fabs(x);
		y = fabs(y);
	}

	if (!success) {
		qWarning() << "Coordinate transformation failed";
		OCTDestroyCoordinateTransformation(poCT);
		return false;
	}

	OCTDestroyCoordinateTransformation(poCT);
	return true;
}


// 重新投影边界框 
// 在reprojectBoundary函数中改进投影识别
void MeasurementareaShow::reprojectBoundary(GeoBoundary& boundary, OGRSpatialReference* targetSRS) {
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(boundary.filePath.toUtf8().constData(), GA_ReadOnly);
	if (!poDataset) {
		qWarning() << "Failed to open file:" << boundary.filePath;
		return;
	}

	// 获取源坐标系 
	OGRSpatialReference sourceSRS;
	const char* pszProjection = poDataset->GetProjectionRef();
	if (pszProjection && strlen(pszProjection) > 0) {
		sourceSRS.importFromWkt(pszProjection);
	}
	else {
		// 如果没有投影信息，检查坐标范围 
		bool isLatLon = true;
		for (const QPointF& corner : boundary.corners) {
			if (qAbs(corner.x()) > 180 || qAbs(corner.y()) > 90) {
				isLatLon = false;
				break;
			}
		}

		if (isLatLon) {
			sourceSRS.SetWellKnownGeogCS("WGS84");
		}
		else {
			// 假设是CGCS2000投影坐标系
			sourceSRS.SetFromUserInput("PROJCS[\"CGCS2000\",GEOGCS[\"GCS_China_Geodetic_Coordinate_System_2000\",DATUM[\"China_2000\",SPHEROID[\"CGCS2000\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.017453292519943295]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",120],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",0],UNIT[\"Meter\",1]]");
		}
	}

	// 转换每个角点坐标
	for (QPointF& corner : boundary.corners) {
		double x = corner.x();
		double y = corner.y();
		if (!transformCoordinates(&sourceSRS, targetSRS, x, y)) {
			qWarning() << "Failed to transform coordinates for file:" << boundary.filePath;
			continue;
		}
		corner.setX(x);
		corner.setY(y);
	}

	GDALClose(poDataset);
}

void MeasurementareaShow::highlightBoundary(const QString& filePath, bool highlight) {
	if (highlight) {
		m_highlightedFiles.insert(filePath);
	}
	else {
		m_highlightedFiles.remove(filePath);
	}
	update(); // 触发重绘
}

void MeasurementareaShow::highlightsBoundary(const QStringList& filePath, bool highlight) {
	if (highlight) {
		for (auto filename : filePath) {
			m_highlightedFiles.insert(filename);
		}
	}
	else {
		for (auto filename : filePath) {
			m_highlightedFiles.remove(filename);
		}
	}
	update(); // 触发重绘
}

void MeasurementareaShow::clearHighlights() {
	m_highlightedFiles.clear();
	update();
}

///////////////////////////////////////
/**
 * @brief 影像选择功能
 */
 // 设置当前影像类型过滤 
void MeasurementareaShow::setImageTypeFilter(const QString& type) {
	m_currentImageTypeFilter = type;
}

// 设置选择模式
void MeasurementareaShow::setSelectionMode(bool enabled) {
	m_useMiddleButtonForPan = enabled;
	if (!enabled) {
		clearSelection();
	}
	update();
}

// 清除选择
void MeasurementareaShow::clearSelection() {
	//m_selectedFiles.clear();
	m_selectionRect = QRect();
	update();
}

// 绘制选择框 
void MeasurementareaShow::drawSelectionRect() {
	if (!m_selectionRect.isValid())  return;

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, width(), height(), 0, -1, 1);

	// 绘制半透明填充 
	glColor4f(0.2f, 0.4f, 0.8f, 0.2f);
	glBegin(GL_QUADS);
	glVertex2i(m_selectionRect.left(), m_selectionRect.top());
	glVertex2i(m_selectionRect.right(), m_selectionRect.top());
	glVertex2i(m_selectionRect.right(), m_selectionRect.bottom());
	glVertex2i(m_selectionRect.left(), m_selectionRect.bottom());
	glEnd();

	// 绘制边框 
	glColor3f(0.0f, 0.0f, 1.0f);
	glLineWidth(1.5f);
	glBegin(GL_LINE_LOOP);
	glVertex2i(m_selectionRect.left(), m_selectionRect.top());
	glVertex2i(m_selectionRect.right(), m_selectionRect.top());
	glVertex2i(m_selectionRect.right(), m_selectionRect.bottom());
	glVertex2i(m_selectionRect.left(), m_selectionRect.bottom());
	glEnd();

	glPopMatrix();
	glPopAttrib();
}

/**
 * @brief 点选择功能
 */

void MeasurementareaShow::setPointSelectionMode(bool enabled) {
	m_isPointSelecting = enabled;
	m_useMiddleButtonForPan = enabled;

	if (!enabled) {
		clearPointSelection();
	}
	update();
}

void MeasurementareaShow::setPointTypeFilter(const QString& type) {
	if (type == "con" || type == "col" || type == "all") {
		m_currentPointTypeFilter = type;
	}
}

void MeasurementareaShow::clearPointSelection() {
	// 取消所有点的高亮 
	highlightPointsById(m_selectedConPoints, "con", false);
	highlightPointsById(m_selectedColPoints, "col", false);

	m_selectedConPoints.clear();
	m_selectedColPoints.clear();
	update();
}

void MeasurementareaShow::setVisiblePointIds(const QSet<QString>& ids) {
	m_visiblePointIds = ids;
	update(); // 触发重绘
}

// 设置是否只显示指定的点ID
void MeasurementareaShow::setOnlyShowVisibleIds(bool enable) {
	m_onlyShowVisibleIds = enable;
	update(); // 触发重绘 
}