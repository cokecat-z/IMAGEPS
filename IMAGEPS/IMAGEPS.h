#ifndef IMAGEPS_H
#define IMAGEPS_H

#include <QtWidgets/QMainWindow>
#include "ui_imageps.h"
#include <QFileDialog>
#include <QFormLayout>
#include <QMessageBox>
#include <QSettings>
#include <QTextCodec>
#include <QMultiMap>
#include <iostream>
#include <QPainter>
#include <QPixmap>
#include <QImage>
#include <QFont>
#include <QScrollBar>
#include <QDesktopServices>
#include <qtoolbox.h>
#include <QDir>
#include <QDirIterator>
#include <QVector>
#include <QDebug>

// GDAL classes - forward declarations to avoid Qt Designer crash
class GDALDataset;
#include <QFileInfo>
#include <QGuiApplication>
#include <QScreen>
#include <QToolTip>
#include <QThreadPool>
#include <QProgressBar>
#include <QWidgetAction>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QtConcurrent/QtConcurrentRun>
#include <QShortcut>
#include <QReadWriteLock>
#include <QWidget>

#include "PublicFunctions.h"
#include "SystemConfig.h"
#include "DataModelConfig.h"
#include "measurementareashow.h"
#include "GLDisplayWidget.h"
#include "SimpleToolS.h"
#include "SetOrthoInfo.h"
#include "ImageInfoShow.h"
#include "PublicStruct.h"
#include "NonEditableModel.h"
#include "logger.h"
#include "QgsMapCanvasWidget.h"
#include "TabbedDockWidget.h"


enum ImageCorner {
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_RIGHT,
	BOTTOM_LEFT
};

struct GeoCoordinates {
	double Eresolution, Nresolution;
	double upperLeftLat, upperLeftLon;
	double upperRightLat, upperRightLon;
	double lowerRightLat, lowerRightLon;
	double lowerLeftLat, lowerLeftLon;
};

struct XMLParseConfig {
	QString xmlSuffix;
	QStringList fieldNames;
};

// 配置文件数据结构 
struct SatelliteConfig {
	QString prefix;      // 星源前缀（如GF1、WV01）
	QString xmlSuffix;   // XML文件后缀
	QString timeTag;     // 时间标签名
};

struct ImageGeoMetadata {
	QString filePath;
	char** rpcMetadata;
	double geoTransform[6];
	double Corners[4];
	QString projection;
	QString flag;
};
//Q_DECLARE_METATYPE(ImageGeoMetadata)
class IMAGEPS : public QMainWindow
{
	Q_OBJECT

public:
	IMAGEPS(QWidget *parent = nullptr);
	~IMAGEPS();
	QString projectdir;//工程文件目录
	QString connectPointsMatchmarkerFile; //连接点文件标记
	QString colPointsMatchmarkerFile; //控制点文件标记
	QString DataModelmarkerFile; //模型文件标记
	QString appDirPath;
	Logger CurrentConfig;
	QList<QString> colPointIds;
	QList<QString> colPointIds_out;
	QList<QString> m_visibleControlPointIds;      // 当前可见的控制点ID列表
	QList<QString> m_visibleControlPointIds_conBack;      // 当前可见的控制点ID列表

	//static IMAGEPS* instance;
	QString getInstanceId() const { return m_instanceId.toString(); }
	Ui::IMAGEPSClass ui;

	QMap<QString, bool> tmpcontrolPointsPath; //加载工程使用
	bool tmpcontrolPointsflag; //加载工程使用
	QMap<QString, bool> tmpimageInterPath; //加载工程使用
	bool tmpimageInterflag; //加载工程使用

		// 配置相关成员变量
	QString m_configFilePath;
	QStringList m_rpcFileExtensions;    // RPC文件扩展名列表
	QStringList m_imageFileExtensions;  // 图像文件扩展名列表
	QStringList m_tilFileExtensions;      // TIL文件扩展名列表 
	QStringList m_pyramidFileExtensions;  // 金字塔文件扩展名列表
	QStringList m_cldFileExtensions;  // 文件夹过滤字段列表

