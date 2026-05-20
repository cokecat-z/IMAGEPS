#ifndef QGSMAPCANVASWIDGET_H
#define QGSMAPCANVASWIDGET_H

#include <QWidget>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QMessageBox>
#include <limits>

// QGIS headers
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

// GDAL headers
#include "gdal_priv.h"
#include "cpl_progress.h"

class QgsMapCanvasWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QgsMapCanvasWidget(QWidget* parent = nullptr);
    ~QgsMapCanvasWidget();

    void loadImage(const QString& filePath);
    bool loadImageWithOverviewCheck(const QString& filePath);
    void clearImage();
    bool hasImageLoaded() const { return m_currentRasterLayer != nullptr; }
    QString currentImagePath() const { return m_currentImagePath; }

signals:
    void imageLoaded(bool success, const QString& message);
    void overviewBuildFinished(bool success);

private slots:
    void onOverviewBuildFinished();

private:
    // QGIS components
    QgsMapCanvas* m_mapCanvas;
    QgsMapToolPan* m_panTool;
    QgsMapToolZoom* m_zoomInTool;
    QgsMapToolZoom* m_zoomOutTool;

    // Current raster layer
    QgsRasterLayer* m_currentRasterLayer;
    QString m_currentImagePath;

    // Progress dialog for overview building
    QProgressDialog* m_progressDialog;
    QFutureWatcher<bool>* m_overviewWatcher;
    bool m_buildingOverviews;

    // Track declined files to prevent repeated prompts
    QString m_lastDeclinedFile;

    // Loading state
    bool m_loadingImage;

    // Helper functions
    void initCanvas();
    bool checkAndBuildOverviews(const QString& filePath);
    bool hasExistingOverviews(const QString& filePath);
    static bool checkHasExistingOverviews(const QString& filePath);
    bool buildExternalOverviews(const QString& filePath);
    static bool buildExternalOverviewsStatic(const QString& filePath, QWidget* parent);
    static int gdalProgressCallback(double dfComplete, const char* pszMessage, void* pProgressArg);
    static int gdalProgressCallbackStatic(double dfComplete, const char* pszMessage, void* pProgressArg);
    void addRasterLayer(const QString& filePath);
    void configureRasterRenderer(QgsRasterLayer* layer);
    void configureSingleBandRenderer(QgsRasterLayer* layer, QgsRasterDataProvider* provider);
    void configureMultiBandRenderer(QgsRasterLayer* layer, QgsRasterDataProvider* provider, int bandCount);
    QgsContrastEnhancement* createContrastEnhancement(QgsRasterDataProvider* provider, int band);
    QgsRasterTransparency* createRasterTransparency(QgsRasterDataProvider* provider, int bandCount);
    Q_INVOKABLE void loadImageAsync(const QString& filePath);
};

#endif // QGSMAPCANVASWIDGET_H