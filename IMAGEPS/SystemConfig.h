#ifndef SYSTEMCONFIG_H
#define SYSTEMCONFIG_H

#include "ui_SystemConfig.h"
#include <QDialog>
#include <QStackedWidget>
#include <QScrollArea>
#include <QFileDialog>
#include <QDomDocument>
#include <QFile>
#include <QPointer>
#include <QProcess>
#include <qtextedit.h>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QLockFile>
#include <QUuid>
#include <QtConcurrent>
#include <gdal_priv.h>   
#include <ogr_spatialref.h>   
#include <QQueue>
#include <QRadioButton>
#include <QClipboard>
#include "ProjectSetting.h"
#include "PublicFunctions.h"
#include "DataModelConfig.h"
#include "SetOrthoInfo.h"

class IMAGEPS;

struct ImageDatacol {
	QString imagePath;
	QStringList demPaths;  // 存储DEM文件中的所有路径 
	QStringList satPaths;  // 存储SAT文件中的所有路径
};

class SystemConfig : public QDialog
{
	Q_OBJECT

public:
	explicit SystemConfig(IMAGEPS* parentImagePS = nullptr, QWidget* parent = nullptr);
	explicit SystemConfig(QDialog *parent = nullptr);
	//SystemConfig(QDialog *parent = nullptr);

	~SystemConfig();

	void setImagePSInstance(IMAGEPS* instance);

	void loadConfig(QString funfunModule = QString::fromLocal8Bit("系统配置功能列表"));
	void initWidget();
	void setProjectdir();

	void settingShowWidget(QString title); 
	void FreeNetworkAdjust(QStringList DEMReferencefilename);
	void controlNetAdjust(QStringList DEMReferencefilename);

	int showFunctionAndWait(const QString& functionName); //模块运行前设置
	bool addWeakTextureNode(const QString& xmlFilePath, bool WeakTexturePS,
		const QStringList& DOMReferencefilename);
	void acceptModule(QString title, QStringList filename);
	void connectPointsMatch(QStringList DEMReferencefilename, QStringList DOMReferencefilename, const QString xmldir);//连接点匹配
	void IntersectCmd(QStringList Referencefilename, QStringList SatelliteImagefilename, QString str, QString OutputPath, bool enableLogging = false); //PSIntersectCmd任务单及算子调用
	void colIntersectCmd(QStringList DOMReferencefilename, QStringList DEMReferencefilename, QStringList SatelliteImagefilename, QString OutputPath, bool enableLogging);
	bool readAndValidatePathsFile(const QString & filePath, QStringList & paths);
	QVector<ImageDatacol> colparallelProcessSatelliteImages(const QStringList & satelliteImagePaths, const QString & precisionCheckDir);
	void controlPointsMatch(QStringList DEMReferencefilename, QStringList DOMReferencefilename, QStringList SatelliteImagefilename, QMap<QString, bool>& controlPointsPath);//控制点匹配
	void modelMatchByInter(QStringList DEMReferencefilename, QStringList DOMReferencefilename, QStringList SatelliteImagefilename, QMap<QString, bool>& cmodelMatchByIntePath);//模型配准
	void controlNetAdjustmergeFile(QMap<QString, bool> controlPointsPath);//控制网平差
	void FreeNetworkAdjustmergeFile(); //自由网平差
	void FreeNetworkAdjustmergeFile(QString filePath, QStringList tieFilePattern); //自由网平差
	void orthorectification(QStringList DEMReferencefilename, QStringList SatelliteImagefilename); //正射纠正
	void AlignmentAdjustment(QStringList DEMReferencefilename, QStringList SatelliteImagefilename); //正射纠正
	void imageInterAction(); //影像融合
	void TrueColorConversion(QStringList imagePath); //真彩色转换
	void ImageColorCorrection(QStringList imagePath); //影像匀色
	void ImageMosaic(QStringList imagePath); //影像镶嵌
	void QuickMosaic(QStringList imagePath); //快速镶嵌
	void SmartMosaic(QStringList imagePath, QStringList CropFilePath); //智能镶嵌
	void ExcuteExetopo(QString mosaicListFile, QString mosaicToopfile,int GenLineOnlyisNo = 0); //智能镶嵌
	void ExcuteExeMuchXQMosaicX(QStringList imagePath, QString mosaicToopfile); //智能镶嵌(分幅)
	void ExcuteExeCutVector(QString imagePath, QString OutputDir, QString attriID); //裁切矢量
	void ImageAbsPositPrecCheck(QStringList DEMReferencefilename, QStringList DOMReferencefilename, QStringList SatelliteImagefilename, QMap<QString, bool>& AbsPositPrecCheckPath); //绝对定位精度质检
	void ImageCropping(QStringList imagePath, QStringList polyShpPath, QString OutputDir); //影像裁切
	void SARImageFiltering(QStringList imagePath);
	void ProjectionTransformation(QStringList imagePath); //投影转换
	void FormatConversion(QStringList imagePath); //格式转换
	void ThumbnailGeneration(QStringList imagePath); //生成缩略图
	void CloudDetection();//影像云检
	void Imageprojection(QStringList imagePath); //生成缩略图
	void OrthoRectificationFusion(QStringList DEMReferencefilename, QStringList DOMReferencefilename, QStringList SatelliteImagefilename);