	//static QMap<QString, ImageGeoMetadata> s_imageMetadata;

	//static ImageGeoMetadata* getImageMetadata(const QString& filePath);

	QMap<QString, ImageGeoMetadata> s_imageMetadata;
	QProgressDialog* m_imageLoadingProgressDialog;

	ImageGeoMetadata* getImageMetadata(const QString& filePath);
	void setupLogToFileWriter(QTextEdit* logEdit, const QString& filePath, int maxLines = 1000, QObject* parent = nullptr);
	void EncryptedPointsList();
	void ControlPointsList();
	void Con_ColtrolPointsList();
	void EncreadResidualData();
	void ConResidualFiles();

	void onExportEncryptedPointsClicked();
	void inExportEncryptedPointsClicked();
	void onExportControlPointsClicked();
	void inExportControlPointsClicked();
	bool showConfirmationDialog(const QString &message);
	void connectAllModificationSignals();
	QMap<QString, bool>  getcolfileValue();
	void setView(TabbedDockWidget* DockWidget, QTabWidget* tableWidget, int currentindex);
	void setView_close(TabbedDockWidget* DockWidget, QTabWidget* tableWidget, int currentindex);
	void displayView(TabbedDockWidget* DockWidget, QTabWidget* tableWidget, int currentindex);

	QDateTime readTimestampFromXml(const QString& nodeName);
	QString getProjectDir(const QString& fullPath);

	QMap<QString, bool> controlPointsPath; //控制点输出文件路径
	QMap<QString, bool> modelMatchByInterPath; //模型配准输出文件路径
	QMap<QString, bool> AbsPositPrecCheckPathPath; //绝对定位质检输出文件路径
	QPointF geoToScreen(double lon, double lat,
		double minLon, double maxLon,
		double minLat, double maxLat,
		int screenWidth, int screenHeight);
	QProgressBar* getProgressBar() { return ui.progressBar; };
	QProgressBar* getProgressBar_imageinfo() { return ui.progressBar_imageinfo; };
	QStringList getSelectFilePath(QString flag) const;
	DataModelConfig* getDataModelConfig() const;
	SystemConfig* getSystemConfig() const;
	bool getWorkProcessflag() const;
	void setWorkProcessflag(bool flag);
	//bool getDataModelPath(const QString& filePath) const;
		// 获取原始数据路径列表
	const QStringList& getDataModelPath() const;

	// 获取DOM参考数据路径列表 
	const QStringList& getDOMFilePath() const;

	// 获取DEM参考数据路径列表 
	const QStringList& getDEMFilePath() const;

	// 获取智能镶嵌数据路径列表 
	QStringList& getSmartMosaicFilePath();
	bool getIsCtrlIMG(QString filename, int col) const;
	void showChineseScrollBarContextMenu(QScrollBar* scrollBar, const QPoint& pos);
	
	void updateVisiblePointsFromTables();
	bool checkPyramidExists(const QFileInfo& fileInfo);

signals:
	void aboutToClose();
	void buildPSIntersectObjCmdFileFinished();

public slots:
	bool newProActionSlot();//新建工程
	bool openProActionSlot();//打开工程
	bool saveProActionSlot();//保存工程
	void loadRecentFiles();
	void updateRecentFiles(const QString& projectPath, const QString& projectPathname);
	//bool openRecentProject(const QString& projectPath, const QString& projectPathname);
	bool openRecentProject(const QString& projectPath_Recent);
	void systemSetSlot();//系统配置
	void dataModelConfigActionSlot();//数据模型配置
	void connectPointsMatchActionSlot();//连接点匹配
	void controlPointsMatchActionSlot();//控制点匹配
	void OrthoRectificationFusion_actionSlot();  //自动化流程一业务
	void FreeNetworkAdjustmentActionSlot();//自由网平差
	void controlNetAdjustmentActionSlot();//控制网网平差
	void orthorectificAtionSlot();  //正射纠正
	void imageInterActionSlot();  //影像融合
	void trueColorConveractionSlot();  //真彩色转换
	void DodgingactionSlot(); //影像匀色
	void ImageMosaicactionSlot(); //影像镶嵌
	void SmartMosaicactionSlot(); //智能镶嵌
	void allMapShow_actionSlot();//全图显示
	void amplify_actionSlot();//放大
	void narrow_actionSlot();//缩小
	void translation_actionSlot();//平移
	void onTabChanged(int index);
	void closeEvent(QCloseEvent *event);
	// Layer switch functions
	void onRasterOnTop();
	void onVectorOnTop();
	void onLayerSwitchComboChanged(int index);
	void onToggleLayerVisibility();
	void updateLayerSwitchCombo();
	QString extractLayerName(const QString& displayText);
	QString getFullLayerName(const QString& displayText);
	bool buildPSIntersectObjCmdFile();
	bool buildPSIntersectObjCmdFile(QString filePath, QStringList tieFilePattern);
	void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
	void onEncryptedPointSelected(const QItemSelection &selected, const QItemSelection &deselected);
	void onControlPointSelected(const QItemSelection &selected, const QItemSelection &deselected);

