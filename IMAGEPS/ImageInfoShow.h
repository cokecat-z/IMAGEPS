#ifndef IMAGEINFOSHOW_H 
#define IMAGEINFOSHOW_H 

#include <QWidget>
#include "qcustomplot.h"
#include <QVector>
#include <QPointF>
#include <QColor>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <QFileInfo>
#include <QXmlStreamReader>
#include <QDomDocument>
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
#include "PublicStruct.h"
#include "MeasurementareaShow.h"   

class IMAGEPS;

class ImageInfoShow : public QCustomPlot
{
	Q_OBJECT
public:
	//explicit ImageInfoShow(IMAGEPS* parentImagePS = nullptr, QWidget* parent = nullptr);
	explicit ImageInfoShow(QWidget *parent = nullptr);
	~ImageInfoShow();

	void setImagePS(IMAGEPS* imagePS);

	QVector<GeoBoundary> m_boundaries;
	QMap<QString, QPolygonF> tmpscreenCorners;
	QStringList tmpfilepath;
	QSet<QString> m_excludedReferences;
	QHash<QRgb, QStringList> m_retainReferences;
	QSet<QString> m_visiblePointIds;
	bool m_onlyShowVisibleIds = false;

	void addGeoBoundary(const QString &filePath, const QColor &color = Qt::red);
	bool parseXmlBoundary(const QString &xmlPath, GeoBoundary &boundary);
	bool containsBoundary(const QVector<GeoBoundary>& boundaries, const QString& filePath);
	bool removeFromRetainReferences(const QString & fileName, const QColor & color);
	bool isProjectedWithZone(const QString& imagePath, int* zone);
	bool calculateCornerCoordinates(const QString filePath, GeoBoundary& boundary);
	void clearBoundaries();
	void removeGeoBoundary(const QString &filePath);

	// 筛选所有参考数据边界 
	void filterReferenceBoundaries();

	bool isReferenceData(const QString & filePath) const;

	// 获取所有原始数据多边形
	QVector<QPolygonF> getOriginalPolygons() const;

	// 判断单个参考数据边界是否需要绘制 
	bool shouldDrawReference(const GeoBoundary& boundary, const QVector<QPolygonF>& originalPolygons) const;

	void loadPoints(QHash<QString, EncryptedPoint> encryptedPoints,
		QHash<QString, ControlPoint> controlPoints,
		const QString& flag);
	//void loadPoints(const QStringList& filePathList, const QStringList filenames, const QString flag, const QList<QString> orderedPointIds);
	void showPoints(bool show);
	void removePointById(QString pointId, const QString flag);
	void removePointsByFilePath(const QString& filePath, const QString& flag);
	void highlightPointById(QString pointId, const QString& flag, bool highlight = true);
	void highlightPointsById(const QSet<QString>& pointIds, const QString& flag, bool highlight);
	void clearPointHighlights();

	// 连接点显示/隐藏函数 
	void setConPointsVisible(bool visible, QString pointId = "-1");
	bool isConPointVisible(QString pointId) const;
	void showAllConPoints();
	void hideAllConPoints();

	// 控制点显示/隐藏函数 
	void setColPointsVisible(bool visible, QString pointId = "-1");
	bool isColPointVisible(QString pointId) const;
	void showAllColPoints();
	void hideAllColPoints();

	// 统一管理函数 
	void setAllPointsVisible(bool visible);

	QPointF pixelToWorld(const QPointF& pixelPos, QString& imageName, const QStringList filenames);
	bool transformCoordinates(OGRSpatialReference* sourceSRS, OGRSpatialReference* targetSRS, double& x, double& y);
	void reprojectBoundary(GeoBoundary& boundary, OGRSpatialReference* targetSRS);
	OGRSpatialReference* createTargetSRS();

	void highlightBoundary(const QString& filePath, bool highlight = true);
	void highlightsBoundary(const QStringList& filePath, bool highlight = true);
	void clearHighlights();
	QPointF screenToWorld(const QPoint &screenPos) const;

	// 交互控制函数 
	void zoomIn();
	void zoomOut();
	void zoomToFullExtent();
	void pan(double dx, double dy);
	void calculateWorldExtent();

	void setImageTypeFilter(const QString& type);
	void setSelectionMode(bool enabled);
	void clearSelection();
	const QSet<QString>& getSelectedFiles() const { return m_selectedFiles; }
	void drawSelectionRect();
	QSet<QString> m_selectedFiles;

	void setPointSelectionMode(bool enabled);
	void setPointTypeFilter(const QString& type);
	const QSet<QString>& getSelectedConPoints() const { return m_selectedConPoints; }
	const QSet<QString>& getSelectedColPoints() const { return m_selectedColPoints; }
	void clearPointSelection();

	QStringList getExcludedReferences() const;
	QSet<QString> getExcludedReferencesSet() const;
	QHash<QRgb, QStringList> getRetainReferencesSet() const;
	void setVisiblePointIds(const QSet<QString>& ids);
	void setOnlyShowVisibleIds(bool enable);

	// 存储每张图像的地理变换参数 
	QMap<QString, double*> m_geoTransforms;
	double m_worldMinX, m_worldMaxX, m_worldMinY, m_worldMaxY;

signals:
	void filesSelected(const QStringList& files);
	void pointsSelected(QSet<QString>& m_selectedConPoints, QSet<QString>& m_selectedColPoints);

protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;

private:
	IMAGEPS* m_imagePS = nullptr;
	struct PointVisibility {
		bool allVisible = true;
		QSet<QString> hiddenIds;
	};
	GDALDataset *currentDataset;
	// 视图变换参数 
	double m_viewCenterX = 0, m_viewCenterY = 0;
	double m_viewScale = 1.0;
	bool m_isPanning = false;
	QPoint m_lastPanPos;

	// 选择框相关变量 
	QRect m_selectionRect;
	bool m_isSelecting = false;
	QPoint m_selectionStart;
	bool m_useMiddleButtonForPan = false;

	// 影像类型过滤 
	QString m_currentImageTypeFilter = "所有类型";

	// 点显示状态管理 
	PointVisibility m_conVisibility;
	PointVisibility m_colVisibility;

	QVector<Point> m_conPoints;
	QVector<Point> m_colPoints;
	bool m_showPoints = false;
	QMap<QString, int> m_conpointIdToIndex;
	QMap<int, QString> m_colpointIdToIndex;
	QSet<QString> m_highlightedconPointIds;
	QSet<QString> m_highlightedcolPointIds;
	QVector<GeoImage> m_images;

	QSet<QString> m_highlightedFiles;
	float m_highlightWidth = 3.0f;
	QColor m_highlightColor = Qt::yellow;

	// 点选择相关变量 
	bool m_isPointSelecting = false;
	QSet<QString> m_selectedConPoints;
	QSet<QString> m_selectedColPoints;
	QString m_currentPointTypeFilter = "all";

	void drawBoundary(const GeoBoundary &boundary);
	void drawPoints();
	QPoint worldToScreen(const QPointF &worldPos) const;
	void updateViewTransform();
	QString formatGeoCoordinate(double x, double y);
	std::vector<QString> getRotatedImageCorners(const QString& imagePath, const QString& rpcPath = "");
};

#endif // IMAGEINFOSHOW_H