	QString calculateTimeDifference(const QDateTime &start, const QDateTime &end);
	QString getTiffImageType(const QString &filePath);
	bool writeMapToXml(const QMap<QString, bool>& mapData, const QString& mapType);
	bool readMapFromXml(QMap<QString, bool>& mapData, const QString& mapType);
	QStringList createDodgingControlFile(
		const QStringList& inputImagePath,
		const QString& DodgingTemplateLibDir
	);
	void createFreeNetworkDodgingPlusFile(
		const QStringList& inputImagePaths,
		const QString& DodgingTemplateLibDir);
	QString  processCurrentImage(int index, const QStringList& inputImagePaths, const QString& DodgingTemplateLibDir);

	QStringList createMaskDodgingFile(
		const QStringList& inputImagePaths,
		const QString& DodgingTemplateLibDir);

	QStringList createFreeNetworkDodgingFile(
		const QStringList& inputImagePaths,
		const QString& DodgingTemplateLibDir);

	QString createCalDodgingCoefFile(
		const QStringList& imagePaths);

	bool createTaskParallelManagerFile(
		const QString& softwareDir,
		const QString& jobType,
		const QStringList& tasks,
		const QString& outputPath);
	bool eventFilter(QObject *obj, QEvent *event) override;
	QString getGlobalShpFileValue() const;
	bool getGlobalDEM() const;
	bool getprogressBarInitialized();
	QStringList getThumbnailGenerationFilePath(QString flag) const;

	void onExportTrueColorSettings();

	void addXmlElement(QDomDocument & doc, QDomElement & parent, const QString & name, const QString & value, const QString & type = NULL);

	void onImportTrueColorSettings();

	QString getXmlValue(const QDomElement & parent, const QString & name);

	bool deleteFilesInAllSubfolders(const QString &folderPath);

	void writeGlobalShpFileValue(QString filePath) const;

	QStringList filterSatelliteImagesByColumn(const QStringList& fileList, int col) const;
	void terminateAllProcesses();  // 终止所有进程 

	void executeMainMatching(const QStringList& SatelliteImagefilename,
		QMap<QString, bool>& controlPointsPath);
	void exemodelMatchByInter(const QStringList& SatelliteImagefilename,
		QMap<QString, bool>& cmodelMatchByIntePath);

	// 流程执行控制 
	void executeCustomProcess(const QStringList& selectedProcesses, const QStringList& inputFiles);
	void executeNextProcess();

	QTextEdit * logEdit;

	bool ConvertTo8Bitflag = false;
	Ui::SystemConfig ui;
	int colcurrentIndex;

	struct TaskInfo {
		QString taskFilePath;
		QString operatorName;
		QString outfilename;
		QString taskFilePath2;
		QString taskFilePath3;
	};
	QQueue<TaskInfo> m_taskQueue;
	QQueue<TaskInfo> m_taskQueue_col;
	QAtomicInt m_runningProcesses;
	QAtomicInt m_completedTasks;
	QDateTime m_startTime;
	int m_totalTasks = 0;

	QQueue<TaskInfo> m_taskQueueIR;
	QAtomicInt m_runningProcessesIR;
	QAtomicInt m_completedTasksIR;
	QDateTime m_startTimeIR;
	int m_totalTasksIR = 0;

private:
	void connects();
	void updateWidgetStates();
	void setWidgetBackground(QWidget *widget, const QString &imagePath);
	

public slots:
	void setPROJCStextSlot(QString PROJCStext);
	void okButtonSlot();
	void cancelButtonSlot();
	void applyButtonSlot();
	void closeEvent(QCloseEvent *event);
	void on_pushButton_clicked();
	void on_pushButton_2_clicked();
	void on_pushButton_3_clicked();
	void on_toolButton_clicked();
	void on_toolButton_2_clicked();
	void on_toolButton_3_clicked();
    void setupScrollAreaLabels();
    //void closeEvent(QCloseEvent *event);
    void handleLabelRightClick(QLabel *label, const QPoint &pos);
    void handlewidgetRightClick(QWidget *label, const QPoint &pos);
	void startNextTasks(QString title, bool enableLogging = true);
	void startNextTaskcols(QString title);