	void referDataList_TabWContextSlot(const QPoint &pos);//参考数据列表右键
	void sateImageDataList_TabWContextSlot(const QPoint &pos);//卫星影像数据列表右键
	void PyramidDataList_TabWContextSlot(const QPoint & pos);//金字塔数据列表右键
	void ImageInterList_TabWContextSlot(const QPoint & pos);//真彩色转换列表右键
	void DodgingList_TabWContextSlot(const QPoint & pos);//影像匀色列表右键
	void ImageMosaicList_TabWContextSlot(const QPoint & pos);//影像镶嵌列表右键
	void QuickMosaicDataList_TabWContextSlot(const QPoint & pos);//快速镶嵌列表右键
	void AlignmentAdjustmentList_TabWContextSlot(const QPoint & pos);//配准纠正列表右键
	void FusionmodelsrcList_TabWContextSlot(const QPoint & pos);//影像融合列表右键
	void FusionmodelList_TabWContextSlot(const QPoint & pos);//影像融合列表右键
	void AlignmentIntsrcList_TabWContextSlot(const QPoint & pos);//配准融合列表右键
	void AlignmentIntList_TabWContextSlot(const QPoint & pos);//配准融合列表右键
	void SmartMosaicDataList_TabWContextSlot(const QPoint & pos);//智能镶嵌列表右键
	void MosaicCropDataList_TabWContextSlot(const QPoint & pos);//智能镶嵌列表右键
	void ImagecropDataList_TabWContextSlot(const QPoint & pos);//影像裁切数据列表右键
	void VectorfileDataList_TabWContextSlot(const QPoint & pos);//裁切矢量数据列表右键
	void AbsPositPrecCheckList_TabWContextSlot(const QPoint & pos);//绝对定位精度质检列表右键
	void RelPositPrecCheckList_TabWContextSlot(const QPoint & pos);//相对定位精度质检列表右键
	void measureAreaShow_GLWContextSlot(const QPoint &pos);//测区显示右键
	void encryptPointsList_TabWContextSlot(const QPoint &pos);//加密点列表右键
	void controlPointsLIst_TabWContextSlot(const QPoint &pos);//控制点列表右键
	void CloudDetectionDataList_TabWContextSlot(const QPoint &pos);//遥感影像云检数据列表列表右键
	void CloudDetectionLIst_TabWContextSlot(const QPoint &pos);//云检成果列表右键
	void ProTransformationDataList_TabWContextSlot(const QPoint &pos);//投影转换数据列表右键
	void FormatConversionDataList_TabWContextSlot(const QPoint &pos);//格式转换数据列表右键
	void SARImageFilteringDataList_TabWContextSlot(const QPoint &pos);//SAR影像滤波数据列表右键
	void ShpFilteringDataList_TabWContextSlot(const QPoint &pos);//Shp数据列表右键
	void imageHandleBus_TreWContextSlot(const QPoint &pos);//影像处理业务右键
	void imageHandleBus_TreWSlot(QTreeWidgetItem *item, int column);//影像处理业务左键
	void pointsInfoList_DockWContextSlot(const QPoint &pos);//点信息列表右键
	void attributeView_actionSLOT(int row, int column); //属性视图

