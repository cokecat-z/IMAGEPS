#ifndef GLDISPLAYWIDGET_H 
#define GLDISPLAYWIDGET_H 

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QVector>
#include <QMatrix4x4>
#include "gdal_priv.h"

struct ImageData {
	QOpenGLTexture* texture = nullptr;
	double minX = 0, maxX = 0, minY = 0, maxY = 0;
	GDALDataType dataType = GDT_Unknown;
	bool isSingleBand = true;
};

class IMAGEPS;

class GLDisplayWidget : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT
public:
	//explicit GLDisplayWidget(IMAGEPS* parentImagePS = nullptr, QWidget* parent = nullptr);

	explicit GLDisplayWidget(QWidget* parent = nullptr);
	~GLDisplayWidget();
	void setImagePS(IMAGEPS* imagePS);

	void loadMultispectralImage(const QString& path);
	void loadPanchromaticImage(const QString& path);
	void loadImage(const QString& path);
	QPointF screenToWorld(const QPoint& screenPos) const;

protected:
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;

	// 鼠标交互 
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
	IMAGEPS* m_imagePS = nullptr;
	QVector<ImageData> m_images;
	QString m_currentImagePath;

	// 视图控制 
	QPointF m_viewCenter;
	float m_viewScale = 1.0f;

	// 金字塔管理 
	QImage readOverview(int overviewLevel, const QRect& region = QRect());
	void updateTextureForCurrentLevel();
	int calculateOverviewLevel(float viewScale) const;
	void clampViewCenter();

	// 数据处理 
	QImage processMultispectralBlock(GDALDataset* dataset, int overviewLevel,
		int x, int y, int width, int height);
	QImage processPanchromaticBlock(GDALRasterBand* band,
		int x, int y, int width, int height);
	QImage processFloat32Block(GDALRasterBand* band,
		int x, int y,
		int width, int height);
	QImage processSingleBand(GDALRasterBand* band,
		int x, int y,
		int width, int height);
	QImage processSingleBand16(GDALRasterBand* band,
		int x, int y,
		int width, int height);
	QImage process8BitMultiBand(GDALRasterBand* bands[3], const QRect& region);
	QImage process16BitMultiBand(GDALRasterBand* bands[3], const QRect& region);
	QImage processMultiBand(GDALDataset* dataset,
		int overviewLevel,
		const QRect& region);
	QImage ensureRGBOrder(const QImage& inputImage, GDALDataset* dataset);

	// 交互状态 
	QPoint m_lastDragPos;
	QPointF m_dragStartViewCenter;
};

#endif // GLDISPLAYWIDGET_H 