	void startNextTasksIRange(QString title, bool enableLogging);

	void move_to_end()
	{
		if(logEdit)
			logEdit->moveCursor(QTextCursor::End);
	}
	void itemSlot(QMap<QString, bool>& itemfunModuleBool);
	void itemswitchSlot(QListWidgetItem* item);
	// 自定义流程相关槽函数 
	void handleBusinessButtonClicked();
	void handleStartProcessClicked();
	void handlePathSettingClicked();
	void handleStopProcessClicked();
	void showContextMenu(const QPoint& pos);
	void deleteProcessingNode();
	void savePathSettings();

	void onCurrentProcessFinished(); // 当前流程完成槽 

signals:
	void CreatepyramidFinished(); //金字塔创建信号
	void BuildSatModelFinished(); //模型构建信号
	void intersectCmdFinished(); //模型配准信号
	void colintersectCmdFinished(); //控制点匹配模型配准信号
	void EncryptedPointsMatchFinished(); //连接点匹配信号
	void controlPointsMatchFinished(); //控制点匹配信号
	void modelMatchByInterFinished(); //模型配准信号
	void FreeNetworkAdjustFinished(); //自由网平差信号
	void controlNetAdjustFinished(); //控制网平差信号
	void orthorectificationFinished(); //正射纠正信号
	void AlignmentAdjustmentFinished(); //正射纠正信号
	void imageInterActionFinished(); //影像融合信号
	void TrueColorConversionFinished(); //真彩色转换信号
	void ImageColorCorrectionFinished(); //影像匀色信号
	void ImageMosaicFinished(); //影像镶嵌信号
	void SmartMosaicFinished(); //智能镶嵌信号
	void CreateTopologyFinished(); //生成拓扑文件信号
	void ExcuteExeCutVectorFinished(); //裁切矢量信号
	void ImageAbsPositPrecCheckFinished(); //绝对定位精度质检信号
	void ThumbnailGenerationFinished(); //生成缩略图信号
	void CloudDetectionFinished(); //云检信号
	void ProjectionTransformationFinished(); //投影转换信号
	void ImageCropFinished(); //影像裁切信号
	void SARImageFilterFinished(); //SAR影像滤波信号
	void ImageRangeFinished(); //影像范围计算信号
	void FormatConversionFinished(); //格式转换信号
	void FunctionAbnormalExit(QString title); //函数非正常退出信号
	void allIntersectCmdsFinished(); //IntersectCmds信号

	void processSequenceFinished(); // 整个流程完成信号
	void currentProcessProgress(int value); // 当前流程进度信号

private:
	//struct TaskInfo {
	//	QString taskFilePath;
	//	QString operatorName;
	//	QString outfilename;
	//	QString taskFilePath2;
	//	QString taskFilePath3;
	//};
	//QQueue<TaskInfo> m_taskQueue;
	//QQueue<TaskInfo> m_taskQueue_col;
	//QAtomicInt m_runningProcesses;
	//QAtomicInt m_completedTasks;
	//QDateTime m_startTime;
	//int m_totalTasks = 0;
	int m_maxConcurrentProcesses = 4; // 根据CPU核心数调整并发数
	bool m_progressBarInitialized = false;//进度条初始化标识
	bool m_progressBarInitializedIR = false;//进度条初始化标识
	
	int m_coltotalTasks = 0;

	QMetaObject::Connection m_intersectCmdConnection;
	QMetaObject::Connection m_allintersectCmdConnection;
	
	QString projectdir;//工程文件目录
	QMap<QString, bool> imageInterPath; //影像融合输出文件路径
	QStringList IntersectCmdPaths;    //模型配准输出文件路径
	QStringList ThumbnailGenerationPaths; //影像云检缩略图输出文件路径
	QStringList CloudDetectionOutPaths; //影像云检缩略图输出文件路径
	QStringList m_orthoSatelliteImages; 

	QList<QStringList> funListConifg;//系统配置列表
	QMap<QString, QList<QStringList>> funModuleTemp;//系统配置功能临时列表
	QMap<QString, bool> funModuleBool;//系统配置功能模块控制 避免重复点击反复读取
	ProjectSetting  projectSetting;
	DataModelConfig* dataModelConfig = nullptr;
	SetOrthoInfo* SetOrthoInfoConfig = nullptr;
	IMAGEPS* m_imagePS;