	void AbsPositPrecCheckPList(); //相对定位精度质检列表
	void CloudDetectionList(); //云检成果列表数据写入

	void killAllProcesses(bool flag = true);  // 杀死所有进程的槽函数

	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void onComboBoxIndexChanged(int index);
	void onComboBox_2IndexChanged(int index);

	// 完整关闭影像显示
	void onImageCloseRequested();

private:
	QIcon createScaledIcon(const QString& path, const QSize& size = QSize(24, 24));
	void initWidget();
	void connects();

	void showWorld();
	void closeWorldShow();
	void clearCentralWidget();
	void clearTableWithHeader(QTableWidget* tableWidget, bool keepColumnWidth);

	void paintEvent(QPaintEvent* event);
	bool eventFilter(QObject *watched, QEvent *event) override;
	void setupAllChineseScrollBars();

    void on_actionOpenImage(QStringList filenames, const QString flag);
	void on_actionOpenImage(const QStringList& existingFiles, const QStringList& newFiles, const QString flag);
    void on_actionOpenImageShow(QString filenamePATH);
    void on_actionOpenVectorShow(QString filenamePATH);
    bool setGeoreferenceFromXMLAndRPC(const QStringList filenamePATH);
	bool setGeoreferenceFromXMLAndRPC(const QStringList& existingFiles, const QStringList& newFiles);
    bool copyAndRenameRpcFiles(QStringList filenamesPath);
    QString getProjectFolderPath(const QString& projectPath);
    QString GetDatasetProjection(GDALDataset* dataset, const QString& prjFilePath);
    void FusionBuildmodel(QStringList filenamelist, const QString& tableName, const QString& modeldir);
    void Fusiondeletemodel(QStringList FilePath, const QString& tableName);

	void inline setImage(const QImage &image)
	{
		m_image = image;
		update(); // 触发paintEvent
	}

	void inline clearImage()
	{
		m_image = QImage();
		update(); // 触发paintEvent
	}

	//GeoCoordinates parseGeoXML(const QString& xmlPath);



	void calculateGeoBounds(const GeoCoordinates& coords,
		double& minLon, double& maxLon,
		double& minLat, double& maxLat);
	void ReadXMLresult();

	void loadXMLConfigs(); // 加载配置文件
	GeoCoordinates parseGeoXML(const QString& xmlPath, const XMLParseConfig& config); // 使用配置解析XML
	GeoCoordinates parseGeoXMLBJ3(const QString& xmlPath, const XMLParseConfig& config); // 使用配置解析XML

	//保存工程辅助函数
	bool writeControlPointXml(const QString& projectFolder);
	bool writeSatImageXml(const QString& projectFolder);
	void writeCreatePydImageXml();
	void writeOutByteImageXml();
	void writeDodgingImageXml();
	void writeMosaicImageXml();
	void writeAbsPositPrecCheckImageXml();
	void writeCLDImageImageXml();
	void loadCreatePydImageData();
	void loadOutByteImageData();
	void loadDodgingImageData();
	void loadMosaicImageData();
	void loadAbsPositPrecCheckImageData();
	void loadCLDImageData();
	bool writeRegisterRectifyImageXml(const QString& projectFolder);
	bool loadRegisterRectifyImageData(const QString& projectFolder);
	bool writeDataToFusionImage(const QString& fusionImagePath,
		const QString& satModelMakerCmdoutPath,
		const QStringList& alignmentIntFilePath,
		QTableWidget* alignmentTable,
		QTableWidget* fusionModelTable);
	bool writeSmartMosaicImage(const QString& fusionImagePath,
		const QStringList& alignmentIntFilePath,
		const QStringList& ShpFilePath,
		QTableWidget* alignmentTable,
		QTableWidget* fusionModelTable);
	bool readDataFromFusionImage(const QString& fusionImagePath,
		QTableWidget* fusionModelTable);
	bool readSatImagesToAlignmentTable(const QString& fusionImagePath,
		QTableWidget* alignmentTable, QStringList& satImagePath);
	bool readSmartMosaicImage(const QString& fusionImagePath,
		QStringList& alignmentIntFilePath,
		QStringList& ShpFilePath,
		QTableWidget* alignmentTable,
		QTableWidget* ShpFilePathTable);
	void writeImageXml(QTableWidget* tableWidget, const QStringList& filePathList, const QString& xmlFileName);
	void loadImageData(QTableWidget* tableWidget, QStringList& filePathList, const QString& xmlFileName);

