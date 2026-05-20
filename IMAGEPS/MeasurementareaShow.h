#ifndef MEASUREMENTAREASHOW_H   
#define MEASUREMENTAREASHOW_H

#include <QOpenGLWidget>   
#include <QOpenGLFunctions>   
#include <QOpenGLTexture>
#include <QVector>   
#include <QMouseEvent>   
#include <QWheelEvent>   
#include <QPointF>   
#include <QFileInfo>   
#include <cmath>
#include <sstream>  
#include <iomanip>  
#include <QXmlStreamReader>
#include <QThread>
#include <proj.h>
#include "gdal.h"   
#include "gdal_priv.h"  
#include "gdal_alg.h"
#include "cpl_conv.h"
#include "gdal_mdreader.h"
#include "ogr_spatialref.h"
#include "gdalwarper.h"
#include <QRect>
#include <QSet>

class IMAGEPS;

struct GeoBoundary {
	QVector<QPointF> corners;  // 存储四个角点坐标
	QColor color;
	QString filePath;  // 添加文件路径成员
};

struct GeoImage {
	QString filePath;
	QColor borderColor;
	double minX, maxX, minY, maxY;
	QOpenGLTexture* texture; 
	bool isPanchromatic;   
};
struct Point {
	QString id;
	QString imageName;
	QPointF position;
	QString fileType; 
};

enum ImageType {
	DEM,
	DOM,
	MULTISPECTRAL,
	PANCHROMATIC,
	UNKNOWN
};

class MeasurementareaShow : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	explicit MeasurementareaShow(IMAGEPS* parentImagePS = nullptr, QWidget* parent = nullptr);

	//explicit MeasurementareaShow(QWidget *parent = nullptr);
	~MeasurementareaShow();
	//static MeasurementareaShow* Measurementareainstance;
	QVector<GeoBoundary> m_boundaries;
	QMap<QString, QPolygonF> tmpscreenCorners;
	QStringList tmpfilepath;
	QSet<QString> m_excludedReferences;

	QSet<QString> m_visiblePointIds; // 存储需要显示的点ID 
	bool m_onlyShowVisibleIds = false; // 是否只显示m_visiblePointIds中的点 

	void addGeoBoundary(const QString &filePath, const QColor &color = Qt::red);
	bool parseXmlBoundary(const QString &xmlPath, GeoBoundary &boundary);
	bool containsBoundary(const QVector<GeoBoundary>& boundaries, const QString& filePath);
	bool isProjectedWithZone(const QString& imagePath, int* zone);
	bool calculateCornerCoordinates(const QString filePath, GeoBoundary& boundary);
	void clearBoundaries();

	void removeGeoBoundary(const QString &filePath); 

	void loadPoints(const QStringList& filePathList, const QStringList filenames, const QString flag, const QList<QString> orderedPointIds);  // 添加加载连接点函数 

	void showPoints(bool show);  // 设置是否显示连接点
	void removePointById(QString pointId, const QString flag); // 根据pointId删除点 
	void removePointsByFilePath(const QString& filePath, const QString& flag);
	void highlightPointById(QString pointId, const QString& flag,bool highlight = true); // 根据pointId高亮
	void highlightPointsById(const QSet<QString>& pointIds,
		const QString& flag,
		bool highlight);
	void clearPointHighlights(); // 清除所有点高亮

	// 连接点(con)显示/隐藏函数 
	void setConPointsVisible(bool visible, QString pointId = "-1");
	bool isConPointVisible(QString pointId) const;
	void showAllConPoints();
	void hideAllConPoints();

	// 控制点(col)显示/隐藏函数
	void setColPointsVisible(bool visible, QString pointId = "-1");
	bool isColPointVisible(QString pointId) const;
	void showAllColPoints();
	void hideAllColPoints();

	// 统一管理函数 
	void setAllPointsVisible(bool visible);

	QPointF pixelToWorld(const QPointF& pixelPos,  QString& imageName, const QStringList filenames);

	// 坐标系转换函数 
	bool transformCoordinates(OGRSpatialReference* sourceSRS, OGRSpatialReference* targetSRS,
		double& x, double& y);
	void reprojectBoundary(GeoBoundary& boundary, OGRSpatialReference* targetSRS);
	OGRSpatialReference* createTargetSRS();

	void highlightBoundary(const QString& filePath, bool highlight = true);
	void highlightsBoundary(const QStringList& filePath, bool highlight = true);
	void clearHighlights();

	QPointF screenToWorld(const QPoint &screenPos) const;

	// 存储每张图像的地理变换参数 
	QMap<QString, double*> m_geoTransforms;
	double m_worldMinX, m_worldMaxX, m_worldMinY, m_worldMaxY;


	// 交互控制函数
	void zoomIn();
	void zoomOut();
	void zoomToFullExtent();
	void pan(double dx, double dy);
	void paintGL();

	void setImageTypeFilter(const QString& type);
	void setSelectionMode(bool enabled);
	void clearSelection();
	const QSet<QString>& getSelectedFiles() const { return m_selectedFiles; }
	void drawSelectionRect(); // 绘制选择框
	QSet<QString> m_selectedFiles; // 当前选中的文件

		// 设置点选择模式
	void setPointSelectionMode(bool enabled);

	// 设置点类型过滤 
	void setPointTypeFilter(const QString& type);

	// 获取选中的点 
	const QSet<QString>& getSelectedConPoints() const { return m_selectedConPoints; }
	const QSet<QString>& getSelectedColPoints() const { return m_selectedColPoints; }

	// 清除点选择
	void clearPointSelection();

	QStringList getExcludedReferences() const;

	void setVisiblePointIds(const QSet<QString>& ids);
	void setOnlyShowVisibleIds(bool enable);