	QMutex acceptModuleMutex;
	QMutex intersectCmdMutex;
	QMutex connectPointsMatchMutex;
	QMutex controlPointsMatchMutex;
	QMutex FreeNetworkAdjustMutex;
	QMutex controlNetAdjustMutex;
	QMutex orthorectificationMutex;
	QMutex imageInterActionMutex;
	QMutex TrueColorConversionMutex;
	QMutex ImageColorCorrectionMutex;
	QMutex ImageMosaicMutex;
	QMutex SmartMosaicMutex;
	QMutex ImageAbsPositPrecCheckMutex;
	QMutex ThumbnailGenerationMutex;
	QMutex CloudDetectionMutex;
	QMutex colintersectCmdMutex;

	QMutex ModelMutex;

	QMutex m_taskMutex;

	// 辅助函数，用于检查并获取锁 
	bool tryLockFunction(QMutex& mutex, const QString& functionName) {
		if (!mutex.tryLock()) {
			//qDebug() << functionName << "is already running, aborting";
			logEdit->append(QString::fromLocal8Bit("已有任务在执行,请稍后再试!!!"));
			QMessageBox::warning(this,
				QString::fromLocal8Bit("警告"),
				QString::fromLocal8Bit("已有任务正在运行中，请等待任务结束"));
			return false;
		}
		return true;
	}
	std::shared_ptr<QMutexLocker> tryLockFunction_test(QMutex& mutex, const QString& functionName) {
		if (!mutex.tryLock()) {
			logEdit->append(QString::fromLocal8Bit("已有任务在执行,请稍后再试!!!"));
			QMessageBox::warning(this,
				QString::fromLocal8Bit("警告"),
				QString::fromLocal8Bit("已有任务正在运行中，请等待任务结束"));
			return nullptr;
		}
		return std::make_shared<QMutexLocker>(&mutex);
	}

	//文件标记
	QString connectPointsMatchmarkerFile;
	QString connectPointsMatchTaskmarkerFile;
	QString FreeNetworkAdjustmarkerFile;
	QString controlNetAdjustmarkerFile;

	QStringList  FreeNetwork_PSDodging_plusXmlFiles;
	QStringList  ExcuteExeMuchXQMosaicXmlFiles;
	QStringList  ImageCroppXmlFiles;
	int m_currentControlPointsTask;

	int ImageCropcurrentIndex;
	QStringList ImageCroplocalInputPaths;
	int ImageCropId = 0;

	QVector<QList<QWidget*>> imageEventColor_eventLightGroups; //匀色设置
	QMap<QCheckBox*, QList<QWidget*>> imageEventColorcheckboxMap;
	bool m_isAboutToClose = false;
	
	bool m_flagMosaic = false;

	QMetaObject::Connection m_intersectConn;
	QMetaObject::Connection m_allFinishedConn;
	int m_connectionId = 0;
	bool m_isMatchingRunning = false;
	bool isFromFinishedSignal = false;
	bool isFromFinishedSignalIR = false;

	QList<QProcess*> m_systemProcesses;  // 存储SystemConfig启动的进程
	bool m_systemProcessesFlag = false;  // 存储SystemConfig启动的进程标志
	QList<QMetaObject::Connection> m_systemProcessesConn;
	QVector<QMetaObject::Connection> connections;

	// 保存配置到文件 
	void saveSettings();
	// 从文件加载配置 
	void loadSettings();

	QMovie *movie_left = nullptr;
	QMovie *movie_right = nullptr;
	QMovie *movie_down = nullptr;

	QSize sizeHint() const;

	// 流程相关成员 
	QStringList m_selectedProcesses;
	QStringList m_processInputFiles;
	int m_currentProcessIndex = -1;
	QQueue<QString> m_processQueue;

	// 自定义流程相关成员 
	QList<QPushButton*> m_selectedButtons; // 已选择的业务按钮 
	QList<QPushButton*> m_visibleButtons;  // 上方显示的按钮 
	QList<QFrame*> m_connectionLines;      // 连接线
	QDialog* m_pathSettingsDialog;         // 路径设置对话框 

	// 拖拽相关成员
	QPoint m_dragStartPosition;            // 拖拽起始位置
	QPushButton* m_draggedButton;          // 当前被拖拽的按钮
	QPushButton* m_contextMenuButton;      // 右键菜单关联的按钮 

	// 自定义流程相关方法
	void setupCustomWorkflowPage();
	void createPathSettingsDialog();
	void updateVisualConnections();

	QVector<ImageDatacol> m_colIntersectData; //控制点影像和对应DEM路径

	//静态变量修改
	int SmartMosaiccurrentIndex = 0;
	QStringList SmartMosaiclocalInputPaths;

	QMap<QString, bool> controlPointsPath_Or;
	int createFNDPlusFilecurrentIndex = 0;

	QStringList createFNDPluslocalInputPaths;
	QString createFNDPluslocalDodgingTemplateLibDir;


};

#endif