    bool createTaskFile(const QString& inputImagePaths);
    bool processImageWithOperator(const QStringList& filenames);
	QPair<QStringList, QStringList> processImageWithOperator_test(const QStringList & filenames);
    void sortNumericColumn(QTableWidget *table, int column, Qt::SortOrder order);
    void sortNumericColumn(QTableView* tableView, int column, Qt::SortOrder order);
	void sortNumericColumn_SmartM(QTableWidget * table, int column, Qt::SortOrder order);
    void productionconfiguration(); //打开生产配置界面
    void setUiEnabled(QWidget* window, bool enabled);

	void loadOriginalImagesFromFiles(QTableWidget* tableWidget, QStringList& filePathList);
	void loadOriginalImagesFromFolder(QTableWidget* tableWidget, QStringList& filePathList);
	void unloadImages(QTableWidget* tableWidget, QStringList& filePathList);
	void viewImages(QTableWidget* tableWidget, const QStringList& filePathList);
	void clearImageDisplayIfNeeded(const QSet<QString>& imageNames, bool isVector = false);
	bool checkFileExistsInTable(QTableWidget* tableWidget, const QString& fileName);
	void addFileToTable(QTableWidget* tableWidget, const QFileInfo& fileInfo);
	void highlightSelectedBoundaries(QTableWidget* tableWidget);
	QString getImagetype(const QString& filePath);
	QStringList filterImageFiles(const QStringList& filePaths, QString title = NULL);
	void loadFileToTextEdit(const QString &filePath, QTextEdit *textEdit);
	void setSelectedRowsValue(int column, const QString& value);
	void loadencListData();
	void loadcolListData();
	void asyncUpdateEncData(const QSet<QString>& selectedIDs, QString flag);
	void asyncUpdateColData(const QSet<QString>& selectedIDs, QString flag);

	void updateVisiblePoints();
	void loadCurrentPage();

	void loadControlPointCurrentPage();
	void moveSelectedRows(QTableWidget * table, int direction);
	void swapTableRows(QTableWidget * table, int row1, int row2);

	QVector<SatelliteConfig> loadSatelliteConfigs();

	QString extractImageTime(const QString & imagePath, const QString & satelliteName);

	QStringList syncFileOrderWithTable(QStringList & filePaths, QTableWidget * table);

	// 从XML文件读取加密点 
	bool readEncryptedPointsFromXml(const QString& filePath);
	bool writeEncryptedPointsToXml(const QString& filePath);

