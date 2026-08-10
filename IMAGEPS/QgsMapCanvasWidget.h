#ifndef QGSMAPCANVASWIDGET_H
#define QGSMAPCANVASWIDGET_H

#include <QWidget>
#include <QProgressDialog>
#include <QFutureWatcher>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QList>
#include <limits>

// QGIS classes - forward declarations to avoid Qt Designer crash
class QgsMapCanvas;
class QgsMapToolPan;
class QgsMapToolZoom;
class QgsRasterLayer;
class QgsRasterDataProvider;
class QgsRasterRenderer;
class QgsSingleBandGrayRenderer;
class QgsMultiBandColorRenderer;
class QgsContrastEnhancement;
class QgsRasterTransparency;
class QgsRectangle;
class QgsVectorLayer;
class QgsFeature;
class QgsGeometry;
class QgsSymbol;
class QgsRenderer;
class QgsLayerTree;
class QgsLayerTreeModel;
class QgsLayerTreeLayer;
class QgsProject;
class QgsCoordinateReferenceSystem;
class QgsCoordinateTransform;
class QgsMarkerSymbol;
class QgsLineSymbol;
class QgsFillSymbol;
class QgsSingleSymbolRenderer;
class QgsMapLayer;



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

    // Vector layer functions
    bool addVectorLayer(const QString& filePath);
    bool removeVectorLayer(QgsVectorLayer* layer);
    void clearVectorLayers();
    QList<QgsVectorLayer*> vectorLayers() const { return m_vectorLayers; }
    bool hasVectorLayers() const { return !m_vectorLayers.isEmpty(); }
    void setVectorLayerVisibility(QgsVectorLayer* layer, bool visible);
    void setVectorLayerOpacity(QgsVectorLayer* layer, double opacity);
    void setLayerOrder(bool rasterOnTop);
    bool checkSpatialReferenceMatch(QgsVectorLayer* vectorLayer) const;

    // Layer switch functions
    bool hasRasterLayer() const;
    QString getRasterLayerName() const;
    QStringList getVectorLayerNames() const;
    bool switchToLayer(const QString& layerName);
    bool toggleLayerVisibility(const QString& layerName);
    QList<QPair<QString, bool>> getAllLayerStatus() const;

signals:
    void imageLoaded(bool success, const QString& message);
    void overviewBuildFinished(bool success);
    void loadingProgress(int percent, const QString& message);
    void loadingStarted(const QString& message);
    void loadingFinished(bool success, const QString& message);
    void vectorLayerAdded(QgsVectorLayer* layer, bool success);
    void vectorLayerRemoved(int count);

private slots:
    void onOverviewBuildFinished();
    void onVectorLayerVisibilityChanged(bool visible);

private:
    // QGIS components
    QgsMapCanvas* m_mapCanvas;
    QgsMapToolPan* m_panTool;
    QgsMapToolZoom* m_zoomInTool;
    QgsMapToolZoom* m_zoomOutTool;
    QgsLayerTree* m_layerTree;
    QgsLayerTreeModel* m_layerTreeModel;

    // Current raster layer
    QgsRasterLayer* m_currentRasterLayer;
    QString m_currentImagePath;
    bool m_rasterLayerVisible;  // 影像图层可见性

    // Vector layers list
    QList<QgsVectorLayer*> m_vectorLayers;
    QMap<QString, bool> m_vectorLayerVisibility;
    int m_vectorLayerColorIndex;  // 用于循环分配不同颜色

    // Progress dialog for overview building
    QProgressDialog* m_progressDialog;
    QFutureWatcher<bool>* m_overviewWatcher;
    bool m_buildingOverviews;

    // Track declined files to prevent repeated prompts
    QString m_lastDeclinedFile;

    // Loading state
    bool m_loadingImage;

    // Layer order flag (true = raster on top, false = vector on top)
    bool m_rasterOnTop;

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

    // Vector layer helper functions
    void configureVectorRenderer(QgsVectorLayer* layer);
    void updateMapCanvasLayers();
    QString analyzeCrsCompatibility(const QgsCoordinateReferenceSystem& rasterCrs, const QgsCoordinateReferenceSystem& vectorCrs) const;
    bool transformVectorLayer(QgsVectorLayer* vectorLayer, const QgsCoordinateReferenceSystem& sourceCrs, const QgsCoordinateReferenceSystem& targetCrs);
};

#endif // QGSMAPCANVASWIDGET_H