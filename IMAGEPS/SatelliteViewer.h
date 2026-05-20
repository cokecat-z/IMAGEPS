#ifndef SATELLITEVIEWER_H 
#define SATELLITEVIEWER_H 

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include "gdal_priv.h"

//struct ImageData {
//	QPixmap pixmap;
//	double minX = 0, maxX = 0, minY = 0, maxY = 0;
//	GDALDataType dataType = GDT_Unknown;
//	bool isSingleBand = true;
//	int currentOverviewLevel = 0;
//};

class SatelliteViewer : public QGraphicsView {
	Q_OBJECT
public:
	explicit SatelliteViewer(QWidget* parent = nullptr);
	~SatelliteViewer();

//	void loadMultispectralImage(const QString& path);
//	void loadPanchromaticImage(const QString& path);
//	void loadImage(const QString& path);
//	QPointF screenToWorld(const QPoint& screenPos) const;
//
//protected:
//	void wheelEvent(QWheelEvent* event) override;
//	void mousePressEvent(QMouseEvent* event) override;
//	void mouseMoveEvent(QMouseEvent* event) override;
//	void mouseReleaseEvent(QMouseEvent* event) override;
//	void resizeEvent(QResizeEvent* event) override;
//
//private:
//	QGraphicsScene* m_scene;
//	QGraphicsPixmapItem* m_pixmapItem;
//	QVector<ImageData> m_images;
//	QString m_currentImagePath;
//
//	// 视图控制 
//	QPointF m_viewCenter;
//	float m_viewScale = 1.0f;
//	QPoint m_lastDragPos;
//	QPointF m_dragStartViewCenter;
//
//	// 数据处理方法 
//	QImage readOverview(int overviewLevel, const QRect& region = QRect());
//	void updateImageForCurrentLevel();
//	int calculateOverviewLevel(float viewScale) const;
//	void clampViewCenter();
//	void updateViewTransform();
//
//	// 图像处理方法 
//	QImage processMultispectralBlock(GDALDataset* dataset, int overviewLevel,
//		int x, int y, int width, int height);
//	QImage processPanchromaticBlock(GDALRasterBand* band,
//		int x, int y, int width, int height);
//	QImage processFloat32Block(GDALRasterBand* band,
//		int x, int y, int width, int height);
//	QImage processSingleBand(GDALRasterBand* band,
//		int x, int y, int width, int height);
//	QImage processSingleBand16(GDALRasterBand* band,
//		int x, int y, int width, int height);
//	QImage process8BitMultiBand(GDALRasterBand* bands[3], const QRect& region);
//	QImage process16BitMultiBand(GDALRasterBand* bands[3], const QRect& region);
//	QImage processMultiBand(GDALDataset* dataset,
//		int overviewLevel,
//		const QRect& region);
//	QImage ensureRGBOrder(const QImage& inputImage, GDALDataset* dataset);
};

#endif // SATELLITEVIEWER_H 