	bool readControlPointsFromXml(const QString& filePath);
	bool writeControlPointsToXml(const QString& filePath);
	// 配置相关函数 
	void loadConfiguration();
	void saveConfiguration();
	QStringList getConfigStringList(const QString& section, const QString& key, const QStringList& defaultValue);

private:
	QUuid m_instanceId;
    QImage m_image;
    QString projectPath;//工程文件路径
    QString parentDir; //原始影像绝对路径
    QStringList CreatepyramidPath;//创建金字塔文件路径
    QStringList DataModelPath;//模型创建文件路径|卫星参考数据文件路径
    QStringList DOMFilePath;//DOM文件路径
    QStringList DEMFilePath;//DEM文件路径
    QStringList ImageInterFilePath;//真彩色转化文件路径
    QStringList AlignmentAdFilePath;//配准纠正文件路径
    QStringList FusionmodelFilePath;//影像融合文件路径
    QStringList AlignmentIntFilePath;//配准融合文件路径
    QStringList SmartMosaicFilePath;//智能镶嵌文件路径
    QStringList MosaicCropFilePath;//智能镶嵌裁切范围文件路径
    QStringList DodgingFilePath;//影像匀色文件路径
    QStringList ImageMosaicFilePath;//影像镶嵌文件路径
    QStringList QuickMosaicFilePath;//快速镶嵌文件路径
    QStringList ImagecropFilePath;//影像裁切文件路径
    QStringList VectorfileFilePath;//裁切矢量文件路径
    QStringList AbsPositPrecCheckFilePath;//绝对定位精度质检文件路径
    QStringList RelPositPrecCheckFilePath;//相对定位精度质检文件路径
    QStringList CloudDetectionDataListFilePath;//云检数据文件路径
    QStringList CloudDetectionListFilePath;//云检成果文件路径
    QStringList ProTransformationFilePath;//投影转换文件路径
    QStringList FormatConversionFilePath;//格式转换文件路径
    QStringList SARImageFilteringFilePath;//SAR影像滤波数据文件路径
    QStringList ShpFilteringFilePath;//Shp滤波数据文件路径
    QStringList VectorDataFilePath;//矢量数据文件路径

	QStringList TmpDataModelPath;//模型创建文件路径|卫星参考数据文件路径
	//QStringList TmpDOMFilePath;//DOM文件路径
	//QStringList TmpDEMFilePath;//DEM文件路径

	QMap<QString, QAction*> Map_mainToolBarAction;//主菜单->菜单子项
	SystemConfig* systemConfig = nullptr;
	DataModelConfig* dataModelConfig = nullptr;
	MeasurementareaShow* Measurementarea = nullptr;
	SimpleToolS* SimpleToolSWidget = nullptr;
    SetOrthoInfo* SetOrthoInfoWidget = nullptr;
	QMetaObject::Connection m_encryptedPointsConnection;
	QMetaObject::Connection m_connection; //连接变量
	QHash<QString, int> DataimageListMap;
	bool OrthoRectificationFusionflag;
	QMap<QString, XMLParseConfig> xmlConfigs; // 存储不同前缀的配置
	QString m_lastPath = "C:"; //记录上次路径
	bool closeflag;
	bool m_isModified = false;  // 标记是否有未保存的更改
	bool m_ImageselectionAll = false;  // 影像选择(卫星影像和参考影像)
	QSettings* settings;

	QAtomicInt m_processedCount;
	struct TaskData {
		QString filePath;
		int index;
		QColor color;
		QString moniFile;
	};
	QVector<TaskData> m_taskData;
	QList<QProcess*> m_childProcesses;  // 存储所有子进程指针
	QList<QList<QVariant>> m_encListData; // 存储加密点表格数据 
	QList<QList<QVariant>> m_colListData; // 存储控制点表格数据 

	NonEditableModel *encryptPointsModel;
	NonEditableModel *ControlPointsModel;

	// 使用哈希表存储加密点数据，键为点ID 
	QHash<QString, EncryptedPoint> m_encryptedPoints;
	// 用于懒加载的分页数据 
	QList<QString> m_visiblePointIds; // 当前可见的点ID列表 
	int m_currentPage = 0;
	const int PAGE_SIZE = 1000000; // 每页显示1000行 

	QHash<QString, ControlPoint> m_controlPoints; // 存储所有控制点数据
	QHash<QString, ControlPoint> m_controlPoints_conBack; // 存储icp控制点数据备份
	int m_currentControlPointPage = 0;            // 当前控制点页码 
	const int CONTROL_POINT_PAGE_SIZE = 1000000;     // 每页显示的控制点数 

    QMetaObject::Connection buildPSIntersectObjCmdFileConnection;

	QColor m_currentColor;
	bool m_imageRangeSignalConnected = false;
	QHash<QRgb, QStringList> m_pendingFiles;

	bool m_isProcessingImageOpen = false;

	QVector<SatelliteConfig> SATELLITE_CONFIGS;

	QString originalOkText;
};

#endif