signals:
	void filesSelected(const QStringList& files);
	void pointsSelected(QSet<QString>& m_selectedConPoints, QSet<QString>& m_selectedColPoints);

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	

	// 鼠标事件处理
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	IMAGEPS* m_imagePS;
	struct ImageData {
		QOpenGLTexture* texture = nullptr;
		float minX = 0, maxX = 0;
		float minY = 0, maxY = 0;
		bool isPanchromatic = false;
		bool isBlockProcessed = false;
	};
	GDALDataset *currentDataset;
	// 视图变换参数
	double m_viewCenterX, m_viewCenterY;
	double m_viewScale;
	bool m_isPanning;
	QPoint m_lastPanPos;

	// 选择框相关变量
	QRect m_selectionRect;
	bool m_isSelecting = false;
	QPoint m_selectionStart;
	bool m_useMiddleButtonForPan = false; // 是否使用中键平移 

	// 影像类型过滤
	QString m_currentImageTypeFilter; // 当前选择的影像类型(DOM/DEM/原始影像)

	// 点显示状态管理
	struct PointVisibility {
		bool allVisible = true; // 整体显示状态
		QSet<QString> hiddenIds;    // 被隐藏的单个点ID
	};

	PointVisibility m_conVisibility; // 连接点显示状态
	PointVisibility m_colVisibility; // 控制点显示状态

	QVector<Point> m_conPoints;  // 存储连接点数据
	QVector<Point> m_colPoints;  // 存储控制点数据
	bool m_showPoints = false;   // 是否显示连接点标志
	QMap<QString, int> m_conpointIdToIndex; // 存储pointId到连接点数组索引的映射 
	QMap<int, QString> m_colpointIdToIndex; // 存储pointId到控制点数组索引的映射 
	QSet<QString> m_highlightedconPointIds; // 存储高亮连接点的ID
	QSet<QString> m_highlightedcolPointIds; // 存储高亮控制点的ID
	QVector<GeoImage> m_images;

	QSet<QString> m_highlightedFiles; // 存储当前高亮的文件路径 
	float m_highlightWidth = 3.0f;    // 高亮线条宽度
	QColor m_highlightColor = Qt::yellow; // 高亮颜色

	// 点选择相关变量
	bool m_isPointSelecting = false; // 是否处于点选择模式
	QSet<QString> m_selectedConPoints; // 选中的连接点
	QSet<QString> m_selectedColPoints; // 选中的控制点
	QString m_currentPointTypeFilter = "all"; // 当前点类型过滤("con"/"col"/"all")

	void drawPoints();  // 绘制连接点函数

	void calculateWorldExtent();
	void drawBoundary(const GeoBoundary &boundary);

	// 坐标转换函数
	QPoint worldToScreen(const QPointF &worldPos) const;

	// 更新视图变换
	void updateViewTransform();

	QString formatGeoCoordinate(double x, double y);

	std::vector<QString> getRotatedImageCorners(const QString& imagePath, const QString& rpcPath = "");

};

#endif 