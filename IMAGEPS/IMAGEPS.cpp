#include "IMAGEPS.h"

//IMAGEPS* this = nullptr;
//QMap<QString, ImageGeoMetadata> IMAGEPS::s_imageMetadata;

IMAGEPS::IMAGEPS(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowTitle(u8"IMAGEPS DESKTOP多源遥感影像智能处理系统 2026V2.6[*]");

	// 设置主窗口初始大小为屏幕的80%
	QScreen *screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->availableGeometry();
	int width = screenGeometry.width()  * 0.8;
	int height = screenGeometry.height()  * 0.8;
	resize(width, height);

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// 中心窗口 
	setGeometry(
		QStyle::alignedRect(
			Qt::LeftToRight,
			Qt::AlignCenter,
			this->size(),
			screen->availableGeometry()
		)
	);
	//instance = this;

	initWidget();
	this->installEventFilter(this);
	setupAllChineseScrollBars();
	connects();
	loadConfiguration();

	SATELLITE_CONFIGS = loadSatelliteConfigs();
	//loadRecentFiles();

	// 统一窗口居中函数 
	auto centerWindow = [](QWidget *window) {
		QScreen *screen = QGuiApplication::primaryScreen();
		QRect screenGeometry = screen->availableGeometry();
		window->move(screenGeometry.center() - window->rect().center());
	};

	if (!systemConfig)
	{
		//systemConfig = new SystemConfig;
		systemConfig = new SystemConfig(this, this);
		systemConfig->resize(1550, 900);
		//systemConfig->resize(1180, 580);
		centerWindow(systemConfig);
	}

	if (!dataModelConfig)
	{
		dataModelConfig = new DataModelConfig(this, this);
		dataModelConfig->resize(1550, 950);
		//dataModelConfig->resize(1180, 580);
		centerWindow(dataModelConfig);
	}	

	if (!SimpleToolSWidget)
	{
		SimpleToolSWidget = new SimpleToolS(this, this);
		centerWindow(SimpleToolSWidget);
	}	

	if (!SetOrthoInfoWidget)
	{
		SetOrthoInfoWidget = new SetOrthoInfo(this, this);
		//SetOrthoInfoWidget->ui.lineEdit_4->setText("CGCS20000");
		centerWindow(SetOrthoInfoWidget);
	}	
	OrthoRectificationFusionflag = false;
	closeflag = false;
	ui.ImageInfoShow_Widget->setImagePS(this);
	ui.ImageInfoShow_Widget->installEventFilter(this);
	ui.ImageInfoShow_Widget->setMouseTracking(true);
	ui.openGLWidget->setImagePS(this);
	ui.openGLWidget->installEventFilter(this);
	ui.openGLWidget->setMouseTracking(true);

	QString exeDir = QCoreApplication::applicationDirPath();
	QDir dir(exeDir);
	this->appDirPath = dir.absolutePath();
	//setupLogToFileWriter(ui.runLog_Edit, "../bin/config/application_log.txt", 100000, this);
	//setUiEnabled(this, false);

	//systemConfig->deleteFilesInAllSubfolders(u8"D:/CokeWork/VsProject/IMAGEPS/IMAGEPS/test_Project-副本");
}

IMAGEPS::~IMAGEPS()
{
	// 清理所有进程
	killAllProcesses(false);
}

void IMAGEPS::initWidget()
{
	//this->showMaximized();
	// 延迟执行最大化并强制布局更新
	//QTimer::singleShot(0, this, [this]() {
	//	this->showMaximized();
	//	this->menuBar()->setMinimumWidth(this->width());
	//	this->layout()->activate();
	//});
	settings = new QSettings("../bin/config/mainToolBarConfig/mainToolBar.ini", QSettings::IniFormat);
	settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

	QStringList List_menuBar;//一级菜单
	List_menuBar << "file_menu" << "look_menu" << "view_menu" << "dataPre_menu" << "imageMatch_menu"
		<< "localAdjust_menu" << "correctInter_menu" << "eventlyColor_menu" << "DSM_menu"
		<< "imageProcess_menu" << "imageProcessStream_action" << "tool_menu" << "help_menu";

	QMap<QString, QString> menuIconMap = {
	{"file_menu", QString::fromLocal8Bit(":/resource/menu/菜单/文件.png")},
	{"view_menu", QString::fromLocal8Bit(":/resource/menu/菜单/视图.png")},
	{"dataPre_menu", QString::fromLocal8Bit(":/resource/menu/菜单/数据预处理.png")},
	{"imageMatch_menu", QString::fromLocal8Bit(":/resource/menu/菜单/影像匹配.png")},
	{"localAdjust_menu", QString::fromLocal8Bit(":/resource/menu/菜单/区域网平差.png")},
	{"correctInter_menu", QString::fromLocal8Bit(":/resource/menu/菜单/纠正融合.png")},
	{"eventlyColor_menu", QString::fromLocal8Bit(":/resource/menu/菜单/匀色镶嵌.png")},
	{"imageProcessStream_menu", QString::fromLocal8Bit(":/resource/menu/菜单/影像处理业务流.png")},
	{"tool_menu", QString::fromLocal8Bit(":/resource/menu/菜单/工具.png")},
	{"help_menu", QString::fromLocal8Bit(":/resource/menu/菜单/帮助.png")}
	};
	QMap<QString, QString> childMenuIconMap = {
	{"imageCloudCheck_menu", QString::fromLocal8Bit(":/resource/menu/数据预处理/影像云检测.png")},
	{"imageAbsPositPrecCheck_menu", QString::fromLocal8Bit(":/resource/menu/数据预处理/影像绝对定位精度质检.png")},
	{"connectPointsFile_menu", QString::fromLocal8Bit(":/resource/menu/工具/连接点文件.png")},
	{"inputControlPoints_menu", QString::fromLocal8Bit(":/resource/menu/工具/导入控制点.png")},
	{"ImageCloudandAndStripingDeformationReplacement_menu", QString::fromLocal8Bit(":/resource/menu/工具/影像云雪和拉花变形替换.png")},
	{"ImageCropTool_menu", QString::fromLocal8Bit(":/resource/menu/工具/影像裁切软件.png")},
	{"HeightAnomalyProcessing_menu", QString::fromLocal8Bit(":/resource/menu/工具/高程异常处理.png")},
	{"Softwaretools_menu", QString::fromLocal8Bit(":/resource/menu/工具/sxsz2.png")},
	{"InvalidValueHandling_menu", QString::fromLocal8Bit(":/resource/menu/工具/无效值处理.png")},
	{"satelliteDataPre_menu", QString::fromLocal8Bit(":/resource/menu/gszh-icon.png")}
	};

	QMap<QString, QString> childActionIconMap = {
	{"genThumImageByDataPre_action", QString::fromLocal8Bit(":/resource/menu/子菜单/生成缩略影像.png")},
	{"cloudCheckBasicThumImage_action", QString::fromLocal8Bit(":/resource/menu/yxyjc-icon.png")},
	{"cloudResultLook_action", QString::fromLocal8Bit(":/resource/menu/子菜单/云检结果查看.png")},
	{"imageAbsPositPrecCheckBusDataOrg_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像绝对定位精度质检业务数据组织.png")},
	{"imageAbsPositPrecCheck_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像绝对定位精度质检.png")},
	{"exchangeFormatcontrolPoints_action", QString::fromLocal8Bit(":/resource/menu/子菜单/交换格式控制点.png")},
	{"inputConnectPointsFile_action", QString::fromLocal8Bit(":/resource/menu/子菜单/导入连接点文件.png")},
	{"outputConnectPointsFile_action", QString::fromLocal8Bit(":/resource/menu/子菜单/导出连接点文件.png")},
	{"ImageCloudandSnowReplacementInteractive_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像云雪自动替换软件(交互式).png")},
	{"ImageCloudandSnowReplacementBatchProcessing_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像云宣自动替换软件(批处理).png")},
	{"ImageCloudandSnowReplacementFullyAutomatic_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像云雪自动替换软件(全自动).png")},
	{"ImageCropTool_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像常规战切.png")},
	{"ImageCutTool_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像多模式裁切.png")},
	{"Threedimensionalmodelcutting_action", QString::fromLocal8Bit(":/resource/menu/工具/立体模型裁切.png")},
	{"HeightAnomalyElimination_action", QString::fromLocal8Bit(":/resource/menu/子菜单/高程异常消除.png")},
	{"HeightAnomalyDetection_action", QString::fromLocal8Bit(":/resource/menu/子菜单/高程异常查找.png")},
	{"DSMDEMLogicalConsistencyProcessing_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM逻辑一致性处理.png")},
	{"ParallelDSM-DEMLogicalConsistencyProcessing_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM逻辑一致性处理(并行).png")},
	{"DSMDEMStandardizationCheck_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM标准化检查.png")},
	{"DSMDEMEdgeMatchingCheck_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM接边检查.png")},
	{"DEMRelatedPropertyCheck_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DEM相关性质检工具.png")},
	{"Invalidvaluereplacement_action", QString::fromLocal8Bit(":/resource/menu/工具/无效值替换.png")},
	{"ImageZeroValueProcessing_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像零值处理.png")},
	{"ImageZerovalueProcessings_action", QString::fromLocal8Bit(":/resource/menu/工具/影像零值处理(白点)并行版.png")},
	{"Invalidvaluelookup_action", QString::fromLocal8Bit(":/resource/menu/工具/无效值查找.png")},
	};

	for (auto iter : List_menuBar)
	{
		if (settings->value("mainToolBarSetting/" + iter + "Enable").toBool())
		{
			QMenu* tempMenu = nullptr;
			if (iter != "imageProcessStream_action") {
				tempMenu = ui.menuBar->addMenu(settings->value("mainToolBarSetting/" + iter + "Name").toString());
			}
			//// 设置图标
			//if (menuIconMap.contains(iter)) {
			//	//QIcon icon;
			//	//QPixmap pixmap(menuIconMap[iter]);
			//	//
			//	//icon.addPixmap(pixmap.scaled(QSize(1024, 1024), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
			//	//tempMenu->setIcon(icon);
			//	tempMenu->setIcon(QIcon(menuIconMap[iter]));
			//}

			QStringList List_tempMenuActions;//一级菜单下按钮
			QMap<QString, QStringList> Map_tempChildMenus;//二级菜单
			if (iter == "file_menu")
			{
				List_tempMenuActions << "newPro_action" << "openPro_action" << "savePro_action" << "backupsPro_action" << "closePro_action"
					<< "systemSet_action" << "shutdownTask_action" << "openProDir_action"  << "quit_action";
			}
			else if (iter == "look_menu")
			{
				List_tempMenuActions << "equalShow_action" << "allMapShow_action" << "amplify_action" << "narrow_action" << "translation_action";
			}
			else if (iter == "view_menu")
			{
				List_tempMenuActions << "satelliteImageView_action" << "referenceDataView_action"<< "TestAreaDisplayView_action" << "encryptionPointsView_action" << "pointInforView_action"
					<< "controlPointsView_action" << "attributeView_action" << "logView_action" << "statusBar_action" << "imageProcessTool_action";
			}
			else if (iter == "dataPre_menu")
			{
				List_tempMenuActions << "fileDecompre_action" << "satelliteDataPre_menu" << "imageEqualProject_action" << "imageCloudCheck_menu"
					<< "referenceDataCreateInput_action" << "referenceDataExtract_action" << "imageAbsPositPrecCheck_menu" << "imageRelPositPrecCheck_menu" << "threeDModelElevatPrecCheck_menu" << "threeDModelBorderPrecCheck_menu";

				Map_tempChildMenus["satelliteDataPre_menu"] << "PleiadesToJP2_action" << "PleiadesToTIF_action" << "GeoEye_action" << "RAWDataGen_action" << "SpotModelDataCorr_action"
					<< "BeiJing3Tool_action" << "ShaoBing2Tool_action" << "LandSatTool_action" << "ZhuHai1Tool_action" << "ModisToolDataExtract_action" << "ModisToolInaly_action"
					<< "radarSlcTool_action" << "RDModelRadarDataIn_action" << "TH05Tool_action" << "Sentinel_1_GBMTool_action";

				Map_tempChildMenus["imageCloudCheck_menu"] << "genThumImageByDataPre_action" << "cloudCheckBasicThumImage_action" << "cloudResultLook_action";

				//Map_tempChildMenus["ImageQualityInspection_menu"] << "imageAbsPositPrecCheck_menu" << "imageRelPositPrecCheck_menu" << "threeDModelElevatPrecCheck_menu" << "threeDModelBorderPrecCheck_menu";
				
				Map_tempChildMenus["imageAbsPositPrecCheck_menu"] << "imageAbsPositPrecCheckBusDataOrg_action" << "imageAbsPositPrecCheck_action";
				Map_tempChildMenus["imageRelPositPrecCheck_menu"] << "imageRelPositPrecCheckBusDataOrg_action" << "imageRelPositPrecCheck_action";
				Map_tempChildMenus["threeDModelElevatPrecCheck_menu"] << "threeDModelElevatPrecCheckBusDataOrg_action" << "threeDModelElevatPrecCheck_action";
				Map_tempChildMenus["threeDModelBorderPrecCheck_menu"] << "threeDModelBorderPrecCheckBusDataOrg_action" << "threeDModelBorderPrecCheck_action";

			}
			else if (iter == "imageMatch_menu")
			{
				List_tempMenuActions << "dataModelConfig_action" << "connectPointsMatch_action" << "controlPointsMatch_action" << "inputControlPoints_menu" << "outputControlPoints_action" << "imageControlPointsFile_menu"
					<< "connectPointsFile_menu";
				Map_tempChildMenus["inputControlPoints_menu"] << "standardFormatcontrolPointsToObject_action" << "standardFormatcontrolPointsToImage_action" << "exchangeFormatcontrolPoints_action";

				Map_tempChildMenus["imageControlPointsFile_menu"] << "inputImageControlPointsFile_action" << "outputImageControlPointsFile_action";

				Map_tempChildMenus["connectPointsFile_menu"] << "inputConnectPointsFile_action" << "outputConnectPointsFile_action";
			}
			else if (iter == "localAdjust_menu")
			{
				List_tempMenuActions << "freedomNetAdjustCal_action" << "controlNetAdjustCal_action" << "controlPointsMatch_action";
			}
			else if (iter == "correctInter_menu")
			{
				List_tempMenuActions << "normalIncidenceCorrect_action" << "imageInter_action";
			}
			else if (iter == "eventlyColor_menu")
			{
				List_tempMenuActions << "trueColorConver_action" << "imageEventlyColor_action" << "imageinlay_action" << "intelligentInlay_action";
			}
			else if (iter == "DSM_menu")
			{
				List_tempMenuActions << "DSMMatch_action" << "pointCloudGrid_action" << "DSMResultRefine_action" << "DSMAutoGenerate_action";
			}
			else if (iter == "imageProcess_menu")
			{
				List_tempMenuActions << "imageProcessTool_action" << "pyramidCreate_menu" << "imageInter_menu" << "matchCorrect_menu"
					<< "matchInter_menu" << "trueColorConver_menu" << "imageEventlyColor_menu" << "imageInlay_menu" << "imageCut_menu"
					<< "intelligentInlay_menu" << "imageDefogging_menu" << "bandRestructure_menu" << "imageEnhance_menu" << "projectChange_menu"
					<< "formatChange_menu" << "imageCloudCheck_menu" << "imageFilter_menu" << "DEMInlay_menu" << "radiationPosition_menu"
					<< "atmosphereCorrection_menu" << "DOMAdjustCorrect_menu" << "imageAbsPositPrecCheck_menu" << "imageRelPositPrecCheck_menu"
					<< "threeDModelElevatPrecCheck_menu" << "threeDModelBorderPrecCheck_menu";

				Map_tempChildMenus["pyramidCreate_menu"] << "pyramidCreateBusDataOrg_action" << "pyramidCreate_action";

				Map_tempChildMenus["imageInter_menu"] << "imageInterBusDataOrg_action" << "imageInte_action";

				Map_tempChildMenus["matchCorrect_menu"] << "matchCorrectBusDataOrg_action" << "modelMatchByCorrect_action" << "imageCorrect_action";

				Map_tempChildMenus["matchInter_menu"] << "matchInterBusDataOrg_action" << "modelMatchByInter_action" << "imageInter_action";

				Map_tempChildMenus["trueColorConver_menu"] << "trueColorConverBusDataOrg_action" << "trueColorConver_action";

				Map_tempChildMenus["imageEventlyColor_menu"] << "imageEventlyColorBusDataOrg_action" << "imageEventlyColor_action";

				Map_tempChildMenus["imageInlay_menu"] << "imageInlayBusDataOrg_action" << "imageInlay_action";

				Map_tempChildMenus["imageCut_menu"] << "imageCutBusDataOrg_action" << "imageCut_action";

				Map_tempChildMenus["intelligentInlay_menu"] << "intelligentInlayBusDataOrg_action" << "intelligentInlay_action";

				Map_tempChildMenus["imageDefogging_menu"] << "imageDefoggingBusDataOrg_action" << "imageDefogging_action";

				Map_tempChildMenus["bandRestructure_menu"] << "bandRestructureBusDataOrg_action" << "bandRestructure_action";

				Map_tempChildMenus["imageEnhance_menu"] << "imageEnhanceBusDataOrg_action" << "imageEnhance_action";

				Map_tempChildMenus["projectChange_menu"] << "projectChangeBusDataOrg_action" << "projectChange_action";

				Map_tempChildMenus["formatChange_menu"] << "formatChangeBusDataOrg_action" << "formatChange_action";

				Map_tempChildMenus["imageCloudCheck_menu"] << "imageCloudCheckBusDataOrg_action" << "genThumImageByImageProcess_action" << "imageCloudCheck_action";

				Map_tempChildMenus["imageFilter_menu"] << "imageFilterBusDataOrg_action" << "imageFilter_action" << "SarImageFilterBusDataOrg_action" << "SarImageFilter_action";

				Map_tempChildMenus["DEMInlay_menu"] << "DEMInlayBusDataOrg_action" << "DEMInlay_action";

				Map_tempChildMenus["radiationPosition_menu"] << "radiationPositionBusDataOrg_action" << "radiationPosition_action";

				Map_tempChildMenus["atmosphereCorrection_menu"] << "atmosphereCorrectionBusDataOrg_action" << "atmosphereCorrection_action";

				Map_tempChildMenus["DOMAdjustCorrect_menu"] << "DOMAdjustCorrectBusDataOrg_action" << "DOMConnectMatch_action" << "DOMLocalAdjust_action" << "DOMSecondCorrect_action" << "allAutoDOMSecondCorrect_action";

				Map_tempChildMenus["imageAbsPositPrecCheck_menu"] << "imageAbsPositPrecCheckBusDataOrg_action" << "imageAbsPositPrecCheck_action";

				Map_tempChildMenus["imageRelPositPrecCheck_menu"] << "imageRelPositPrecCheckBusDataOrg_action" << "imageRelPositPrecCheck_action";

				Map_tempChildMenus["threeDModelElevatPrecCheck_menu"] << "threeDModelElevatPrecCheckBusDataOrg_action" << "threeDModelElevatPrecCheck_action";

				Map_tempChildMenus["threeDModelBorderPrecCheck_menu"] << "threeDModelBorderPrecCheckBusDataOrg_action" << "threeDModelBorderPrecCheck_action";
			}
			else if (iter == "imageProcessStream_action")
			{
				QAction* action = new QAction(this);
				action->setObjectName(settings->value("mainToolBarSetting/" + iter + "Name").toString());
				action->setText(settings->value("mainToolBarSetting/" + iter + "Name").toString()); // 去除后缀显示 

				// 连接动作触发信号 
				connect(action, &QAction::triggered, this, &IMAGEPS::productionconfiguration);

				// 添加到菜单栏 
				menuBar()->addAction(action);
				//menuBar()->addAction(settings->value("mainToolBarSetting/" + iter + "Name").toString());

				List_tempMenuActions << "eventlyColorInalyProStreamDataOrg_action" << "eventlyColorInaly_action" << "imageProcessProStreamDataOrg_action" << "imageProcessAuto2_action" << "oneButtonDataProcessAuto1_action" ;
			}
			else if (iter == "tool_menu")
			{
				List_tempMenuActions << "Buildpyramid_action" << "imageContrastAnalysis_action" << "imageMatchCorrect_action" << "epipolarRayImageProc_action" << "controlPointprojectChange_action"
					<< "matchResultBackups_menu" << "threeDModelRecover_action" << "pointPositionVacuate_menu" << "loadVectorRange_menu" << "unloadVectorRange_action" << "seniorTool_menu"
					<< "satelliteImagePreview_menu" << "shutdownTask_action" << "residualShow_menu" << "imageDeformityCorrect_action" << "windowsColonyService_action"
					<< "windowsColonyPreStateQuery_action" << "ImageCloudandAndStripingDeformationReplacement_menu" << "DSMDEMInteractiveEditing_action" << "projectChange_action" << "formatChange_action" << "Rpb2Rpc_action" << "RpcToRpb_action"
					<< "Softwaretools_menu" << "RemoteSensingImageDecompression_action" << "ImageResampling_action" << "ImageRangeProduction_action" << "MapExtentGeneration_action" << "GeoRectifyPlatform_action"
					<< "DEMMosaic_action" << "ImageCoordinateProcessingTool_action" << "HeightAnomalyProcessing_menu" << "NoDataAttribute_action" << "GeoDataExtractorTool_action"
					<< "WindowsClusterService_action" << "InvalidValueHandling_menu";

				Map_tempChildMenus["matchResultBackups_menu"] << "controlPointsMatchResultBackups_action" << "connectPointsMatchResultBackups_action";

				Map_tempChildMenus["Softwaretools_menu"] << "ImageFilterTool_action" <<"BandSplitTool_action" << "BandOrganizeTool_action" << "BandCompositeTool_action" << "BandMathXTool_action" << "ImageCropTool_menu"
					<< "Orthorectification_action" << "ImageFusionSoftware_action" << "TrueColorConversionSoftware_action" << "ImageColorCorrectionSoftware_action"
					<< "ImageMosaicSoftware_action";

				//Map_tempChildMenus["inputControlPoints_menu"] << "standardFormatcontrolPointsToObject_action" << "standardFormatcontrolPointsToImage_action" << "exchangeFormatcontrolPoints_action";

				//Map_tempChildMenus["imageControlPointsFile_menu"] << "inputImageControlPointsFile_action" << "outputImageControlPointsFile_action";

				//Map_tempChildMenus["connectPointsFile_menu"] << "inputConnectPointsFile_action" << "outputConnectPointsFile_action";

				Map_tempChildMenus["pointPositionVacuate_menu"] << "controlPointsVacuate_action" << "connectPointsVacuate_action";

				Map_tempChildMenus["loadVectorRange_menu"] << "loadVectorFile_action" << "adminDivisionWorld_action" << "adminDivisionWorldChina_action" << "adminDivisionWorldChinaProvince_action";

				Map_tempChildMenus["seniorTool_menu"] << "inputConnectPointsMatchResult_action" << "inputAdjustResult_action" << "inputLaserMeasureHighData_action" << "generateMeasureAreaModelKMLFile_action"
					<< "generateMeasureAreaModelSHPFile_action" << "inputGXLConnectPoints_action" << "inputGXLControlPoints_action" << "inputCloudTubeSceneWithinConnectPoints_action"
					<< "inputCloudTubeSceneBetweenConnectPoints_action" << "outputConnectPointsImageError_action" << "connectPointsMatchResultRefine_action" << "outputConnectPointsMatchColoud_action";

				Map_tempChildMenus["satelliteImagePreview_menu"] << "previewGenerate_action" << "previewLoad_action" << "previewDownload_action";

				Map_tempChildMenus["residualShow_menu"] << "imageSideResidual_action" << "objectSideResidual_action" << "imageResidualList_action";

				Map_tempChildMenus["ImageCloudandAndStripingDeformationReplacement_menu"] << "ImageCloudandSnowReplacementInteractive_action" << "ImageCloudandSnowReplacementBatchProcessing_action" << "ImageCloudandSnowReplacementFullyAutomatic_action";

				Map_tempChildMenus["ImageCropTool_menu"] << "ImageCropTool_action" << "ImageCutTool_action" << "Threedimensionalmodelcutting_action";

				Map_tempChildMenus["HeightAnomalyProcessing_menu"] << "HeightAnomalyElimination_action" << "HeightAnomalyDetection_action" << "DSMDEMLogicalConsistencyProcessing_action"
					<< "ParallelDSM-DEMLogicalConsistencyProcessing_action" << "DSMDEMStandardizationCheck_action" << "DSMDEMEdgeMatchingCheck_action" << "DEMRelatedPropertyCheck_action";
				
				Map_tempChildMenus["InvalidValueHandling_menu"] << "Invalidvaluereplacement_action" << "ImageZeroValueProcessing_action" << "ImageZerovalueProcessings_action" << "Invalidvaluelookup_action";
			}
			else if (iter == "help_menu")
			{
				List_tempMenuActions << "widgetStyle_menu" << "remotePermissionUpgrade_menu" << "remotePermissionUpgradeHaspDog_action" << "test_action" << "about_action" << "useManual_action";

				Map_tempChildMenus["widgetStyle_menu"] << "defaultStyle_action" << "prepareStyle1_action" << "prepareStyle2_action";

				Map_tempChildMenus["remotePermissionUpgrade_menu"] << "remotePermissionUpgradeSuperDog_action" << "remotePermissionUpgradeHaspDog_action";
			}

			for (auto actionIter : List_tempMenuActions)
			{
				if (settings->value(iter + "Setting/" + actionIter + "Enable").toBool())
				{
					if (Map_tempChildMenus.find(actionIter) == Map_tempChildMenus.end())
						Map_mainToolBarAction[actionIter] = tempMenu->addAction(settings->value(iter + "Setting/" + actionIter + "Name").toString());
					else
					{
						QMenu* childTempMenu = tempMenu->addMenu(settings->value(iter + "Setting/" + actionIter + "Name").toString());

						if (childMenuIconMap.contains(actionIter)) {
							QPixmap pixmap(childMenuIconMap[actionIter]);
							childTempMenu->setIcon(QIcon(pixmap.scaled(32, 32, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
						}

						for (auto childIter : Map_tempChildMenus[actionIter])
						{
							if (settings->value(actionIter + "Setting/" + childIter + "Enable").toBool())
							{
								//Map_mainToolBarAction[childIter] = childTempMenu->addAction(settings->value(actionIter + "Setting/" + childIter + "Name").toString());
								QAction* childAction = childTempMenu->addAction(
									settings->value(actionIter + "Setting/" + childIter + "Name").toString()
								);
								Map_mainToolBarAction[childIter] = childAction;

								// 设置子项图标（如果存在）
								if (childActionIconMap.contains(childIter)) {
									QPixmap childPixmap(childActionIconMap[childIter]);
									if (!childPixmap.isNull()) {
										childPixmap = childPixmap.scaled(QSize(24, 24), Qt::KeepAspectRatio, Qt::SmoothTransformation);
										childAction->setIcon(QIcon(childPixmap));
									}
								}
							
							}
						}
					}
				}

			}
			List_tempMenuActions.clear();
			Map_tempChildMenus.clear();
		}
	}

	//根据系统配置使能添加影像处理业务页
	QList<QPair<QString, QList<QStringList>>> config = PublicFunctions::loadConfigFileL(QString::fromLocal8Bit("../bin/config/mainWindowConfig/影像处理业务.csv"));

	ui.treeWidget->setColumnCount(1);

	// 正向遍历，保持读取顺序
	for (const auto& item : config) {
		QStringList keys = item.first.split(",");
		if (keys.size() >= 2 && keys[1] == "TRUE") {
			QTreeWidgetItem* treeItem = new QTreeWidgetItem;
			treeItem->setText(0, keys[0]);
			ui.treeWidget->addTopLevelItem(treeItem);

			// 添加子项 
			for (const auto& data : item.second) {
				if (data.size() >= 2 && data[1] == "TRUE") {
					QTreeWidgetItem* childTreeItem = new QTreeWidgetItem;
					childTreeItem->setText(0, data[0]);
					treeItem->addChild(childTreeItem);
				}
			}
		}
	}

	closeflag = false;
	//ui.measureAreaShow_GLW->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.ImageInfoShow_Widget->setContextMenuPolicy(Qt::CustomContextMenu);
	//ui.Image_show->setContextMenuPolicy(Qt::CustomContextMenu);
	ui.openGLWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	//文件
	Map_mainToolBarAction["savePro_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/文件/保存工程.png")));
	//Map_mainToolBarAction["openPro_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/文件/打开工程.png")));
	Map_mainToolBarAction["openProDir_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/文件/打开工程文件夹.png")));
	Map_mainToolBarAction["quit_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/文件/退出.png")));
	//Map_mainToolBarAction["systemSet_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/文件/系统设置.png")));
	//Map_mainToolBarAction["newPro_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/文件/新建工程.png")));

	//视图
	Map_mainToolBarAction["referenceDataView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/视图/参考数据列表视图.png")));
	Map_mainToolBarAction["pointInforView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/视图/点信息列表视图.png")));
	Map_mainToolBarAction["encryptionPointsView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/视图/加密点列表视图.png")));
	Map_mainToolBarAction["controlPointsView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/视图/控制点列表视图 (2).png")));
	Map_mainToolBarAction["logView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/视图/日志视图.png")));
	Map_mainToolBarAction["attributeView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/视图/属性日志.png")));
	Map_mainToolBarAction["satelliteImageView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/视图/卫星影像列表视图.png")));
	Map_mainToolBarAction["imageProcessTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/子菜单/影像处理业务工具箱.png")));
	Map_mainToolBarAction["TestAreaDisplayView_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/yjjc2.png")));

	//数据预处理
	//Map_mainToolBarAction["RDModelRadarDataIn_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/drgj-icon.png")));
	//Map_mainToolBarAction["imageEqualProject_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/数据预处理/影像赋投影.png")));

	//影像匹配
	Map_mainToolBarAction["connectPointsMatch_action"]->setIcon(QIcon(":/resource/menu/ljdpp-icon.png"));
	Map_mainToolBarAction["controlPointsMatch_action"]->setIcon(QIcon(":/resource/menu/kzdpp-icon.png"));
	Map_mainToolBarAction["dataModelConfig_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/影像匹配/数据模型配置.png")));

	//平差
	Map_mainToolBarAction["freedomNetAdjustCal_action"]->setIcon(QIcon(":/resource/menu/zywpc-icon.png"));
	Map_mainToolBarAction["controlNetAdjustCal_action"]->setIcon(QIcon(":/resource/menu/qywpc-icon.png"));

	//纠正融合
	Map_mainToolBarAction["normalIncidenceCorrect_action"]->setIcon(QIcon(":/resource/menu/yxjz-icon.png"));
	Map_mainToolBarAction["imageInter_action"]->setIcon(QIcon(":/resource/menu/yxrh-icon.png"));

	//匀色镶嵌
	Map_mainToolBarAction["imageinlay_action"]->setIcon(QIcon(":/resource/menu/yxxq-icon.png"));
	Map_mainToolBarAction["imageEventlyColor_action"]->setIcon(QIcon(":/resource/menu/yxys-icon.png"));
	Map_mainToolBarAction["trueColorConver_action"]->setIcon(QIcon(":/resource/menu/zcszh-icon.png"));
	Map_mainToolBarAction["intelligentInlay_action"]->setIcon(QIcon(":/resource/menu/znxq-icon.png"));

	//影像业务处理流
	//Map_mainToolBarAction["oneButtonDataProcessAuto1_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/影像处理业务流/一键式数据处理.png")));

	////工具
	//Map_mainToolBarAction["DEMMosaic_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/DEM镶嵌.png")));
	//Map_mainToolBarAction["Rpb2Rpc_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/RPB转RPC.png")));
	//Map_mainToolBarAction["RpcToRpb_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/RPC转RPB.png")));
	//Map_mainToolBarAction["BandSplitTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段拆分软件.png")));
	//Map_mainToolBarAction["BandCompositeTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段合成软件.png")));
	//Map_mainToolBarAction["BandMathXTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段计算软件.png")));
	//Map_mainToolBarAction["BandOrganizeTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段重组软件.png")));
	//Map_mainToolBarAction["GeoDataExtractorTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/参考数据提取.png")));
	Map_mainToolBarAction["outputControlPoints_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/导出控制点.png")));
	//Map_mainToolBarAction["formatChange_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/格式转换.png")));
	//Map_mainToolBarAction["NoDataAttribute_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/设置NoData属性.png")));
	//Map_mainToolBarAction["projectChange_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/投影转换.png")));
	//Map_mainToolBarAction["ImageFilterTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像滤波.png")));
	//Map_mainToolBarAction["ImageCoordinateProcessingTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像坐标信息处理工具.png")));
	//Map_mainToolBarAction["WindowsClusterService_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/Windows集群服务.png")));
	//Map_mainToolBarAction["Orthorectification_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/正射纠正软件.png")));
	//Map_mainToolBarAction["ImageFusionSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像融合软件.png")));
	//Map_mainToolBarAction["TrueColorConversionSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/真彩色转换软件.png")));
	//Map_mainToolBarAction["ImageColorCorrectionSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像匀色软件.png")));
	//Map_mainToolBarAction["ImageMosaicSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像镀嵌软件.png")));
	//Map_mainToolBarAction["RemoteSensingImageDecompression_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/遥感影像解压缩.png")));
	//Map_mainToolBarAction["Buildpyramid_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/创建金字塔.png")));
	//Map_mainToolBarAction["ImageResampling_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像重采样.png")));
	//Map_mainToolBarAction["ImageRangeProduction_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像范围生产.png")));
	//Map_mainToolBarAction["MapExtentGeneration_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/图幅范围生成.png")));
	Map_mainToolBarAction["shutdownTask_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/子菜单/停止当前任务.png")));

	//帮助
	Map_mainToolBarAction["useManual_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/子菜单/使用手册.png")));
	Map_mainToolBarAction["remotePermissionUpgradeHaspDog_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/gjj2.png")));

	ui.refereDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows); //选择整行
	ui.sateImageDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.PyramidDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.imageInterList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.fusionmodelList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.dataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.AlignmentAdjustmentList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.AlignmentIntegrationList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.fusionmodelList_TableW_2->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.encryptPointsList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.controlPointsLIst_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.ImageMosaicDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.DodgingDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.SmartMosaicDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.MosaicCropDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.AbsPositPrecCheckDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.CloudDetectionDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.ProjectionTransformationDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.FormatConversionDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.SARImageFilteringDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.QuickMosaicDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.ImagecropDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.VectorfileDataList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.CloudDetectionList_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.objectPosPoints_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.objectPos_TableW->setSelectionBehavior(QAbstractItemView::SelectRows);

	ui.encryptPointsList_TableW->verticalHeader()->setVisible(false); // 隐藏行号   
	ui.controlPointsLIst_TableW->verticalHeader()->setVisible(false);  
	ui.refereDataList_TableW->verticalHeader()->setVisible(false); 
	ui.sateImageDataList_TableW->verticalHeader()->setVisible(false);  
	ui.PyramidDataList_TableW->verticalHeader()->setVisible(false);   
	ui.imageInterList_TableW->verticalHeader()->setVisible(false);
	ui.dataList_TableW->verticalHeader()->setVisible(false);
	ui.fusionmodelList_TableW->verticalHeader()->setVisible(false);
	ui.AlignmentAdjustmentList_TableW->verticalHeader()->setVisible(false);
	ui.AlignmentIntegrationList_TableW->verticalHeader()->setVisible(false);
	ui.SmartMosaicDataList_TableW->verticalHeader()->setVisible(false);
	ui.AbsPositPrecCheckDataList_TableW->verticalHeader()->setVisible(false);
	ui.MosaicCropDataList_TableW->verticalHeader()->setVisible(false);
	ui.fusionmodelList_TableW_2->verticalHeader()->setVisible(false);
	ui.ImageMosaicDataList_TableW->verticalHeader()->setVisible(false);
	ui.DodgingDataList_TableW->verticalHeader()->setVisible(false);
	ui.objectPos_TableW->verticalHeader()->setVisible(false);
	ui.objectPosPoints_TableW->verticalHeader()->setVisible(false);
	ui.CloudDetectionDataList_TableW->verticalHeader()->setVisible(false);
	ui.ProjectionTransformationDataList_TableW->verticalHeader()->setVisible(false);
	ui.FormatConversionDataList_TableW->verticalHeader()->setVisible(false);
	ui.SARImageFilteringDataList_TableW->verticalHeader()->setVisible(false);
	ui.QuickMosaicDataList_TableW->verticalHeader()->setVisible(false);
	ui.ImagecropDataList_TableW->verticalHeader()->setVisible(false);
	ui.VectorfileDataList_TableW->verticalHeader()->setVisible(false);
	ui.CloudDetectionList_TableW->verticalHeader()->setVisible(false);

	//ui.PyramidDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); //完整显示
	//ui.refereDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents); 
	//ui.sateImageDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.imageInterList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.dataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.AlignmentAdjustmentList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.AlignmentIntegrationList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.encryptPointsList_TableW->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//ui.encryptPointsList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.controlPointsLIst_TableW->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//ui.controlPointsLIst_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.objectPosPoints_TableW->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	//ui.objectPosPoints_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.fusionmodelList_TableW_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	//ui.fusionmodelList_TableW_2->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	//ui.fusionmodelList_TableW->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
	//ui.fusionmodelList_TableW->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
	//ui.ImageMosaicDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.DodgingDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.SmartMosaicDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.MosaicCropDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.AbsPositPrecCheckDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.CloudDetectionDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.ProjectionTransformationDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.FormatConversionDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.QuickMosaicDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.ImagecropDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	//ui.VectorfileDataList_TableW->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

	ui.encryptPointsList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers); // 禁止编辑
	ui.controlPointsLIst_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers); 
	ui.CloudDetectionList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers); 
	ui.refereDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.sateImageDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.PyramidDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.imageInterList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.dataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.fusionmodelList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.AlignmentAdjustmentList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.AlignmentIntegrationList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.fusionmodelList_TableW_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.DodgingDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.ImageMosaicDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.MosaicCropDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.SmartMosaicDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.QuickMosaicDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.ProjectionTransformationDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.FormatConversionDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.SARImageFilteringDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.AbsPositPrecCheckDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.RelPositPrecCheckDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.threeDModelElevatPrecCheckDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.threeDModelBorderPrecCheckDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.CloudDetectionDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.ImagecropDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.VectorfileDataList_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.objectPosPoints_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.objectPos_TableW->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.encryptPointsList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.controlPointsLIst_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.CloudDetectionList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.dataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.fusionmodelList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.ImageMosaicDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.DodgingDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.SmartMosaicDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.CloudDetectionDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.ProjectionTransformationDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.FormatConversionDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.SARImageFilteringDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.QuickMosaicDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.ImagecropDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.VectorfileDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.AbsPositPrecCheckDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);

	ui.refereDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.sateImageDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.PyramidDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.imageInterList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.dataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.fusionmodelList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.AlignmentAdjustmentList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.AlignmentIntegrationList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.SmartMosaicDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.AbsPositPrecCheckDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.MosaicCropDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.fusionmodelList_TableW_2->horizontalHeader()->setStretchLastSection(true);
	ui.ImageMosaicDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.DodgingDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.objectPosPoints_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.CloudDetectionDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.ProjectionTransformationDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.FormatConversionDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.SARImageFilteringDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.QuickMosaicDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.ImagecropDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.VectorfileDataList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.CloudDetectionList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.encryptPointsList_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.controlPointsLIst_TableW->horizontalHeader()->setStretchLastSection(true);
	ui.objectPos_TableW->horizontalHeader()->setStretchLastSection(true);

	ui.encryptPointsList_TableW->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

	ui.refereDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.sateImageDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.PyramidDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.imageInterList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.dataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.fusionmodelList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.AlignmentAdjustmentList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.AlignmentIntegrationList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.fusionmodelList_TableW_2->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.DodgingDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.ImageMosaicDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.MosaicCropDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.SmartMosaicDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.QuickMosaicDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.ProjectionTransformationDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.FormatConversionDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.SARImageFilteringDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.AbsPositPrecCheckDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.RelPositPrecCheckDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.threeDModelElevatPrecCheckDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.threeDModelBorderPrecCheckDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.CloudDetectionDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.ImagecropDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.VectorfileDataList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.encryptPointsList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.controlPointsLIst_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.CloudDetectionList_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.objectPosPoints_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	ui.objectPos_TableW->horizontalHeader()->setTextElideMode(Qt::ElideRight);

	// 初始化隐藏非固定页面 
	for (int i = 3; i < ui.tabWidget_3->count(); ++i) {
		ui.tabWidget_3->setTabEnabled(i, false);
	}
	ui.tabWidget_3->setStyleSheet(
		"QTabBar::tab:disabled { width: 0; color: transparent; }"
		"QTabBar::scroller { width: 0; }"
		"QTabBar::tab:selected {background: #00a99d; color: white;border - bottom: 2px solid #008080;}"
	);

	ui.tabWidget_3->setUsesScrollButtons(true); // 启用滚动按钮 
	// 设置CloudDetectionList_TableW的标签显示方式 
	ui.tabWidget_3->setElideMode(Qt::ElideRight);

	// 添加标签页时设置完整表名为工具提示 
	for (int i = 0; i < ui.tabWidget_3->count(); i++) {
		QString fullTabName = ui.tabWidget_3->tabText(i); // 获取完整表名 
		ui.tabWidget_3->setTabToolTip(i, fullTabName); // 设置完整表名为工具提示 
	}

	// 确保tabBar能显示工具提示 
	ui.tabWidget_3->tabBar()->setMouseTracking(true);

	// 初始化隐藏非固定页面 
	for (int i = 2; i < ui.tabWidget_2->count(); ++i) {
		ui.tabWidget_2->setTabEnabled(i, false);
	}
	ui.tabWidget_2->setStyleSheet(
		"QTabBar::tab:disabled { width: 0; color: transparent; }"
		"QTabBar::scroller { width: 0; }"
		"QTabBar::tab:selected {background: #00a99d; color: white;border - bottom: 2px solid #008080;}"
	);

	ui.tabWidget_2->setUsesScrollButtons(true); // 启用滚动按钮 
	// 设置tabWidget_2的标签显示方式 
	ui.tabWidget_2->setElideMode(Qt::ElideRight);

	// 添加标签页时设置完整表名为工具提示 
	for (int i = 0; i < ui.tabWidget_2->count(); i++) {
		QString fullTabName = ui.tabWidget_2->tabText(i); // 获取完整表名 
		ui.tabWidget_2->setTabToolTip(i, fullTabName); // 设置完整表名为工具提示 
	}

	// 确保tabBar能显示工具提示 
	ui.tabWidget_2->tabBar()->setMouseTracking(true);

	// 初始化隐藏非固定页面 
	for (int i = 1; i < ui.tabWidget->count(); ++i) {
		ui.tabWidget->setTabEnabled(i, false);
	}
	ui.tabWidget->setStyleSheet(
		"QTabBar::tab:disabled { width: 0; color: transparent; }"
		"QTabBar::scroller { width: 0; }"
		"QTabBar::tab:selected {background: #00a99d; color: white;border - bottom: 2px solid #008080;}"
	);

	ui.tabWidget->setUsesScrollButtons(true); // 启用滚动按钮 
	// 设置tabWidget的标签显示方式 
	ui.tabWidget->setElideMode(Qt::ElideRight);

	// 添加标签页时设置完整表名为工具提示 
	for (int i = 0; i < ui.tabWidget->count(); i++) {
		QString fullTabName = ui.tabWidget->tabText(i); // 获取完整表名 
		ui.tabWidget->setTabToolTip(i, fullTabName); // 设置完整表名为工具提示 
	}

	// 确保tabBar能显示工具提示 
	ui.tabWidget->tabBar()->setMouseTracking(true);

	DataimageListMap.insert(QString::fromLocal8Bit("金字塔创建业务数据组织"), 2);  
	DataimageListMap.insert(QString::fromLocal8Bit("真彩色转换业务数据组织"), 3);
	DataimageListMap.insert(QString::fromLocal8Bit("影像融合业务数据组织"), 4);
	DataimageListMap.insert(QString::fromLocal8Bit("配准纠正业务数据组织"), 5);
	DataimageListMap.insert(QString::fromLocal8Bit("配准融合业务数据组织"), 6);
	DataimageListMap.insert(QString::fromLocal8Bit("影像匀色业务数据组织"), 7);
	DataimageListMap.insert(QString::fromLocal8Bit("影像镶嵌业务数据组织"), 8);
	DataimageListMap.insert(QString::fromLocal8Bit("智能镶嵌业务数据组织"), 9);
	DataimageListMap.insert(QString::fromLocal8Bit("快速镶嵌业务数据组织"), 10);
	DataimageListMap.insert(QString::fromLocal8Bit("投影转换业务数据组织"), 11);
	DataimageListMap.insert(QString::fromLocal8Bit("格式转换业务数据组织"), 12);
	DataimageListMap.insert(QString::fromLocal8Bit("绝对定位精度质检业务数据组织"), 13);
	DataimageListMap.insert(QString::fromLocal8Bit("相对定位精度质检业务数据组织"), 14);
	DataimageListMap.insert(QString::fromLocal8Bit("立体模型高程精度质检业务数据组织"), 15);
	DataimageListMap.insert(QString::fromLocal8Bit("立体模型高程精度质检业务数据组织"), 16);
	DataimageListMap.insert(QString::fromLocal8Bit("影像云检业务数据组织"), 17);
	DataimageListMap.insert(QString::fromLocal8Bit("影像裁切业务数据组织"), 18);
	DataimageListMap.insert(QString::fromLocal8Bit("SAR影像滤波业务数据组织"), 19);

	// 在初始化代码中
	//ui.dockWidget_9->setFeatures(ui.dockWidget_9->features() & ~QDockWidget::DockWidgetClosable);
	ui.progressBar->setVisible(false);
	ui.progressBar_imageinfo->setVisible(false);
	ui.imagecheck_boBox->setCurrentIndex(0);

	showWorld();
	//初始化使能
	Map_mainToolBarAction["openProDir_action"]->setEnabled(false);
	Map_mainToolBarAction["savePro_action"]->setEnabled(false);
	//Map_mainToolBarAction["backupsPro_action"]->setEnabled(false);
	//Map_mainToolBarAction["closePro_action"]->setEnabled(false);

	encryptPointsModel = new NonEditableModel(0, 6, this); // 5列 
	encryptPointsModel->setHorizontalHeaderLabels({ u8"序号", u8"ID", u8"重叠度", u8"残差", u8"状态", " " });
	ui.encryptPointsList_TableW->setModel(encryptPointsModel);

	ControlPointsModel = new NonEditableModel(0, 10, this); // 9列 
	ControlPointsModel->setHorizontalHeaderLabels({ u8"序号", u8"ID", u8"类型", u8"重叠度", u8"vxy(像方)", u8"vxy(物方)", u8"vz(高程)", u8"状态", u8"权重", " " });
	ui.controlPointsLIst_TableW->setModel(ControlPointsModel);

	ui.encryptPointsList_TableW->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	ui.encryptPointsList_TableW->viewport()->installEventFilter(this);

	ui.controlPointsLIst_TableW->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	ui.controlPointsLIst_TableW->viewport()->installEventFilter(this);

	// 设置按钮属性 
	ui.btnUp->setToolTip(QString::fromLocal8Bit("上移选中行"));
	ui.btnDown->setToolTip(QString::fromLocal8Bit("下移选中行"));
}

void IMAGEPS::connects()
{
	connect(Map_mainToolBarAction["newPro_action"], &QAction::triggered, this, &IMAGEPS::newProActionSlot);
	connect(Map_mainToolBarAction["openPro_action"], &QAction::triggered, this, &IMAGEPS::openProActionSlot);
	connect(Map_mainToolBarAction["savePro_action"], &QAction::triggered, this, &IMAGEPS::saveProActionSlot);
	//connect(Map_mainToolBarAction["shutdownTask_action"], &QAction::triggered, this, &IMAGEPS::killAllProcesses);
	connect(Map_mainToolBarAction["shutdownTask_action"], &QAction::triggered, this, [this]() {
		killAllProcesses(true);
	});

	connect(Map_mainToolBarAction.value("useManual_action"), &QAction::triggered, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		QString filePath = this->appDirPath +
			QString::fromLocal8Bit("/IMAGEPS_DESKTOP用户手册.pdf");
		QFileInfo fileInfo(filePath);
		if (!fileInfo.exists()) {
			qWarning() << "PDF file does not exist:" << filePath;
			return 0;
		}

		if (!QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()))) {
			qWarning() << "Failed to open PDF file:" << filePath;
		}
	});

	connect(Map_mainToolBarAction["closePro_action"], &QAction::triggered, this, [=]
	{
		clearCentralWidget();
		QMessageBox::StandardButton saveBtn = QMessageBox::question(
			this,
			QString::fromLocal8Bit("警告"),
			QString::fromLocal8Bit("是否保存工程后再关闭软件？"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::Yes
		);

		if (saveBtn == QMessageBox::Yes)
		{
			bool saveSuccess = saveProActionSlot();

			if (!saveSuccess)
			{
				QMessageBox::warning(this,
					QString::fromLocal8Bit("错误"),
					QString::fromLocal8Bit("工程保存失败！"));
				
				return;
			}

			QMessageBox::StandardButton closeBtn = QMessageBox::question(
				this,
				QString::fromLocal8Bit("确认"),
				QString::fromLocal8Bit("工程已保存，是否关闭软件？"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::Yes
			);

			if (closeBtn == QMessageBox::Yes)
			{
				showWorld();
				closeflag = false;
				//Map_mainToolBarAction["closePro_action"]->setEnabled(false);
				Map_mainToolBarAction["savePro_action"]->setEnabled(false);
				//Map_mainToolBarAction["backupsPro_action"]->setEnabled(false);
				Map_mainToolBarAction["openProDir_action"]->setEnabled(false);
				//Map_mainToolBarAction["newPro_action"]->setEnabled(true);
				//Map_mainToolBarAction["openPro_action"]->setEnabled(true);
			}
		}
		else
		{
			showWorld();
			closeflag = false;
			//Map_mainToolBarAction["closePro_action"]->setEnabled(false);
			Map_mainToolBarAction["savePro_action"]->setEnabled(false);
			//Map_mainToolBarAction["backupsPro_action"]->setEnabled(false);
			Map_mainToolBarAction["openProDir_action"]->setEnabled(false);
			//Map_mainToolBarAction["newPro_action"]->setEnabled(true);
			//Map_mainToolBarAction["openPro_action"]->setEnabled(true);
		}

		//showWorld();

		//Map_mainToolBarAction["closePro_action"]->setEnabled(false);
		//Map_mainToolBarAction["savePro_action"]->setEnabled(false);
		//Map_mainToolBarAction["backupsPro_action"]->setEnabled(false);
		//Map_mainToolBarAction["openProDir_action"]->setEnabled(false);
		//Map_mainToolBarAction["newPro_action"]->setEnabled(true);
		//Map_mainToolBarAction["openPro_action"]->setEnabled(true);

	});

	connect(Map_mainToolBarAction["openProDir_action"], &QAction::triggered, this, [=]
	{
		if (projectPath.isEmpty())
			return;

		QString urlPath;

		urlPath = getProjectFolderPath(projectPath);
		QDesktopServices::openUrl(QUrl::fromLocalFile(urlPath.replace("\\", "/")));
	});

	connect(Map_mainToolBarAction["satelliteImageView_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_2, ui.tabWidget_2, 1);
	});

	connect(Map_mainToolBarAction["referenceDataView_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_2, ui.tabWidget_2, 0);
	});

	connect(Map_mainToolBarAction["encryptionPointsView_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_10, ui.tabWidget_3, 0);
	});

	connect(Map_mainToolBarAction["controlPointsView_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_10, ui.tabWidget_3, 1);
	});

	connect(Map_mainToolBarAction["imageProcessTool_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_10, ui.tabWidget_3, 2);
	});

	connect(Map_mainToolBarAction["pointInforView_action"], &QAction::triggered, this, [=]
	{
		if (ui.pointsInfoList_DockW->isVisible()) {
			ui.pointsInfoList_DockW->hide();
		}
		else {
			ui.pointsInfoList_DockW->show();
		}
	});

	connect(Map_mainToolBarAction["TestAreaDisplayView_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_9, ui.tabWidget, 0);
	});

	connect(Map_mainToolBarAction["attributeView_action"], &QAction::triggered, this, [=]
	{

		if (ui.dockWidget_4->isVisible()) {
			ui.dockWidget_4->hide();
		}
		else {
			ui.dockWidget_4->show();
		}
	});

	connect(Map_mainToolBarAction["logView_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_13, ui.log_TabW, 2);
		setView(ui.dockWidget_13, ui.log_TabW, 1);
		setView(ui.dockWidget_13, ui.log_TabW, 0);
		//if (ui.dockWidget_13->isVisible()) {
		//	ui.dockWidget_13->hide();
		//}
		//else {
		//	ui.dockWidget_13->show();
		//}
	});

	connect(Map_mainToolBarAction["quit_action"], &QAction::triggered, this, [=]
	{
		this->close();
	});

	connect(Map_mainToolBarAction["cloudResultLook_action"], &QAction::triggered, this, [=]
	{
		setView(ui.dockWidget_10, ui.tabWidget_3, 3);
	});

	connect(Map_mainToolBarAction["genThumImageByDataPre_action"], &QAction::triggered, this, [=]
	{
		if (!showConfirmationDialog(QString::fromLocal8Bit("生成缩略图")))
			return;
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		systemConfig->ThumbnailGeneration(DataModelPath);
	});

	connect(Map_mainToolBarAction["cloudCheckBasicThumImage_action"], &QAction::triggered, this, [=]
	{
		if (!showConfirmationDialog(QString::fromLocal8Bit("云检测")))
			return;
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		// 先断开之前的连接（避免重复调用）
		disconnect(systemConfig, &SystemConfig::CloudDetectionFinished,
			this, qOverload<>(&IMAGEPS::CloudDetectionList));
		// 重新连接信号
		connect(systemConfig, &SystemConfig::CloudDetectionFinished,
			this, qOverload<>(&IMAGEPS::CloudDetectionList));

		systemConfig->CloudDetection();
	});

	/**
	* 工具软件
	*/
	//影像云雪自动替换软件（交互式）
	connect(Map_mainToolBarAction["ImageCloudandSnowReplacementInteractive_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSCloudReplaceTool2.exe"));

	});

	//影像云雪自动替换软件（批处理）
	connect(Map_mainToolBarAction["ImageCloudandSnowReplacementBatchProcessing_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSCloudReplaceTool.exe"));

	});

	//影像云雪自动替换软件（全自动）
	connect(Map_mainToolBarAction["ImageCloudandSnowReplacementFullyAutomatic_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSAutoCloudReplaceTool.exe"));

	});

	//DSM/DEM交互式编辑
	connect(Map_mainToolBarAction["DSMDEMInteractiveEditing_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSDemEditTool.exe"));

	});

	//投影转换
	connect(Map_mainToolBarAction["projectChange_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageReprojectTool.exe"));

	});

	//格式转换
	connect(Map_mainToolBarAction["formatChange_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSFormatTransTool2.exe"));

	});

	//影像滤波
	connect(Map_mainToolBarAction["ImageFilterTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageFilterTool.exe"));

	});

	//Rpb2Rpc
	connect(Map_mainToolBarAction["Rpb2Rpc_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSRpb2Rpc.exe"));

	});

	//RpcToRpb
	connect(Map_mainToolBarAction["RpcToRpb_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSRpcToRpb.exe"));

	});

	//波段拆分软件
	connect(Map_mainToolBarAction["BandSplitTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSBandSplitTool.exe"));

	});

	//波段重组软件
	connect(Map_mainToolBarAction["BandOrganizeTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSBandOrganizeTool.exe"));

	});

	//波段合成软件
	connect(Map_mainToolBarAction["BandCompositeTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSBandCompositeTool.exe"));

	});

	//波段计算软件
	connect(Map_mainToolBarAction["BandMathXTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSBandMathXTool.exe"));

	});

	//影像常规裁切
	connect(Map_mainToolBarAction["ImageCropTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageCropTool.exe"));

	});

	//影像多模式裁切
	connect(Map_mainToolBarAction["ImageCutTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageCutTool.exe"));

	});

	//立体模型裁切
	connect(Map_mainToolBarAction["立体模型裁切"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageCropTool_ZY.exe"));

	});

	//配准纠正软件
	connect(Map_mainToolBarAction["GeoRectifyPlatform_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSGeoRectifyPlatform.exe"));

	});

	//正射纠正软件
	connect(Map_mainToolBarAction["Orthorectification_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSOrthoTool.exe"));

	});

	//影像融合软件
	connect(Map_mainToolBarAction["ImageFusionSoftware_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSFusionTool2.exe"));

	});

	//真彩色转换软件
	connect(Map_mainToolBarAction["TrueColorConversionSoftware_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageOutByteTool2.exe"));

	});

	//影像匀色软件
	connect(Map_mainToolBarAction["ImageColorCorrectionSoftware_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSDodgingTool2.exe"));

	});

	//影像镶嵌软件
	connect(Map_mainToolBarAction["ImageMosaicSoftware_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSMosaicTool2.exe"));

	});

	//遥感影像解压缩
	connect(Map_mainToolBarAction["RemoteSensingImageDecompression_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSUnzipTool.exe"));

	});

	//创建金字塔
	connect(Map_mainToolBarAction["Buildpyramid_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSCreatePyramidApp.exe"));

	});

	//影像重采样
	connect(Map_mainToolBarAction["ImageResampling_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageResampleTool.exe"));

	});

	//影像范围生产
	connect(Map_mainToolBarAction["ImageRangeProduction_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSGenShpTool.exe"));

	});

	//图幅范围生成
	connect(Map_mainToolBarAction["MapExtentGeneration_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSMapGenTool.exe"));

	});

	//Windows集群服务
	connect(Map_mainToolBarAction["WindowsClusterService_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSSlaveServer.exe"));

	});

	//无效值替换
	connect(Map_mainToolBarAction["Invalidvaluereplacement_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageValueRelaceTool.exe"));

	});

	//影像零值处理(白点)
	connect(Map_mainToolBarAction["ImageZeroValueProcessing_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSWhiteDotEraseTool.exe"));

	});

	//影像零值处理(白点)并行版
	connect(Map_mainToolBarAction["ImageZerovalueProcessings_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSWhiteDotEraseTool2.exe"));

	});

	//无效值查找
	connect(Map_mainToolBarAction["Invalidvaluelookup_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSHoleCheckTool.exe"));

	});
	
	/**
	* 扩展模块
	*/
	//DEM镶嵌
	connect(Map_mainToolBarAction["DEMMosaic_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSDemMosaicTool.exe"));

	});

	//影像坐标信息处理工具
	connect(Map_mainToolBarAction["ImageCoordinateProcessingTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSGeoInfoRemoveTool.exe"));

	});

	//设置NoData属性
	connect(Map_mainToolBarAction["NoDataAttribute_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSInvalidValueTool.exe"));

	});

	//高程异常消除
	connect(Map_mainToolBarAction["HeightAnomalyDetection_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSRemoveCorseValueTool.exe"));

	});

	//高程异常查找
	connect(Map_mainToolBarAction["HeightAnomalyElimination_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSFindCorseValueTool.exe"));

	});

	//DSM-DEM逻辑一致性处理
	connect(Map_mainToolBarAction["DSMDEMLogicalConsistencyProcessing_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSDsmDemCheckTool.exe"));

	});

	//DSM-DEM逻辑一致性处理（并行）
	connect(Map_mainToolBarAction["ParallelDSM-DEMLogicalConsistencyProcessing_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSDsmDemCheckTool2.exe"));

	});

	//DSM-DEM标准化检查
	connect(Map_mainToolBarAction["DSMDEMStandardizationCheck_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSDsmDemStandCheckTool.exe"));

	});

	//DSM-DEM接边检查
	connect(Map_mainToolBarAction["DSMDEMEdgeMatchingCheck_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSDsmDemRelCheckTool.exe"));

	});

	//DEM相关性质检工具
	connect(Map_mainToolBarAction["DEMRelatedPropertyCheck_action"], &QAction::triggered, this, [=]
	{
		//QProcess * process = new QProcess(this);
		////启动程序，并输入参数
		//process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSImageCorrelatonTool.exe"));

		SimpleToolSWidget->setWindowTitle(QString::fromLocal8Bit("DEM相关性质检工具"));
		SimpleToolSWidget->setWindowIcon(QIcon(u8":/resource/menu/文件/打开工程文件夹.png"));
		SimpleToolSWidget->show();
		SimpleToolSWidget->resize(450, 350);
		SimpleToolSWidget->ui.stackedWidget->setCurrentWidget(SimpleToolSWidget->ui.DEMRelatedPropertyCheck_page);
		SimpleToolSWidget->m_ExeName = u8"DEM相关性质检";
		SimpleToolSWidget->m_ProgressBar = SimpleToolSWidget->ui.progressBar_9;
	});

	//影像赋投影
	connect(Map_mainToolBarAction["imageEqualProject_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSSetProjectionTool.exe"));

	});

	//参考数据提取
	connect(Map_mainToolBarAction["GeoDataExtractorTool_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSGeoDataExtractorTool.exe"));

	});

	//R-D模型雷达数据导入
	connect(Map_mainToolBarAction["RDModelRadarDataIn_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/PSRd2RfmTool.exe"));

	});

	//远程许可升级(hasp狗)
	connect(Map_mainToolBarAction["remotePermissionUpgradeHaspDog_action"], &QAction::triggered, this, [=]
	{
		QProcess * process = new QProcess(this);
		//启动程序，并输入参数
		process->start(this->appDirPath + QString::fromLocal8Bit("/Software/RUS_HASP_IMAGEPS.exe"));

	});

	//测试****************
	connect(Map_mainToolBarAction["test_action"], &QAction::triggered, this, [=]
	{
		// 创建主窗口并显示影像
		//SatelliteViewer viewer;

		QString originImagePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIFF Image(*.png);;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (originImagePath.isEmpty()) {

			QMessageBox::warning(this, u8"错误", u8"未选中影像文件");
		}

		//ui.graphicsView->loadTiff(originImagePath); // 替换为实际卫星影像路径
		//ui.graphicsView->loadImage(u8"E:/测试数据/高景/dom/SV_dom.tif"); // 替换为实际卫星影像路径
		//ui.graphicsView->.resize(800, 600);
		//viewer.show();

	});


	//影像绝对定位精度质检
	connect(Map_mainToolBarAction["imageAbsPositPrecCheckBusDataOrg_action"], &QAction::triggered, this, [=]
	{
		int index1 = DataimageListMap.value(QString::fromLocal8Bit("绝对定位精度质检业务数据组织"));
		for (int i = 0; i < ui.tabWidget_2->count(); ++i) {

			QWidget* page = ui.tabWidget_2->widget(index1);
			if (ui.tabWidget_2->isTabEnabled(index1) && page->isVisible() &&
				(ui.tabWidget_2->currentIndex() == index1))
			{
				if (index1 >= 2) {

					ui.tabWidget_2->setTabEnabled(index1, false);
					ui.tabWidget_2->setStyleSheet(
						"QTabBar::tab:disabled { width: 0; color: transparent; }"
						"QTabBar::scroller { width: 0; }"
					);
					ui.tabWidget_2->setCurrentIndex(1);
					return;
				}
			}
		}

		if (DataimageListMap.contains(QString::fromLocal8Bit("绝对定位精度质检业务数据组织"))) {
			int index = DataimageListMap.value(QString::fromLocal8Bit("绝对定位精度质检业务数据组织"));
			if (index >= 0 && index < ui.tabWidget_2->count()) {
				ui.tabWidget_2->setTabEnabled(index, true);
				ui.tabWidget_2->tabBar()->setStyleSheet("");
				QWidget *page = ui.tabWidget_2->widget(index);
				page->setVisible(true);
				page->show();
				ui.tabWidget_2->setCurrentIndex(index);
				ui.tabWidget_2->tabBar()->adjustSize();
				ui.tabWidget_2->repaint();
				QApplication::processEvents();
			}
			return;
		}
	});

	connect(Map_mainToolBarAction["imageAbsPositPrecCheck_action"], &QAction::triggered, this, [=]
	{
		if (!showConfirmationDialog(QString::fromLocal8Bit("绝对定位精度质检")))
			return;
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		// 先断开之前的连接（避免重复调用）
		disconnect(systemConfig, &SystemConfig::ImageAbsPositPrecCheckFinished,
			this, qOverload<>(&IMAGEPS::AbsPositPrecCheckPList));
		// 重新连接信号
		connect(systemConfig, &SystemConfig::ImageAbsPositPrecCheckFinished,
			this, qOverload<>(&IMAGEPS::AbsPositPrecCheckPList));

		systemConfig->ImageAbsPositPrecCheck(DEMFilePath, DOMFilePath, AbsPositPrecCheckFilePath, AbsPositPrecCheckPathPath);

	});

	connect(Map_mainToolBarAction["systemSet_action"], &QAction::triggered, this, &IMAGEPS::systemSetSlot);

	connect(Map_mainToolBarAction["allMapShow_action"], &QAction::triggered, this, &IMAGEPS::allMapShow_actionSlot);
	connect(Map_mainToolBarAction["amplify_action"], &QAction::triggered, this, &IMAGEPS::amplify_actionSlot);
	connect(Map_mainToolBarAction["narrow_action"], &QAction::triggered, this, &IMAGEPS::narrow_actionSlot);

	//tools
	connect(Map_mainToolBarAction["outputConnectPointsFile_action"], &QAction::triggered, this, &IMAGEPS::onExportEncryptedPointsClicked);
	connect(Map_mainToolBarAction["inputConnectPointsFile_action"], &QAction::triggered, this, &IMAGEPS::inExportEncryptedPointsClicked);
	connect(Map_mainToolBarAction["outputControlPoints_action"], &QAction::triggered, this, &IMAGEPS::onExportControlPointsClicked);
	connect(Map_mainToolBarAction["exchangeFormatcontrolPoints_action"], &QAction::triggered, this, &IMAGEPS::inExportControlPointsClicked);
	//connect(Map_mainToolBarAction["translation_action"], &QAction::triggered, this, &IMAGEPS::systemSetSlot);

	connect(ui.refereDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(referDataList_TabWContextSlot(QPoint)));
	connect(ui.sateImageDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(sateImageDataList_TabWContextSlot(QPoint)));
	connect(ui.PyramidDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(PyramidDataList_TabWContextSlot(QPoint)));
	connect(ui.imageInterList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ImageInterList_TabWContextSlot(QPoint)));
	connect(ui.DodgingDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(DodgingList_TabWContextSlot(QPoint)));
	connect(ui.AlignmentAdjustmentList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(AlignmentAdjustmentList_TabWContextSlot(QPoint)));
	connect(ui.dataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(FusionmodelsrcList_TabWContextSlot(QPoint)));
	connect(ui.fusionmodelList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(FusionmodelList_TabWContextSlot(QPoint)));
	connect(ui.AlignmentIntegrationList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(AlignmentIntsrcList_TabWContextSlot(QPoint)));
	connect(ui.fusionmodelList_TableW_2, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(AlignmentIntList_TabWContextSlot(QPoint)));
	connect(ui.SmartMosaicDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(SmartMosaicDataList_TabWContextSlot(QPoint)));
	connect(ui.MosaicCropDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(MosaicCropDataList_TabWContextSlot(QPoint)));
	connect(ui.ImageMosaicDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ImageMosaicList_TabWContextSlot(QPoint)));
	connect(ui.AbsPositPrecCheckDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(AbsPositPrecCheckList_TabWContextSlot(QPoint)));
	connect(ui.RelPositPrecCheckDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(RelPositPrecCheckList_TabWContextSlot(QPoint)));
	connect(ui.CloudDetectionDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(CloudDetectionDataList_TabWContextSlot(QPoint)));
	connect(ui.ProjectionTransformationDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ProTransformationDataList_TabWContextSlot(QPoint)));
	connect(ui.FormatConversionDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(FormatConversionDataList_TabWContextSlot(QPoint)));
	connect(ui.SARImageFilteringDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(SARImageFilteringDataList_TabWContextSlot(QPoint)));
	connect(ui.QuickMosaicDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(QuickMosaicDataList_TabWContextSlot(QPoint)));
	connect(ui.ImagecropDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(ImagecropDataList_TabWContextSlot(QPoint)));
	connect(ui.VectorfileDataList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(VectorfileDataList_TabWContextSlot(QPoint)));
	connect(ui.tabWidget_2, &QTabWidget::currentChanged, this, [this](int index) {
		if (index >= 0) {  // 确保索引有效
			QString tabName = ui.tabWidget_2->tabText(index);
			ui.dockWidget_2->setWindowTitle(tabName);
		}
	});
	connect(ui.tabWidget_3, &QTabWidget::currentChanged, this, [this](int index) {
		if (index >= 0) {  // 确保索引有效
			QString tabName = ui.tabWidget_3->tabText(index);
			ui.dockWidget_10->setWindowTitle(tabName);
		}
	});
	//connect(ui.measureAreaShow_GLW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(measureAreaShow_GLWContextSlot(QPoint)));
	connect(ui.encryptPointsList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(encryptPointsList_TabWContextSlot(QPoint)));
	connect(ui.controlPointsLIst_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(controlPointsLIst_TabWContextSlot(QPoint)));
	connect(ui.CloudDetectionList_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(CloudDetectionLIst_TabWContextSlot(QPoint)));

	//图像信息点击事件
	connect(ui.refereDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.sateImageDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.PyramidDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.objectPosPoints_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.imageInterList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.dataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.DodgingDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.ImageMosaicDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.AlignmentAdjustmentList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.AlignmentIntegrationList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.SmartMosaicDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.AbsPositPrecCheckDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.RelPositPrecCheckDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.CloudDetectionDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.CloudDetectionList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.ProjectionTransformationDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.FormatConversionDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.SARImageFilteringDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.QuickMosaicDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.ImagecropDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);
	connect(ui.VectorfileDataList_TableW, &QTableWidget::cellClicked, this, &IMAGEPS::attributeView_actionSLOT);

	connect(ui.treeWidget, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(imageHandleBus_TreWContextSlot(QPoint)));
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(imageHandleBus_TreWSlot(QTreeWidgetItem *, int)));

	connect(ui.objectPos_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pointsInfoList_DockWContextSlot(QPoint)));
	connect(ui.objectPosPoints_TableW, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(pointsInfoList_DockWContextSlot(QPoint)));

	connect(Map_mainToolBarAction["dataModelConfig_action"], &QAction::triggered, this, &IMAGEPS::dataModelConfigActionSlot);
	connect(Map_mainToolBarAction["connectPointsMatch_action"], &QAction::triggered, this, &IMAGEPS::connectPointsMatchActionSlot);
	connect(Map_mainToolBarAction["controlPointsMatch_action"], &QAction::triggered, this, &IMAGEPS::controlPointsMatchActionSlot);
	connect(Map_mainToolBarAction["freedomNetAdjustCal_action"], &QAction::triggered, this, &IMAGEPS::FreeNetworkAdjustmentActionSlot);
	connect(Map_mainToolBarAction["controlNetAdjustCal_action"], &QAction::triggered, this, &IMAGEPS::controlNetAdjustmentActionSlot);
	connect(Map_mainToolBarAction["normalIncidenceCorrect_action"], &QAction::triggered, this, &IMAGEPS::orthorectificAtionSlot);
	connect(Map_mainToolBarAction["imageInter_action"], &QAction::triggered, this, &IMAGEPS::imageInterActionSlot);
	connect(Map_mainToolBarAction["trueColorConver_action"], &QAction::triggered, this, &IMAGEPS::trueColorConveractionSlot);
	connect(Map_mainToolBarAction["intelligentInlay_action"], &QAction::triggered, this, &IMAGEPS::SmartMosaicactionSlot);
	connect(Map_mainToolBarAction["imageEventlyColor_action"], &QAction::triggered, this, &IMAGEPS::DodgingactionSlot);
	connect(Map_mainToolBarAction["imageinlay_action"], &QAction::triggered, this, &IMAGEPS::ImageMosaicactionSlot);
	connect(Map_mainToolBarAction["oneButtonDataProcessAuto1_action"], &QAction::triggered, this, &IMAGEPS::OrthoRectificationFusion_actionSlot);
	
	connect(ui.tabWidget, &QTabWidget::currentChanged, this, &IMAGEPS::onTabChanged);
	//connect(ui.runLog_Edit, &QWidget::customContextMenuRequested, this, [=](QPoint)
	//{
	//	QMenu* menu = new QMenu;
	//	menu->addAction(QString::fromLocal8Bit("清除"));
	//	menu->addAction(QString::fromLocal8Bit("拷贝"));
	//	menu->exec(cursor().pos());
	//	foreach(QAction* pAction, menu->actions()) delete pAction;
	//	delete menu;
	//});
	//connect(ui.imageLog_Edit, &QWidget::customContextMenuRequested, this, [=](QPoint)
	//{
	//	QMenu* menu = new QMenu;
	//	menu->addAction(QString::fromLocal8Bit("清除"));
	//	menu->addAction(QString::fromLocal8Bit("拷贝"));
	//	menu->exec(cursor().pos());
	//	foreach(QAction* pAction, menu->actions()) delete pAction;
	//	delete menu;
	//});
	//connect(ui.objectLog_Edit, &QWidget::customContextMenuRequested, this, [=](QPoint)
	//{
	//	QMenu* menu = new QMenu;
	//	menu->addAction(QString::fromLocal8Bit("清除"));
	//	menu->addAction(QString::fromLocal8Bit("拷贝"));
	//	menu->exec(cursor().pos());
	//	foreach(QAction* pAction, menu->actions()) delete pAction;
	//	delete menu;
	//});


	//connect(ui.encryptPointsList_TableW->horizontalHeader(), &QHeaderView::sectionClicked,
	//	[this](int logicalIndex) {
	//	if (logicalIndex != 4) {
	//		Qt::SortOrder order = ui.encryptPointsList_TableW->horizontalHeader()->sortIndicatorOrder();
	//		sortNumericColumn(ui.encryptPointsList_TableW, logicalIndex, order);
	//	}
	//	else {
	//		return;
	//	}
	//});

	auto connectSortFunction = [this](QTableWidget* table, const QSet<int>& excludedColumns = {}) {
		connect(table->horizontalHeader(), &QHeaderView::sectionClicked,
			[this, table, excludedColumns](int logicalIndex) {
			if (!excludedColumns.contains(logicalIndex)) {
				Qt::SortOrder order = table->horizontalHeader()->sortIndicatorOrder();
				//table->setSortingEnabled(false); // 临时禁用自动排序
				sortNumericColumn(table, logicalIndex, order);
				//table->setSortingEnabled(true); // 恢复自动排序 
			}
		});
	};
	//connectSortFunction(ui.encryptPointsList_TableW, { 4 }); // 排除第4列
	//connectSortFunction(ui.controlPointsLIst_TableW);
	connectSortFunction(ui.refereDataList_TableW);
	connectSortFunction(ui.sateImageDataList_TableW);
	connectSortFunction(ui.PyramidDataList_TableW);
	connectSortFunction(ui.imageInterList_TableW);
	connectSortFunction(ui.dataList_TableW);
	connectSortFunction(ui.fusionmodelList_TableW);
	connectSortFunction(ui.AlignmentAdjustmentList_TableW);
	connectSortFunction(ui.AlignmentIntegrationList_TableW);
	//connectSortFunction(ui.SmartMosaicDataList_TableW);
	connectSortFunction(ui.AbsPositPrecCheckDataList_TableW);
	connectSortFunction(ui.RelPositPrecCheckDataList_TableW);
	connectSortFunction(ui.CloudDetectionList_TableW);
	connectSortFunction(ui.CloudDetectionDataList_TableW);
	connectSortFunction(ui.ProjectionTransformationDataList_TableW);
	connectSortFunction(ui.FormatConversionDataList_TableW);
	connectSortFunction(ui.SARImageFilteringDataList_TableW);
	connectSortFunction(ui.fusionmodelList_TableW_2);
	connectSortFunction(ui.ImageMosaicDataList_TableW);
	connectSortFunction(ui.QuickMosaicDataList_TableW);
	connectSortFunction(ui.ImagecropDataList_TableW);
	connectSortFunction(ui.VectorfileDataList_TableW);
	connectSortFunction(ui.DodgingDataList_TableW);
	connectSortFunction(ui.objectPos_TableW);
	connectSortFunction(ui.objectPosPoints_TableW);

	auto connectSort_ViewFunction = [this](QTableView* tableView, const QSet<int>& excludedColumns = {}) {
		connect(tableView->horizontalHeader(), &QHeaderView::sectionClicked,
			[this, tableView, excludedColumns](int logicalIndex) {
			if (!excludedColumns.contains(logicalIndex)) {
				Qt::SortOrder order = tableView->horizontalHeader()->sortIndicatorOrder();
				sortNumericColumn(tableView, logicalIndex, order);
			}
		});
	};
	connectSort_ViewFunction(ui.encryptPointsList_TableW, { 4 }); // 排除第4列
	connectSort_ViewFunction(ui.controlPointsLIst_TableW);

	auto connectSort_SmartM = [this](QTableWidget* table, const QSet<int>& excludedColumns = {}) {
		connect(table->horizontalHeader(), &QHeaderView::sectionClicked,
			[this, table, excludedColumns](int logicalIndex) {
			if (!excludedColumns.contains(logicalIndex)) {
				Qt::SortOrder order = table->horizontalHeader()->sortIndicatorOrder();
				//table->setSortingEnabled(false); // 临时禁用自动排序
				sortNumericColumn_SmartM(table, logicalIndex, order);
				//table->setSortingEnabled(true); // 恢复自动排序 
			}
		});
	};
	connectSort_SmartM(ui.SmartMosaicDataList_TableW);

	connectAllModificationSignals();

	// 为所有需要支持高亮的表格添加选择变化的连接 
	auto connectSelectionChanged = [this](QTableWidget* table) {
		connect(table->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, table]() {
			highlightSelectedBoundaries(table);
		});
	};

	connectSelectionChanged(ui.refereDataList_TableW);
	connectSelectionChanged(ui.sateImageDataList_TableW);
	connectSelectionChanged(ui.PyramidDataList_TableW);
	connectSelectionChanged(ui.imageInterList_TableW);
	connectSelectionChanged(ui.dataList_TableW);
	connectSelectionChanged(ui.fusionmodelList_TableW);
	connectSelectionChanged(ui.AlignmentAdjustmentList_TableW);
	connectSelectionChanged(ui.AlignmentIntegrationList_TableW);
	connectSelectionChanged(ui.SmartMosaicDataList_TableW);
	connectSelectionChanged(ui.AbsPositPrecCheckDataList_TableW);
	connectSelectionChanged(ui.RelPositPrecCheckDataList_TableW);
	connectSelectionChanged(ui.CloudDetectionList_TableW);
	connectSelectionChanged(ui.CloudDetectionDataList_TableW);
	connectSelectionChanged(ui.ProjectionTransformationDataList_TableW);
	connectSelectionChanged(ui.FormatConversionDataList_TableW);
	connectSelectionChanged(ui.SARImageFilteringDataList_TableW);
	connectSelectionChanged(ui.fusionmodelList_TableW_2);
	connectSelectionChanged(ui.ImageMosaicDataList_TableW);
	connectSelectionChanged(ui.QuickMosaicDataList_TableW);
	connectSelectionChanged(ui.ImagecropDataList_TableW);
	connectSelectionChanged(ui.VectorfileDataList_TableW);
	connectSelectionChanged(ui.DodgingDataList_TableW);

	// 连接radioButton信号 
	connect(ui.imagecheck_radioButton, &QRadioButton::toggled, this, [this](bool checked) {
		m_ImageselectionAll = checked;
		ui.ImageInfoShow_Widget->setSelectionMode(checked);

		// 设置/恢复鼠标样式
		if (checked) {
			ui.ImageInfoShow_Widget->setCursor(Qt::CrossCursor);     // 选中时设为十字 
		}
		else {
			ui.ImageInfoShow_Widget->setCursor(Qt::PointingHandCursor);  // 取消时设为指向手势 
			ui.ImageInfoShow_Widget->clearSelection();
		}
	});

	connect(ui.pointselect_radioButton, &QRadioButton::toggled, this, [this](bool checked) {
		ui.ImageInfoShow_Widget->setPointSelectionMode(checked);

		// 设置/恢复鼠标样式
		if (checked) {
			ui.ImageInfoShow_Widget->setCursor(Qt::CrossCursor);     // 选中时设为十字 
		}
		else {
			ui.ImageInfoShow_Widget->setCursor(Qt::PointingHandCursor);  // 取消时设为指向手势 
			ui.ImageInfoShow_Widget->clearSelection();
		}
	});

	// 连接comboBox信号 
	connect(ui.imagecheck_boBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		QString type;
		switch (index) {
		case 0: type = QString::fromLocal8Bit("所有类型"); break;
		case 1: type = QString::fromLocal8Bit("原始影像"); break;
		case 2: type = QString::fromLocal8Bit("DEM"); break;
		case 3: type = QString::fromLocal8Bit("DOM"); break;
		default: type = "";
		}
		ui.ImageInfoShow_Widget->setImageTypeFilter(type);
	});

	connect(ui.pointselect_Box, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[this](int index) {
		QString type;
		switch (index) {
		case 0: type = QString::fromLocal8Bit("all"); break;
		case 1: type = QString::fromLocal8Bit("con"); break;
		case 2: type = QString::fromLocal8Bit("col"); break;
		default: type = "";
		}
		//QString type = ui.pointselect_Box->itemData(index).toString();
		ui.ImageInfoShow_Widget->setPointTypeFilter(type);
	});

	connect(ui.ImageInfoShow_Widget, &ImageInfoShow::pointsSelected,
		this, [this](QSet<QString>& conPoints, QSet<QString>& colPoints) {

		// 高性能选择函数 
		auto performHighSpeedSelection = [](QTableView* tableView, const QSet<QString>& selectedIds) {
			if (!tableView || selectedIds.isEmpty())  return;

			NonEditableModel* model = qobject_cast<NonEditableModel*>(tableView->model());
			if (!model) return;

			// 禁用更新 
			tableView->setUpdatesEnabled(false);

			// 一次性清除所有选择 
			tableView->selectionModel()->clearSelection();

			// 收集所有需要选中的行 
			QList<int> selectedRows;
			int rowCount = model->rowCount();

			// 快速构建行号列表 
			for (int row = 0; row < rowCount; ++row) {
				QModelIndex idIndex = model->index(row, 1); // ID列 
				QString pointId = model->data(idIndex).toString();

				if (selectedIds.contains(pointId)) {
					selectedRows.append(row);
				}
			}

			// 排序以便合并连续范围 
			std::sort(selectedRows.begin(), selectedRows.end());

			// 合并连续的行范围 
			QItemSelection selection;
			if (!selectedRows.isEmpty()) {
				int start = selectedRows.first();
				int end = start;

				for (int i = 1; i < selectedRows.size(); ++i) {
					if (selectedRows[i] == end + 1) {
						end = selectedRows[i];
					}
					else {
						// 添加当前范围 
						QModelIndex leftIndex = model->index(start, 0);
						QModelIndex rightIndex = model->index(end, model->columnCount() - 1);
						selection.merge(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
						start = selectedRows[i];
						end = start;
					}
				}

				// 添加最后一个范围 
				QModelIndex leftIndex = model->index(start, 0);
				QModelIndex rightIndex = model->index(end, model->columnCount() - 1);
				selection.merge(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
			}

			// 应用合并后的选择范围 
			if (!selection.isEmpty()) {
				tableView->selectionModel()->select(selection, QItemSelectionModel::Select);
			}

			// 启用更新 
			tableView->setUpdatesEnabled(true);
			tableView->viewport()->update();
		};

		// 执行选择 
		performHighSpeedSelection(ui.encryptPointsList_TableW, conPoints);
		performHighSpeedSelection(ui.controlPointsLIst_TableW, colPoints);
		ui.ImageInfoShow_Widget->clearSelection();
	});

	//connect(ui.ImageInfoShow_Widget, &ImageInfoShow::pointsSelected,
	//	this, [this](QSet<QString>& conPoints, QSet<QString>& colPoints) {

	//	// 高性能选择函数 
	//	auto performHighSpeedSelection = [](QTableView* tableView, const QSet<QString>& selectedIds) {
	//		if (!tableView || selectedIds.isEmpty())  return;

	//		NonEditableModel* model = qobject_cast<NonEditableModel*>(tableView->model());
	//		if (!model) return;

	//		// 禁用更新 
	//		tableView->setUpdatesEnabled(false);

	//		// 一次性清除所有选择 
	//		tableView->selectionModel()->clearSelection();

	//		// 收集所有需要选中的行 
	//		QList<int> selectedRows;
	//		int rowCount = model->rowCount();

	//		// 快速构建行号列表 
	//		for (int row = 0; row < rowCount; ++row) {
	//			QModelIndex idIndex = model->index(row, 1); // ID列 
	//			QString pointId = model->data(idIndex).toString();

	//			if (selectedIds.contains(pointId)) {
	//				selectedRows.append(row);
	//			}
	//		}

	//		//// 如果选择的点太多，只选择前10000个 
	//		//if (selectedRows.size() > 10000) {
	//		//	selectedRows = selectedRows.mid(0, 10000);
	//		//}

	//		// 排序以便合并连续范围 
	//		std::sort(selectedRows.begin(), selectedRows.end());

	//		// 合并连续的行范围 
	//		QItemSelection selection;
	//		if (!selectedRows.isEmpty()) {
	//			int start = selectedRows.first();
	//			int end = start;

	//			for (int i = 1; i < selectedRows.size(); ++i) {
	//				if (selectedRows[i] == end + 1) {
	//					end = selectedRows[i];
	//				}
	//				else {
	//					// 添加当前范围 
	//					QModelIndex leftIndex = model->index(start, 0);
	//					QModelIndex rightIndex = model->index(end, model->columnCount() - 1);
	//					selection.merge(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
	//					start = selectedRows[i];
	//					end = start;
	//				}

	//				// 添加最后一个范围 
	//				QModelIndex leftIndex = model->index(start, 0);
	//				QModelIndex rightIndex = model->index(end, model->columnCount() - 1);
	//				selection.merge(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
	//			}
	//		}

	//		// 应用合并后的选择范围 
	//		if (!selection.isEmpty()) {
	//			tableView->selectionModel()->select(selection, QItemSelectionModel::Select);
	//		}

	//		// 启用更新 
	//		tableView->setUpdatesEnabled(true);
	//		tableView->viewport()->update();
	//	};

	//	// 执行选择 
	//	performHighSpeedSelection(ui.encryptPointsList_TableW, conPoints);
	//	performHighSpeedSelection(ui.controlPointsLIst_TableW, colPoints);
	//});

	//connect(ui.ImageInfoShow_Widget, &ImageInfoShow::pointsSelected,
	//	this, [this](QSet<QString>& conPoints, QSet<QString>& colPoints) {

	//	// 高性能选择函数 
	//	auto performHighSpeedSelection = [](QTableView* tableView, const QSet<QString>& selectedIds) {
	//		if (!tableView || selectedIds.isEmpty())  return;

	//		NonEditableModel* model = qobject_cast<NonEditableModel*>(tableView->model());
	//		if (!model) return;

	//		// 禁用更新 
	//		tableView->setUpdatesEnabled(false);

	//		// 一次性清除所有选择 
	//		tableView->selectionModel()->clearSelection();

	//		// 收集所有需要选中的行 
	//		QList<int> selectedRows;
	//		int rowCount = model->rowCount();

	//		// 快速构建行号列表 
	//		for (int row = 0; row < rowCount; ++row) {
	//			QModelIndex idIndex = model->index(row, 1); // ID列 
	//			QString pointId = model->data(idIndex).toString();

	//			if (selectedIds.contains(pointId)) {
	//				selectedRows.append(row);
	//			}
	//		}

	//		//// 如果选择的点太多，只选择前10000个 
	//		//if (selectedRows.size() > 10000) {
	//		//	selectedRows = selectedRows.mid(0, 10000);
	//		//}

	//		// 排序以便合并连续范围 
	//		std::sort(selectedRows.begin(), selectedRows.end());

	//		// 合并连续的行范围 
	//		QItemSelection selection;
	//		if (!selectedRows.isEmpty()) {
	//			int start = selectedRows.first();
	//			int end = start;

	//			for (int i = 1; i < selectedRows.size(); ++i) {
	//				if (selectedRows[i] == end + 1) {
	//					end = selectedRows[i];
	//				}
	//				else {
	//					// 添加当前范围 
	//					QModelIndex leftIndex = model->index(start, 0);
	//					QModelIndex rightIndex = model->index(end, model->columnCount() - 1);
	//					selection.merge(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
	//				}
	//			}
	//		}

	//		// 应用合并后的选择范围 
	//		if (!selection.isEmpty()) {
	//			tableView->selectionModel()->select(selection, QItemSelectionModel::Select);
	//		}

	//		// 启用更新 
	//		tableView->setUpdatesEnabled(true);
	//		tableView->viewport()->update();
	//	};

	//	// 执行选择 
	//	performHighSpeedSelection(ui.encryptPointsList_TableW, conPoints);
	//	performHighSpeedSelection(ui.controlPointsLIst_TableW, colPoints);
	//});

	//connect(ui.ImageInfoShow_Widget, &ImageInfoShow::pointsSelected,
	//	this, [this](QSet<QString>& conPoints, QSet<QString>& colPoints) {

	//	// 禁用表格重绘以提高性能 
	//	ui.encryptPointsList_TableW->setUpdatesEnabled(false);
	//	ui.controlPointsLIst_TableW->setUpdatesEnabled(false);

	//	// 清除所有选择 
	//	ui.encryptPointsList_TableW->selectionModel()->clearSelection();
	//	ui.controlPointsLIst_TableW->selectionModel()->clearSelection();

	//	// 处理encryptPointsList_TableW的多点选择 
	//	QItemSelectionModel *selectionModel = ui.encryptPointsList_TableW->selectionModel();
	//	QItemSelection selectionenc;

	//	// 遍历所有行，找出所有匹配的点 
	//	for (int row = 0; row < ui.encryptPointsList_TableW->model()->rowCount(); ++row) {
	//		QModelIndex index = ui.encryptPointsList_TableW->model()->index(row, 1); // 假设点ID在第2列 
	//		QString pointId = index.data().toString();

	//		// 如果当前点在conPoints集合中，则选中整行 
	//		if (conPoints.contains(pointId)) {
	//			QModelIndex leftIndex = ui.encryptPointsList_TableW->model()->index(row, 0);
	//			QModelIndex rightIndex = ui.encryptPointsList_TableW->model()->index(row, ui.encryptPointsList_TableW->model()->columnCount() - 1);
	//			selectionenc.merge(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
	//		}
	//	}

	//	// 批量应用所有选择 
	//	if (!selectionenc.isEmpty()) {
	//		selectionModel->select(selectionenc, QItemSelectionModel::ClearAndSelect);

	//		// 添加以下代码强制更新样式 
	//		ui.encryptPointsList_TableW->style()->unpolish(ui.encryptPointsList_TableW);
	//		ui.encryptPointsList_TableW->style()->polish(ui.encryptPointsList_TableW);
	//	}

	//	// 处理controlPointsLIst_TableW的多点选择 
	//	QItemSelectionModel *selectioncolModel = ui.controlPointsLIst_TableW->selectionModel();
	//	QItemSelection selectioncol;

	//	// 遍历所有行，找出所有匹配的点 
	//	for (int row = 0; row < ui.controlPointsLIst_TableW->model()->rowCount(); ++row) {
	//		QModelIndex index = ui.controlPointsLIst_TableW->model()->index(row, 1); // 假设点ID在第2列 
	//		QString pointId = index.data().toString();

	//		// 如果当前点在colPoints集合中，则选中整行 
	//		if (colPoints.contains(pointId)) {
	//			QModelIndex leftIndex = ui.controlPointsLIst_TableW->model()->index(row, 0);
	//			QModelIndex rightIndex = ui.controlPointsLIst_TableW->model()->index(row, ui.controlPointsLIst_TableW->model()->columnCount() - 1);
	//			selectioncol.merge(QItemSelection(leftIndex, rightIndex), QItemSelectionModel::Select);
	//		}
	//	}

	//	// 批量应用所有选择 
	//	if (!selectioncol.isEmpty()) {
	//		selectioncolModel->select(selectioncol, QItemSelectionModel::ClearAndSelect);

	//		// 添加以下代码强制更新样式 
	//		ui.controlPointsLIst_TableW->style()->unpolish(ui.controlPointsLIst_TableW);
	//		ui.controlPointsLIst_TableW->style()->polish(ui.controlPointsLIst_TableW);
	//	}

	//	//// 处理controlPointsLIst_TableW的多点选择 
	//	//for (int row = 0; row < ui.controlPointsLIst_TableW->rowCount(); ++row) {
	//	//	QTableWidgetItem* item = ui.controlPointsLIst_TableW->item(row, 1);
	//	//	if (!item) continue;

	//	//	QString pointId = item->text();

	//	//	// 如果当前点在colPoints集合中，则选中整行 
	//	//	if (colPoints.contains(pointId)) {
	//	//		ui.controlPointsLIst_TableW->setRangeSelected(
	//	//			QTableWidgetSelectionRange(row, 0, row, ui.controlPointsLIst_TableW->columnCount() - 1),
	//	//			true
	//	//		);
	//	//	}
	//	//}

	//	// 启用表格重绘并更新 
	//	ui.encryptPointsList_TableW->setUpdatesEnabled(true);
	//	ui.controlPointsLIst_TableW->setUpdatesEnabled(true);

	//	// 强制刷新视图 
	//	ui.encryptPointsList_TableW->viewport()->update();
	//	ui.controlPointsLIst_TableW->viewport()->update();

	//	// 清除测量区域的选中状态 
	//	ui.ImageInfoShow_Widget->clearSelection();
	//});

	connect(ui.ImageInfoShow_Widget, &ImageInfoShow::filesSelected, this, [this](const QStringList& files) {
		// 获取当前选择的类型
		QString currentType = ui.imagecheck_boBox->currentText();

		// 检查Ctrl键是否按下 
		bool ctrlPressed = QApplication::keyboardModifiers() & Qt::ControlModifier;

		// 构建需要选中的文件名集合（不含扩展名）
		QSet<QString> targetFiles;
		for (const auto& filepath : files) {
			targetFiles.insert(QFileInfo(filepath).completeBaseName());
		}

		// 根据不同类型处理不同的表格
		if (currentType == QString::fromLocal8Bit("所有类型")) {
			// 处理sateImageDataList_TableW表格 
			ui.sateImageDataList_TableW->setSelectionMode(QAbstractItemView::MultiSelection);
			ui.sateImageDataList_TableW->clearSelection();
			for (int row = 0; row < ui.sateImageDataList_TableW->rowCount(); ++row) {
				QTableWidgetItem* item = ui.sateImageDataList_TableW->item(row, 1);
				if (item && targetFiles.contains(item->text())) {
					ui.sateImageDataList_TableW->setRangeSelected(
						QTableWidgetSelectionRange(row, 0, row, ui.sateImageDataList_TableW->columnCount() - 1),
						true
					);
				}
			}

			// 处理refereDataList_TableW表格 
			ui.refereDataList_TableW->setSelectionMode(QAbstractItemView::MultiSelection);
			ui.refereDataList_TableW->clearSelection();
			for (int row = 0; row < ui.refereDataList_TableW->rowCount(); ++row) {
				QTableWidgetItem* item = ui.refereDataList_TableW->item(row, 1);
				if (item && targetFiles.contains(item->text())) {
					ui.refereDataList_TableW->setRangeSelected(
						QTableWidgetSelectionRange(row, 0, row, ui.refereDataList_TableW->columnCount() - 1),
						true
					);
				}
			}

			// 恢复选择模式
			ui.sateImageDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
			ui.refereDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
		}
		else if (currentType == QString::fromLocal8Bit("原始影像")) {
			// 只处理sateImageDataList_TableW表格 
			ui.sateImageDataList_TableW->setSelectionMode(QAbstractItemView::MultiSelection);
			ui.sateImageDataList_TableW->clearSelection();
			for (int row = 0; row < ui.sateImageDataList_TableW->rowCount(); ++row) {
				QTableWidgetItem* item = ui.sateImageDataList_TableW->item(row, 1);
				if (item && targetFiles.contains(item->text())) {
					ui.sateImageDataList_TableW->setRangeSelected(
						QTableWidgetSelectionRange(row, 0, row, ui.sateImageDataList_TableW->columnCount() - 1),
						true
					);
				}
			}
			ui.sateImageDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
		}
		else if (currentType == QString::fromLocal8Bit("DOM") || currentType == QString::fromLocal8Bit("DEM")) {
			// 只处理refereDataList_TableW表格
			ui.refereDataList_TableW->setSelectionMode(QAbstractItemView::MultiSelection);
			ui.refereDataList_TableW->clearSelection();
			for (int row = 0; row < ui.refereDataList_TableW->rowCount(); ++row) {
				QTableWidgetItem* item = ui.refereDataList_TableW->item(row, 1);

				if (item && targetFiles.contains(item->text())) {
					ui.refereDataList_TableW->setRangeSelected(
						QTableWidgetSelectionRange(row, 0, row, ui.refereDataList_TableW->columnCount() - 1),
						true
					);
				}
			}
			ui.refereDataList_TableW->setSelectionMode(QAbstractItemView::ExtendedSelection);
		}
		ui.sateImageDataList_TableW->style()->unpolish(ui.sateImageDataList_TableW);
		ui.sateImageDataList_TableW->style()->polish(ui.sateImageDataList_TableW);
		ui.sateImageDataList_TableW->viewport()->update();
		ui.sateImageDataList_TableW->update();

		ui.ImageInfoShow_Widget->clearSelection();
		if (!ctrlPressed) {
			ui.ImageInfoShow_Widget->m_selectedFiles.clear();
		}
	});

	connect(ui.encryptPointsList_TableW->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &IMAGEPS::onSelectionChanged);

	connect(ui.controlPointsLIst_TableW->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &IMAGEPS::onSelectionChanged);
	// 连接表格点击事件 
	connect(ui.encryptPointsList_TableW->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &IMAGEPS::onEncryptedPointSelected);
	//connect(ui.encryptPointsList_TableW->selectionModel(), &QItemSelectionModel::selectionChanged,
	//	this, &IMAGEPS::onSelectionChanged, Qt::QueuedConnection);
	connect(ui.controlPointsLIst_TableW->selectionModel(), &QItemSelectionModel::selectionChanged,
		this, &IMAGEPS::onControlPointSelected, Qt::QueuedConnection);

	connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxIndexChanged(int)));
	connect(ui.comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBox_2IndexChanged(int)));

	//// 当表格数据变化时自动更新显示 
	//connect(ui.encryptPointsList_TableW->model(), &QAbstractItemModel::dataChanged,
	//	this, &IMAGEPS::updateVisiblePointsFromTables);
	//connect(ui.controlPointsLIst_TableW->model(), &QAbstractItemModel::dataChanged,
	//	this, &IMAGEPS::updateVisiblePointsFromTables);

	// 当行数变化时也更新
	//connect(ui.encryptPointsList_TableW->model(), &QAbstractItemModel::rowsInserted,
	//	this, &IMAGEPS::updateVisiblePointsFromTables);
	//connect(ui.encryptPointsList_TableW->model(), &QAbstractItemModel::rowsRemoved,
	//	this, &IMAGEPS::updateVisiblePointsFromTables);
	if (encryptPointsModel) {
		//connect(encryptPointsModel, &QAbstractItemModel::rowsInserted,
		//	this, &IMAGEPS::updateVisiblePointsFromTables);
		//connect(encryptPointsModel, &QAbstractItemModel::rowsRemoved,
		//	this, &IMAGEPS::updateVisiblePointsFromTables);

		//connect(encryptPointsModel, &QAbstractItemModel::layoutChanged,
		//	this, &IMAGEPS::updateVisiblePointsFromTables);
		connect(encryptPointsModel, &QAbstractItemModel::modelReset,
			this, &IMAGEPS::updateVisiblePointsFromTables);
	}
	if (ControlPointsModel) {
		//connect(ControlPointsModel, &QAbstractItemModel::rowsInserted,
		//	this, &IMAGEPS::updateVisiblePointsFromTables);
		//connect(ControlPointsModel, &QAbstractItemModel::rowsRemoved,
		//	this, &IMAGEPS::updateVisiblePointsFromTables);

		//connect(ControlPointsModel, &QAbstractItemModel::layoutChanged,
		//	this, &IMAGEPS::updateVisiblePointsFromTables);
		connect(ControlPointsModel, &QAbstractItemModel::modelReset,
			this, &IMAGEPS::updateVisiblePointsFromTables);
	}

	// 上移按钮 
	connect(ui.btnUp, &QPushButton::clicked, this, [this]() {
		moveSelectedRows(ui.SmartMosaicDataList_TableW, -1); // -1表示上移
	});

	// 下移按钮 
	connect(ui.btnDown, &QPushButton::clicked, [this]() {
		moveSelectedRows(ui.SmartMosaicDataList_TableW, 1); // 1表示下移
	});

	connect(ui.pointshow_Box, QOverload<int>::of(&QComboBox::currentIndexChanged),
		[this](int index) {
		switch (index) {
		case 0: 
			ui.ImageInfoShow_Widget->setConPointsVisible(true, "-1");
			ui.ImageInfoShow_Widget->setColPointsVisible(true, "-1");
			ui.pointselect_Box->setCurrentIndex(0);
			ui.pointselect_Box->setEnabled(true);
			break;
		case 1: 
			ui.ImageInfoShow_Widget->setConPointsVisible(true, "-1");
			ui.ImageInfoShow_Widget->setColPointsVisible(false, "-1");
			ui.pointselect_Box->setCurrentIndex(1);
			ui.pointselect_Box->setEnabled(false);
			break;
		case 2: 
			ui.ImageInfoShow_Widget->setConPointsVisible(false, "-1");
			ui.ImageInfoShow_Widget->setColPointsVisible(true, "-1");
			ui.pointselect_Box->setCurrentIndex(2);
			ui.pointselect_Box->setEnabled(false);
			break;
		default:
			return;
		}
	});
}

void IMAGEPS::showWorld()
{
	//cv::Mat image = cv::imread("../bin/resource/mainWindowBackgroundImage/world.png");

	//// 目标尺寸（放大 2 倍）
	//cv::Size targetSize(image.cols * 2, image.rows * 2);

	//// 使用插值方法提升分辨率
	//cv::Mat highRes;
	//cv::resize(image, highRes, targetSize, 0, 0, cv::INTER_LANCZOS4);
	//if (image.empty()) {
	//	std::cerr << "Error: Could not read image file." << std::endl;
	//	return;
	//}

	//QImage qimage = PublicFunctions::cvMatToQImage(image);
	//setImage(qimage);

	// 使用Qt加载图片代替OpenCV 
	QImage image("../bin/resource/mainWindowBackgroundImage/world.png");

	if (image.isNull()) {
		std::cerr << "Error: Could not read image file." << std::endl;
		return;
	}

	// 目标尺寸（放大2倍） 
	QSize targetSize(image.width() * 2, image.height() * 2);

	// 使用Qt的高质量缩放算法代替OpenCV的INTER_LANCZOS4
	QImage highRes = image.scaled(targetSize,
		Qt::IgnoreAspectRatio,
		Qt::SmoothTransformation);

	setImage(highRes);

	// 获取所有菜单 
	QList<QMenu*> menus = ui.menuBar->findChildren<QMenu*>();

	// 定义需要保持可用的菜单名称
	QStringList allowedMenus = {
		QString::fromLocal8Bit("文件"),     // file_menu
		QString::fromLocal8Bit("工具"),     // tool_menu 
		QString::fromLocal8Bit("帮助"),     // help_menu
		QString::fromLocal8Bit("数据预处理"),
		QString::fromLocal8Bit("影像云雪和拉花变形替换"),
		QString::fromLocal8Bit("影像裁切软件"),
		QString::fromLocal8Bit("高程异常处理"),
		QString::fromLocal8Bit("软件工具"),
		QString::fromLocal8Bit("无效值处理"),
		QString::fromLocal8Bit("卫星数据预处理")
	};

	// 定义工具菜单中需要保持可用的子菜单名称 
	QStringList allowedToolSubmenus = {
		QString::fromLocal8Bit("导入控制点"),
		QString::fromLocal8Bit("连接点文件"),
	};

	foreach(QMenu* menu, menus) {
		QString menuName = menu->title();

		// 检查是否是允许的顶级菜单
		if (allowedMenus.contains(menuName)) {
			menu->setEnabled(true);

			// 如果是数据预处理菜单，需要特别处理其子菜单 
			if (menuName == QString::fromLocal8Bit("数据预处理")) {
				QList<QAction*> actions = menu->actions();
				foreach(QAction* action, actions) {
					// 默认禁用所有二级菜单项
					action->setEnabled(false);

					// 检查是否是"影像赋投影"子菜单项 
					QString actionText = action->text();
					if (actionText == QString::fromLocal8Bit("影像赋投影") || actionText == QString::fromLocal8Bit("卫星数据预处理")) {
						// 启用"影像赋投影"子菜单项 
						action->setEnabled(true);

						// 如果它有子菜单，启用所有三级子菜单项
						if (action->menu()) {
							QList<QAction*> subActions = action->menu()->actions();
							foreach(QAction* subAction, subActions) {
								subAction->setEnabled(true);
							}
						}
					}
				}
			}
			else {
				// 其他允许的顶级菜单(文件、帮助)，保持所有子项可用 
				QList<QAction*> actions = menu->actions();
				foreach(QAction* action, actions) {
					action->setEnabled(true);
				}
			}
		}
		else {
			// 禁用非允许的菜单及其所有子项 
			menu->setEnabled(true);
			QList<QAction*> actions = menu->actions();
			foreach(QAction* action, actions) {
				action->setEnabled(false);
			}
		}
	}

	ui.centralWidget->hide();
}

void IMAGEPS::closeWorldShow()
{
	// 恢复所有菜单项的可用状态 
	QList<QMenu*> menus = ui.menuBar->findChildren<QMenu*>();
	foreach(QMenu* menu, menus) {
		menu->setEnabled(true);
		QList<QAction*> actions = menu->actions();
		foreach(QAction* action, actions) {
			action->setEnabled(true);
			// 递归恢复子菜单状态 
			if (action->menu()) {
				QList<QAction*> subActions = action->menu()->actions();
				foreach(QAction* subAction, subActions) {
					subAction->setEnabled(true);
				}
			}
		}
	}

	clearImage();
	ui.centralWidget->show();
}

//void IMAGEPS::showWorld()
//{
//	cv::Mat image = cv::imread("../bin/resource/mainWindowBackgroundImage/world.png");
//
//	// 目标尺寸（放大 2 倍）
//	cv::Size targetSize(image.cols * 2, image.rows * 2);
//
//	// 使用插值方法提升分辨率
//	cv::Mat highRes;
//	cv::resize(image, highRes, targetSize, 0, 0, cv::INTER_LANCZOS4); // 线性插值
//	if (image.empty()) {
//		std::cerr << "Error: Could not read image file." << std::endl;
//		return;
//	}
//
//	QImage qimage = PublicFunctions::cvMatToQImage(image);
//	setImage(qimage);
//	ui.centralWidget->hide();
//}
//
//void IMAGEPS::closeWorldShow()
//{
//	clearImage();
//	ui.centralWidget->show();
//}

/**
 * @brief 清除表格内容但保留表头和列宽
 * @param tableWidget 要清除的表格指针
 * @param keepColumnWidth 是否保持当前列宽（默认保持）
 */
void IMAGEPS::clearTableWithHeader(QTableWidget* tableWidget, bool keepColumnWidth = true)
{
	if (!tableWidget) {
		qWarning() << "表格指针为空!";
		return;
	}

	// 1. 保存表头标签 
	QStringList headers;
	for (int i = 0; i < tableWidget->columnCount(); ++i) {
		if (QTableWidgetItem* headerItem = tableWidget->horizontalHeaderItem(i)) {
			headers << headerItem->text();
		}
		else {
			headers << ""; // 处理空表头的情况 
		}
	}

	// 2. 保存列宽（如果需要）
	QList<int> columnWidths;
	if (keepColumnWidth) {
		for (int i = 0; i < tableWidget->columnCount(); ++i) {
			columnWidths << tableWidget->columnWidth(i);
		}
	}

	// 3. 安全清除内容（包括释放单元格控件）
	tableWidget->setUpdatesEnabled(false); // 禁用刷新提升性能

	// 释放所有单元格widget
	for (int row = 0; row < tableWidget->rowCount(); ++row) {
		for (int col = 0; col < tableWidget->columnCount(); ++col) {
			if (QWidget* widget = tableWidget->cellWidget(row, col)) {
				widget->deleteLater();
			}
		}
	}

	tableWidget->clearContents();
	tableWidget->setRowCount(0);

	// 4. 恢复表头和列宽 
	if (!headers.isEmpty()) {
		tableWidget->setHorizontalHeaderLabels(headers);
	}

	if (keepColumnWidth && !columnWidths.isEmpty()) {
		for (int i = 0; i < columnWidths.size() && i < tableWidget->columnCount(); ++i) {
			tableWidget->setColumnWidth(i, columnWidths[i]);
		}
	}

	tableWidget->setUpdatesEnabled(true); // 恢复刷新 
}

void IMAGEPS::clearCentralWidget() {
	ui.ImageInfoShow_Widget->clearBoundaries();
	ui.ImageInfoShow_Widget->clearPointHighlights();
	ui.ImageInfoShow_Widget->showPoints(false); 
	ui.ImageInfoShow_Widget->clearHighlights();
	ui.ImageInfoShow_Widget->zoomToFullExtent();

	clearTableWithHeader(ui.refereDataList_TableW);
	clearTableWithHeader(ui.sateImageDataList_TableW);
	clearTableWithHeader(ui.PyramidDataList_TableW);
	clearTableWithHeader(ui.imageInterList_TableW);
	clearTableWithHeader(ui.dataList_TableW);
	clearTableWithHeader(ui.fusionmodelList_TableW);
	clearTableWithHeader(ui.AlignmentAdjustmentList_TableW);
	clearTableWithHeader(ui.AlignmentIntegrationList_TableW);
	clearTableWithHeader(ui.fusionmodelList_TableW_2);
	clearTableWithHeader(ui.DodgingDataList_TableW);
	clearTableWithHeader(ui.ImageMosaicDataList_TableW);
	//clearTableWithHeader(ui.encryptPointsList_TableW);
	//clearTableWithHeader(ui.controlPointsLIst_TableW);
	clearTableWithHeader(ui.MosaicCropDataList_TableW);
	clearTableWithHeader(ui.SmartMosaicDataList_TableW);
	clearTableWithHeader(ui.AbsPositPrecCheckDataList_TableW);
	clearTableWithHeader(ui.RelPositPrecCheckDataList_TableW);

	CreatepyramidPath.clear();
	DataModelPath.clear();
	DOMFilePath.clear();
	DEMFilePath.clear();
	ImageInterFilePath.clear();
	AlignmentAdFilePath.clear();
	FusionmodelFilePath.clear();
	AlignmentIntFilePath.clear();
	DodgingFilePath.clear();
	ImageMosaicFilePath.clear();
	s_imageMetadata.clear();

	systemConfig->~SystemConfig();
	systemConfig = nullptr;
	dataModelConfig->~DataModelConfig();
	dataModelConfig = nullptr;
}

bool IMAGEPS::newProActionSlot()
{
	//QString tmpprojectPath = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("选择保存位置"), "./", tr("xqp(*.xqp)"));
	QString tmpprojectPath = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("选择保存位置"), "./", tr("ps(*.ps)"));
	QFile newProFile(tmpprojectPath);
	if (tmpprojectPath.isEmpty())
		return false;
	else if (!newProFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("保存工程失败"));
		return false;
	}
	projectPath = tmpprojectPath;

	QFileInfo fileInfo(projectPath);
	QString projectname = fileInfo.completeBaseName();

	//QString projectDir = projectPath.split(".").at(0) + "_Project/";
	QString projectDir = fileInfo.dir().canonicalPath() + "/" + projectname + "_Project/";
	this->projectdir = projectDir;
	m_lastPath = projectDir;
	//qApp->setProperty("projectDir", QVariant(projectdir));
	QFileInfo filename(projectPath);
	QString filePath = "../bin/config/projectPathConfig/projectPath.csv";

	//QList<QStringList> records = {
	//	{filename.fileName(), projectDir},
	//};
	QList<QStringList> records = {
	{projectPath, projectDir},
	};

	PublicFunctions::writeToCsv(filePath, records);

	Logger projectLogger;
	CurrentConfig.setConfig({
		projectDir, // 日志目录
		projectname,          // 前缀 
		10,                  // 最大10MB 
		30,                  // 保留30天 
		true,                // 控制台输出 
		true                 // 使用系统日志
		});

	PROJECT_LOG_INFO(CurrentConfig, projectname + QString::fromLocal8Bit("工程启动"));

	//LogConfig newConfig = projectLogger.getConfig();
	
	//CurrentConfig.setConfig(newConfig);
	//this->CurrentConfig.setConfig(newConfig);

	//生成文件夹
	QMap<QString, QStringList> Map_dir;
	Map_dir["Temp"] << "CreatePyd" << "DsmMatch" << "DataModel" << "Fusion" << "ImageInfo" << "JobMoni" << "KML" << "LinuxJob" << "Ortho" << "Rasterize" << "Shp" << "Thumb";
	Map_dir["BackUp"];
	Map_dir["BandOrganize"];
	Map_dir["Cld"] << "CloudDect" << "OverView";
	Map_dir["CtlPointMatch"] << "task";
	Map_dir["DeHaze"];
	Map_dir["Dodging"];
	Map_dir["Epip"];
	Map_dir["Fusion"];
	Map_dir["GenDem"] << "DsmMatch" << "Rasterize";
	Map_dir["ImageFilter"];
	Map_dir["ImageFormatTransform"];
	Map_dir["imageHandleBusinessConfig"];
	Map_dir["ImageReproject"];
	Map_dir["ImageStretch"];
	Map_dir["Mosaic"];
	Map_dir["Ortho"];
	Map_dir["OutByte"];
	Map_dir["PrecisionCheck"] << "task";
	Map_dir["RadCorrection"] << "AtmosCorrection" << "RadCalibration";
	Map_dir["RegisteOrtho"] << "Ortho" << "Register";
	Map_dir["RegisterFusion"] << "Fusion" << "Register";
	Map_dir["SatBA"] << "NewRPCs";
	Map_dir["SatTiePointMatch"] << "Mul";
	Map_dir["SmartMosaic"];

	bool mkdirFlag = true;
	for (auto iter = Map_dir.begin(); iter != Map_dir.end(); iter++)
	{
		if (iter.value().isEmpty())
		{
			mkdirFlag = QDir().mkpath(projectDir + iter.key());
			if (!mkdirFlag)
			{
				QMessageBox::critical(this, "Error", "mkdir " + projectDir + iter.key() + " failed");
				return false;
			}
		}
		else
		{
			for (auto dir : iter.value())
			{
				mkdirFlag = QDir().mkpath(projectDir + iter.key() + "/" + dir);
				if (!mkdirFlag)
				{
					QMessageBox::critical(this, "Error", "mkdir " + projectDir + iter.key() + "/" + dir + " failed");
					return false;
				}
			}
		}
	}
	
	//生成文件
	QList<QString> List_files;
	List_files << "AtmosCorrection" << "BandOrganizeImage" << "CLDImage" << "ControlPoint" << "CreatePydImage" << "CustomFlowImage" << "DehazeImage"
		<< "DemMosaicImage" << "DodgingImage" << "DodgingMosaicImage" << "DomCorrection" << "FilterImage" << "FusionImage" << "TmpOutPath" << "ImageCrop" << "ImageCropVer" << "ImageFormatTransformImage"
		<< "ImagePoint" << "ImageRange" << "ImageReprojectImage" << "ImageStretchImage" << "MatchModel" << "MCImage" << "MosaicImage" << "ObjPoint" << "OrthoTargetShpList"
		<< "OutByteImage" << "PrecisionCheckImage" << "PSTimestamp" << "RadCalibration" << "RefImagePoint" << "RegisterFusionImage" << "RegisterRectifyImage" << "RelPrecisionCheckImage"
		<< "SarFilterImage" << "SatImage" << "SmartMosaicImage" << "StereoModelPrecisionCheckImage" << "StereoRelPrecisionCheckImage";

	QFile file;
	for (auto data : List_files)
	{
		file.setFileName(projectDir + data + ".xml");
		if (!file.open(QIODevice::WriteOnly))
		{
			QMessageBox::critical(this, "Error", "mkdir " + projectDir + data + " failed");
			return false;
		}
		file.close();
	}

	closeWorldShow();
	closeflag = true;

	Map_mainToolBarAction["openProDir_action"]->setEnabled(true);
	Map_mainToolBarAction["savePro_action"]->setEnabled(true);
	//Map_mainToolBarAction["backupsPro_action"]->setEnabled(true);
	//Map_mainToolBarAction["closePro_action"]->setEnabled(true);
	//Map_mainToolBarAction["newPro_action"]->setEnabled(false);
	//Map_mainToolBarAction["openPro_action"]->setEnabled(false);

	return true;
}

bool IMAGEPS::openProActionSlot()
{
	//QString tmpprojectPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择工程文件"), "./", tr("xqp(*.xqp)"));
	QString tmpprojectPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择工程文件"), "./", tr("ps(*.ps *.xqp)"));

	QFile newProFile(tmpprojectPath);
	if (tmpprojectPath.isEmpty())
		return false;
	else if (!newProFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("打开工程失败"));
		return false;
	}
	//QString nativePath = QDir::toNativeSeparators(tmpprojectPath);
	projectPath = tmpprojectPath;

	QFileInfo fileInfo(projectPath);
	QString projectname = fileInfo.completeBaseName();

	//QString projectDir = projectPath.split(".").at(0) + "_Project/";
	QString projectDir = fileInfo.dir().canonicalPath() + "/" + projectname + "_Project/";
	this->projectdir = projectDir;
	m_lastPath = this->projectdir;

	//Logger projectLogger(projectname);
	CurrentConfig.setConfig({
		projectDir, // 日志目录
		projectname,          // 前缀 
		10,                  // 最大10MB 
		30,                  // 保留30天 
		true,                // 控制台输出 
		true                 // 使用系统日志
		});

	PROJECT_LOG_INFO(CurrentConfig, projectname + QString::fromLocal8Bit("工程启动"));

	//LogConfig newConfig = projectLogger.getConfig();

	//CurrentConfig.setConfig(newConfig);
	//this->CurrentConfig.setConfig(newConfig);

	QDir dir(this->projectdir);
	if (!dir.exists())
	{
		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("工程文件夹不存在"));
		return false;
	}

	// 读取Satimage.xml 文件 
	QString satImageFile = this->projectdir + "Satimage.xml";
	QFile file(satImageFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error", u8"无法打开Satimage.xml 文件");
		return false;
	}
	if (file.size() != 0)
	{
		QDomDocument doc;
		if (!doc.setContent(&file))
		{
			file.close();
			QMessageBox::critical(this, "Error", u8"Satimage.xml 文件格式错误");
			return false;
		}
		file.close();

		// 解析XML内容 
		QDomElement root = doc.documentElement();

		// 1. 处理卫星影像数据 
		QDomElement pssatImages = root.firstChildElement("PSSatImages");
		if (!pssatImages.isNull())
		{
			QDomElement satImages = pssatImages.firstChildElement("SatImages");
			if (!satImages.isNull())
			{
				QDomNodeList satImageList = satImages.elementsByTagName("SatImage");
				for (int i = 0; i < satImageList.count(); i++)
				{
					QDomElement satImage = satImageList.at(i).toElement();
					QString filePath = satImage.attribute("SatImagePath");
					QString satID = satImage.attribute("SatID");
					QString sensorID = satImage.attribute("SenserID");
					double imageE = satImage.attribute("imageE").toDouble();
					QString joinBA = satImage.attribute("JoinBA");
					QString baseImage = satImage.attribute("BaseImage");
					QString joinDC = satImage.attribute("JoinDC");
					QString errorImg = satImage.attribute("ErrorImg");
					QString joinOrtho = satImage.attribute("JoinOrtho");
					QString joinFusion = satImage.attribute("JoinFusion");
					QString joinGmbMatch = satImage.attribute("JoinGmbMatch");

					// 使用sateImageDataList_TabWContextSlot中的加载逻辑 
					QFileInfo fileInfo(filePath);
					DataModelPath.push_back(filePath);

					int newRowIndex = ui.sateImageDataList_TableW->rowCount();
					ui.sateImageDataList_TableW->insertRow(newRowIndex);

					for (int col = 0; col < ui.sateImageDataList_TableW->columnCount(); col++)
					{
						if (!ui.sateImageDataList_TableW->item(newRowIndex, col))
						{
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							ui.sateImageDataList_TableW->setItem(newRowIndex, col, item);
						}
					}

					ui.sateImageDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));
					ui.sateImageDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
					ui.sateImageDataList_TableW->item(newRowIndex, 2)->setText(satID);
					ui.sateImageDataList_TableW->item(newRowIndex, 3)->setText(sensorID);
					ui.sateImageDataList_TableW->item(newRowIndex, 4)->setText(joinBA);
					ui.sateImageDataList_TableW->item(newRowIndex, 5)->setText(baseImage);
					ui.sateImageDataList_TableW->item(newRowIndex, 6)->setText(joinDC);
					ui.sateImageDataList_TableW->item(newRowIndex, 7)->setText(QString::number(imageE));
					ui.sateImageDataList_TableW->item(newRowIndex, 8)->setText(errorImg);
					ui.sateImageDataList_TableW->item(newRowIndex, 9)->setText(joinOrtho);
					ui.sateImageDataList_TableW->item(newRowIndex, 10)->setText(joinFusion);
					ui.sateImageDataList_TableW->item(newRowIndex, 11)->setText(joinGmbMatch);
				}
			}
		}
		ui.sateImageDataList_TableW->resizeColumnsToContents();
		// 2. 处理参考数据 
		QDomElement refDoms = root.firstChildElement("RefDoms");
		if (!refDoms.isNull())
		{
			QDomNodeList refDomList = refDoms.elementsByTagName("RefDom");
			for (int i = 0; i < refDomList.count(); i++)
			{
				QDomElement refDom = refDomList.at(i).toElement();
				QString filePath = refDom.attribute("DomPath");

				// 使用referDataList_TabWContextSlot中的加载逻辑 
				QFileInfo fileInfo(filePath);
				DOMFilePath.push_back(filePath);

				int newRowIndex = ui.refereDataList_TableW->rowCount();
				ui.refereDataList_TableW->insertRow(newRowIndex);

				for (int col = 0; col < ui.refereDataList_TableW->columnCount(); col++)
				{
					if (!ui.refereDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.refereDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				ui.refereDataList_TableW->item(newRowIndex, 0)->setData(Qt::DisplayRole, newRowIndex + 1);
				ui.refereDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.refereDataList_TableW->item(newRowIndex, 2)->setText("DOM");
				ui.refereDataList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是"));
			}
		}
		
		QDomElement refDems = root.firstChildElement("RefDems");
		if (!refDems.isNull())
		{
			QDomNodeList refDemList = refDems.elementsByTagName("RefDem");
			for (int i = 0; i < refDemList.count(); i++)
			{
				QDomElement refDem = refDemList.at(i).toElement();
				QString filePath = refDem.attribute("DemPath");

				// 使用referDataList_TabWContextSlot中的加载逻辑 
				QFileInfo fileInfo(filePath);
				DEMFilePath.push_back(filePath);

				int newRowIndex = ui.refereDataList_TableW->rowCount();
				ui.refereDataList_TableW->insertRow(newRowIndex);

				for (int col = 0; col < ui.refereDataList_TableW->columnCount(); col++)
				{
					if (!ui.refereDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.refereDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				ui.refereDataList_TableW->item(newRowIndex, 0)->setData(Qt::DisplayRole, newRowIndex + 1);
				ui.refereDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.refereDataList_TableW->item(newRowIndex, 2)->setText("DEM");
				ui.refereDataList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是"));
			}
		}
		ui.refereDataList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(DataModelPath);
		// 加载影像显示 
		on_actionOpenImage(DataModelPath, u8"原始影像");
		on_actionOpenImage(DOMFilePath, u8"DOM");
		on_actionOpenImage(DEMFilePath, u8"DEM");
	}
	else
	{
		file.close();
	}

	// 加载加密点数据 
	QString encPointsFile = projectdir + "PSEncPoints.xml";
	if (QFile::exists(encPointsFile)) {
		if (!readEncryptedPointsFromXml(encPointsFile)) {
			QMessageBox::warning(this, tr("Warning"), tr("Failed to load encrypted points data."));
		}
	}

	// 从XML加载控制点数据 
	QString controlPointsXmlPath = projectdir + "PSColPoints.xml";
	if (QFile::exists(controlPointsXmlPath)) {
		if (!readControlPointsFromXml(controlPointsXmlPath)) {
			QMessageBox::warning(this, tr("Warning"), tr("Failed to load control points data!"));
		}
	}

	loadCreatePydImageData();
	loadOutByteImageData();
	loadMosaicImageData();
	loadDodgingImageData();
	loadAbsPositPrecCheckImageData();
	loadCLDImageData();

	loadImageData(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath, "ImageReprojectImage.xml");
	loadImageData(ui.FormatConversionDataList_TableW, FormatConversionFilePath, "ImageFormatTransformImage.xml");
	loadImageData(ui.QuickMosaicDataList_TableW, QuickMosaicFilePath, "DemMosaicImage.xml");
	loadImageData(ui.ImagecropDataList_TableW, ImagecropFilePath, "ImageCrop.xml");
	loadImageData(ui.VectorfileDataList_TableW, VectorfileFilePath, "ImageCropVer.xml");
	loadImageData(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath, "SarFilterImage.xml");
	// 加载RegisterRectifyImage.xml 数据 
	if (!loadRegisterRectifyImageData(this->projectdir)) {
		qDebug() << u8"加载RegisterRectifyImage.xml 数据失败";
	}
	dataModelConfig->loadMatchModelToTable();
	readDataFromFusionImage(this->projectdir + "FusionImage.xml", ui.fusionmodelList_TableW);
	readSatImagesToAlignmentTable(this->projectdir + "FusionImage.xml", ui.dataList_TableW, FusionmodelFilePath);
	readDataFromFusionImage(this->projectdir + "RegisterFusionImage.xml", ui.fusionmodelList_TableW_2);
	readSatImagesToAlignmentTable(this->projectdir + "RegisterFusionImage.xml", ui.AlignmentIntegrationList_TableW, AlignmentIntFilePath);
	readSmartMosaicImage(this->projectdir + "SmartMosaicImage.xml", SmartMosaicFilePath, MosaicCropFilePath, ui.SmartMosaicDataList_TableW, ui.MosaicCropDataList_TableW);

	if (systemConfig->readMapFromXml(this->tmpcontrolPointsPath, "controlPointsPath"))
	{
		this->tmpcontrolPointsflag = true;
	}
	if (systemConfig->readMapFromXml(this->tmpimageInterPath, "imageInterPath"))
	{
		this->tmpimageInterflag = true;
	}
	
	closeWorldShow();
	closeflag = true;
	Map_mainToolBarAction["openProDir_action"]->setEnabled(true);
	Map_mainToolBarAction["savePro_action"]->setEnabled(true);
	//Map_mainToolBarAction["backupsPro_action"]->setEnabled(true);
	//Map_mainToolBarAction["closePro_action"]->setEnabled(true);
	//Map_mainToolBarAction["newPro_action"]->setEnabled(false);
	//Map_mainToolBarAction["openPro_action"]->setEnabled(false);

	return true;
}

//void IMAGEPS::openProActionSlot()
//{
//	projectPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择工程文件"), "./", tr("xqp(*.xqp)"));
//
//	QFile newProFile(projectPath);
//	if (projectPath.isEmpty())
//		return;
//	else if (!newProFile.open(QIODevice::ReadOnly | QIODevice::Text))
//	{
//		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("打开工程失败"));
//		return;
//	}
//
//	instance->projectdir = projectPath.split(".").at(0) + "_Project/";
//	QDir dir(instance->projectdir);
//	if (!dir.exists())
//	{
//		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("工程文件夹不存在"));
//		return;
//	}
//
//	//QString filePath = "../bin/config/projectPathConfig/projectPath.csv";
//	//auto fileMap = PublicFunctions::loadFromCsv(filePath);
//
//	//QFileInfo filename(projectPath);
//	//instance->projectdir = PublicFunctions::findFileDir(fileMap, filename.fileName());
//	//qApp->setProperty("projectDir", QVariant(projectdir));
//	Map_mainToolBarAction["openProDir_action"]->setEnabled(true);
//	Map_mainToolBarAction["savePro_action"]->setEnabled(true);
//	Map_mainToolBarAction["backupsPro_action"]->setEnabled(true);
//	Map_mainToolBarAction["closePro_action"]->setEnabled(true);
//
//	closeWorldShow();
//}

bool IMAGEPS::saveProActionSlot()
{
	if (projectPath.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return false;
	}

	QString projectFolder = getProjectFolderPath(projectPath);

	// 保存Satimage.xml  
	if (!writeSatImageXml(projectFolder)) {
		//QMessageBox::critical(this, "Error", u8"保存Satimage.xml 失败");
		//return false;
	}

	// 保存ControlPoint.xml  
	if (!writeControlPointXml(projectFolder)) {
		//QMessageBox::critical(this, "Error", u8"保存ControlPoint.xml 失败");
		//return false;
	}
	writeCreatePydImageXml();
	writeOutByteImageXml();
	writeMosaicImageXml();
	writeDodgingImageXml();
	writeAbsPositPrecCheckImageXml();
	writeCLDImageImageXml();
	writeImageXml(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath, "ImageReprojectImage.xml");
	writeImageXml(ui.FormatConversionDataList_TableW, FormatConversionFilePath, "ImageFormatTransformImage.xml");
	writeImageXml(ui.QuickMosaicDataList_TableW, QuickMosaicFilePath, "DemMosaicImage.xml");
	writeImageXml(ui.ImagecropDataList_TableW, ImagecropFilePath, "ImageCrop.xml");
	writeImageXml(ui.VectorfileDataList_TableW, VectorfileFilePath, "ImageCropVer.xml");
	writeImageXml(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath, "SarFilterImage.xml");
	
	// 保存RegisterRectifyImage.xml  
	if (!writeRegisterRectifyImageXml(projectFolder)) {
		//QMessageBox::critical(this, "Error", u8"保存RegisterRectifyImage.xml 失败");
		//return false;
	}
	dataModelConfig->writeMatchModelFromSatModelMakerCmdout();

	if (ui.fusionmodelList_TableW->rowCount() > 0) {
		// 表格有数据才执行写入操作
		writeDataToFusionImage(
			projectFolder + "/" + "FusionImage.xml",
			projectFolder + "/" + "Fusion" + "/" + "SatModelMakerCmdout.xml",
			FusionmodelFilePath,
			ui.dataList_TableW,
			ui.fusionmodelList_TableW
		);
	}
	else {
		PROJECT_LOG_INFO(this->CurrentConfig, QString::fromLocal8Bit("影像融合配对列表无数据,不保存到文件 "));
	}
	//writeDataToFusionImage(projectFolder + "/" + "FusionImage.xml", projectFolder + "/" + "Fusion" + "/" + "SatModelMakerCmdout.xml", FusionmodelFilePath, ui.dataList_TableW, ui.fusionmodelList_TableW);
	writeDataToFusionImage(projectFolder + "/" + "RegisterFusionImage.xml", projectFolder + "/" + "RegisterFusion" + "/" + "SatModelMakerCmdout.xml", AlignmentIntFilePath, ui.AlignmentIntegrationList_TableW, ui.fusionmodelList_TableW_2);
	writeSmartMosaicImage(projectFolder + "/" + "SmartMosaicImage.xml", SmartMosaicFilePath, MosaicCropFilePath,ui.SmartMosaicDataList_TableW, ui.MosaicCropDataList_TableW);

	// 保存加密点数据 
	QString encPointsFile = projectdir + "PSEncPoints.xml";
	if (!writeEncryptedPointsToXml(encPointsFile)) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to save encrypted points data."));
	}

	// 保存控制点数据到XML
	QString controlPointsXmlPath = projectdir + "PSColPoints.xml";
	if (!writeControlPointsToXml(controlPointsXmlPath)) {
		QMessageBox::warning(this, tr("Warning"), tr("Failed to save control points data!"));
		return false;
	}

	m_isModified = false;
	setWindowModified(false);
	QMessageBox::information(this, "Success", u8"工程保存成功");
	return true;
}

void IMAGEPS::loadRecentFiles()
{
	QString filePath = "../bin/config/projectPathConfig/projectPath.csv";
	QHash<QString, QString> fileMap = PublicFunctions::loadFromCsv(filePath);

	QMenu* fileMenu = nullptr;
	for (QAction* action : ui.menuBar->actions()) {
		if (action->text() == settings->value("mainToolBarSetting/file_menuName").toString()) {
			fileMenu = action->menu();
			break;
		}
	}

	if (!fileMenu) {
		qDebug() << "找不到file_menu菜单";
		return;
	}

	QMenu* recentMenu = nullptr;
	QString recentActionName = settings->value("file_menuSetting/recentOpen_actionName").toString();
	foreach(QAction* action, fileMenu->actions()) {
		if (action->text() == recentActionName) {
			recentMenu = action->menu();
			break;
		}
	}

	QAction* quitAction = nullptr;
	QString quitActionName = settings->value("file_menuSetting/quit_actionName").toString();
	foreach(QAction* action, fileMenu->actions()) {
		if (action->text() == quitActionName) {
			quitAction = action;
			break;
		}
	}
	if (!recentMenu) {
		recentMenu = fileMenu->addMenu(settings->value("file_menuSetting/recentOpen_actionName").toString());

		if (quitAction) {
			fileMenu->insertMenu(quitAction, recentMenu);
		}
		else {
			fileMenu->addMenu(recentMenu);
		}
	}
	else {
		recentMenu->clear();
	}

	int count = 0;
	const int maxRecentFiles = 5; 

	for (auto it = fileMap.begin(); it != fileMap.end() && count < maxRecentFiles; ++it, ++count) {
		QString fileName = it.key();
		QString filePath = it.value();

		QAction* fileAction = new QAction(fileName, this);
		fileAction->setData(filePath); // 存储完整路径

		// 连接点击信号到槽函数
		connect(fileAction, &QAction::triggered, this, [this, filePath, fileName]() {
			//openRecentProject(filePath, fileName);
			updateRecentFiles(filePath, fileName);
		});

		recentMenu->addAction(fileAction);
	}

	if (fileMap.isEmpty()) {
		QAction* noFilesAction = new QAction(tr(u8"无最近文件"), this);
		noFilesAction->setEnabled(false);
		recentMenu->addAction(noFilesAction);
	}

	recentMenu->addSeparator();

}

void IMAGEPS::updateRecentFiles(const QString& projectPath, const QString& projectPathname)
{
	// 2. 读取现有最近项目 
	QString csvPath = "../bin/config/projectPathConfig/projectPath.csv";
	QHash<QString, QString> fileMap = PublicFunctions::loadFromCsv(csvPath);

	// 3. 移除重复项（如果存在）
	fileMap.remove(projectPathname);

	// 4. 创建新的有序映射（保持最近使用的在前面）
	QHash<QString, QString> newFileMap;
	newFileMap.insert(projectPathname, projectPath); // 新项目放在最前面 

	// 5. 添加原有项目（不超过最大限制）
	const int maxRecentFiles = 5;
	int count = 1;
	for (auto it = fileMap.begin(); it != fileMap.end() && count < maxRecentFiles; ++it, ++count) {
		newFileMap.insert(it.key(), it.value());
	}

	// 6. 保存更新后的CSV文件 
	QFile file(csvPath);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QTextStream out(&file);
		out.setCodec("UTF-8");

		for (auto it = newFileMap.begin(); it != newFileMap.end(); ++it) {
			out << it.key() << "," << it.value() << "\n";
		}

		file.close();
	}

	// 7. 更新菜单显示
	loadRecentFiles();
}

bool IMAGEPS::openRecentProject(const QString& projectPath_Recent)
{
	//QString tmpprojectPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择工程文件"), "./", tr("xqp(*.xqp)"));
	QString tmpprojectPath = projectPath_Recent;

	QFile newProFile(tmpprojectPath);
	if (tmpprojectPath.isEmpty())
		return false;
	else if (!newProFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("打开工程失败"));
		return false;
	}
	//QString nativePath = QDir::toNativeSeparators(tmpprojectPath);
	projectPath = tmpprojectPath;
	QFileInfo fileInfo(projectPath);
	QString projectname = fileInfo.completeBaseName();

	//instance->projectdir = projectPath.split(".").at(0) + "_Project" + "/";
	this->projectdir = fileInfo.dir().canonicalPath() + "/" + projectname + "_Project/";;
	m_lastPath = this->projectdir;

	//Logger projectLogger(projectname);
	CurrentConfig.setConfig({
		this->projectdir, // 日志目录
		projectname,          // 前缀 
		10,                  // 最大10MB 
		30,                  // 保留30天 
		true,                // 控制台输出 
		true                 // 使用系统日志
		});

	PROJECT_LOG_INFO(CurrentConfig, projectname + QString::fromLocal8Bit(" 工程启动"));

	//LogConfig newConfig = projectLogger.getConfig();

	//CurrentConfig.setConfig(newConfig);
	//this->CurrentConfig.setConfig(newConfig);

	QDir dir(this->projectdir);
	if (!dir.exists())
	{
		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("工程文件夹不存在"));
		return false;
	}

	// 读取Satimage.xml 文件 
	QString satImageFile = this->projectdir + "Satimage.xml";
	QFile file(satImageFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(this, "Error", u8"无法打开Satimage.xml 文件");
		return false;
	}
	if (file.size() != 0)
	{
		QDomDocument doc;
		if (!doc.setContent(&file))
		{
			file.close();
			QMessageBox::critical(this, "Error", u8"Satimage.xml 文件格式错误");
			return false;
		}
		file.close();

		// 解析XML内容 
		QDomElement root = doc.documentElement();

		// 1. 处理卫星影像数据 
		QDomElement pssatImages = root.firstChildElement("PSSatImages");
		if (!pssatImages.isNull())
		{
			QDomElement satImages = pssatImages.firstChildElement("SatImages");
			if (!satImages.isNull())
			{
				QDomNodeList satImageList = satImages.elementsByTagName("SatImage");
				for (int i = 0; i < satImageList.count(); i++)
				{
					QDomElement satImage = satImageList.at(i).toElement();
					QString filePath = satImage.attribute("SatImagePath");
					QString satID = satImage.attribute("SatID");
					QString sensorID = satImage.attribute("SenserID");
					double imageE = satImage.attribute("imageE").toDouble();
					QString joinBA = satImage.attribute("JoinBA");
					QString baseImage = satImage.attribute("BaseImage");
					QString joinDC = satImage.attribute("JoinDC");
					QString errorImg = satImage.attribute("ErrorImg");
					QString joinOrtho = satImage.attribute("JoinOrtho");
					QString joinFusion = satImage.attribute("JoinFusion");
					QString joinGmbMatch = satImage.attribute("JoinGmbMatch");

					// 使用sateImageDataList_TabWContextSlot中的加载逻辑 
					QFileInfo fileInfo(filePath);
					DataModelPath.push_back(filePath);

					int newRowIndex = ui.sateImageDataList_TableW->rowCount();
					ui.sateImageDataList_TableW->insertRow(newRowIndex);

					for (int col = 0; col < ui.sateImageDataList_TableW->columnCount(); col++)
					{
						if (!ui.sateImageDataList_TableW->item(newRowIndex, col))
						{
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							ui.sateImageDataList_TableW->setItem(newRowIndex, col, item);
						}
					}

					ui.sateImageDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));
					ui.sateImageDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
					ui.sateImageDataList_TableW->item(newRowIndex, 2)->setText(satID);
					ui.sateImageDataList_TableW->item(newRowIndex, 3)->setText(sensorID);
					ui.sateImageDataList_TableW->item(newRowIndex, 4)->setText(joinBA);
					ui.sateImageDataList_TableW->item(newRowIndex, 5)->setText(baseImage);
					ui.sateImageDataList_TableW->item(newRowIndex, 6)->setText(joinDC);
					ui.sateImageDataList_TableW->item(newRowIndex, 7)->setText(QString::number(imageE));
					ui.sateImageDataList_TableW->item(newRowIndex, 8)->setText(errorImg);
					ui.sateImageDataList_TableW->item(newRowIndex, 9)->setText(joinOrtho);
					ui.sateImageDataList_TableW->item(newRowIndex, 10)->setText(joinFusion);
					ui.sateImageDataList_TableW->item(newRowIndex, 11)->setText(joinGmbMatch);
				}
			}
		}
		ui.sateImageDataList_TableW->resizeColumnsToContents();
		// 2. 处理参考数据 
		QDomElement refDoms = root.firstChildElement("RefDoms");
		if (!refDoms.isNull())
		{
			QDomNodeList refDomList = refDoms.elementsByTagName("RefDom");
			for (int i = 0; i < refDomList.count(); i++)
			{
				QDomElement refDom = refDomList.at(i).toElement();
				QString filePath = refDom.attribute("DomPath");

				// 使用referDataList_TabWContextSlot中的加载逻辑 
				QFileInfo fileInfo(filePath);
				DOMFilePath.push_back(filePath);

				int newRowIndex = ui.refereDataList_TableW->rowCount();
				ui.refereDataList_TableW->insertRow(newRowIndex);

				for (int col = 0; col < ui.refereDataList_TableW->columnCount(); col++)
				{
					if (!ui.refereDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.refereDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				ui.refereDataList_TableW->item(newRowIndex, 0)->setData(Qt::DisplayRole, newRowIndex + 1);
				ui.refereDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.refereDataList_TableW->item(newRowIndex, 2)->setText("DOM");
				ui.refereDataList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是"));
			}
		}

		QDomElement refDems = root.firstChildElement("RefDems");
		if (!refDems.isNull())
		{
			QDomNodeList refDemList = refDems.elementsByTagName("RefDem");
			for (int i = 0; i < refDemList.count(); i++)
			{
				QDomElement refDem = refDemList.at(i).toElement();
				QString filePath = refDem.attribute("DemPath");

				// 使用referDataList_TabWContextSlot中的加载逻辑 
				QFileInfo fileInfo(filePath);
				DEMFilePath.push_back(filePath);

				int newRowIndex = ui.refereDataList_TableW->rowCount();
				ui.refereDataList_TableW->insertRow(newRowIndex);

				for (int col = 0; col < ui.refereDataList_TableW->columnCount(); col++)
				{
					if (!ui.refereDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.refereDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				ui.refereDataList_TableW->item(newRowIndex, 0)->setData(Qt::DisplayRole, newRowIndex + 1);
				ui.refereDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.refereDataList_TableW->item(newRowIndex, 2)->setText("DEM");
				ui.refereDataList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是"));
			}
		}
		ui.refereDataList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(DataModelPath);
		// 加载影像显示 
		on_actionOpenImage(DataModelPath, u8"原始影像");
		on_actionOpenImage(DOMFilePath, u8"DOM");
		on_actionOpenImage(DEMFilePath, u8"DEM");
	}
	else
	{
		file.close();
	}

	// 加载加密点数据 
	QString encPointsFile = projectdir + "PSEncPoints.xml";
	if (QFile::exists(encPointsFile)) {
		if (!readEncryptedPointsFromXml(encPointsFile)) {
			QMessageBox::warning(this, tr("Warning"), tr("Failed to load encrypted points data."));
		}
	}

	// 从XML加载控制点数据 
	QString controlPointsXmlPath = projectdir + "PSColPoints.xml";
	if (QFile::exists(controlPointsXmlPath)) {
		if (!readControlPointsFromXml(controlPointsXmlPath)) {
			QMessageBox::warning(this, tr("Warning"), tr("Failed to load control points data!"));
		}
	}

	loadCreatePydImageData();
	loadOutByteImageData();
	loadMosaicImageData();
	loadDodgingImageData();
	loadAbsPositPrecCheckImageData();
	loadCLDImageData();

	loadImageData(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath, "ImageReprojectImage.xml");
	loadImageData(ui.FormatConversionDataList_TableW, FormatConversionFilePath, "ImageFormatTransformImage.xml");
	loadImageData(ui.QuickMosaicDataList_TableW, QuickMosaicFilePath, "DemMosaicImage.xml");
	loadImageData(ui.ImagecropDataList_TableW, ImagecropFilePath, "ImageCrop.xml");
	loadImageData(ui.VectorfileDataList_TableW, VectorfileFilePath, "ImageCropVer.xml");
	loadImageData(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath, "SarFilterImage.xml");
	// 加载RegisterRectifyImage.xml 数据 
	if (!loadRegisterRectifyImageData(this->projectdir)) {
		qDebug() << u8"加载RegisterRectifyImage.xml 数据失败";
	}
	dataModelConfig->loadMatchModelToTable();
	readDataFromFusionImage(this->projectdir + "FusionImage.xml", ui.fusionmodelList_TableW);
	readSatImagesToAlignmentTable(this->projectdir + "FusionImage.xml", ui.dataList_TableW, FusionmodelFilePath);
	readDataFromFusionImage(this->projectdir + "RegisterFusionImage.xml", ui.fusionmodelList_TableW_2);
	readSatImagesToAlignmentTable(this->projectdir + "RegisterFusionImage.xml", ui.AlignmentIntegrationList_TableW, AlignmentIntFilePath);
	readSmartMosaicImage(this->projectdir + "SmartMosaicImage.xml", SmartMosaicFilePath, MosaicCropFilePath, ui.SmartMosaicDataList_TableW, ui.MosaicCropDataList_TableW);

	if (systemConfig->readMapFromXml(this->tmpcontrolPointsPath, "controlPointsPath"))
	{
		this->tmpcontrolPointsflag = true;
	}
	if (systemConfig->readMapFromXml(this->tmpimageInterPath, "imageInterPath"))
	{
		this->tmpimageInterflag = true;
	}

	closeWorldShow();
	closeflag = true;
	Map_mainToolBarAction["openProDir_action"]->setEnabled(true);
	Map_mainToolBarAction["savePro_action"]->setEnabled(true);
	//Map_mainToolBarAction["backupsPro_action"]->setEnabled(true);
	//Map_mainToolBarAction["closePro_action"]->setEnabled(true);
	//Map_mainToolBarAction["newPro_action"]->setEnabled(false);
	//Map_mainToolBarAction["openPro_action"]->setEnabled(false);

	return true;
}

//void IMAGEPS::openRecentProject(const QString& projectPath_Recent, const QString& projectPathname)
//{
//	clearCentralWidget();
//
//	//initWidget();
//	//connects();
//
//	if (!systemConfig)
//	{
//		systemConfig = new SystemConfig;
//	}
//
//	if (!dataModelConfig)
//	{
//		dataModelConfig = new DataModelConfig;
//	}
//	OrthoRectificationFusionflag = false;
//	closeflag = false;
//
//	QDir dir_Recent(projectPath_Recent);
//	dir_Recent.cdUp();  // 切换到父目录
//	QString parentPath = dir_Recent.absolutePath() + "/";
//	projectPath = parentPath + projectPathname;
//
//	QFile newProFile(projectPath);
//	if (projectPath.isEmpty())
//		return;
//	else if (!newProFile.open(QIODevice::ReadOnly | QIODevice::Text))
//	{
//		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("打开工程失败"));
//		return;
//	}
//
//	instance->projectdir = projectPath.split(".").at(0) + "_Project/";
//	m_lastPath = instance->projectdir;
//	QDir dir(instance->projectdir);
//	if (!dir.exists())
//	{
//		QMessageBox::critical(this, "Error", QString::fromLocal8Bit("工程文件夹不存在"));
//		return;
//	}
//
//	// 读取Satimage.xml 文件 
//	QString satImageFile = instance->projectdir + "Satimage.xml";
//	QFile file(satImageFile);
//	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
//	{
//		QMessageBox::critical(this, "Error", u8"无法打开Satimage.xml 文件");
//		return;
//	}
//	if (file.size() != 0)
//	{
//		QDomDocument doc;
//		if (!doc.setContent(&file))
//		{
//			file.close();
//			QMessageBox::critical(this, "Error", u8"Satimage.xml 文件格式错误");
//			return;
//		}
//		file.close();
//
//		// 解析XML内容 
//		QDomElement root = doc.documentElement();
//
//		// 1. 处理卫星影像数据 
//		QDomElement pssatImages = root.firstChildElement("PSSatImages");
//		if (!pssatImages.isNull())
//		{
//			QDomElement satImages = pssatImages.firstChildElement("SatImages");
//			if (!satImages.isNull())
//			{
//				QDomNodeList satImageList = satImages.elementsByTagName("SatImage");
//				for (int i = 0; i < satImageList.count(); i++)
//				{
//					QDomElement satImage = satImageList.at(i).toElement();
//					QString filePath = satImage.attribute("SatImagePath");
//					QString satID = satImage.attribute("SatID");
//					QString sensorID = satImage.attribute("SenserID");
//					double imageE = satImage.attribute("imageE").toDouble();
//					QString joinBA = satImage.attribute("JoinBA");
//					QString baseImage = satImage.attribute("BaseImage");
//					QString joinDC = satImage.attribute("JoinDC");
//					QString errorImg = satImage.attribute("ErrorImg");
//					QString joinOrtho = satImage.attribute("JoinOrtho");
//					QString joinFusion = satImage.attribute("JoinFusion");
//					QString joinGmbMatch = satImage.attribute("JoinGmbMatch");
//
//					// 使用sateImageDataList_TabWContextSlot中的加载逻辑 
//					QFileInfo fileInfo(filePath);
//					DataModelPath.push_back(filePath);
//
//					int newRowIndex = ui.sateImageDataList_TableW->rowCount();
//					ui.sateImageDataList_TableW->insertRow(newRowIndex);
//
//					for (int col = 0; col < ui.sateImageDataList_TableW->columnCount(); col++)
//					{
//						if (!ui.sateImageDataList_TableW->item(newRowIndex, col))
//						{
//							QTableWidgetItem* item = new QTableWidgetItem;
//							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//							ui.sateImageDataList_TableW->setItem(newRowIndex, col, item);
//						}
//					}
//
//					ui.sateImageDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));
//					ui.sateImageDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
//					ui.sateImageDataList_TableW->item(newRowIndex, 2)->setText(satID);
//					ui.sateImageDataList_TableW->item(newRowIndex, 3)->setText(sensorID);
//					ui.sateImageDataList_TableW->item(newRowIndex, 4)->setText(joinBA);
//					ui.sateImageDataList_TableW->item(newRowIndex, 5)->setText(baseImage);
//					ui.sateImageDataList_TableW->item(newRowIndex, 6)->setText(joinDC);
//					ui.sateImageDataList_TableW->item(newRowIndex, 7)->setText(QString::number(imageE));
//					ui.sateImageDataList_TableW->item(newRowIndex, 8)->setText(errorImg);
//					ui.sateImageDataList_TableW->item(newRowIndex, 9)->setText(joinOrtho);
//					ui.sateImageDataList_TableW->item(newRowIndex, 10)->setText(joinFusion);
//					ui.sateImageDataList_TableW->item(newRowIndex, 11)->setText(joinGmbMatch);
//				}
//			}
//		}
//		ui.sateImageDataList_TableW->resizeColumnsToContents();
//		// 2. 处理参考数据 
//		QDomElement refDoms = root.firstChildElement("RefDoms");
//		if (!refDoms.isNull())
//		{
//			QDomNodeList refDomList = refDoms.elementsByTagName("RefDom");
//			for (int i = 0; i < refDomList.count(); i++)
//			{
//				QDomElement refDom = refDomList.at(i).toElement();
//				QString filePath = refDom.attribute("DomPath");
//
//				// 使用referDataList_TabWContextSlot中的加载逻辑 
//				QFileInfo fileInfo(filePath);
//				DOMFilePath.push_back(filePath);
//
//				int newRowIndex = ui.refereDataList_TableW->rowCount();
//				ui.refereDataList_TableW->insertRow(newRowIndex);
//
//				for (int col = 0; col < ui.refereDataList_TableW->columnCount(); col++)
//				{
//					if (!ui.refereDataList_TableW->item(newRowIndex, col))
//					{
//						QTableWidgetItem* item = new QTableWidgetItem;
//						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//						ui.refereDataList_TableW->setItem(newRowIndex, col, item);
//					}
//				}
//
//				ui.refereDataList_TableW->item(newRowIndex, 0)->setData(Qt::DisplayRole, newRowIndex + 1);
//				ui.refereDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
//				ui.refereDataList_TableW->item(newRowIndex, 2)->setText("DOM");
//				ui.refereDataList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是"));
//			}
//		}
//
//		QDomElement refDems = root.firstChildElement("RefDems");
//		if (!refDems.isNull())
//		{
//			QDomNodeList refDemList = refDems.elementsByTagName("RefDem");
//			for (int i = 0; i < refDemList.count(); i++)
//			{
//				QDomElement refDem = refDemList.at(i).toElement();
//				QString filePath = refDem.attribute("DemPath");
//
//				// 使用referDataList_TabWContextSlot中的加载逻辑 
//				QFileInfo fileInfo(filePath);
//				DEMFilePath.push_back(filePath);
//
//				int newRowIndex = ui.refereDataList_TableW->rowCount();
//				ui.refereDataList_TableW->insertRow(newRowIndex);
//
//				for (int col = 0; col < ui.refereDataList_TableW->columnCount(); col++)
//				{
//					if (!ui.refereDataList_TableW->item(newRowIndex, col))
//					{
//						QTableWidgetItem* item = new QTableWidgetItem;
//						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
//						ui.refereDataList_TableW->setItem(newRowIndex, col, item);
//					}
//				}
//
//				ui.refereDataList_TableW->item(newRowIndex, 0)->setData(Qt::DisplayRole, newRowIndex + 1);
//				ui.refereDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
//				ui.refereDataList_TableW->item(newRowIndex, 2)->setText("DEM");
//				ui.refereDataList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是"));
//			}
//		}
//		ui.refereDataList_TableW->resizeColumnsToContents();
//		Map_mainToolBarAction["openProDir_action"]->setEnabled(true);
//		Map_mainToolBarAction["savePro_action"]->setEnabled(true);
//		//Map_mainToolBarAction["backupsPro_action"]->setEnabled(true);
//		//Map_mainToolBarAction["closePro_action"]->setEnabled(true);
//		Map_mainToolBarAction["newPro_action"]->setEnabled(false);
//		Map_mainToolBarAction["openPro_action"]->setEnabled(false);
//
//		setGeoreferenceFromXMLAndRPC(DataModelPath);
//		// 加载影像显示 
//		on_actionOpenImage(DataModelPath, u8"原始影像");
//		on_actionOpenImage(DOMFilePath, u8"DOM");
//		on_actionOpenImage(DEMFilePath, u8"DEM");
//	}
//	else
//	{
//		file.close();
//	}
//
//	QFile markerFile(instance->projectdir + "ControlPoint.xml");
//	if (!markerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//		qDebug() << "Failed to open ControlPoint.xml  for reading";
//		return;
//	}
//	if (markerFile.size() != 0)
//	{
//		// 使用 QXmlStreamReader 解析 XML 
//		QXmlStreamReader xml(&markerFile);
//
//		// 定义存储时间的变量 
//		QDateTime connectTime, colTime, dataModelTime;
//		QString timeFormat = "yyyy-MM-dd  hh:mm:ss"; // 注意：日期和时间之间有2个空格 
//
//		while (!xml.atEnd() && !xml.hasError()) {
//			xml.readNext();
//
//			if (xml.isStartElement()) {
//				// 读取 SatTiePointMatch 时间 
//				if (xml.name() == "SatTiePointMatch") {
//					QString timeStr = xml.readElementText();
//					connectTime = QDateTime::fromString(timeStr, timeFormat);
//					if (!connectTime.isValid()) {
//						qDebug() << "Invalid SatTiePointMatch time:" << timeStr;
//					}
//				}
//				// 读取 CtlPointMatch 时间 
//				else if (xml.name() == "CtlPointMatch") {
//					QString timeStr = xml.readElementText();
//					colTime = QDateTime::fromString(timeStr, timeFormat);
//					if (!colTime.isValid()) {
//						qDebug() << "Invalid CtlPointMatch time:" << timeStr;
//					}
//				}
//				// 读取 DataModelmarker 时间 
//				else if (xml.name() == "DataModelmarker") {
//					QString timeStr = xml.readElementText();
//					dataModelTime = QDateTime::fromString(timeStr, timeFormat);
//					if (!dataModelTime.isValid()) {
//						qDebug() << "Invalid DataModelmarker time:" << timeStr;
//					}
//				}
//			}
//		}
//
//		// 检查解析错误 
//		if (xml.hasError()) {
//			qDebug() << "XML error:" << xml.errorString();
//		}
//
//		markerFile.close();
//
//		//// 调用四个函数，并传入时间参数 
//		//EncryptedPointsList(connectTime);
//		//ControlPointsList(colTime);
//		//EncreadResidualData(connectTime);
//		//ConResidualFiles(colTime);
//	}
//	else
//	{
//		markerFile.close();
//	}
//
//	loadCreatePydImageData();
//	loadOutByteImageData();
//	loadMosaicImageData();
//	loadDodgingImageData();
//	loadAbsPositPrecCheckImageData();
//	loadCLDImageData();
//
//	loadImageData(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath, "ImageReprojectImage.xml");
//	loadImageData(ui.FormatConversionDataList_TableW, FormatConversionFilePath, "ImageFormatTransformImage.xml");
//	loadImageData(ui.QuickMosaicDataList_TableW, QuickMosaicFilePath, "DemMosaicImage.xml");
//	loadImageData(ui.ImagecropDataList_TableW, ImagecropFilePath, "ImageCrop.xml");
//	loadImageData(ui.VectorfileDataList_TableW, VectorfileFilePath, "ImageCropVer.xml");
//	loadImageData(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath, "SarFilterImage.xml");
//	// 加载RegisterRectifyImage.xml 数据 
//	if (!loadRegisterRectifyImageData(instance->projectdir)) {
//		qDebug() << u8"加载RegisterRectifyImage.xml 数据失败";
//	}
//
//
//	Map_mainToolBarAction["openProDir_action"]->setEnabled(true);
//	Map_mainToolBarAction["savePro_action"]->setEnabled(true);
//	//Map_mainToolBarAction["backupsPro_action"]->setEnabled(true);
//	//Map_mainToolBarAction["closePro_action"]->setEnabled(true);
//
//	dataModelConfig->loadMatchModelToTable();
//	readDataFromFusionImage(instance->projectdir + "FusionImage.xml", ui.fusionmodelList_TableW);
//	readSatImagesToAlignmentTable(instance->projectdir + "FusionImage.xml", ui.dataList_TableW, FusionmodelFilePath);
//	readDataFromFusionImage(instance->projectdir + "RegisterFusionImage.xml", ui.fusionmodelList_TableW_2);
//	readSatImagesToAlignmentTable(instance->projectdir + "RegisterFusionImage.xml", ui.AlignmentIntegrationList_TableW, AlignmentIntFilePath);
//
//	if (systemConfig->readMapFromXml(this->tmpcontrolPointsPath, "controlPointsPath"))
//	{
//		this->tmpcontrolPointsflag = true;
//	}
//	if (systemConfig->readMapFromXml(this->tmpimageInterPath, "imageInterPath"))
//	{
//		this->tmpimageInterflag = true;
//	}
//
//	closeWorldShow();
//	closeflag = true;
//}

void IMAGEPS::sortNumericColumn(QTableWidget* table, int column, Qt::SortOrder order)
{
	if (!table || column < 0 || column >= table->columnCount())
		return;

	// 1. 收集所有行数据（安全方式）
	struct RowData {
		bool isNumeric;
		double numericValue;
		QString textValue;
		QString firstColumnValue;  // 新增：存储第1列的值
		int originalRow;
		QList<QVariant> columnData;
	};

	QList<RowData> sortableData;
	int rowCount = table->rowCount();
	int colCount = table->columnCount();

	// 判断是否需要复合排序 
	bool useCompositeSort = (table == ui.sateImageDataList_TableW && column == 3); // 第3列(索引2)

	// 先收集所有数据，不操作实际表格项 
	for (int row = 0; row < rowCount; ++row) {
		RowData rd;
		rd.originalRow = row;

		// 收集整行数据 
		for (int col = 0; col < colCount; ++col) {
			QTableWidgetItem* item = table->item(row, col);
			rd.columnData.append(item ? item->text() : QVariant());
		}

		// 存储第1列的值
		QTableWidgetItem* firstColItem = table->item(row, 1);
		rd.firstColumnValue = firstColItem ? firstColItem->text() : "";

		// 处理排序列
		QTableWidgetItem* sortItem = table->item(row, column);
		if (!sortItem || sortItem->text().trimmed().isEmpty()) {
			rd.isNumeric = false;
			rd.numericValue = 0.0;
			rd.textValue = "";
		}
		else {
			QString text = sortItem->text();
			bool ok;
			double value = text.toDouble(&ok);
			rd.isNumeric = ok;
			rd.numericValue = value;
			rd.textValue = text;
		}

		sortableData.append(rd);
	}

	// 2. 排序逻辑 
	auto comparator = [order, useCompositeSort](const RowData& a, const RowData& b) {
		// 空行排在最后 
		bool aEmpty = !a.isNumeric  && a.textValue.isEmpty();
		bool bEmpty = !b.isNumeric  && b.textValue.isEmpty();

		if (aEmpty && bEmpty) return false;
		if (aEmpty) return false;
		if (bEmpty) return true;

		// 复合排序逻辑 
		if (useCompositeSort) {
			// 先比较第3列
			if (a.isNumeric != b.isNumeric) {
				return a.isNumeric > b.isNumeric;
			}
			if (a.isNumeric) {
				if (a.numericValue != b.numericValue) {
					return order == Qt::AscendingOrder ?
						a.numericValue  < b.numericValue :
						a.numericValue  > b.numericValue;
				}
			}
			else {
				int textCompare = a.textValue.compare(b.textValue, Qt::CaseInsensitive);
				if (textCompare != 0) {
					return order == Qt::AscendingOrder ? textCompare < 0 : textCompare > 0;
				}
			}

			// 第3列相同的情况下，比较第1列
			return a.firstColumnValue.compare(b.firstColumnValue, Qt::CaseInsensitive) < 0;
		}

		// 原有排序逻辑 
		if (a.isNumeric != b.isNumeric) {
			return a.isNumeric > b.isNumeric;
		}
		if (a.isNumeric) {
			return order == Qt::AscendingOrder ?
				a.numericValue  < b.numericValue :
				a.numericValue  > b.numericValue;
		}
		return order == Qt::AscendingOrder ?
			a.textValue.compare(b.textValue, Qt::CaseInsensitive) < 0 :
			a.textValue.compare(b.textValue, Qt::CaseInsensitive) > 0;
	};

	std::stable_sort(sortableData.begin(), sortableData.end(), comparator);

	// 3. 完全重建表格（最安全的方式）
	table->blockSignals(true);  // 禁止信号，提高性能 
	table->clearContents();
	table->setRowCount(rowCount);

	for (int newRow = 0; newRow < sortableData.size(); ++newRow) {
		const auto& rd = sortableData[newRow];

		for (int col = 0; col < colCount; ++col) {
			QTableWidgetItem* newItem = new QTableWidgetItem();
			newItem->setText(rd.columnData[col].toString());

			// 恢复基本属性（可根据需要添加更多）
			QTableWidgetItem* origItem = table->item(rd.originalRow, col);
			if (origItem) {
				newItem->setTextAlignment(origItem->textAlignment());
				newItem->setBackground(origItem->background());
				newItem->setForeground(origItem->foreground());
				newItem->setFlags(origItem->flags());
			}

			table->setItem(newRow, col, newItem);
		}
	}
	table->blockSignals(false);  // 恢复信号 
}

void IMAGEPS::sortNumericColumn(QTableView* tableView, int column, Qt::SortOrder order)
{
	if (!tableView || column < 0)
		return;

	NonEditableModel* model = qobject_cast<NonEditableModel*>(tableView->model());
	if (!model) return;

	// 禁用视图更新 
	tableView->setUpdatesEnabled(false);

	// 获取当前模型数据 
	int rowCount = model->rowCount();
	int colCount = model->columnCount();

	if (rowCount == 0 || column >= colCount) {
		tableView->setUpdatesEnabled(true);
		return;
	}

	// 1. 收集所有行数据 
	struct RowData {
		bool isNumeric;
		double numericValue;
		QString textValue;
		QList<QVariant> originalRowData;
	};

	QList<RowData> allRowData;
	allRowData.reserve(rowCount);

	for (int row = 0; row < rowCount; ++row) {
		RowData rd;

		// 获取排序列的数据 
		QModelIndex sortIndex = model->index(row, column);
		QString text = model->data(sortIndex).toString().trimmed();

		//if (text.isEmpty()) {
		//	rd.isNumeric = false;
		//	rd.numericValue = 0.0;
		//}
		//else {
			bool ok;
			double value = text.toDouble(&ok);
			rd.isNumeric = ok;
			rd.numericValue = value;
			rd.textValue = text;

			// 收集整行数据 
			for (int col = 0; col < colCount; ++col) {
				QModelIndex index = model->index(row, col);
				rd.originalRowData.append(model->data(index));
			}

			allRowData.append(rd);
		//}
	}
	qDebug() << allRowData.size();
	// 2. 排序逻辑 
	auto comparator = [order](const RowData& a, const RowData& b) {
		// 空值排在最后 
		bool aEmpty = a.textValue.isEmpty();
		bool bEmpty = b.textValue.isEmpty();

		if (aEmpty && bEmpty) return false;
		if (aEmpty) return false;
		if (bEmpty) return true;

		if (a.isNumeric  && b.isNumeric) {
			return order == Qt::AscendingOrder ?
				a.numericValue  < b.numericValue :
				a.numericValue  > b.numericValue;
		}

		// 数值优先于文本 
		if (a.isNumeric != b.isNumeric) {
			return a.isNumeric > b.isNumeric;
		}

		if (a.isNumeric) {
			return order == Qt::AscendingOrder ?
				a.numericValue  < b.numericValue :
				a.numericValue  > b.numericValue;
		}

		// 文本比较 
		return order == Qt::AscendingOrder ?
			a.textValue.compare(b.textValue, Qt::CaseInsensitive) < 0 :
			a.textValue.compare(b.textValue, Qt::CaseInsensitive) > 0;
	};
	qDebug() << allRowData.size();
	std::stable_sort(allRowData.begin(), allRowData.end(), comparator);

	// 3. 准备批量数据 
	QList<QList<QVariant>> sortedData;
	sortedData.reserve(rowCount);

	for (const auto& rd : allRowData) {
		sortedData.append(rd.originalRowData);
	}

	// 4. 批量设置排序后的数据 
	model->setDataBatch(sortedData);

	// 启用视图更新 
	tableView->setUpdatesEnabled(true);
}

void IMAGEPS::sortNumericColumn_SmartM(QTableWidget* table, int column, Qt::SortOrder order)
{
	if (!table || column < 0 || column >= table->columnCount())
		return;

	//收集所有行数据
	struct RowData {
		bool isNumeric;
		double numericValue;
		QString textValue;
		QString satelliteType;  // 第4列：卫星类型 
		QDate dateValue;        // 第5列：日期值 
		int originalRow;
		QList<QVariant> columnData;
	};

	QList<RowData> sortableData;
	int rowCount = table->rowCount();
	int colCount = table->columnCount();

	// 判断是否需要复合排序（基于第4列和第5列）
	bool useCompositeSort = (column == 3 || column == 4); // 第4列或第5列

	// 先收集所有数据，不操作实际表格项
	for (int row = 0; row < rowCount; ++row) {
		RowData rd;
		rd.originalRow = row;

		// 收集整行数据 
		for (int col = 0; col < colCount; ++col) {
			QTableWidgetItem* item = table->item(row, col);
			rd.columnData.append(item ? item->text() : QVariant());
		}

		// 存储第4列（卫星类型）
		QTableWidgetItem* satelliteTypeItem = table->item(row, 3);
		rd.satelliteType = satelliteTypeItem ? satelliteTypeItem->text() : "";

		// 存储第5列（日期）并转换为QDate 
		QTableWidgetItem* dateItem = table->item(row, 4);
		if (dateItem && !dateItem->text().isEmpty()) {
			rd.dateValue = QDate::fromString(dateItem->text(), "yyyy-MM-dd");
		}
		else {
			rd.dateValue = QDate(); // 无效日期
		}

		// 处理排序列 
		QTableWidgetItem* sortItem = table->item(row, column);
		if (!sortItem || sortItem->text().trimmed().isEmpty()) {
			rd.isNumeric = false;
			rd.numericValue = 0.0;
			rd.textValue = "";
		}
		else {
			QString text = sortItem->text();
			bool ok;
			double value = text.toDouble(&ok);
			rd.isNumeric = ok;
			rd.numericValue = value;
			rd.textValue = text;
		}

		sortableData.append(rd);
	}

	// 排序逻辑
	auto comparator = [order, useCompositeSort](const RowData& a, const RowData& b) {
		// 空行排在最后
		bool aEmpty = !a.isNumeric && a.textValue.isEmpty();
		bool bEmpty = !b.isNumeric && b.textValue.isEmpty();

		if (aEmpty && bEmpty) return false;
		if (aEmpty) return false;
		if (bEmpty) return true;

		// 复合排序逻辑（先按第4列，再按第5列）
		if (useCompositeSort) {
			// 首先比较卫星类型（第4列）
			int satelliteCompare = a.satelliteType.compare(b.satelliteType, Qt::CaseInsensitive);
			if (satelliteCompare != 0) {
				return order == Qt::AscendingOrder ? satelliteCompare < 0 : satelliteCompare > 0;
			}

			// 卫星类型相同的情况下，比较日期（第5列）
			if (a.dateValue.isValid() && b.dateValue.isValid()) {
				if (a.dateValue != b.dateValue) {
					return order == Qt::AscendingOrder ?
						a.dateValue < b.dateValue :
						a.dateValue > b.dateValue;
				}
			}
			else if (a.dateValue.isValid()) {
				return order == Qt::AscendingOrder;
			}
			else if (b.dateValue.isValid()) {
				return !(order == Qt::AscendingOrder);
			}

			// 如果日期也相同，保持原有顺序
			return false;
		}

		if (a.isNumeric != b.isNumeric) {
			return a.isNumeric > b.isNumeric;
		}
		if (a.isNumeric) {
			return order == Qt::AscendingOrder ?
				a.numericValue < b.numericValue :
				a.numericValue > b.numericValue;
		}
		return order == Qt::AscendingOrder ?
			a.textValue.compare(b.textValue, Qt::CaseInsensitive) < 0 :
			a.textValue.compare(b.textValue, Qt::CaseInsensitive) > 0;
	};

	std::stable_sort(sortableData.begin(), sortableData.end(), comparator);

	table->blockSignals(true);  // 禁止信号，提高性能 
	table->clearContents();
	table->setRowCount(rowCount);

	for (int newRow = 0; newRow < sortableData.size(); ++newRow) {
		const auto& rd = sortableData[newRow];

		for (int col = 0; col < colCount; ++col) {
			QTableWidgetItem* newItem = new QTableWidgetItem();
			newItem->setText(rd.columnData[col].toString());

			QTableWidgetItem* origItem = table->item(rd.originalRow, col);
			if (origItem) {
				newItem->setTextAlignment(origItem->textAlignment());
				newItem->setBackground(origItem->background());
				newItem->setForeground(origItem->foreground());
				newItem->setFlags(origItem->flags());
			}

			table->setItem(newRow, col, newItem);
		}
	}
	table->blockSignals(false);  // 恢复信号 
}

//void IMAGEPS::sortNumericColumn(QTableView* tableView, int column, Qt::SortOrder order)
//{
//	if (!tableView || column < 0 || column >= tableView->model()->columnCount())
//		return;
//
//	// 获取模型指针 
//	QAbstractItemModel* model = tableView->model();
//	if (!model) return;
//
//	// 1. 收集所有行数据（安全方式）
//	struct RowData {
//		bool isNumeric;
//		double numericValue;
//		QString textValue;
//		int originalRow;
//		QList<QVariant> columnData;  // 使用QVariant存储单元格数据 
//	};
//
//	QList<RowData> sortableData;
//	int rowCount = model->rowCount();
//	int colCount = model->columnCount();
//
//	// 先收集所有数据，不操作实际表格项 
//	for (int row = 0; row < rowCount; ++row) {
//		RowData rd;
//		rd.originalRow = row;
//
//		// 收集整行数据 
//		for (int col = 0; col < colCount; ++col) {
//			QModelIndex index = model->index(row, col);
//			rd.columnData.append(model->data(index));
//		}
//
//		// 处理排序列 
//		QModelIndex sortIndex = model->index(row, column);
//		QString text = model->data(sortIndex).toString().trimmed();
//		if (text.isEmpty()) {
//			rd.isNumeric = false;
//			rd.numericValue = 0.0;
//			rd.textValue = "";
//		}
//		else {
//			bool ok;
//			double value = text.toDouble(&ok);
//			rd.isNumeric = ok;
//			rd.numericValue = value;
//			rd.textValue = text;
//		}
//
//		sortableData.append(rd);
//	}
//
//	// 2. 排序逻辑 
//	auto comparator = [order](const RowData& a, const RowData& b) {
//		// 空行排在最后 
//		bool aEmpty = !a.isNumeric  && a.textValue.isEmpty();
//		bool bEmpty = !b.isNumeric  && b.textValue.isEmpty();
//
//		if (aEmpty && bEmpty) return false;
//		if (aEmpty) return false;
//		if (bEmpty) return true;
//
//		if (a.isNumeric != b.isNumeric) {
//			return a.isNumeric > b.isNumeric;
//		}
//		if (a.isNumeric) {
//			return order == Qt::AscendingOrder ?
//				a.numericValue  < b.numericValue :
//				a.numericValue  > b.numericValue;
//		}
//		return order == Qt::AscendingOrder ?
//			a.textValue.compare(b.textValue, Qt::CaseInsensitive) < 0 :
//			a.textValue.compare(b.textValue, Qt::CaseInsensitive) > 0;
//	};
//
//	std::stable_sort(sortableData.begin(), sortableData.end(), comparator);
//
//	//// 3. 创建新的模型数据 
//	//NonEditableModel* newModel = new NonEditableModel(rowCount, colCount, this);
//
//	//// 复制表头 
//	//for (int col = 0; col < colCount; ++col) {
//	//	newModel->setHeaderData(col, Qt::Horizontal, model->headerData(col, Qt::Horizontal));
//	//}
//	NonEditableModel* origModel = qobject_cast<NonEditableModel*>(tableView->model());
//	origModel->removeRows(0, origModel->rowCount()); // 清空原有数据 
//	// 填充排序后的数据 
//	for (int newRow = 0; newRow < sortableData.size(); ++newRow) {
//		const auto& rd = sortableData[newRow];
//
//		for (int col = 0; col < colCount; ++col) {
//			QStandardItem* newItem = new QStandardItem();
//			newItem->setData(rd.columnData[col], Qt::DisplayRole);
//
//			// 恢复基本属性（可根据需要添加更多）
//			QModelIndex origIndex = model->index(rd.originalRow, col);
//			if (origIndex.isValid()) {
//				newItem->setData(model->data(origIndex, Qt::TextAlignmentRole), Qt::TextAlignmentRole);
//				newItem->setData(model->data(origIndex, Qt::BackgroundRole), Qt::BackgroundRole);
//				newItem->setData(model->data(origIndex, Qt::ForegroundRole), Qt::ForegroundRole);
//				newItem->setFlags(model->flags(origIndex));
//			}
//
//			//origModel->setItem(newRow, col, newItem);
//		}
//	}
//
//	//// 4. 替换模型 
//	//QItemSelectionModel* oldSelectionModel = tableView->selectionModel();
//	//tableView->setModel(origModel);
//	//if (tableView == ui.encryptPointsList_TableW) {
//	//	encryptPointsModel = newModel;
//	//}
//	//else if (tableView == ui.controlPointsLIst_TableW) {
//	//	ControlPointsModel = newModel;
//	//}
//	//delete oldSelectionModel;
//	//delete model;
//}

void IMAGEPS::productionconfiguration() {

	// 存储原始按钮状态 
	//static QString originalOkText;

	// 1. 获取按钮并修改状态 
	QPushButton* okButton = systemConfig->findChild<QPushButton*>("okButton");
	QPushButton* cancelButton = systemConfig->findChild<QPushButton*>("cancelButton");
	QPushButton* applyButton = systemConfig->findChild<QPushButton*>("applyButton");

	if (okButton && applyButton) {
		// 保存原始状态 
		originalOkText = okButton->text();

		// 修改按钮状态
		okButton->setText(QString::fromLocal8Bit("开始执行"));
		applyButton->setVisible(false);
	}

	// 1. 设置界面状态 - 只显示指定的功能项
	QStringList visibleItems = { QString::fromLocal8Bit("运行环境"), QString::fromLocal8Bit("影像匹配"), QString::fromLocal8Bit("卫片平差"), QString::fromLocal8Bit("正射纠正"),
								 QString::fromLocal8Bit("影像融合"), QString::fromLocal8Bit("影像匀色"), QString::fromLocal8Bit("影像镶嵌"), 
								 QString::fromLocal8Bit("智能镶嵌"), QString::fromLocal8Bit("金字塔创建"), QString::fromLocal8Bit("真彩色转换"),
								 QString::fromLocal8Bit("生产配置")
							  };
	for (int i = 0; i < systemConfig->ui.funListWidget->count(); ++i) {
		QListWidgetItem* item = systemConfig->ui.funListWidget->item(i);
		item->setHidden(!visibleItems.contains(item->text()));
	}

	systemConfig->settingShowWidget(QString::fromLocal8Bit("生产配置"));

	// 2. 显示模态对话框
	QEventLoop loop;
	connect(systemConfig, &SystemConfig::accepted, &loop, &QEventLoop::quit);
	connect(systemConfig, &SystemConfig::rejected, &loop, &QEventLoop::quit);

	int result = systemConfig->exec();

	// 3. 无论用户确认还是取消，都恢复按钮状态
	if (okButton && applyButton) {
		okButton->setText(originalOkText);
		applyButton->setVisible(true);
	}

	// 用户取消操作 
	if (result != QDialog::Accepted) {
		return;
	}
	else {
		OrthoRectificationFusion_actionSlot();
	}
}

//void IMAGEPS::productionconfiguration() {
//
//	systemConfig->settingShowWidget(QString::fromLocal8Bit("生产配置"));
//
//	// 4. 显示模态对话框并等待用户操作 
//	QEventLoop loop;
//	connect(systemConfig, &SystemConfig::accepted, &loop, &QEventLoop::quit);
//	connect(systemConfig, &SystemConfig::rejected, &loop, &QEventLoop::quit);
//
//	int result = systemConfig->exec();
//	// 使用封装后的函数显示界面并等待用户确认 
//	//int result = systemConfig->showFunctionAndWait(QString::fromLocal8Bit("生产配置"));
//
//	// 用户取消操作
//	if (result != QDialog::Accepted) {
//		return;
//	}
//	else {
//		qDebug() << "Error: 11111";
//	}
//}

////////////////////////////////////
bool IMAGEPS::writeControlPointXml(const QString& projectFolder)
{
	// 1. 读取PSTimestamp.xml  中的时间数据 
	QDomDocument timestampDoc;
	QFile timestampFile(projectFolder + "/PSTimestamp.xml");
	if (!timestampFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("打开PSTimestamp.xml文件失败"));
		//qDebug() << "Failed to open PSTimestamp.xml";
		return false;
	}

	if (!timestampDoc.setContent(&timestampFile)) {
		PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("解析PSTimestamp.xml文件失败"));

		//qDebug() << "Failed to parse PSTimestamp.xml";
		timestampFile.close();
		return false;
	}
	timestampFile.close();

	// 2. 获取时间节点数据 
	QDomElement timestampsRoot = timestampDoc.documentElement();
	QString satTiePointTime, ctlPointTime, dataModelTime, FNAModelmarkerTime, CTNModelmarkerTime, INAModelmarkerTime, FUModelmarkerTime;

	QDomElement satTiePointElem = timestampsRoot.firstChildElement("SatTiePointMatch");
	if (!satTiePointElem.isNull()) {
		satTiePointTime = satTiePointElem.text();
	}

	QDomElement ctlPointElem = timestampsRoot.firstChildElement("CtlPointMatch");
	if (!ctlPointElem.isNull()) {
		ctlPointTime = ctlPointElem.text();
	}

	QDomElement dataModelElem = timestampsRoot.firstChildElement("DataModelmarker");
	if (!dataModelElem.isNull()) {
		dataModelTime = dataModelElem.text();
	}

	QDomElement FNAModelmarkerElem = timestampsRoot.firstChildElement("FNAModelmarker");
	if (!satTiePointElem.isNull()) {
		satTiePointTime = satTiePointElem.text();
	}

	QDomElement CTNModelmarkerElem = timestampsRoot.firstChildElement("CTNModelmarker");
	if (!satTiePointElem.isNull()) {
		satTiePointTime = satTiePointElem.text();
	}

	QDomElement INAModelmarkerElem = timestampsRoot.firstChildElement("INAModelmarker");
	if (!satTiePointElem.isNull()) {
		satTiePointTime = satTiePointElem.text();
	}

	QDomElement FUModelmarkerElem = timestampsRoot.firstChildElement("FUModelmarker");
	if (!satTiePointElem.isNull()) {
		satTiePointTime = satTiePointElem.text();
	}

	// 3. 写入ControlPoint.xml  (每次重新构建)
	QFile markerfile(projectFolder + "/ControlPoint.xml");
	if (!markerfile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("打开ControlPoint文件失败"));

		//qDebug() << "Failed to open ControlPoint.xml  for writing";
		return false;
	}

	QXmlStreamWriter xml(&markerfile);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();

	// 写入根节点 <marker>
	xml.writeStartElement("marker");

	// 写入时间节点 
	if (!satTiePointTime.isEmpty()) {
		xml.writeTextElement("SatTiePointMatch", satTiePointTime);
	}
	if (!ctlPointTime.isEmpty()) {
		xml.writeTextElement("CtlPointMatch", ctlPointTime);
	}
	if (!dataModelTime.isEmpty()) {
		xml.writeTextElement("DataModelmarker", dataModelTime);
	}
	if (!dataModelTime.isEmpty()) {
		xml.writeTextElement("FNAModelmarker", FNAModelmarkerTime);
	}
	if (!dataModelTime.isEmpty()) {
		xml.writeTextElement("CTNModelmarker", CTNModelmarkerTime);
	}
	if (!dataModelTime.isEmpty()) {
		xml.writeTextElement("INAModelmarker", INAModelmarkerTime);
	}
	if (!dataModelTime.isEmpty()) {
		xml.writeTextElement("FUModelmarker", FUModelmarkerTime);
	}

	xml.writeEndElement();   // </marker>
	xml.writeEndDocument();
	markerfile.close();

	return true;
}

bool IMAGEPS::writeSatImageXml(const QString& projectFolder)
{
	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("PSSatProject");
	doc.appendChild(root);

	// 1. 处理卫星影像数据 (PSSatImages)
	QDomElement pssatImages = doc.createElement("PSSatImages");
	root.appendChild(pssatImages);

	QDomElement satImages = doc.createElement("SatImages");
	satImages.setAttribute("SatModelType", "CommonSat");
	pssatImages.appendChild(satImages);

	// 从sateImageDataList_TableW表中读取数据 
	for (int row = 0; row < ui.sateImageDataList_TableW->rowCount(); ++row) {
		QString fileName = ui.sateImageDataList_TableW->item(row, 1)->text();
		QString satID = ui.sateImageDataList_TableW->item(row, 2)->text();
		QString sensorID = ui.sateImageDataList_TableW->item(row, 3)->text();

		QString joinBA = ui.sateImageDataList_TableW->item(row, 4)->text();
		QString baseImage = ui.sateImageDataList_TableW->item(row, 5)->text();
		QString joinDC = ui.sateImageDataList_TableW->item(row, 6)->text();
		double imageE = ui.sateImageDataList_TableW->item(row, 7)->text().toDouble();
		QString errorImg = ui.sateImageDataList_TableW->item(row, 8)->text();
		QString joinOrtho = ui.sateImageDataList_TableW->item(row, 9)->text();
		QString joinFusion = ui.sateImageDataList_TableW->item(row, 10)->text();
		QString joinGmbMatch = ui.sateImageDataList_TableW->item(row, 11)->text();

		// 在DataModelPath中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : DataModelPath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			QDomElement satImage = doc.createElement("SatImage");
			satImage.setAttribute("SatID", satID);
			satImage.setAttribute("SatImagePath", filePath);
			satImage.setAttribute("SenserID", sensorID);
			satImage.setAttribute("SensorType", "1");
			satImage.setAttribute("Visible", "1");
			satImage.setAttribute("JoinBA", joinBA);
			satImage.setAttribute("BaseImage", baseImage);
			satImage.setAttribute("JoinDC", joinDC);
			satImage.setAttribute("imageE", imageE);
			satImage.setAttribute("ErrorImg", errorImg);
			satImage.setAttribute("JoinOrtho", joinOrtho);
			satImage.setAttribute("JoinFusion", joinFusion);
			satImage.setAttribute("JoinGmbMatch", joinGmbMatch);
			satImages.appendChild(satImage);
		}
	}

	// 2. 处理参考数据 (RefDoms和RefDems)
	QDomElement refDoms = doc.createElement("RefDoms");
	root.appendChild(refDoms);

	QDomElement refDems = doc.createElement("RefDems");
	root.appendChild(refDems);

	// 从refereDataList_TableW表中读取数据 
	for (int row = 0; row < ui.refereDataList_TableW->rowCount(); ++row) {
		QString fileName = ui.refereDataList_TableW->item(row, 1)->text();
		QString dataType = ui.refereDataList_TableW->item(row, 2)->text();

		if (dataType == "DOM") {
			// 在DOMFilePath中查找匹配的文件路径 
			QString filePath;
			for (const QString &path : DOMFilePath) {
				QFileInfo info(path);
				if (info.completeBaseName() == fileName) {
					filePath = path;
					break;
				}
			}

			if (!filePath.isEmpty()) {
				QDomElement refDom = doc.createElement("RefDom");
				refDom.setAttribute("Bands", "3");
				refDom.setAttribute("DomPath", filePath);
				refDoms.appendChild(refDom);
			}
		}
		else if (dataType == "DEM") {
			// 在DEMFilePath中查找匹配的文件路径 
			QString filePath;
			for (const QString &path : DEMFilePath) {
				QFileInfo info(path);
				if (info.completeBaseName() == fileName) {
					filePath = path;
					break;
				}
			}

			if (!filePath.isEmpty()) {
				QDomElement refDem = doc.createElement("RefDem");
				refDem.setAttribute("DemPath", filePath);
				refDems.appendChild(refDem);
			}
		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectFolder + "/Satimage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("保存Satimage.xml文件失败"));

		QMessageBox::critical(this, "Error", u8"无法保存Satimage.xml工程文件");
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();

	return true;
}

void IMAGEPS::writeCreatePydImageXml()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 遍历PyramidDataList_TableW表格 
	for (int row = 0; row < ui.PyramidDataList_TableW->rowCount(); ++row) {
		// 获取第2列的文件名 
		QString fileName = ui.PyramidDataList_TableW->item(row, 1)->text();

		// 在CreatepyramidPath中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : CreatepyramidPath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			// 创建Image节点 
			QDomElement imageElement = doc.createElement("Image");
			root.appendChild(imageElement);

			// 创建ImagePath节点 
			QDomElement pathElement = doc.createElement("ImagePath");
			pathElement.appendChild(doc.createTextNode(filePath));
			imageElement.appendChild(pathElement);
		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectdir + "CreatePydImage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("保存CreatePydImage.xml文件失败"));

		QMessageBox::critical(this, "Error", u8"无法写入CreatePydImage.xml");
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();
}

void IMAGEPS::writeOutByteImageXml()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 遍历imageInterList_TableW表格 
	for (int row = 0; row < ui.imageInterList_TableW->rowCount(); ++row) {
		// 获取第2列的文件名 
		QString fileName = ui.imageInterList_TableW->item(row, 1)->text();
		// 获取第3列的SensorID 
		QString sensorID = ui.imageInterList_TableW->item(row, 2)->text();

		// 在ImageInterFilePath中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : ImageInterFilePath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			// 创建Image节点 
			QDomElement imageElement = doc.createElement("Image");
			root.appendChild(imageElement);

			// 创建ImagePath节点 
			QDomElement pathElement = doc.createElement("ImagePath");
			pathElement.appendChild(doc.createTextNode(filePath));
			imageElement.appendChild(pathElement);

			// 创建InnerID节点 
			QDomElement innerIdElement = doc.createElement("InnerID");
			innerIdElement.setAttribute("type", "int32");
			innerIdElement.appendChild(doc.createTextNode(QString::number(row + 1)));
			imageElement.appendChild(innerIdElement);

			// 创建SensorID节点 
			QDomElement sensorElement = doc.createElement("SensorID");
			sensorElement.appendChild(doc.createTextNode(sensorID));
			imageElement.appendChild(sensorElement);
		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectdir + "OutByteImage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("保存OutByteImage.xml文件失败"));

		QMessageBox::critical(this, "Error", u8"无法写入OutByteImage.xml");
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();
}

void IMAGEPS::writeDodgingImageXml()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 遍历DodgingDataList_TableW表格 
	for (int row = 0; row < ui.DodgingDataList_TableW->rowCount(); ++row) {
		// 获取第2列的文件名 
		QString fileName = ui.DodgingDataList_TableW->item(row, 1)->text();
		// 获取第3列的SensorID 
		QString sensorID = ui.DodgingDataList_TableW->item(row, 2)->text();

		// 在DodgingFilePath中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : DodgingFilePath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			// 创建Image节点 
			QDomElement imageElement = doc.createElement("Image");
			root.appendChild(imageElement);

			// 创建ImagePath节点 
			QDomElement pathElement = doc.createElement("ImagePath");
			pathElement.appendChild(doc.createTextNode(filePath));
			imageElement.appendChild(pathElement);

			// 创建InnerID节点 
			QDomElement innerIdElement = doc.createElement("InnerID");
			innerIdElement.setAttribute("type", "int32");
			innerIdElement.appendChild(doc.createTextNode(QString::number(row + 1)));
			imageElement.appendChild(innerIdElement);

			// 创建SensorID节点 
			QDomElement sensorElement = doc.createElement("SensorID");
			sensorElement.appendChild(doc.createTextNode(sensorID));
			imageElement.appendChild(sensorElement);
		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectdir + "DodgingImage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法写入DodgingImage.xml");
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();
}

void IMAGEPS::writeMosaicImageXml()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 遍历ImageMosaicDataList_TableW表格 
	for (int row = 0; row < ui.ImageMosaicDataList_TableW->rowCount(); ++row) {
		// 获取第2列的文件名 
		QString fileName = ui.ImageMosaicDataList_TableW->item(row, 1)->text();
		// 获取第3列的SensorID 
		QString sensorID = ui.ImageMosaicDataList_TableW->item(row, 2)->text();

		// 在ImageMosaicFilePath中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : ImageMosaicFilePath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			// 创建Image节点 
			QDomElement imageElement = doc.createElement("Image");
			root.appendChild(imageElement);

			// 创建ImagePath节点 
			QDomElement pathElement = doc.createElement("ImagePath");
			pathElement.appendChild(doc.createTextNode(filePath));
			imageElement.appendChild(pathElement);

			// 创建InnerID节点 
			QDomElement innerIdElement = doc.createElement("InnerID");
			innerIdElement.setAttribute("type", "int32");
			innerIdElement.appendChild(doc.createTextNode(QString::number(row + 1)));
			imageElement.appendChild(innerIdElement);

			// 创建SensorID节点 
			QDomElement sensorElement = doc.createElement("SensorID");
			sensorElement.appendChild(doc.createTextNode(sensorID));
			imageElement.appendChild(sensorElement);
		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectdir + "MosaicImage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法写入MosaicImage.xml");
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();
}

void IMAGEPS::writeAbsPositPrecCheckImageXml()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 遍历AbsPositPrecCheckDataList_TableW表格 
	for (int row = 0; row < ui.AbsPositPrecCheckDataList_TableW->rowCount(); ++row) {
		// 获取第2列的文件名 
		QString fileName = ui.AbsPositPrecCheckDataList_TableW->item(row, 1)->text();
		// 获取第3列的Matched 
		QString Matched = ui.AbsPositPrecCheckDataList_TableW->item(row, 2)->text();

		double MinX = ui.AbsPositPrecCheckDataList_TableW->item(row, 4)->text().toDouble();
		double MinY = ui.AbsPositPrecCheckDataList_TableW->item(row, 5)->text().toDouble();
		double MaxX = ui.AbsPositPrecCheckDataList_TableW->item(row, 6)->text().toDouble();
		double MaxY = ui.AbsPositPrecCheckDataList_TableW->item(row, 7)->text().toDouble();
		double AveX = ui.AbsPositPrecCheckDataList_TableW->item(row, 8)->text().toDouble();
		double AveY = ui.AbsPositPrecCheckDataList_TableW->item(row, 9)->text().toDouble();
		double RmsX = ui.AbsPositPrecCheckDataList_TableW->item(row, 10)->text().toDouble();
		double RmsY = ui.AbsPositPrecCheckDataList_TableW->item(row, 11)->text().toDouble();
		double RmsXY = ui.AbsPositPrecCheckDataList_TableW->item(row, 12)->text().toDouble();
		double AveXY = ui.AbsPositPrecCheckDataList_TableW->item(row, 13)->text().toDouble();
		double GcpNum = ui.AbsPositPrecCheckDataList_TableW->item(row, 3)->text().toDouble();

		// 在AbsPositPrecCheckFilePath中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : AbsPositPrecCheckFilePath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			// 创建Image节点 
			QDomElement imageElement = doc.createElement("Image");
			root.appendChild(imageElement);

			// 创建ImagePath节点 
			QDomElement pathElement = doc.createElement("ImagePath");
			pathElement.appendChild(doc.createTextNode(filePath));
			imageElement.appendChild(pathElement);

			// 创建InnerID节点 
			QDomElement innerIdElement = doc.createElement("InnerID");
			innerIdElement.setAttribute("type", "int32");
			innerIdElement.appendChild(doc.createTextNode(QString::number(row + 1)));
			imageElement.appendChild(innerIdElement);

			// 创建Matched节点 
			QDomElement MatchedElement = doc.createElement("Matched");
			MatchedElement.appendChild(doc.createTextNode(Matched));
			imageElement.appendChild(MatchedElement);

			// 创建GcpNum节点 
			QDomElement GcpNumElement = doc.createElement("GcpNum");
			GcpNumElement.appendChild(doc.createTextNode(GcpNum == 0 ? "" : QString::number(GcpNum)));
			imageElement.appendChild(GcpNumElement);

			// 创建MinX节点 
			QDomElement MinXElement = doc.createElement("MinX");
			MinXElement.appendChild(doc.createTextNode(MinX == 0 ? "" : QString::number(MinX)));
			imageElement.appendChild(MinXElement);

			QDomElement MinYElement = doc.createElement("MinY");
			MinYElement.appendChild(doc.createTextNode(MinY == 0 ? "" : QString::number(MinY)));
			imageElement.appendChild(MinYElement);

			QDomElement MaxXElement = doc.createElement("MaxX");
			MaxXElement.appendChild(doc.createTextNode(MaxX == 0 ? "" : QString::number(MaxX)));
			imageElement.appendChild(MaxXElement);

			QDomElement MaxYElement = doc.createElement("MaxY");
			MaxYElement.appendChild(doc.createTextNode(MaxY == 0 ? "" : QString::number(MaxY)));
			imageElement.appendChild(MaxYElement);

			QDomElement AveXElement = doc.createElement("AveX");
			AveXElement.appendChild(doc.createTextNode(AveX == 0 ? "" : QString::number(AveX)));
			imageElement.appendChild(AveXElement);

			QDomElement AveYElement = doc.createElement("AveY");
			AveYElement.appendChild(doc.createTextNode(AveY == 0 ? "" : QString::number(AveY)));
			imageElement.appendChild(AveYElement);

			QDomElement RmsXlement = doc.createElement("RmsX");
			RmsXlement.appendChild(doc.createTextNode(RmsX == 0 ? "" : QString::number(RmsX)));
			imageElement.appendChild(RmsXlement);

			QDomElement RmsYElement = doc.createElement("RmsY");
			RmsYElement.appendChild(doc.createTextNode(RmsY == 0 ? "" : QString::number(RmsY)));
			imageElement.appendChild(RmsYElement);

			QDomElement RmsXYElement = doc.createElement("RmsXY");
			RmsXYElement.appendChild(doc.createTextNode(RmsXY == 0 ? "" : QString::number(RmsXY)));
			imageElement.appendChild(RmsXYElement);

			QDomElement AveXYElement = doc.createElement("AveXY");
			AveXYElement.appendChild(doc.createTextNode(AveXY == 0 ? "" : QString::number(AveXY)));
			imageElement.appendChild(AveXYElement);

		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectdir + "PrecisionCheckImage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法写入PrecisionCheckImage.xml");
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();
}

void IMAGEPS::writeCLDImageImageXml()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 遍历CloudDetectionDataList_TableW表格 
	for (int row = 0; row < ui.CloudDetectionDataList_TableW->rowCount(); ++row) {
		// 获取第2列的文件名 
		QString fileName = ui.CloudDetectionDataList_TableW->item(row, 1)->text();

		// 在CloudDetectionDataListFilePath中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : CloudDetectionDataListFilePath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			// 创建Image节点 
			QDomElement imageElement = doc.createElement("Image");
			root.appendChild(imageElement);

			// 创建ImagePath节点 
			QDomElement pathElement = doc.createElement("ImagePath");
			pathElement.appendChild(doc.createTextNode(filePath));
			imageElement.appendChild(pathElement);

			// 创建InnerID节点 
			QDomElement innerIdElement = doc.createElement("InnerID");
			innerIdElement.setAttribute("type", "int32");
			innerIdElement.appendChild(doc.createTextNode(QString::number(row + 1)));
			imageElement.appendChild(innerIdElement);
		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectdir + "CLDImage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法写入CLDImage.xml");
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();
}

/**
 * @brief:从CreatePydImage.xml 读取数据并加载到PyramidDataList_TableW 
 */
void IMAGEPS::loadCreatePydImageData()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	QFile file(projectdir + "CreatePydImage.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法读取CreatePydImage.xml");
		return;
	}

	if (file.size() == 0)
	{
		file.close();
		return;
	}

	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::critical(this, "Error", u8"CreatePydImage.xml 格式错误");
		return;
	}
	file.close();

	// 清空表格 
	ui.PyramidDataList_TableW->setRowCount(0);

	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();
		QDomElement pathElement = imageElement.firstChildElement("ImagePath");

		if (!pathElement.isNull()) {
			QString filePath = pathElement.text();
			QFileInfo fileInfo(filePath);

			// 添加到CreatepyramidPath列表 
			CreatepyramidPath.append(filePath);

			// 添加到表格 
			int newRow = ui.PyramidDataList_TableW->rowCount();
			ui.PyramidDataList_TableW->insertRow(newRow);

			// 确保所有列都有item 
			for (int col = 0; col < ui.PyramidDataList_TableW->columnCount(); ++col) {
				if (!ui.PyramidDataList_TableW->item(newRow, col)) {
					QTableWidgetItem* item = new QTableWidgetItem;
					item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					ui.PyramidDataList_TableW->setItem(newRow, col, item);
				}
			}

			// 设置数据 
			ui.PyramidDataList_TableW->item(newRow, 0)->setText(QString::number(newRow + 1));
			ui.PyramidDataList_TableW->item(newRow, 1)->setText(fileInfo.completeBaseName());
			// 其他列可以根据需要设置默认值 
		}
	}
	ui.PyramidDataList_TableW->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(CreatepyramidPath);
}

/**
 * @brief:从OutByteImage.xml 读取数据并加载到imageInterList_TableW
 */
void IMAGEPS::loadOutByteImageData()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	QFile file(projectdir + "OutByteImage.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法读取OutByteImage.xml");
		return;
	}

	if (file.size() == 0)
	{
		file.close();
		return;
	}


	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::critical(this, "Error", u8"OutByteImage.xml 格式错误");
		return;
	}
	file.close();

	// 清空表格 
	ui.imageInterList_TableW->setRowCount(0);

	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();
		QDomElement pathElement = imageElement.firstChildElement("ImagePath");
		QDomElement sensorElement = imageElement.firstChildElement("SensorID");

		if (!pathElement.isNull() && !sensorElement.isNull()) {
			QString filePath = pathElement.text();
			QString sensorID = sensorElement.text();
			QFileInfo fileInfo(filePath);

			// 添加到ImageInterFilePath列表 
			ImageInterFilePath.append(filePath);

			// 添加到表格 
			int newRow = ui.imageInterList_TableW->rowCount();
			ui.imageInterList_TableW->insertRow(newRow);

			// 确保所有列都有item 
			for (int col = 0; col < ui.imageInterList_TableW->columnCount(); ++col) {
				if (!ui.imageInterList_TableW->item(newRow, col)) {
					QTableWidgetItem* item = new QTableWidgetItem;
					item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					ui.imageInterList_TableW->setItem(newRow, col, item);
				}
			}

			// 设置数据 
			ui.imageInterList_TableW->item(newRow, 0)->setText(QString::number(newRow + 1));
			ui.imageInterList_TableW->item(newRow, 1)->setText(fileInfo.completeBaseName());
			ui.imageInterList_TableW->item(newRow, 2)->setText(sensorID);
			// 其他列可以根据需要设置默认值 
		}
	}
	ui.imageInterList_TableW->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(ImageInterFilePath);
}

/**
 * @brief:从DodgingImage.xml 读取数据并加载到DodgingDataList_TableW
 */
void IMAGEPS::loadDodgingImageData()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	QFile file(projectdir + "DodgingImage.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法读取DodgingImage.xml");
		return;
	}

	if (file.size() == 0)
	{
		file.close();
		return;
	}


	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::critical(this, "Error", u8"DodgingImage.xml 格式错误");
		return;
	}
	file.close();

	// 清空表格 
	ui.DodgingDataList_TableW->setRowCount(0);

	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();
		QDomElement pathElement = imageElement.firstChildElement("ImagePath");
		QDomElement sensorElement = imageElement.firstChildElement("SensorID");

		if (!pathElement.isNull() && !sensorElement.isNull()) {
			QString filePath = pathElement.text();
			QString sensorID = sensorElement.text();
			QFileInfo fileInfo(filePath);

			// 添加到DodgingFilePath列表 
			DodgingFilePath.append(filePath);

			// 添加到表格 
			int newRow = ui.DodgingDataList_TableW->rowCount();
			ui.DodgingDataList_TableW->insertRow(newRow);

			// 确保所有列都有item 
			for (int col = 0; col < ui.DodgingDataList_TableW->columnCount(); ++col) {
				if (!ui.DodgingDataList_TableW->item(newRow, col)) {
					QTableWidgetItem* item = new QTableWidgetItem;
					item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					ui.DodgingDataList_TableW->setItem(newRow, col, item);
				}
			}

			// 设置数据 
			ui.DodgingDataList_TableW->item(newRow, 0)->setText(QString::number(newRow + 1));
			ui.DodgingDataList_TableW->item(newRow, 1)->setText(fileInfo.completeBaseName());
			ui.DodgingDataList_TableW->item(newRow, 2)->setText(sensorID);
			// 其他列可以根据需要设置默认值 
		}
	}
	ui.DodgingDataList_TableW->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(DodgingFilePath);
}

/**
 * @brief:从MosaicImage.xml 读取数据并加载到ImageMosaicDataList_TableW
 */
void IMAGEPS::loadMosaicImageData()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	QFile file(projectdir + "MosaicImage.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法读取MosaicImage.xml");
		return;
	}

	if (file.size() == 0)
	{
		file.close();
		return;
	}


	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::critical(this, "Error", u8"MosaicImage.xml 格式错误");
		return;
	}
	file.close();

	// 清空表格 
	ui.ImageMosaicDataList_TableW->setRowCount(0);

	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();
		QDomElement pathElement = imageElement.firstChildElement("ImagePath");
		QDomElement sensorElement = imageElement.firstChildElement("SensorID");

		if (!pathElement.isNull() && !sensorElement.isNull()) {
			QString filePath = pathElement.text();
			QString sensorID = sensorElement.text();
			QFileInfo fileInfo(filePath);

			// 添加到ImageMosaicFilePath列表 
			ImageMosaicFilePath.append(filePath);

			// 添加到表格 
			int newRow = ui.ImageMosaicDataList_TableW->rowCount();
			ui.ImageMosaicDataList_TableW->insertRow(newRow);

			// 确保所有列都有item 
			for (int col = 0; col < ui.ImageMosaicDataList_TableW->columnCount(); ++col) {
				if (!ui.ImageMosaicDataList_TableW->item(newRow, col)) {
					QTableWidgetItem* item = new QTableWidgetItem;
					item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					ui.ImageMosaicDataList_TableW->setItem(newRow, col, item);
				}
			}

			// 设置数据 
			ui.ImageMosaicDataList_TableW->item(newRow, 0)->setText(QString::number(newRow + 1));
			ui.ImageMosaicDataList_TableW->item(newRow, 1)->setText(fileInfo.completeBaseName());
			ui.ImageMosaicDataList_TableW->item(newRow, 2)->setText(sensorID);
		}
	}
	ui.ImageMosaicDataList_TableW->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(ImageMosaicFilePath);
}

/**
 * @brief:从PrecisionCheckImage.xml 读取数据并加载到AbsPositPrecCheckDataList_TableW
 */
void IMAGEPS::loadAbsPositPrecCheckImageData()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	QFile file(projectdir + "PrecisionCheckImage.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法读取PrecisionCheckImage.xml");
		return;
	}

	if (file.size() == 0)
	{
		file.close();
		return;
	}


	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::critical(this, "Error", u8"PrecisionCheckImage.xml 格式错误");
		return;
	}
	file.close();

	// 清空表格 
	ui.AbsPositPrecCheckDataList_TableW->setRowCount(0);

	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();
		QDomElement pathElement = imageElement.firstChildElement("ImagePath");
		QDomElement MatchedElement = imageElement.firstChildElement("Matched");

		if (!pathElement.isNull() && !MatchedElement.isNull()) {
			QString filePath = pathElement.text();
			QString sensorID = MatchedElement.text();
			QFileInfo fileInfo(filePath);

			// 添加到AbsPositPrecCheckFilePath列表 
			if (!AbsPositPrecCheckFilePath.contains(filePath)) {
				AbsPositPrecCheckFilePath.append(filePath);
			}

			// 添加到表格 
			int newRow = ui.AbsPositPrecCheckDataList_TableW->rowCount();
			ui.AbsPositPrecCheckDataList_TableW->insertRow(newRow);

			// 确保所有列都有item 
			for (int col = 0; col < ui.AbsPositPrecCheckDataList_TableW->columnCount(); ++col) {
				if (!ui.AbsPositPrecCheckDataList_TableW->item(newRow, col)) {
					QTableWidgetItem* item = new QTableWidgetItem;
					item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					ui.AbsPositPrecCheckDataList_TableW->setItem(newRow, col, item);
				}
			}

			// 设置数据 
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 0)->setText(QString::number(newRow + 1));
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 1)->setText(fileInfo.completeBaseName());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 2)->setText(imageElement.firstChildElement("Matched").text());
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 3)->setText(QString::number(imageElement.firstChildElement("GcpNum").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 4)->setText(QString::number(imageElement.firstChildElement("MinX").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 5)->setText(QString::number(imageElement.firstChildElement("MinY").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 6)->setText(QString::number(imageElement.firstChildElement("MaxX").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 7)->setText(QString::number(imageElement.firstChildElement("MaxY").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 8)->setText(QString::number(imageElement.firstChildElement("AveX").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 9)->setText(QString::number(imageElement.firstChildElement("AveY").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 10)->setText(QString::number(imageElement.firstChildElement("RmsX").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 11)->setText(QString::number(imageElement.firstChildElement("RmsY").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 12)->setText(QString::number(imageElement.firstChildElement("RmsXY").text().toDouble()));
			//ui.AbsPositPrecCheckDataList_TableW->item(newRow, 13)->setText(QString::number(imageElement.firstChildElement("AveXY").text().toDouble()));

			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 3)->setText(imageElement.firstChildElement("GcpNum").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 4)->setText(imageElement.firstChildElement("MinX").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 5)->setText(imageElement.firstChildElement("MinY").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 6)->setText(imageElement.firstChildElement("MaxX").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 7)->setText(imageElement.firstChildElement("MaxY").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 8)->setText(imageElement.firstChildElement("AveX").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 9)->setText(imageElement.firstChildElement("AveY").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 10)->setText(imageElement.firstChildElement("RmsX").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 11)->setText(imageElement.firstChildElement("RmsY").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 12)->setText(imageElement.firstChildElement("RmsXY").text());
			ui.AbsPositPrecCheckDataList_TableW->item(newRow, 13)->setText(imageElement.firstChildElement("AveXY").text());
		}
	}
	ui.AbsPositPrecCheckDataList_TableW->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(AbsPositPrecCheckFilePath);
}

/**
 * @brief:从CLDImage.xml 读取数据并加载到CloudDetectionDataList_TableW CloudDetectionList_TableW数据载入
 */
void IMAGEPS::loadCLDImageData()
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	QFile file(projectdir + "CLDImage.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法读取CLDImage.xml");
		return;
	}

	if (file.size() == 0)
	{
		file.close();
		return;
	}


	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::critical(this, "Error", u8"CLDImage.xml 格式错误");
		return;
	}
	file.close();

	// 清空表格 
	ui.CloudDetectionDataList_TableW->setRowCount(0);

	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();
		QDomElement pathElement = imageElement.firstChildElement("ImagePath");

		if (!pathElement.isNull()) {
			QString filePath = pathElement.text();
			QFileInfo fileInfo(filePath);

			// 添加到CloudDetectionDataListFilePath列表 
			if (!CloudDetectionDataListFilePath.contains(filePath)) {
				CloudDetectionDataListFilePath.append(filePath);
			}

			// 添加到表格 
			int newRow = ui.CloudDetectionDataList_TableW->rowCount();
			ui.CloudDetectionDataList_TableW->insertRow(newRow);

			// 确保所有列都有item 
			for (int col = 0; col < ui.CloudDetectionDataList_TableW->columnCount(); ++col) {
				if (!ui.CloudDetectionDataList_TableW->item(newRow, col)) {
					QTableWidgetItem* item = new QTableWidgetItem;
					item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					ui.CloudDetectionDataList_TableW->setItem(newRow, col, item);
				}
			}

			// 设置数据 
			ui.CloudDetectionDataList_TableW->item(newRow, 0)->setText(QString::number(newRow + 1));
			ui.CloudDetectionDataList_TableW->item(newRow, 1)->setText(fileInfo.completeBaseName());
		}
	}

	// 清空现有数据但保留表头
	ui.CloudDetectionList_TableW->clearContents();
	ui.CloudDetectionList_TableW->setRowCount(0);

	int i = 0;
	for (auto filepath : CloudDetectionDataListFilePath)
	{
		QFileInfo fileinfo(filepath);

		QString CldiFilePath = projectdir + "Cld" + "/" + "CloudDect" + "/" + fileinfo.completeBaseName() + "_cld.txt";
		QString OverviewFilePath = projectdir + "Cld" + "/" + "OverView" + "/" + fileinfo.completeBaseName() + ".jpg";
		QFile cldFile(CldiFilePath);

		if (cldFile.exists()) {
			CloudDetectionListFilePath.append(OverviewFilePath);
			ui.CloudDetectionList_TableW->insertRow(i);

			// 第一列：序号
			QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(i + 1));
			indexItem->setTextAlignment(Qt::AlignCenter);
			ui.CloudDetectionList_TableW->setItem(i, 0, indexItem);

			// 第二列：影像名称（Overview文件名）
			QFileInfo overviewInfo(OverviewFilePath);
			QTableWidgetItem *nameItem = new QTableWidgetItem(overviewInfo.completeBaseName());
			nameItem->setTextAlignment(Qt::AlignCenter);
			ui.CloudDetectionList_TableW->setItem(i, 1, nameItem);

			// 第三列：云检成果（从CloudDect的txt文件读取CloudRatio）
			QFileInfo cloudDectInfo(CldiFilePath);
			QString resultTxtPath = cloudDectInfo.path() + "/" + cloudDectInfo.completeBaseName() + ".txt";

			double cloudRatio = 0.0;
			QFile txtFile(resultTxtPath);
			if (txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream in(&txtFile);
				while (!in.atEnd()) {
					QString line = in.readLine();
					if (line.contains("CloudRatio")) {
						QStringList parts = line.split("=");
						if (parts.size() == 2) {
							cloudRatio = parts[1].trimmed().toDouble();
							break;
						}
					}
				}
				txtFile.close();
			}

			QTableWidgetItem *ratioItem = new QTableWidgetItem(QString::number(cloudRatio, 'f', 4));
			ratioItem->setTextAlignment(Qt::AlignCenter);
			ui.CloudDetectionList_TableW->setItem(i, 2, ratioItem);
			i++;
		}
	}
	// 自动调整列宽 
	ui.CloudDetectionList_TableW->resizeColumnsToContents();

	setGeoreferenceFromXMLAndRPC(CloudDetectionDataListFilePath);
}

/**
 * 写入RegisterRectifyImage.xml 文件
 */
bool IMAGEPS::writeRegisterRectifyImageXml(const QString& projectFolder)
{
	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 从AlignmentAdjustmentList_TableW表中读取数据 
	for (int row = 0; row < ui.AlignmentAdjustmentList_TableW->rowCount(); ++row) {
		// 获取第1列数据(InnerID)
		QString innerID = ui.AlignmentAdjustmentList_TableW->item(row, 0)->text();

		// 获取第2列数据(文件名)
		QString fileName = ui.AlignmentAdjustmentList_TableW->item(row, 1)->text();

		// 在AlignmentAdFilePath中查找对应的文件路径 
		QString filePath;
		for (const QString &path : AlignmentAdFilePath) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (filePath.isEmpty()) {
			qDebug() << "未找到文件路径:" << fileName;
			continue;
		}

		// 获取第3列数据(Matched)
		QString matched = ui.AlignmentAdjustmentList_TableW->item(row, 2)->text();

		// 获取第4列数据(Rectified)
		QString rectified = ui.AlignmentAdjustmentList_TableW->item(row, 3)->text();

		// 创建Image元素 
		QDomElement imageElement = doc.createElement("Image");
		root.appendChild(imageElement);

		// 添加ImagePath子元素 
		QDomElement pathElement = doc.createElement("ImagePath");
		pathElement.appendChild(doc.createTextNode(filePath));
		imageElement.appendChild(pathElement);

		// 添加InnerID子元素 
		QDomElement idElement = doc.createElement("InnerID");
		idElement.setAttribute("type", "int32");
		idElement.appendChild(doc.createTextNode(innerID));
		imageElement.appendChild(idElement);

		// 添加SensorID子元素(空)
		QDomElement sensorElement = doc.createElement("SensorID");
		imageElement.appendChild(sensorElement);

		// 添加Matched子元素 
		QDomElement matchedElement = doc.createElement("Matched");
		matchedElement.setAttribute("type", "bool");
		matchedElement.appendChild(doc.createTextNode(matched));
		imageElement.appendChild(matchedElement);

		// 添加Rectified子元素 
		QDomElement rectifiedElement = doc.createElement("Rectified");
		rectifiedElement.setAttribute("type", "bool");
		rectifiedElement.appendChild(doc.createTextNode(rectified));
		imageElement.appendChild(rectifiedElement);
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectFolder + "/RegisterRectifyImage.xml");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法保存RegisterRectifyImage.xml  文件");
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4); // 缩进4个空格 
	file.close();

	return true;
}

/**
 *从RegisterRectifyImage.xml 读取数据并加载到表格
 */
bool IMAGEPS::loadRegisterRectifyImageData(const QString& projectFolder)
{
	// 清空表格 
	ui.AlignmentAdjustmentList_TableW->setRowCount(0);

	// 打开XML文件 
	QFile file(projectFolder + "/RegisterRectifyImage.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << u8"无法打开RegisterRectifyImage.xml 文件";
		return false;
	}

	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		qDebug() << "RegisterRectifyImage.xml 文件格式错误";
		return false;
	}
	file.close();

	// 解析XML内容 
	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();

		// 获取ImagePath 
		QString filePath = imageElement.firstChildElement("ImagePath").text();
		QFileInfo fileInfo(filePath);
		QString fileName = fileInfo.completeBaseName();

		// 获取InnerID 
		QString innerID = imageElement.firstChildElement("InnerID").text();

		// 获取Matched 
		QString matched = imageElement.firstChildElement("Matched").text();

		// 获取Rectified 
		QString rectified = imageElement.firstChildElement("Rectified").text();

		// 添加到AlignmentAdFilePath 
		if (!AlignmentAdFilePath.contains(filePath)) {
			AlignmentAdFilePath.append(filePath);
		}

		// 在表格中添加新行 
		int newRow = ui.AlignmentAdjustmentList_TableW->rowCount();
		ui.AlignmentAdjustmentList_TableW->insertRow(newRow);

		// 确保所有列都有QTableWidgetItem 
		for (int col = 0; col < ui.AlignmentAdjustmentList_TableW->columnCount(); ++col) {
			if (!ui.AlignmentAdjustmentList_TableW->item(newRow, col)) {
				QTableWidgetItem* item = new QTableWidgetItem;
				item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
				ui.AlignmentAdjustmentList_TableW->setItem(newRow, col, item);
			}
		}

		// 设置各列数据 
		ui.AlignmentAdjustmentList_TableW->item(newRow, 0)->setText(innerID); // InnerID 
		ui.AlignmentAdjustmentList_TableW->item(newRow, 1)->setText(fileName); // 文件名 
		ui.AlignmentAdjustmentList_TableW->item(newRow, 2)->setText(matched); // Matched 
		ui.AlignmentAdjustmentList_TableW->item(newRow, 3)->setText(rectified); // Rectified 
	}
	ui.AlignmentAdjustmentList_TableW->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(AlignmentAdFilePath);
	return true;
}

  /**
   * @brief 将数据写入FusionImage.xml （带<Fusion>根标签）
   * @param fusionImagePath 输出文件路径
   * @param satModelMakerCmdoutPath 输入模型文件路径
   * @param alignmentIntFilePath 对齐文件路径列表
   * @param alignmentTable 对齐表格指针
   * @param fusionModelTable 融合模型表格指针
   * @return 成功返回true，失败返回false
   */
bool IMAGEPS::writeDataToFusionImage(const QString& fusionImagePath,
	const QString& satModelMakerCmdoutPath,
	const QStringList& alignmentIntFilePath,
	QTableWidget* alignmentTable,
	QTableWidget* fusionModelTable)
{
	// 创建XML文档 
	QDomDocument fusionDoc;
	QDomProcessingInstruction instruction = fusionDoc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	fusionDoc.appendChild(instruction);

	// 创建根节点<Fusion>
	QDomElement fusionRoot = fusionDoc.createElement("Fusion");
	fusionDoc.appendChild(fusionRoot);

	// 1. 创建并填充SatImages节点 
	QDomElement satImages = fusionDoc.createElement("SatImages");
	fusionRoot.appendChild(satImages);

	// 用于记录已处理的文件名，避免重复添加 
	QSet<QString> processedFiles;

	// 从对齐表格填充SatImages 
	for (int row = 0; row < alignmentTable->rowCount(); ++row) {
		QString fileName = alignmentTable->item(row, 1)->text();
		QString filePath;

		// 查找匹配的文件路径 
		for (const QString& path : alignmentIntFilePath) {
			if (QFileInfo(path).completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (filePath.isEmpty()) {
			qDebug() << "Path not found for:" << fileName;
			continue;
		}

		// 创建Image节点 
		QDomElement imageElement = fusionDoc.createElement("Image");

		// 添加ImagePath 
		QDomElement pathElement = fusionDoc.createElement("ImagePath");
		pathElement.appendChild(fusionDoc.createTextNode(filePath));
		imageElement.appendChild(pathElement);

		// 添加InnerID 
		QDomElement idElement = fusionDoc.createElement("InnerID");
		idElement.setAttribute("type", "int32");
		idElement.appendChild(fusionDoc.createTextNode(alignmentTable->item(row, 0)->text()));
		imageElement.appendChild(idElement);

		// 添加SensorID 
		QDomElement sensorElement = fusionDoc.createElement("SensorID");
		sensorElement.appendChild(fusionDoc.createTextNode(alignmentTable->item(row, 2)->text()));
		imageElement.appendChild(sensorElement);

		// 添加Matched（默认false）
		QDomElement matchedElement = fusionDoc.createElement("Matched");
		matchedElement.setAttribute("type", "bool");
		matchedElement.appendChild(fusionDoc.createTextNode("false"));
		imageElement.appendChild(matchedElement);

		// 添加Fusioned（默认false）
		QDomElement fusionedElement = fusionDoc.createElement("Fusioned");
		fusionedElement.setAttribute("type", "bool");
		fusionedElement.appendChild(fusionDoc.createTextNode("false"));
		imageElement.appendChild(fusionedElement);

		satImages.appendChild(imageElement);
		processedFiles.insert(QFileInfo(filePath).completeBaseName());
	}

	// 检查融合模型表格中的文件名，如果不在已处理列表中则添加 
	for (int row = 0; row < fusionModelTable->rowCount(); ++row) {
		// 检查第2列文件名（基准图像）
		QString fileName1 = fusionModelTable->item(row, 2)->text();
		if (!fileName1.isEmpty() && !processedFiles.contains(fileName1)) {
			QString filePath1;
			for (const QString& path : alignmentIntFilePath) {
				if (QFileInfo(path).completeBaseName() == fileName1) {
					filePath1 = path;
					break;
				}
			}

			if (!filePath1.isEmpty()) {
				// 创建Image节点 
				QDomElement imageElement = fusionDoc.createElement("Image");

				// 添加ImagePath 
				QDomElement pathElement = fusionDoc.createElement("ImagePath");
				pathElement.appendChild(fusionDoc.createTextNode(filePath1));
				imageElement.appendChild(pathElement);

				// 添加InnerID（默认-1表示未知）
				QDomElement idElement = fusionDoc.createElement("InnerID");
				idElement.setAttribute("type", "int32");
				idElement.appendChild(fusionDoc.createTextNode("-1"));
				imageElement.appendChild(idElement);

				// 添加SensorID（使用getTiffImageType函数获取）
				QDomElement sensorElement = fusionDoc.createElement("SensorID");
				QString sensorType = systemConfig->getTiffImageType(filePath1);
				sensorElement.appendChild(fusionDoc.createTextNode(sensorType));
				imageElement.appendChild(sensorElement);

				// 添加Matched（默认false）
				QDomElement matchedElement = fusionDoc.createElement("Matched");
				matchedElement.setAttribute("type", "bool");
				matchedElement.appendChild(fusionDoc.createTextNode("false"));
				imageElement.appendChild(matchedElement);

				// 添加Fusioned（默认false）
				QDomElement fusionedElement = fusionDoc.createElement("Fusioned");
				fusionedElement.setAttribute("type", "bool");
				fusionedElement.appendChild(fusionDoc.createTextNode("false"));
				imageElement.appendChild(fusionedElement);

				satImages.appendChild(imageElement);
				processedFiles.insert(fileName1);
			}
		}

		// 检查第3列文件名（匹配图像）
		QString fileName2 = fusionModelTable->item(row, 3)->text();
		if (!fileName2.isEmpty() && !processedFiles.contains(fileName2)) {
			QString filePath2;
			for (const QString& path : alignmentIntFilePath) {
				if (QFileInfo(path).completeBaseName() == fileName2) {
					filePath2 = path;
					break;
				}
			}

			if (!filePath2.isEmpty()) {
				// 创建Image节点 
				QDomElement imageElement = fusionDoc.createElement("Image");

				// 添加ImagePath 
				QDomElement pathElement = fusionDoc.createElement("ImagePath");
				pathElement.appendChild(fusionDoc.createTextNode(filePath2));
				imageElement.appendChild(pathElement);

				// 添加InnerID（默认-1表示未知）
				QDomElement idElement = fusionDoc.createElement("InnerID");
				idElement.setAttribute("type", "int32");
				idElement.appendChild(fusionDoc.createTextNode("-1"));
				imageElement.appendChild(idElement);

				// 添加SensorID（使用getTiffImageType函数获取）
				QDomElement sensorElement = fusionDoc.createElement("SensorID");
				QString sensorType = systemConfig->getTiffImageType(filePath2);
				sensorElement.appendChild(fusionDoc.createTextNode(sensorType));
				imageElement.appendChild(sensorElement);

				// 添加Matched（默认false）
				QDomElement matchedElement = fusionDoc.createElement("Matched");
				matchedElement.setAttribute("type", "bool");
				matchedElement.appendChild(fusionDoc.createTextNode("false"));
				imageElement.appendChild(matchedElement);

				// 添加Fusioned（默认false）
				QDomElement fusionedElement = fusionDoc.createElement("Fusioned");
				fusionedElement.setAttribute("type", "bool");
				fusionedElement.appendChild(fusionDoc.createTextNode("false"));
				imageElement.appendChild(fusionedElement);

				satImages.appendChild(imageElement);
				processedFiles.insert(fileName2);
			}
		}
	}

	// 2. 创建并填充FusionModels节点 
	QDomElement fusionModels = fusionDoc.createElement("FusionModels");
	fusionRoot.appendChild(fusionModels);

	// 从SatModelMakerCmdout.xml 读取模型数据 
	QFile satModelFile(satModelMakerCmdoutPath);
	if (!satModelFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open SatModelMakerCmdout.xml";
		//return false;
	}

	QDomDocument satModelDoc;
	if (!satModelDoc.setContent(&satModelFile)) {
		satModelFile.close();
		qDebug() << "Failed to parse SatModelMakerCmdout.xml";
		//return false;
	}
	satModelFile.close();

	QDomElement satRoot = satModelDoc.documentElement();

	// 复制ModelNum 
	QDomElement modelNum = fusionDoc.createElement("ModelNum");
	modelNum.setAttribute("type", "int32");
	QDomElement srcModelNum = satRoot.firstChildElement("ModelNum");
	modelNum.appendChild(fusionDoc.createTextNode(srcModelNum.isNull() ? "0" : srcModelNum.text()));
	fusionModels.appendChild(modelNum);

	// 创建Models节点 
	QDomElement models = fusionDoc.createElement("Models");
	fusionModels.appendChild(models);

	// 复制模型数据 
	QDomElement srcModels = satRoot.firstChildElement("Models");
	if (!srcModels.isNull()) {
		QDomNodeList modelList = srcModels.childNodes();
		for (int i = 0; i < modelList.size(); ++i) {
			QDomNode node = modelList.at(i);
			if (node.isElement() && node.nodeName().startsWith("Model_")) {
				models.appendChild(node.cloneNode(true));
			}
		}
	}

	// 3. 更新Matched和Fusioned状态 
	QDomNodeList imageList = satImages.elementsByTagName("Image");
	for (int row = 0; row < fusionModelTable->rowCount(); ++row) {
		QString fileName1 = fusionModelTable->item(row, 2)->text();
		QString fileName2 = fusionModelTable->item(row, 3)->text();

		// 更新Matched状态 
		if (fusionModelTable->columnCount() > 4) {
			bool matched = (fusionModelTable->item(row, 4)->text() == QString::fromLocal8Bit("是"));
			for (int i = 0; i < imageList.size(); ++i) {
				QDomElement image = imageList.at(i).toElement();
				QString path = image.firstChildElement("ImagePath").text();
				if (QFileInfo(path).completeBaseName() == fileName1) {
					image.firstChildElement("Matched").firstChild().setNodeValue(matched ? "true" : "false");
					break;
				}
			}
		}

		// 更新Fusioned状态 
		if (fusionModelTable->columnCount() > 5) {
			bool fusioned = (fusionModelTable->item(row, 5)->text() == QString::fromLocal8Bit("是"));
			for (int i = 0; i < imageList.size(); ++i) {
				QDomElement image = imageList.at(i).toElement();
				QString path = image.firstChildElement("ImagePath").text();
				if (QFileInfo(path).completeBaseName() == fileName1 ||
					QFileInfo(path).completeBaseName() == fileName2) {
					image.firstChildElement("Fusioned").firstChild().setNodeValue(fusioned ? "true" : "false");
				}
			}
		}
	}

	// 保存文件 
	QFile file(fusionImagePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Failed to create output file";
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	fusionDoc.save(out, 4);
	file.close();

	return true;
}

/**
 * @brief 将数据写入SmartMosaicImage.xml （带<Fusion>根标签）
 * @param fusionImagePath 输出文件路径
 * @param alignmentIntFilePath 对齐文件路径列表
 * @param ShpFilePath 裁切文件路径列表
 * @param alignmentTable 对齐表格指针
 * @param ShpFilePathTable 裁切文件表格指针
 * @return 成功返回true，失败返回false
 */
bool IMAGEPS::writeSmartMosaicImage(const QString& fusionImagePath,
	const QStringList& alignmentIntFilePath,
	const QStringList& ShpFilePath,
	QTableWidget* alignmentTable,
	QTableWidget* ShpFilePathTable)
{
	// 创建XML文档 
	QDomDocument fusionDoc;
	QDomProcessingInstruction instruction = fusionDoc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	fusionDoc.appendChild(instruction);

	// 创建根节点<Fusion>
	QDomElement fusionRoot = fusionDoc.createElement("SmartMosaic");
	fusionDoc.appendChild(fusionRoot);

	// 1. 创建并填充SatImages节点 
	QDomElement satImages = fusionDoc.createElement("SatImages");
	fusionRoot.appendChild(satImages);

	// 用于记录已处理的文件名，避免重复添加 
	//QSet<QString> processedFiles;

	// 从对齐表格填充SatImages 
	for (int row = 0; row < alignmentTable->rowCount(); ++row) {
		QString fileName = alignmentTable->item(row, 1)->text();
		QString filePath;

		// 查找匹配的文件路径 
		for (const QString& path : alignmentIntFilePath) {
			if (QFileInfo(path).completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (filePath.isEmpty()) {
			qDebug() << "Path not found for:" << fileName;
			continue;
		}

		// 创建Image节点 
		QDomElement imageElement = fusionDoc.createElement("Image");

		// 添加ImagePath 
		QDomElement pathElement = fusionDoc.createElement("ImagePath");
		pathElement.appendChild(fusionDoc.createTextNode(filePath));
		imageElement.appendChild(pathElement);

		// 添加InnerID 
		QDomElement idElement = fusionDoc.createElement("InnerID");
		idElement.setAttribute("type", "int32");
		idElement.appendChild(fusionDoc.createTextNode(alignmentTable->item(row, 0)->text()));
		imageElement.appendChild(idElement);

		// 添加SensorID 
		QDomElement sensorElement = fusionDoc.createElement("SensorID");
		sensorElement.appendChild(fusionDoc.createTextNode(alignmentTable->item(row, 2)->text()));
		imageElement.appendChild(sensorElement);

		// 添加StellarSource
		QDomElement StellarSourceElement = fusionDoc.createElement("StellarSource");
		StellarSourceElement.appendChild(fusionDoc.createTextNode(alignmentTable->item(row, 3)->text()));
		imageElement.appendChild(StellarSourceElement);

		// 添加Timelike
		QDomElement TimelikeElement = fusionDoc.createElement("Timelike");
		TimelikeElement.appendChild(fusionDoc.createTextNode(alignmentTable->item(row, 4)->text()));
		imageElement.appendChild(TimelikeElement);

		satImages.appendChild(imageElement);
		//processedFiles.insert(QFileInfo(filePath).completeBaseName());
	}

	// 2. 创建并填充ShpFiles节点 
	QDomElement ShpFiles = fusionDoc.createElement("ShpFiles");
	fusionRoot.appendChild(ShpFiles);

	for (int row = 0; row < ShpFilePathTable->rowCount(); ++row) {
		QString fileName = ShpFilePathTable->item(row, 1)->text();
		QString filePath;

		// 查找匹配的文件路径 
		for (const QString& path : ShpFilePath) {
			if (QFileInfo(path).completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (filePath.isEmpty()) {
			qDebug() << "Path not found for:" << fileName;
			continue;
		}

		// 创建ShpFile节点 
		QDomElement imageElement = fusionDoc.createElement("ShpFile");

		// 添加ShpPath 
		QDomElement pathElement = fusionDoc.createElement("ShpPath");
		pathElement.appendChild(fusionDoc.createTextNode(filePath));
		imageElement.appendChild(pathElement);

		// 添加InnerID 
		QDomElement idElement = fusionDoc.createElement("InnerID");
		idElement.setAttribute("type", "int32");
		idElement.appendChild(fusionDoc.createTextNode(ShpFilePathTable->item(row, 0)->text()));
		imageElement.appendChild(idElement);

		ShpFiles.appendChild(imageElement);
		//processedFiles.insert(QFileInfo(filePath).completeBaseName());
	}
	// 创建ShpFile节点 
	QDomElement GlobalShpFile = fusionDoc.createElement("GlobalShpFile");
	GlobalShpFile.appendChild(fusionDoc.createTextNode(systemConfig->getGlobalShpFileValue()));
	ShpFiles.appendChild(GlobalShpFile);

	// 保存文件 
	QFile file(fusionImagePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Failed to create output file";
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	fusionDoc.save(out, 4);
	file.close();

	return true;
}

/**
 * @brief 从FusionImage.xml 读取融合模型数据到表格
 * @param fusionImagePath 输入文件路径
 * @param fusionModelTable 目标表格指针
 * @return 成功返回true，失败返回false
 */
bool IMAGEPS::readDataFromFusionImage(const QString& fusionImagePath,
	QTableWidget* fusionModelTable)
{
	// 1. 参数校验 
	if (!fusionModelTable) {
		qDebug() << "Invalid table widget";
		return false;
	}

	// 2. 读取XML文件 
	QFile xmlFile(fusionImagePath);
	if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open FusionImage.xml";
		return false;
	}

	QDomDocument doc;
	if (!doc.setContent(&xmlFile)) {
		xmlFile.close();
		qDebug() << "Failed to parse XML content";
		return false;
	}
	xmlFile.close();

	// 3. 清空表格 
	fusionModelTable->clearContents();
	fusionModelTable->setRowCount(0);

	// 4. 解析XML结构 
	QDomElement root = doc.documentElement();
	if (root.tagName() != "Fusion") {
		qDebug() << "Invalid root element, expected <Fusion>";
		return false;
	}

	QDomElement fusionModels = root.firstChildElement("FusionModels");
	if (fusionModels.isNull()) {
		qDebug() << "No FusionModels data found";
		return true; // 空数据不算错误 
	}

	QDomElement models = fusionModels.firstChildElement("Models");
	if (models.isNull()) {
		qDebug() << "No Models data found";
		return true;
	}

	// 5. 遍历模型数据 
	QDomNodeList modelNodes = models.childNodes();
	for (int i = 0; i < modelNodes.size(); ++i) {
		QDomNode node = modelNodes.at(i);
		if (!node.isElement() || !node.nodeName().startsWith("Model_")) {
			continue;
		}

		QDomElement model = node.toElement();

		// 添加新行 
		int row = fusionModelTable->rowCount();
		fusionModelTable->insertRow(row);

		// 6. 填充表格各列数据 
		// 第1列：序号 
		QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(row + 1));
		indexItem->setTextAlignment(Qt::AlignCenter);
		fusionModelTable->setItem(row, 0, indexItem);

		// 第2列：模型ID 
		QDomElement modelIdElem = model.firstChildElement("ModelID");
		QString modelId = modelIdElem.isNull() ? "" : modelIdElem.text();
		QTableWidgetItem* modelIdItem = new QTableWidgetItem(modelId);
		modelIdItem->setTextAlignment(Qt::AlignCenter);
		fusionModelTable->setItem(row, 1, modelIdItem);

		// 第3列：基准图像（优先取NadImageName，其次PanImageName）
		QString baseImage;
		QDomElement nadImageElem = model.firstChildElement("NadImageName");
		if (nadImageElem.isNull()) {
			nadImageElem = model.firstChildElement("PanImageName");
		}
		if (!nadImageElem.isNull()) {
			baseImage = QFileInfo(nadImageElem.text()).completeBaseName();
		}
		QTableWidgetItem* baseImageItem = new QTableWidgetItem(baseImage);
		baseImageItem->setTextAlignment(Qt::AlignCenter);
		fusionModelTable->setItem(row, 2, baseImageItem);

		// 第4列：匹配图像 
		QString matchImage;
		QDomElement mulImageElem = model.firstChildElement("MulImageName");
		if (!mulImageElem.isNull()) {
			matchImage = QFileInfo(mulImageElem.text()).completeBaseName();
		}
		QTableWidgetItem* matchImageItem = new QTableWidgetItem(matchImage);
		matchImageItem->setTextAlignment(Qt::AlignCenter);
		fusionModelTable->setItem(row, 3, matchImageItem);

		// 第5列：Matched状态（从SatImages获取）
		if (fusionModelTable->columnCount() > 4) {
			bool isMatched = false;
			QDomElement satImages = root.firstChildElement("SatImages");
			if (!satImages.isNull()) {
				QDomNodeList images = satImages.elementsByTagName("Image");
				for (int j = 0; j < images.size(); ++j) {
					QDomElement image = images.at(j).toElement();
					QString path = image.firstChildElement("ImagePath").text();
					if (QFileInfo(path).completeBaseName() == baseImage) {
						QDomElement matchedElem = image.firstChildElement("Matched");
						isMatched = matchedElem.text().compare("true", Qt::CaseInsensitive) == 0;
						break;
					}
				}
			}
			QString matchedDisplay = isMatched ? QString::fromLocal8Bit("是") : QString::fromLocal8Bit("否");
			QTableWidgetItem* matchedItem = new QTableWidgetItem(matchedDisplay);
			matchedItem->setTextAlignment(Qt::AlignCenter);
			fusionModelTable->setItem(row, 4, matchedItem);
		}

		// 第6列：Fusioned状态（从SatImages获取）
		if (fusionModelTable->columnCount() > 5) {
			bool isFusioned = false;
			QDomElement satImages = root.firstChildElement("SatImages");
			if (!satImages.isNull()) {
				QDomNodeList images = satImages.elementsByTagName("Image");
				for (int j = 0; j < images.size(); ++j) {
					QDomElement image = images.at(j).toElement();
					QString path = image.firstChildElement("ImagePath").text();
					QString currentImage = QFileInfo(path).completeBaseName();
					if (currentImage == baseImage || currentImage == matchImage) {
						QDomElement fusionedElem = image.firstChildElement("Fusioned");
						if (fusionedElem.text() == "true") {
							isFusioned = true;
							break;
						}
					}
				}
			}
			QString fusionedDisplay = isFusioned ? QString::fromLocal8Bit("是") : QString::fromLocal8Bit("否");
			QTableWidgetItem* fusionedItem = new QTableWidgetItem(fusionedDisplay);
			fusionedItem->setTextAlignment(Qt::AlignCenter);
			fusionModelTable->setItem(row, 5, fusionedItem);
		}
	}

	// 7. 自动调整列宽 
	fusionModelTable->resizeColumnsToContents();

	return true;
}

 /**
  * @brief 从FusionImage.xml  读取卫星图像数据到表格
  * @param fusionImagePath 输入文件路径
  * @param alignmentTable 目标表格指针
  * @param satImagePath 输出参数，存储所有图像路径
  * @return 成功返回true，失败返回false
  */
bool IMAGEPS::readSatImagesToAlignmentTable(const QString& fusionImagePath,
	QTableWidget* alignmentTable, QStringList& satImagePath)
{
	// 参数检查 
	if (!alignmentTable) {
		qDebug() << "Invalid table pointer";
		return false;
	}

	// 读取XML文件 
	QFile file(fusionImagePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open file";
		return false;
	}

	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		qDebug() << "Failed to parse XML";
		return false;
	}
	file.close();

	// 检查根节点 
	QDomElement root = doc.documentElement();
	if (root.tagName() != "Fusion") {
		qDebug() << "Invalid root element";
		return false;
	}

	// 清空表格 
	alignmentTable->clearContents();
	alignmentTable->setRowCount(0);

	// 获取SatImages节点 
	QDomElement satImages = root.firstChildElement("SatImages");
	if (satImages.isNull()) {
		qDebug() << "No SatImages found";
		return true; // 空文件不算错误 
	}

	// 获取FusionModels/Models节点及其所有Model_*子节点中的路径 
	QSet<QString> modelPaths;
	QDomElement fusionModels = root.firstChildElement("FusionModels");
	if (!fusionModels.isNull()) {
		QDomElement models = fusionModels.firstChildElement("Models");
		if (!models.isNull()) {
			QDomNodeList modelList = models.childNodes();
			for (int i = 0; i < modelList.size(); ++i) {
				QDomNode node = modelList.at(i);
				if (node.isElement() && node.nodeName().startsWith("Model_")) {
					QDomElement model = node.toElement();

					// 检查NadImageName 
					QDomElement nadImage = model.firstChildElement("NadImageName");
					if (!nadImage.isNull()) {
						modelPaths.insert(nadImage.text());
					}

					// 检查MulImageName 
					QDomElement mulImage = model.firstChildElement("MulImageName");
					if (!mulImage.isNull()) {
						modelPaths.insert(mulImage.text());
					}

					// 检查PanImageName（如果有）
					QDomElement panImage = model.firstChildElement("PanImageName");
					if (!panImage.isNull()) {
						modelPaths.insert(panImage.text());
					}
				}
			}
		}
	}

	// 遍历图像数据 
	QDomNodeList imageList = satImages.elementsByTagName("Image");
	for (int i = 0; i < imageList.size(); ++i) {
		QDomElement image = imageList.at(i).toElement();
		QString filePath = image.firstChildElement("ImagePath").text();

		// 添加到satImagePath（无论是否在Models中）
		if (!satImagePath.contains(filePath)) {
			satImagePath.append(filePath.replace("\\", "/"));
		}

		// 只有当路径不在Models中时才添加到表格 
		if (!modelPaths.contains(filePath)) {
			// 添加新行 
			int row = alignmentTable->rowCount();
			alignmentTable->insertRow(row);

			// 填充序号列 
			QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(row + 1));
			indexItem->setTextAlignment(Qt::AlignCenter);
			alignmentTable->setItem(row, 0, indexItem);

			// 填充文件名 
			QString fileName = QFileInfo(filePath).completeBaseName();
			QTableWidgetItem* nameItem = new QTableWidgetItem(fileName);
			nameItem->setTextAlignment(Qt::AlignCenter);
			alignmentTable->setItem(row, 1, nameItem);

			// 填充SensorID 
			QString sensorId = image.firstChildElement("SensorID").text();
			QTableWidgetItem* sensorItem = new QTableWidgetItem(sensorId);
			sensorItem->setTextAlignment(Qt::AlignCenter);
			alignmentTable->setItem(row, 2, sensorItem);

			// 填充InnerID（如果有该列）
			if (alignmentTable->columnCount() > 3) {
				QString innerId = image.firstChildElement("InnerID").text();
				QTableWidgetItem* innerItem = new QTableWidgetItem(innerId);
				innerItem->setTextAlignment(Qt::AlignCenter);
				alignmentTable->setItem(row, 3, innerItem);
			}
		}
	}

	// 调整列宽 
	alignmentTable->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(satImagePath);
	return true;
}


/**
 * @brief 将数据写入SmartMosaicImage.xml （带<Fusion>根标签）
 * @param fusionImagePath 输入件路径
 * @param alignmentIntFilePath 对齐文件路径列表
 * @param ShpFilePath 裁切文件路径列表
 * @param alignmentTable 对齐表格指针
 * @param ShpFilePathTable 裁切文件表格指针
 * @return 成功返回true，失败返回false
 */
bool IMAGEPS::readSmartMosaicImage(const QString& fusionImagePath,
	QStringList& alignmentIntFilePath,
	QStringList& ShpFilePath,
	QTableWidget* alignmentTable,
	QTableWidget* ShpFilePathTable)
{
	// 参数检查 
	if (!alignmentTable || !ShpFilePathTable) {
		qDebug() << "Invalid table pointer";
		return false;
	}

	// 读取XML文件 
	QFile file(fusionImagePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open file";
		return false;
	}

	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		qDebug() << "Failed to parse XML";
		return false;
	}
	file.close();

	// 检查根节点 
	QDomElement root = doc.documentElement();
	if (root.tagName() != "SmartMosaic") {
		qDebug() << "Invalid root element";
		return false;
	}

	// 清空表格 
	alignmentTable->clearContents();
	alignmentTable->setRowCount(0);

	// 获取SatImages节点 
	QDomElement satImages = root.firstChildElement("SatImages");
	if (satImages.isNull()) {
		qDebug() << "No SatImages found";
		return true; // 空文件不算错误 
	}

	// 遍历图像数据 
	QDomNodeList imageList = satImages.elementsByTagName("Image");
	for (int i = 0; i < imageList.size(); ++i) {
		QDomElement image = imageList.at(i).toElement();
		QString filePath = image.firstChildElement("ImagePath").text();

		// 添加到alignmentIntFilePath
		if (!alignmentIntFilePath.contains(filePath)) {
			alignmentIntFilePath.append(filePath.replace("\\", "/"));
		}

		// 添加新行 
		int row = alignmentTable->rowCount();
		alignmentTable->insertRow(row);

		// 填充序号列 
		QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(row + 1));
		indexItem->setTextAlignment(Qt::AlignCenter);
		alignmentTable->setItem(row, 0, indexItem);

		// 填充文件名 
		QString fileName = QFileInfo(filePath).completeBaseName();
		QTableWidgetItem* nameItem = new QTableWidgetItem(fileName);
		nameItem->setTextAlignment(Qt::AlignCenter);
		alignmentTable->setItem(row, 1, nameItem);

		// 填充SensorID 
		QString sensorId = image.firstChildElement("SensorID").text();
		QTableWidgetItem* sensorItem = new QTableWidgetItem(sensorId);
		sensorItem->setTextAlignment(Qt::AlignCenter);
		alignmentTable->setItem(row, 2, sensorItem);

		// 填充StellarSource
		QString StellarSourceId = image.firstChildElement("StellarSource").text();
		QTableWidgetItem* StellarSourceItem = new QTableWidgetItem(StellarSourceId);
		StellarSourceItem->setTextAlignment(Qt::AlignCenter);
		alignmentTable->setItem(row, 3, StellarSourceItem);

		// 填充Timelike
		QString Timelike = image.firstChildElement("Timelike").text();
		QTableWidgetItem* TimelikeItem = new QTableWidgetItem(Timelike);
		TimelikeItem->setTextAlignment(Qt::AlignCenter);
		alignmentTable->setItem(row, 4, TimelikeItem);
	}

	// 调整列宽 
	alignmentTable->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(alignmentIntFilePath);

	// 获取ShpFiles节点 
	QDomElement ShpFiles = root.firstChildElement("ShpFiles");
	if (ShpFiles.isNull()) {
		qDebug() << "No ShpFiles found";
		return true; // 空文件不算错误 
	}

	// 遍历图像数据 
	QDomNodeList ShpFileList = ShpFiles.elementsByTagName("ShpFile");
	for (int i = 0; i < ShpFileList.size(); ++i) {
		QDomElement image = ShpFileList.at(i).toElement();
		QString filePath = image.firstChildElement("ShpPath").text();

		// 添加到ShpFilePath
		if (!ShpFilePath.contains(filePath)) {
			ShpFilePath.append(filePath.replace("\\", "/"));
		}

		// 添加新行 
		int row = ShpFilePathTable->rowCount();
		ShpFilePathTable->insertRow(row);

		// 填充序号列 
		QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(row + 1));
		indexItem->setTextAlignment(Qt::AlignCenter);
		ShpFilePathTable->setItem(row, 0, indexItem);

		// 填充文件名 
		QString fileName = QFileInfo(filePath).completeBaseName();
		QTableWidgetItem* nameItem = new QTableWidgetItem(fileName);
		nameItem->setTextAlignment(Qt::AlignCenter);
		ShpFilePathTable->setItem(row, 1, nameItem);

	}
	ShpFilePathTable->resizeColumnsToContents();
	QString globalShpPath = ShpFiles.firstChildElement("GlobalShpFile").text();
	if (!globalShpPath.isEmpty()) {
		systemConfig->writeGlobalShpFileValue(globalShpPath.replace("\\", "/"));
	}

	return true;
}

// 保存加密点到XML文件 
bool IMAGEPS::writeEncryptedPointsToXml(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning() << "Failed to open file for writing:" << filePath;
		return false;
	}

	QXmlStreamWriter xmlWriter(&file);
	xmlWriter.setAutoFormatting(true);
	xmlWriter.writeStartDocument();
	xmlWriter.writeStartElement("PSEncPoints");

	int pointNumber = 0;
	for (auto it = m_encryptedPoints.begin(); it != m_encryptedPoints.end(); ++it) {
		const EncryptedPoint& point = it.value();

		xmlWriter.writeStartElement("PSEncPoint");
		xmlWriter.writeAttribute("Number", QString::number(pointNumber++));
		xmlWriter.writeAttribute("ID", point.id);
		xmlWriter.writeAttribute("Overlap", QString::number(point.overlap));
		xmlWriter.writeAttribute("Residual", point.residual);
		xmlWriter.writeAttribute("State", point.state);
		xmlWriter.writeAttribute("PointX", point.pointX);
		xmlWriter.writeAttribute("PointY", point.pointY);
		xmlWriter.writeAttribute("PointZ", point.pointZ);

		// 写入卫星影像数据 
		for (const SatImageData& satImage : point.satImages) {
			xmlWriter.writeStartElement("SatImage");
			xmlWriter.writeAttribute("Number", "0");
			xmlWriter.writeAttribute("SatImagePath", satImage.path);
			xmlWriter.writeAttribute("imageX", QString::number(satImage.imageX, 'f', 4));
			xmlWriter.writeAttribute("imageY", QString::number(satImage.imageY, 'f', 4));
			xmlWriter.writeAttribute("imageVX", QString::number(0.0, 'f', 4)); // 默认值 
			xmlWriter.writeAttribute("imageVY", QString::number(0.0, 'f', 4)); // 默认值 
			xmlWriter.writeEndElement();  // SatImage 
		}

		xmlWriter.writeEndElement();  // PSEncPoint 
	}

	xmlWriter.writeEndElement();  // PSEncPoints 
	xmlWriter.writeEndDocument();

	file.close();
	return true;
}

// 从XML文件读取加密点 
bool IMAGEPS::readEncryptedPointsFromXml(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Failed to open file for reading:" << filePath;
		return false;
	}

	m_encryptedPoints.clear();

	QXmlStreamReader xmlReader(&file);
	while (!xmlReader.atEnd() && !xmlReader.hasError()) {
		QXmlStreamReader::TokenType token = xmlReader.readNext();

		if (token == QXmlStreamReader::StartElement && xmlReader.name() == "PSEncPoint") {
			EncryptedPoint point;

			QXmlStreamAttributes attributes = xmlReader.attributes();
			point.id = attributes.value("ID").toString();
			point.overlap = attributes.value("Overlap").toInt();
			point.residual = attributes.value("Residual").toString();
			point.state = attributes.value("State").toString();
			point.pointX = attributes.value("PointX").toString();
			point.pointY = attributes.value("PointY").toString();
			point.pointZ = attributes.value("PointZ").toString();

			// 读取卫星影像数据 
			while (!(xmlReader.readNext() == QXmlStreamReader::EndElement &&
				xmlReader.name() == "PSEncPoint")) {
				if (xmlReader.name() == "SatImage" &&
					xmlReader.tokenType() == QXmlStreamReader::StartElement) {
					SatImageData satImage;
					QXmlStreamAttributes satAttrs = xmlReader.attributes();
					satImage.path = satAttrs.value("SatImagePath").toString();
					satImage.imageX = satAttrs.value("imageX").toDouble();
					satImage.imageY = satAttrs.value("imageY").toDouble();
					point.satImages.append(satImage);
				}
			}

			m_encryptedPoints.insert(point.id, point);
		}
	}

	if (xmlReader.hasError()) {
		qWarning() << "XML parsing error:" << xmlReader.errorString();
		return false;
	}

	file.close();

	// 初始化UI 
	m_currentPage = 0;
	updateVisiblePoints();

	ui.ImageInfoShow_Widget->loadPoints(m_encryptedPoints, QHash<QString, ControlPoint>(), "con");
	ui.ImageInfoShow_Widget->showPoints(true);

	return true;
}

/**
 * @brief 从XML文件读取控制点数据
 */
bool IMAGEPS::readControlPointsFromXml(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Failed to open XML file:" << filePath;
		return false;
	}

	QXmlStreamReader xml(&file);
	m_controlPoints.clear();
	m_visibleControlPointIds.clear();

	while (!xml.atEnd() && !xml.hasError()) {
		QXmlStreamReader::TokenType token = xml.readNext();
		if (token == QXmlStreamReader::StartElement && xml.name() == "PSColPoint") {
			ControlPoint point;
			QXmlStreamAttributes attributes = xml.attributes();

			point.id = attributes.value("ID").toString();
			point.type = attributes.value("Type").toString();
			point.overlap = attributes.value("Overlap").toInt();
			point.imageSide = attributes.value("ImageSide").toString();
			point.objectSide = attributes.value("ObjectSide").toString();
			point.height = attributes.value("Height").toString();
			point.state = attributes.value("State").toString();
			point.priority = attributes.value("Priority").toString();
			point.pointX = attributes.value("PointX").toString();
			point.pointY = attributes.value("PointY").toString();
			point.pointZ = attributes.value("PointZ").toString();

			// 读取SatImage子节点
			while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "PSColPoint")) {
				xml.readNext();
				if (xml.tokenType() == QXmlStreamReader::StartElement && xml.name() == "SatImage") {
					SatImageData satImage;
					QXmlStreamAttributes satAttributes = xml.attributes();

					satImage.path = satAttributes.value("SatImagePath").toString();
					satImage.imageX = satAttributes.value("imageX").toDouble();
					satImage.imageY = satAttributes.value("imageY").toDouble();
					satImage.imageVX = satAttributes.value("imageVX").toDouble();
					satImage.imageVY = satAttributes.value("imageVY").toDouble();

					point.satImages.append(satImage);
				}
			}

			m_controlPoints.insert(point.id, point);
		}
	}

	if (xml.hasError()) {
		qWarning() << "XML parse error:" << xml.errorString();
		return false;
	}

	file.close();

	// 初始化可见点ID列表 
	m_visibleControlPointIds = m_controlPoints.keys();
	std::sort(m_visibleControlPointIds.begin(), m_visibleControlPointIds.end(),
		[](const QString &a, const QString &b) {
		return a.toInt() < b.toInt();
	});

	m_controlPoints_conBack = m_controlPoints;
	m_visibleControlPointIds_conBack = m_visibleControlPointIds;

	ui.ImageInfoShow_Widget->loadPoints(QHash<QString, EncryptedPoint>(), m_controlPoints, "col");
	ui.ImageInfoShow_Widget->showPoints(true);

	// 加载第一页数据 
	loadControlPointCurrentPage();

	return true;
}

/**
 * @brief 将控制点数据写入XML文件
 */
bool IMAGEPS::writeControlPointsToXml(const QString& filePath)
{
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning() << "Failed to open XML file for writing:" << filePath;
		return false;
	}

	QXmlStreamWriter xml(&file);
	xml.setAutoFormatting(true);
	xml.writeStartDocument();
	xml.writeStartElement("PSColPoints");

	for (auto it = m_controlPoints.begin(); it != m_controlPoints.end(); ++it) {
		const ControlPoint& point = it.value();

		xml.writeStartElement("PSColPoint");
		xml.writeAttribute("Number", "0");
		xml.writeAttribute("ID", point.id);
		xml.writeAttribute("Type", point.type);
		xml.writeAttribute("Overlap", QString::number(point.overlap));
		xml.writeAttribute("ImageSide", point.imageSide);
		xml.writeAttribute("ObjectSide", point.objectSide);
		xml.writeAttribute("Height", point.height);
		xml.writeAttribute("State", point.state);
		xml.writeAttribute("Priority", point.priority);
		xml.writeAttribute("PointX", point.pointX);
		xml.writeAttribute("PointY", point.pointY);
		xml.writeAttribute("PointZ", point.pointZ);

		// 写入SatImage子节点
		for (const SatImageData& satImage : point.satImages) {
			xml.writeStartElement("SatImage");
			xml.writeAttribute("Number", "0");
			xml.writeAttribute("SatImagePath", satImage.path);
			xml.writeAttribute("imageX", QString::number(satImage.imageX, 'f', 6));
			xml.writeAttribute("imageY", QString::number(satImage.imageY, 'f', 6));
			xml.writeAttribute("imageVX", QString::number(satImage.imageVX, 'f', 6));
			xml.writeAttribute("imageVY", QString::number(satImage.imageVY, 'f', 6));
			xml.writeEndElement();  // SatImage 
		}

		xml.writeEndElement();  // PSColPoint
	}

	xml.writeEndElement();  // PSColPoints
	xml.writeEndDocument();

	file.close();
	return true;
}

///////////////////////////////
/**
 * @brief 通用写入和读取数据函数(3列数据列表)
 */
void IMAGEPS::writeImageXml(QTableWidget* tableWidget, const QStringList& filePathList, const QString& xmlFileName)
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	// 创建新的XML文档结构 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("SatImages");
	doc.appendChild(root);

	// 遍历表格 
	for (int row = 0; row < tableWidget->rowCount(); ++row) {
		// 获取第2列的文件名 
		QString fileName = tableWidget->item(row, 1)->text();

		// 检查是否有第3列（SensorID）
		QString sensorID;
		if (tableWidget->columnCount() > 2) {
			sensorID = tableWidget->item(row, 2)->text();
		}

		// 在文件路径列表中查找匹配的文件路径 
		QString filePath;
		for (const QString &path : filePathList) {
			QFileInfo info(path);
			if (info.completeBaseName() == fileName) {
				filePath = path;
				break;
			}
		}

		if (!filePath.isEmpty()) {
			// 创建Image节点 
			QDomElement imageElement = doc.createElement("Image");
			root.appendChild(imageElement);

			// 创建ImagePath节点 
			QDomElement pathElement = doc.createElement("ImagePath");
			pathElement.appendChild(doc.createTextNode(filePath));
			imageElement.appendChild(pathElement);

			// 创建InnerID节点 
			QDomElement innerIdElement = doc.createElement("InnerID");
			innerIdElement.setAttribute("type", "int32");
			innerIdElement.appendChild(doc.createTextNode(QString::number(row + 1)));
			imageElement.appendChild(innerIdElement);

			// 创建SensorID节点（仅在表格有第3列时写入）
			if (tableWidget->columnCount() > 2) {
				QDomElement sensorElement = doc.createElement("SensorID");
				sensorElement.appendChild(doc.createTextNode(sensorID));
				imageElement.appendChild(sensorElement);
			}
		}
	}

	// 将XML写入文件 (覆盖原有文件)
	QFile file(projectdir + xmlFileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法写入" + xmlFileName);
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();
}

void IMAGEPS::loadImageData(QTableWidget* tableWidget, QStringList& filePathList, const QString& xmlFileName)
{
	if (projectdir.isEmpty()) {
		QMessageBox::warning(this, "Warning", u8"请先创建或打开工程");
		return;
	}

	QFile file(projectdir + xmlFileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::critical(this, "Error", u8"无法读取" + xmlFileName);
		return;
	}

	if (file.size() == 0) {
		file.close();
		return;
	}

	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::critical(this, "Error", xmlFileName + u8" 格式错误");
		return;
	}
	file.close();

	// 清空表格 
	tableWidget->setRowCount(0);

	QDomElement root = doc.documentElement();
	QDomNodeList imageList = root.elementsByTagName("Image");

	for (int i = 0; i < imageList.count(); ++i) {
		QDomElement imageElement = imageList.at(i).toElement();
		QDomElement pathElement = imageElement.firstChildElement("ImagePath");
		QDomElement sensorElement = imageElement.firstChildElement("SensorID");

		if (!pathElement.isNull()) {
			QString filePath = pathElement.text();
			QString sensorID;

			// 仅在存在SensorID节点时读取
			if (!sensorElement.isNull()) {
				sensorID = sensorElement.text();
			}

			QFileInfo fileInfo(filePath);

			// 添加到文件路径列表 
			filePathList.append(filePath);

			// 添加到表格 
			int newRow = tableWidget->rowCount();
			tableWidget->insertRow(newRow);

			// 确保所有列都有item 
			for (int col = 0; col < tableWidget->columnCount(); ++col) {
				if (!tableWidget->item(newRow, col)) {
					QTableWidgetItem* item = new QTableWidgetItem;
					item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					tableWidget->setItem(newRow, col, item);
				}
			}

			// 设置数据 
			tableWidget->item(newRow, 0)->setText(QString::number(newRow + 1));
			tableWidget->item(newRow, 1)->setText(fileInfo.completeBaseName());

			// 仅在表格有第3列时设置SensorID 
			if (tableWidget->columnCount() > 2) {
				tableWidget->item(newRow, 2)->setText(sensorID);
			}
		}
	}
	tableWidget->resizeColumnsToContents();
	setGeoreferenceFromXMLAndRPC(filePathList);
}
//////////////////////////////

void IMAGEPS::systemSetSlot()
{
	if (!systemConfig)
	{
		systemConfig = new SystemConfig(this, this);
	}
	systemConfig->show();
}

void IMAGEPS::dataModelConfigActionSlot()
{
	dataModelConfig->logEdit = ui.runLog_Edit;
	ui.log_TabW->setCurrentIndex(0);
	dataModelConfig->loadFile(DataModelPath);
	dataModelConfig->show();
}

/**
 * @brief 连接点匹配
 */
void IMAGEPS::connectPointsMatchActionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("连接点匹配")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		// 1. 设置界面状态 - 只显示指定的功能项
		QStringList visibleItems = { QString::fromLocal8Bit("运行环境"), QString::fromLocal8Bit("影像匹配")};
		for (int i = 0; i < systemConfig->ui.funListWidget->count(); ++i) {
			QListWidgetItem* item = systemConfig->ui.funListWidget->item(i);
			item->setHidden(!visibleItems.contains(item->text()));
		}

		systemConfig->settingShowWidget(QString::fromLocal8Bit("影像匹配"));

		// 2. 显示模态对话框
		QEventLoop loop;
		connect(systemConfig, &SystemConfig::accepted, &loop, &QEventLoop::quit);
		connect(systemConfig, &SystemConfig::rejected, &loop, &QEventLoop::quit);

		int result = systemConfig->exec();

		// 用户取消操作 
		if (result != QDialog::Accepted) {
			return;
		}

		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		// 检查表格第4列是否全部为"是"
		bool allYes = true;
		for (int row = 0; row < dataModelConfig->ui.tableWidget_2->rowCount(); ++row) {
			QTableWidgetItem* item = dataModelConfig->ui.tableWidget_2->item(row, 3);
			if (!item || item->text() != QString::fromLocal8Bit("是")) {
				allYes = false;
				break;
			}
		}

		// 如果全部为"是"，则直接返回不执行后续逻辑
		if (allYes && dataModelConfig->ui.tableWidget_2->rowCount() != 0) {
			QDateTime startTime = QDateTime::currentDateTime();
			PROJECT_LOG_INFO(this->CurrentConfig, QString::fromLocal8Bit("连接点匹配时模型配对列表是否匹配列都为是"));
			PublicFunctions::writeTimestampToXml("SatTiePointMatch", this->projectdir);

			ui.runLog_Edit->append(QString::fromLocal8Bit("****连接点匹配"));
			ui.runLog_Edit->append(QString::fromLocal8Bit("****开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

			ui.runLog_Edit->append(QString::fromLocal8Bit("****当前") + QString::number(dataModelConfig->ui.tableWidget_2->rowCount()) + QString::fromLocal8Bit("个模型已存在匹配成功数据"));

			QDateTime endTime = QDateTime::currentDateTime();
			ui.runLog_Edit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			ui.runLog_Edit->append(QString::fromLocal8Bit("****连接点匹配耗时：") + systemConfig->calculateTimeDifference(startTime, endTime));
			return;
		}

		// 先断开之前的连接（避免重复调用）
		disconnect(systemConfig, &SystemConfig::EncryptedPointsMatchFinished,
			this, qOverload<>(&IMAGEPS::EncryptedPointsList));
		// 重新连接信号 
		connect(systemConfig, &SystemConfig::EncryptedPointsMatchFinished,
			this, qOverload<>(&IMAGEPS::EncryptedPointsList));

		disconnect(m_encryptedPointsConnection);

		// 使用 lambda 表达式重新连接信号   
		m_encryptedPointsConnection = connect(systemConfig, &SystemConfig::EncryptedPointsMatchFinished,
			dataModelConfig, [=]() {
			for (int row = 0; row < dataModelConfig->ui.tableWidget_2->rowCount(); ++row) {
				// 创建新的 QTableWidgetItem 
				QTableWidgetItem *item = new QTableWidgetItem(QString::fromLocal8Bit("是"));

				item->setTextAlignment(Qt::AlignCenter);
				// 设置该项到指定行列 
				dataModelConfig->ui.tableWidget_2->setItem(row, 3, item);
			}
		});

		systemConfig->connectPointsMatch(DEMFilePath, DOMFilePath,"SatTiePointMatch");
	}
}

/**
 * @brief 控制点匹配
 */
void IMAGEPS::controlPointsMatchActionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("控制点匹配")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		// 先断开之前的连接（避免重复调用）
		disconnect(systemConfig, &SystemConfig::controlPointsMatchFinished,
			this, qOverload<>(&IMAGEPS::ControlPointsList));
		// 重新连接信号
		connect(systemConfig, &SystemConfig::controlPointsMatchFinished,
			this, qOverload<>(&IMAGEPS::ControlPointsList));


		systemConfig->controlPointsMatch(DEMFilePath, DOMFilePath, DataModelPath, controlPointsPath);
	}
}

/**
 * @brief 自由网平差
 */
void IMAGEPS::FreeNetworkAdjustmentActionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("自由网平差")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);
		dataModelConfig->BuildSatModelMakerCmdXML(DataModelPath, "SatBA"); //构建SatModelMakerCmd任务单

		// 先断开之前的连接（避免重复调用）
		disconnect(systemConfig, &SystemConfig::FreeNetworkAdjustFinished,
			this, qOverload<>(&IMAGEPS::EncreadResidualData));
		// 重新连接信号
		connect(systemConfig, &SystemConfig::FreeNetworkAdjustFinished,
			this, qOverload<>(&IMAGEPS::EncreadResidualData));

		systemConfig->FreeNetworkAdjust(DEMFilePath);
	}
}

/**
 * @brief 控制网平差
 */
void IMAGEPS::controlNetAdjustmentActionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("控制网平差")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);
		dataModelConfig->BuildSatModelMakerCmdXML(DataModelPath, "CtlPointMatch"); //构建SatModelMakerCmd任务单

		// 先断开之前的连接（避免重复调用）
		disconnect(systemConfig, &SystemConfig::controlNetAdjustFinished,
			this, qOverload<>(&IMAGEPS::ConResidualFiles));
		// 重新连接信号
		connect(systemConfig, &SystemConfig::controlNetAdjustFinished,
			this, qOverload<>(&IMAGEPS::ConResidualFiles));

		systemConfig->controlNetAdjust(DEMFilePath);
	}
}

/**
 * @brief 正射纠正
 */
void IMAGEPS::orthorectificAtionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("正射纠正")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		// 从projectdir + "Ortho"目录下读取文件作为OrtholoutPath 
		QString orthoDir = projectdir + "Ortho" + "/";
		QDir orthoDirectory(orthoDir);
		QStringList orthoFiles = orthoDirectory.entryList(QDir::Files | QDir::Readable);
		QSet<QString> orthoFileNames;
		for (const auto& file : orthoFiles) {
			QFileInfo fileInfo(file);
			orthoFileNames.insert(fileInfo.completeBaseName());
		}

		// 使用临时变量保存最终要处理的路径 
		QStringList finalAlignmentPaths = DataModelPath;
		QStringList duplicateFiles;

		// 检查重复文件 
		for (const auto& path : finalAlignmentPaths) {
			QFileInfo fileInfo(path);
			if (orthoFileNames.contains(fileInfo.completeBaseName())) {
				duplicateFiles.append(path);
			}
		}

		// 如果有重复文件 
		if (!duplicateFiles.isEmpty()) {
			int num = duplicateFiles.size();
			QString message = QString::fromLocal8Bit("已经存在%1个纠正成果，是否重新纠正？").arg(num);

			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(this, QString::fromLocal8Bit("纠正成果确认"),
				message,
				QMessageBox::Yes | QMessageBox::No);

			if (reply == QMessageBox::No) {
				// 如果所有文件都重复且用户选择不重新处理，则直接退出 
				if (duplicateFiles.size() == DataModelPath.size()) {
					ui.runLog_Edit->append(QString::fromLocal8Bit(" 用户取消重新纠正已存在的成果"));
					return;
				}

				// 否则只移除重复文件 
				for (const auto& path : duplicateFiles) {
					finalAlignmentPaths.removeAll(path);
				}
			}
		}

		// 如果没有需要处理的文件，直接返回 
		if (finalAlignmentPaths.isEmpty()) {
			PROJECT_LOG_INFO(this->CurrentConfig, QString::fromLocal8Bit("正射纠正 没有需要处理的影像文件"));

			ui.runLog_Edit->append(QString::fromLocal8Bit(" 没有需要处理的影像文件"));
			return;
		}

		copyAndRenameRpcFiles(finalAlignmentPaths);

		systemConfig->orthorectification(DEMFilePath, finalAlignmentPaths);
	}
}

/**
 * @brief 影像融合
 */
void IMAGEPS::imageInterActionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("影像融合")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		QString outputPath = projectdir + QString::fromLocal8Bit("Ortho/");

		QDir directory(outputPath);

		// 定义要匹配的文件扩展名（不区分大小写）
		QStringList filters;
		filters << "*.tif" << "*.TIF"
			<< "*.img" << "*.IMG"
			<< "*.envi" << "*.ENVI"
			<< "*.pix" << "*.PIX";

		// 获取所有匹配的文件路径（绝对路径）
		QStringList files = directory.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);

		// 转换为完整路径
		QStringList fullPaths;
		for (const QString &file : files) {
			fullPaths.append(directory.absoluteFilePath(file));
		}

		QStringList filteredFiles = systemConfig->filterSatelliteImagesByColumn(fullPaths, 10);

		dataModelConfig->BuildSatModelMakerCmdXML(filteredFiles, "Fusion"); //构建SatModelMakerCmd任务单

		systemConfig->imageInterAction();
	}
}

/**
 * @brief 真彩色转换
 */
void IMAGEPS::trueColorConveractionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("真彩色转换")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		QStringList filenamelist = QStringList();
		systemConfig->TrueColorConversion(filenamelist);
	}
}

/**
 * @brief 影像匀色
 */
void IMAGEPS::DodgingactionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("影像匀色")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		QStringList filenamelist = QStringList();

		QString outputDir;
		if (!systemConfig->ConvertTo8Bitflag && systemConfig->ui.output8bImage_checkBox->isChecked()) {
			// 修改输出目录为Dodging目录 
			outputDir = projectdir + "Fusion/";
		}
		else {
			outputDir = projectdir + "OutByte/";
		}
		QDir dir(outputDir);

		//QDir dir(projectdir + QString::fromLocal8Bit("OutByte/"));

		if (!dir.exists()) {
			PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("影像匀色") + outputDir + QString::fromLocal8Bit("文件不存在"));

			//qWarning() << "文件夹不存在:";
			return;
		}

		// 设置名称过滤器（不区分大小写）
		QStringList filters;
		filters << "*.tif" << "*.TIF" << "*.tiff" << "*.TIFF"
			<< "*.img" << "*.IMG";

		// 获取所有符合条件的文件 
		QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

		// 转换为绝对路径 
		for (const QFileInfo &fileInfo : fileList) {
			//QString Type = systemConfig->getTiffImageType(fileInfo.absoluteFilePath());
			//if (Type == "MSS")
				filenamelist.append(fileInfo.absoluteFilePath());
		}

		systemConfig->ImageColorCorrection(filenamelist);
	}
}

/**
 * @brief 影像镶嵌
 */
void IMAGEPS::ImageMosaicactionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("影像镶嵌")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		QStringList filenamelist = QStringList();

		QDir dir(projectdir + QString::fromLocal8Bit("Dodging/"));

		if (!dir.exists()) {
			PROJECT_LOG_ERROR(this->CurrentConfig, projectdir + QString::fromLocal8Bit("Dodging/") +  QString::fromLocal8Bit("文件夹不存在"));

			//qWarning() << "文件夹不存在:" ;
			return;
		}

		// 设置名称过滤器（不区分大小写）
		QStringList filters;
		filters << "*.tif" << "*.TIF" << "*.tiff" << "*.TIFF"
			<< "*.img" << "*.IMG";

		// 获取所有符合条件的文件 
		QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

		// 转换为绝对路径 
		for (const QFileInfo &fileInfo : fileList) {
			QString Type = systemConfig->getTiffImageType(fileInfo.absoluteFilePath());
			filenamelist.append(fileInfo.absoluteFilePath());
		}

		systemConfig->ImageMosaic(filenamelist);
	}
}

/**
 * @brief 智能镶嵌
 */
void IMAGEPS::SmartMosaicactionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("智能镶嵌")))
		return;
	if (!OrthoRectificationFusionflag)
	{
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);

		//QStringList filenamelist = QStringList();

		//QDir dir(projectdir + QString::fromLocal8Bit("Dodging/"));

		//if (!dir.exists()) {
		//	//qWarning() << "文件夹不存在:";
		//	PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("影像匀色成果文件夹不存在"));
		//	ui.runLog_Edit->append(QString::fromLocal8Bit(" 影像匀色成果文件夹不存在"));
		//	return;
		//}

		//// 设置名称过滤器（不区分大小写）
		//QStringList filters;
		//filters << "*.tif" << "*.TIF" << "*.tiff" << "*.TIFF"
		//	<< "*.img" << "*.IMG";

		//// 获取所有符合条件的文件 
		//QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

		//// 转换为绝对路径 
		//for (const QFileInfo &fileInfo : fileList) {
		//	QString Type = systemConfig->getTiffImageType(fileInfo.absoluteFilePath());
		//	if (Type == "MSS")
		//		filenamelist.append(fileInfo.absoluteFilePath());
		//}
		SmartMosaicFilePath = syncFileOrderWithTable(SmartMosaicFilePath,
			ui.SmartMosaicDataList_TableW);
		QStringList MosaicCropFilePath;
		if (!systemConfig->ui.lineEdit_37->text().trimmed().isEmpty()) {
			MosaicCropFilePath.append(systemConfig->ui.lineEdit_37->text());
		}

		systemConfig->SmartMosaic(SmartMosaicFilePath, MosaicCropFilePath);
	}
}

/**
 * @brief 影像生产
 */
void IMAGEPS::OrthoRectificationFusion_actionSlot()
{
	if (!showConfirmationDialog(QString::fromLocal8Bit("影像生产")))
		return;
	OrthoRectificationFusionflag = true;
	systemConfig->logEdit = ui.runLog_Edit;
	ui.log_TabW->setCurrentIndex(0);

	m_connection = connect(systemConfig, &SystemConfig::FunctionAbnormalExit, this,
		[=](QString title) {
		OrthoRectificationFusionflag = false;

		if (m_connection) {
			disconnect(m_connection);
			m_connection = QMetaObject::Connection();
		}
	});

	systemConfig->OrthoRectificationFusion(DEMFilePath, DOMFilePath, DataModelPath);

}

////////////////////////////////////
/**
 * @brief 参考数据右键菜单
 */
void IMAGEPS::referDataList_TabWContextSlot(const QPoint &pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/参考数据列表.csv"), ",");
	for (auto data : config)
	{
		//if (data[1] == "TRUE")
		//	actionMap[data[0]] = menu->addAction(data[0]);
		if (data[1] == "TRUE") {
			QAction* action = menu->addAction(data[0]);
			actionMap[data[0]] = action;

			if (data[0] == QString::fromLocal8Bit("卸载参考数据")) {
				action->setEnabled(!m_isProcessingImageOpen);  // 根据处理状态启用/禁用 
			}
		}
	}

	// 加载参考 DOM   
	connect(actionMap[QString::fromLocal8Bit("加载参考DOM")], &QAction::triggered, this, [=] {
		QStringList domPaths = QFileDialog::getOpenFileNames(
			this,
			QString::fromLocal8Bit("选择加载参考DOM文件"),
			m_lastPath,
			tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)"))
		);
		QTableWidget* table = ui.refereDataList_TableW;

		if (!domPaths.isEmpty()) {
			m_lastPath = QFileInfo(domPaths.first()).path();  // 更新最后路径 

			//std::vector<int> existingRow;
			int indexDom = 0;
			//TmpDOMFilePath.clear();
			for (const QString &domPath : domPaths) {
				QFileInfo fileInfo(domPath);
				QString fileName = fileInfo.completeBaseName();  // 去掉扩展名 

				// 检查是否已存在相同文件名 
				bool containsExit = true;
				int existingRow = -1;
				//existingRow.push_back(-1);
				for (int i = 0; i < table->rowCount(); i++) {
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DOM")) {
						containsExit = false;
						break;
					}
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
						//existingRow[indexDom] = i;
						existingRow = i;
						break;
					}
				}
				if (containsExit) {
					DOMFilePath.push_back(domPath);
					int newRowIndex = table->rowCount();
					table->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem 
					for (int col = 0; col < table->columnCount(); col++) {
						if (!table->item(newRowIndex, col)) {
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							table->setItem(newRowIndex, col, item);
						}
					}

					int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);

					table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
					table->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());                   // 文件名 
					table->item(newRowIndex, 2)->setText("DOM");                      // DOM 
					table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
				}
				indexDom++;
			}

			//on_actionOpenImage(TmpDOMFilePath, u8"DOM");
			auto fileLists = processImageWithOperator_test(DOMFilePath);
			QStringList existingFiles = fileLists.first;
			QStringList newFiles = fileLists.second;
			
			on_actionOpenImage(existingFiles, newFiles, u8"DOM");

			//QStringList tmpPath = ui.ImageInfoShow_Widget->getExcludedReferences();
			//ui.ImageInfoShow_Widget->m_excludedReferences.clear();

			//QSet<QString> domSet = QSet<QString>::fromList(domPaths);
			//QSet<QString> tmpSet = QSet<QString>::fromList(tmpPath);

			//// 从tmpPath中减去domPaths中已有的路径 
			//domSet.subtract(tmpSet);

			//PROJECT_LOG_INFO(this->CurrentConfig, (QString::fromLocal8Bit("TmpDOMFilePath: tmpSet : domSet") + QString::number(TmpDOMFilePath.size())+ " : " + QString::number(tmpSet.size()) +  " : " + QString::number(domSet.size())));
			//indexDom = 0;
			//for (auto filePath : domSet.toList())
			//{
			//	QFileInfo fileInfo(filePath);
			//	QString fileName = fileInfo.completeBaseName();

			//	bool containsExit = true;
			//	int existingRow = -1;
			//	//existingRow.push_back(-1);
			//	for (int i = 0; i < table->rowCount(); i++) {
			//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DOM")) {
			//			containsExit = false;
			//			break;
			//		}
			//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
			//			//existingRow[indexDom] = i;
			//			existingRow = i;
			//			break;
			//		}
			//	}
			//	if (containsExit) {
			//		DOMFilePath.push_back(filePath);
			//		int newRowIndex = table->rowCount();
			//		table->insertRow(newRowIndex);

			//		// 确保所有单元格都有 QTableWidgetItem 
			//		for (int col = 0; col < table->columnCount(); col++) {
			//			if (!table->item(newRowIndex, col)) {
			//				QTableWidgetItem* item = new QTableWidgetItem;
			//				item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			//				table->setItem(newRowIndex, col, item);
			//			}
			//		}

			//		int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);

			//		table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
			//		table->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());                   // 文件名 
			//		table->item(newRowIndex, 2)->setText("DOM");                      // DOM 
			//		table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
			//		indexDom++;
			//	}
			//}
		}
		// 添加后按第一列(文件名)升序排序 
		table->sortItems(0, Qt::AscendingOrder);
		table->resizeColumnsToContents();
		//on_actionOpenImage(DOMFilePath, u8"DOM");
	});

	connect(actionMap[QString::fromLocal8Bit("加载参考DOM(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载参考DOM(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QTableWidget* table = ui.refereDataList_TableW;
		//TmpDOMFilePath.clear();
		for (const QString &domPath : filePaths) {
			if (!domPath.isEmpty()) {
				QFileInfo fileInfo(domPath);
				QString fileName = fileInfo.completeBaseName();  // 去掉扩展名 
				// 检查是否已存在相同文件名 
				int existingRow = -1;
				bool containsExit = true;
				for (int i = 0; i < table->rowCount(); i++) {
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DOM")) {
						containsExit = false;
						break;
					}
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
						existingRow = i;
						break;
					}
				}
				if (containsExit) {
					DOMFilePath.push_back(domPath);
					int newRowIndex = table->rowCount();
					table->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem 
					for (int col = 0; col < table->columnCount(); col++) {
						if (!table->item(newRowIndex, col)) {
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							table->setItem(newRowIndex, col, item);
						}
					}

					int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);
					// 使用 setData 确保数值排序 
					table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
					table->item(newRowIndex, 1)->setText(fileName);                   // 文件名 
					table->item(newRowIndex, 2)->setText("DOM");                      // DOM 
					table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
				}
			}
		}
		//processImageWithOperator(TmpDOMFilePath);
		//on_actionOpenImage(TmpDOMFilePath, u8"DOM");
		auto fileLists = processImageWithOperator_test(DOMFilePath);
		QStringList existingFiles = fileLists.first;
		QStringList newFiles = fileLists.second;

		on_actionOpenImage(existingFiles, newFiles, u8"DOM");

		//QStringList tmpPath = ui.ImageInfoShow_Widget->getExcludedReferences();
		//ui.ImageInfoShow_Widget->m_excludedReferences.clear();

		//QSet<QString> domSet = QSet<QString>::fromList(filePaths);
		//QSet<QString> tmpSet = QSet<QString>::fromList(tmpPath);

		//// 从tmpPath中减去domPaths中已有的路径 
		//domSet.subtract(tmpSet);
		//PROJECT_LOG_INFO(this->CurrentConfig, (QString::fromLocal8Bit("TmpDOMFilePath: tmpSet : domSet") + QString::number(TmpDOMFilePath.size()) + " : " + QString::number(tmpSet.size()) + " : " + QString::number(domSet.size())));

		//for (auto filePath : domSet.toList())
		//{
		//	QFileInfo fileInfo(filePath);
		//	QString fileName = fileInfo.completeBaseName();

		//	bool containsExit = true;
		//	int existingRow = -1;
		//	//existingRow.push_back(-1);
		//	for (int i = 0; i < table->rowCount(); i++) {
		//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DOM")) {
		//			containsExit = false;
		//			break;
		//		}
		//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
		//			//existingRow[indexDom] = i;
		//			existingRow = i;
		//			break;
		//		}
		//	}
		//	if (containsExit) {
		//		DOMFilePath.push_back(filePath);
		//		int newRowIndex = table->rowCount();
		//		table->insertRow(newRowIndex);

		//		// 确保所有单元格都有 QTableWidgetItem 
		//		for (int col = 0; col < table->columnCount(); col++) {
		//			if (!table->item(newRowIndex, col)) {
		//				QTableWidgetItem* item = new QTableWidgetItem;
		//				item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		//				table->setItem(newRowIndex, col, item);
		//			}
		//		}

		//		int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);
		//		// 使用 setData 确保数值排序 
		//		table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
		//		table->item(newRowIndex, 1)->setText(fileName);                   // 文件名 
		//		table->item(newRowIndex, 2)->setText("DOM");                      // DOM 
		//		table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
		//	}
		//}

		table->resizeColumnsToContents();
		// 最后按序号升序排序 
		table->sortItems(0, Qt::AscendingOrder);
		//on_actionOpenImage(DOMFilePath, u8"DOM");
	});
	// 加载参考 DEM   
	connect(actionMap[QString::fromLocal8Bit("加载参考DEM")], &QAction::triggered, this, [=] {
		QStringList demPaths = QFileDialog::getOpenFileNames(
			this,
			QString::fromLocal8Bit("选择加载参考DEM"),
			m_lastPath,
			tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)"))
		);
		QTableWidget* table = ui.refereDataList_TableW;

		if (!demPaths.isEmpty()) {
			m_lastPath = QFileInfo(demPaths.first()).path();  // 更新最后路径 
			//TmpDEMFilePath.clear();
			for (const QString &demPath : demPaths) {
				QFileInfo fileInfo(demPath);
				QString fileName = fileInfo.completeBaseName();  // 去掉扩展名 

				// 检查是否已存在相同文件名 
				int existingRow = -1;
				bool containsExit = true;
				for (int i = 0; i < table->rowCount(); i++) {
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DEM")) {
						containsExit = false;
						continue;
					}
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
						existingRow = i;
						break;
					}
				}
				if (containsExit) {
					DEMFilePath.push_back(demPath);
					int newRowIndex = table->rowCount();
					table->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem 
					for (int col = 0; col < table->columnCount(); col++) {
						if (!table->item(newRowIndex, col)) {
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							table->setItem(newRowIndex, col, item);
						}
					}

					int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);

					table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
					table->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());                   // 文件名 
					table->item(newRowIndex, 2)->setText("DEM");                      // DOM 
					table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
				}
			}

			//on_actionOpenImage(TmpDEMFilePath, u8"DEM");
			auto fileLists = processImageWithOperator_test(DEMFilePath);
			QStringList existingFiles = fileLists.first;
			QStringList newFiles = fileLists.second;

			on_actionOpenImage(existingFiles, newFiles, u8"DEM");

			//QStringList tmpPath = ui.ImageInfoShow_Widget->getExcludedReferences();
			//ui.ImageInfoShow_Widget->m_excludedReferences.clear();

			//QSet<QString> demSet = QSet<QString>::fromList(demPaths);
			//QSet<QString> tmpSet = QSet<QString>::fromList(tmpPath);

			//// 从tmpPath中减去domPaths中已有的路径 
			//demSet.subtract(tmpSet);
			//PROJECT_LOG_INFO(this->CurrentConfig, (QString::fromLocal8Bit("TmpDEMFilePath: tmpSet : demSet") + QString::number(TmpDEMFilePath.size()) + " : " + QString::number(tmpSet.size()) + " : " + QString::number(demSet.size())));

			//for (auto filePath : demSet.toList())
			//{
			//	QFileInfo fileInfo(filePath);
			//	QString fileName = fileInfo.completeBaseName();

			//	bool containsExit = true;
			//	int existingRow = -1;
			//	//existingRow.push_back(-1);
			//	for (int i = 0; i < table->rowCount(); i++) {
			//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DEM")) {
			//			containsExit = false;
			//			break;
			//		}
			//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
			//			//existingRow[indexDom] = i;
			//			existingRow = i;
			//			break;
			//		}
			//	}
			//	if (containsExit) {
			//		DEMFilePath.push_back(filePath);
			//		int newRowIndex = table->rowCount();
			//		table->insertRow(newRowIndex);

			//		// 确保所有单元格都有 QTableWidgetItem 
			//		for (int col = 0; col < table->columnCount(); col++) {
			//			if (!table->item(newRowIndex, col)) {
			//				QTableWidgetItem* item = new QTableWidgetItem;
			//				item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			//				table->setItem(newRowIndex, col, item);
			//			}
			//		}

			//		int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);

			//		table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
			//		table->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());                   // 文件名 
			//		table->item(newRowIndex, 2)->setText("DEM");                      // DOM 
			//		table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
			//	}
			//}

		}
		// 添加后按第一列(文件名)升序排序 
		table->sortItems(0, Qt::AscendingOrder);
		table->resizeColumnsToContents();
		//on_actionOpenImage(DEMFilePath, u8"DEM");
	});

	connect(actionMap[QString::fromLocal8Bit("加载参考DEM(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载参考DEM(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QTableWidget* table = ui.refereDataList_TableW;
		//TmpDEMFilePath.clear();
		for (const QString &demPath : filePaths) {
			if (!demPath.isEmpty()) {
				QFileInfo fileInfo(demPath);
				QString fileName = fileInfo.completeBaseName();  // 去掉扩展名\
 
				// 检查是否已存在相同文件名 
				int existingRow = -1;
				bool containsexit = true;

				for (int i = 0; i < table->rowCount(); i++) {
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DEM")) {
						containsexit = false;
						break;
					}
					if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
						existingRow = i;
						break;
					}
				}

				if (containsexit)
				{
					DEMFilePath.push_back(demPath);
					int newRowIndex = table->rowCount();
					table->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem 
					for (int col = 0; col < table->columnCount(); col++) {
						if (!table->item(newRowIndex, col)) {
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							table->setItem(newRowIndex, col, item);
						}
					}

					int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);
					// 使用 setData 确保数值排序 
					table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
					table->item(newRowIndex, 1)->setText(fileName);                   // 文件名 
					table->item(newRowIndex, 2)->setText("DEM");                      // DOM 
					table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
				}
			}
		}
		//processImageWithOperator(TmpDEMFilePath);
		//on_actionOpenImage(TmpDEMFilePath, u8"DEM");
		auto fileLists = processImageWithOperator_test(DEMFilePath);
		QStringList existingFiles = fileLists.first;
		QStringList newFiles = fileLists.second;

		on_actionOpenImage(existingFiles, newFiles, u8"DEM");

		//QStringList tmpPath = ui.ImageInfoShow_Widget->getExcludedReferences();
		//ui.ImageInfoShow_Widget->m_excludedReferences.clear();

		//QSet<QString> demSet = QSet<QString>::fromList(filePaths);
		//QSet<QString> tmpSet = QSet<QString>::fromList(tmpPath);

		//// 从tmpPath中减去domPaths中已有的路径 
		//demSet.subtract(tmpSet);
		//PROJECT_LOG_INFO(this->CurrentConfig, (QString::fromLocal8Bit("TmpDEMFilePath: tmpSet : demSet") + QString::number(TmpDEMFilePath.size()) + " : " + QString::number(tmpSet.size()) + " : " + QString::number(demSet.size())));

		//for (auto filePath : demSet.toList())
		//{
		//	QFileInfo fileInfo(filePath);
		//	QString fileName = fileInfo.completeBaseName();

		//	bool containsExit = true;
		//	int existingRow = -1;
		//	//existingRow.push_back(-1);
		//	for (int i = 0; i < table->rowCount(); i++) {
		//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName && table->item(i, 2) && table->item(i, 2)->text() == QString::fromLocal8Bit("DEM")) {
		//			containsExit = false;
		//			break;
		//		}
		//		if (table->item(i, 1) && table->item(i, 1)->text() == fileName) {
		//			//existingRow[indexDom] = i;
		//			existingRow = i;
		//			break;
		//		}
		//	}
		//	if (containsExit) {
		//		DEMFilePath.push_back(filePath);
		//		int newRowIndex = table->rowCount();
		//		table->insertRow(newRowIndex);

		//		// 确保所有单元格都有 QTableWidgetItem 
		//		for (int col = 0; col < table->columnCount(); col++) {
		//			if (!table->item(newRowIndex, col)) {
		//				QTableWidgetItem* item = new QTableWidgetItem;
		//				item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		//				table->setItem(newRowIndex, col, item);
		//			}
		//		}

		//		int rowNumber = (existingRow != -1) ? table->item(existingRow, 0)->text().toInt() : (newRowIndex + 1);
		//		// 使用 setData 确保数值排序 
		//		table->item(newRowIndex, 0)->setData(Qt::DisplayRole, rowNumber); // 序号 
		//		table->item(newRowIndex, 1)->setText(fileName);                   // 文件名 
		//		table->item(newRowIndex, 2)->setText("DEM");                      // DOM 
		//		table->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("是")); // 是否参考 
		//	}
		//}
		// 添加后按第一列(文件名)升序排序 
		table->sortItems(0, Qt::AscendingOrder);
		table->resizeColumnsToContents();
		//on_actionOpenImage(DEMFilePath, u8"DEM");
	});

	connect(actionMap[QString::fromLocal8Bit("卸载参考数据")], &QAction::triggered, this, [=]
	{
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.refereDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		QStringList firstColumnDOM;
		QStringList firstColumnDEM;
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.refereDataList_TableW->item(item->row(), 1);
			QTableWidgetItem* firstColumnType = ui.refereDataList_TableW->item(item->row(), 2);
			if (firstColumnItem) {
				if(firstColumnType->text() == QString::fromLocal8Bit("DOM"))
					firstColumnDOM.append(firstColumnItem->text());  
				else if(firstColumnType->text() == QString::fromLocal8Bit("DEM"))
					firstColumnDEM.append(firstColumnItem->text());
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.refereDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnDOM)
		{
			for (auto filenamePATH : DOMFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					DOMFilePath.removeAll(filenamePATH);
					ui.ImageInfoShow_Widget->removeGeoBoundary(filenamePATH);
					// 确定当前颜色
					QColor currentColortmp = Qt::green;
					ui.ImageInfoShow_Widget->removeFromRetainReferences(fileInfo.completeBaseName(), currentColortmp);
				}
			}
		}
		for (auto filename : firstColumnDEM)
		{
			for (auto filenamePATH : DEMFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					DEMFilePath.removeAll(filenamePATH);
					ui.ImageInfoShow_Widget->removeGeoBoundary(filenamePATH);
					// 确定当前颜色
					QColor currentColortmp = Qt::blue;
					ui.ImageInfoShow_Widget->removeFromRetainReferences(fileInfo.completeBaseName(), currentColortmp);
				}
			}
		}

	});

	connect(actionMap[QString::fromLocal8Bit("查看参考数据")], &QAction::triggered, this, [=]
	{
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.refereDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		QStringList firstColumnDOM;
		QStringList firstColumnDEM;
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.refereDataList_TableW->item(item->row(), 1);
			QTableWidgetItem* firstColumnType = ui.refereDataList_TableW->item(item->row(), 2);
			if (firstColumnItem) {
				if (firstColumnType->text() == QString::fromLocal8Bit("DOM"))
					firstColumnDOM.append(firstColumnItem->text());
				else if (firstColumnType->text() == QString::fromLocal8Bit("DEM"))
					firstColumnDEM.append(firstColumnItem->text());
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnDOM)
		{
			for (auto filenamePATH : DOMFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
		for (auto filename : firstColumnDEM)
		{
			for (auto filenamePATH : DEMFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}

	});

	menu->exec(cursor().pos());
	QList<QAction*> list = menu->actions();
	foreach(QAction* pAction, list) delete pAction;
	delete menu;
}

/**
 * @brief 卫星影像数据右键菜单
 */
void IMAGEPS::sateImageDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/卫星影像数据列表.csv"), ",");
	//for (auto data : config)
	//{
	//	if (data[1] == "TRUE")
	//		actionMap[data[0]] = menu->addAction(data[0]);
	//}

	// 查找"高级"菜单项 
	QAction* advancedAction = nullptr;
	QMenu* advancedSubMenu = nullptr;

	for (auto data : config)
	{
		if (data[1] == "TRUE")
		{
			if (data[0] == QString::fromLocal8Bit("高级"))
			{
				// 创建高级子菜单 
				advancedSubMenu = new QMenu(QString::fromLocal8Bit("高级"));
				advancedAction = menu->addMenu(advancedSubMenu);

				// 加载高级子菜单的配置 
				QList<QStringList> advancedConfig = PublicFunctions::loadFile(
					QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/卫星影像数据列表_高级.csv"), ",");

				for (auto advData : advancedConfig)
				{
					if (advData[1] == "TRUE")
					{
						QAction* subAction = advancedSubMenu->addAction(advData[0]);
						actionMap[advData[0]] = subAction;
					}
				}
			}
			else
			{
				// 添加普通菜单项 
				//actionMap[data[0]] = menu->addAction(data[0]);
				QAction* action = menu->addAction(data[0]);
				actionMap[data[0]] = action;
				if (data[0] == QString::fromLocal8Bit("卸载影像")) {
					action->setEnabled(!m_isProcessingImageOpen);  // 根据处理状态启用/禁用 
				}
			}
		}
	}

	// 获取当前选中的行数
	QList<QTableWidgetItem*> selectedItems = ui.sateImageDataList_TableW->selectedItems();
	QSet<int> selectedRows;
	for (QTableWidgetItem* item : selectedItems) {
		selectedRows.insert(item->row());
	}
	int selectedRowCount = selectedRows.size();

	// 如果选中的行只有0行
	if (selectedRowCount == 0) {
		auto it = actionMap.find(QString::fromLocal8Bit("卸载影像"));
		if (it != actionMap.end()) {
			it.value()->setEnabled(false);
		}

		it = actionMap.find(QString::fromLocal8Bit("查看影像"));
		if (it != actionMap.end()) {
			it.value()->setEnabled(false);
		}

		it = actionMap.find(QString::fromLocal8Bit("设置纠正成果属性信息"));
		if (it != actionMap.end()) {
			it.value()->setEnabled(false);
		}

		it = actionMap.find(QString::fromLocal8Bit("设为基准影像"));
		if (it != actionMap.end()) {
			it.value()->setEnabled(false);
		}

		it = actionMap.find(QString::fromLocal8Bit("设为非基准影像"));
		if (it != actionMap.end()) {
			it.value()->setEnabled(false);
		}
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		// 支持多选文件的文件对话框 
		QStringList originImagePaths = QFileDialog::getOpenFileNames(
			this,
			QString::fromLocal8Bit("选择加载原始影像文件"),
			m_lastPath,  // 使用上次路径 
			tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)")
			.arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til *.pix)"))
		);

		if (originImagePaths.isEmpty()) {
			return;
		}

		// 更新最后选择的路径 
		QFileInfo firstFileInfo(originImagePaths.first());
		m_lastPath = firstFileInfo.absolutePath();

		QStringList finalImagePaths = filterImageFiles(originImagePaths, "卫星影像数据");

		QStringList newDataModelPaths;

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			// 检查是否已存在相同文件名的记录 
			bool alreadyExists = false;
			for (int i = 0; i < ui.sateImageDataList_TableW->rowCount(); i++) {
				if (fileInfo.fileName().contains(ui.sateImageDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists) {
				hasNewFiles = true;
				//DataModelPath.push_back(originImagePath);
				newDataModelPaths.append(originImagePath);

				// 插入新行 
				int newRowIndex = ui.sateImageDataList_TableW->rowCount();
				ui.sateImageDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.sateImageDataList_TableW->columnCount(); col++) {
					if (!ui.sateImageDataList_TableW->item(newRowIndex, col)) {
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.sateImageDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				// 设置序号（当前行号 + 1）
				ui.sateImageDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.sateImageDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				//QString tmptype = fileInfo.completeBaseName().left(3);
				//if (tmptype == "DP0")
				//	tmptype = "JL";
				QString tmptype = getImagetype(originImagePath);

				ui.sateImageDataList_TableW->item(newRowIndex, 2)->setText(tmptype);   // "GF1"

				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.sateImageDataList_TableW->item(newRowIndex, 3)->setText(panPart.left(3));
				ui.sateImageDataList_TableW->item(newRowIndex, 4)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 5)->setText(QString::fromLocal8Bit("否"));
				ui.sateImageDataList_TableW->item(newRowIndex, 6)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 8)->setText(QString::fromLocal8Bit("否"));
				ui.sateImageDataList_TableW->item(newRowIndex, 9)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 10)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 11)->setText(QString::fromLocal8Bit("是"));

				parentDir = fileInfo.absolutePath() + "/";
			}
		}
		ui.sateImageDataList_TableW->resizeColumnsToContents();
		ui.sateImageDataList_num->setText(QString::fromLocal8Bit("卫星影像列表数量: %1").arg(ui.sateImageDataList_TableW->rowCount()));
		if (hasNewFiles) {
			//setGeoreferenceFromXMLAndRPC(DataModelPath);
			//on_actionOpenImage(DataModelPath, u8"原始影像");

			// 将新增路径合并到主数据模型 
			DataModelPath.append(newDataModelPaths);

			auto fileLists = processImageWithOperator_test(newDataModelPaths);
			QStringList existingFiles = fileLists.first;
			QStringList newFiles = fileLists.second;

			//setGeoreferenceFromXMLAndRPC(existingFiles, newFiles);
			on_actionOpenImage(existingFiles, newFiles, u8"原始影像");

		}

		hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.SmartMosaicDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.SmartMosaicDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				QString panPart = systemConfig->getTiffImageType(originImagePath);
				if (panPart == "MSS") {
					hasNewFiles = true;
					SmartMosaicFilePath.push_back(this->projectdir + QString::fromLocal8Bit("Dodging/") + fileInfo.fileName());
					// 插入新行
					int newRowIndex = ui.SmartMosaicDataList_TableW->rowCount();
					ui.SmartMosaicDataList_TableW->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem
					for (int col = 0; col < ui.SmartMosaicDataList_TableW->columnCount(); col++)
					{
						if (!ui.SmartMosaicDataList_TableW->item(newRowIndex, col))
						{
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							ui.SmartMosaicDataList_TableW->setItem(newRowIndex, col, item);
						}
					}
					// 设置序号（当前行号 + 1）
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

					// 设置文件名（去掉扩展名）
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


					//QString panPart = systemConfig->getTiffImageType(originImagePath);

					ui.SmartMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);

					QString tmptype = getImagetype(originImagePath);
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 3)->setText(tmptype);

					QString time = extractImageTime(originImagePath, tmptype);
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 4)->setText(time);
				}
			}
		}
		ui.SmartMosaicDataList_TableW->resizeColumnsToContents();
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=] {

		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);

		if (folderPath.isEmpty()) {
			return;
		}


		m_lastPath = folderPath;


		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";


		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories
		);

		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}

		QStringList finalImagePaths = filterImageFiles(filePaths, "卫星影像数据");
		bool hasNewFiles = false;

		// 使用局部变量暂存新增路径，避免并行时数据竞争
		QStringList newDataModelPaths;

		for (const QString &originImagePath : finalImagePaths) {
			bool alreadyExists = false;
			QFileInfo fileInfo(originImagePath);

			for (int i = 0; i < ui.sateImageDataList_TableW->rowCount(); i++) {
				if (fileInfo.fileName().contains(ui.sateImageDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists) {
				hasNewFiles = true;
				newDataModelPaths.append(originImagePath); // 暂存新增路径


				int newRowIndex = ui.sateImageDataList_TableW->rowCount();
				ui.sateImageDataList_TableW->insertRow(newRowIndex);

				// 创建并设置表格项
				for (int col = 0; col < ui.sateImageDataList_TableW->columnCount(); col++) {
					if (!ui.sateImageDataList_TableW->item(newRowIndex, col)) {
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.sateImageDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				// 设置表格内容 
				ui.sateImageDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));


				ui.sateImageDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());




				QString tmptype = getImagetype(originImagePath);
				ui.sateImageDataList_TableW->item(newRowIndex, 2)->setText(tmptype);

				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.sateImageDataList_TableW->item(newRowIndex, 3)->setText(panPart.left(3));

				// 设置其他列数据 
				ui.sateImageDataList_TableW->item(newRowIndex, 4)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 5)->setText(QString::fromLocal8Bit("否"));
				ui.sateImageDataList_TableW->item(newRowIndex, 6)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 8)->setText(QString::fromLocal8Bit("否"));
				ui.sateImageDataList_TableW->item(newRowIndex, 9)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 10)->setText(QString::fromLocal8Bit("是"));
				ui.sateImageDataList_TableW->item(newRowIndex, 11)->setText(QString::fromLocal8Bit("是"));

				parentDir = fileInfo.absolutePath() + "/";
			}
		}

		// 更新UI显示 
		ui.sateImageDataList_TableW->resizeColumnsToContents();
		ui.sateImageDataList_num->setText(QString::fromLocal8Bit("卫星影像列表数量: %1").arg(ui.sateImageDataList_TableW->rowCount()));

		// 确保UI更新完成 
		QCoreApplication::processEvents();
		//processImageWithOperator(newDataModelPaths);


		if (hasNewFiles) {
			// 将新增路径合并到主数据模型 
			DataModelPath.append(newDataModelPaths);

			auto fileLists = processImageWithOperator_test(newDataModelPaths);
			QStringList existingFiles = fileLists.first;
			QStringList newFiles = fileLists.second;

			//setGeoreferenceFromXMLAndRPC(existingFiles, newFiles);
			on_actionOpenImage(existingFiles, newFiles, u8"原始影像");

		}

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.SmartMosaicDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.SmartMosaicDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				QString panPart = systemConfig->getTiffImageType(originImagePath);
				if (panPart == "MSS") {
					SmartMosaicFilePath.push_back(this->projectdir + QString::fromLocal8Bit("Dodging/") + fileInfo.fileName());
					// 插入新行
					int newRowIndex = ui.SmartMosaicDataList_TableW->rowCount();
					ui.SmartMosaicDataList_TableW->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem
					for (int col = 0; col < ui.SmartMosaicDataList_TableW->columnCount(); col++)
					{
						if (!ui.SmartMosaicDataList_TableW->item(newRowIndex, col))
						{
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							ui.SmartMosaicDataList_TableW->setItem(newRowIndex, col, item);
						}
					}
					// 设置序号（当前行号 + 1）
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

					// 设置文件名（去掉扩展名）
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

					//QString panPart = systemConfig->getTiffImageType(originImagePath);

					ui.SmartMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);

					QString tmptype = getImagetype(originImagePath);
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 3)->setText(tmptype);

					QString time = extractImageTime(originImagePath, tmptype);
					ui.SmartMosaicDataList_TableW->item(newRowIndex, 4)->setText(time);
				}
			}
		}
		ui.SmartMosaicDataList_TableW->resizeColumnsToContents();

	});


	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> selectedItems = ui.sateImageDataList_TableW->selectedItems();
		if (selectedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : selectedItems) {
			rowsToDelete.insert(item->row());
			
			QTableWidgetItem* firstColumnItem = ui.sateImageDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> sortedRows = rowsToDelete.values();
		std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : sortedRows) {
			ui.sateImageDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (int i = 0; i < ui.SmartMosaicDataList_TableW->rowCount(); ) {
				QTableWidgetItem* item = ui.SmartMosaicDataList_TableW->item(i, 1); 
				if (item && item->text() == filename) {
					ui.SmartMosaicDataList_TableW->removeRow(i);
					// 不递增i，因为删除行后后面的行会前移 
				}
				else {
					i++;
				}
			}

			for (auto filenamePATH : DataModelPath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					ui.ImageInfoShow_Widget->removeGeoBoundary(filenamePATH);
					ui.ImageInfoShow_Widget->removePointsByFilePath(filenamePATH,"con");
					ui.ImageInfoShow_Widget->removePointsByFilePath(filenamePATH,"col");
					ui.encryptPointsnum->setText(QString::fromLocal8Bit("加密点列表数量: %1").arg(encryptPointsModel->rowCount()));
					ui.controlPointsnum->setText(QString::fromLocal8Bit("控制点列表数量: %1").arg(ControlPointsModel->rowCount()));
					DataModelPath.removeAll(filenamePATH);

					dataModelConfig->DataModelfilenames.remove(filenamePATH);
					// 查找所有匹配的项
					QList<QTableWidgetItem*> itemsNad = dataModelConfig->ui.tableWidget->findItems(fileInfo.completeBaseName(), Qt::MatchExactly);

					// 遍历找到的项并删除对应行
					foreach(QTableWidgetItem* item, itemsNad) {
						if (item->column() == 1) {  // 确保是在指定列中找到的
							dataModelConfig->ui.tableWidget->removeRow(item->row());
						}
					}
				}
			}

			for (auto filenamePATH : SmartMosaicFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					SmartMosaicFilePath.removeAll(filenamePATH);
				}
			}
		}
		ui.sateImageDataList_num->setText(QString::fromLocal8Bit("卫星影像列表数量: %1").arg(ui.sateImageDataList_TableW->rowCount()));
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> selectedItems = ui.sateImageDataList_TableW->selectedItems();
		if (selectedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : selectedItems) {
			rowsToDelete.insert(item->row());

			QTableWidgetItem* firstColumnItem = ui.sateImageDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> sortedRows = rowsToDelete.values();
		std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : DataModelPath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
		
	});

	connect(actionMap[QString::fromLocal8Bit("设置纠正成果属性信息")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		QStringList filePath;

		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> selectedItems = ui.sateImageDataList_TableW->selectedItems();
		if (selectedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单 
		}

		// 提取所有选中的行号（避免重复）
		QSet<int> selectedRows;
		for (QTableWidgetItem* item : selectedItems) {
			selectedRows.insert(item->row());
		}

		// 获取选中行的第一列的值 
		for (int row : selectedRows) {
			QTableWidgetItem* firstColumnItem = ui.sateImageDataList_TableW->item(row, 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());    // 存储第一列的值 
			}
		}

		// 按从大到小的顺序 
		QList<int> sortedRows = selectedRows.values();
		std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

		for (auto filename : firstColumnValues) {
			for (auto filenamePATH : DataModelPath) {
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename) {
					filePath.append(filenamePATH);
					break;
				}
			}
		}

		if (filePath.isEmpty())   return;

		// 设置当前文件列表（会自动从统一配置文件中加载设置）
		SetOrthoInfoWidget->m_configFile = projectdir + QString::fromLocal8Bit("ortho_settings.ini");
		SetOrthoInfoWidget->setCurrentFileList(filePath);

		// 显示对话框 
		if (SetOrthoInfoWidget->exec() == QDialog::Accepted) {
			SetOrthoInfoWidget->saveAllSettingsToFile(SetOrthoInfoWidget->m_configFile);

			// 获取配置信息并更新表格 
			for (int row : sortedRows) {
				QTableWidgetItem* filenameItem = ui.sateImageDataList_TableW->item(row, 1);
				if (!filenameItem) continue;

				QMap<QString, QString> configInfo = SetOrthoInfoWidget->getFileConfigInfo(filenameItem->text());
				if (!configInfo.empty() && configInfo.contains("GSDX")) {
					// 确保第7列存在 
					if (ui.sateImageDataList_TableW->columnCount() <= 7) {
						ui.sateImageDataList_TableW->setColumnCount(8);
					}

					// 获取或创建第7列的item 
					QTableWidgetItem* gsdxItem = ui.sateImageDataList_TableW->item(row, 7);
					if (!gsdxItem) {
						gsdxItem = new QTableWidgetItem();
						ui.sateImageDataList_TableW->setItem(row, 7, gsdxItem);
					}

					// 设置GSDX值 
					gsdxItem->setText(configInfo["GSDX"]);
				}
			}

		}
	});

	// 设为基准影像
	connect(actionMap[QString::fromLocal8Bit("设为基准影像")], &QAction::triggered, this, [=] {
		setSelectedRowsValue(5, QString::fromLocal8Bit("是")); // 第4列是平差列 
	});

	// 设为非基准影像
	connect(actionMap[QString::fromLocal8Bit("设为非基准影像")], &QAction::triggered, this, [=] {
		setSelectedRowsValue(5, QString::fromLocal8Bit("否"));
	});

	// 添加高级子菜单项的连接 
	if (advancedSubMenu)
	{
		// 参与平差 
		connect(actionMap[QString::fromLocal8Bit("参与平差")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(4, QString::fromLocal8Bit("是")); // 第4列是平差列 
		});

		// 不参与平差
		connect(actionMap[QString::fromLocal8Bit("不参与平差")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(4, QString::fromLocal8Bit("否"));
		});

		// 参与纠正 
		connect(actionMap[QString::fromLocal8Bit("参与纠正")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(9, QString::fromLocal8Bit("是")); // 第5列是纠正列
		});

		// 不参与纠正
		connect(actionMap[QString::fromLocal8Bit("不参与纠正")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(9, QString::fromLocal8Bit("否"));
		});

		// 参与融合 
		connect(actionMap[QString::fromLocal8Bit("参与融合")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(10, QString::fromLocal8Bit("是")); // 第6列是融合列
		});

		// 不参与融合
		connect(actionMap[QString::fromLocal8Bit("不参与融合")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(10, QString::fromLocal8Bit("否"));
		});

		// 参与控制点匹配 
		connect(actionMap[QString::fromLocal8Bit("参与控制点匹配")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(11, QString::fromLocal8Bit("是")); // 第8列是控制点匹配列
		});

		// 不参与控制点匹配
		connect(actionMap[QString::fromLocal8Bit("不参与控制点匹配")], &QAction::triggered, this, [=] {
			setSelectedRowsValue(11, QString::fromLocal8Bit("否"));
		});
	}


	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 金字塔创建右键菜单
 */
void IMAGEPS::PyramidDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/金字塔创建数据.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.PyramidDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.PyramidDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				CreatepyramidPath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.PyramidDataList_TableW->rowCount();
				ui.PyramidDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.PyramidDataList_TableW->columnCount(); col++)
				{
					if (!ui.PyramidDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.PyramidDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.PyramidDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.PyramidDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				/*ui.PyramidDataList_TableW->item(ui.refereDataList_TableW->rowCount()  - 1, 2)->setText("DEM");
				ui.P极光ramidDataList_TableW->item(ui.refereDataList_TableW->rowCount()  - 1, 3)->setText(QString::fromLocal8Bit("是"));*/
			}
		}
		ui.PyramidDataList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(CreatepyramidPath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);
		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.PyramidDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.PyramidDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				CreatepyramidPath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.PyramidDataList_TableW->rowCount();
				ui.PyramidDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.PyramidDataList_TableW->columnCount(); col++)
				{
					if (!ui.PyramidDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.PyramidDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.PyramidDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.PyramidDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
			}
		}
		ui.PyramidDataList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(CreatepyramidPath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.PyramidDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.PyramidDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.PyramidDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : CreatepyramidPath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					CreatepyramidPath.removeAll(filenamePATH);
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 真彩色转换右键菜单
 */
void IMAGEPS::ImageInterList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/真彩色转换.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.imageInterList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.imageInterList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				ImageInterFilePath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.imageInterList_TableW->rowCount();
				ui.imageInterList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.imageInterList_TableW->columnCount(); col++)
				{
					if (!ui.imageInterList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.imageInterList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.imageInterList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.imageInterList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.imageInterList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.imageInterList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(ImageInterFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);
		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.imageInterList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.imageInterList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				ImageInterFilePath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.imageInterList_TableW->rowCount();
				ui.imageInterList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.imageInterList_TableW->columnCount(); col++)
				{
					if (!ui.imageInterList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.imageInterList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.imageInterList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.imageInterList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.imageInterList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.imageInterList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(ImageInterFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.imageInterList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.imageInterList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.imageInterList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : ImageInterFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					ImageInterFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.imageInterList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.imageInterList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : ImageInterFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 影像匀色右键菜单
 */
void IMAGEPS::DodgingList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/影像匀色.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.DodgingDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.DodgingDataList_TableW->item(i, 1)->text())){
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				DodgingFilePath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.DodgingDataList_TableW->rowCount();
				ui.DodgingDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.DodgingDataList_TableW->columnCount(); col++)
				{
					if (!ui.DodgingDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.DodgingDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.DodgingDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.DodgingDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.DodgingDataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.DodgingDataList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(DodgingFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.DodgingDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.DodgingDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				DodgingFilePath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.DodgingDataList_TableW->rowCount();
				ui.DodgingDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.DodgingDataList_TableW->columnCount(); col++)
				{
					if (!ui.DodgingDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.DodgingDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.DodgingDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.DodgingDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.DodgingDataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.DodgingDataList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(DodgingFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.DodgingDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.DodgingDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.DodgingDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : DodgingFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					DodgingFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.DodgingDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.DodgingDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : DodgingFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 影像镶嵌右键菜单
 */
void IMAGEPS::ImageMosaicList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/影像镶嵌.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIF Image(*.t极光if);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.ImageMosaicDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.ImageMosaicDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				ImageMosaicFilePath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.ImageMosaicDataList_TableW->rowCount();
				ui.ImageMosaicDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.ImageMosaicDataList_TableW->columnCount(); col++)
				{
					if (!ui.ImageMosaicDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.ImageMosaicDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.ImageMosaicDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.ImageMosaicDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.ImageMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.ImageMosaicDataList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(ImageMosaicFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.ImageMosaicDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.ImageMosaicDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				ImageMosaicFilePath.push_back(originImagePath);
				// 插入极光新行 
				int newRowIndex = ui.ImageMosaicDataList_TableW->rowCount();
				ui.ImageMosaicDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.ImageMosaicDataList_TableW->columnCount(); col++)
				{
					if (!ui.ImageMosaicDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.ImageMosaicDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.ImageMosaicDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.ImageMosaicDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.ImageMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.ImageMosaicDataList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(ImageMosaicFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.ImageMosaicDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.ImageMosaicDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.ImageMosaicDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : ImageMosaicFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					ImageMosaicFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.ImageMosaicDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.ImageMosaicDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : ImageMosaicFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 快速镶嵌右键
 */
void IMAGEPS::QuickMosaicDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/快速镶嵌数据列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}


	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFiles(ui.QuickMosaicDataList_TableW, QuickMosaicFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFolder(ui.QuickMosaicDataList_TableW, QuickMosaicFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=] {
		unloadImages(ui.QuickMosaicDataList_TableW, QuickMosaicFilePath);
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 配准纠正右键菜单
 */
void IMAGEPS::AlignmentAdjustmentList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/配准纠正.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList  originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.AlignmentAdjustmentList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.AlignmentAdjustmentList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				AlignmentAdFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.AlignmentAdjustmentList_TableW->rowCount();
				ui.AlignmentAdjustmentList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.AlignmentAdjustmentList_TableW->columnCount(); col++)
				{
					if (!ui.AlignmentAdjustmentList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.AlignmentAdjustmentList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


				//QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 2)->setText(QString::fromLocal8Bit("否"));
				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("否"));
			}
		}
		ui.AlignmentAdjustmentList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(AlignmentAdFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择
		}
		m_lastPath = folderPath;
		// 设置文件过滤器
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.AlignmentAdjustmentList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.AlignmentAdjustmentList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				AlignmentAdFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.AlignmentAdjustmentList_TableW->rowCount();
				ui.AlignmentAdjustmentList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.AlignmentAdjustmentList_TableW->columnCount(); col++)
				{
					if (!ui.AlignmentAdjustmentList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.AlignmentAdjustmentList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				//QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 2)->setText(QString::fromLocal8Bit("否"));
				ui.AlignmentAdjustmentList_TableW->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("否"));
			}
		}
		ui.AlignmentAdjustmentList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(AlignmentAdFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AlignmentAdjustmentList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AlignmentAdjustmentList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.AlignmentAdjustmentList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AlignmentAdFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					AlignmentAdFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AlignmentAdjustmentList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AlignmentAdjustmentList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AlignmentAdFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 影像业务处理模块影像融合功能
 */
void IMAGEPS::FusionBuildmodel(QStringList filenamelist, const QString& tableName, const QString& modeldir)
{
	QString xmlPath = dataModelConfig->BuildSatModelMakerCmdXML(filenamelist, modeldir);

	QTableWidget* srcTable = nullptr;
	QTableWidget* targetTable = nullptr;
	if (tableName == QString::fromLocal8Bit("fusionmodelList_TableW"))
	{
		targetTable = ui.fusionmodelList_TableW;
		srcTable = ui.dataList_TableW;
	}
	else if (tableName == QString::fromLocal8Bit("fusionmodelList_TableW_2"))
	{
		targetTable = ui.fusionmodelList_TableW_2;
		srcTable = ui.AlignmentIntegrationList_TableW;
	}

	//创建调用可执行程序的类
	QProcess * process = new QProcess(this);
	QString authCmd;
#ifdef Q_OS_LINUX 
	authCmd = this->appDirPath + QString::fromLocal8Bit("/linux64/PSSatModelMakerCmd.x");
#else
	authCmd = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSSatModelMakerCmd.exe");
#endif
	PROJECT_LOG_INFO(this->CurrentConfig, QString::fromLocal8Bit("影像融合单模块配对功能开始执行...."));

	//启动程序，并输入参数
	process->start(authCmd, QStringList() << xmlPath);
	m_childProcesses.append(process);
	PublicFunctions::writeTimestampToXml("FUModelmarker", this->projectdir);

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		process->deleteLater(); // 安全释放内存
		m_childProcesses.removeOne(process);
		//QDateTime timestamp = PublicFunctions::readTimestampFromXml("FUModelmarker");
		//if (!timestamp.isValid()) {
		//	return;
		//}
		PROJECT_LOG_INFO(this->CurrentConfig, QString::fromLocal8Bit("影像融合单模块配对功能执行结束...."));

		//QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + modeldir + "/", timestamp);
		//QString tmpPath = projectdir + modeldir + "/" + QString::fromLocal8Bit("SatModelMakerCmdout.xml");
		QString SatModelMakerCmdoutPath = projectdir + modeldir + "/" + QString::fromLocal8Bit("SatModelMakerCmdout.xml");
		//QString SatModelMakerCmdoutPath = "";
		//for (auto datafile : resultFiles)
		//{
		//	if (tmpPath == datafile)
		//		SatModelMakerCmdoutPath = tmpPath;
		//}
		//if (SatModelMakerCmdoutPath == "")
		//{
		//	QMessageBox::warning(this, "Warning", QString::fromLocal8Bit("没有可用的匹配模型，请先构建匹配模型！"));
		//	return;
		//}

		QFile file(SatModelMakerCmdoutPath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QMessageBox::critical(nullptr, u8"错误", "打开SatModelMakerCmdout.xml文件失败  " + file.errorString());
			PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("融合前配对: 打开SatModelMakerCmdout.xml文件失败"));
			return;
		}

		QXmlStreamReader xml(&file);
		QRegularExpression modelRegex("^Model_\\d+$"); // 匹配 Model_0, Model_1, ..., Model_N

		while (!xml.atEnd() && !xml.hasError()) {
			QXmlStreamReader::TokenType token = xml.readNext();

			if (token == QXmlStreamReader::StartElement) {
				QString elementName = xml.name().toString();

				// 检查是否是 Model_X 节点
				if (modelRegex.match(elementName).hasMatch()) {
					QMap<QString, QString> modelData;

					// 解析当前 Model_X 的所有子节点
					while (!(xml.tokenType() == QXmlStreamReader::EndElement &&
						xml.name() == elementName)) {
						if (xml.tokenType() == QXmlStreamReader::StartElement) {
							QString childName = xml.name().toString();
							xml.readNext();
							if (xml.tokenType() == QXmlStreamReader::Characters) {
								modelData[childName] = xml.text().toString().trimmed();
							}
						}
						xml.readNext();
					}

					// 检查NadImageName和MulImageName是否为空 
					QFileInfo fileInfoNadImageName(modelData[QString::fromLocal8Bit("NadImageName")]);
					QFileInfo fileInfoMulImageName(modelData[QString::fromLocal8Bit("MulImageName")]);

					if (!fileInfoNadImageName.completeBaseName().isEmpty() &&
						!fileInfoMulImageName.completeBaseName().isEmpty())
					{
						// 如果任一文件名为空，则从srcTable 中删除对应的文件名
						if (!fileInfoNadImageName.completeBaseName().isEmpty()) {
							//removeFileFromDataListTable(fileInfoNadImageName.completeBaseName());
							for (int i = 0; i < srcTable->rowCount(); ++i) {
								QTableWidgetItem* item = srcTable->item(i, 1); // 假设文件名在第1列 
								if (item && item->text() == fileInfoNadImageName.completeBaseName()) {
									srcTable->removeRow(i);
									break;
								}
							}
						}
						if (!fileInfoMulImageName.completeBaseName().isEmpty()) {
							//removeFileFromDataListTable(fileInfoMulImageName.completeBaseName());
							for (int i = 0; i < srcTable->rowCount(); ++i) {
								QTableWidgetItem* item = srcTable->item(i, 1); // 假设文件名在第1列 
								if (item && item->text() == fileInfoMulImageName.completeBaseName()) {
									srcTable->removeRow(i);
									break;
								}
							}
						}
					}
					else{
						continue; // 跳过这行数据，不写入表中 
					}

					// 插入新行
					int newRowIndex = targetTable->rowCount();
					targetTable->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem
					for (int col = 0; col < targetTable->columnCount(); col++)
					{
						if (!targetTable->item(newRowIndex, col))
						{
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							targetTable->setItem(newRowIndex, col, item);
						}
					}
					// 设置序号（当前行号 + 1）
					if (targetTable->columnCount() > 0)
						targetTable->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

					// 设置模型号
					if (targetTable->columnCount() > 1 && modelData.contains(QString::fromLocal8Bit("ModelID")))
						targetTable->item(newRowIndex, 1)->setText(modelData[QString::fromLocal8Bit("ModelID")]);

					//QFileInfo fileInfoNadImageName(modelData[QString::fromLocal8Bit("NadImageName")]);
					targetTable->item(newRowIndex, 2)->setText(fileInfoNadImageName.completeBaseName());
					//QFileInfo fileInfoMulImageName(modelData[QString::fromLocal8Bit("MulImageName")]);
					targetTable->item(newRowIndex, 3)->setText(fileInfoMulImageName.completeBaseName());

					if (targetTable->columnCount() > 4)
						targetTable->item(newRowIndex, 4)->setText(QString::fromLocal8Bit("否"));
					if (targetTable->columnCount() > 5)
						targetTable->item(newRowIndex, 5)->setText(QString::fromLocal8Bit("否"));
					targetTable->resizeColumnsToContents();
				}
			}
		}

		if (xml.hasError()) {
			QMessageBox::critical(nullptr, "XML Error", xml.errorString());
		}

		file.close();

	});
}

void IMAGEPS::Fusiondeletemodel(QStringList FilePath, const QString& tableName)
{
	QTableWidget* targetTable = nullptr;
	QTableWidget* targetTable_src = nullptr;
	if (tableName == QString::fromLocal8Bit("fusionmodelList_TableW"))
	{
		targetTable_src = ui.dataList_TableW;
		targetTable = ui.fusionmodelList_TableW;
	}
	else if (tableName == QString::fromLocal8Bit("fusionmodelList_TableW_2"))
	{
		targetTable_src = ui.AlignmentIntegrationList_TableW;
		targetTable = ui.fusionmodelList_TableW_2;
	}

	QList<QTableWidgetItem*> selectedItems = targetTable->selectedItems();
	if (selectedItems.isEmpty()) {
		QMessageBox::warning(this, "Warning", "No rows selected!");
		return;
	}
	QSet<int> rowsToProcess;
	for (QTableWidgetItem* item : selectedItems) {
		rowsToProcess.insert(item->row());
	}
	QList<int> rows = rowsToProcess.values();
	std::sort(rows.begin(), rows.end(), std::greater<int>());
	for (int row : rows) {
		for (auto data : FilePath)
		{
			QString tmpnad = targetTable->item(row, 2)->text().trimmed();
			QString tmpmul = targetTable->item(row, 3)->text().trimmed();
			QFileInfo fileInfodata(data);
			if (fileInfodata.completeBaseName() == tmpnad || fileInfodata.completeBaseName() == tmpmul)
			{
				bool state = true;

				QFileInfo fileInfo(data);
				for (int i = 0; i < targetTable_src->rowCount(); i++)
				{
					if (fileInfo.fileName().contains(targetTable_src->item(i, 1)->text()))
					{
						state = false;
						break;
					}

				}
				if (!state)
				{
					state = true;
					continue;
				}
				// 插入新行
				int newRowIndex = targetTable_src->rowCount();
				targetTable_src->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < targetTable_src->columnCount(); col++)
				{
					if (!targetTable_src->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						targetTable_src->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				targetTable_src->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				targetTable_src->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString tmptype = systemConfig->getTiffImageType(data);
				targetTable_src->item(newRowIndex, 2)->setText(tmptype);
				targetTable_src->resizeColumnsToContents();
				//data.value() = true;
			}
			//data.value() = false;
		}
		targetTable->removeRow(row);
	}

	for (int i = 0; i < targetTable->rowCount(); ++i) {
		// 确保第一列存在 QTableWidgetItem
		QTableWidgetItem* item = targetTable->item(i, 0);
		if (!item) {
			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			targetTable->setItem(i, 0, item);
		}
		// 设置序号为当前行号 + 1
		item->setText(QString::number(i + 1));
	}
}

/**
 * @brief 影像融合右键菜单
 */
void IMAGEPS::FusionmodelsrcList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/影像融合_src.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	// 获取当前选中的行数
	QList<QTableWidgetItem*> selectedItems = ui.dataList_TableW->selectedItems();
	QSet<int> selectedRows;
	for (QTableWidgetItem* item : selectedItems) {
		selectedRows.insert(item->row());
	}
	int selectedRowCount = selectedRows.size();

	// 如果选中的行只有一行，将"模型配对"菜单项置为不可用
	if (selectedRowCount <= 1) {
		actionMap[QString::fromLocal8Bit("模型配对")]->setEnabled(false);
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.dataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.dataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				FusionmodelFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.dataList_TableW->rowCount();
				ui.dataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.dataList_TableW->columnCount(); col++)
				{
					if (!ui.dataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.dataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.dataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.dataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.dataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.dataList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(FusionmodelFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.dataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.dataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				FusionmodelFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.dataList_TableW->rowCount();
				ui.dataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.dataList_TableW->columnCount(); col++)
				{
					if (!ui.dataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.dataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.dataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.dataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.dataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.dataList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(FusionmodelFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.dataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.dataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.dataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : FusionmodelFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					FusionmodelFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("模型配对")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		QStringList Buildfilenames;
		// 获取选中的所有单元格
		QList<QTableWidgetItem*> referedItems = ui.dataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何单元格，不执行操作
		}

		// 收集选中的行号（使用 QSet 避免重复）
		QSet<int> rows;
		for (QTableWidgetItem* item : referedItems) {
			rows.insert(item->row());
		}
		if (rows.size() <= 1) {
			return; // 选中行数不足，不执行操作
		}

		// 基于唯一行号提取第一列的值 
		for (int row : rows) {
			QTableWidgetItem* firstColumnItem = ui.dataList_TableW->item(row, 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text()); // 每行只添加一次
			}
		}

		// 根据第一列值匹配文件路径
		for (auto filename : firstColumnValues) {
			for (auto filenamePATH : FusionmodelFilePath) {
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename) {
					Buildfilenames.push_back(filenamePATH);
					// FusionmodelFilePath.removeAll(filenamePATH); // 原注释代码，可根据需要调整 
				}
			}
		}
		FusionBuildmodel(Buildfilenames, QString::fromLocal8Bit("fusionmodelList_TableW"), "Fusion");
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.dataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; 
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.dataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : FusionmodelFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

void IMAGEPS::FusionmodelList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/影像融合_model.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("删除配对模型")], &QAction::triggered, this, [=]
	{
		Fusiondeletemodel(FusionmodelFilePath, QString::fromLocal8Bit("fusionmodelList_TableW"));
	});

	connect(actionMap[QString::fromLocal8Bit("查看模型")], &QAction::triggered, this, [=]
	{

	});

	connect(actionMap[QString::fromLocal8Bit("查看处理进度")], &QAction::triggered, this, [=]
	{

	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 配准融合右键菜单
 */
void IMAGEPS::AlignmentIntsrcList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/配准融合_src.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	// 获取当前选中的行数
	QList<QTableWidgetItem*> selectedItems = ui.AlignmentIntegrationList_TableW->selectedItems();
	QSet<int> selectedRows;
	for (QTableWidgetItem* item : selectedItems) {
		selectedRows.insert(item->row());
	}
	int selectedRowCount = selectedRows.size();

	// 如果选中的行只有一行，将"模型配对"菜单项置为不可用
	if (selectedRowCount <= 1) {
		actionMap[QString::fromLocal8Bit("模型配对")]->setEnabled(false);
	}

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载原始影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.AlignmentIntegrationList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.AlignmentIntegrationList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				AlignmentIntFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.AlignmentIntegrationList_TableW->rowCount();
				ui.AlignmentIntegrationList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.AlignmentIntegrationList_TableW->columnCount(); col++)
				{
					if (!ui.AlignmentIntegrationList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.AlignmentIntegrationList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.AlignmentIntegrationList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.AlignmentIntegrationList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.AlignmentIntegrationList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.AlignmentIntegrationList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(AlignmentIntFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择
		}
		m_lastPath = folderPath;
		// 设置文件过滤器
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.AlignmentIntegrationList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.AlignmentIntegrationList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				AlignmentIntFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.AlignmentIntegrationList_TableW->rowCount();
				ui.AlignmentIntegrationList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.AlignmentIntegrationList_TableW->columnCount(); col++)
				{
					if (!ui.AlignmentIntegrationList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.AlignmentIntegrationList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.AlignmentIntegrationList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.AlignmentIntegrationList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.AlignmentIntegrationList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
		ui.AlignmentIntegrationList_TableW->resizeColumnsToContents();
		setGeoreferenceFromXMLAndRPC(AlignmentIntFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AlignmentIntegrationList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AlignmentIntegrationList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.AlignmentIntegrationList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AlignmentIntFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					AlignmentIntFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("模型配对")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		QStringList Buildfilenames;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AlignmentIntegrationList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}

		// 检查选中的行数，如果只有一行则直接返回
		QSet<int> rows;
		for (QTableWidgetItem* item : referedItems) {
			rows.insert(item->row());
		}
		if (rows.size() <= 1) {
			return;
		}

		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AlignmentIntegrationList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		//// 删除所有选中的行
		//for (int row : referedRows) {
		//	ui.AlignmentIntegrationList_TableW->removeRow(row);
		//}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AlignmentIntFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					Buildfilenames.push_back(filenamePATH);
					//AlignmentIntFilePath.removeAll(filenamePATH);
				}
			}
		}
		FusionBuildmodel(Buildfilenames, QString::fromLocal8Bit("fusionmodelList_TableW_2"), "RegisterFusion");

	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AlignmentIntegrationList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AlignmentIntegrationList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AlignmentIntFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

void IMAGEPS::AlignmentIntList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/配准融合_model.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("删除配对模型")], &QAction::triggered, this, [=]
	{
		Fusiondeletemodel(AlignmentIntFilePath, QString::fromLocal8Bit("fusionmodelList_TableW_2"));
	});

	connect(actionMap[QString::fromLocal8Bit("查看模型")], &QAction::triggered, this, [=]
	{

	});

	connect(actionMap[QString::fromLocal8Bit("查看处理进度")], &QAction::triggered, this, [=]
	{

	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 智能镶嵌右键菜单
 */
void IMAGEPS::SmartMosaicDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/智能镶嵌.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);
			QString baseName = fileInfo.completeBaseName(); // 获取无后缀文件名

			bool alreadyExists = false;
			for (int i = 0; i < ui.SmartMosaicDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.SmartMosaicDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			// 检查是否存在于DataModelPath中
			bool existsInDataModel = false;
			QString dataModelImagePath;
			for (const QString &path : DataModelPath) {
				if (QFileInfo(path).completeBaseName() == baseName) {
					existsInDataModel = true;
					dataModelImagePath = path;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				SmartMosaicFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.SmartMosaicDataList_TableW->rowCount();
				ui.SmartMosaicDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.SmartMosaicDataList_TableW->columnCount(); col++)
				{
					if (!ui.SmartMosaicDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.SmartMosaicDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString panPart;
				if (existsInDataModel) {
					panPart = systemConfig->getTiffImageType(dataModelImagePath);
				}
				else {
					panPart = systemConfig->getTiffImageType(originImagePath);
				}
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);

				// 设置影像类型（优先使用DataModelPath中的文件）
				QString tmptype;
				if (existsInDataModel) {
					tmptype = getImagetype(dataModelImagePath);
				}
				else {
					tmptype = getImagetype(originImagePath);
				}
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 3)->setText(tmptype);

				// 设置影像时间（优先使用DataModelPath中的文件）
				QString time;
				if (existsInDataModel) {
					time = extractImageTime(dataModelImagePath, tmptype);
				}
				else {
					time = extractImageTime(originImagePath, tmptype);
				}
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 4)->setText(time);

				//QString panPart = systemConfig->getTiffImageType(originImagePath);

				//ui.SmartMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);

				//QString tmptype = getImagetype(originImagePath);
				//ui.SmartMosaicDataList_TableW->item(newRowIndex, 3)->setText(tmptype);

				//QString time = extractImageTime(originImagePath, tmptype);
				//ui.SmartMosaicDataList_TableW->item(newRowIndex, 4)->setText(time);
			}
		}
		ui.SmartMosaicDataList_TableW->resizeColumnsToContents();
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(SmartMosaicFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择
		}
		m_lastPath = folderPath;
		// 设置文件过滤器
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			QString baseName = fileInfo.completeBaseName(); // 获取无后缀文件名
			for (int i = 0; i < ui.SmartMosaicDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.SmartMosaicDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			// 检查是否存在于DataModelPath中
			bool existsInDataModel = false;
			QString dataModelImagePath;
			for (const QString &path : DataModelPath) {
				if (QFileInfo(path).completeBaseName() == baseName) {
					existsInDataModel = true;
					dataModelImagePath = path;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				SmartMosaicFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.SmartMosaicDataList_TableW->rowCount();
				ui.SmartMosaicDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.SmartMosaicDataList_TableW->columnCount(); col++)
				{
					if (!ui.SmartMosaicDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.SmartMosaicDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				QString panPart;
				if (existsInDataModel) {
					panPart = systemConfig->getTiffImageType(dataModelImagePath);
				}
				else {
					panPart = systemConfig->getTiffImageType(originImagePath);
				}
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);

				// 设置影像类型（优先使用DataModelPath中的文件）
				QString tmptype;
				if (existsInDataModel) {
					tmptype = getImagetype(dataModelImagePath);
				}
				else {
					tmptype = getImagetype(originImagePath);
				}
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 3)->setText(tmptype);

				// 设置影像时间（优先使用DataModelPath中的文件）
				QString time;
				if (existsInDataModel) {
					time = extractImageTime(dataModelImagePath, tmptype);
				}
				else {
					time = extractImageTime(originImagePath, tmptype);
				}
				ui.SmartMosaicDataList_TableW->item(newRowIndex, 4)->setText(time);

				//QString panPart = systemConfig->getTiffImageType(originImagePath);

				//ui.SmartMosaicDataList_TableW->item(newRowIndex, 2)->setText(panPart);

				//QString tmptype = getImagetype(originImagePath);
				//ui.SmartMosaicDataList_TableW->item(newRowIndex, 3)->setText(tmptype);

				//QString time = extractImageTime(originImagePath, tmptype);
				//ui.SmartMosaicDataList_TableW->item(newRowIndex, 4)->setText(time);
			}
		}
		ui.SmartMosaicDataList_TableW->resizeColumnsToContents();

		setGeoreferenceFromXMLAndRPC(SmartMosaicFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.SmartMosaicDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.SmartMosaicDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.SmartMosaicDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : SmartMosaicFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					SmartMosaicFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.SmartMosaicDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.SmartMosaicDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : SmartMosaicFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

void IMAGEPS::MosaicCropDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/智能镶嵌_crop.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载裁切范围(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载裁切范围文件"), m_lastPath, tr("shapeFile(*.shp)"));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		for (const QString &originImagePath : originImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.MosaicCropDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.MosaicCropDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				MosaicCropFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.MosaicCropDataList_TableW->rowCount();
				ui.MosaicCropDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.MosaicCropDataList_TableW->columnCount(); col++)
				{
					if (!ui.MosaicCropDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.MosaicCropDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.MosaicCropDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.MosaicCropDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				ui.MosaicCropDataList_TableW->resizeColumnsToContents();

				//QString panPart = systemConfig->getTiffImageType(originImagePath);

				//ui.MosaicCropDataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载裁切范围(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载裁切范围(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择
		}
		m_lastPath = folderPath;
		// 设置文件过滤器
		QStringList filters;
		filters << "*.shp";
		// 使用 QDirIterator 递归遍历所有子目录
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}

		for (const QString &originImagePath : filePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.MosaicCropDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.MosaicCropDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				MosaicCropFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.MosaicCropDataList_TableW->rowCount();
				ui.MosaicCropDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.MosaicCropDataList_TableW->columnCount(); col++)
				{
					if (!ui.MosaicCropDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.MosaicCropDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.MosaicCropDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.MosaicCropDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				ui.MosaicCropDataList_TableW->resizeColumnsToContents();
				//QString panPart = systemConfig->getTiffImageType(originImagePath);

				//ui.MosaicCropDataList_TableW->item(newRowIndex, 2)->setText(panPart);
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("卸载裁切范围")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.MosaicCropDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.MosaicCropDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.MosaicCropDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : MosaicCropFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					MosaicCropFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 影像裁切右键
 */
void IMAGEPS::ImagecropDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/影像裁切数据列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}


	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFiles(ui.ImagecropDataList_TableW, ImagecropFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFolder(ui.ImagecropDataList_TableW, ImagecropFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=] {
		unloadImages(ui.ImagecropDataList_TableW, ImagecropFilePath);
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 裁切矢量右键
 */
void IMAGEPS::VectorfileDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/裁切矢量数据列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}


	connect(actionMap[QString::fromLocal8Bit("加载矢量(文件)")], &QAction::triggered, this, [=] {
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载矢量文件"), m_lastPath, tr("shapeFile(*.shp)"));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}

		for (const QString &originImagePath : originImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.VectorfileDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.VectorfileDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}
			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				VectorfileFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.VectorfileDataList_TableW->rowCount();
				ui.VectorfileDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.VectorfileDataList_TableW->columnCount(); col++)
				{
					if (!ui.VectorfileDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.VectorfileDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.VectorfileDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.VectorfileDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.VectorfileDataList_TableW->resizeColumnsToContents();
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载矢量(文件夹)")], &QAction::triggered, this, [=] {
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载矢量(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择
		}
		m_lastPath = folderPath;
		// 设置文件过滤器
		QStringList filters;
		filters << "*.shp";
		// 使用 QDirIterator 递归遍历所有子目录
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}

		for (const QString &originImagePath : filePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.VectorfileDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.VectorfileDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				VectorfileFilePath.push_back(originImagePath);
				// 插入新行
				int newRowIndex = ui.VectorfileDataList_TableW->rowCount();
				ui.VectorfileDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.VectorfileDataList_TableW->columnCount(); col++)
				{
					if (!ui.VectorfileDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.VectorfileDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.VectorfileDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.VectorfileDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.VectorfileDataList_TableW->resizeColumnsToContents();
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("卸载矢量")], &QAction::triggered, this, [=] {
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.VectorfileDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.VectorfileDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.VectorfileDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : VectorfileFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					VectorfileFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}


/**
 * @brief 绝对定位精度质检右键菜单
 */
void IMAGEPS::AbsPositPrecCheckList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/绝对定位精度质检.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("加载待检影像(文件)")], &QAction::triggered, this, [=]
	{
		QStringList originImagePaths = QFileDialog::getOpenFileNames(this, QString::fromLocal8Bit("选择加载待检影像文件"), m_lastPath, tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)")));

		if (!originImagePaths.isEmpty()) {
			m_lastPath = QFileInfo(originImagePaths.first()).path();  // 更新最后路径 
		}
		QStringList finalImagePaths = filterImageFiles(originImagePaths);
		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			bool alreadyExists = false;
			for (int i = 0; i < ui.AbsPositPrecCheckDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.AbsPositPrecCheckDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!originImagePath.isEmpty() && !alreadyExists)
			{
				hasNewFiles = true;
				AbsPositPrecCheckFilePath.push_back(originImagePath);
				// 插入新行 
				int newRowIndex = ui.AbsPositPrecCheckDataList_TableW->rowCount();
				ui.AbsPositPrecCheckDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.AbsPositPrecCheckDataList_TableW->columnCount(); col++)
				{
					if (!ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.AbsPositPrecCheckDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());


				QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, 2)->setText(QString::fromLocal8Bit("否"));
				ui.AbsPositPrecCheckDataList_TableW->resizeColumnsToContents();

			}
		}
		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(AbsPositPrecCheckFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载待检影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载待检影像(文件夹)"),
			m_lastPath,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);
		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}
		m_lastPath = folderPath; // 更新最后路径 
		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";
		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);
		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		for (const QString &originImagePath : finalImagePaths) {
			bool containsexit = true;
			QFileInfo fileInfo(originImagePath);
			for (int i = 0; i < ui.AbsPositPrecCheckDataList_TableW->rowCount(); i++)
			{
				if (fileInfo.fileName().contains(ui.AbsPositPrecCheckDataList_TableW->item(i, 1)->text()))
				{
					containsexit = false;
					break;
				}
			}

			if (!originImagePath.isEmpty() && containsexit)
			{
				AbsPositPrecCheckFilePath.push_back(originImagePath);
				// 插入极光新行 
				int newRowIndex = ui.AbsPositPrecCheckDataList_TableW->rowCount();
				ui.AbsPositPrecCheckDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.AbsPositPrecCheckDataList_TableW->columnCount(); col++)
				{
					if (!ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.AbsPositPrecCheckDataList_TableW->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

				//QString panPart = systemConfig->getTiffImageType(originImagePath);

				ui.AbsPositPrecCheckDataList_TableW->item(newRowIndex, 2)->setText(QString::fromLocal8Bit("否"));
				ui.AbsPositPrecCheckDataList_TableW->resizeColumnsToContents();
			}
		}
		setGeoreferenceFromXMLAndRPC(AbsPositPrecCheckFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AbsPositPrecCheckDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AbsPositPrecCheckDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.AbsPositPrecCheckDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AbsPositPrecCheckFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					AbsPositPrecCheckFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AbsPositPrecCheckDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AbsPositPrecCheckDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AbsPositPrecCheckFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 相对定位精度质检右键菜单
 */
void IMAGEPS::RelPositPrecCheckList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/相对定位精度质检.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("质检模型构建")], &QAction::triggered, this, [=]
	{
		QStringList sourceData;
		int rowCount = ui.sateImageDataList_TableW->rowCount();
		for (int i = 0; i < rowCount; i++) {
			QTableWidgetItem* item = ui.sateImageDataList_TableW->item(i, 1);
			if (item && !item->text().isEmpty()) {
				sourceData.append(item->text());
			}
		}

		QVector<QPair<QString, QString>> pairs;
		for (int i = 0; i < sourceData.size(); i++) {
			for (int j = i + 1; j < sourceData.size(); j++) {
				pairs.append(qMakePair(sourceData[i], sourceData[j]));
			}
		}

		ui.RelPositPrecCheckDataList_TableW->clearContents();
		ui.RelPositPrecCheckDataList_TableW->setRowCount(pairs.size());

		for (int i = 0; i < pairs.size(); i++) {
			ui.RelPositPrecCheckDataList_TableW->item(i, 0)->setText(QString::number(i));
			if (ui.RelPositPrecCheckDataList_TableW->item(i, 1) == nullptr) {
				ui.RelPositPrecCheckDataList_TableW->setItem(i, 1, new QTableWidgetItem(pairs[i].first));
			}
			else {
				ui.RelPositPrecCheckDataList_TableW->item(i, 1)->setText(pairs[i].first);
			}

			if (ui.RelPositPrecCheckDataList_TableW->item(i, 2) == nullptr) {
				ui.RelPositPrecCheckDataList_TableW->setItem(i, 2, new QTableWidgetItem(pairs[i].second));
			}
			else {
				ui.RelPositPrecCheckDataList_TableW->item(i, 2)->setText(pairs[i].second);
			}
			ui.RelPositPrecCheckDataList_TableW->item(i, 3)->setText(QString::fromLocal8Bit("否"));
		}

	});

	connect(actionMap[QString::fromLocal8Bit("质检模型删除")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AbsPositPrecCheckDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AbsPositPrecCheckDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.AbsPositPrecCheckDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AbsPositPrecCheckFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					AbsPositPrecCheckFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> referedItems = ui.AbsPositPrecCheckDataList_TableW->selectedItems();
		if (referedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : referedItems) {
			rowsToDelete.insert(item->row());
			QTableWidgetItem* firstColumnItem = ui.AbsPositPrecCheckDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : AbsPositPrecCheckFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

///////////////////////////////
/**
 * @brief 测区显示右键
 */
void IMAGEPS::measureAreaShow_GLWContextSlot(const QPoint & pos)
{
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/测区显示.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			menu->addAction(data[0]);
	}
	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 加密点列表右键
 */
void IMAGEPS::encryptPointsList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/加密点列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	//connect(actionMap[QString::fromLocal8Bit("删除")], &QAction::triggered, this, [=]
	//{
	//	// 获取选中的所有行（避免重复）
	//	//QList<QTableWidgetItem*> referedItems = ui.encryptPointsList_TableW->selectedItems();
	//	QModelIndexList selectedRows = ui.encryptPointsList_TableW->selectionModel()->selectedRows();
	//	if (selectedRows.isEmpty()) {
	//		return; // 如果没有选中任何行，不显示菜单
	//	}
	//	// 提取所有选中的行号（避免重复）
	//	QSet<int> rowsToDelete;

	//	//NonEditableModel *model = qobject_cast<NonEditableModel*>(ui.encryptPointsList_TableW->model());
	//	if (!encryptPointsModel) {
	//		qWarning() << "No valid model set for QTableView";
	//		return;
	//	}

	//	for (const QModelIndex& index : selectedRows) {
	//		int row = index.row();
	//		rowsToDelete.insert(row);

	//		// 获取第1列（索引为1）的单元格数据
	//		QModelIndex keyIndex = encryptPointsModel->index(row, 1); // 第1列
	//		if (!keyIndex.isValid())  continue;

	//		QVariant data = encryptPointsModel->data(keyIndex, Qt::DisplayRole);
	//		QString keyText = data.toString();
	//		if (keyText.isEmpty())  continue;
	//		int key = keyText.toInt();

	//		// 取消高亮ID为key的连接点 
	//		ui.ImageInfoShow_Widget->highlightPointById(keyText, false);

	//		// 删除ID为key的连接点 
	//		ui.ImageInfoShow_Widget->removePointById(keyText, "con");
	//		ui.encryptPointsnum->setText(QString::fromLocal8Bit("加密点列表数量: %1").arg(encryptPointsModel->rowCount()));
	//	}
	//	// 按从大到小的顺序删除（防止索引错乱）
	//	QList<int> referedRows = rowsToDelete.values();
	//	std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
	//	// 删除所有选中的行
	//	for (int row : referedRows) {
	//		encryptPointsModel->removeRow(row); // 通过模型删除行
	//	}
	//	if (encryptPointsModel->rowCount() == 0)
	//		ui.encryptPointsnum->setText(QString::fromLocal8Bit("加密点列表数量: %1").arg(encryptPointsModel->rowCount()));
	//});

	connect(actionMap[QString::fromLocal8Bit("删除")], &QAction::triggered, this, [=]
	{
		// 获取选中的所有行 
		const QModelIndexList& selectedRows = ui.encryptPointsList_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty())  return;
		if (!encryptPointsModel) return;

		// 批量收集要删除的行和点ID 
		QVector<int> rowsToDelete;
		QSet<QString> pointIdsToDelete;

		// 一次性收集所有需要删除的数据 
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();
			QModelIndex keyIndex = encryptPointsModel->index(row, 1);
			if (!keyIndex.isValid())  continue;

			QString pointId = encryptPointsModel->data(keyIndex, Qt::DisplayRole).toString();
			if (!pointId.isEmpty()) {
				rowsToDelete.append(row);
				pointIdsToDelete.insert(pointId);
			}
		}

		// 按从大到小排序 
		std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

		// 批量删除图形显示 
		for (const QString& pointId : pointIdsToDelete) {
			ui.ImageInfoShow_Widget->highlightPointById(pointId, false);
			ui.ImageInfoShow_Widget->removePointById(pointId, "con");
		}

		// 批量删除模型行 
		encryptPointsModel->removeRowsBatch(rowsToDelete);

		// 批量删除哈希表中的数据 
		for (const QString& pointId : pointIdsToDelete) {
			m_encryptedPoints.remove(pointId);
		}

		// 更新显示 
		int currentCount = encryptPointsModel->rowCount();
		ui.encryptPointsnum->setText(QString::fromLocal8Bit(" 加密点列表数量: %1").arg(currentCount));
	});

	connect(actionMap[QString::fromLocal8Bit("显示")], &QAction::triggered, this, [=]
	{
		QModelIndexList selectedRows = ui.encryptPointsList_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		//// 获取选中的所有行（避免重复）
		//QList<QTableWidgetItem*> referedItems = ui.encryptPointsList_TableW->selectedItems();
		//if (referedItems.isEmpty()) {
		//	return;
		//}
		if (!encryptPointsModel) {
			qWarning() << "No valid model set for QTableView";
			return;
		}
		// 提取所有选中的行号
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();

			// 获取第1列（索引为1）的单元格数据
			QModelIndex keyIndex = encryptPointsModel->index(row, 1); // 第1列
			if (!keyIndex.isValid())  continue;

			QVariant data = encryptPointsModel->data(keyIndex, Qt::DisplayRole);
			QString keyText = data.toString();
			if (keyText.isEmpty())  continue;

			ui.ImageInfoShow_Widget->setConPointsVisible(true, keyText);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("隐藏")], &QAction::triggered, this, [=]
	{
		// 获取选中的所有行（避免重复）
		QModelIndexList selectedRows = ui.encryptPointsList_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		//QList<QTableWidgetItem*> referedItems = ui.encryptPointsList_TableW->selectedItems();
		//if (referedItems.isEmpty()) {
		//	return; 
		//}
		if (!encryptPointsModel) {
			qWarning() << "No valid model set for QTableView";
			return;
		}
		// 提取所有选中的行号
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();
			// 获取第1列（索引为1）的单元格数据
			QModelIndex keyIndex = encryptPointsModel->index(row, 1); // 第1列
			if (!keyIndex.isValid())  continue;

			QVariant data = encryptPointsModel->data(keyIndex, Qt::DisplayRole);
			QString keyText = data.toString();
			if (keyText.isEmpty())  continue;

			ui.ImageInfoShow_Widget->setConPointsVisible(false, keyText);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("设置为已确认")], &QAction::triggered, this, [=]
	{
		// 收集选中的ID 
		QSet<QString> selectedIDs;
		QModelIndexList selectedRows = ui.encryptPointsList_TableW->selectionModel()->selectedRows();
		for (const QModelIndex& index : selectedRows) {
			QModelIndex idIndex = ui.encryptPointsList_TableW->model()->index(index.row(), 1);
			selectedIDs.insert(ui.encryptPointsList_TableW->model()->data(idIndex).toString());
		}

		// 启动异步更新 
		asyncUpdateEncData(selectedIDs, QString::fromLocal8Bit("Y"));
	});

	connect(actionMap[QString::fromLocal8Bit("设置为未确认")], &QAction::triggered, this, [=]
	{
		// 收集选中的ID 
		QSet<QString> selectedIDs;
		QModelIndexList selectedRows = ui.encryptPointsList_TableW->selectionModel()->selectedRows();
		for (const QModelIndex& index : selectedRows) {
			QModelIndex idIndex = ui.encryptPointsList_TableW->model()->index(index.row(), 1);
			selectedIDs.insert(ui.encryptPointsList_TableW->model()->data(idIndex).toString());
		}

		// 启动异步更新 
		asyncUpdateEncData(selectedIDs, QString::fromLocal8Bit("N"));
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 控制点列表右键
 */
void IMAGEPS::controlPointsLIst_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/控制点列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	//connect(actionMap[QString::fromLocal8Bit("删除")], &QAction::triggered, this, [=]
	//{
	//	// 获取选中的所有行（避免重复）
	//	//QList<QTableWidgetItem*> referedItems = ui.controlPointsLIst_TableW->selectedItems();
	//	//if (referedItems.isEmpty()) {
	//	//	return; // 如果没有选中任何行，不显示菜单
	//	//}
	//	QModelIndexList selectedRows = ui.controlPointsLIst_TableW->selectionModel()->selectedRows();
	//	if (selectedRows.isEmpty()) {
	//		return; // 如果没有选中任何行，不显示菜单
	//	}
	//	// 提取所有选中的行号（避免重复）
	//	QSet<int> rowsToDelete;
	//	if (!ControlPointsModel) {
	//		qWarning() << "No valid model set for QTableView";
	//		return;
	//	}

	//	for (const QModelIndex& index : selectedRows) {
	//		int row = index.row();
	//		rowsToDelete.insert(row);

	//		// 获取第1列（索引为1）的单元格数据
	//		QModelIndex keyIndex = ControlPointsModel->index(row, 1); // 第1列
	//		if (!keyIndex.isValid())  continue;

	//		QVariant data = ControlPointsModel->data(keyIndex, Qt::DisplayRole);
	//		QString keyText = data.toString();
	//		if (keyText.isEmpty())  continue;
	//		int key = keyText.toInt();

	//		// 取消高亮ID为key的控制点 
	//		ui.ImageInfoShow_Widget->highlightPointById(keyText, false);

	//		// 删除ID为key的控制点 
	//		ui.ImageInfoShow_Widget->removePointById(keyText, "col");
	//		ui.controlPointsnum->setText(QString::fromLocal8Bit("控制点列表数量: %1").arg(ControlPointsModel->rowCount()));
	//	}
	//	// 按从大到小的顺序删除（防止索引错乱）
	//	QList<int> referedRows = rowsToDelete.values();
	//	std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
	//	// 删除所有选中的行
	//	for (int row : referedRows) {
	//		ControlPointsModel->removeRow(row);
	//	}
	//	if(ControlPointsModel->rowCount() == 0)
	//		ui.controlPointsnum->setText(QString::fromLocal8Bit("控制点列表数量: %1").arg(ControlPointsModel->rowCount()));
	//});

	connect(actionMap[QString::fromLocal8Bit("删除")], &QAction::triggered, this, [=]
	{
		// 获取选中的所有行 
		const QModelIndexList& selectedRows = ui.controlPointsLIst_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty())  return;
		if (!ControlPointsModel) return;

		// 批量收集要删除的行和点ID 
		QVector<int> rowsToDelete;
		QSet<QString> pointIdsToDelete;

		// 一次性收集所有需要删除的数据 
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();
			QModelIndex keyIndex = ControlPointsModel->index(row, 1);
			if (!keyIndex.isValid())  continue;

			QString pointId = ControlPointsModel->data(keyIndex, Qt::DisplayRole).toString();
			if (!pointId.isEmpty()) {
				rowsToDelete.append(row);
				pointIdsToDelete.insert(pointId);
			}
		}

		// 按从大到小排序 
		std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

		// 批量删除图形显示 
		for (const QString& pointId : pointIdsToDelete) {
			ui.ImageInfoShow_Widget->highlightPointById(pointId, false);
			ui.ImageInfoShow_Widget->removePointById(pointId, "col");
		}

		// 批量删除模型行 
		ControlPointsModel->removeRowsBatch(rowsToDelete);

		// 批量删除哈希表中的数据 
		for (const QString& pointId : pointIdsToDelete) {
			m_controlPoints.remove(pointId);
		}

		// 更新显示 
		int currentCount = ControlPointsModel->rowCount();
		ui.controlPointsnum->setText(QString::fromLocal8Bit(" 加密点列表数量: %1").arg(currentCount));
	});

	connect(actionMap[QString::fromLocal8Bit("显示")], &QAction::triggered, this, [=]
	{
		// 获取选中的所有行（避免重复）
		QModelIndexList selectedRows = ui.controlPointsLIst_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		//QList<QTableWidgetItem*> referedItems = ui.controlPointsLIst_TableW->selectedItems();
		//if (referedItems.isEmpty()) {
		//	return; // 如果没有选中任何行，不显示菜单
		//}
		// 提取所有选中的行号（避免重复）
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();

			// 获取第1列（索引为1）的单元格数据
			QModelIndex keyIndex = encryptPointsModel->index(row, 1); // 第1列
			if (!keyIndex.isValid())  continue;

			QVariant data = encryptPointsModel->data(keyIndex, Qt::DisplayRole);
			QString keyText = data.toString();
			if (keyText.isEmpty())  continue;

			ui.ImageInfoShow_Widget->setColPointsVisible(true, keyText);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("隐藏")], &QAction::triggered, this, [=]
	{
		//// 获取选中的所有行（避免重复）
		//QList<QTableWidgetItem*> referedItems = ui.controlPointsLIst_TableW->selectedItems();
		//if (referedItems.isEmpty()) {
		//	return; // 如果没有选中任何行，不显示菜单
		//}
		QModelIndexList selectedRows = ui.controlPointsLIst_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();

			// 获取第1列（索引为1）的单元格数据
			QModelIndex keyIndex = encryptPointsModel->index(row, 1); // 第1列
			if (!keyIndex.isValid())  continue;

			QVariant data = encryptPointsModel->data(keyIndex, Qt::DisplayRole);
			QString keyText = data.toString();
			if (keyText.isEmpty())  continue;

			ui.ImageInfoShow_Widget->setColPointsVisible(false, keyText);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("设置为已确认")], &QAction::triggered, this, [=]
	{
		// 收集选中的ID 
		QSet<QString> selectedIDs;
		QModelIndexList selectedRows = ui.controlPointsLIst_TableW->selectionModel()->selectedRows();
		for (const QModelIndex& index : selectedRows) {
			QModelIndex idIndex = ui.controlPointsLIst_TableW->model()->index(index.row(), 1);
			selectedIDs.insert(ui.controlPointsLIst_TableW->model()->data(idIndex).toString());
		}

		// 启动异步更新 
		asyncUpdateColData(selectedIDs, QString::fromLocal8Bit("Y"));
	});

	connect(actionMap[QString::fromLocal8Bit("设置为未确认")], &QAction::triggered, this, [=]
	{
		// 收集选中的ID 
		QSet<QString> selectedIDs;
		QModelIndexList selectedRows = ui.controlPointsLIst_TableW->selectionModel()->selectedRows();
		for (const QModelIndex& index : selectedRows) {
			QModelIndex idIndex = ui.controlPointsLIst_TableW->model()->index(index.row(), 1);
			selectedIDs.insert(ui.controlPointsLIst_TableW->model()->data(idIndex).toString());
		}

		// 启动异步更新 
		asyncUpdateColData(selectedIDs, QString::fromLocal8Bit("N"));
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 遥感影像云检数据列表右键
 */
void IMAGEPS::CloudDetectionDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/云检数据列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}


	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=]
	{
		// 支持多选文件的文件对话框 
		QStringList originImagePaths = QFileDialog::getOpenFileNames(
			this,
			QString::fromLocal8Bit("选择加载原始影像文件"),
			m_lastPath,  // 使用上次路径 
			tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)")
			.arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)"))
		);

		if (originImagePaths.isEmpty()) {
			return;
		}

		// 更新最后选择的路径 
		QFileInfo firstFileInfo(originImagePaths.first());
		m_lastPath = firstFileInfo.absolutePath();
		QStringList finalImagePaths = filterImageFiles(originImagePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			QFileInfo fileInfo(originImagePath);

			// 检查是否已存在相同文件名的记录 
			bool alreadyExists = false;
			for (int i = 0; i < ui.CloudDetectionDataList_TableW->rowCount(); i++) {
				if (fileInfo.fileName().contains(ui.CloudDetectionDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists) {
				hasNewFiles = true;
				CloudDetectionDataListFilePath.push_back(originImagePath);

				// 插入新行 
				int newRowIndex = ui.CloudDetectionDataList_TableW->rowCount();
				ui.CloudDetectionDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.CloudDetectionDataList_TableW->columnCount(); col++) {
					if (!ui.CloudDetectionDataList_TableW->item(newRowIndex, col)) {
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.CloudDetectionDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				// 设置序号（当前行号 + 1）
				ui.CloudDetectionDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.CloudDetectionDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.CloudDetectionDataList_TableW->resizeColumnsToContents();
			}
		}

		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(CloudDetectionDataListFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=]
	{
		QString folderPath = QFileDialog::getExistingDirectory(
			this,
			QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
			m_lastPath,  // 使用上次路径 
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
		);

		if (folderPath.isEmpty()) {
			return; // 用户取消选择 
		}

		// 更新最后选择的路径 
		m_lastPath = folderPath;

		// 设置文件过滤器 
		QStringList filters;
		filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";

		// 使用 QDirIterator 递归遍历所有子目录 
		QDirIterator dirIterator(
			folderPath,
			filters,
			QDir::Files | QDir::NoDotAndDotDot,
			QDirIterator::Subdirectories // 递归遍历子目录 
		);

		QStringList filePaths;
		while (dirIterator.hasNext()) {
			filePaths.append(dirIterator.next());
		}
		QStringList finalImagePaths = filterImageFiles(filePaths);

		bool hasNewFiles = false;
		for (const QString &originImagePath : finalImagePaths) {
			bool alreadyExists = false;
			QFileInfo fileInfo(originImagePath);

			for (int i = 0; i < ui.CloudDetectionDataList_TableW->rowCount(); i++) {
				if (fileInfo.fileName().contains(ui.CloudDetectionDataList_TableW->item(i, 1)->text())) {
					alreadyExists = true;
					break;
				}
			}

			if (!alreadyExists) {
				hasNewFiles = true;
				CloudDetectionDataListFilePath.push_back(originImagePath);

				// 插入新行 
				int newRowIndex = ui.CloudDetectionDataList_TableW->rowCount();
				ui.CloudDetectionDataList_TableW->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem 
				for (int col = 0; col < ui.CloudDetectionDataList_TableW->columnCount(); col++) {
					if (!ui.CloudDetectionDataList_TableW->item(newRowIndex, col)) {
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.CloudDetectionDataList_TableW->setItem(newRowIndex, col, item);
					}
				}

				// 设置序号（当前行号 + 1）
				ui.CloudDetectionDataList_TableW->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.CloudDetectionDataList_TableW->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());
				ui.CloudDetectionDataList_TableW->resizeColumnsToContents();

			}
		}

		if (hasNewFiles) {
			setGeoreferenceFromXMLAndRPC(CloudDetectionDataListFilePath);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> selectedItems = ui.CloudDetectionDataList_TableW->selectedItems();
		if (selectedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : selectedItems) {
			rowsToDelete.insert(item->row());

			QTableWidgetItem* firstColumnItem = ui.CloudDetectionDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> sortedRows = rowsToDelete.values();
		std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : sortedRows) {
			ui.CloudDetectionDataList_TableW->removeRow(row);
		}
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : CloudDetectionDataListFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					CloudDetectionDataListFilePath.removeAll(filenamePATH);
				}
			}
		}
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=]
	{
		QStringList firstColumnValues;
		// 获取选中的所有行（避免重复）
		QList<QTableWidgetItem*> selectedItems = ui.CloudDetectionDataList_TableW->selectedItems();
		if (selectedItems.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (QTableWidgetItem* item : selectedItems) {
			rowsToDelete.insert(item->row());

			QTableWidgetItem* firstColumnItem = ui.CloudDetectionDataList_TableW->item(item->row(), 1);
			if (firstColumnItem) {
				firstColumnValues.append(firstColumnItem->text());  // 存储第一列的值
			}
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> sortedRows = rowsToDelete.values();
		std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());
		displayView(ui.dockWidget_9, ui.tabWidget, 1);
		for (auto filename : firstColumnValues)
		{
			for (auto filenamePATH : CloudDetectionDataListFilePath)
			{
				QFileInfo fileInfo(filenamePATH);
				if (fileInfo.completeBaseName() == filename)
				{
					on_actionOpenImageShow(filenamePATH);
					break;
				}
			}
		}

	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 投影转换数据列表右键
 */
void IMAGEPS::ProTransformationDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/投影转换数据列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}


	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFiles(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFolder(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=] {
		unloadImages(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=] {
		viewImages(ui.ProjectionTransformationDataList_TableW, ProTransformationFilePath);
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 格式转换数据列表右键
 */
void IMAGEPS::FormatConversionDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/格式转换数据列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}


	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFiles(ui.FormatConversionDataList_TableW, FormatConversionFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFolder(ui.FormatConversionDataList_TableW, FormatConversionFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=] {
		unloadImages(ui.FormatConversionDataList_TableW, FormatConversionFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=] {
		viewImages(ui.FormatConversionDataList_TableW, FormatConversionFilePath);
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief SAR影像滤波数据列表右键
 */
void IMAGEPS::SARImageFilteringDataList_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/SAR影像滤波数据列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}


	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFiles(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("加载原始影像(文件夹)")], &QAction::triggered, this, [=] {
		loadOriginalImagesFromFolder(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("卸载影像")], &QAction::triggered, this, [=] {
		unloadImages(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath);
	});

	connect(actionMap[QString::fromLocal8Bit("查看影像")], &QAction::triggered, this, [=] {
		viewImages(ui.SARImageFilteringDataList_TableW, SARImageFilteringFilePath);
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}


/**
 * @brief 云检成果列表右键
 */
void IMAGEPS::CloudDetectionLIst_TabWContextSlot(const QPoint & pos)
{
	QMap<QString, QAction*> actionMap;
	QMenu* menu = new QMenu;
	QList<QStringList> config = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/mainWindowConfig/contextMenu/云检成果列表.csv"), ",");
	for (auto data : config)
	{
		if (data[1] == "TRUE")
			actionMap[data[0]] = menu->addAction(data[0]);
	}

	connect(actionMap[QString::fromLocal8Bit("删除")], &QAction::triggered, this, [=]
	{
		QModelIndexList selectedRows = ui.CloudDetectionList_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();
			rowsToDelete.insert(row);

			QTableWidgetItem *keyItem = ui.CloudDetectionList_TableW->item(row, 1);
			if (!keyItem || keyItem->text().isEmpty()) return;
			int key = keyItem->text().toInt();
		}
		// 按从大到小的顺序删除（防止索引错乱）
		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());
		// 删除所有选中的行
		for (int row : referedRows) {
			ui.CloudDetectionList_TableW->removeRow(row);
		}
	});

	connect(actionMap[QString::fromLocal8Bit("刷新")], &QAction::triggered, this, [=]
	{

	});

	connect(actionMap[QString::fromLocal8Bit("预览")], &QAction::triggered, this, [=]
	{
		QModelIndexList selectedRows = ui.CloudDetectionList_TableW->selectionModel()->selectedRows();
		if (selectedRows.isEmpty()) {
			return; // 如果没有选中任何行，不显示菜单
		}
		// 提取所有选中的行号（避免重复）
		QSet<int> rowsToDelete;
		for (const QModelIndex& index : selectedRows) {
			int row = index.row();
			rowsToDelete.insert(row);

			QTableWidgetItem *keyItem = ui.CloudDetectionList_TableW->item(row, 1);
			if (!keyItem || keyItem->text().isEmpty()) return;
			int key = keyItem->text().toInt();
		}

		QList<int> referedRows = rowsToDelete.values();
		std::sort(referedRows.begin(), referedRows.end(), std::greater<int>());

		for (int row : referedRows) {
			QString filePath = ui.CloudDetectionList_TableW->item(row, 1)->text();
			for (auto filename : CloudDetectionListFilePath)
			{
				QFileInfo fileinfo(filename);
				if (filePath == fileinfo.completeBaseName())
				{
					filePath = filename;
				}
			}
			// 验证文件存在 
			if (!QFile::exists(filePath)) {
				QMessageBox::warning(this, "Error", "File not found:\n" + filePath);
				return;
			}
			// 使用系统默认程序打开
			QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
		}
	});

	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 影像业务处理模块右键
 */
void IMAGEPS::imageHandleBus_TreWContextSlot(const QPoint & pos)
{
	QTreeWidgetItem* treeItem = ui.treeWidget->itemAt(pos);

	if (!treeItem)
		return;

	QMenu* menu = new QMenu;

	QAction* expandAllAction = menu->addAction(QString::fromLocal8Bit("展开"));
	connect(expandAllAction, &QAction::triggered, this, [=]
	{
		ui.treeWidget->expandAll();
	});

	QAction* collapseAllAction = menu->addAction(QString::fromLocal8Bit("折叠"));
	connect(collapseAllAction, &QAction::triggered, this, [=]
	{
		ui.treeWidget->collapseAll();
	});

	if (treeItem->parent() && !treeItem->text(0).contains(QString::fromLocal8Bit("业务数据组织")))
	{
		QAction* configSetAction = menu->addAction(QString::fromLocal8Bit("参数设置"));
		connect(configSetAction, &QAction::triggered, this, [=]
		{
			// 存储原始按钮状态 
			//static QString originalOkText;

			// 1. 获取按钮并修改状态 
			QPushButton* okButton = systemConfig->findChild<QPushButton*>("okButton");
			QPushButton* cancelButton = systemConfig->findChild<QPushButton*>("cancelButton");
			QPushButton* applyButton = systemConfig->findChild<QPushButton*>("applyButton");

			if (okButton && applyButton) {
				// 保存原始状态 
				originalOkText = okButton->text();

				// 修改按钮状态
				okButton->setText(QString::fromLocal8Bit("保存"));
				applyButton->setVisible(false);
			}
			//systemConfig->settingShowWidget(treeItem->text(0));
			// 使用封装后的函数显示界面并等待用户确认 
			int result = systemConfig->showFunctionAndWait(treeItem->text(0));

			if (okButton && applyButton) {
				okButton->setText(originalOkText);
				applyButton->setVisible(true);
			}

			// 用户取消操作
			if (result != QDialog::Accepted) {
				
				return;
			}
		});
	}
	
	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}

/**
 * @brief 影像业务处理模块功能
 */
void IMAGEPS::imageHandleBus_TreWSlot(QTreeWidgetItem *treeItem, int column)
{
	if (!treeItem || !treeItem->parent())
		return;

	int index1 = DataimageListMap.value(treeItem->text(0));
	for (int i = 0; i < ui.tabWidget_2->count(); ++i) {

		QWidget* page = ui.tabWidget_2->widget(index1);
		if (ui.tabWidget_2->isTabEnabled(index1) && page->isVisible() &&
			(ui.tabWidget_2->currentIndex() == index1))
		{
			if (index1 >= 2) { 
				setView(ui.dockWidget_2, ui.tabWidget_2, index1);
				ui.tabWidget_2->setTabEnabled(index1, false);
				ui.tabWidget_2->setStyleSheet(
					"QTabBar::tab:disabled { width: 0; color: transparent; }"
					"QTabBar::scroller { width: 0; }"
					"QTabBar::tab:selected {background: #00a99d; color: white;border - bottom: 2px solid #008080;}"
				);
				ui.tabWidget_2->setCurrentIndex(1);
				return;
			}
		}
	}

	if (DataimageListMap.contains(treeItem->text(0))) {
		int index = DataimageListMap.value(treeItem->text(0));
		if (index >= 0 && index < ui.tabWidget_2->count()) {
			setView(ui.dockWidget_2, ui.tabWidget_2, index);
			ui.tabWidget_2->setTabEnabled(index, true);
			ui.tabWidget_2->tabBar()->setStyleSheet("");
			QWidget *page = ui.tabWidget_2->widget(index);
			page->setVisible(true);
			page->show();
			ui.tabWidget_2->setCurrentIndex(index);
			ui.tabWidget_2->tabBar()->adjustSize();
			ui.tabWidget_2->repaint();
			QApplication::processEvents();
		}
		return;
	}
	if (!showConfirmationDialog(treeItem->text(0)))
		return;
	if (!OrthoRectificationFusionflag)
	{
		if (treeItem->text(0) == QString::fromLocal8Bit("金字塔创建")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);
			systemConfig->acceptModule(treeItem->text(0), CreatepyramidPath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("影像融合")){
			if (ui.fusionmodelList_TableW->rowCount() == 0)
			{
				QMessageBox::warning(this, u8"提示", u8"没有可用的匹配模型，请先添加数据！");
				return;
			}
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);
			systemConfig->imageInterAction();
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("配准融合")){
			if (ui.fusionmodelList_TableW_2->rowCount() == 0)
			{
				QMessageBox::warning(this, u8"提示", u8"没有可用的匹配模型，请先添加数据！");
				return;
			}
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);
			systemConfig->imageInterAction();


			QObject::disconnect(systemConfig, &SystemConfig::imageInterActionFinished, nullptr, nullptr);

			// 2. 建立新连接 
			QObject::connect(systemConfig, &SystemConfig::imageInterActionFinished, this, [&]() {

				int rowCount = ui.fusionmodelList_TableW_2->rowCount();

				for (int row = 0; row < rowCount; ++row) {
					ui.fusionmodelList_TableW_2->item(row, 4)->setText(QString::fromLocal8Bit("是"));
				}
			});
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("真彩色转换")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);
			systemConfig->TrueColorConversion(ImageInterFilePath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("影像纠正")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			// 从projectdir + "RegisteOrtho/Ortho"目录下读取文件作为OrtholoutPath 
			QString orthoDir = projectdir + "RegisteOrtho/Ortho" + "/";
			QDir orthoDirectory(orthoDir);
			QStringList orthoFiles = orthoDirectory.entryList(QDir::Files | QDir::Readable);
			QSet<QString> orthoFileNames;
			for (const auto& file : orthoFiles) {
				QFileInfo fileInfo(file);
				orthoFileNames.insert(fileInfo.completeBaseName());
			}

			// 使用临时变量保存最终要处理的路径 
			QStringList finalAlignmentPaths = AlignmentAdFilePath;
			QStringList duplicateFiles;

			// 检查重复文件 
			for (const auto& path : finalAlignmentPaths) {
				QFileInfo fileInfo(path);
				if (orthoFileNames.contains(fileInfo.completeBaseName())) {
					duplicateFiles.append(path);
				}
			}

			// 如果有重复文件 
			if (!duplicateFiles.isEmpty()) {
				int num = duplicateFiles.size();
				QString message = QString::fromLocal8Bit("已经存在%1个纠正成果，是否重新纠正？").arg(num);

				QMessageBox::StandardButton reply;
				reply = QMessageBox::question(this, QString::fromLocal8Bit("纠正成果确认"),
					message,
					QMessageBox::Yes | QMessageBox::No);

				if (reply == QMessageBox::No) {
					// 如果所有文件都重复且用户选择不重新处理，则直接退出 
					if (duplicateFiles.size() == AlignmentAdFilePath.size()) {
						ui.runLog_Edit->append(QString::fromLocal8Bit(" 用户取消重新纠正已存在的成果"));
						return;
					}

					// 否则只移除重复文件 
					for (const auto& path : duplicateFiles) {
						finalAlignmentPaths.removeAll(path);
					}
				}
			}

			// 如果没有需要处理的文件，直接返回 
			if (finalAlignmentPaths.isEmpty()) {
				ui.runLog_Edit->append(QString::fromLocal8Bit(" 没有需要处理的影像文件"));
				return;
			}

			bool allSuccess = true;

			for (auto data : finalAlignmentPaths) {
				QFileInfo fileInfo(data);
				QString destDir = fileInfo.absolutePath() + "/";
				//QString sourceDir = fileInfo.absolutePath() + "/";
	
				QString sourceDir = projectdir + "RegisteOrtho/Register/";

				QDir sourceDirectory(sourceDir);
				if (!sourceDirectory.exists()) {
					qWarning() << "Source directory does not exist:" << sourceDir;
					return;
				}

				QDir destDirectory(destDir);
				if (!destDirectory.exists()) {
					if (!destDirectory.mkpath(".")) {
						qWarning() << "Failed to create destination directory:" << destDir;
						return;
					}
				}

				QStringList filters;
				filters << "*_rpc.txt";
				QStringList files = sourceDirectory.entryList(filters, QDir::Files | QDir::Readable);
				if (files.isEmpty()) {
					qWarning() << "No *_rpc.txt  files found in source directory";
					//return;
				}

				foreach(const QString &file, files) {
					QFileInfo sourceFileInfo(sourceDirectory, file);
					QString sourcePath = sourceFileInfo.absoluteFilePath();
					QString baseName = file;
					baseName.chop(4);
					QString destFileName = baseName + ".ba";
					QString destPath = destDirectory.filePath(destFileName);

					if (QFile::exists(destPath)) {
						if (!QFile::remove(destPath)) {
							qWarning() << "Failed to remove existing file:" << destPath;
							allSuccess = false;
							continue;
						}
					}
					if (!QFile::copy(sourcePath, destPath)) {
						qWarning() << "Failed to copy file from" << sourcePath << "to" << destPath;
						allSuccess = false;
					}
				}
			}

			// 使用临时变量传入 
			systemConfig->AlignmentAdjustment(DEMFilePath, finalAlignmentPaths);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("影像匀色")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			QStringList filenamelist = QStringList();
			for (const QFileInfo &fileInfo : DodgingFilePath) {
				//QString Type = systemConfig->getTiffImageType(fileInfo.absoluteFilePath());
				//if (Type == "MSS")
				filenamelist.append(fileInfo.absoluteFilePath());
			}

			systemConfig->ImageColorCorrection(filenamelist);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("影像裁切")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			QString OutputDir = projectdir + "Mosaic" + "/";
			if (VectorfileFilePath.empty())
			{
				ui.runLog_Edit->append(QString::fromLocal8Bit(" 裁切矢量文件为空"));
				return;
			}
			systemConfig->ImageCropping(ImagecropFilePath, VectorfileFilePath, OutputDir);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("影像镶嵌")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			QStringList filenamelist = QStringList();
			for (const QFileInfo &fileInfo : ImageMosaicFilePath) {
				QString Type = systemConfig->getTiffImageType(fileInfo.absoluteFilePath());
				filenamelist.append(fileInfo.absoluteFilePath());
			}
			systemConfig->ImageMosaic(filenamelist);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("智能镶嵌")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			SmartMosaicFilePath = syncFileOrderWithTable(SmartMosaicFilePath,
				ui.SmartMosaicDataList_TableW);
			systemConfig->SmartMosaic(SmartMosaicFilePath, MosaicCropFilePath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("快速镶嵌")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			systemConfig->QuickMosaic(QuickMosaicFilePath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("纠正模型配准")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);
			systemConfig->modelMatchByInter(DEMFilePath, DOMFilePath, AlignmentAdFilePath, modelMatchByInterPath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("融合模型配准")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			// 使用状态机模式避免递归
			enum ProcessingStep { STEP_DEM, STEP_DOM, STEP_FINISH };
			static ProcessingStep currentStep = STEP_DEM; // 使用static保持状态

			// 使用成员变量而不是局部捕获 
			auto* config = this->systemConfig; // 获取成员变量 

			QObject::connect(config, &SystemConfig::intersectCmdFinished, this,
				[this, config]() mutable { // 注意mutable允许修改捕获的变量 
				if (!config) return;

				switch (currentStep) {
				case STEP_DEM:
					currentStep = STEP_DOM;
					config->IntersectCmd(this->DOMFilePath,
						this->AlignmentIntFilePath,
						QString::fromLocal8Bit("DOM"),
						QString::fromLocal8Bit("Fusion"),
						true);
					break;

				case STEP_DOM:
					currentStep = STEP_FINISH;
					// 更新UI
					int rowCount = ui.fusionmodelList_TableW_2->rowCount();
					for (int row = 0; row < rowCount; ++row) {
						ui.fusionmodelList_TableW_2->item(row, 4)->setText(QString::fromLocal8Bit("是"));
					}
					// 断开连接
					QObject::disconnect(config, &SystemConfig::intersectCmdFinished, this, nullptr);
					break;
				}
			});

			// 启动第一步
			currentStep = STEP_DEM; // 重置状态 
			config->IntersectCmd(this->DEMFilePath,
				this->AlignmentIntFilePath,
				QString::fromLocal8Bit("DEM"),
				QString::fromLocal8Bit("Fusion"),
				true);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("绝对定位精度质检")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			// 先断开之前的连接（避免重复调用）
			disconnect(systemConfig, &SystemConfig::ImageAbsPositPrecCheckFinished,
				this, qOverload<>(&IMAGEPS::AbsPositPrecCheckPList));
			// 重新连接信号
			connect(systemConfig, &SystemConfig::ImageAbsPositPrecCheckFinished,
				this, qOverload<>(&IMAGEPS::AbsPositPrecCheckPList));

			systemConfig->ImageAbsPositPrecCheck(DEMFilePath, DOMFilePath, AbsPositPrecCheckFilePath, AbsPositPrecCheckPathPath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("生成缩略影像")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			systemConfig->ThumbnailGeneration(CloudDetectionDataListFilePath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("影像云检")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			// 先断开之前的连接（避免重复调用）
			disconnect(systemConfig, &SystemConfig::CloudDetectionFinished,
				this, qOverload<>(&IMAGEPS::CloudDetectionList));
			// 重新连接信号
			connect(systemConfig, &SystemConfig::CloudDetectionFinished,
				this, qOverload<>(&IMAGEPS::CloudDetectionList));

			systemConfig->CloudDetection();
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("投影转换")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			systemConfig->ProjectionTransformation(ProTransformationFilePath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("格式转换")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			systemConfig->FormatConversion(FormatConversionFilePath);
		}
		else if (treeItem->text(0) == QString::fromLocal8Bit("SAR影像滤波")){
			systemConfig->logEdit = ui.runLog_Edit;
			ui.log_TabW->setCurrentIndex(0);

			systemConfig->SARImageFiltering(SARImageFilteringFilePath);
		}
	}
	else
	{
		QMessageBox::warning(this, "Warning", u8"***已有任务正在进行,请稍后再试!!!");
	}
}

void IMAGEPS::pointsInfoList_DockWContextSlot(const QPoint & pos)
{
	QMenu* menu = new QMenu;
	menu->addAction(QString::fromLocal8Bit("删除"));
	menu->addAction(QString::fromLocal8Bit("复制"));
	menu->addAction(QString::fromLocal8Bit("立体查看"));
	menu->exec(cursor().pos());

	foreach(QAction* pAction, menu->actions()) delete pAction;
	delete menu;
}
///////////////////////////////////
// 1. 加载原始影像(文件)
void IMAGEPS::loadOriginalImagesFromFiles(QTableWidget* tableWidget, QStringList& filePathList)
{
	// 支持多选文件的文件对话框 
	QStringList originImagePaths = QFileDialog::getOpenFileNames(
		this,
		QString::fromLocal8Bit("选择加载原始影像文件"),
		m_lastPath,  // 使用上次路径 
		tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;PIX Image(*.pix);;DigitalGlobe image Files(*til);;All Files(*)")
		.arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)"))
	);

	if (originImagePaths.isEmpty()) {
		return;
	}

	// 更新最后选择的路径 
	QFileInfo firstFileInfo(originImagePaths.first());
	m_lastPath = firstFileInfo.absolutePath();
	QStringList finalImagePaths = filterImageFiles(originImagePaths);

	bool hasNewFiles = false;
	for (const QString &originImagePath : finalImagePaths) {
		QFileInfo fileInfo(originImagePath);

		// 检查是否已存在相同文件名的记录 
		bool alreadyExists = checkFileExistsInTable(tableWidget, fileInfo.fileName());

		if (!alreadyExists) {
			hasNewFiles = true;
			filePathList.push_back(originImagePath);

			// 插入新行并设置内容 
			addFileToTable(tableWidget, fileInfo);
		}
	}

	if (hasNewFiles) {
		setGeoreferenceFromXMLAndRPC(filePathList);
	}
}

// 2. 加载原始影像(文件夹)
void IMAGEPS::loadOriginalImagesFromFolder(QTableWidget* tableWidget, QStringList& filePathList)
{
	QString folderPath = QFileDialog::getExistingDirectory(
		this,
		QString::fromLocal8Bit("选择加载原始影像(文件夹)"),
		m_lastPath,  // 使用上次路径 
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
	);

	if (folderPath.isEmpty()) {
		return; // 用户取消选择 
	}

	// 更新最后选择的路径 
	m_lastPath = folderPath;

	// 设置文件过滤器 
	QStringList filters;
	filters << "*.tif" << "*.tiff" << "*.img" << "*.til" << "*.pix";

	// 使用 QDirIterator 递归遍历所有子目录 
	QDirIterator dirIterator(
		folderPath,
		filters,
		QDir::Files | QDir::NoDotAndDotDot,
		QDirIterator::Subdirectories // 递归遍历子目录 
	);

	QStringList filePaths;
	while (dirIterator.hasNext()) {
		filePaths.append(dirIterator.next());
	}
	QStringList finalImagePaths = filterImageFiles(filePaths);

	bool hasNewFiles = false;
	for (const QString &originImagePath : finalImagePaths) {
		QFileInfo fileInfo(originImagePath);

		// 检查是否已存在相同文件名的记录
		bool alreadyExists = checkFileExistsInTable(tableWidget, fileInfo.fileName());

		if (!alreadyExists) {
			hasNewFiles = true;
			filePathList.push_back(originImagePath);

			// 插入新行并设置内容 
			addFileToTable(tableWidget, fileInfo);
		}
	}

	if (hasNewFiles) {
		setGeoreferenceFromXMLAndRPC(filePathList);
	}
}

// 3. 卸载影像 
void IMAGEPS::unloadImages(QTableWidget* tableWidget, QStringList& filePathList)
{
	QStringList firstColumnValues;
	// 获取选中的所有行（避免重复）
	QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
	if (selectedItems.isEmpty()) {
		return; // 如果没有选中任何行，不显示菜单 
	}

	// 提取所有选中的行号（避免重复）
	QSet<int> rowsToDelete;
	for (QTableWidgetItem* item : selectedItems) {
		rowsToDelete.insert(item->row());

		QTableWidgetItem* firstColumnItem = tableWidget->item(item->row(), 1);
		if (firstColumnItem) {
			firstColumnValues.append(firstColumnItem->text());   // 存储第一列的值
		}
	}

	// 按从大到小的顺序删除（防止索引错乱）
	QList<int> sortedRows = rowsToDelete.values();
	std::sort(sortedRows.begin(), sortedRows.end(), std::greater<int>());

	// 删除所有选中的行 
	for (int row : sortedRows) {
		tableWidget->removeRow(row);
	}

	// 从文件路径列表中移除对应的项 
	for (auto filename : firstColumnValues) {
		for (auto filenamePATH : filePathList) {
			QFileInfo fileInfo(filenamePATH);
			if (fileInfo.completeBaseName() == filename) {
				filePathList.removeAll(filenamePATH);
			}
		}
	}
}

// 4. 查看影像
void IMAGEPS::viewImages(QTableWidget* tableWidget, const QStringList& filePathList)
{
	QStringList firstColumnValues;
	// 获取选中的所有行（避免重复）
	QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
	if (selectedItems.isEmpty()) {
		return; // 如果没有选中任何行，不显示菜单
	}

	// 提取所有选中的行号（避免重复）
	QSet<int> rows;
	for (QTableWidgetItem* item : selectedItems) {
		rows.insert(item->row());

		QTableWidgetItem* firstColumnItem = tableWidget->item(item->row(), 1);
		if (firstColumnItem) {
			firstColumnValues.append(firstColumnItem->text());   // 存储第一列的值 
		}
	}
	displayView(ui.dockWidget_9, ui.tabWidget, 1);
	for (auto filename : firstColumnValues) {
		for (auto filenamePATH : filePathList) {
			QFileInfo fileInfo(filenamePATH);
			if (fileInfo.completeBaseName() == filename) {
				on_actionOpenImageShow(filenamePATH);
				break;
			}
		}
	}
}

// 辅助函数：检查文件是否已存在于表格中
bool IMAGEPS::checkFileExistsInTable(QTableWidget* tableWidget, const QString& fileName)
{
	for (int i = 0; i < tableWidget->rowCount(); i++) {
		if (fileName.contains(tableWidget->item(i, 1)->text())) {
			return true;
		}
	}
	return false;
}

// 辅助函数：添加文件到表格中 
void IMAGEPS::addFileToTable(QTableWidget* tableWidget, const QFileInfo& fileInfo)
{
	// 插入新行 
	int newRowIndex = tableWidget->rowCount();
	tableWidget->insertRow(newRowIndex);

	// 确保所有单元格都有 QTableWidgetItem 
	for (int col = 0; col < tableWidget->columnCount(); col++) {
		if (!tableWidget->item(newRowIndex, col)) {
			QTableWidgetItem* item = new QTableWidgetItem;
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			tableWidget->setItem(newRowIndex, col, item);
		}
	}

	// 设置序号（当前行号 + 1）
	tableWidget->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

	// 设置文件名（去掉扩展名）
	tableWidget->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

	// 检查表格是否有第4列（即列数 >= 4）
	bool hasFourthColumn = (tableWidget->columnCount() >= 4);

	// 如果有第4列，则给第3列添加数据
	if (hasFourthColumn) {
		QString panPart = systemConfig->getTiffImageType(fileInfo.filePath());

		tableWidget->item(newRowIndex, 2)->setText(panPart.left(3));

	}
	tableWidget->resizeColumnsToContents();

}

// 星源类型获取函数
QString IMAGEPS::getImagetype(const QString& filePath)
{
	// 获取文件基本信息
	QFileInfo fileInfo(filePath);
	//QString tmptype = fileInfo.completeBaseName().left(3);、
	QString tmptype = fileInfo.completeBaseName().section('_', 0, 0);

	// 查找同目录下的.RPB或.rpb文件（区分大小写）
	QString rpbFilePath;
	bool rpbFileFound = false;

	// 获取目录下所有文件
	QDir dir(fileInfo.path());
	QStringList files = dir.entryList(QDir::Files);

	// 构建我们需要的文件名（不含扩展名）
	QString baseName = fileInfo.completeBaseName();

	// 检查每个文件是否匹配
	for (const QString &file : files) {
		QFileInfo fi(dir.filePath(file));

		// 检查文件名主体是否匹配且扩展名是.RPB或.rpb
		if (fi.completeBaseName().compare(baseName, Qt::CaseSensitive) == 0) {
			if (fi.suffix().compare("RPB", Qt::CaseSensitive) == 0 ||
				fi.suffix().compare("rpb", Qt::CaseSensitive) == 0) {
				rpbFilePath = fi.filePath();
				rpbFileFound = true;
				break;
			}
		}
	}

	// 如果找到.RPB文件，尝试读取satId字段
	if (rpbFileFound) {
		QFile rpbFile(rpbFilePath);
		if (rpbFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&rpbFile);
			QString line;
			bool satIdFound = false;

			// 逐行读取文件查找satId 
			while (!in.atEnd() && !satIdFound) {
				line = in.readLine().trimmed();

				// 检查是否是satId行 
				if (line.startsWith("satId")) {
					// 提取引号中的内容
					int start = line.indexOf('"');
					int end = line.lastIndexOf('"');
					if (start != -1 && end != -1 && start < end) {
						QString tmp = line.mid(start + 1, end - start - 1);
						if (tmp == "XXX") {
							break;
						}
						else {
							tmptype = line.mid(start + 1, end - start - 1);
						}
						satIdFound = true;
					}
				}
			}
			rpbFile.close();
		}
	}

	// 处理DP0特殊情况
	if (tmptype == "DP0") {
		tmptype = "JL";
	}

	return tmptype;
}

// 过滤函数（使用配置文件）
QStringList IMAGEPS::filterImageFiles(const QStringList& filePaths, QString title)
{
	// 存储最终结果
	QStringList finalImagePaths;

	if (!title.isEmpty()) {
		// 首先检查所有文件是否有对应的RPC文件 
		bool hasAtLeastOneRpc = false;
		for (const QString &filePath : filePaths) {
			QFileInfo fileInfo(filePath);
			QString suffix = fileInfo.suffix().toLower();
			QString dirPath = fileInfo.absolutePath();
			QString baseName = fileInfo.completeBaseName();

			bool isCloudFile = false;
			for (const QString& cloudExt : m_cldFileExtensions) {
				if (baseName.contains(cloudExt, Qt::CaseInsensitive)) {
					isCloudFile = true;
					break;
				}
			}

			if (isCloudFile) continue; // 跳过云标记文件

			// 检查当前文件是否有对应的RPC文件（从配置读取）
			bool hasRpc = false;
			for (const QString& rpcExt : m_rpcFileExtensions) {
				QString rpcFilePath = dirPath + "/" + baseName + rpcExt;
				if (QFile::exists(rpcFilePath)) {
					hasRpc = true;
					break;
				}
			}

			if (hasRpc) {
				hasAtLeastOneRpc = true;
				break;
			}
		}

		// 如果没有找到任何RPC文件，直接返回空列表
		if (!hasAtLeastOneRpc) {
			return finalImagePaths;
		}
	}

	// 按照目录分组处理文件 
	QMap<QString, QList<QFileInfo>> dirToFilesMap;

	// 1. 按目录分组所有文件 
	for (const QString &filePath : filePaths) {
		QFileInfo fileInfo(filePath);
		QString dirPath = fileInfo.absolutePath();
		dirToFilesMap[dirPath].append(fileInfo);
	}

	// 2. 处理每个目录下的文件 
	foreach(const QString &dirPath, dirToFilesMap.keys()) {
		QList<QFileInfo> filesInDir = dirToFilesMap.value(dirPath);
		QList<QFileInfo> validTilFiles; // 存储所有有效的.til文件

		// 2.1 找出所有带RPC的.til文件 
		foreach(const QFileInfo &fileInfo, filesInDir) {
			QString suffix = fileInfo.suffix().toLower();
			QString baseName = fileInfo.completeBaseName();
			
			bool isCloudFile = false;
			for (const QString& cloudExt : m_cldFileExtensions) {
				if (baseName.contains(cloudExt, Qt::CaseInsensitive)) {
					isCloudFile = true;
					break;
				}
			}
			if (isCloudFile) continue;

			if (suffix == "til") {
				//QString baseName = fileInfo.completeBaseName();

				// 检查是否存在RPC文件（从配置读取）
				bool hasRpc = false;
				for (const QString& rpcExt : m_rpcFileExtensions) {
					QString rpcFilePath = dirPath + "/" + baseName + rpcExt;
					if (QFile::exists(rpcFilePath)) {
						hasRpc = true;
						break;
					}
				}

				if (hasRpc) {
					validTilFiles.append(fileInfo);
				}
			}
		}

		// 2.2 处理带RPC的.til文件 
		if (!validTilFiles.isEmpty()) {
			QSet<QString> processedNames; // 记录已处理的文件名 

			// 处理每个有效的.til文件 
			foreach(const QFileInfo &tilFile, validTilFiles) {
				QString baseName = tilFile.completeBaseName();
				bool foundImage = false;

				// 查找同名的图像文件（从配置读取支持的格式）
				foreach(const QFileInfo &fileInfo, filesInDir) {
					QString suffix = fileInfo.suffix().toLower();
					QString completeBaseName = fileInfo.completeBaseName();
				 
					bool isCloudFile = false;
					for (const QString& cloudExt : m_cldFileExtensions) {
						if (completeBaseName.contains(cloudExt, Qt::CaseInsensitive)) {
							isCloudFile = true;
							break;
						}
					}
					if (isCloudFile) continue;

					// 检查是否为支持的图像格式且文件名匹配
					bool isSupportedImage = false;
					for (const QString& imageExt : m_imageFileExtensions) {
						// 移除扩展名前的点（如果有）
						QString cleanExt = imageExt.startsWith(".") ? imageExt.mid(1) : imageExt;
						if (suffix == cleanExt) {
							isSupportedImage = true;
							break;
						}
					}

					if (isSupportedImage && completeBaseName == baseName) {
						finalImagePaths.append(fileInfo.absoluteFilePath());
						foundImage = true;
						processedNames.insert(baseName);
						break;
					}
				}

				// 如果没有找到同名图像文件，添加.til文件 
				if (!foundImage && !processedNames.contains(baseName)) {
					finalImagePaths.append(tilFile.absoluteFilePath());
					processedNames.insert(baseName);
				}
			}
		}
		// 2.3 如果没有有效的.til文件，添加所有支持的图像文件 
		else {
			foreach(const QFileInfo &fileInfo, filesInDir) {
				QString suffix = fileInfo.suffix().toLower();
				QString baseName = fileInfo.completeBaseName();

				bool isCloudFile = false;
				for (const QString& cloudExt : m_cldFileExtensions) {
					if (baseName.contains(cloudExt, Qt::CaseInsensitive)) {
						isCloudFile = true;
						break;
					}
				}
				if (isCloudFile) continue;

				// 检查是否为支持的图像格式 
				bool isSupportedImage = false;
				for (const QString& imageExt : m_imageFileExtensions) {
					QString cleanExt = imageExt.startsWith(".") ? imageExt.mid(1) : imageExt;
					if (suffix == cleanExt) {
						isSupportedImage = true;
						break;
					}
				}

				if (isSupportedImage) {
					finalImagePaths.append(fileInfo.absoluteFilePath());
				}
			}
		}
	}

	return finalImagePaths;
}

// 过滤函数
//QStringList IMAGEPS::filterImageFiles(const QStringList& filePaths, QString title)
//{
//	// 存储最终结果
//	QStringList finalImagePaths;
//
//	if (!title.isEmpty()) {
//		// 首先检查所有文件是否有对应的RPC文件
//		bool hasAtLeastOneRpc = false;
//		for (const QString &filePath : filePaths) {
//			QFileInfo fileInfo(filePath);
//			QString suffix = fileInfo.suffix().toLower();
//			QString dirPath = fileInfo.absolutePath();
//			QString baseName = fileInfo.completeBaseName();
//
//			// 检查当前文件是否有对应的RPC文件
//			if (QFile::exists(dirPath + "/" + baseName + ".rpb") ||
//				QFile::exists(dirPath + "/" + baseName + ".RPB") ||
//				QFile::exists(dirPath + "/" + baseName + "_rpc.txt") ||
//				QFile::exists(dirPath + "/" + baseName + ".rpc") ||
//				QFile::exists(dirPath + "/" + baseName + ".IMD")) {
//				hasAtLeastOneRpc = true;
//				break;
//			}
//		}
//
//		// 如果没有找到任何RPC文件，直接返回空列表
//		if (!hasAtLeastOneRpc) {
//			return finalImagePaths;
//		}
//	}
//
//	// 按照目录分组处理文件 
//	QMap<QString, QList<QFileInfo>> dirToFilesMap;
//
//	// 1. 按目录分组所有文件 
//	for (const QString &filePath : filePaths) {
//		QFileInfo fileInfo(filePath);
//		QString dirPath = fileInfo.absolutePath();
//		dirToFilesMap[dirPath].append(fileInfo);
//	}
//
//	// 2. 处理每个目录下的文件
//	foreach(const QString &dirPath, dirToFilesMap.keys()) {
//		QList<QFileInfo> filesInDir = dirToFilesMap.value(dirPath);
//		QList<QFileInfo> validTilFiles; // 存储所有有效的.til文件
//
//		// 2.1 找出所有带RPC的.til文件 
//		foreach(const QFileInfo &fileInfo, filesInDir) {
//			QString suffix = fileInfo.suffix().toLower();
//			if (suffix == "til") {
//				QString baseName = fileInfo.completeBaseName();
//
//				// 检查是否存在RPC文件 
//				bool hasRpc = QFile::exists(dirPath + "/" + baseName + ".rpb") ||
//					QFile::exists(dirPath + "/" + baseName + ".RPB") ||
//					QFile::exists(dirPath + "/" + baseName + "_rpc.txt") ||
//					QFile::exists(dirPath + "/" + baseName + ".rpc") ||
//					QFile::exists(dirPath + "/" + baseName + ".IMD");
//
//				if (hasRpc) {
//					validTilFiles.append(fileInfo);
//				}
//			}
//		}
//
//		// 2.2 处理带RPC的.til文件 
//		if (!validTilFiles.isEmpty()) {
//			QSet<QString> processedNames; // 记录已处理的文件名
//
//			// 处理每个有效的.til文件
//			foreach(const QFileInfo &tilFile, validTilFiles) {
//				QString baseName = tilFile.completeBaseName();
//				bool foundTif = false;
//
//				// 查找同名的.tif文件
//				foreach(const QFileInfo &fileInfo, filesInDir) {
//					QString suffix = fileInfo.suffix().toLower();
//					if ((suffix == "tif" || suffix == "tiff") &&
//						fileInfo.completeBaseName() == baseName) {
//						finalImagePaths.append(fileInfo.absoluteFilePath());
//						foundTif = true;
//						processedNames.insert(baseName);
//						break;
//					}
//				}
//
//				// 如果没有找到同名.tif文件，添加.til文件
//				if (!foundTif && !processedNames.contains(baseName)) {
//					finalImagePaths.append(tilFile.absoluteFilePath());
//					processedNames.insert(baseName);
//				}
//			}
//		}
//		// 2.3 如果没有有效的.til文件，添加所有.tif文件
//		else {
//			foreach(const QFileInfo &fileInfo, filesInDir) {
//				QString suffix = fileInfo.suffix().toLower();
//				if (suffix == "tif" || suffix == "tiff") {
//					finalImagePaths.append(fileInfo.absoluteFilePath());
//				}
//			}
//		}
//	}
//
//	return finalImagePaths;
//}

// 辅助函数：设置选中行指定列的值
void IMAGEPS::setSelectedRowsValue(int column, const QString& value)
{
	// 获取所有选中的行（去重）
	QSet<int> selectedRows;
	QList<QTableWidgetItem*> selectedItems = ui.sateImageDataList_TableW->selectedItems();

	for (QTableWidgetItem* item : selectedItems) {
		selectedRows.insert(item->row());
	}

	// 设置每一选中行的指定列值 
	foreach(int row, selectedRows) {
		QTableWidgetItem* item = ui.sateImageDataList_TableW->item(row, column);
		if (item) {
			item->setText(value);
		}
	}

	// 如果有选中行，刷新表格显示 
	if (!selectedRows.isEmpty()) {
		ui.sateImageDataList_TableW->viewport()->update();
	}
}

// 加载配置文件
void IMAGEPS::loadConfiguration()
{
	m_configFilePath = "../bin/config/projectPathConfig/loadFileTyprConfig.ini";
	QSettings settings(m_configFilePath, QSettings::IniFormat);

	// 设置默认值
	QStringList defaultRpcExtensions = { ".rpb", ".RPB", "_rpc.txt",  ".rpc", ".IMD" };
	QStringList defaultImageExtensions = { "tif", "tiff", "img", "pix"};
	QStringList defaultTilExtensions = { ".til", ".TIL" };
	QStringList defaultPyramidExtensions = { ".ovr", ".OVR", ".rrd", ".RRD" };
	QStringList defaultcldExtensions = { "_cld", "_CLD"};

	// 读取RPC文件扩展名配置 
	m_rpcFileExtensions = getConfigStringList("RPC", "Extensions", defaultRpcExtensions);

	// 读取图像文件扩展名配置 
	m_imageFileExtensions = getConfigStringList("Image", "Extensions", defaultImageExtensions);

	// 读取TIL文件扩展名配置
	m_tilFileExtensions = getConfigStringList("Images", "TilExtensions", defaultTilExtensions);

	// 读取金字塔文件扩展名配置 
	m_pyramidFileExtensions = getConfigStringList("Pyramid", "PyramidFiles", defaultPyramidExtensions);

	// 读取文件夹过滤字段配置 
	m_cldFileExtensions = getConfigStringList("Cloud", "CloudFiles", defaultcldExtensions);
}

// 保存配置文件
void IMAGEPS::saveConfiguration()
{
	QSettings settings(m_configFilePath, QSettings::IniFormat);

	// 保存RPC文件扩展名配置
	settings.beginGroup("RPC");
	settings.setValue("Extensions", m_rpcFileExtensions.join(","));
	settings.endGroup();

	// 保存图像文件扩展名配置
	settings.beginGroup("Image");
	settings.setValue("Extensions", m_imageFileExtensions.join(","));
	settings.endGroup();
}

// 辅助函数：从配置读取字符串列表
QStringList IMAGEPS::getConfigStringList(const QString& section, const QString& key, const QStringList& defaultValue)
{
	QSettings settings(m_configFilePath, QSettings::IniFormat);
	settings.beginGroup(section);

	QString value = settings.value(key, defaultValue.join(",")).toString();
	QStringList result = value.split(",", Qt::SkipEmptyParts);

	// 清理每个扩展名（去除前后空格，确保格式统一）
	for (QString& ext : result) {
		ext = ext.trimmed();
		if (section == "RPC" || section == "Images" || section == "Pyramid") {
			if (!ext.startsWith(".")) {
				ext = "." + ext;  // 确保扩展名以点开头 
			}
		}
	}

	settings.endGroup();
	return result.isEmpty() ? defaultValue : result;
}

/**
 * @brief 检查文件是否已存在金字塔文件
 * @param fileInfo 文件信息
 * @return 是否存在金字塔
 */
bool IMAGEPS::checkPyramidExists(const QFileInfo& fileInfo)
{
	QString baseName = fileInfo.fileName();
	QString suffix = fileInfo.suffix().toLower();
	QDir dir(fileInfo.absolutePath());

	// 1. 首先检查原文件的金字塔文件（使用配置文件中的扩展名）
	for (const QString& pyramidExt : m_pyramidFileExtensions) {
		QString pyramidFilePath = dir.absoluteFilePath(baseName + pyramidExt);
		if (QFile::exists(pyramidFilePath)) {
			return true;
		}
	}

	// 2. 如果原文件是支持的图像格式，检查同名.til文件的金字塔文件 
	bool isSupportedImage = false;
	for (const QString& imageExt : m_imageFileExtensions) {
		QString cleanExt = imageExt.startsWith(".") ? imageExt.mid(1) : imageExt;
		if (suffix == cleanExt) {
			isSupportedImage = true;
			break;
		}
	}

	if (isSupportedImage) {
		// 检查所有支持的TIL文件扩展名
		for (const QString& tilExt : m_tilFileExtensions) {
			QString tilFileName = fileInfo.completeBaseName() + tilExt;
			QString tilFilePath = dir.absoluteFilePath(tilFileName);

			// 如果TIL文件存在，检查其金字塔文件
			if (QFile::exists(tilFilePath)) {
				for (const QString& pyramidExt : m_pyramidFileExtensions) {
					QString tilPyramidPath = dir.absoluteFilePath(tilFileName + pyramidExt);
					if (QFile::exists(tilPyramidPath)) {
						return true;
					}
				}
			}
		}
	}

	return false;
}

///////////////////////////////////
/**
 * @brief 加密点列表数据写入
 */
void IMAGEPS::EncryptedPointsList()
{
	// 1. 获取文件路径模式 
	QStringList tieFilePattern;
	QStringList tpFilePattern;
	QString tiffBasePath = parentDir;

	QDateTime timestamp = PublicFunctions::readTimestampFromXml("SatTiePointMatch", this->projectdir);
	//if (!timestamp.isValid()) {
	//	return;
	//}

	QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/", timestamp);
	for (const QString& filePath : resultFiles) {
		QFileInfo fileInfo(filePath);
		QString suffix = fileInfo.suffix().toLower();
		QString fileName = fileInfo.fileName();

		if (fileName.endsWith(".tp", Qt::CaseInsensitive)) {
			tpFilePattern << filePath;
		}
		else if (fileName.endsWith(".tie", Qt::CaseInsensitive) &&
			!fileName.contains("_origin", Qt::CaseInsensitive)) {
			tieFilePattern << filePath;
		}
	}

	buildPSIntersectObjCmdFile();

	// 创建并启动进程 
	QProcess *process1 = new QProcess(this);
#ifdef Q_OS_LINUX 
	QString program = this->appDirPath + QString::fromLocal8Bit("/linux64/PSIntersectObjCmd.x");
#else 
	QString program = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSIntersectObjCmd.exe");
#endif 
	QStringList arguments;
	QString XMLpath = projectdir + "SatTiePointMatch" + "/" + "PSIntersectObjCmd.task";
	arguments << XMLpath;
	QString moniFile = projectdir.replace("\\", "/") + QString::fromLocal8Bit("SatTiePointMatch/PSIntersectObjCmd") + QString::fromLocal8Bit(".moni");

	// 启动进程 
	process1->start(program, arguments);
	m_childProcesses.append(process1);

	// 进程结束时的处理 
	connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		QFile file(moniFile);
		m_childProcesses.removeOne(process1);
		int maxWaitTime = 5000; // 5秒 
		int checkInterval = 200; // 0.2秒 
		int elapsed = 0;
		while (!file.exists() && elapsed < maxWaitTime) {
			QThread::msleep(checkInterval);
			elapsed += checkInterval;
			QCoreApplication::processEvents();
		}

		if (file.exists()) {
			//if (!tpFilePattern.isEmpty()) {
			//	QList<QString> orderedPointIds;
			//	ui.measureAreaShow_GLW->loadPoints(tpFilePattern, DataModelPath, "con", orderedPointIds);
			//	ui.measureAreaShow_GLW->showPoints(true);
			//}

			QString objFilePath = QFileInfo(moniFile).path() + "/obj.txt";
			QFile objFile(objFilePath);
			QHash<QString, QStringList> objDataMap;  // 存储ID到重叠度的映射 

			if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream in(&objFile);
				QString line = in.readLine();     // 第一行是数据行数（跳过）

				while (!in.atEnd()) {
					line = in.readLine().trimmed();
					if (line.isEmpty())    continue;

					QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
					if (parts.size() >= 5) {  // 确保有足够列 
						QString pointId = parts[0];
						QStringList IdData;
						IdData << parts[1] << parts[2] << parts[3] << parts[4];  // 第五列是重叠度 
						objDataMap.insert(pointId, IdData);
					}
				}
				objFile.close();
			}
			else {
				qWarning() << "无法打开obj.txt   文件:" << objFilePath;
			}

			QList<QStringList> allTieData;
			QHash<QString, QStringList> uniqueDataMap;
			QHash<QString, QString> keyToTiffMap; // 存储键到TIFF文件名的映射 

			int fileIndex = 0;
			for (auto resultfile : tieFilePattern)
			{
				if (fileIndex >= tpFilePattern.size() || fileIndex >= tieFilePattern.size())
					break;

				QString tpFilePath = tpFilePattern.at(fileIndex);

				if (!QFile::exists(tpFilePath)) {
					if (fileIndex == 0) {
						QMessageBox::critical(this, u8"错误", u8"找不到.tie或.tp文件");
						return;
					}
					break;
				}

				// 读取.tp文件获取TIFF文件名映射 
				QFile tpFile(tpFilePath);
				if (tpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream tpIn(&tpFile);
					while (!tpIn.atEnd()) {
						QString line = tpIn.readLine().trimmed();
						if (line.isEmpty())   continue;

						QStringList fields = line.split("  ", Qt::SkipEmptyParts);
						if (fields.size() >= 2) {
							QString key = fields[0];
							QString tiffName = fields[1];
							keyToTiffMap.insert(key, tiffName);
						}
					}
					tpFile.close();
				}

				fileIndex++;
			}

			// 清空原有数据 
			m_encryptedPoints.clear();

			// 3. 填充数据结构 
			QStringList pointIds = objDataMap.keys();
			std::sort(pointIds.begin(), pointIds.end(), [](const QString &a, const QString &b) {
				return a.toInt() < b.toInt();
			});

			//// 填充加密点数据结构 
			//for (const QString &pointId : pointIds) {
			//	EncryptedPoint point;
			//	point.id = pointId;

			//	const QStringList &dataList = objDataMap.value(pointId);
			//	point.residual = "";
			//	point.state = "Y";

			//	if (dataList.size() >= 4) {
			//		point.overlap = dataList[3].toInt();
			//		point.pointX = dataList[0];
			//		point.pointY = dataList[1];
			//		point.pointZ = dataList[2];
			//	}
			//	else {
			//		// 处理不足3个元素的情况
			//		point.overlap = 0;
			//		point.pointX = "0";
			//		point.pointY = "0";
			//		point.pointZ = "0";
			//	}


			//	// 从tp文件获取卫星影像信息 
			//	for (const QString& tpFilePath : tpFilePattern) {
			//		QFile tpFile(tpFilePath);
			//		if (tpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//			QTextStream tpIn(&tpFile);
			//			while (!tpIn.atEnd()) {
			//				QString line = tpIn.readLine().trimmed();
			//				if (line.isEmpty())  continue;

			//				QStringList fields = line.split(" ", Qt::SkipEmptyParts);
			//				if (fields.size() >= 4 && fields[0] == pointId) {
			//					SatImageData satImage;
			//					satImage.path = fields[1];
			//					satImage.imageX = fields[2].toDouble();
			//					satImage.imageY = fields[3].toDouble();
			//					point.satImages.append(satImage);
			//				}
			//			}
			//			tpFile.close();
			//		}
			//	}

			//	m_encryptedPoints.insert(pointId, point);
			//}


			QHash<QString, QList<SatImageData>> tpDataHash;

			// 一次性读取所有TP文件 
			for (const QString& tpFilePath : tpFilePattern) {
				QFile tpFile(tpFilePath);
				if (tpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream tpIn(&tpFile);
					QString line;

					while (tpIn.readLineInto(&line)) {
						line = line.trimmed();
						if (line.isEmpty())  continue;

						QStringList fields = line.split(" ", Qt::SkipEmptyParts);
						if (fields.size() >= 4) {
							QString pointId = fields[0];

							SatImageData satImage;
							satImage.path = fields[1];
							satImage.imageX = fields[2].toDouble();
							satImage.imageY = fields[3].toDouble();

							// 使用哈希表提高查找效率 
							if (!tpDataHash.contains(pointId)) {
								tpDataHash.insert(pointId, QList<SatImageData>());
								tpDataHash[pointId].append(satImage);
							}
						}
					}
				}
				tpFile.close();
			}

			// 第二步：批量创建加密点对象 
			QHash<QString, EncryptedPoint> tempPoints;
			tempPoints.reserve(pointIds.size());  // 预分配内存 

			// 批量处理所有点ID 
			for (const QString &pointId : pointIds) {
				EncryptedPoint point;
				point.id = pointId;
				point.residual = "";
				point.state = "Y";

				// 批量赋值坐标数据 
				const QStringList &dataList = objDataMap.value(pointId);

				point.overlap = (dataList.size() >= 4) ? dataList[3].toInt() : 0;

				// 批量设置坐标值 
				if (dataList.size() >= 4) {
					point.pointX = dataList[0];
					point.pointY = dataList[1];
					point.pointZ = dataList[2];

					// 从预加载的哈希表中获取卫星影像数据 
					point.satImages = tpDataHash.value(pointId);

					tempPoints.insert(pointId, point);
				}
			}

			// 第三步：批量替换原有数据 
			m_encryptedPoints = tempPoints;

			if (!tpFilePattern.isEmpty()) {
				QList<QString> orderedPointIds;
				ui.ImageInfoShow_Widget->loadPoints(m_encryptedPoints, QHash<QString, ControlPoint>(), "con");
				ui.ImageInfoShow_Widget->showPoints(true);
			}

			// 初始化懒加载 
			m_currentPage = 0;
			updateVisiblePoints();
		}
	});

	Con_ColtrolPointsList();
}

/**
 * @brief 控制点列表数据写入
 */
void IMAGEPS::ControlPointsList()
{
	if (systemConfig->ui.checkBox_44->isChecked()) {
		// 清空原有数据
		m_controlPoints.clear();
		m_visibleControlPointIds.clear();
		m_currentControlPointPage = 0;

		m_controlPoints = m_controlPoints_conBack;
		m_visibleControlPointIds = m_visibleControlPointIds_conBack;
	}else {
		// 清空原有数据
		m_controlPoints.clear();
		m_visibleControlPointIds.clear();
		m_currentControlPointPage = 0;
	}

	// 初始化ID计数器 
	int baseId = 0;

	// 存储所有数据，以便后续查询 
	QMap<QString, QList<QStringList>> allTieData;
	QMap<QString, QList<QStringList>> pointCoordinates;

	// 获取控制点文件路径 
	QMap<QString, bool> tmpcontrolPointsPath;
	QStringList tmpcolPath;
	if (!OrthoRectificationFusionflag) {
		tmpcontrolPointsPath = controlPointsPath;
	}
	else {
		tmpcontrolPointsPath = this->controlPointsPath;
	}

	// 第一遍遍历：收集所有数据并计算重叠度 
	for (auto data = tmpcontrolPointsPath.begin(); data != tmpcontrolPointsPath.end(); ++data) {
		QString txtFilePath = data.key();
		QFile txtFile(txtFilePath);
		if (!txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << txtFilePath << QString::fromLocal8Bit("无法打开.txt文件");
			continue;
		}

		tmpcolPath.push_back(data.key());
		QTextStream tieIn(&txtFile);
		bool isFirstLine = true;
		int globalPointIdnum = 0;

		while (!tieIn.atEnd()) {
			QString line = tieIn.readLine().trimmed();
			if (line.isEmpty())  continue;

			// 跳过第一行（点的数量）
			if (isFirstLine) {
				isFirstLine = false;
				continue;
			}

			QStringList fields = line.split(" ", Qt::SkipEmptyParts);
			if (fields.size() < 8) continue;

			// 修改点ID：添加baseId前缀确保全局唯一 
			QString originalPointId = fields[0];
			//QString globalPointId = QString::number(baseId + originalPointId.toInt());
			QString globalPointId = QString::number(baseId + globalPointIdnum);
			globalPointIdnum++;
			// 提取文件名 
			int lastUnderscorePos = QFileInfo(txtFilePath).fileName().lastIndexOf('_',
				QFileInfo(txtFilePath).fileName().lastIndexOf('@') - 1);
			QString fileName = QFileInfo(txtFilePath).fileName().left(lastUnderscorePos);

			// 保存完整8列数据到全局map中 
			allTieData[globalPointId].append(fields);

			// 保存文件名和坐标数据 (fileName, x, y, z)
			QStringList coordData;
			coordData << fileName << fields[1] << fields[2] <<
				(fields.size() > 7 ? fields[3] : "0") << fields[7] << fields[8];
			pointCoordinates[globalPointId].append(coordData);

			// 添加到控制点数据结构
			if (!m_controlPoints.contains(globalPointId)) {
				ControlPoint point;
				point.id = globalPointId;
				point.type = QString::fromLocal8Bit("控制点");
				point.state = "Y";
				point.priority = "1";

				// 设置坐标 
				point.pointX = fields[1];
				point.pointY = fields[2];
				point.pointZ = fields.size() > 3 ? fields[3] : "";

				m_controlPoints.insert(globalPointId, point);
			}

			// 添加卫星影像数据 
			SatImageData satImage;
			satImage.path = fileName;
			satImage.imageX = fields[7].toDouble();
			satImage.imageY = fields[8].toDouble();
			m_controlPoints[globalPointId].satImages.append(satImage);
			//qDebug() << globalPointId << " " << fields[1] << " " << fields[2] << " " << fields[3];
			// 添加到按文件顺序的点ID列表
			if (!this->colPointIds_out.contains(
				globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3])) {
				this->colPointIds_out.append(
					globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3]);
			}
		}
		txtFile.close();
		baseId += 100000;
	}

	// 计算重叠度 
	for (auto it = m_controlPoints.begin(); it != m_controlPoints.end(); ++it) {
		QString pointId = it.key();
		int overlap = 0;
		if (pointCoordinates.contains(pointId)) {
			overlap = pointCoordinates[pointId].size();
		}
		m_controlPoints[pointId].overlap = overlap;
	}

	// 初始化可见点ID列表
	m_visibleControlPointIds = m_controlPoints.keys();
	std::sort(m_visibleControlPointIds.begin(), m_visibleControlPointIds.end(),
		[](const QString &a, const QString &b) {
		return a.toInt() < b.toInt();
	});

	// 加载第一页数据 
	loadControlPointCurrentPage();

	//// 连接表格点击事件
	//connect(ui.controlPointsLIst_TableW->selectionModel(), &QItemSelectionModel::selectionChanged,
	//	this, &IMAGEPS::onControlPointSelected);

	////// 加载点数据到显示窗口 
	//this->colPointIds = m_visibleControlPointIds;
	//if (!tmpcontrolPointsPath.isEmpty()) {
	//	ui.ImageInfoShow_Widget->loadPoints(tmpcolPath, DataModelPath, "col", m_visibleControlPointIds);
	//	ui.ImageInfoShow_Widget->showPoints(true);
	//}

	if (!tmpcontrolPointsPath.isEmpty()) {
		ui.ImageInfoShow_Widget->loadPoints(QHash<QString, EncryptedPoint>(), m_controlPoints, "col");
		ui.ImageInfoShow_Widget->showPoints(true);
	}

	// 更新数量显示 
	//ui.controlPointsnum->setText(QString::fromLocal8Bit("  控制点列表数量: %1").arg(m_controlPoints.size()));
}

void IMAGEPS::Con_ColtrolPointsList()
{
	// 清空原有数据 
	m_controlPoints.clear();
	m_controlPoints_conBack.clear();
	m_visibleControlPointIds.clear();
	m_visibleControlPointIds_conBack.clear();
	m_currentControlPointPage = 0;

	// 存储所有数据，以便后续查询 
	QMap<QString, QList<QStringList>> allTieData;
	QMap<QString, QList<QStringList>> pointCoordinates;

	// 获取 SatTiePointMatch 文件夹下的所有 model_*.icp 文件
	QString tiePointDir = projectdir + QString::fromLocal8Bit("SatTiePointMatch");
	QDir dir(tiePointDir);
	QStringList icpFiles = dir.entryList(QStringList() << "model_*.icp", QDir::Files);

	if (icpFiles.isEmpty()) {
		qDebug() << QString::fromLocal8Bit("未找到 model_*.icp 文件在目录:") << tiePointDir;
		return;
	}

	// 第一遍遍历：收集所有数据并计算重叠度 
	for (const QString& icpFile : icpFiles) {
		QString txtFilePath = tiePointDir + "/" + icpFile;
		QFile txtFile(txtFilePath);
		if (!txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << txtFilePath << QString::fromLocal8Bit("无法打开.icp文件");
			continue;
		}

		QTextStream tieIn(&txtFile);
		bool isFirstLine = true;

		while (!tieIn.atEnd()) {
			QString line = tieIn.readLine().trimmed();
			if (line.isEmpty()) continue;

			// 跳过第一行（点的数量）
			if (isFirstLine) {
				isFirstLine = false;
				continue;
			}

			QStringList fields = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
			if (fields.size() < 7) continue;

			// 使用文件中的ID作为全局ID 
			QString globalPointId = fields[0];

			// 提取文件名 (第5列)
			QString fileName = fields[4];

			// 保存完整数据到全局map中 
			allTieData[globalPointId].append(fields);

			// 保存文件名和坐标数据 (fileName, x, y, z, imageX, imageY)
			QStringList coordData;
			coordData << fileName
				<< fields[1]  // X坐标 
				<< fields[2]  // Y坐标
				<< fields[3]  // Z坐标 
				<< fields[5]  // imageX
				<< fields[6]; // imageY
			pointCoordinates[globalPointId].append(coordData);

			// 添加到控制点数据结构 
			if (!m_controlPoints.contains(globalPointId)) {
				ControlPoint point;
				point.id = globalPointId;
				point.type = QString::fromLocal8Bit("控制点");
				point.state = "Y";
				point.priority = "1";

				// 设置坐标 
				point.pointX = fields[1];
				point.pointY = fields[2];
				point.pointZ = fields[3];

				m_controlPoints.insert(globalPointId, point);
			}

			// 添加卫星影像数据 
			SatImageData satImage;
			satImage.path = fileName;
			satImage.imageX = fields[5].toDouble();  // 第6列对应imageX 
			satImage.imageY = fields[6].toDouble();  // 第7列对应imageY
			m_controlPoints[globalPointId].satImages.append(satImage);

			// 添加到按文件顺序的点ID列表
			QString pointInfo = globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3];
			if (!this->colPointIds_out.contains(pointInfo)) {
				this->colPointIds_out.append(pointInfo);
			}
		}
		txtFile.close();
	}

	// 计算重叠度 
	for (auto it = m_controlPoints.begin(); it != m_controlPoints.end(); ++it) {
		QString pointId = it.key();
		int overlap = 0;
		if (pointCoordinates.contains(pointId)) {
			overlap = pointCoordinates[pointId].size();
		}
		m_controlPoints[pointId].overlap = overlap;
	}

	// 初始化可见点ID列表
	m_visibleControlPointIds = m_controlPoints.keys();
	std::sort(m_visibleControlPointIds.begin(), m_visibleControlPointIds.end(),
		[](const QString &a, const QString &b) {
		return a.toInt() < b.toInt();
	});

	m_controlPoints_conBack = m_controlPoints;
	m_visibleControlPointIds_conBack = m_visibleControlPointIds;

	// 加载第一页数据 
	loadControlPointCurrentPage();

	if (!m_controlPoints.isEmpty()) {
		ui.ImageInfoShow_Widget->loadPoints(QHash<QString, EncryptedPoint>(), m_controlPoints, "col");
		ui.ImageInfoShow_Widget->showPoints(true);
	}
}

/**
 * @brief 加密点列表平差数据写入
 */
void IMAGEPS::EncreadResidualData()
{
	QString residualFile = "";

	QDateTime timestamp = PublicFunctions::readTimestampFromXml("SatTiePointMatch", this->projectdir);
	//if (!timestamp.isValid()) {
	//	return;
	//}

	QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("SatBA") + "/" + "NewRPCs" + "/", timestamp);
	for (const QString& filePath : resultFiles) {
		QFileInfo fileInfo(filePath);

		if (fileInfo.fileName() == "ImgResidual.txt") {
			residualFile = filePath;
			break;
		}
	}

	if (!QFile::exists(residualFile)) {
		qDebug() << "Residual file not found:" << residualFile;
		return;
	}

	QFile file(residualFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open residual file:" << residualFile;
		return;
	}

	// 存储ID和对应的所有residual数据 
	QMap<QString, QList<QPair<double, double>>> idToResiduals;

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.isEmpty())   continue;

		QStringList parts = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
		if (parts.size() < 4) continue;

		QString id = parts[0];
		double y = parts[2].toDouble();
		double x = parts[3].toDouble();

		idToResiduals[id].append(QPair<double, double>(x, y));
	}
	file.close();

	// 更新数据结构中的残差值 
	for (auto it = idToResiduals.begin(); it != idToResiduals.end(); ++it) {
		QString id = it.key();
		if (m_encryptedPoints.contains(id)) {
			double maxResidual = 0.0;
			for (const auto& pair : it.value()) {
				double residual = sqrt(pair.first  * pair.first + pair.second  * pair.second);
				if (residual > maxResidual) {
					maxResidual = residual;
				}
			}
			m_encryptedPoints[id].residual = QString::number(maxResidual);
		}
	}

	// 更新当前可见点的残差值 
	updateVisiblePoints();

	loadFileToTextEdit(projectdir + "SatBA/NewRPCs/" + QString::fromLocal8Bit("ImgResidual.txt"), ui.imageLog_Edit);
}

/**
 * @brief 控制点列表平差数据写入
 */
void IMAGEPS::ConResidualFiles()
{
	QString imgResidualFile = "";
	QString objResidualFile = "";

	QDateTime timestamp = PublicFunctions::readTimestampFromXml("CtlPointMatch", this->projectdir);
	//if (!timestamp.isValid()) {
	//	return;
	//}

	// 查找残差文件
	QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(
		projectdir + QString::fromLocal8Bit("SatBA") + "/" + "NewRPCs" + "/", timestamp);
	for (const QString& filePath : resultFiles) {
		QFileInfo fileInfo(filePath);
		if (fileInfo.fileName() == "ImgResidual.txt") {
			imgResidualFile = filePath;
		}
		else if (fileInfo.fileName() == "ObjResidual.txt") {
			objResidualFile = filePath;
		}
	}

	// 处理影像残差文件
	if (!imgResidualFile.isEmpty() && QFile::exists(imgResidualFile)) {
		QFile file(imgResidualFile);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);
			QMap<QString, QPair<double, double>> imgResidualData;

			while (!in.atEnd()) {
				QString line = in.readLine().trimmed();
				if (line.isEmpty())  continue;

				QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
				if (parts.size() < 3) continue;

				QString id;
				double x, y;
				if (parts.size() == 3) {
					id = parts[0];
					y = parts[1].toDouble();
					x = parts[2].toDouble();
				}
				else {
					id = parts[0];
					y = parts[2].toDouble();
					x = parts[3].toDouble();
				}
				imgResidualData.insert(id, QPair<double, double>(x, y));
			}
			file.close();

			// 更新控制点数据结构的残差值 
			for (auto it = imgResidualData.begin(); it != imgResidualData.end(); ++it) {
				QString id = it.key();
				if (m_controlPoints.contains(id)) {
					double residual = sqrt(it.value().first  * it.value().first +
						it.value().second  * it.value().second);
					// 更新数据结构中的残差
					m_controlPoints[id].imageSide = QString::number(residual);
				}
			}
		}
	}

	// 处理物方残差文件
	if (!objResidualFile.isEmpty() && QFile::exists(objResidualFile)) {
		QFile file(objResidualFile);
		if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&file);
			QMap<QString, QPair<double, double>> objResidualData;

			while (!in.atEnd()) {
				QString line = in.readLine().trimmed();
				if (line.isEmpty())  continue;

				QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
				if (parts.size() < 7) continue;

				QString id = parts[1];
				double x = parts[5].toDouble();
				double y = parts[6].toDouble();
				objResidualData.insert(id, QPair<double, double>(x, y));
			}
			file.close();

			// 更新控制点数据结构的物方残差 
			for (auto it = objResidualData.begin(); it != objResidualData.end(); ++it) {
				QString id = it.key();
				if (m_controlPoints.contains(id)) {
					double residual = sqrt(it.value().first  * it.value().first +
						it.value().second  * it.value().second);
					// 更新数据结构中的物方残差 
					m_controlPoints[id].objectSide = QString::number(residual);
					m_controlPoints[id].height = "0";
				}
			}
		}
	}

	// 更新当前可见点的残差值 
	loadControlPointCurrentPage();

	// 加载日志文件 
	if (!imgResidualFile.isEmpty()) {
		loadFileToTextEdit(imgResidualFile, ui.imageLog_Edit);
	}
	if (!objResidualFile.isEmpty()) {
		loadFileToTextEdit(objResidualFile, ui.objectLog_Edit);
	}
}

void IMAGEPS::onExportEncryptedPointsClicked()
{
	QString saveFilePath = QFileDialog::getSaveFileName(
		this,
		QString::fromLocal8Bit("保存加密点文件"),
		QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
		QString::fromLocal8Bit("TP文件 (*.tp);;所有文件 (*)")
	);

	// 用户取消选择
	if (saveFilePath.isEmpty()) {
		return;
	}

	// 确保文件扩展名为.tp 
	if (!saveFilePath.endsWith(".tp", Qt::CaseInsensitive)) {
		saveFilePath += ".tp";
	}

	QFile outputFile(saveFilePath);
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this,
			QString::fromLocal8Bit("错误"),
			QString::fromLocal8Bit("无法创建输出文件: %1").arg(saveFilePath));
		return;
	}

	QTextStream out(&outputFile);

	QDateTime timestamp = PublicFunctions::readTimestampFromXml("SatTiePointMatch", this->projectdir);
	//if (!timestamp.isValid()) {
	//	return;
	//}
	QStringList tpFilePattern;
	QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/", timestamp);
	for (const QString& filePath : resultFiles) {
		QFileInfo fileInfo(filePath);
		QString suffix = fileInfo.suffix().toLower();
		QString fileName = fileInfo.fileName();

		if (fileName.endsWith(".tp", Qt::CaseInsensitive)) {
			tpFilePattern << filePath;
		}
	}

	bool hasContent = false;
	for (const QString& tpFilePath : tpFilePattern) {
		QFile inputFile(tpFilePath);
		if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&inputFile);
			while (!in.atEnd()) {
				QString line = in.readLine();
				if (!line.trimmed().isEmpty()) {  // 跳过空行
					out << line << "\n";
					hasContent = true;
				}
			}
			inputFile.close();
		}
	}

	outputFile.close();

	// 4. 检查是否成功合并了内容 
	if (!hasContent) {
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("没有找到可导出的加密点数据"));
		outputFile.remove();   // 删除空文件
		return;
	}

	// 5. 提示用户导出成功
	QMessageBox::information(this,
		QString::fromLocal8Bit("成功"),
		QString::fromLocal8Bit("加密点已成功导出到: %1").arg(saveFilePath));
}

void IMAGEPS::inExportEncryptedPointsClicked()
{
	QStringList tieFilePattern;
	QStringList tpFilePattern;
	QString tiffBasePath = parentDir;

	QStringList selectedFiles = QFileDialog::getOpenFileNames(
		this,
		QString::fromLocal8Bit("选择连接点文件"),
		projectdir + QString::fromLocal8Bit("SatTiePointMatch"),
		QString::fromLocal8Bit("TP文件 (*.tp)")
	);

	// 如果用户选择了文件，则处理文件路径
	if (!selectedFiles.isEmpty()) {
		tpFilePattern = selectedFiles;

		// 为每个.tp文件查找对应的.tie文件 
		for (const QString &tpFile : tpFilePattern) {
			QFileInfo tpInfo(tpFile);
			QString tieFile = tpInfo.absolutePath() + "/" + tpInfo.completeBaseName() + ".tie";

			if (QFile::exists(tieFile)) {
				tieFilePattern << tieFile;
			}
		}

		// 如果.tie文件数量不匹配，提示用户
		if (tieFilePattern.size() != tpFilePattern.size()) {
			QMessageBox::warning(
				this,
				QString::fromLocal8Bit("警告"),
				QString::fromLocal8Bit("部分.tp文件没有找到对应的.tie文件")
			);
		}
	}
	else {
		return;
	}

	//buildPSIntersectObjCmdFile();
	QString tmpprojectDir = getProjectDir(tieFilePattern[0]);

	buildPSIntersectObjCmdFileConnection = connect(this, &IMAGEPS::buildPSIntersectObjCmdFileFinished, this, [this, tmpprojectDir, tieFilePattern, tpFilePattern]() mutable {
		disconnect(buildPSIntersectObjCmdFileConnection);

		// 创建并启动进程 
		QProcess *process1 = new QProcess(this);
#ifdef Q_OS_LINUX 
		QString program = this->appDirPath + QString::fromLocal8Bit("/linux64/PSIntersectObjCmd.x");
#else 
		QString program = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSIntersectObjCmd.exe");
#endif 
		QStringList arguments;
		QString XMLpath = tmpprojectDir + "/" + "PSIntersectObjCmd.task";
		arguments << XMLpath;
		QString moniFile = tmpprojectDir + QString::fromLocal8Bit("/PSIntersectObjCmd") + QString::fromLocal8Bit(".moni");

		// 启动进程 
		process1->start(program, arguments);
		m_childProcesses.append(process1);

		// 进程结束时的处理 
		connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[=](int exitCode, QProcess::ExitStatus status) {
			QFile file(moniFile);
			m_childProcesses.removeOne(process1);
			int maxWaitTime = 5000; // 5秒 
			int checkInterval = 200; // 0.2秒 
			int elapsed = 0;
			while (!file.exists() && elapsed < maxWaitTime) {
				QThread::msleep(checkInterval);
				elapsed += checkInterval;
				QCoreApplication::processEvents();
			}

			if (file.exists()) {
				//if (!tpFilePattern.isEmpty()) {
				//	QList<QString> orderedPointIds;
				//	ui.measureAreaShow_GLW->loadPoints(tpFilePattern, DataModelPath, "con", orderedPointIds);
				//	ui.measureAreaShow_GLW->showPoints(true);
				//}

				QString objFilePath = QFileInfo(moniFile).path() + "/obj.txt";
				QFile objFile(objFilePath);
				QHash<QString, QStringList> objDataMap;  // 存储ID到重叠度的映射 

				if (objFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream in(&objFile);
					QString line = in.readLine();     // 第一行是数据行数（跳过）

					while (!in.atEnd()) {
						line = in.readLine().trimmed();
						if (line.isEmpty())    continue;

						QStringList parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
						if (parts.size() >= 5) {  // 确保有足够列 
							QString pointId = parts[0];
							QStringList IdData;
							IdData << parts[1] << parts[2] << parts[3] << parts[4];  // 第五列是重叠度 
							objDataMap.insert(pointId, IdData);
						}
					}
					objFile.close();
				}
				else {
					qWarning() << "无法打开obj.txt   文件:" << objFilePath;
				}

				QList<QStringList> allTieData;
				QHash<QString, QStringList> uniqueDataMap;
				QHash<QString, QString> keyToTiffMap; // 存储键到TIFF文件名的映射 

				int fileIndex = 0;
				for (auto resultfile : tieFilePattern)
				{
					if (fileIndex >= tpFilePattern.size() || fileIndex >= tieFilePattern.size())
						break;

					QString tpFilePath = tpFilePattern.at(fileIndex);

					if (!QFile::exists(tpFilePath)) {
						if (fileIndex == 0) {
							QMessageBox::critical(this, u8"错误", u8"找不到.tie或.tp文件");
							return;
						}
						break;
					}

					// 读取.tp文件获取TIFF文件名映射 
					QFile tpFile(tpFilePath);
					if (tpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
						QTextStream tpIn(&tpFile);
						while (!tpIn.atEnd()) {
							QString line = tpIn.readLine().trimmed();
							if (line.isEmpty())   continue;

							QStringList fields = line.split("  ", Qt::SkipEmptyParts);
							if (fields.size() >= 2) {
								QString key = fields[0];
								QString tiffName = fields[1];
								keyToTiffMap.insert(key, tiffName);
							}
						}
						tpFile.close();
					}

					fileIndex++;
				}

				// 清空原有数据 
				m_encryptedPoints.clear();

				// 3. 填充数据结构 
				QStringList pointIds = objDataMap.keys();
				std::sort(pointIds.begin(), pointIds.end(), [](const QString &a, const QString &b) {
					return a.toInt() < b.toInt();
				});

				//// 填充加密点数据结构 
				//for (const QString &pointId : pointIds) {
				//	EncryptedPoint point;
				//	point.id = pointId;

				//	const QStringList &dataList = objDataMap.value(pointId);
				//	point.residual = "";
				//	point.state = "Y";

				//	if (dataList.size() >= 4) {
				//		point.overlap = dataList[3].toInt();
				//		point.pointX = dataList[0];
				//		point.pointY = dataList[1];
				//		point.pointZ = dataList[2];
				//	}
				//	else {
				//		// 处理不足3个元素的情况
				//		point.overlap = 0;
				//		point.pointX = "0";
				//		point.pointY = "0";
				//		point.pointZ = "0";
				//	}


				//	// 从tp文件获取卫星影像信息 
				//	for (const QString& tpFilePath : tpFilePattern) {
				//		QFile tpFile(tpFilePath);
				//		if (tpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				//			QTextStream tpIn(&tpFile);
				//			while (!tpIn.atEnd()) {
				//				QString line = tpIn.readLine().trimmed();
				//				if (line.isEmpty())  continue;

				//				QStringList fields = line.split(" ", Qt::SkipEmptyParts);
				//				if (fields.size() >= 4 && fields[0] == pointId) {
				//					SatImageData satImage;
				//					satImage.path = fields[1];
				//					satImage.imageX = fields[2].toDouble();
				//					satImage.imageY = fields[3].toDouble();
				//					point.satImages.append(satImage);
				//				}
				//			}
				//			tpFile.close();
				//		}
				//	}

				//	m_encryptedPoints.insert(pointId, point);
				//}

				QHash<QString, QList<SatImageData>> tpDataHash;

				// 一次性读取所有TP文件 
				for (const QString& tpFilePath : tpFilePattern) {
					QFile tpFile(tpFilePath);
					if (tpFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
						QTextStream tpIn(&tpFile);
						QString line;

						while (tpIn.readLineInto(&line)) {
							line = line.trimmed();
							if (line.isEmpty())  continue;

							QStringList fields = line.split(" ", Qt::SkipEmptyParts);
							if (fields.size() >= 4) {
								QString pointId = fields[0];

								SatImageData satImage;
								satImage.path = fields[1];
								satImage.imageX = fields[2].toDouble();
								satImage.imageY = fields[3].toDouble();

								// 使用哈希表提高查找效率 
								if (!tpDataHash.contains(pointId)) {
									tpDataHash.insert(pointId, QList<SatImageData>());
									tpDataHash[pointId].append(satImage);
								}
							}
						}
					}
					tpFile.close();
				}

				// 第二步：批量创建加密点对象 
				QHash<QString, EncryptedPoint> tempPoints;
				tempPoints.reserve(pointIds.size());  // 预分配内存 

				// 批量处理所有点ID 
				for (const QString &pointId : pointIds) {
					EncryptedPoint point;
					point.id = pointId;
					point.residual = "";
					point.state = "Y";

					// 批量赋值坐标数据 
					const QStringList &dataList = objDataMap.value(pointId);

					point.overlap = (dataList.size() >= 4) ? dataList[3].toInt() : 0;

					// 批量设置坐标值 
					if (dataList.size() >= 4) {
						point.pointX = dataList[0];
						point.pointY = dataList[1];
						point.pointZ = dataList[2];

						// 从预加载的哈希表中获取卫星影像数据 
						point.satImages = tpDataHash.value(pointId);

						tempPoints.insert(pointId, point);
					}
				}

				// 第三步：批量替换原有数据 
				m_encryptedPoints = tempPoints;

				if (!tpFilePattern.isEmpty()) {
					QList<QString> orderedPointIds;
					ui.ImageInfoShow_Widget->loadPoints(m_encryptedPoints, QHash<QString, ControlPoint>(), "con");
					ui.ImageInfoShow_Widget->showPoints(true);
				}

				// 初始化懒加载 
				m_currentPage = 0;
				updateVisiblePoints();
			}
		});

	}, Qt::UniqueConnection);

	buildPSIntersectObjCmdFile(tmpprojectDir, tieFilePattern);
}

void IMAGEPS::onExportControlPointsClicked()
{
	// 1. 检查是否有数据可导出
	if (colPointIds_out.isEmpty() && ui.controlPointsLIst_TableW->model()->rowCount() > 0) {
		QMessageBox::information(this,
			QString::fromLocal8Bit("提示"),
			QString::fromLocal8Bit("没有控制点数据可导出"));
		return;
	}

	// 2. 弹出文件保存对话框 
	QString saveFilePath = QFileDialog::getSaveFileName(
		this,
		QString::fromLocal8Bit("导出控制点"),
		QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
		QString::fromLocal8Bit("文本文件 (*.cp);;所有文件 (*)")
	);

	// 用户取消选择 
	if (saveFilePath.isEmpty()) {
		return;
	}

	// 3. 创建并打开输出文件
	QFile outputFile(saveFilePath);
	if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this,
			QString::fromLocal8Bit("错误"),
			QString::fromLocal8Bit("无法创建输出文件: %1").arg(saveFilePath));
		return;
	}

	QTextStream out(&outputFile);

	// 4. 写入数据数量（第一行）
	out << colPointIds_out.size() << "\n";

	// 5. 写入所有控制点数据（从第二行开始）
	for (const QString& pointId : colPointIds_out) {
		out << pointId << "\n";
	}

	outputFile.close();

	// 6. 提示用户导出成功
	QMessageBox::information(this,
		QString::fromLocal8Bit("成功"),
		QString::fromLocal8Bit("控制点已成功导出到: %1").arg(saveFilePath));
}

void IMAGEPS::inExportControlPointsClicked()
{
	// 清空原有数据 
	m_currentControlPointPage = 0;

	// 初始化ID计数器         
	int baseId = 0;

	// 存储所有数据，以便后续查询          
	QMap<QString, QList<QStringList>> allTieData;
	QMap<QString, QList<QStringList>> pointCoordinates;

	// 清空第一个表格          
	ControlPointsModel->setRowCount(0);

	// 第一遍遍历：收集所有数据并计算重叠度      
	QMap<QString, int> overlapCount;

	// 设置文件过滤器，同时支持.txt和.icp文件
	QString filter = QString::fromLocal8Bit("控制点文件 (*.txt *.icp)");
	//QString initialDir = projectdir;

	QStringList selectedFiles = QFileDialog::getOpenFileNames(
		this,
		QString::fromLocal8Bit("选择控制点文件"),
		projectdir,
		filter
	);

	QStringList tmpcolPath;
	if (selectedFiles.isEmpty()) {
		return;
	}

	// 第一遍遍历：收集所有数据并计算重叠度 
	for (auto data : selectedFiles) {
		QString txtFilePath = data;
		QFile txtFile(txtFilePath);
		if (!txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qDebug() << txtFilePath << QString::fromLocal8Bit("无法打开控制点文件");
			PROJECT_LOG_ERROR(this->CurrentConfig, QString::fromLocal8Bit("无法打开控制点文件: ") + txtFilePath );
			ui.runLog_Edit->append(QString::fromLocal8Bit("无法打开控制点文件: ") + txtFilePath);
			continue;
		}

		bool isIcpFile = txtFilePath.endsWith(".icp");
		tmpcolPath.push_back(data);
		QTextStream tieIn(&txtFile);
		bool isFirstLine = true;

		while (!tieIn.atEnd()) {
			QString line = tieIn.readLine().trimmed();
			if (line.isEmpty())  continue;

			// 跳过第一行（点的数量，仅对.txt文件）
			if (isFirstLine && !isIcpFile) {
				isFirstLine = false;
				continue;
			}

			QStringList fields;
			if (isIcpFile) {
				// 处理.icp文件格式 
				fields = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
				if (fields.size() < 7) continue;

				// 提取文件名（第5列）
				QString fileName = fields[4];

				// 修改点ID：添加baseId前缀确保全局唯一 
				//QString originalPointId = fields[0];
				QString globalPointId = fields[0];

				// 保存完整数据到全局map中 
				allTieData[globalPointId].append(fields);

				// 保存文件名和坐标数据 (fileName, x, y, z, imageX, imageY)
				QStringList coordData;
				coordData << fileName
					<< fields[1]  // X坐标 
					<< fields[2]  // Y坐标
					<< fields[3]  // Z坐标 
					<< fields[5]  // imageX
					<< fields[6]; // imageY
				pointCoordinates[globalPointId].append(coordData);

				// 添加到控制点数据结构 
				if (!m_controlPoints.contains(globalPointId)) {
					ControlPoint point;
					point.id = globalPointId;
					point.type = QString::fromLocal8Bit("控制点");
					point.state = "Y";
					point.priority = "1";

					// 设置坐标 
					point.pointX = fields[1];
					point.pointY = fields[2];
					point.pointZ = fields[3];

					m_controlPoints.insert(globalPointId, point);
				}

				// 添加卫星影像数据 
				SatImageData satImage;
				satImage.path = fileName;
				satImage.imageX = fields[5].toDouble();
				satImage.imageY = fields[6].toDouble();
				m_controlPoints[globalPointId].satImages.append(satImage);

				// 添加到按文件顺序的点ID列表
				QString pointInfo = globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3];
				if (!this->colPointIds_out.contains(pointInfo)) {
					this->colPointIds_out.append(pointInfo);
				}
			}
			else {
				// 处理原有.txt文件格式
				fields = line.split(" ", Qt::SkipEmptyParts);
				if (fields.size() < 8) continue;

				// 修改点ID：添加baseId前缀确保全局唯一 
				QString originalPointId = fields[0];
				QString globalPointId = QString::number(baseId + originalPointId.toInt());

				// 提取文件名 
				int lastUnderscorePos = QFileInfo(txtFilePath).fileName().lastIndexOf('_',
					QFileInfo(txtFilePath).fileName().lastIndexOf('@') - 1);
				QString fileName = QFileInfo(txtFilePath).fileName().left(lastUnderscorePos);

				// 保存完整8列数据到全局map中 
				allTieData[globalPointId].append(fields);

				// 保存文件名和坐标数据 (fileName, x, y, z)
				QStringList coordData;
				coordData << fileName << fields[1] << fields[2] <<
					(fields.size() > 7 ? fields[3] : "0") << fields[7] << fields[8];
				pointCoordinates[globalPointId].append(coordData);

				// 添加到控制点数据结构
				if (!m_controlPoints.contains(globalPointId)) {
					ControlPoint point;
					point.id = globalPointId;
					point.type = QString::fromLocal8Bit("控制点");
					point.state = "Y";
					point.priority = "1";

					// 设置坐标 
					point.pointX = fields[1];
					point.pointY = fields[2];
					point.pointZ = fields.size() > 3 ? fields[3] : "";

					m_controlPoints.insert(globalPointId, point);
				}

				// 添加卫星影像数据 
				SatImageData satImage;
				satImage.path = fileName;
				satImage.imageX = fields[7].toDouble();
				satImage.imageY = fields[8].toDouble();
				m_controlPoints[globalPointId].satImages.append(satImage);

				// 添加到按文件顺序的点ID列表 
				if (!this->colPointIds_out.contains(
					globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3])) {
					this->colPointIds_out.append(
						globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3]);
				}
			}
		}
		txtFile.close();
		baseId += 100000;
	}

	// 计算重叠度 
	for (auto it = m_controlPoints.begin(); it != m_controlPoints.end(); ++it) {
		QString pointId = it.key();
		int overlap = 0;
		if (pointCoordinates.contains(pointId)) {
			overlap = pointCoordinates[pointId].size();
		}
		m_controlPoints[pointId].overlap = overlap;
	}

	// 初始化可见点ID列表 
	m_visibleControlPointIds = m_controlPoints.keys();
	std::sort(m_visibleControlPointIds.begin(), m_visibleControlPointIds.end(),
		[](const QString &a, const QString &b) {
		return a.toInt() < b.toInt();
	});

	// 加载第一页数据 
	loadControlPointCurrentPage();

	if (!selectedFiles.isEmpty()) {
		ui.ImageInfoShow_Widget->loadPoints(QHash<QString, EncryptedPoint>(), m_controlPoints, "col");
		ui.ImageInfoShow_Widget->showPoints(true);
	}
}

//void IMAGEPS::inExportControlPointsClicked()
//{
//	// 清空原有数据
//	/*m_controlPoints.clear();
//	m_visibleControlPointIds.clear();*/
//	m_currentControlPointPage = 0;
//
//	// 初始化ID计数器         
//	int baseId = 0;
//
//	// 存储所有数据，以便后续查询         
//	QMap<QString, QList<QStringList>> allTieData; // key: point ID, value: list of data rows (all 8 columns + fileName)      
//	QMap<QString, QList<QStringList>> pointCoordinates; // key: point ID, value: list of (fileName, x, y, z)
//
//	// 清空第一个表格         
//	ControlPointsModel->setRowCount(0);
//
//	// 第一遍遍历：收集所有数据并计算重叠度      
//	QMap<QString, int> overlapCount; // 存储每个点的重叠度
//
//	QStringList selectedFiles = QFileDialog::getOpenFileNames(
//		this,
//		QString::fromLocal8Bit("选择控制点文件"),
//		projectdir + QString::fromLocal8Bit("CtlPointMatch"),
//		QString::fromLocal8Bit("txt文件 (*.txt)")
//	);
//
//	QStringList tmpcolPath;
//	if (selectedFiles.isEmpty()) {
//		return;
//	}
//
//	// 第一遍遍历：收集所有数据并计算重叠度 
//	for (auto data : selectedFiles) {
//		QString txtFilePath = data;
//		QFile txtFile(txtFilePath);
//		if (!txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			qDebug() << txtFilePath << QString::fromLocal8Bit("无法打开.txt文件");
//			continue;
//		}
//
//		tmpcolPath.push_back(data);
//		QTextStream tieIn(&txtFile);
//		bool isFirstLine = true;
//
//		while (!tieIn.atEnd()) {
//			QString line = tieIn.readLine().trimmed();
//			if (line.isEmpty())  continue;
//
//			// 跳过第一行（点的数量）
//			if (isFirstLine) {
//				isFirstLine = false;
//				continue;
//			}
//
//			QStringList fields = line.split(" ", Qt::SkipEmptyParts);
//			if (fields.size() < 8) continue;
//
//			// 修改点ID：添加baseId前缀确保全局唯一 
//			QString originalPointId = fields[0];
//			QString globalPointId = QString::number(baseId + originalPointId.toInt());
//
//			// 提取文件名 
//			int lastUnderscorePos = QFileInfo(txtFilePath).fileName().lastIndexOf('_',
//				QFileInfo(txtFilePath).fileName().lastIndexOf('@') - 1);
//			QString fileName = QFileInfo(txtFilePath).fileName().left(lastUnderscorePos);
//
//			// 保存完整8列数据到全局map中 
//			allTieData[globalPointId].append(fields);
//
//			// 保存文件名和坐标数据 (fileName, x, y, z)
//			QStringList coordData;
//			coordData << fileName << fields[1] << fields[2] <<
//				(fields.size() > 7 ? fields[3] : "0") << fields[7] << fields[8];
//			pointCoordinates[globalPointId].append(coordData);
//
//			// 添加到控制点数据结构
//			if (!m_controlPoints.contains(globalPointId)) {
//				ControlPoint point;
//				point.id = globalPointId;
//				point.type = QString::fromLocal8Bit("控制点");
//				point.state = "Y";
//				point.priority = "1";
//
//				// 设置坐标 
//				point.pointX = fields[1];
//				point.pointY = fields[2];
//				point.pointZ = fields.size() > 3 ? fields[3] : "";
//
//				m_controlPoints.insert(globalPointId, point);
//			}
//
//			// 添加卫星影像数据 
//			SatImageData satImage;
//			satImage.path = fileName;
//			satImage.imageX = fields[7].toDouble();
//			satImage.imageY = fields[8].toDouble();
//			m_controlPoints[globalPointId].satImages.append(satImage);
//
//			// 添加到按文件顺序的点ID列表
//			if (!this->colPointIds_out.contains(
//				globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3])) {
//				this->colPointIds_out.append(
//					globalPointId + " " + fields[1] + " " + fields[2] + " " + fields[3]);
//			}
//		}
//		txtFile.close();
//		baseId += 100000;
//	}
//
//	// 计算重叠度 
//	for (auto it = m_controlPoints.begin(); it != m_controlPoints.end(); ++it) {
//		QString pointId = it.key();
//		int overlap = 0;
//		if (pointCoordinates.contains(pointId)) {
//			overlap = pointCoordinates[pointId].size();
//		}
//		m_controlPoints[pointId].overlap = overlap;
//	}
//
//	// 初始化可见点ID列表
//	m_visibleControlPointIds = m_controlPoints.keys();
//	std::sort(m_visibleControlPointIds.begin(), m_visibleControlPointIds.end(),
//		[](const QString &a, const QString &b) {
//		return a.toInt() < b.toInt();
//	});
//
//	// 加载第一页数据 
//	loadControlPointCurrentPage();
//
//	if (!selectedFiles.isEmpty()) {
//		ui.ImageInfoShow_Widget->loadPoints(QHash<QString, EncryptedPoint>(), m_controlPoints, "col");
//		ui.ImageInfoShow_Widget->showPoints(true);
//	}
//}

/**
 * @brief 相对定位精度质检列表数据写入
 */
void IMAGEPS::AbsPositPrecCheckPList()
{
	int row = 0;
	// 遍历所有文件路径
	for (auto it = AbsPositPrecCheckPathPath.begin(); it != AbsPositPrecCheckPathPath.end(); ++it) {
		QString filePath = it.key();
		QFileInfo fileInfo(filePath);

		// 查找同名的.dms文件 
		QString dmsFilePath = fileInfo.path() + "/" + fileInfo.completeBaseName() + ".dms";
		QFile dmsFile(dmsFilePath);

		if (!dmsFile.exists()) {
			qWarning() << u8"未找到对应的.dms文件:" << dmsFilePath;
			continue;
		}

		// 打开并读取.dms文件 
		if (!dmsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qWarning() << u8"无法打开.dms文件:" << dmsFilePath;
			continue;
		}

		// 存储解析出的值 
		QMap<QString, double> dmsValues;
		QTextStream in(&dmsFile);
		QString firstLine; // 存储第一行内容 

		// 逐行读取文件
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (firstLine.isEmpty()) {
				firstLine = line; // 保存第一行
			}

			// 解析键值对
			QStringList parts = line.split(':');
			if (parts.size() == 2) {
				QString key = parts[0].trimmed();
				double value = parts[1].trimmed().toDouble();
				dmsValues[key] = value;
			}
		}
		dmsFile.close();

		// 检查必需的值是否存在
		const QStringList requiredKeys = { "MinGeoX", "MaxGeoX", "MinGeoY", "MaxGeoY",
										"AveGeoX", "AveGeoY", "dmsGeoX", "dmsGeoY" };
		for (const QString& key : requiredKeys) {
			if (!dmsValues.contains(key)) {
				qWarning() << u8"缺少必需的值:" << key << u8"在文件:" << dmsFilePath;
				continue;
			}
		}

		// 计算两个平方和开方的值
		double aveGeoNorm = sqrt(dmsValues["AveGeoX"] * dmsValues["AveGeoX"] +
			dmsValues["AveGeoY"] * dmsValues["AveGeoY"]);
		double dmsGeoNorm = sqrt(dmsValues["dmsGeoX"] * dmsValues["dmsGeoX"] +
			dmsValues["dmsGeoY"] * dmsValues["dmsGeoY"]);

		// 填充表格数据
		auto setItem = [&](int column, const QVariant& data) {
			QTableWidgetItem* item = new QTableWidgetItem(data.toString());
			item->setTextAlignment(Qt::AlignCenter);
			ui.AbsPositPrecCheckDataList_TableW->setItem(row, column, item);
		};

		//// 第1列：序号
		//setItem(0, row + 1);

		//// 第2列：文件名（不含扩展名）
		//setItem(1, fileInfo.completeBaseName());

		// 第3列：原始文件路径 
		setItem(2, QString::fromLocal8Bit("是"));

		// 第4列：dms文件第一行 
		setItem(3, firstLine);

		// 第5-12列：
		setItem(4, dmsValues["MinGeoX"]);
		setItem(5, dmsValues["MaxGeoX"]);
		setItem(6, dmsValues["MinGeoY"]);
		setItem(7, dmsValues["MaxGeoY"]);
		setItem(8, dmsValues["AveGeoX"]);
		setItem(9, dmsValues["AveGeoY"]);
		setItem(10, dmsValues["dmsGeoX"]);
		setItem(11, dmsValues["dmsGeoY"]);

		// 第13列：AveGeo平方和开方
		setItem(12, aveGeoNorm);

		// 第14列：dmsGeo平方和开方
		setItem(13, dmsGeoNorm);
		row++;
	}

	// 调整列宽以适应内容
	ui.AbsPositPrecCheckDataList_TableW->resizeColumnsToContents();
}

/**
 * @brief 云检成果列表数据写入
 */
void IMAGEPS::CloudDetectionList()
{
	// 清空现有数据但保留表头
	ui.CloudDetectionList_TableW->clearContents();
	ui.CloudDetectionList_TableW->setRowCount(0);

	// 获取缩略图路径列表（Overview影像）
	CloudDetectionListFilePath = systemConfig->getThumbnailGenerationFilePath("OverView");
	// 获取云检成果路径列表 
	QStringList cloudDectPaths = systemConfig->getThumbnailGenerationFilePath("CloudDect");

	// 确保数据量一致 
	int count = qMin(CloudDetectionListFilePath.size(), cloudDectPaths.size());

	for (int i = 0; i < count; ++i) {
		// 添加新行 
		ui.CloudDetectionList_TableW->insertRow(i);

		// 第一列：序号
		QTableWidgetItem *indexItem = new QTableWidgetItem(QString::number(i + 1));
		indexItem->setTextAlignment(Qt::AlignCenter);
		ui.CloudDetectionList_TableW->setItem(i, 0, indexItem);

		// 第二列：影像名称（Overview文件名）
		QFileInfo overviewInfo(CloudDetectionListFilePath[i]);
		QTableWidgetItem *nameItem = new QTableWidgetItem(overviewInfo.completeBaseName());
		nameItem->setTextAlignment(Qt::AlignCenter);
		ui.CloudDetectionList_TableW->setItem(i, 1, nameItem);

		// 第三列：云检成果（从CloudDect的txt文件读取CloudRatio）
		QFileInfo cloudDectInfo(cloudDectPaths[i]);
		QString resultTxtPath = cloudDectInfo.path() + "/" + cloudDectInfo.completeBaseName() + ".txt";

		double cloudRatio = 0.0;
		QFile txtFile(resultTxtPath);
		if (txtFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&txtFile);
			while (!in.atEnd()) {
				QString line = in.readLine();
				if (line.contains("CloudRatio")) {
					QStringList parts = line.split("=");
					if (parts.size() == 2) {
						cloudRatio = parts[1].trimmed().toDouble();
						break;
					}
				}
			}
			txtFile.close();
		}

		QTableWidgetItem *ratioItem = new QTableWidgetItem(QString::number(cloudRatio, 'f', 4));
		ratioItem->setTextAlignment(Qt::AlignCenter);
		ui.CloudDetectionList_TableW->setItem(i, 2, ratioItem);
	}
	// 自动调整列宽 
	ui.CloudDetectionList_TableW->resizeColumnsToContents();
}


QDateTime IMAGEPS::readTimestampFromXml(const QString& nodeName)
{
	QFile file(this->projectdir + "ControlPoint.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open ControlPoint.xml";
		return QDateTime();
	}

	QXmlStreamReader reader(&file);
	QDateTime result;

	while (!reader.atEnd()) {
		reader.readNext();

		if (reader.isStartElement() && reader.name() == nodeName) {
			reader.readNext();
			if (reader.isCharacters()) {
				// 使用自定义格式解析，匹配"yyyy-MM-dd  hh:mm:ss"
				result = QDateTime::fromString(reader.text().toString(), "yyyy-MM-dd  hh:mm:ss");
				break;
			}
		}
	}

	file.close();

	if (result.isNull()) {
		qDebug() << "Timestamp node" << nodeName << "not found or invalid";
	}

	return result;
}

QString IMAGEPS::GetDatasetProjection(GDALDataset* dataset, const QString& prjFilePath)
{
	// 1. 首先尝试从.prj文件读取投影信息 
	if (!prjFilePath.isEmpty()) {
		QFile prjFile(prjFilePath);
		if (prjFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream stream(&prjFile);
			QString wkt = stream.readAll().trimmed();
			prjFile.close();

			if (!wkt.isEmpty()) {
				return wkt;
			}
		}
	}

	// 2. 尝试从数据集本身获取投影 
	const char* pszProjection = dataset->GetProjectionRef();
	if (pszProjection && strlen(pszProjection) > 0) {
		return QString(pszProjection);
	}

	// 3. 作为后备方案，返回WGS84投影 
	OGRSpatialReference oSRS;
	oSRS.SetWellKnownGeogCS("WGS84");
	char* pszSRS_WKT = nullptr;
	if (oSRS.exportToWkt(&pszSRS_WKT) == OGRERR_NONE) {
		QString wkt(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
		return wkt;
	}

	// 4. 如果所有方法都失败，返回空字符串 
	return QString();
}

void IMAGEPS::loadXMLConfigs() {
	QFile configFile("../bin/config/SrcDataParseGeoXMLConfig/SrcDataParseGeoXMLConfig.csv");
	if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qWarning() << "Failed to open SrcDataParseGeoXMLConfig.csv";
		return;
	}

	QTextStream in(&configFile);
	QString currentPrefix;

	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.startsWith("#")) {
			// 新配置块开始 
			currentPrefix = line.mid(1, 3); // 取#后的3个字符作为前缀 
			xmlConfigs[currentPrefix].xmlSuffix = line.mid(line.indexOf(':') + 1);
		}
		else if (!line.isEmpty() && !currentPrefix.isEmpty()) {
			// 添加字段名 
			xmlConfigs[currentPrefix].fieldNames.append(line);
		}
	}
}

GeoCoordinates IMAGEPS::parseGeoXML(const QString& xmlPath, const XMLParseConfig& config) {
	if (config.fieldNames.isEmpty()) {
		qWarning() << "XMLParseConfig is empty, return empty GeoCoordinates";
		return GeoCoordinates{};
	}
	
	QFile file(xmlPath);
	GeoCoordinates coords{};

	if (file.open(QIODevice::ReadOnly)) {
		QXmlStreamReader xml(&file);
		while (!xml.atEnd()) {
			xml.readNext();
			if (xml.isStartElement()) {
				// 使用配置中的字段名进行解析 
				if (xml.name() == config.fieldNames[0]) {
					coords.Eresolution = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[1]) {
					coords.Nresolution = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[2]) {
					coords.upperLeftLat = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[3]) {
					coords.upperLeftLon = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[4]) {
					coords.upperRightLat = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[5]) {
					coords.upperRightLon = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[6]) {
					coords.lowerRightLat = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[7]) {
					coords.lowerRightLon = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[8]) {
					coords.lowerLeftLat = xml.readElementText().toDouble();
				}
				else if (xml.name() == config.fieldNames[9]) {
					coords.lowerLeftLon = xml.readElementText().toDouble();
				}
			}
		}
	}
	return coords;
}

GeoCoordinates IMAGEPS::parseGeoXMLBJ3(const QString& xmlPath, const XMLParseConfig& config) {
	if (config.fieldNames.isEmpty()) {
		qWarning() << "XMLParseConfig is empty, return empty GeoCoordinates";
		return GeoCoordinates{};
	}
	
	QFile file(xmlPath);
	GeoCoordinates coords{};
	QVector<QPointF> geoPoints;
	QVector<QPoint> pixelPoints;
	bool hasGSD = false;

	if (file.open(QIODevice::ReadOnly)) {
		QXmlStreamReader xml(&file);

		while (!xml.atEnd()) {
			xml.readNext();

			if (xml.isStartElement()) {
				// 解析GSD 
				if (xml.name() == config.fieldNames[0]) { // PRODUCT_GSD 
					double gsd = xml.readElementText().toDouble();
					coords.Eresolution = gsd;
					coords.Nresolution = gsd;
					hasGSD = true;
				}
				// 解析顶点信息 
				else if (xml.name() == config.fieldNames[1]) { // Vertex 
					int col = 0, row = 0;
					double lon = 0, lat = 0;
					bool hasCol = false, hasRow = false, hasLon = false, hasLat = false;

					while (!(xml.isEndElement() && xml.name() == config.fieldNames[1])) {
						xml.readNext();

						if (xml.isStartElement()) {
							if (xml.name() == config.fieldNames[2]) { // COL 
								col = xml.readElementText().toInt();
								hasCol = true;
							}
							else if (xml.name() == config.fieldNames[3]) { // ROW 
								row = xml.readElementText().toInt();
								hasRow = true;
							}
							else if (xml.name() == config.fieldNames[4]) { // LON 
								lon = xml.readElementText().toDouble();
								hasLon = true;
							}
							else if (xml.name() == config.fieldNames[5]) { // LAT 
								lat = xml.readElementText().toDouble();
								hasLat = true;
							}
						}
					}

					if (hasCol && hasRow && hasLon && hasLat) {
						pixelPoints.append(QPoint(col, row));
						geoPoints.append(QPointF(lon, lat));
					}
				}
			}
		}

		// 动态确定四个角点 
		if (hasGSD && geoPoints.size() >= 4) {
			int minCol = INT_MAX, maxCol = INT_MIN;
			int minRow = INT_MAX, maxRow = INT_MIN;

			for (const QPoint& pt : pixelPoints) {
				minCol = qMin(minCol, pt.x());
				maxCol = qMax(maxCol, pt.x());
				minRow = qMin(minRow, pt.y());
				maxRow = qMax(maxRow, pt.y());
			}

			for (int i = 0; i < pixelPoints.size(); ++i) {
				const QPoint& pt = pixelPoints[i];

				if (pt.x() == minCol && pt.y() == minRow) {
					coords.upperLeftLon = geoPoints[i].x();
					coords.upperLeftLat = geoPoints[i].y();
				}
				else if (pt.x() == maxCol && pt.y() == minRow) {
					coords.upperRightLon = geoPoints[i].x();
					coords.upperRightLat = geoPoints[i].y();
				}
				else if (pt.x() == maxCol && pt.y() == maxRow) {
					coords.lowerRightLon = geoPoints[i].x();
					coords.lowerRightLat = geoPoints[i].y();
				}
				else if (pt.x() == minCol && pt.y() == maxRow) {
					coords.lowerLeftLon = geoPoints[i].x();
					coords.lowerLeftLat = geoPoints[i].y();
				}
			}
		}

		if (xml.hasError()) {
			qWarning() << u8"XML解析错误:" << xml.errorString();
		}
	}
	else {
		qWarning() << u8"无法打开文件:" << xmlPath;
	}

	return coords;
}

QPointF IMAGEPS::geoToScreen(double lon, double lat,
	double minLon, double maxLon,
	double minLat, double maxLat,
	int screenWidth, int screenHeight) {
	// 计算地理范围
	const double lonRange = maxLon - minLon;
	const double latRange = maxLat - minLat;

	// 计算缩放比例（保持宽高比）
	const double scaleX = screenWidth / lonRange;
	const double scaleY = screenHeight / latRange;
	const double scale = qMin(scaleX, scaleY);

	// 转换为屏幕坐标（Y轴反向）
	const double x = (lon - minLon) * scale;
	const double y = (maxLat - lat) * scale; // 反转Y轴

	return QPointF(x, y);
}

void IMAGEPS::calculateGeoBounds(const GeoCoordinates& coords,
	double& minLon, double& maxLon,
	double& minLat, double& maxLat) {
	minLon = qMin(qMin(coords.upperLeftLon, coords.lowerLeftLon),
		qMin(coords.upperRightLon, coords.lowerRightLon));
	maxLon = qMax(qMax(coords.upperLeftLon, coords.lowerLeftLon),
		qMax(coords.upperRightLon, coords.lowerRightLon));
	minLat = qMin(qMin(coords.upperLeftLat, coords.upperRightLat),
		qMin(coords.lowerLeftLat, coords.lowerRightLat));
	maxLat = qMax(qMax(coords.upperLeftLat, coords.upperRightLat),
		qMax(coords.lowerLeftLat, coords.lowerRightLat));
}

ImageGeoMetadata* IMAGEPS::getImageMetadata(const QString& filePath) {
	if (s_imageMetadata.contains(filePath)) {
		return &s_imageMetadata[filePath];
	}
	return nullptr;
}

// 1. 解析 XML
void IMAGEPS::ReadXMLresult()
{
	GeoCoordinates geoCoords;

	// 2. 计算地理范围
	double minLon, maxLon, minLat, maxLat;
	calculateGeoBounds(geoCoords, minLon, maxLon, minLat, maxLat);

	// 3. 转换四角点坐标
	const int screenWidth = 588;
	const int screenHeight = 646;
	QPolygonF screenCorners;
	screenCorners << geoToScreen(geoCoords.upperLeftLon, geoCoords.upperLeftLat,
		minLon, maxLon, minLat, maxLat,
		screenWidth, screenHeight);
	screenCorners << geoToScreen(geoCoords.upperRightLon, geoCoords.upperRightLat,
		minLon, maxLon, minLat, maxLat,
		screenWidth, screenHeight);
	screenCorners << geoToScreen(geoCoords.lowerRightLon, geoCoords.lowerRightLat,
		minLon, maxLon, minLat, maxLat,
		screenWidth, screenHeight);
	screenCorners << geoToScreen(geoCoords.lowerLeftLon, geoCoords.lowerLeftLat,
		minLon, maxLon, minLat, maxLat,
		screenWidth, screenHeight);

	//qDebug() << screenCorners;
}

//bool IMAGEPS::setGeoreferenceFromXMLAndRPC(const QStringList filenamePATH) {
//	static bool configLoaded = false;
//	if (!configLoaded) {
//		loadXMLConfigs();
//		configLoaded = true;
//	}
//
//	for (auto data : filenamePATH) {
//		QFileInfo fileInfo(data);
//		QString filePath = fileInfo.absolutePath() + "/";
//		QString originalExt = fileInfo.suffix().toLower();
//
//		QString tiffPath = filePath + fileInfo.completeBaseName() + "." + originalExt;
//		QString metadataPath = this->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + ".metadata";
//
//		// 初始化ImageGeoMetadata结构体 
//		ImageGeoMetadata metadata;
//		metadata.filePath = tiffPath;
//		QString projection;
//		// 1. 解析.metadata文件 
//		QFile metadataFile(metadataPath);
//		if (metadataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			QTextStream in(&metadataFile);
//			double resolutionX = 0.0;
//			double resolutionY = 0.0;
//			
//			bool hasProjection = false;
//
//			while (!in.atEnd()) {
//				QString line = in.readLine().trimmed();
//				if (line.startsWith("resolutionX:")) {
//					resolutionX = line.split(":")[1].trimmed().toDouble();
//				}
//				else if (line.startsWith("resolutionY:")) {
//					resolutionY = line.split(":")[1].trimmed().toDouble();
//				}
//				else if (!hasProjection) {
//					int projcsPos = line.indexOf("PROJCS[");
//					int geogcsPos = line.indexOf("GEOGCS[");
//					if (projcsPos != -1) {
//						qDebug() << line.mid(projcsPos).trimmed();
//
//						metadata.projection = line.mid(projcsPos).trimmed();
//						hasProjection = true;
//					}
//					else if (geogcsPos != -1) {
//						metadata.projection = line.mid(geogcsPos).trimmed();
//						qDebug() << line.mid(geogcsPos).trimmed();
//
//						hasProjection = true;
//					}
//				}
//			}
//			metadataFile.close();
//
//			// 设置geoTransform 
//			metadata.geoTransform[0] = 0;  // 左上角X坐标 
//			metadata.geoTransform[1] = resolutionX;  // X方向分辨率 
//			metadata.geoTransform[2] = 0;  // 旋转项 
//			metadata.geoTransform[3] = 0;  // 左上角Y坐标 
//			metadata.geoTransform[4] = 0;  // 旋转项 
//			metadata.geoTransform[5] = resolutionY;  // Y方向分辨率(负值)
//
//		}
//		else {
//			qWarning() << "Failed to open metadata file:" << metadataPath;
//			continue;
//		}
//
//		// 2. 解析XML获取四角点坐标 
//		GeoCoordinates geoCoords;
//		QString xmlPath = this->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml");
//		if (QFile::exists(xmlPath)) {
//			// 调用parseXmlBoundary解析XML 
//			GeoBoundary screenCorners;
//			MeasurementareaShow::Measurementareainstance->parseXmlBoundary(xmlPath, screenCorners);
//
//			// 计算地理范围 
//			double minX = std::min({ screenCorners.corners[0].x(), screenCorners.corners[1].x(),
//								   screenCorners.corners[2].x(), screenCorners.corners[3].x() });
//			double maxX = std::max({ screenCorners.corners[0].x(), screenCorners.corners[1].x(),
//								   screenCorners.corners[2].x(), screenCorners.corners[3].x() });
//			double minY = std::min({ screenCorners.corners[0].y(), screenCorners.corners[1].y(),
//								   screenCorners.corners[2].y(), screenCorners.corners[3].y()});
//			double maxY = std::max({ screenCorners.corners[0].y(), screenCorners.corners[1].y(),
//								   screenCorners.corners[2].y(), screenCorners.corners[3].y()});
//
//			// 更新geoTransform的左上角坐标 
//			metadata.geoTransform[0] = minX;
//			metadata.geoTransform[3] = -maxY;
//		}
//		else {
//			qWarning() << "XML file not found:" << xmlPath;
//		}
//
//		//// 3. 加载RPC文件(如果存在)
//		QString rpcPath = filePath + fileInfo.completeBaseName() + "_rpc.txt";
//		if (QFile::exists(rpcPath)) {
//			char** rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
//			if (rpcMetadata != nullptr) {
//				metadata.rpcMetadata = CSLDuplicate(rpcMetadata);
//				CSLDestroy(rpcMetadata);
//			}
//			else {
//				qWarning() << "Failed to load RPC file:" << rpcPath;
//			}
//		}
//
//		// 4. 存储元数据 
//		s_imageMetadata.insert(tiffPath, metadata);
//
//		// 5. 更新表格中的分辨率显示 
//		QList<QTableWidgetItem*> items = ui.sateImageDataList_TableW->findItems(fileInfo.completeBaseName(), Qt::MatchExactly);
//		if (!items.isEmpty()) {
//			ui.sateImageDataList_TableW->item(items.first()->row(), 7)->setText(QString::number(metadata.geoTransform[1], 'f', 6));
//		}
//	}
//	return true;
//}

bool IMAGEPS::setGeoreferenceFromXMLAndRPC(const QStringList filenamePATH) {

	//processImageWithOperator(filenamePATH);
	//QObject::connect(systemConfig, &SystemConfig::ImageRangeFinished, this, [=]() {
		for (auto data : filenamePATH) {
			QFileInfo fileInfo(data);
			QString filePath = fileInfo.absolutePath() + "/";
			//QString originalExt = fileInfo.suffix().toLower();
			QString originalExt = fileInfo.suffix();

			QString tiffPath = filePath + fileInfo.completeBaseName() + "." + originalExt;
			QString rpcPath = filePath + fileInfo.completeBaseName() + QString::fromLocal8Bit("_rpc.txt");
			QString xmlPath = this->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml");

			QPolygonF screenCorners;
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

			// 按顺时针顺序获取四个角点
			screenCorners << QPointF(
				root.firstChildElement("UpperLeftLong").text().toDouble(),
				root.firstChildElement("UpperLeftLat").text().toDouble()
			);
			screenCorners << QPointF(
				root.firstChildElement("UpperRightLong").text().toDouble(),
				root.firstChildElement("UpperRightLat").text().toDouble()
			);
			screenCorners << QPointF(
				root.firstChildElement("LowerRightLong").text().toDouble(),
				root.firstChildElement("LowerRightLat").text().toDouble()
			);
			screenCorners << QPointF(
				root.firstChildElement("LowerLeftLong").text().toDouble(),
				root.firstChildElement("LowerLeftLat").text().toDouble()
			);

			GDALAllRegister();
			GDALDataset *dataset = (GDALDataset *)GDALOpen(tiffPath.toStdString().c_str(), GA_ReadOnly); // [修改] 改为只读模式 
			if (dataset == nullptr) {
				//qWarning() << u8"Failed to open image file!";
				continue;
			}

			// 4. 加载 RPC/RPB 文件（支持大小写）
			char** rpcMetadata = nullptr;
			QStringList rpbExtensions = { ".rpb", ".RPB" }; // 支持两种大小写格式 
			bool rpbLoaded = false;

			// 先尝试加载RPC文件 
			if (QFile::exists(rpcPath)) {
				rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
				if (rpcMetadata == nullptr) {
					qWarning() << u8"Failed to load RPC file:" << rpcPath;
				}
			}

			// 如果RPC加载失败，尝试加载各种RPB文件
			if (rpcMetadata == nullptr) {
				foreach(const QString &ext, rpbExtensions) {
					QString currentRpbPath = filePath + fileInfo.completeBaseName() + ext;
					if (QFile::exists(currentRpbPath)) {
						rpcMetadata = GDALLoadRPBFile(currentRpbPath.toStdString().c_str());
						if (rpcMetadata != nullptr) {
							qDebug() << u8"Successfully loaded RPB file:" << currentRpbPath;
							rpbLoaded = true;
							break; // 找到可用的就停止搜索 
						}
						else {
							qWarning() << u8"Failed to load RPB file:" << currentRpbPath;
						}
					}
				}
			}

			if (rpcMetadata == nullptr) {
				qWarning() << u8"Failed to load both RPC and RPB files! Checked paths:";
				qWarning() << "RPC:" << rpcPath;
				foreach(const QString &ext, rpbExtensions) {
					qWarning() << "RPB:" << filePath + fileInfo.completeBaseName() + ext;
				}
				//GDALClose(dataset);
				//continue;
			}

			//// 4. 加载 RPC 文件 
			//char **rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
			//if (rpcMetadata == nullptr) {
			//	qWarning() << u8"Failed to load RPC file!";
			//	GDALClose(dataset);
			//	continue;
			//}

			ImageGeoMetadata metadata;
			metadata.filePath = tiffPath;
			metadata.rpcMetadata = CSLDuplicate(rpcMetadata); // 复制RPC元数据 

			// 5. 计算 GeoTransform 
			double minX = std::min({ screenCorners[0].x(), screenCorners[1].x(),
									screenCorners[2].x(), screenCorners[3].x() });
			double maxX = std::max({ screenCorners[0].x(), screenCorners[1].x(),
									screenCorners[2].x(), screenCorners[3].x() });
			double minY = std::min({ screenCorners[0].y(), screenCorners[1].y(),
									screenCorners[2].y(), screenCorners[3].y() });
			double maxY = std::max({ screenCorners[0].y(), screenCorners[1].y(),
									screenCorners[2].y(), screenCorners[3].y() });

			metadata.Corners[0] = minX;  // 左上角 X（西）
			metadata.Corners[1] = maxY;  // 左上角 Y（北）
			metadata.Corners[2] = maxX;  // 右下角 X（东）
			metadata.Corners[3] = minY;  // 右下角 Y（南）

			QString metadataPath = this->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + ".metadata";

			// 初始化ImageGeoMetadata结构体 
			//ImageGeoMetadata metadata;
			metadata.filePath = tiffPath;
			QString projection;
			// 解析.metadata文件 
			double resolutionX = 0.0;
			double resolutionY = 0.0;
			QFile metadataFile(metadataPath);
			if (metadataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				QTextStream in(&metadataFile);


				bool hasProjection = false;

				while (!in.atEnd()) {
					QString line = in.readLine().trimmed();
					if (line.startsWith("resolutionX:")) {
						resolutionX = line.split(":")[1].trimmed().toDouble();
					}
					else if (line.startsWith("resolutionY:")) {
						resolutionY = line.split(":")[1].trimmed().toDouble();
					}
					else if (!hasProjection) {
						int projcsPos = line.indexOf("PROJCS[");
						int geogcsPos = line.indexOf("GEOGCS[");
						if (projcsPos != -1) {
							//qDebug() << line.mid(projcsPos).trimmed();

							metadata.projection = line.mid(projcsPos).trimmed();
							hasProjection = true;
						}
						else if (geogcsPos != -1) {
							metadata.projection = line.mid(geogcsPos).trimmed();
							//qDebug() << line.mid(geogcsPos).trimmed();

							hasProjection = true;
						}
					}
				}
				metadataFile.close();
			}

			//QList<QPointF> screenCornersF = parseXmlCoordinates(doc);
			//QList<QPointF> wgs84Corners;
			//transformCoordinates(screenCornersF, wgs84Corners, metadata.projection);
			//writeConvertedToXml(wgs84Corners, xmlPathDom);

			// 设置geoTransform 
			metadata.geoTransform[0] = minX;  // 左上角X坐标 
			metadata.geoTransform[1] = resolutionX;  // X方向分辨率 
			metadata.geoTransform[2] = 0;  // 旋转项 
			metadata.geoTransform[3] = maxY;  // 左上角Y坐标 
			metadata.geoTransform[4] = 0;  // 旋转项 
			metadata.geoTransform[5] = -resolutionY;  // Y方向分辨率(负值)

			QList<QTableWidgetItem*> items = ui.sateImageDataList_TableW->findItems(fileInfo.completeBaseName(), Qt::MatchExactly);

			if (!items.isEmpty()) {
				//ui.sateImageDataList_TableW->item(items.first()->row(), 7)->setText(QString::number(metadata.geoTransform[1], 'f', 9));
				int row = items.first()->row();
				QTableWidgetItem* item = ui.sateImageDataList_TableW->item(row, 7);

				// 如果item不存在，先创建它 
				if (item == nullptr) {
					item = new QTableWidgetItem();
					ui.sateImageDataList_TableW->setItem(row, 7, item);
				}

				// 只有当内容为空时才设置新值 
				if (item->text().isEmpty()) {
					item->setText(QString::number(metadata.geoTransform[1], 'f', 9));
				}
			}

			s_imageMetadata.insert(tiffPath, metadata);

			// 7. 清理资源 
			CSLDestroy(rpcMetadata);
			GDALClose(dataset);
		}
		return true;
	//});
	//return true;
}

bool IMAGEPS::setGeoreferenceFromXMLAndRPC(const QStringList& existingFiles, const QStringList& newFiles) {

	for (const QString& filePath : existingFiles) {
		QFileInfo fileInfo(filePath);
		QString filePathDir = fileInfo.absolutePath() + "/";
		QString originalExt = fileInfo.suffix();
		QString tiffPath = filePathDir + fileInfo.completeBaseName() + "." + originalExt;
		QString rpcPath = filePathDir + fileInfo.completeBaseName() + QString::fromLocal8Bit("_rpc.txt");
		QString xmlPath = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml");
		
		QPolygonF screenCorners;
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

		// 按顺时针顺序获取四个角点
		screenCorners << QPointF(
			root.firstChildElement("UpperLeftLong").text().toDouble(),
			root.firstChildElement("UpperLeftLat").text().toDouble()
		);
		screenCorners << QPointF(
			root.firstChildElement("UpperRightLong").text().toDouble(),
			root.firstChildElement("UpperRightLat").text().toDouble()
		);
		screenCorners << QPointF(
			root.firstChildElement("LowerRightLong").text().toDouble(),
			root.firstChildElement("LowerRightLat").text().toDouble()
		);
		screenCorners << QPointF(
			root.firstChildElement("LowerLeftLong").text().toDouble(),
			root.firstChildElement("LowerLeftLat").text().toDouble()
		);

		GDALAllRegister();
		GDALDataset *dataset = (GDALDataset *)GDALOpen(tiffPath.toStdString().c_str(), GA_ReadOnly); // [修改] 改为只读模式 
		if (dataset == nullptr) {
			//qWarning() << u8"Failed to open image file!";
			continue;
		}

		// 4. 加载 RPC/RPB 文件（支持大小写）
		char** rpcMetadata = nullptr;
		QStringList rpbExtensions = { ".rpb", ".RPB" }; // 支持两种大小写格式 
		bool rpbLoaded = false;

		// 先尝试加载RPC文件 
		if (QFile::exists(rpcPath)) {
			rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
			if (rpcMetadata == nullptr) {
				qWarning() << u8"Failed to load RPC file:" << rpcPath;
			}
		}

		// 如果RPC加载失败，尝试加载各种RPB文件
		if (rpcMetadata == nullptr) {
			foreach(const QString &ext, rpbExtensions) {
				QString currentRpbPath = filePath + fileInfo.completeBaseName() + ext;
				if (QFile::exists(currentRpbPath)) {
					rpcMetadata = GDALLoadRPBFile(currentRpbPath.toStdString().c_str());
					if (rpcMetadata != nullptr) {
						qDebug() << u8"Successfully loaded RPB file:" << currentRpbPath;
						rpbLoaded = true;
						break; // 找到可用的就停止搜索 
					}
					else {
						qWarning() << u8"Failed to load RPB file:" << currentRpbPath;
					}
				}
			}
		}

		if (rpcMetadata == nullptr) {
			qWarning() << u8"Failed to load both RPC and RPB files! Checked paths:";
			qWarning() << "RPC:" << rpcPath;
			foreach(const QString &ext, rpbExtensions) {
				qWarning() << "RPB:" << filePath + fileInfo.completeBaseName() + ext;
			}
			//GDALClose(dataset);
			//continue;
		}

		//// 4. 加载 RPC 文件 
		//char **rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
		//if (rpcMetadata == nullptr) {
		//	qWarning() << u8"Failed to load RPC file!";
		//	GDALClose(dataset);
		//	continue;
		//}

		ImageGeoMetadata metadata;
		metadata.filePath = tiffPath;
		metadata.rpcMetadata = CSLDuplicate(rpcMetadata); // 复制RPC元数据 

		// 5. 计算 GeoTransform 
		double minX = std::min({ screenCorners[0].x(), screenCorners[1].x(),
								screenCorners[2].x(), screenCorners[3].x() });
		double maxX = std::max({ screenCorners[0].x(), screenCorners[1].x(),
								screenCorners[2].x(), screenCorners[3].x() });
		double minY = std::min({ screenCorners[0].y(), screenCorners[1].y(),
								screenCorners[2].y(), screenCorners[3].y() });
		double maxY = std::max({ screenCorners[0].y(), screenCorners[1].y(),
								screenCorners[2].y(), screenCorners[3].y() });

		metadata.Corners[0] = minX;  // 左上角 X（西）
		metadata.Corners[1] = maxY;  // 左上角 Y（北）
		metadata.Corners[2] = maxX;  // 右下角 X（东）
		metadata.Corners[3] = minY;  // 右下角 Y（南）

		QString metadataPath = this->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + ".metadata";

		// 初始化ImageGeoMetadata结构体 
		//ImageGeoMetadata metadata;
		metadata.filePath = tiffPath;
		QString projection;
		// 解析.metadata文件 
		double resolutionX = 0.0;
		double resolutionY = 0.0;
		QFile metadataFile(metadataPath);
		if (metadataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			QTextStream in(&metadataFile);


			bool hasProjection = false;

			while (!in.atEnd()) {
				QString line = in.readLine().trimmed();
				if (line.startsWith("resolutionX:")) {
					resolutionX = line.split(":")[1].trimmed().toDouble();
				}
				else if (line.startsWith("resolutionY:")) {
					resolutionY = line.split(":")[1].trimmed().toDouble();
				}
				else if (!hasProjection) {
					int projcsPos = line.indexOf("PROJCS[");
					int geogcsPos = line.indexOf("GEOGCS[");
					if (projcsPos != -1) {
						//qDebug() << line.mid(projcsPos).trimmed();

						metadata.projection = line.mid(projcsPos).trimmed();
						hasProjection = true;
					}
					else if (geogcsPos != -1) {
						metadata.projection = line.mid(geogcsPos).trimmed();
						//qDebug() << line.mid(geogcsPos).trimmed();

						hasProjection = true;
					}
				}
			}
			metadataFile.close();
		}

		//QList<QPointF> screenCornersF = parseXmlCoordinates(doc);
		//QList<QPointF> wgs84Corners;
		//transformCoordinates(screenCornersF, wgs84Corners, metadata.projection);
		//writeConvertedToXml(wgs84Corners, xmlPathDom);

		// 设置geoTransform 
		metadata.geoTransform[0] = minX;  // 左上角X坐标 
		metadata.geoTransform[1] = resolutionX;  // X方向分辨率 
		metadata.geoTransform[2] = 0;  // 旋转项 
		metadata.geoTransform[3] = maxY;  // 左上角Y坐标 
		metadata.geoTransform[4] = 0;  // 旋转项 
		metadata.geoTransform[5] = -resolutionY;  // Y方向分辨率(负值)

		QList<QTableWidgetItem*> items = ui.sateImageDataList_TableW->findItems(fileInfo.completeBaseName(), Qt::MatchExactly);

		if (!items.isEmpty()) {
			//ui.sateImageDataList_TableW->item(items.first()->row(), 7)->setText(QString::number(metadata.geoTransform[1], 'f', 9));
			int row = items.first()->row();
			QTableWidgetItem* item = ui.sateImageDataList_TableW->item(row, 7);

			// 如果item不存在，先创建它 
			if (item == nullptr) {
				item = new QTableWidgetItem();
				ui.sateImageDataList_TableW->setItem(row, 7, item);
			}

			// 只有当内容为空时才设置新值 
			if (item->text().isEmpty()) {
				item->setText(QString::number(metadata.geoTransform[1], 'f', 9));
			}
		}

		s_imageMetadata.insert(tiffPath, metadata);

		// 7. 清理资源 
		CSLDestroy(rpcMetadata);
		GDALClose(dataset);
	}

	if (!newFiles.isEmpty() && !m_imageRangeSignalConnected) {
		QObject::connect(systemConfig, &SystemConfig::ImageRangeFinished, this, [=]() {
			for (const QString& filePath : newFiles) {
				QFileInfo fileInfo(filePath);
				QString filePath = fileInfo.absolutePath() + "/";
				//QString originalExt = fileInfo.suffix().toLower();
				QString originalExt = fileInfo.suffix();

				QString tiffPath = filePath + fileInfo.completeBaseName() + "." + originalExt;
				QString rpcPath = filePath + fileInfo.completeBaseName() + QString::fromLocal8Bit("_rpc.txt");
				QString xmlPath = this->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml");

				QPolygonF screenCorners;
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

				// 按顺时针顺序获取四个角点
				screenCorners << QPointF(
					root.firstChildElement("UpperLeftLong").text().toDouble(),
					root.firstChildElement("UpperLeftLat").text().toDouble()
				);
				screenCorners << QPointF(
					root.firstChildElement("UpperRightLong").text().toDouble(),
					root.firstChildElement("UpperRightLat").text().toDouble()
				);
				screenCorners << QPointF(
					root.firstChildElement("LowerRightLong").text().toDouble(),
					root.firstChildElement("LowerRightLat").text().toDouble()
				);
				screenCorners << QPointF(
					root.firstChildElement("LowerLeftLong").text().toDouble(),
					root.firstChildElement("LowerLeftLat").text().toDouble()
				);

				GDALAllRegister();
				GDALDataset *dataset = (GDALDataset *)GDALOpen(tiffPath.toStdString().c_str(), GA_ReadOnly); // [修改] 改为只读模式 
				if (dataset == nullptr) {
					//qWarning() << u8"Failed to open image file!";
					continue;
				}

				// 4. 加载 RPC/RPB 文件（支持大小写）
				char** rpcMetadata = nullptr;
				QStringList rpbExtensions = { ".rpb", ".RPB" }; // 支持两种大小写格式 
				bool rpbLoaded = false;

				// 先尝试加载RPC文件 
				if (QFile::exists(rpcPath)) {
					rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
					if (rpcMetadata == nullptr) {
						qWarning() << u8"Failed to load RPC file:" << rpcPath;
					}
				}

				// 如果RPC加载失败，尝试加载各种RPB文件
				if (rpcMetadata == nullptr) {
					foreach(const QString &ext, rpbExtensions) {
						QString currentRpbPath = filePath + fileInfo.completeBaseName() + ext;
						if (QFile::exists(currentRpbPath)) {
							rpcMetadata = GDALLoadRPBFile(currentRpbPath.toStdString().c_str());
							if (rpcMetadata != nullptr) {
								qDebug() << u8"Successfully loaded RPB file:" << currentRpbPath;
								rpbLoaded = true;
								break; // 找到可用的就停止搜索 
							}
							else {
								qWarning() << u8"Failed to load RPB file:" << currentRpbPath;
							}
						}
					}
				}

				if (rpcMetadata == nullptr) {
					qWarning() << u8"Failed to load both RPC and RPB files! Checked paths:";
					qWarning() << "RPC:" << rpcPath;
					foreach(const QString &ext, rpbExtensions) {
						qWarning() << "RPB:" << filePath + fileInfo.completeBaseName() + ext;
					}
					//GDALClose(dataset);
					//continue;
				}

				//// 4. 加载 RPC 文件 
				//char **rpcMetadata = GDALLoadRPCFile(rpcPath.toStdString().c_str());
				//if (rpcMetadata == nullptr) {
				//	qWarning() << u8"Failed to load RPC file!";
				//	GDALClose(dataset);
				//	continue;
				//}

				ImageGeoMetadata metadata;
				metadata.filePath = tiffPath;
				metadata.rpcMetadata = CSLDuplicate(rpcMetadata); // 复制RPC元数据 

				// 5. 计算 GeoTransform 
				double minX = std::min({ screenCorners[0].x(), screenCorners[1].x(),
										screenCorners[2].x(), screenCorners[3].x() });
				double maxX = std::max({ screenCorners[0].x(), screenCorners[1].x(),
										screenCorners[2].x(), screenCorners[3].x() });
				double minY = std::min({ screenCorners[0].y(), screenCorners[1].y(),
										screenCorners[2].y(), screenCorners[3].y() });
				double maxY = std::max({ screenCorners[0].y(), screenCorners[1].y(),
										screenCorners[2].y(), screenCorners[3].y() });

				metadata.Corners[0] = minX;  // 左上角 X（西）
				metadata.Corners[1] = maxY;  // 左上角 Y（北）
				metadata.Corners[2] = maxX;  // 右下角 X（东）
				metadata.Corners[3] = minY;  // 右下角 Y（南）

				QString metadataPath = this->projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + ".metadata";

				// 初始化ImageGeoMetadata结构体 
				//ImageGeoMetadata metadata;
				metadata.filePath = tiffPath;
				QString projection;
				// 解析.metadata文件 
				double resolutionX = 0.0;
				double resolutionY = 0.0;
				QFile metadataFile(metadataPath);
				if (metadataFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					QTextStream in(&metadataFile);


					bool hasProjection = false;

					while (!in.atEnd()) {
						QString line = in.readLine().trimmed();
						if (line.startsWith("resolutionX:")) {
							resolutionX = line.split(":")[1].trimmed().toDouble();
						}
						else if (line.startsWith("resolutionY:")) {
							resolutionY = line.split(":")[1].trimmed().toDouble();
						}
						else if (!hasProjection) {
							int projcsPos = line.indexOf("PROJCS[");
							int geogcsPos = line.indexOf("GEOGCS[");
							if (projcsPos != -1) {
								//qDebug() << line.mid(projcsPos).trimmed();

								metadata.projection = line.mid(projcsPos).trimmed();
								hasProjection = true;
							}
							else if (geogcsPos != -1) {
								metadata.projection = line.mid(geogcsPos).trimmed();
								//qDebug() << line.mid(geogcsPos).trimmed();

								hasProjection = true;
							}
						}
					}
					metadataFile.close();
				}

				//QList<QPointF> screenCornersF = parseXmlCoordinates(doc);
				//QList<QPointF> wgs84Corners;
				//transformCoordinates(screenCornersF, wgs84Corners, metadata.projection);
				//writeConvertedToXml(wgs84Corners, xmlPathDom);

				// 设置geoTransform 
				metadata.geoTransform[0] = minX;  // 左上角X坐标 
				metadata.geoTransform[1] = resolutionX;  // X方向分辨率 
				metadata.geoTransform[2] = 0;  // 旋转项 
				metadata.geoTransform[3] = maxY;  // 左上角Y坐标 
				metadata.geoTransform[4] = 0;  // 旋转项 
				metadata.geoTransform[5] = -resolutionY;  // Y方向分辨率(负值)

				QList<QTableWidgetItem*> items = ui.sateImageDataList_TableW->findItems(fileInfo.completeBaseName(), Qt::MatchExactly);

				if (!items.isEmpty()) {
					//ui.sateImageDataList_TableW->item(items.first()->row(), 7)->setText(QString::number(metadata.geoTransform[1], 'f', 9));
					int row = items.first()->row();
					QTableWidgetItem* item = ui.sateImageDataList_TableW->item(row, 7);

					// 如果item不存在，先创建它 
					if (item == nullptr) {
						item = new QTableWidgetItem();
						ui.sateImageDataList_TableW->setItem(row, 7, item);
					}

					// 只有当内容为空时才设置新值 
					if (item->text().isEmpty()) {
						item->setText(QString::number(metadata.geoTransform[1], 'f', 9));
					}
				}

				s_imageMetadata.insert(tiffPath, metadata);

				// 7. 清理资源 
				CSLDestroy(rpcMetadata);
				GDALClose(dataset);
			}
			return true;
		});
	}
	return true;
}
 
void IMAGEPS::attributeView_actionSLOT(int row, int column)
{
	QString filePath = "";
	// 获取是哪个表格触发的
	QTableWidget *table = qobject_cast<QTableWidget*>(sender());
	QString tableName = table->objectName();
	//qDebug() << u8"表名：" << tableName;  
	if (!table) return;
	// 获取第二列（索引为1）的数据
	QTableWidgetItem *item = table->item(row, 1);
	if (item) {
		QString filename = item->text();

		if (tableName == QString::fromLocal8Bit("sateImageDataList_TableW"))
		{
			for (auto data : DataModelPath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("refereDataList_TableW"))
		{
			if (filePath == "") {
				for (auto data : DOMFilePath)
				{
					QFileInfo fileInfo(data);
					if (filename == fileInfo.completeBaseName())
						filePath = data;
				}
			}
			if (filePath == "")
			{
				for (auto data : DEMFilePath)
				{
					QFileInfo fileInfo(data);
					if (filename == fileInfo.completeBaseName())
						filePath = data;
				}
			}

		}
		else if (tableName == QString::fromLocal8Bit("PyramidDataList_TableW"))
		{
			for (auto data : CreatepyramidPath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("imageInterList_TableW"))
		{
			for (auto data : ImageInterFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("dataList_TableW"))
		{
			for (auto data : FusionmodelFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("AlignmentAdjustmentList_TableW"))
		{
			for (auto data : AlignmentAdFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("AlignmentIntegrationList_TableW"))
		{
			for (auto data : AlignmentIntFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("objectPosPoints_TableW"))
		{
			for (auto data : DataModelPath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("DodgingDataList_TableW"))
		{
			for (auto data : DodgingFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("ImageMosaicDataList_TableW"))
		{
			for (auto data : ImageMosaicFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("SmartMosaicDataList_TableW"))
		{
			for (auto data : SmartMosaicFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("QuickMosaicDataList_TableW"))
		{
			for (auto data : QuickMosaicFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("ImagecropDataList_TableW"))
		{
			for (auto data : ImagecropFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("VectorfileDataList_TableW"))
		{
			for (auto data : VectorfileFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("AbsPositPrecCheckDataList_TableW"))
		{
			for (auto data : AbsPositPrecCheckFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("RelPositPrecCheckDataList_TableW"))
		{
			for (auto data : RelPositPrecCheckFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("CloudDetectionList_TableW"))
		{
			for (auto data : CloudDetectionListFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("CloudDetectionDataList_TableW"))
		{
			for (auto data : CloudDetectionDataListFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("ProjectionTransformationDataList_TableW"))
		{
			for (auto data : ProTransformationFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("FormatConversionDataList_TableW"))
		{
			for (auto data : FormatConversionFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}
		else if (tableName == QString::fromLocal8Bit("SARImageFilteringDataList_TableW"))
		{
			for (auto data : SARImageFilteringFilePath)
			{
				QFileInfo fileInfo(data);
				if (filename == fileInfo.completeBaseName())
					filePath = data;
			}
		}

		//// 清除之前的高亮 
		//ui.measureAreaShow_GLW->clearHighlights();
		//
		//// 高亮当前选中项
		//ui.measureAreaShow_GLW->highlightBoundary(filePath);

		// 从元数据缓存获取信息 
		ImageGeoMetadata* metadata = getImageMetadata(filePath);

		// 获取 dockWidget_4 的默认内容 widget（dockWidgetContents_4）
		QWidget *contentWidget = ui.dockWidget_4->widget();
		if (!contentWidget) {
			contentWidget = new QWidget(ui.dockWidget_4);
			ui.dockWidget_4->setWidget(contentWidget);
		}
		// 清除原有内容（保留 dockWidgetContents_4，仅清空其布局）
		QLayout *oldLayout = contentWidget->layout();
		if (oldLayout) {
			QLayoutItem *item;
			while ((item = oldLayout->takeAt(0)) != nullptr) {
				delete item->widget();
				delete item;
			}
			delete oldLayout;
		}
		// 注册 GDAL 驱动
		GDALAllRegister();
		// 打开 TIFF 文件
		GDALDataset *poDataset = (GDALDataset *)GDALOpen(filePath.toStdString().c_str(), GA_ReadOnly);
		if (!poDataset) {
			qDebug() << "无法打开文件：" << filePath;
			return;
		}
		// 获取基本信息
		int bandCount = poDataset->GetRasterCount();
		int width = poDataset->GetRasterXSize();
		int height = poDataset->GetRasterYSize();
		// 获取第一个波段的位深
		GDALRasterBand *poBand = poDataset->GetRasterBand(1);
		int bitDepth = GDALGetDataTypeSize(poBand->GetRasterDataType());
		// 获取投影信息
		//const char *pszProjection = poDataset->GetProjectionRef();
		//QString projection = metadata->projection.isEmpty() ?
		//	QString::fromLocal8Bit("无投影信息") : metadata->projection;
		QString projection;

		// 优先使用元数据中的投影信息 
		if (metadata && !metadata->projection.isEmpty()) {
			projection = metadata->projection;
		}
		else {
			// 回退到数据集自身的投影信息 
			const char* pszProjection = poDataset->GetProjectionRef();
			projection = (pszProjection && strlen(pszProjection) > 0) ?
				QString::fromUtf8(pszProjection) :
				QString::fromLocal8Bit("无投影信息");
		}

		double elevation = 0.0;
		if (metadata && metadata->rpcMetadata) {
			const char* pszHeightOffset = CSLFetchNameValue(metadata->rpcMetadata, "HEIGHT_OFF");
			if (pszHeightOffset) elevation = CPLAtof(pszHeightOffset);
		}
		else {
			// 获取高程信息（DEM 数据）
			double elevation = 0.0;
			bool hasElevation = false;
			if (!hasElevation) {
				char** papszRPC = poDataset->GetMetadata("RPC");
				if (papszRPC != nullptr) {
					const char* pszHeightOffset = CSLFetchNameValue(papszRPC, "HEIGHT_OFF");
					if (pszHeightOffset != nullptr) {
						elevation = CPLAtof(pszHeightOffset);
						hasElevation = true;
					}
				}
			}
		}
		// ===== 获取边界坐标和分辨率 =====
		double adfGeoTransform[6];
		bool hasGeoTransform = false;
		if (metadata) {
			adfGeoTransform[0] = metadata->geoTransform[0];
			adfGeoTransform[1] = metadata->geoTransform[1];
			adfGeoTransform[2] = metadata->geoTransform[2];
			adfGeoTransform[3] = metadata->geoTransform[3];
			adfGeoTransform[4] = metadata->geoTransform[4];
			adfGeoTransform[5] = metadata->geoTransform[5];
		}
		else if (poDataset) {
			hasGeoTransform = (poDataset->GetGeoTransform(adfGeoTransform) == CE_None);
		}

		double minX	 =  0.0;
		double maxY	 =  0.0;
		double maxX	 =  0.0;
		double minY	 =  0.0;
		if (metadata) {
			// 东西南北边界坐标
			minX = metadata->Corners[0];      // 左上角 X（西）
			maxY = metadata->Corners[1];      // 左上角 Y（北）
			maxX = metadata->Corners[2];     // 右下角 X（东）
			minY = metadata->Corners[3];     // 右下角 Y（南）
		}
		else
		{
			// 计算东西南北边界坐标
			minX = adfGeoTransform[0];                     // 左上角 X（西）
			maxY = adfGeoTransform[3];                     // 左上角 Y（北）
			maxX = minX + width * adfGeoTransform[1];     // 右下角 X（东）
			minY = maxY + height * adfGeoTransform[5];     // 右下角 Y（南）
		}

		// 获取东西/南北分辨率
		double resolutionX = adfGeoTransform[1];  // 东西方向分辨率（经度方向）
		double resolutionY = fabs(adfGeoTransform[5]);  // 南北方向分辨率（纬度方向，取正值）


		// ===== 创建 QTreeWidget =====
		QTreeWidget *treeWidget = new QTreeWidget(contentWidget);
		treeWidget->setColumnCount(2);
		treeWidget->setHeaderHidden(true);  // 隐藏表头
		treeWidget->setColumnWidth(0, 260);  // 第一列宽度
		treeWidget->setAlternatingRowColors(true);  // 交替行颜色
		treeWidget->setAttribute(Qt::WA_Hover);
		treeWidget->setMouseTracking(true);
		treeWidget->installEventFilter(this);
		treeWidget->setStyleSheet("QTreeWidget::item:hover { background-color: #E6F3FF; }");

		// ===== 1. 影像信息 =====
		QTreeWidgetItem *imageRoot = new QTreeWidgetItem(treeWidget, { QString::fromLocal8Bit("影像信息") });
		imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("宽度"), QString::number(width) }));
		imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("高度"), QString::number(height) }));
		imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("波段数"), QString::number(bandCount) }));
		imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("位深"), QString::number(bitDepth) }));
		if (metadata->geoTransform || hasGeoTransform) {
			imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("西"), QString("%1").arg(minX, 0, 'f', 9) }));
			imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("东"), QString("%1").arg(maxX, 0, 'f', 9) }));
			imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("南"), QString("%1").arg(minY, 0, 'f', 9) }));
			imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("北"), QString("%1").arg(maxY, 0, 'f', 9) }));
			imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("E分辨率"), QString::number(resolutionX, 'f', 9) }));
			imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("N分辨率"), QString::number(resolutionY, 'f', 9) }));
		}
		else {
			imageRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("边界坐标"), QString::fromLocal8Bit("无地理变换信息") }));
		}

		// ===== 2. 投影信息 =====
		QTreeWidgetItem *projRoot = new QTreeWidgetItem(treeWidget, { QString::fromLocal8Bit("投影信息") });
		projRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("投影"), projection }));
		projRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("影像路径"), filePath }));
		projRoot->addChild(new QTreeWidgetItem({ QString::fromLocal8Bit("平均高程"), QString::number(elevation, 'f', 2) }));

		// 展开所有项
		treeWidget->expandAll();
		// 设置布局（直接让 QTreeWidget 填满 dockWidgetContents_4）
		QVBoxLayout *mainLayout = new QVBoxLayout(contentWidget);
		mainLayout->addWidget(treeWidget);
		contentWidget->setLayout(mainLayout);
		// 关闭 GDAL 数据集
		GDALClose(poDataset);
	}
}

bool IMAGEPS::copyAndRenameRpcFiles(QStringList filenamesPath)
{
	QString sourceDir = projectdir + QString::fromLocal8Bit("SatBA") + "/" + "NewRPCs" + "/";
	bool allSuccess = true;

	for (auto data : filenamesPath)
	{
		QFileInfo fileInfo(data);

		QString destDir = fileInfo.absolutePath() + "/";
		// 检查源目录是否存在
		QDir sourceDirectory(sourceDir);
		if (!sourceDirectory.exists()) {
			qWarning() << "Source directory does not exist:" << sourceDir;
			return false;
		}
		// 检查目标目录是否存在，不存在则创建
		QDir destDirectory(destDir);
		if (!destDirectory.exists()) {
			if (!destDirectory.mkpath(".")) {
				qWarning() << "Failed to create destination directory:" << destDir;
				return false;
			}
		}
		// 获取所有匹配 *_rpc.txt 的文件
		QStringList filters;
		filters << "*_rpc.txt";
		QStringList files = sourceDirectory.entryList(filters, QDir::Files | QDir::Readable);
		if (files.isEmpty()) {
			qWarning() << "No *_rpc.txt files found in source directory";
			return false;
		}
		// 处理每个文件
		foreach(const QString &file, files) {
			QFileInfo sourceFileInfo(sourceDirectory, file);
			QString sourcePath = sourceFileInfo.absoluteFilePath();
			// 构建目标文件名（替换后缀为.ba）
			QString baseName = file;
			baseName.chop(4); // 移除".txt"
			QString destFileName = baseName + ".ba";
			QString destPath = destDirectory.filePath(destFileName);
			// 复制文件（会覆盖已存在的文件）
			if (QFile::exists(destPath)) {
				if (!QFile::remove(destPath)) {
					qWarning() << "Failed to remove existing file:" << destPath;
					allSuccess = false;
					continue;
				}
			}
			if (!QFile::copy(sourcePath, destPath)) {
				qWarning() << "Failed to copy file from" << sourcePath << "to" << destPath;
				allSuccess = false;
			}
			else {
				//qDebug() << "Copied and renamed:" << sourcePath << "->" << destPath;
			}
		}
	}
	return allSuccess;
}

/**
 * @brief 获取工程文件夹路径
 */
QString IMAGEPS::getProjectFolderPath(const QString& projectPath)
{
	// 1. 获取文件信息 
	QFileInfo fileInfo(projectPath);

	// 2. 检查路径有效性 
	if (!fileInfo.exists()) {
		qWarning() << "Project file does not exist:" << projectPath;
		return QString();
	}

	// 3. 构造新路径（替换扩展名为_Project）
	QString newPath = fileInfo.absolutePath() + "/" +
		fileInfo.completeBaseName() + "_Project";

	// 4. 确保路径分隔符统一（可选）
	newPath = QDir::toNativeSeparators(newPath);

	// 示例输出：D:/CokeWork/VsProject/IMAGEPS/IMAGEPS/123_Project 
	qDebug() << "Project folder path:" << newPath;

	return newPath;
}


bool IMAGEPS::createTaskFile(const QString& inputImagePath) {
	// 创建XML文档 
	QDomDocument doc;
	QDomProcessingInstruction header = doc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
	doc.appendChild(header);

	// 创建根元素 
	QDomElement root = doc.createElement("XQImageRangeCmd");
	root.setAttribute("version", "1.0");
	doc.appendChild(root);

	// 添加输入文件路径元素 
	QDomElement inputElement = doc.createElement("InputImageFilePath");
	inputElement.setAttribute("type", "string");
	//QString tmp = inputImagePath;
	QString tmp = QDir::cleanPath(inputImagePath);
	QDomText inputText = doc.createTextNode(tmp);
	inputElement.appendChild(inputText);
	root.appendChild(inputElement);

	// 添加输出文件路径元素 
	QDomElement outputElement = doc.createElement("OutputImageRangeFilePath");
	outputElement.setAttribute("type", "string");
	QFileInfo fileInfo(tmp);
	QString outputPath = QDir(projectdir).filePath("Temp/ImageInfo");
	outputPath = QDir(outputPath).filePath(fileInfo.completeBaseName() + ".xml");
	QDomText outputText = doc.createTextNode(outputPath);
	outputElement.appendChild(outputText);
	root.appendChild(outputElement);

	// 保存到文件 
	QFile file(projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".task"));
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}

	QTextStream stream(&file);
	stream.setCodec("UTF-8");
	doc.save(stream, 4, QDomNode::EncodingFromDocument);
	file.close();

	return true;
}

///**
// * @brief 图像坐标,投影,分辨率获取函数
// */
//bool IMAGEPS::processImageWithOperator(const QStringList& filenames)
//{
//	bool allSuccess = true;
//
//	for (const QString &filePath : filenames) {
//		QFileInfo fileInfo(filePath);
//		QString baseName = fileInfo.completeBaseName();
//		QString XMLpath = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".xml");
//		QString XMLpath_task = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".task");
//		QString moniFile = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".moni");
//
//		// 检查文件是否已存在且有效 
//		if (QFile::exists(XMLpath) && QFile::exists(moniFile)) {
//			continue;  // 文件已存在，无需处理
//		}
//
//		// 创建新的任务文件
//		if (!createTaskFile(filePath)) {
//			allSuccess = false;
//			continue;
//		}
//
//		// 创建并启动进程 
//		QProcess process;
//		//QString program = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageRangeCmd.exe");
//#ifdef Q_OS_LINUX 
//		QString program = this->appDirPath + QString::fromLocal8Bit("/linux64/PSImageRangeCmd.x");
//#else
//		QString program = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageRangeCmd.exe");
//#endif
//		QStringList arguments;
//		arguments << XMLpath_task;
//
//		// 启动进程
//		process.start(program, arguments);
//		if (!process.waitForStarted()) {
//			allSuccess = false;
//			continue;
//		}
//
//		// 等待进程完成 
//		if (!process.waitForFinished()) {
//			allSuccess = false;
//			continue;
//		}
//
//		// 检查结果文件是否生成
//		QFile file(moniFile);
//		int maxWaitTime = 5000; // 5秒
//		int checkInterval = 20; // 0.02秒 
//		int elapsed = 0;
//		while (!file.exists() && elapsed < maxWaitTime) {
//			QThread::msleep(checkInterval);
//			elapsed += checkInterval;
//			QCoreApplication::processEvents();
//		}
//
//		if (!file.exists()) {
//			allSuccess = false;
//		}
//	}
//
//	return allSuccess;
//}

/**
 * @brief 图像坐标,投影,分辨率获取函数
 */
bool IMAGEPS::processImageWithOperator(const QStringList& filenames)
{
	bool allSuccess = true;

	// 初始化任务队列和相关参数
	//systemConfig->m_taskQueue.clear();
	//systemConfig->m_runningProcesses.store(0);
	//systemConfig->m_completedTasks.store(0);
	//systemConfig->m_startTime = QDateTime::currentDateTime();
	//systemConfig->m_totalTasks = filenames.size();

	for (const QString &filePath : filenames) {
		QFileInfo fileInfo(filePath);
		QString baseName = fileInfo.completeBaseName();
		QString XMLpath = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".xml");
		QString XMLpath_task = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".task");
		QString moniFile = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".moni");

		// 检查文件是否已存在且有效 
		if (QFile::exists(XMLpath) && QFile::exists(moniFile)) {
			continue;  // 文件已存在，无需处理
		}

		// 创建新的任务文件
		if (!createTaskFile(filePath)) {
			allSuccess = false;
			continue;
		}

		// 将任务添加到队列
		SystemConfig::TaskInfo task;
		task.taskFilePath = XMLpath_task;
		//task.operatorName = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSOrthoCmd.exe");
#ifdef Q_OS_LINUX 
		task.operatorName = this->appDirPath + QString::fromLocal8Bit("/linux64/PSImageRangeCmd.x");
#else
		task.operatorName = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageRangeCmd.exe");
#endif
		task.outfilename = XMLpath;
		systemConfig->m_taskQueueIR.enqueue(task);
		systemConfig->m_totalTasksIR++;
	}
	disconnect(systemConfig, &SystemConfig::ImageRangeFinished, nullptr, nullptr);
	PROJECT_LOG_INFO(this->CurrentConfig, QString::fromLocal8Bit("影像范围计算算子开始执行......"));
	systemConfig->logEdit = ui.runLog_Edit;
	ui.log_TabW->setCurrentIndex(0);
	systemConfig->startNextTasksIRange(QString::fromLocal8Bit("****影像范围计算"), false);
	return allSuccess;
}

QPair<QStringList, QStringList> IMAGEPS::processImageWithOperator_test(const QStringList& filenames)
{
	QStringList existingFiles;  // 已有XML的文件列表
	QStringList newFiles;       // 需要处理的文件列表

	for (const QString &filePath : filenames) {
		QFileInfo fileInfo(filePath);
		QString baseName = fileInfo.completeBaseName();
		QString XMLpath = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".xml");
		QString moniFile = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".moni");

		// 检查文件是否已存在且有效
		if (QFile::exists(XMLpath) && QFile::exists(moniFile)) {
			existingFiles.append(filePath);
		}
		else {
			newFiles.append(filePath);

			// 创建新的任务文件 
			QString XMLpath_task = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".task");
			if (!createTaskFile(filePath)) {
				continue;
			}

			// 将任务添加到队列 
			SystemConfig::TaskInfo task;
			task.taskFilePath = XMLpath_task;
#ifdef Q_OS_LINUX 
			task.operatorName = this->appDirPath + QString::fromLocal8Bit("/linux64/PSImageRangeCmd.x");
#else 
			task.operatorName = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageRangeCmd.exe");
#endif 
			task.outfilename = XMLpath;
			systemConfig->m_taskQueueIR.enqueue(task);
			systemConfig->m_totalTasksIR++;
		}
	}

	// 如果有新文件需要处理，启动任务
	if (!newFiles.isEmpty()) {
		//disconnect(systemConfig, &SystemConfig::ImageRangeFinished, nullptr, nullptr);
		PROJECT_LOG_INFO(this->CurrentConfig, QString::fromLocal8Bit("影像范围计算算子开始执行......"));
		systemConfig->logEdit = ui.runLog_Edit;
		ui.log_TabW->setCurrentIndex(0);
		systemConfig->startNextTasksIRange(QString::fromLocal8Bit("****影像范围计算"), false);
	}

	return qMakePair(existingFiles, newFiles);
}

void IMAGEPS::on_actionOpenImage(QStringList filenames, const QString flag) {
	if (filenames.isEmpty())
		return;

	QColor currentColor;

	if (flag == u8"原始影像")
		currentColor = Qt::white;
	else if(flag == "DEM")
		currentColor = Qt::blue;
	else if(flag == "DOM")
		currentColor = Qt::green;

	//QObject::connect(systemConfig, &SystemConfig::ImageRangeFinished, this, [=]() {
		for (const QString &filePath : filenames) {
			QFileInfo fileInfo(filePath);
			QString baseName = fileInfo.completeBaseName();
			QString XMLpath = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".xml");
			QString XMLpath_task = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".task");
			QString moniFile = projectdir + QString::fromLocal8Bit("Temp/ImageInfo/") + baseName + QString::fromLocal8Bit(".moni");

			// 检查文件是否已存在且有效 
			if (QFile::exists(XMLpath) && QFile::exists(moniFile)) {
				// 直接加载现有文件 
				ui.ImageInfoShow_Widget->addGeoBoundary(filePath, currentColor);
				//qDebug() << "使用缓存文件加载：" << baseName;
				//continue;
			}

//			// 创建新的任务文件 
//			createTaskFile(filePath);
//
//			// 创建并启动进程
//			QProcess *process1 = new QProcess(this);
//#ifdef Q_OS_LINUX 
//			QString program = this->appDirPath + QString::fromLocal8Bit("/linux64/PSImageRangeCmd.x");
//#else
//			QString program = this->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageRangeCmd.exe");
//#endif
//			QStringList arguments;
//			arguments << XMLpath_task;
//
//			// 启动进程 
//			process1->start(program, arguments);
//			m_childProcesses.append(process1);
//			// 进程结束时的处理 
//			connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
//				[=](int exitCode, QProcess::ExitStatus status) {
//				QFile file(moniFile);
//				m_childProcesses.removeOne(process1);
//
//				if (file.exists()) {
//					ui.ImageInfoShow_Widget->addGeoBoundary(filePath, currentColor);
//				
//				}
//				else {
//					qWarning() << u8"超时等待或文件未生成：" << moniFile;
//				}
//				process1->deleteLater();
//			});
		}

		ui.ImageInfoShow_Widget->update();
	//});
}

void IMAGEPS::on_actionOpenImage(const QStringList& existingFiles, const QStringList& newFiles, const QString flag)
{
	if (existingFiles.isEmpty() && newFiles.isEmpty()) {
		return;
	}
	m_isProcessingImageOpen = true;

	// 确定当前颜色
	QColor currentColor;
	if (flag == u8"原始影像") {
		currentColor = Qt::white;
	}
	else if (flag == "DEM") {
		currentColor = Qt::blue;
	}
	else if (flag == "DOM") {
		currentColor = Qt::green;
	}
	else {
		currentColor = Qt::gray;  // 默认颜色
	}
	m_currentColor = currentColor;
	QRgb currentRgb = currentColor.rgba();

	// 第一次调用时连接信号
	if (!m_imageRangeSignalConnected) {
		QObject::connect(systemConfig, &SystemConfig::ImageRangeFinished, this, [this]() {

			// 首先处理m_pendingFiles中的项并添加到显示控件 
			QHash<QRgb, QStringList> tempFiles = m_pendingFiles;
			QHashIterator<QRgb, QStringList> it(tempFiles);
			while (it.hasNext()) {
				it.next();
				QColor color = QColor::fromRgba(it.key());
				const QStringList& files = it.value();

				for (const QString& filePath : files) {
					ui.ImageInfoShow_Widget->addGeoBoundary(filePath, color);
					m_pendingFiles[color.rgba()].removeAll(filePath);

					if (m_pendingFiles[color.rgba()].isEmpty()) {
						m_pendingFiles.remove(color.rgba());
					}
					if (color == Qt::white) {
						QStringList filepathlist;
						filepathlist.push_back(filePath);
						setGeoreferenceFromXMLAndRPC(filepathlist);
					}
				}
			}
			ui.ImageInfoShow_Widget->filterReferenceBoundaries();
			ui.ImageInfoShow_Widget->calculateWorldExtent();
			ui.ImageInfoShow_Widget->zoomToFullExtent();
			ui.ImageInfoShow_Widget->update();

			ui.refereDataList_TableW->setUpdatesEnabled(false);

			// 获取需要保留的项（按颜色分组）
			QHash<QRgb, QStringList> retainRefs = ui.ImageInfoShow_Widget->getRetainReferencesSet();

			QStringList firstColumnDOM;
			QStringList firstColumnDEM;
			// 遍历表格并删除不需要的行（反向遍历避免索引问题）
			for (int row = ui.refereDataList_TableW->rowCount() - 1; row >= 0; --row) {
				QTableWidgetItem* fileItem = ui.refereDataList_TableW->item(row, 1); // 文件名列
				QTableWidgetItem* typeItem = ui.refereDataList_TableW->item(row, 2); // 类型列(DOM/DEM)

				if (fileItem && typeItem) {
					QString fileName = fileItem->text();
					QString type = typeItem->text();
					// 确定当前颜色
					QColor currentColortmp;
					if (type == "DEM") {
						currentColortmp = Qt::blue;
					}
					else if (type == "DOM") {
						currentColortmp = Qt::green;
					}
					else {
						currentColortmp = Qt::gray;  // 默认颜色
					}

					QRgb expectedColor = currentColortmp.rgba();

					// 检查是否在保留列表中
					bool shouldKeep = false;
					if (retainRefs.contains(expectedColor)) {
						shouldKeep = retainRefs[expectedColor].contains(fileName);
					}

					// 如果不在保留列表中，则删除该行
					if (!shouldKeep) {
						ui.refereDataList_TableW->removeRow(row);
						if (type == QString::fromLocal8Bit("DOM")) {
							firstColumnDOM.append(fileName);
							//for (auto filenamePATH : DOMFilePath)
							//{
							//	QFileInfo fileInfo(filenamePATH);
							//	if (fileInfo.completeBaseName() == fileName)
							//	{
							//		DOMFilePath.removeAll(filenamePATH);
							//	}
							//}
						}
						else if (type == QString::fromLocal8Bit("DEM")) {
							firstColumnDEM.append(fileName);
							//for (auto filenamePATH : DEMFilePath)
							//{
							//	QFileInfo fileInfo(filenamePATH);
							//	if (fileInfo.completeBaseName() == fileName)
							//	{
							//		DEMFilePath.removeAll(filenamePATH);
							//	}
							//}
						}
					}
				}
			}

			for (auto filename : firstColumnDOM)
			{
				for (auto filenamePATH : DOMFilePath)
				{
					QFileInfo fileInfo(filenamePATH);
					if (fileInfo.completeBaseName() == filename)
					{
						DOMFilePath.removeAll(filenamePATH);
					}
				}
			}
			for (auto filename : firstColumnDEM)
			{
				for (auto filenamePATH : DEMFilePath)
				{
					QFileInfo fileInfo(filenamePATH);
					if (fileInfo.completeBaseName() == filename)
					{
						DEMFilePath.removeAll(filenamePATH);
					}
				}
			}

			// 重新排序第一列的序号（正向遍历）
			for (int row = 0; row < ui.refereDataList_TableW->rowCount(); ++row) {
				if (QTableWidgetItem* indexItem = ui.refereDataList_TableW->item(row, 0)) {
					indexItem->setText(QString::number(row + 1)); // 从1开始编号 
				}
			}

			ui.refereDataList_TableW->setUpdatesEnabled(true);
			m_isProcessingImageOpen = false;

		});
		m_imageRangeSignalConnected = true;
	}
	// 处理新文件 
	if (!newFiles.isEmpty()) {
		if (!m_pendingFiles.contains(currentRgb)) {
			m_pendingFiles.insert(currentRgb, QStringList());
		}
		m_pendingFiles[currentRgb].append(newFiles);
		if (!existingFiles.isEmpty()) {
			m_pendingFiles[currentRgb].append(existingFiles);
		}
	}
	else {
		if (!m_pendingFiles.contains(currentRgb)) {
			m_pendingFiles.insert(currentRgb, QStringList());
		}
		m_pendingFiles[currentRgb].append(existingFiles);
		emit systemConfig->ImageRangeFinished();
	}

	ui.ImageInfoShow_Widget->update();
}

void IMAGEPS::on_actionOpenImageShow(QString filenamePATH)
{
	if (!filenamePATH.isEmpty()) {
		
		QFileInfo fileInfo(filenamePATH);
		QString baseName = fileInfo.completeBaseName();

		ui.tabWidget->setTabText(1, baseName);
		//ui.dockWidget_9->setWindowTitle(baseName + QString::fromUtf8(u8" - 遥感图像浏览器"));

		//ui.tabWidget->setCurrentIndex(1);

		//ui.dockWidget_9->show();
		//ui.dockWidget_9->raise();

		// 加载图像到OpenGL窗口 
		ui.openGLWidget->loadImage(filenamePATH);
	}
	else {
		QMessageBox::warning(this, tr(u8"警告"), tr(u8"未选择有效图像文件"));
	}
}

//void IMAGEPS::on_actionOpenImageShow() {
//	// 1. 文件选择对话框 
//	QStringList filePaths = QFileDialog::getOpenFileNames(
//		this,
//		tr(u8"打开遥感图像文件（支持DEM/DOM/多光谱/全色）"),
//		"",
//		tr(u8"遥感图像 (*.tif *.tiff *.img);;所有文件 (*.*)")
//	);
//
//	if (filePaths.isEmpty())   return;
//
//	// 2. 清除旧图像 
//	ui.Image_show->clearImages();
//
//	// 3. 加载新图像（仅处理第一个文件，符合"一次显示一种图像"需求）
//	const QString& filePath = filePaths.first();
//	QFileInfo fileInfo(filePath);
//
//	// 更新UI显示 
//	ui.tabWidget->setTabText(1, fileInfo.completeBaseName());
//	ui.dockWidget_9->setWindowTitle(fileInfo.completeBaseName() +
//		QString::fromUtf8(u8" - 遥感图像浏览器"));
//
//	// 4. 调用新增的显示函数（自动识别类型）
//	ui.Image_show->displayImage(filePath);
//}

void IMAGEPS::onTabChanged(int index)
{
	if (index == 0) { // 测区显示标签页
		ui.ImageInfoShow_Widget->update();
	}
	else if (index == 1) { // 图像显示标签页
		ui.openGLWidget->update();
	}
}

QString IMAGEPS::getProjectDir(const QString& fullPath)
{
	QFileInfo fileInfo(fullPath);
	// 获取绝对路径并转到上一级目录 
	QString dirPath = fileInfo.absoluteDir().absolutePath();  // 先去掉文件名 
	QDir dir(dirPath);
	dir.cdUp();  // 再向上一级（去掉SatTiePointMatch）
	//return dir.absolutePath() + "/"; // 确保以/结尾
	return dirPath; // 确保以/结尾
}

/**
 * @brief 构建 PSIntersectObjCmd.task  文件
 * @return 成功返回true，失败返回false
 */
bool IMAGEPS::buildPSIntersectObjCmdFile()
{
	systemConfig->FreeNetworkAdjustmergeFile();//序列化平差连接点文件
	// 创建 QDomDocument 
	QDomDocument doc;

	// 添加 XML 声明 
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 <XQIntersectObjCmd>
	QDomElement root = doc.createElement("XQIntersectObjCmd");

	// 添加基本参数节点 
	QDomElement tieFile = doc.createElement("TieFile");
	tieFile.appendChild(doc.createTextNode(projectdir + "SatBA" + "/" + "PSBundle.tie"));
	root.appendChild(tieFile);

	// 创建 <DEM> 子节点
	QDomElement DEM = doc.createElement("DEM");
	QString exeDir = QCoreApplication::applicationDirPath();
	QDir dir(exeDir);
	QString tmpPathdir = dir.absolutePath();

	// 从配置文件中读取DEM路径 
	QString demConfigPath = QString::fromLocal8Bit("../bin/config/projectPathConfig/globaldemORinputdem.txt");   // 配置文件路径 
	//QString demFilePath = tmpPathdir + "/Software/etc/globaldem/globaldem.tif";   // 默认路径 

	//// 尝试从配置文件读取 
	//QFile demConfigFile(demConfigPath);
	//if (demConfigFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
	//	QTextStream in(&demConfigFile);
	//	QString configLine = in.readLine().trimmed();
	//	if (!configLine.isEmpty()) {
	//		demFilePath = configLine;
	//	}
	//	demConfigFile.close();
	//}

	bool demFilePath = systemConfig->getGlobalDEM();

	if (!demFilePath)
	{
		if (!DEMFilePath.empty()) {
			for (auto data : DEMFilePath)
			{
				QDomElement DEM = doc.createElement("DEM");
				QDomText DEMText = doc.createTextNode(data);
				DEM.appendChild(DEMText);
				root.appendChild(DEM);
			}
		}
		else {
			QDomElement DEM = doc.createElement("DEM");
			QString exeDir = QCoreApplication::applicationDirPath();
			QDir dir(exeDir);
			QString tmpPath = dir.absolutePath();

			QDomText DEMText = doc.createTextNode(tmpPath + "/Software/etc/globaldem/globaldem.tif");
			DEM.appendChild(DEMText);
			root.appendChild(DEM);
		}
	}
	else
	{
		QDomElement DEM = doc.createElement("DEM");
		QString exeDir = QCoreApplication::applicationDirPath();
		QDir dir(exeDir);
		QString tmpPath = dir.absolutePath();

		QDomText DEMText = doc.createTextNode(tmpPath + "/Software/etc/globaldem/globaldem.tif");
		DEM.appendChild(DEMText);
		root.appendChild(DEM);
	}

	QDomElement outputObjFile = doc.createElement("OutputObjFilePath");
	outputObjFile.appendChild(doc.createTextNode(projectdir + "SatTiePointMatch" + "/" + "obj.txt"));
	root.appendChild(outputObjFile);

	// 解析SatModelMakerCmdout.xml 文件获取模型信息 
	QDomDocument cmdoutDoc;
	QFile cmdoutFile(projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/" + "SatModelMakerCmdout.xml");
	if (!cmdoutFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open SatModelMakerCmdout.xml";
		return false;
	}
	if (!cmdoutDoc.setContent(&cmdoutFile)) {
		cmdoutFile.close();
		qDebug() << "Failed to parse SatModelMakerCmdout.xml";
		return false;
	}
	cmdoutFile.close();

	// 获取模型数量 
	QDomElement cmdoutRoot = cmdoutDoc.documentElement();
	QDomElement modelNumElement = cmdoutRoot.firstChildElement("ModelNum");
	int modelNum = modelNumElement.text().toInt();

	QDomElement numOfModels = doc.createElement("NumOfModels");
	numOfModels.setAttribute("type", "int32");
	numOfModels.appendChild(doc.createTextNode(QString::number(modelNum)));
	root.appendChild(numOfModels);

	// 创建Models节点
	QDomElement modelsElement = doc.createElement("Models");

	// 遍历所有模型 
	QDomElement cmdoutModels = cmdoutRoot.firstChildElement("Models");
	QDomElement cmdoutModel = cmdoutModels.firstChildElement();
	int modelIndex = 0;

	while (!cmdoutModel.isNull()) {
		QDomElement modelElement = doc.createElement(QString("Model_%1").arg(modelIndex));

		// 处理NadImage (全色影像)
		QDomElement nadImageName = cmdoutModel.firstChildElement("NadImageName");
		if (!nadImageName.isNull()) {
			QDomElement imageElement = doc.createElement("Image");

			// ImageID
			QDomElement imageId = doc.createElement("ImageID");
			imageId.setAttribute("type", "int32");
			imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("NadImageIndex").text()));
			imageElement.appendChild(imageId);

			// ImageInfoFile (.nfo文件)
			QString nadImagePath = nadImageName.text();
			nadImagePath.replace(".tiff", ".nfo");
			nadImagePath.replace(".TIFF", ".nfo");
			nadImagePath.replace(".tif", ".nfo");
			nadImagePath.replace(".TIF", ".nfo");
			nadImagePath.replace(".til", ".nfo");
			nadImagePath.replace(".TIL", ".nfo");
			QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
			imageInfoFile.appendChild(doc.createTextNode(nadImagePath));
			imageElement.appendChild(imageInfoFile);

			// SensorType (全色影像对应0)
			QDomElement sensorType = doc.createElement("SensorType");
			sensorType.setAttribute("type", "int32");
			sensorType.appendChild(doc.createTextNode("0"));
			imageElement.appendChild(sensorType);

			modelElement.appendChild(imageElement);
		}

		// 处理FwdImage (如果有)   
		QDomElement fwdImageName = cmdoutModel.firstChildElement("FwdImageName");
		if (!fwdImageName.isNull()) {
			QDomElement imageElement = doc.createElement("Image");
			QDomElement imageId = doc.createElement("ImageID");
			imageId.setAttribute("type", "int32");
			imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("FwdImageIndex").text()));
			imageElement.appendChild(imageId);
			// 将.tiff替换为.nfo
			QString fwdImagePath = fwdImageName.text();
			fwdImagePath.replace(".tiff", ".nfo");
			fwdImagePath.replace(".TIFF", ".nfo");
			fwdImagePath.replace(".tif", ".nfo");
			fwdImagePath.replace(".TIF", ".nfo");
			fwdImagePath.replace(".til", ".nfo");
			fwdImagePath.replace(".TIL", ".nfo");
			QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
			imageInfoFile.appendChild(doc.createTextNode(fwdImagePath));
			imageElement.appendChild(imageInfoFile);

			QDomElement sensorType = doc.createElement("SensorType");
			sensorType.setAttribute("type", "int32");
			sensorType.appendChild(doc.createTextNode("1")); // FwdImage对应1
			imageElement.appendChild(sensorType);
			modelElement.appendChild(imageElement);
		}

		// 处理BwdImage (如果有)   
		QDomElement bwdImageName = cmdoutModel.firstChildElement("BwdImageName");
		if (!bwdImageName.isNull()) {
			QDomElement imageElement = doc.createElement("Image");
			QDomElement imageId = doc.createElement("ImageID");
			imageId.setAttribute("type", "int32");
			imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("BwdImageIndex").text()));
			imageElement.appendChild(imageId);
			// 将.tiff替换为.nfo
			QString bwdImagePath = bwdImageName.text();
			bwdImagePath.replace(".tiff", ".nfo");
			bwdImagePath.replace(".TIFF", ".nfo");
			bwdImagePath.replace(".tif", ".nfo");
			bwdImagePath.replace(".TIF", ".nfo");
			bwdImagePath.replace(".til", ".nfo");
			bwdImagePath.replace(".TIL", ".nfo");
			QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
			imageInfoFile.appendChild(doc.createTextNode(bwdImagePath));
			imageElement.appendChild(imageInfoFile);

			QDomElement sensorType = doc.createElement("SensorType");
			sensorType.setAttribute("type", "int32");
			sensorType.appendChild(doc.createTextNode("2")); // BwdImage对应2
			imageElement.appendChild(sensorType);
			modelElement.appendChild(imageElement);
		}

		// 处理MulImage (多光谱影像)
		QDomElement mulImageName = cmdoutModel.firstChildElement("MulImageName");
		if (!mulImageName.isNull()) {
			QDomElement imageElement = doc.createElement("Image");

			// ImageID 
			QDomElement imageId = doc.createElement("ImageID");
			imageId.setAttribute("type", "int32");
			imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("MulImageIndex").text()));
			imageElement.appendChild(imageId);

			// ImageInfoFile (.nfo文件)
			QString mulImagePath = mulImageName.text();
			mulImagePath.replace(".tiff", ".nfo");
			mulImagePath.replace(".TIFF", ".nfo");
			mulImagePath.replace(".tif", ".nfo");
			mulImagePath.replace(".TIF", ".nfo");
			mulImagePath.replace(".til", ".nfo");
			mulImagePath.replace(".TIL", ".nfo");
			QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
			imageInfoFile.appendChild(doc.createTextNode(mulImagePath));
			imageElement.appendChild(imageInfoFile);

			// SensorType (多光谱影像对应3)
			QDomElement sensorType = doc.createElement("SensorType");
			sensorType.setAttribute("type", "int32");
			sensorType.appendChild(doc.createTextNode("3"));
			imageElement.appendChild(sensorType);

			modelElement.appendChild(imageElement);
		}

		modelsElement.appendChild(modelElement);
		cmdoutModel = cmdoutModel.nextSiblingElement();
		modelIndex++;
	}

	root.appendChild(modelsElement);
	doc.appendChild(root);

	// 写入文件
	QFile file(projectdir + "SatTiePointMatch" + "/" + "PSIntersectObjCmd.task");
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qDebug() << "Failed to open output file";
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << doc.toString(4);   // 4 表示缩进 4 个空格，使 XML 可读性更好
	file.close();

	return true;
}

bool IMAGEPS::buildPSIntersectObjCmdFile(QString filePath, QStringList tieFilePattern)
{
	systemConfig->FreeNetworkAdjustmergeFile(filePath, tieFilePattern);//序列化平差连接点文件
	// 创建 QDomDocument 
	QDomDocument doc;

	// 添加 XML 声明 
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 <XQIntersectObjCmd>
	QDomElement root = doc.createElement("XQIntersectObjCmd");

	// 添加基本参数节点 
	QDomElement tieFile = doc.createElement("TieFile");
	//tieFile.appendChild(doc.createTextNode(projectdir + "SatBA" + "/" + "PSBundle.tie"));
	tieFile.appendChild(doc.createTextNode(filePath + "/PSBundle.tie"));
	root.appendChild(tieFile);

	//for (auto data : DEMFilePath)
	//{
	//	QDomElement dem = doc.createElement("DEM");
	//	dem.appendChild(doc.createTextNode(data));
	//	root.appendChild(dem);
	//}
	//QString exeDir = QCoreApplication::applicationDirPath();
	//QDir dir(exeDir);
	//QString tmpPath = dir.absolutePath();
	//QDomElement dem = doc.createElement("DEM");
	//dem.appendChild(doc.createTextNode(tmpPath + "\\Software\\etc\\globaldem\\globaldem.tif"));
	//root.appendChild(dem);
	//QDomElement dem = doc.createElement("DEM");
	//dem.appendChild(doc.createTextNode(demFilePath));
	//root.appendChild(dem);

		// 创建 <DEM> 子节点
	QDomElement DEM = doc.createElement("DEM");
	QString exeDir = QCoreApplication::applicationDirPath();
	QDir dir(exeDir);
	QString tmpPathdir = dir.absolutePath();

	// 从配置文件中读取DEM路径 
	QString demConfigPath = QString::fromLocal8Bit("../bin/config/projectPathConfig/globaldemORinputdem.txt");   // 配置文件路径 
	//QString demFilePath = tmpPathdir + "/Software/etc/globaldem/globaldem.tif";   // 默认路径 

	//// 尝试从配置文件读取 
	//QFile demConfigFile(demConfigPath);
	//if (demConfigFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
	//	QTextStream in(&demConfigFile);
	//	QString configLine = in.readLine().trimmed();
	//	if (!configLine.isEmpty()) {
	//		demFilePath = configLine;
	//	}
	//	demConfigFile.close();
	//}
	bool demFilePath = systemConfig->getGlobalDEM();

	if (!demFilePath)
	{
		if (!DEMFilePath.empty()) {
			for (auto data : DEMFilePath)
			{
				QDomElement DEM = doc.createElement("DEM");
				QDomText DEMText = doc.createTextNode(data);
				DEM.appendChild(DEMText);
				root.appendChild(DEM);
			}
		}
		else {
			QDomElement DEM = doc.createElement("DEM");
			QString exeDir = QCoreApplication::applicationDirPath();
			QDir dir(exeDir);
			QString tmpPath = dir.absolutePath();

			QDomText DEMText = doc.createTextNode(tmpPath + "/Software/etc/globaldem/globaldem.tif");
			DEM.appendChild(DEMText);
			root.appendChild(DEM);
		}
	}
	else
	{
		QDomElement DEM = doc.createElement("DEM");
		QString exeDir = QCoreApplication::applicationDirPath();
		QDir dir(exeDir);
		QString tmpPath = dir.absolutePath();

		QDomText DEMText = doc.createTextNode(tmpPath + "/Software/etc/globaldem/globaldem.tif");
		DEM.appendChild(DEMText);
		root.appendChild(DEM);
	}

	QDomElement outputObjFile = doc.createElement("OutputObjFilePath");
	outputObjFile.appendChild(doc.createTextNode(filePath + "/" + "obj.txt"));
	root.appendChild(outputObjFile);

	// 检查SatModelMakerCmdout.xml 文件是否存在 
	QString cmdoutFilePath = filePath + "/" + "SatModelMakerCmdout.xml";
	QFileInfo cmdoutFileInfo(cmdoutFilePath);

	if (!cmdoutFileInfo.exists()) {
		// 创建 QDomDocument
		QDomDocument docsatM;

		// 添加 XML 声明
		QDomProcessingInstruction instruction = docsatM.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		docsatM.appendChild(instruction);

		// 创建根节点 <SatImages>
		QDomElement rootsatM = docsatM.createElement("XQSatImages");

		// 创建 <InputFiles> 子节点
		QDomElement InputFiles = docsatM.createElement("InputFiles");

		for (auto data : DataModelPath)
		{
			// 创建 <InputImageFilePath> 子节点
			QDomElement InputFilesPath = docsatM.createElement("ImageFile");
			//inputImagePath.setAttribute("type", "string");
			QString tmp = data;
			QDomText inputImagePathText = docsatM.createTextNode(tmp);
			InputFilesPath.appendChild(inputImagePathText);
			InputFiles.appendChild(InputFilesPath);
		}
		rootsatM.appendChild(InputFiles);

		// 创建 <OutputFile> 子节点
		QDomElement Outputfile = docsatM.createElement("OutputFile");
		QDomText OutputfileText = docsatM.createTextNode(filePath + "/" + "SatModelMakerCmdout.xml");
		Outputfile.appendChild(OutputfileText);
		rootsatM.appendChild(Outputfile);

		// 创建 <Parameters> 子节点
		QDomElement Parameters = docsatM.createElement("Parameters");
		// 创建 <StereoModel> 子节点
		QDomElement StereoModel = docsatM.createElement("StereoModel");
		StereoModel.setAttribute("type", "bool");
		QDomText StereoModelText = docsatM.createTextNode(QVariant(false).toString());
		StereoModel.appendChild(StereoModelText);
		Parameters.appendChild(StereoModel);

		// 添加说明注释   
		QDomComment comment1 = docsatM.createComment(QString::fromLocal8Bit("false为单片，true为立体"));
		Parameters.appendChild(comment1);

		// 创建 <MuxAsSingleModel> 子节点
		QDomElement MuxAsSingleModel = docsatM.createElement("MuxAsSingleModel");
		MuxAsSingleModel.setAttribute("type", "int32");
		QDomText MuxAsSingleModelText = docsatM.createTextNode(QString::number(0));
		MuxAsSingleModel.appendChild(MuxAsSingleModelText);
		Parameters.appendChild(MuxAsSingleModel);

		// 添加说明注释   
		QDomComment comment2 = docsatM.createComment(QString::fromLocal8Bit("多光谱作为独立模型,也就是不和全色配对"));
		Parameters.appendChild(comment2);
		rootsatM.appendChild(Parameters);

		docsatM.appendChild(rootsatM);

		// 写入文件
		QString xmlPath = filePath + "/" + QString::fromLocal8Bit("SatModelMakerCmd.xml");

		QFile filesatM(xmlPath);
		if (!filesatM.open(QIODevice::WriteOnly | QIODevice::Text)) {
			return "";
		}

		QTextStream out(&filesatM);
		out.setCodec("UTF-8");
		out << docsatM.toString(4);  // 4 表示缩进 4 个空格，使 XML 可读性更好
		filesatM.close();

		// 创建并启动SatModelMakerCmd进程
		QProcess *process = new QProcess(this);
		QString authCmd;
#ifdef Q_OS_LINUX 
		authCmd = QDir(this->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSSatModelMakerCmd.x"));
#else
		authCmd = QDir(this->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSSatModelMakerCmd.exe"));
#endif
		process->start(authCmd, QStringList() << xmlPath);
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[this, process, filePath, doc, root](int exitCode, QProcess::ExitStatus status) mutable {
			// 解析SatModelMakerCmdout.xml 文件获取模型信息 
			QDomDocument cmdoutDoc;
			QFile cmdoutFile(filePath + "/" + "SatModelMakerCmdout.xml");
			if (!cmdoutFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				qDebug() << "Failed to open SatModelMakerCmdout.xml";
				return false;
			}
			if (!cmdoutDoc.setContent(&cmdoutFile)) {
				cmdoutFile.close();
				qDebug() << "Failed to parse SatModelMakerCmdout.xml";
				return false;
			}
			cmdoutFile.close();

			// 获取模型数量 
			QDomElement cmdoutRoot = cmdoutDoc.documentElement();
			QDomElement modelNumElement = cmdoutRoot.firstChildElement("ModelNum");
			int modelNum = modelNumElement.text().toInt();

			QDomElement numOfModels = doc.createElement("NumOfModels");
			numOfModels.setAttribute("type", "int32");
			numOfModels.appendChild(doc.createTextNode(QString::number(modelNum)));
			root.appendChild(numOfModels);

			// 创建Models节点
			QDomElement modelsElement = doc.createElement("Models");

			// 遍历所有模型 
			QDomElement cmdoutModels = cmdoutRoot.firstChildElement("Models");
			QDomElement cmdoutModel = cmdoutModels.firstChildElement();
			int modelIndex = 0;

			while (!cmdoutModel.isNull()) {
				QDomElement modelElement = doc.createElement(QString("Model_%1").arg(modelIndex));

				// 处理NadImage (全色影像)
				QDomElement nadImageName = cmdoutModel.firstChildElement("NadImageName");
				if (!nadImageName.isNull()) {
					QDomElement imageElement = doc.createElement("Image");

					// ImageID
					QDomElement imageId = doc.createElement("ImageID");
					imageId.setAttribute("type", "int32");
					imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("NadImageIndex").text()));
					imageElement.appendChild(imageId);

					// ImageInfoFile (.nfo文件)
					QString nadImagePath = nadImageName.text();
					nadImagePath.replace(".tiff", ".nfo");
					nadImagePath.replace(".TIFF", ".nfo");
					nadImagePath.replace(".tif", ".nfo");
					nadImagePath.replace(".TIF", ".nfo");
					nadImagePath.replace(".til", ".nfo");
					nadImagePath.replace(".TIL", ".nfo");
					QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
					imageInfoFile.appendChild(doc.createTextNode(nadImagePath));
					imageElement.appendChild(imageInfoFile);

					// SensorType (全色影像对应0)
					QDomElement sensorType = doc.createElement("SensorType");
					sensorType.setAttribute("type", "int32");
					sensorType.appendChild(doc.createTextNode("0"));
					imageElement.appendChild(sensorType);

					modelElement.appendChild(imageElement);
				}

				// 处理FwdImage (如果有)   
				QDomElement fwdImageName = cmdoutModel.firstChildElement("FwdImageName");
				if (!fwdImageName.isNull()) {
					QDomElement imageElement = doc.createElement("Image");
					QDomElement imageId = doc.createElement("ImageID");
					imageId.setAttribute("type", "int32");
					imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("FwdImageIndex").text()));
					imageElement.appendChild(imageId);
					// 将.tiff替换为.nfo
					QString fwdImagePath = fwdImageName.text();
					fwdImagePath.replace(".tiff", ".nfo");
					fwdImagePath.replace(".TIFF", ".nfo");
					fwdImagePath.replace(".tif", ".nfo");
					fwdImagePath.replace(".TIF", ".nfo");
					fwdImagePath.replace(".til", ".nfo");
					fwdImagePath.replace(".TIL", ".nfo");
					QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
					imageInfoFile.appendChild(doc.createTextNode(fwdImagePath));
					imageElement.appendChild(imageInfoFile);

					QDomElement sensorType = doc.createElement("SensorType");
					sensorType.setAttribute("type", "int32");
					sensorType.appendChild(doc.createTextNode("1")); // FwdImage对应1
					imageElement.appendChild(sensorType);
					modelElement.appendChild(imageElement);
				}

				// 处理BwdImage (如果有)   
				QDomElement bwdImageName = cmdoutModel.firstChildElement("BwdImageName");
				if (!bwdImageName.isNull()) {
					QDomElement imageElement = doc.createElement("Image");
					QDomElement imageId = doc.createElement("ImageID");
					imageId.setAttribute("type", "int32");
					imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("BwdImageIndex").text()));
					imageElement.appendChild(imageId);
					// 将.tiff替换为.nfo
					QString bwdImagePath = bwdImageName.text();
					bwdImagePath.replace(".tiff", ".nfo");
					bwdImagePath.replace(".TIFF", ".nfo");
					bwdImagePath.replace(".tif", ".nfo");
					bwdImagePath.replace(".TIF", ".nfo");
					bwdImagePath.replace(".til", ".nfo");
					bwdImagePath.replace(".TIL", ".nfo");
					QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
					imageInfoFile.appendChild(doc.createTextNode(bwdImagePath));
					imageElement.appendChild(imageInfoFile);

					QDomElement sensorType = doc.createElement("SensorType");
					sensorType.setAttribute("type", "int32");
					sensorType.appendChild(doc.createTextNode("2")); // BwdImage对应2
					imageElement.appendChild(sensorType);
					modelElement.appendChild(imageElement);
				}

				// 处理MulImage (多光谱影像)
				QDomElement mulImageName = cmdoutModel.firstChildElement("MulImageName");
				if (!mulImageName.isNull()) {
					QDomElement imageElement = doc.createElement("Image");

					// ImageID 
					QDomElement imageId = doc.createElement("ImageID");
					imageId.setAttribute("type", "int32");
					imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("MulImageIndex").text()));
					imageElement.appendChild(imageId);

					// ImageInfoFile (.nfo文件)
					QString mulImagePath = mulImageName.text();
					mulImagePath.replace(".tiff", ".nfo");
					mulImagePath.replace(".TIFF", ".nfo");
					mulImagePath.replace(".tif", ".nfo");
					mulImagePath.replace(".TIF", ".nfo");
					mulImagePath.replace(".til", ".nfo");
					mulImagePath.replace(".TIL", ".nfo");
					QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
					imageInfoFile.appendChild(doc.createTextNode(mulImagePath));
					imageElement.appendChild(imageInfoFile);

					// SensorType (多光谱影像对应3)
					QDomElement sensorType = doc.createElement("SensorType");
					sensorType.setAttribute("type", "int32");
					sensorType.appendChild(doc.createTextNode("3"));
					imageElement.appendChild(sensorType);

					modelElement.appendChild(imageElement);
				}

				modelsElement.appendChild(modelElement);
				cmdoutModel = cmdoutModel.nextSiblingElement();
				modelIndex++;
			}

			root.appendChild(modelsElement);
			doc.appendChild(root);

			// 写入文件
			QFile file(filePath + "/" + "PSIntersectObjCmd.task");
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				qDebug() << "Failed to open output file";
				return false;
			}

			QTextStream out(&file);
			out.setCodec("UTF-8");
			out << doc.toString(4);   // 4 表示缩进 4 个空格，使 XML 可读性更好
			file.close();

			emit buildPSIntersectObjCmdFileFinished();
		});
	}
	else{
		// 解析SatModelMakerCmdout.xml 文件获取模型信息 
		QDomDocument cmdoutDoc;
		QFile cmdoutFile(filePath + "/" + "SatModelMakerCmdout.xml");
		if (!cmdoutFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			qDebug() << "Failed to open SatModelMakerCmdout.xml";
			return false;
		}
		if (!cmdoutDoc.setContent(&cmdoutFile)) {
			cmdoutFile.close();
			qDebug() << "Failed to parse SatModelMakerCmdout.xml";
			return false;
		}
		cmdoutFile.close();

		// 获取模型数量 
		QDomElement cmdoutRoot = cmdoutDoc.documentElement();
		QDomElement modelNumElement = cmdoutRoot.firstChildElement("ModelNum");
		int modelNum = modelNumElement.text().toInt();

		QDomElement numOfModels = doc.createElement("NumOfModels");
		numOfModels.setAttribute("type", "int32");
		numOfModels.appendChild(doc.createTextNode(QString::number(modelNum)));
		root.appendChild(numOfModels);

		// 创建Models节点
		QDomElement modelsElement = doc.createElement("Models");

		// 遍历所有模型 
		QDomElement cmdoutModels = cmdoutRoot.firstChildElement("Models");
		QDomElement cmdoutModel = cmdoutModels.firstChildElement();
		int modelIndex = 0;

		while (!cmdoutModel.isNull()) {
			QDomElement modelElement = doc.createElement(QString("Model_%1").arg(modelIndex));

			// 处理NadImage (全色影像)
			QDomElement nadImageName = cmdoutModel.firstChildElement("NadImageName");
			if (!nadImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");

				// ImageID
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("NadImageIndex").text()));
				imageElement.appendChild(imageId);

				// ImageInfoFile (.nfo文件)
				QString nadImagePath = nadImageName.text();
				nadImagePath.replace(".tiff", ".nfo");
				nadImagePath.replace(".TIFF", ".nfo");
				nadImagePath.replace(".tif", ".nfo");
				nadImagePath.replace(".TIF", ".nfo");
				nadImagePath.replace(".til", ".nfo");
				nadImagePath.replace(".TIL", ".nfo");
				QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
				imageInfoFile.appendChild(doc.createTextNode(nadImagePath));
				imageElement.appendChild(imageInfoFile);

				// SensorType (全色影像对应0)
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("0"));
				imageElement.appendChild(sensorType);

				modelElement.appendChild(imageElement);
			}

			// 处理FwdImage (如果有)   
			QDomElement fwdImageName = cmdoutModel.firstChildElement("FwdImageName");
			if (!fwdImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("FwdImageIndex").text()));
				imageElement.appendChild(imageId);
				// 将.tiff替换为.nfo
				QString fwdImagePath = fwdImageName.text();
				fwdImagePath.replace(".tiff", ".nfo");
				fwdImagePath.replace(".TIFF", ".nfo");
				fwdImagePath.replace(".tif", ".nfo");
				fwdImagePath.replace(".TIF", ".nfo");
				fwdImagePath.replace(".til", ".nfo");
				fwdImagePath.replace(".TIL", ".nfo");
				QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
				imageInfoFile.appendChild(doc.createTextNode(fwdImagePath));
				imageElement.appendChild(imageInfoFile);

				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("1")); // FwdImage对应1
				imageElement.appendChild(sensorType);
				modelElement.appendChild(imageElement);
			}

			// 处理BwdImage (如果有)   
			QDomElement bwdImageName = cmdoutModel.firstChildElement("BwdImageName");
			if (!bwdImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("BwdImageIndex").text()));
				imageElement.appendChild(imageId);
				// 将.tiff替换为.nfo
				QString bwdImagePath = bwdImageName.text();
				bwdImagePath.replace(".tiff", ".nfo");
				bwdImagePath.replace(".TIFF", ".nfo");
				bwdImagePath.replace(".tif", ".nfo");
				bwdImagePath.replace(".TIF", ".nfo");
				bwdImagePath.replace(".til", ".nfo");
				bwdImagePath.replace(".TIL", ".nfo");
				QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
				imageInfoFile.appendChild(doc.createTextNode(bwdImagePath));
				imageElement.appendChild(imageInfoFile);

				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("2")); // BwdImage对应2
				imageElement.appendChild(sensorType);
				modelElement.appendChild(imageElement);
			}

			// 处理MulImage (多光谱影像)
			QDomElement mulImageName = cmdoutModel.firstChildElement("MulImageName");
			if (!mulImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");

				// ImageID 
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("MulImageIndex").text()));
				imageElement.appendChild(imageId);

				// ImageInfoFile (.nfo文件)
				QString mulImagePath = mulImageName.text();
				mulImagePath.replace(".tiff", ".nfo");
				mulImagePath.replace(".TIFF", ".nfo");
				mulImagePath.replace(".tif", ".nfo");
				mulImagePath.replace(".TIF", ".nfo");
				mulImagePath.replace(".til", ".nfo");
				mulImagePath.replace(".TIL", ".nfo");
				QDomElement imageInfoFile = doc.createElement("ImageInfoFile");
				imageInfoFile.appendChild(doc.createTextNode(mulImagePath));
				imageElement.appendChild(imageInfoFile);

				// SensorType (多光谱影像对应3)
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("3"));
				imageElement.appendChild(sensorType);

				modelElement.appendChild(imageElement);
			}

			modelsElement.appendChild(modelElement);
			cmdoutModel = cmdoutModel.nextSiblingElement();
			modelIndex++;
		}

		root.appendChild(modelsElement);
		doc.appendChild(root);

		// 写入文件
		QFile file(filePath + "/" + "PSIntersectObjCmd.task");
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qDebug() << "Failed to open output file";
			return false;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);   // 4 表示缩进 4 个空格，使 XML 可读性更好
		file.close();
		emit buildPSIntersectObjCmdFileFinished();
	}

	return true;
}

void IMAGEPS::allMapShow_actionSlot()
{
	ui.ImageInfoShow_Widget->zoomToFullExtent();
}

void IMAGEPS::amplify_actionSlot()
{
	ui.ImageInfoShow_Widget->zoomIn();
}

void IMAGEPS::narrow_actionSlot()
{
	ui.ImageInfoShow_Widget->zoomOut();
}

void IMAGEPS::translation_actionSlot()
{
	//ui.measureAreaShow_GLW->pan();
}

void IMAGEPS::highlightSelectedBoundaries(QTableWidget* tableWidget) {
	if (!tableWidget || !ui.ImageInfoShow_Widget) {
		return;
	}

	// 获取选中的所有行
	QList<QTableWidgetItem*> selectedItems = tableWidget->selectedItems();
	QSet<int> selectedRows;

	// 收集所有选中的行号 
	for (QTableWidgetItem* item : selectedItems) {
		selectedRows.insert(item->row());
	}

	QStringList filePathlist;
	// 获取是哪个表格触发的
	QString tableName = tableWidget->objectName(); 

	if (!m_ImageselectionAll) {
		// 清除之前的高亮 
		ui.ImageInfoShow_Widget->clearHighlights();
	}

	if (tableName == QString::fromLocal8Bit("sateImageDataList_TableW"))
	{
		filePathlist = DataModelPath;
	}
	else if (tableName == QString::fromLocal8Bit("refereDataList_TableW"))
	{
		filePathlist = DOMFilePath;
		filePathlist.append(DEMFilePath);
	}
	else if (tableName == QString::fromLocal8Bit("PyramidDataList_TableW"))
	{
		filePathlist = CreatepyramidPath;
	}
	else if (tableName == QString::fromLocal8Bit("imageInterList_TableW"))
	{
		filePathlist = ImageInterFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("dataList_TableW"))
	{
		filePathlist = FusionmodelFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("AlignmentAdjustmentList_TableW"))
	{
		filePathlist = AlignmentAdFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("AlignmentIntegrationList_TableW"))
	{
		filePathlist = AlignmentIntFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("objectPosPoints_TableW"))
	{
		filePathlist = DataModelPath;
	}
	else if (tableName == QString::fromLocal8Bit("DodgingDataList_TableW"))
	{
		filePathlist = DodgingFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("ImageMosaicDataList_TableW"))
	{
		filePathlist = ImageMosaicFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("SmartMosaicDataList_TableW"))
	{
		filePathlist = SmartMosaicFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("QuickMosaicDataList_TableW"))
	{
		filePathlist = QuickMosaicFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("ImagecropDataList_TableW"))
	{
		filePathlist = ImagecropFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("VectorfileDataList_TableW"))
	{
		filePathlist = VectorfileFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("AbsPositPrecCheckDataList_TableW"))
	{
		filePathlist = AbsPositPrecCheckFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("RelPositPrecCheckDataList_TableW"))
	{
		filePathlist = RelPositPrecCheckFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("CloudDetectionList_TableW"))
	{
		filePathlist = CloudDetectionListFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("CloudDetectionDataList_TableW"))
	{
		filePathlist = CloudDetectionDataListFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("ProjectionTransformationDataList_TableW"))
	{
		filePathlist = ProTransformationFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("FormatConversionDataList_TableW"))
	{
		filePathlist = FormatConversionFilePath;
	}
	else if (tableName == QString::fromLocal8Bit("SARImageFilteringDataList_TableW"))
	{
		filePathlist = SARImageFilteringFilePath;
	}
	QStringList tmpfilePath;
	// 遍历所有选中的行
	for (int row : selectedRows) {
		// 获取文件路径
		QTableWidgetItem* fileItem = tableWidget->item(row, 1);
		if (!fileItem) continue;
		
		QString filePath = fileItem->text();
		for (auto filepath : filePathlist) {
			QFileInfo fileinfo(filepath);

			if (fileinfo.completeBaseName() == filePath) {
				//tmpfilePath.append(filepath);
				if (!tmpfilePath.contains(filepath)) {
					tmpfilePath.append(filepath);
				}
			}
		}
	}
	ui.ImageInfoShow_Widget->highlightsBoundary(tmpfilePath);

}

void IMAGEPS::closeEvent(QCloseEvent *event)
{
	if (!closeflag) {
		emit aboutToClose();
		event->accept();
		return;
	}

	// 有未保存修改时的处理流程 
	if (m_isModified || isWindowModified()) {
		QMessageBox::StandardButton btn = QMessageBox::question(
			this,
			QString::fromLocal8Bit("未保存的修改"),
			QString::fromLocal8Bit("当前工程有未保存的更改，是否保存？"),
			QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
			QMessageBox::Save
		);

		if (btn == QMessageBox::Save) {
			if (!saveProActionSlot()) {
				QMessageBox::warning(this,
					QString::fromLocal8Bit("错误"),
					QString::fromLocal8Bit("保存失败，请检查文件权限或磁盘空间！"));
				event->ignore();
				return;
			}

			// 保存成功后二次确认关闭 
			QMessageBox::StandardButton confirmBtn = QMessageBox::question(
				this,
				QString::fromLocal8Bit("确认关闭"),
				QString::fromLocal8Bit("工程已保存，确定要退出吗？"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No
			);

			if (confirmBtn == QMessageBox::Yes) {
				emit aboutToClose();
				event->accept();
			}
			else {
				event->ignore();
			}
		}
		else if (btn == QMessageBox::Discard) {
			// 不保存直接退出
			QMessageBox::StandardButton confirmBtn = QMessageBox::question(
				this,
				QString::fromLocal8Bit("警告"),
				QString::fromLocal8Bit("所有未保存的更改将丢失，确定退出？"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No
			);

			if (confirmBtn == QMessageBox::Yes) {
				emit aboutToClose();
				event->accept();
			}
			else {
				event->ignore();
			}
		}
		else {
			// 用户点击取消 
			event->ignore();
		}
	}
	// 没有未保存修改时的简单确认 
	else {
		QMessageBox::StandardButton btn = QMessageBox::question(
			this,
			QString::fromLocal8Bit("确认关闭"),
			QString::fromLocal8Bit("确定要退出程序吗？"),
			QMessageBox::Yes | QMessageBox::No,
			QMessageBox::No
		);

		event->setAccepted(btn == QMessageBox::Yes);
		if (btn == QMessageBox::Yes) {
			emit aboutToClose();
		}
	}
}

void IMAGEPS::paintEvent(QPaintEvent* event)
{
	if (!m_image.isNull()) {
		QPainter painter(this);
		// 将图像缩放至窗口大小
		QPixmap pixmap = QPixmap::fromImage(m_image.scaled(size(), Qt::KeepAspectRatio));
		painter.drawPixmap(0, 0, this->width(), this->height(), pixmap);
	}
}

// 事件过滤器
bool IMAGEPS::eventFilter(QObject *watched, QEvent *event) {

	// 为新创建的滚动条动态添加中文菜单支持 
	if (event->type() == QEvent::ChildAdded) {
		QChildEvent* childEvent = static_cast<QChildEvent*>(event);
		if (QScrollBar* scrollBar = qobject_cast<QScrollBar*>(childEvent->child())) {
			scrollBar->installEventFilter(this);
			scrollBar->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(scrollBar, &QScrollBar::customContextMenuRequested,
				this, [this, scrollBar](const QPoint &pos) {
				showChineseScrollBarContextMenu(scrollBar, pos);
			});
		}
	}

	// 处理QTreeWidget的悬停事件 
	if (auto *tree = qobject_cast<QTreeWidget*>(watched)) {
		if (event->type() == QEvent::HoverMove || event->type() == QEvent::HoverEnter) {
			QHoverEvent *hoverEvent = static_cast<QHoverEvent*>(event);
			QTreeWidgetItem *item = tree->itemAt(hoverEvent->pos());

			if (item) {
				int column = tree->columnAt(hoverEvent->pos().x());
				if (column >= 0) {
					// 检查文本是否被截断 
					QFontMetrics fm(tree->font());
					int textWidth = fm.horizontalAdvance(item->text(column));
					int columnWidth = tree->columnWidth(column);

					if (textWidth > columnWidth - 10) {  // 考虑边距 
						// 显示完整内容作为工具提示 
						QToolTip::showText(
							QCursor::pos(),
							item->text(column),
							tree,
							tree->visualItemRect(item).translated(tree->viewport()->mapToGlobal(QPoint(0, 0)))
						);
					}
					else {
						QToolTip::hideText();
					}
				}
			}
			return true;
		}
		else if (event->type() == QEvent::HoverLeave) {
			QToolTip::hideText();
			return true;
		}
	}

	if (watched == ui.ImageInfoShow_Widget) {
		if (event->type() == QEvent::MouseMove) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint pos = mouseEvent->pos();

			QPointF worldPos = ui.ImageInfoShow_Widget->screenToWorld(pos);

			if (ui.ImageInfoShow_Widget->m_boundaries.isEmpty()) {
				ui.label_2->setText(QString::fromLocal8Bit(" 经度: %1°").arg(worldPos.x(), 0, 'f', 6));
				ui.label_4->setText(QString::fromLocal8Bit(" 纬度: %1°").arg(worldPos.y(), 0, 'f', 6));
				return false;
			}

			double minLon = std::numeric_limits<double>::max();
			double maxLon = std::numeric_limits<double>::lowest();
			double minLat = std::numeric_limits<double>::max();
			double maxLat = std::numeric_limits<double>::lowest();

			for (const auto& boundary : ui.ImageInfoShow_Widget->m_boundaries) {
				for (const auto& corner : boundary.corners) {
					minLon = qMin(minLon, corner.x());
					maxLon = qMax(maxLon, corner.x());
					minLat = qMin(minLat, corner.y());
					maxLat = qMax(maxLat, corner.y());
				}
			}

			double worldWidth = ui.ImageInfoShow_Widget->m_worldMaxX - ui.ImageInfoShow_Widget->m_worldMinX;
			double worldHeight = ui.ImageInfoShow_Widget->m_worldMaxY - ui.ImageInfoShow_Widget->m_worldMinY;

			if (qFuzzyIsNull(worldWidth) || qFuzzyIsNull(worldHeight)) {
				return false;
			}

			double longitude = minLon + (worldPos.x() - ui.ImageInfoShow_Widget->m_worldMinX) / worldWidth * (maxLon - minLon);
			double latitude = minLat + (worldPos.y() - ui.ImageInfoShow_Widget->m_worldMinY) / worldHeight * (maxLat - minLat);

			ui.label_2->setText(QString::fromLocal8Bit(" %1 度").arg(longitude, 0, 'f', 6));
			ui.label_4->setText(QString::fromLocal8Bit(" %1 度").arg(latitude, 0, 'f', 6));
		}
	}
	// 处理OpenGLWidget的鼠标移动事件
	if (watched == ui.openGLWidget) {
		if (event->type() == QEvent::MouseMove) {
			QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint pos = mouseEvent->pos();

			// 1. 获取鼠标在OpenGL窗口中的坐标（像素坐标）
			QPointF glPos = ui.openGLWidget->mapFromGlobal(mouseEvent->globalPos());

			// 2. 转换为OpenGL世界坐标 
			QPointF worldPos = ui.openGLWidget->screenToWorld(pos);

			// 3. 转换为米制坐标（假设1单位=1米）
			// 如果您的OpenGL场景已经是米制单位，可以直接使用 
			double meterX = worldPos.x();
			double meterY = worldPos.y();

			// 4. 更新显示 
			ui.label_2->setText(QString::fromLocal8Bit("X:  %1 米").arg(meterX, 0, 'f', 2));
			ui.label_4->setText(QString::fromLocal8Bit("Y:  %1 米").arg(fabs(meterY), 0, 'f', 2));
		}
	}

	if (event->type() == QEvent::MouseButtonPress) {
		// 清除所有 QTableWidget 的选中状态
		QList<QTableWidget*> tableWidgets = findChildren<QTableWidget*>();
		for (QTableWidget* tableWidget : tableWidgets) {
			tableWidget->clearSelection();
		}

		//QList<QTableView*> tableViews = findChildren<QTableView*>();
		//for (QTableView* tableView : tableViews) {
		//	if (tableView->selectionModel()) {
		//		tableView->selectionModel()->clearSelection();
		//	}
		//}
	}

	// 新增：处理在IMAGEPS界面内左键点击时清除QTableView中的选中状态 
	if (event->type() == QEvent::MouseButtonPress) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

		// 检查是否是左键点击 
		if (mouseEvent->button() == Qt::LeftButton) {
			// 检查是否有组合键 
			bool hasModifier = (mouseEvent->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier | Qt::AltModifier));
			if (hasModifier) {
				return false; // 有组合键时不清除选择 
			}

			// 检查是否是滑轮点击（中键） 
			if (mouseEvent->button() == Qt::MiddleButton) {
				return false;
			}

			// 检查点击位置是否在QTableView或其视口上 
			QWidget *clickedWidget = QApplication::widgetAt(mouseEvent->globalPos());

			// 检查是否在表格滑轮上点击 
			if (QScrollBar *scrollBar = qobject_cast<QScrollBar*>(clickedWidget)) {
				return false; // 在滑轮上点击时不处理 
			}

			// 检查是否在滚动条上点击 
			if (qobject_cast<QScrollBar*>(clickedWidget)) {
				return false;
			}

			// 检查是否在表格相关部件上点击 
			if (qobject_cast<QTableView*>(clickedWidget) ||
				qobject_cast<QTableView*>(clickedWidget->parent())) {
				return false;
			}

			// 清除所有QTableView的选中状态 
			QList<QTableView*> tableViews = findChildren<QTableView*>();
			for (QTableView *tableView : tableViews) {
				if (clickedWidget == tableView ||
					clickedWidget == tableView->viewport()) {
					return false; // 在表格上点击时不处理 
				}

				// 清除所有表格的选中状态 
				for (QTableView *tableView : tableViews) {
					tableView->clearSelection();
				}
			}
		}
	}

	if (watched == ui.encryptPointsList_TableW->viewport() && event->type() == QEvent::Wheel) {
		QScrollBar* vScrollBar = ui.encryptPointsList_TableW->verticalScrollBar();
		if (vScrollBar->value() == vScrollBar->maximum()) {
			// 滚动到底部，加载下一页 
			int totalPages = (m_visiblePointIds.size() + PAGE_SIZE - 1) / PAGE_SIZE;
			if (m_currentPage < totalPages - 1) {
				m_currentPage++;
				loadCurrentPage();
			}
		}
		else if (vScrollBar->value() == vScrollBar->minimum() && m_currentPage > 0) {
			// 滚动到顶部，加载上一页 
			m_currentPage--;
			loadCurrentPage();
			vScrollBar->setValue(vScrollBar->maximum() - 1); // 保持滚动位置 
		}
	}

	return QMainWindow::eventFilter(watched, event);
}

void IMAGEPS::setupAllChineseScrollBars()
{
	// 为现有滚动条安装事件过滤器 
	QList<QScrollBar*> scrollBars = this->findChildren<QScrollBar*>();
	for (QScrollBar* scrollBar : scrollBars) {
		scrollBar->installEventFilter(this);
		scrollBar->setContextMenuPolicy(Qt::CustomContextMenu);
		connect(scrollBar, &QScrollBar::customContextMenuRequested,
			this, [this, scrollBar](const QPoint &pos) {
			showChineseScrollBarContextMenu(scrollBar, pos);
		});
	}
}

void IMAGEPS::showChineseScrollBarContextMenu(QScrollBar* scrollBar, const QPoint& pos)
{
	if (!scrollBar) return;

	QMenu menu(this);

	// 添加中文菜单项
	QAction *scrollHere = menu.addAction(u8" 滚动到此位置");
	connect(scrollHere, &QAction::triggered, [scrollBar, pos]() {
		if (scrollBar->orientation() == Qt::Vertical) {
			scrollBar->setValue(scrollBar->minimum() +
				(scrollBar->maximum() - scrollBar->minimum())*
				pos.y() / scrollBar->height());
		}
		else {
			scrollBar->setValue(scrollBar->minimum() +
				(scrollBar->maximum() - scrollBar->minimum())*
				pos.x() / scrollBar->width());
		}
	});

	menu.addSeparator();

	menu.addAction(u8" 顶部", [scrollBar]() { scrollBar->setValue(scrollBar->minimum()); });
	menu.addAction(u8" 底部", [scrollBar]() { scrollBar->setValue(scrollBar->maximum()); });
	menu.addSeparator();

	menu.addAction(u8" 上翻页", [scrollBar]() {
		scrollBar->triggerAction(QAbstractSlider::SliderPageStepSub);
	});
	menu.addAction(u8" 下翻页", [scrollBar]() {
		scrollBar->triggerAction(QAbstractSlider::SliderPageStepAdd);
	});
	menu.addSeparator();

	menu.addAction(u8" 上滚", [scrollBar]() {
		scrollBar->triggerAction(QAbstractSlider::SliderSingleStepSub);
	});
	menu.addAction(u8" 下滚", [scrollBar]() {
		scrollBar->triggerAction(QAbstractSlider::SliderSingleStepAdd);
	});

	menu.exec(scrollBar->mapToGlobal(pos));
}

bool IMAGEPS::showConfirmationDialog(const QString &message) {
	QMessageBox msgBox(
		QMessageBox::Question,
		message,
		QString::fromLocal8Bit("确认进行") + message + QString::fromLocal8Bit("?"),
		QMessageBox::Yes | QMessageBox::No,
		this
	);

	// 设置按钮文本 
	msgBox.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("是"));
	msgBox.setButtonText(QMessageBox::No, QString::fromLocal8Bit("否"));

	// 设置默认按钮 
	msgBox.setDefaultButton(QMessageBox::No);

	// 显示并返回结果 
	return (msgBox.exec() == QMessageBox::Yes);
}

//bool IMAGEPS::showConfirmationDialog(const QString &message) {
//	
//	QMessageBox::StandardButton reply;
//	reply = QMessageBox::question(
//		this,                                     
//		message,        
//		QString::fromLocal8Bit("确认进行") + message + QString::fromLocal8Bit("?"),
//		QMessageBox::Yes | QMessageBox::No,       
//		QMessageBox::No                          
//	);
//
//	// 返回用户选择（true=Yes，false=No）
//	return (reply == QMessageBox::Yes);
//}

/**
 * @brief 连接所有控件的修改信号，用于检测界面内容变化
 *
 * 该函数会监听以下控件的修改行为：
 * 1. QTextEdit/QPlainTextEdit 文本变化
 * 2. QTableWidget 单元格内容变化
 * 3. QTreeWidget 树节点内容变化
 *
 * 当检测到修改时，会自动设置 m_isModified=true 并标记窗口为未保存状态
 */
void IMAGEPS::connectAllModificationSignals()
{
	// QTextEdit 
	for (QTextEdit* edit : findChildren<QTextEdit*>()) {
		connect(edit, &QTextEdit::textChanged, [this] {
			m_isModified = true;
			setWindowModified(true);
		});
	}

	for (QTableWidget* table : findChildren<QTableWidget*>()) {

		connect(table, &QTableWidget::itemChanged, [this](QTableWidgetItem* item) {
			m_isModified = true;
			setWindowModified(true);
		});
	}

	for (QTreeWidget* tree : findChildren<QTreeWidget*>()) {
		tree->setEditTriggers(QAbstractItemView::DoubleClicked);

		connect(tree, &QTreeWidget::itemChanged, [this](QTreeWidgetItem* item, int col) {
			m_isModified = true;
			setWindowModified(true);
		});
	}

	// QTableView 
	for (QTableView* tableView : findChildren<QTableView*>()) {
		// 设置编辑触发方式 
		tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

		// 监听模型数据变化
		auto connectModelSignals = [this](QAbstractItemModel* model) {
			if (model) {
				connect(model, &QAbstractItemModel::dataChanged, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
					Q_UNUSED(topLeft);
					Q_UNUSED(bottomRight);
					m_isModified = true;
					setWindowModified(true);
				});
			}
		};

		// 连接当前模型 
		connectModelSignals(tableView->model());
	}
}

/**
 * @brief 实时将QTextEdit控件中的文本写入文件
 * @param logEdit QTextEdit控件指针
 * @param filePath 要写入的文件路径
 * @param maxLines 最大保留行数(0表示不限制)
 * @param parent 父对象指针(用于自动清理)
 */
void IMAGEPS::setupLogToFileWriter(QTextEdit* logEdit, const QString& filePath, int maxLines, QObject* parent) {
	// 确保参数有效
	if (!logEdit || filePath.isEmpty()) {
		qWarning() << "Invalid parameters for log writer";
		return;
	}

	// 创建文件对象并尝试打开
	QFile* logFile = new QFile(filePath);
	if (!logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
		qWarning() << "Failed to open log file:" << filePath;
		delete logFile;
		return;
	}

	// 创建文本流
	QTextStream* stream = new QTextStream(logFile);
	stream->setCodec("UTF-8"); // 设置编码为UTF-8 

	// 如果指定了父对象，则自动清理 
	if (parent) {
		logFile->setParent(parent);
		stream->setDevice(nullptr); // 防止双重删除 
		delete stream;
		stream = new QTextStream(logFile);
	}

	// 初始写入现有内容
	*stream << logEdit->toPlainText();
	stream->flush();

	// 连接文本变化信号
	QObject::connect(logEdit, &QTextEdit::textChanged, [=]() {
		// 获取新增的文本 
		QString newText = logEdit->toPlainText();
		QStringList lines = newText.split('\n');

		// 如果设置了最大行数，则截断 
		if (maxLines > 0 && lines.count() > maxLines) {
			lines = lines.mid(lines.count() - maxLines);
			logEdit->setPlainText(lines.join('\n'));
			logEdit->moveCursor(QTextCursor::End);
		}

		// 写入文件 
		logFile->resize(0); // 清空文件 
		*stream << logEdit->toPlainText();
		stream->flush();
	});

	// 自动清理处理 
	if (!parent) {
		// 如果没有父对象，需要在适当时候手动删除 
		QObject::connect(logEdit, &QObject::destroyed, [=]() {
			stream->flush();
			delete stream;
			logFile->close();
			delete logFile;
		});
	}
}

QMap<QString, bool> IMAGEPS::getcolfileValue() {
	if (controlPointsPath.empty() && this->tmpcontrolPointsflag)
	{
		controlPointsPath = this->tmpcontrolPointsPath;
		this->tmpimageInterflag = false;
	}
	if (!OrthoRectificationFusionflag)
	{
		return controlPointsPath;
	}
	else
	{
		return this->controlPointsPath;
	}
}

void IMAGEPS::setView(TabbedDockWidget* DockWidget, QTabWidget* tableWidget, int currentindex) {
	tableWidget->setCurrentIndex(currentindex);
	QWidget* page = tableWidget->widget(currentindex);
	if (tableWidget->isTabEnabled(currentindex) && page->isVisible() &&
		(tableWidget->currentIndex() == currentindex))
	{
		tableWidget->setTabEnabled(currentindex, false);
		tableWidget->setStyleSheet(
			"QTabBar::tab:disabled { width: 0; color: transparent; }"
			"QTabBar::scroller { width: 0; }"
			"QTabBar::tab:selected {background: #00a99d; color: white;border - bottom: 2px solid #008080;}"
		);
		tableWidget->setCurrentIndex(currentindex);

		bool allDisabled = true;
		for (int i = 0; i < tableWidget->count(); ++i) {
			if (tableWidget->isTabEnabled(i)) {
				allDisabled = false;
				break;
			}
		}

		if (allDisabled) {
			DockWidget->hide(); // 全部禁用则隐藏DockWidget 
		}
		else {
			// 查找并切换到下一个可用标签页
			int nextIndex = -1;
			for (int i = currentindex + 1; i < tableWidget->count(); ++i) {
				if (tableWidget->isTabEnabled(i)) {
					nextIndex = i;
					break;
				}
			}
			if (nextIndex == -1) {
				for (int i = 0; i < currentindex; ++i) {
					if (tableWidget->isTabEnabled(i)) {
						nextIndex = i;
						break;
					}
				}
			}
			if (nextIndex >= 0) {
				tableWidget->setCurrentIndex(nextIndex);
			}
		}
		return;
	}

	if (currentindex >= 0 && currentindex < tableWidget->count()) {
		DockWidget->show();
		tableWidget->setTabEnabled(currentindex, true);
		tableWidget->tabBar()->setStyleSheet("");
		QWidget *page = tableWidget->widget(currentindex);
		page->setVisible(true);
		page->show();
		tableWidget->setCurrentIndex(currentindex);
		tableWidget->tabBar()->adjustSize();
		tableWidget->repaint();
		QApplication::processEvents();
	}
}

void IMAGEPS::displayView(TabbedDockWidget* DockWidget, QTabWidget* tableWidget, int currentindex) {

	if (currentindex >= 0 && currentindex < tableWidget->count()) {
		DockWidget->show();
		tableWidget->setTabEnabled(currentindex, true);
		tableWidget->tabBar()->setStyleSheet("");
		QWidget *page = tableWidget->widget(currentindex);
		page->setVisible(true);
		page->show();
		tableWidget->setCurrentIndex(currentindex);
		tableWidget->tabBar()->adjustSize();
		tableWidget->repaint();
		QApplication::processEvents();
	}
}

void IMAGEPS::killAllProcesses(bool flag)
{
	if (flag)
	{
		//if (QMessageBox::question(this,
		//	QString::fromLocal8Bit("确认"),
		//	QString::fromLocal8Bit("确定要终止当前任务吗？"),
		//	QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
		//{
		//	return;
		//}
		QMessageBox msgBox(
			QMessageBox::Question,
			QString::fromLocal8Bit("确认"),
			QString::fromLocal8Bit("确定要终止当前任务吗？"),
			QMessageBox::Yes | QMessageBox::No,
			this
		);

		msgBox.setButtonText(QMessageBox::Yes, QString::fromLocal8Bit("是"));
		msgBox.setButtonText(QMessageBox::No, QString::fromLocal8Bit("否"));

		if (msgBox.exec() != QMessageBox::Yes) {
			return;
		}
		ui.progressBar->setValue(0);
		ui.progressBar->setVisible(false);
		ui.progressBar_imageinfo->setValue(0);
		ui.progressBar_imageinfo->setVisible(false);
	}
	// 杀死SystemConfig类中启动的进程
	if (systemConfig) {
		systemConfig->terminateAllProcesses();
	}

	// 杀死本类中启动的进程 
	for (QProcess* process : m_childProcesses) {
		if (process && process->state() != QProcess::NotRunning) {
			process->kill();
			process->waitForFinished(1000); // 等待1秒确保进程终止
		}
	}

	// 清空进程列表 
	m_childProcesses.clear();

	if (flag)
	{
		QMessageBox::information(this,
			QString::fromLocal8Bit("完成"),
			QString::fromLocal8Bit("已终止当前所有任务"), QString::fromLocal8Bit("确定"));
	}
}

// 将文件内容写入指定的 QTextEdit
void IMAGEPS::loadFileToTextEdit(const QString &filePath, QTextEdit *textEdit) {
	if (!textEdit) {
		qWarning() << "Invalid QTextEdit pointer!";
		return;
	}

	//textEdit->clear(); // 清空原有内容

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		textEdit->setPlainText("Error: Cannot open file.");
		return;
	}
	QTextStream in(&file);
	textEdit->setPlainText(in.readAll());  // 覆盖原有内容
	file.close();
}

void IMAGEPS::setUiEnabled(QWidget* window, bool enabled)
{
	// 定义不需要禁用的控件名称列表 
	static const QSet<QString> excludedWidgets = {
		"progressBar",  // 进度条 
		"runLog_Edit",  // 运行日志 
		"imageLog_Edit", // 图像日志 
		"objectLog_Edit", // 对象日志 
		"menuBar",      // 菜单栏 
		"glWidget1",    // OpenGL窗口1 
		"glWidget2"     // OpenGL窗口2 
	};

	// 定义不需要禁用的QAction名称列表 
	static const QSet<QString> excludedActions = {
		"quit_action",  // 退出 
		"about_action", // 关于 
		"useManual_action" // 使用手册 
	};

	// 遍历所有子控件 
	QList<QWidget*> widgets = window->findChildren<QWidget*>();
	foreach(QWidget* widget, widgets) {
		if (excludedWidgets.contains(widget->objectName())) {
			continue; // 跳过不需要禁用的控件 
		}

		// 设置启用状态 
		widget->setEnabled(enabled);

		// 对于禁用的控件设置等待光标 
		if (!enabled && !excludedWidgets.contains(widget->objectName())) {
			widget->setCursor(Qt::WaitCursor);
		}
		else {
			widget->unsetCursor();
		}
	}

	// 处理菜单栏中的QAction 
	QList<QAction*> actions = window->findChildren<QAction*>();
	foreach(QAction* action, actions) {
		if (excludedActions.contains(action->objectName())) {
			continue; // 跳过不需要禁用的动作 
		}
		action->setEnabled(enabled);
	}

	// 特别处理主工具栏中的QAction 
	foreach(QAction* action, Map_mainToolBarAction.values()) {
		if (excludedActions.contains(action->objectName())) {
			continue; // 跳过不需要禁用的动作 
		}
		action->setEnabled(enabled);
	}
}

QStringList IMAGEPS::getSelectFilePath(QString flag) const {
	QStringList reFilepath;
	if (flag == QString::fromLocal8Bit("所有类型")) {
		reFilepath << DataModelPath << DOMFilePath << DEMFilePath;
		return reFilepath;
	}
	else if (flag == QString::fromLocal8Bit("原始影像"))
		return DataModelPath;
	else if (flag == QString::fromLocal8Bit("DOM"))
		return DOMFilePath;
	else if (flag == QString::fromLocal8Bit("DEM"))
		return DEMFilePath;
	return reFilepath;
}

DataModelConfig* IMAGEPS::getDataModelConfig() const {
	return dataModelConfig;
}

SystemConfig* IMAGEPS::getSystemConfig() const {
	return systemConfig;
}

bool IMAGEPS::getWorkProcessflag() const {
	return OrthoRectificationFusionflag;
}

void IMAGEPS::setWorkProcessflag(bool flag){
	OrthoRectificationFusionflag = flag;
}

//bool IMAGEPS::getDataModelPath(const QString& filePath) const {
//	//return DataModelPath;
//	return DataModelPath.contains(filePath);
//}

// 获取原始数据路径列表
const QStringList& IMAGEPS::getDataModelPath() const{
	return DataModelPath;
}

// 获取DOM参考数据路径列表 
const QStringList& IMAGEPS::getDOMFilePath() const {
	return DOMFilePath;
}

// 获取DEM参考数据路径列表 
const QStringList& IMAGEPS::getDEMFilePath() const {
	return DEMFilePath;
}

// 获取智能镶嵌数据路径列表 
QStringList& IMAGEPS::getSmartMosaicFilePath(){
	SmartMosaicFilePath = syncFileOrderWithTable(SmartMosaicFilePath,
		ui.SmartMosaicDataList_TableW);

	return SmartMosaicFilePath;
}

bool IMAGEPS::getIsCtrlIMG(QString filename, int col) const
{
	// 检查表格和列是否有效
	if (!ui.sateImageDataList_TableW || col < 0 || col >= ui.sateImageDataList_TableW->columnCount())
	{
		qWarning() << "Invalid table or column index";
		return true;
	}

	// 确保文件名不为空
	if (filename.isEmpty())
	{
		qWarning() << "Filename is empty";
		return true;
	}
	QFileInfo fileinfo(filename);
	// 遍历表格 
	for (int row = 0; row < ui.sateImageDataList_TableW->rowCount(); ++row)
	{
		QTableWidgetItem* fileNameItem = ui.sateImageDataList_TableW->item(row, 1);
		if (fileNameItem && fileNameItem->text() == fileinfo.completeBaseName())
		{
			QTableWidgetItem* targetItem = ui.sateImageDataList_TableW->item(row, col);
			if (!targetItem)
			{
				qWarning() << "Target item is null for row" << row << "column" << col;
				return true;
			}

			QString value = targetItem->text();
			return (value.compare(QString::fromLocal8Bit("是"), Qt::CaseInsensitive) == 0 ||
				value.compare("Yes", Qt::CaseInsensitive) == 0);
		}
	}

	qWarning() << "Filename not found in table:" << filename;
	return true;
}

/**
 * @brief 高亮点
 */
void IMAGEPS::onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(deselected);

	// 1. 获取信号源表格对象 
	QItemSelectionModel* selModel = qobject_cast<QItemSelectionModel*>(sender());
	if (!selModel) return;

	QTableView* sourceView = qobject_cast<QTableView*>(selModel->parent());
	if (!sourceView) return;

	// 2. 确定表格类型和标识 
	QString pointType;
	if (sourceView == ui.encryptPointsList_TableW) {
		pointType = "con";
	}
	else if (sourceView == ui.controlPointsLIst_TableW) {
		pointType = "col";
	}
	else {
		return; // 未知表格忽略 
	}

	// 3. 检查是否单选（行数=1且无组合键）
	const int selectedCount = selModel->selectedRows().count();
	if (selectedCount <= 1 &&
		!(QApplication::keyboardModifiers() & (Qt::ControlModifier | Qt::ShiftModifier))) {
		return; // 跳过单选 
	}

	// 4. 获取选中点ID（带性能优化）
	QSet<QString> pointIds;
	const QModelIndexList selectedRows = selModel->selectedRows(1); // 第2列 

	// 预分配内存（Qt5.14.2兼容方式）
	if (selectedRows.size() > 1000) {
		pointIds.reserve(selectedRows.size());
	}

	foreach(const QModelIndex &idx, selectedRows) {
		pointIds.insert(idx.data().toString());
	}

	// 5. 防抖处理（50ms延迟）
	static QTimer debounceTimer;
	if (!debounceTimer.isActive()) {
		debounceTimer.singleShot(50, this, [=]() {
			// 高亮 
			ui.ImageInfoShow_Widget->highlightPointsById(pointIds, pointType, !pointIds.isEmpty());

			// 滚动到最后选中项（多选时更符合直觉）
			if (!selectedRows.isEmpty() && sourceView) {
				sourceView->scrollTo(selectedRows.last(), QAbstractItemView::PositionAtCenter);
			}
		});
	}
}

void IMAGEPS::onEncryptedPointSelected(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(deselected);

	if (selected.isEmpty())  return;

	int row = selected.indexes().first().row();
	if (row < 0 || row >= m_visiblePointIds.size())  return;

	QString pointId = m_visiblePointIds[row + m_currentPage * PAGE_SIZE];
	if (!m_encryptedPoints.contains(pointId))  return;

	const EncryptedPoint& point = m_encryptedPoints[pointId];

	// 1. 更新lineEdit显示当前点ID 
	ui.lineEdit->setText(pointId);

	// 2. 高亮显示选中的点 
	ui.ImageInfoShow_Widget->clearPointHighlights();
	ui.ImageInfoShow_Widget->highlightPointById(pointId, "con");

	// 3. 更新objectPosPoints_TableW表格 
	ui.objectPosPoints_TableW->setRowCount(0);
	for (const SatImageData& satImage : point.satImages) {
		int row = ui.objectPosPoints_TableW->rowCount();
		ui.objectPosPoints_TableW->insertRow(row);

		ui.objectPosPoints_TableW->setItem(row, 0, new QTableWidgetItem(pointId));
		ui.objectPosPoints_TableW->setItem(row, 1, new QTableWidgetItem(satImage.path));
		ui.objectPosPoints_TableW->setItem(row, 2, new QTableWidgetItem(QString::number(satImage.imageX, 'f', 4)));
		ui.objectPosPoints_TableW->setItem(row, 3, new QTableWidgetItem(QString::number(satImage.imageY, 'f', 4)));
	}
	ui.objectPosPoints_TableW->resizeColumnsToContents();

	// 4. 更新objectPos_TableW表格显示坐标信息 
	ui.objectPos_TableW->setRowCount(3);
	ui.objectPos_TableW->setColumnCount(2);

	ui.objectPos_TableW->setItem(0, 0, new QTableWidgetItem("X"));
	ui.objectPos_TableW->setItem(0, 1, new QTableWidgetItem(point.pointX));

	ui.objectPos_TableW->setItem(1, 0, new QTableWidgetItem("Y"));
	ui.objectPos_TableW->setItem(1, 1, new QTableWidgetItem(point.pointY));

	ui.objectPos_TableW->setItem(2, 0, new QTableWidgetItem("Z"));
	ui.objectPos_TableW->setItem(2, 1, new QTableWidgetItem(point.pointZ));

	ui.objectPos_TableW->resizeColumnsToContents();
}

/**
 * @brief 处理控制点选择事件
 */
void IMAGEPS::onControlPointSelected(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(deselected);

	if (selected.isEmpty())  return;

	int row = selected.indexes().first().row();
	if (row < 0 || row >= m_visibleControlPointIds.size())  return;

	QString pointId = m_visibleControlPointIds[row + m_currentControlPointPage * CONTROL_POINT_PAGE_SIZE];
	if (!m_controlPoints.contains(pointId))  return;

	const ControlPoint& point = m_controlPoints[pointId];

	// 1. 更新lineEdit显示当前点ID
	ui.lineEdit_2->setText(pointId);

	// 2. 高亮显示选中的点
	ui.ImageInfoShow_Widget->clearPointHighlights();
	ui.ImageInfoShow_Widget->highlightPointById(pointId, "col");

	// 3. 更新objectPosPoints_TableW表格 
	ui.objectPosPoints_TableW->setRowCount(0);
	for (const SatImageData& satImage : point.satImages) {
		int row = ui.objectPosPoints_TableW->rowCount();
		ui.objectPosPoints_TableW->insertRow(row);

		ui.objectPosPoints_TableW->setItem(row, 0, new QTableWidgetItem(QString::number(row + 1)));
		ui.objectPosPoints_TableW->setItem(row, 1, new QTableWidgetItem(satImage.path));
		ui.objectPosPoints_TableW->setItem(row, 2, new QTableWidgetItem(QString::number(satImage.imageX, 'f', 6)));
		ui.objectPosPoints_TableW->setItem(row, 3, new QTableWidgetItem(QString::number(satImage.imageY, 'f', 6)));
	}

	// 4. 更新objectPos_TableW表格显示坐标信息 
	ui.objectPos_TableW->setRowCount(3);
	ui.objectPos_TableW->setColumnCount(2);

	ui.objectPos_TableW->setItem(0, 0, new QTableWidgetItem("X"));
	ui.objectPos_TableW->setItem(0, 1, new QTableWidgetItem(point.pointX));

	ui.objectPos_TableW->setItem(1, 0, new QTableWidgetItem("Y"));
	ui.objectPos_TableW->setItem(1, 1, new QTableWidgetItem(point.pointY));

	ui.objectPos_TableW->setItem(2, 0, new QTableWidgetItem("Z"));
	ui.objectPos_TableW->setItem(2, 1, new QTableWidgetItem(point.pointZ));

	// 调整列宽 
	ui.objectPosPoints_TableW->resizeColumnsToContents();
	ui.objectPos_TableW->resizeColumnsToContents();
}

void IMAGEPS::on_pushButton_clicked()
{
	// 获取lineEdit中的查询文本
	QString searchValue = ui.lineEdit->text().trimmed();
	if (searchValue.isEmpty()) {
		QMessageBox::warning(this, u8"查询失败", u8"请输入要查询的内容！");
		return;
	}

	// 遍历encryptPointsList_TableW表格的第2列数据
	//int rowCount = encryptPointsModel->rowCount();
	int rowCount = encryptPointsModel->rowCount();

	bool found = false;

	for (int row = 0; row < rowCount; ++row) {
		// 获取第2列（索引为1）的单元格数据
					// 获取第1列（索引为1）的单元格数据
		QModelIndex keyIndex = encryptPointsModel->index(row, 1); // 第1列
		if (!keyIndex.isValid())  continue;

		QVariant data = encryptPointsModel->data(keyIndex, Qt::DisplayRole);
		QString keyText = data.toString();
		if (keyText.isEmpty())  continue;

		// 比较单元格文本与查询值（区分大小写匹配）
		if (keyText == searchValue) {
			// 选中该行
			ui.encryptPointsList_TableW->selectRow(row);
			found = true;
			break; // 找到第一个匹配项后退出循环
		}
	}

	// 如果未找到匹配项，弹出警告
	if (!found) {
		QMessageBox::warning(this, u8"加密点查询", u8"查询失败！");
	}
}

void IMAGEPS::on_pushButton_2_clicked()
{
	// 获取lineEdit_2中的查询文本
	QString searchValue = ui.lineEdit_2->text().trimmed();
	if (searchValue.isEmpty()) {
		QMessageBox::warning(this, u8"查询失败", u8"请输入要查询的内容！");
		return;
	}

	// 遍历controlPointsLIst_TableW表格的第2列数据
	int rowCount = ControlPointsModel->rowCount();

	bool found = false;

	for (int row = 0; row < rowCount; ++row) {

		// 获取第1列（索引为1）的单元格数据
		QModelIndex keyIndex = ControlPointsModel->index(row, 1); // 第1列
		if (!keyIndex.isValid())  continue;

		QVariant data = ControlPointsModel->data(keyIndex, Qt::DisplayRole);
		QString keyText = data.toString();
		if (keyText.isEmpty())  continue;
		int key = keyText.toInt();

		// 比较单元格文本与查询值（区分大小写匹配）
		if (keyText == searchValue) {
			// 选中该行
			ui.controlPointsLIst_TableW->selectRow(row);
			found = true;
			break; // 找到第一个匹配项后退出循环
		}
	}

	// 如果未找到匹配项，弹出警告
	if (!found) {
		QMessageBox::warning(this, u8"加密点查询", u8"查询失败！");
	}
}

// 在加载表格数据后，将数据备份到 m_encListData
//void IMAGEPS::loadencListData()
//{
//	m_encListData.clear();
//	int rowCount = ui.encryptPointsList_TableW->rowCount();
//	int colCount = ui.encryptPointsList_TableW->columnCount();
//
//	if (rowCount <= 0 || colCount <= 0) return; // 空表格直接返回
//
//	// 预留容量
//	m_encListData.reserve(rowCount);
//
//	// 视图优化设置 
//	bool oldSortingEnabled = ui.encryptPointsList_TableW->isSortingEnabled();
//	ui.encryptPointsList_TableW->setSortingEnabled(false);
//	ui.encryptPointsList_TableW->blockSignals(true);
//
//	for (int row = 0; row < rowCount; ++row)
//	{
//		QList<QVariant> rowData;
//		rowData.reserve(colCount);
//
//		for (int col = 0; col < colCount; ++col)
//		{
//			QTableWidgetItem* item = ui.encryptPointsList_TableW->item(row, col);
//			rowData.append(item ? item->data(Qt::EditRole) : QVariant());
//		}
//
//		m_encListData.append(rowData);
//	}
//
//	// 恢复视图状态
//	ui.encryptPointsList_TableW->setSortingEnabled(oldSortingEnabled);
//	ui.encryptPointsList_TableW->blockSignals(false);
//
//	// 触发必要的更新
//	ui.encryptPointsList_TableW->viewport()->update();
//}

// 在加载表格数据后，将数据备份到 m_encListData
void IMAGEPS::loadencListData()
{
	m_encListData.clear();

	// 获取 QTableView 的模型（假设使用 NonEditableModel）
	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.encryptPointsList_TableW->model());
	if (!model) {
		qWarning() << "QTableView does not have a NonEditableModel set!";
		return;
	}

	int rowCount = model->rowCount();
	int colCount = model->columnCount();

	if (rowCount <= 0 || colCount <= 0) return; // 空表格直接返回

	// 预留容量
	m_encListData.reserve(rowCount);

	// 视图优化设置  
	bool oldSortingEnabled = ui.encryptPointsList_TableW->isSortingEnabled();
	ui.encryptPointsList_TableW->setSortingEnabled(false);
	ui.encryptPointsList_TableW->blockSignals(true);

	for (int row = 0; row < rowCount; ++row)
	{
		QList<QVariant> rowData;
		rowData.reserve(colCount);

		for (int col = 0; col < colCount; ++col)
		{
			// 从模型中获取数据，使用 Qt::EditRole
			QVariant data = model->data(model->index(row, col), Qt::EditRole);
			rowData.append(data);
		}

		m_encListData.append(rowData);
	}

	// 恢复视图状态
	ui.encryptPointsList_TableW->setSortingEnabled(oldSortingEnabled);
	ui.encryptPointsList_TableW->blockSignals(false);

	// 触发必要的更新
	ui.encryptPointsList_TableW->viewport()->update();
}

// 在加载表格数据后，将数据备份到 m_colListData
void IMAGEPS::loadcolListData()
{
	m_colListData.clear();

	// 获取 QTableView 的模型
	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.controlPointsLIst_TableW->model());
	if (!model) {
		qWarning() << "QTableView does not have a NonEditableModel set!";
		return;
	}

	int rowCount = model->rowCount();
	int colCount = model->columnCount();

	if (rowCount <= 0 || colCount <= 0) return; // 空表格直接返回

	// 预留容量
	m_colListData.reserve(rowCount);

	// 视图优化设置  
	bool oldSortingEnabled = ui.controlPointsLIst_TableW->isSortingEnabled();
	ui.controlPointsLIst_TableW->setSortingEnabled(false);
	ui.controlPointsLIst_TableW->blockSignals(true);

	for (int row = 0; row < rowCount; ++row)
	{
		QList<QVariant> rowData;
		rowData.reserve(colCount);

		for (int col = 0; col < colCount; ++col)
		{
			// 从模型中获取数据，使用 Qt::EditRole
			QVariant data = model->data(model->index(row, col), Qt::EditRole);
			rowData.append(data);
		}

		m_colListData.append(rowData);
	}

	// 恢复视图状态
	ui.controlPointsLIst_TableW->setSortingEnabled(oldSortingEnabled);
	ui.controlPointsLIst_TableW->blockSignals(false);

	// 触发必要的更新
	ui.controlPointsLIst_TableW->viewport()->update();
}

void IMAGEPS::onComboBoxIndexChanged(int index)
{
	Q_UNUSED(index);
	// 重置到第一页 
	m_currentPage = 0;
	// 更新可见点 
	updateVisiblePoints();
}

//void IMAGEPS::onComboBoxIndexChanged(int index)
//{
//	// 获取模型指针 
//	QAbstractItemModel* model = ui.encryptPointsList_TableW->model();
//
//	// 如果是NonEditableModel，直接操作；否则创建新模型 
//	NonEditableModel* standardModel = qobject_cast<NonEditableModel*>(model);
//	if (!standardModel) {
//		standardModel = new NonEditableModel(this);
//		ui.encryptPointsList_TableW->setModel(standardModel);
//	}
//
//	// 清除原有内容 
//	standardModel->setRowCount(0);
//
//	// 设置列数 
//	int colCount = m_encListData.isEmpty() ? 0 : m_encListData.first().size();
//	standardModel->setColumnCount(colCount);
//
//	// 填充数据 
//	foreach(const QList<QVariant>& rowData, m_encListData)
//	{
//		QVariant value = rowData.value(2);    // 第二列数据 
//		bool isNumber = false;
//		int numValue = value.toInt(&isNumber);
//
//		// 获取第4列数据（索引为3）
//		QVariant col4Value = rowData.value(4);
//		QString col4Str = col4Value.toString().trimmed().toUpper();
//
//		bool showRow = false;
//		switch (index)
//		{
//		case 0: showRow = true; break;       // 显示所有数据 
//		case 1: showRow = (numValue < 3); break;  // 显示numValue < 3的数据 
//		case 2: showRow = (numValue >= 2); break; // 显示numValue >= 2的数据 
//		case 3: showRow = (numValue >= 3); break; // 显示numValue >= 3的数据 
//		case 4: showRow = (numValue >= 4); break; // 显示numValue >= 4的数据 
//		case 5: showRow = (numValue >= 5); break; // 显示numValue >= 5的数据 
//		case 6: showRow = (numValue >= 6); break; // 显示numValue >= 6的数据 
//		case 7: showRow = (col4Str == "Y"); break; // 显示第4列为"Y"的数据 
//		case 8: showRow = (col4Str == "N"); break; // 显示第4列为"N"的数据 
//		default: showRow = false; break;     // 其他情况不显示 
//		}
//
//		if (showRow)
//		{
//			QList<QStandardItem*> items;
//			for (int col = 0; col < colCount; ++col)
//			{
//				QStandardItem* item = new QStandardItem(rowData[col].toString());
//				items.append(item);
//			}
//			standardModel->appendRow(items);
//		}
//	}
//
//	// 更新数量显示 
//	ui.encryptPointsnum->setText(QString::fromLocal8Bit(" 加密点列表数量: %1").arg(standardModel->rowCount()));
//
//}

void IMAGEPS::onComboBox_2IndexChanged(int index)
{
	// 获取模型指针
	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.controlPointsLIst_TableW->model());
	if (!model) {
		model = new NonEditableModel(0, 10, this); // 初始化10列
		ui.controlPointsLIst_TableW->setModel(model);
		model->setHorizontalHeaderLabels({
			u8"序号", u8"ID", u8"类型", u8"重叠度", u8"vxy(像方)", u8"vxy(物方)", u8"vz(高程)", u8"状态", u8"权重", " "
			});
	}

	// 禁用视图更新
	ui.controlPointsLIst_TableW->setUpdatesEnabled(false);

	// 批量准备数据
	QList<QList<QVariant>> batchData;
	int row = 0;

	for (const QString& pointId : m_visibleControlPointIds) {
		const ControlPoint& point = m_controlPoints[pointId];

		bool showRow = false;
		switch (index) {
		case 0: showRow = true; break;
		case 1: showRow = (point.state == "Y"); break;
		row2: showRow = (point.state == "N"); break;
		default: showRow = false; break;
		}

		if (showRow) {
			QList<QVariant> rowData;
			rowData.reserve(10);

			rowData.append(QString::number(row + 1));           // 序号
			rowData.append(point.id);                              // ID
			rowData.append(point.type);                            // 类型
			rowData.append(QString::number(point.overlap));        // 重叠度
			rowData.append(point.imageSide);                       // 影像侧残差
			rowData.append(point.objectSide);                      // 物方侧残差
			rowData.append(point.height);                          // 高度
			rowData.append(point.state);                           // 状态
			rowData.append(point.priority);                        // 优先级
			rowData.append("");                                    // 空列

			batchData.append(rowData);
			row++;
		}
	}

	// 批量设置数据（高性能）
	model->setDataBatch(batchData);

	// 启用视图更新
	ui.controlPointsLIst_TableW->setUpdatesEnabled(true);

	// 调整列宽
	ui.controlPointsLIst_TableW->resizeColumnsToContents();

	//ControlPointsModel = model;

	// 更新数量显示
	ui.controlPointsnum->setText(QString::fromLocal8Bit("   控制点数量: %1").arg(model->rowCount()));
}

//void IMAGEPS::onComboBox_2IndexChanged(int index)
//{
//	// 获取模型指针
//	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.controlPointsLIst_TableW->model());
//	if (!model) {
//		model = new NonEditableModel(this);
//		ui.controlPointsLIst_TableW->setModel(model);
//	}
//
//	// 清除原有内容
//	model->setRowCount(0);
//
//	// 设置列数 
//	model->setColumnCount(10); // 序号、ID、类型、重叠度、残差、影像侧、物方侧、状态、优先级
//
//	// 填充数据 
//	int row = 0;
//	for (const QString& pointId : m_visibleControlPointIds) {
//		const ControlPoint& point = m_controlPoints[pointId];
//
//		bool showRow = false;
//		switch (index) {
//		case 0: showRow = true; break;
//		case 1: showRow = (point.state == "Y"); break;
//		case 2: showRow = (point.state == "N"); break;
//		default: showRow = false; break;
//		}
//
//		if (showRow) {
//			QList<QStandardItem*> items;
//			items.append(new  QStandardItem(QString::number(row + 1))); // 序号 
//			items.append(new  QStandardItem(point.id));                  // ID 
//			items.append(new  QStandardItem(point.type));                // 类型
//			items.append(new  QStandardItem(QString::number(point.overlap)));  // 重叠度 
//			items.append(new  QStandardItem(point.imageSide));           // 影像侧残差
//			items.append(new  QStandardItem(point.objectSide));          // 物方侧残差
//			items.append(new  QStandardItem(point.height));              // 高度 
//			items.append(new  QStandardItem(point.state));               // 状态
//			items.append(new  QStandardItem(point.priority));            // 优先级
//			items.append(new  QStandardItem(""));            // 优先级
//
//			model->appendRow(items);
//			row++;
//		}
//	}
//
//	// 设置表头 
//	if (model->horizontalHeaderItem(0) == nullptr) {
//		model->setHorizontalHeaderLabels({
//			u8"序号", u8"ID", u8"类型", u8"重叠度", u8"vxy(像方)", u8"vxy(物方)", u8"vz(高程)", u8"状态", u8"权重", " "
//			});
//	}
//	ControlPointsModel = model;
//	// 更新数量显示
//	ui.controlPointsnum->setText(QString::fromLocal8Bit("  控制点数量: %1").arg(model->rowCount()));
//}

//void IMAGEPS::onComboBox_2IndexChanged(int index)
//{
//	// 获取模型指针 
//	QAbstractItemModel* model = ui.controlPointsLIst_TableW->model();
//
//	// 如果是NonEditableModel，直接操作；否则创建新模型 
//	NonEditableModel* standardModel = qobject_cast<NonEditableModel*>(model);
//	if (!standardModel) {
//		standardModel = new NonEditableModel(this);
//		ui.controlPointsLIst_TableW->setModel(standardModel);
//	}
//
//	// 清除原有内容 
//	standardModel->setRowCount(0);
//
//	// 设置列数 
//	int colCount = m_colListData.isEmpty() ? 0 : m_colListData.first().size();
//	standardModel->setColumnCount(colCount);
//
//	// 填充数据 
//	foreach(const QList<QVariant>& rowData, m_colListData)
//	{
//		// 获取第8列数据（索引为7）
//		QVariant col7Value = rowData.value(7);
//		QString col7Str = col7Value.toString().trimmed().toUpper();
//
//		bool showRow = false;
//		switch (index)
//		{
//		case 0: showRow = true; break;
//		case 1: showRow = (col7Str == "Y"); break; // 显示第7列为"Y"的数据 
//		case 2: showRow = (col7Str == "N"); break; // 显示第7列为"N"的数据 
//		default: showRow = false; break;
//		}
//
//		if (showRow)
//		{
//			QList<QStandardItem*> items;
//			for (int col = 0; col < colCount; ++col)
//			{
//				QStandardItem* item = new QStandardItem(rowData[col].toString());
//				items.append(item);
//			}
//			standardModel->appendRow(items);
//		}
//	}
//
//	// 更新数量显示 
//	ui.controlPointsnum->setText(QString::fromLocal8Bit(" 加密点列表数量: %1").arg(standardModel->rowCount()));
//
//}

//void IMAGEPS::onComboBox_2IndexChanged(int index)
//{
//	// 清空模型 
//	ControlPointsModel->removeRows(0, ControlPointsModel->rowCount());
//
//	int colCount = m_colListData.isEmpty() ? 0 : m_colListData.first().size();
//
//	QList<QStandardItem*> visibleRows;
//
//	foreach(const QList<QVariant>& rowData, m_colListData)
//	{
//		// 获取第8列数据（索引为7）
//		QVariant col7Value = rowData.value(7);
//		QString col7Str = col7Value.toString().trimmed().toUpper();
//
//		bool showRow = false;
//		switch (index)
//		{
//		case 0: showRow = true; break;
//		case 1: showRow = (col7Str == "Y"); break; // 显示第7列为"Y"的数据 
//		case 2: showRow = (col7Str == "N"); break; // 显示第7列为"N"的数据 
//		default: showRow = false; break;
//		}
//
//		if (showRow)
//		{
//			QList<QStandardItem*> rowItems;
//			for (int col = 0; col < colCount; ++col)
//			{
//				QStandardItem* item = new QStandardItem(rowData[col].toString());
//				rowItems.append(item);
//			}
//			visibleRows.append(rowItems);
//		}
//	}
//
//	// 批量添加行
//	foreach(auto& row, visibleRows) {
//		ControlPointsModel->appendRow(row);
//	}
//
//	// 更新数量显示
//	ui.controlPointsnum->setText(QString::fromLocal8Bit(" 控制点列表数量: %1").arg(ControlPointsModel->rowCount()));
//}

void IMAGEPS::updateVisiblePointsFromTables() {
	QSet<QString> ids;

	//// 从加密点表格获取ID
	//for (int row = 0; row < ui.encryptPointsList_TableW->rowCount(); ++row) {
	//	QTableWidgetItem* item = ui.encryptPointsList_TableW->item(row, 1); // 第1列是ID 
	//	if (item && !item->text().isEmpty()) {
	//		ids.insert(item->text());
	//	}
	//}
	QAbstractItemModel* modelenc = ui.encryptPointsList_TableW->model();

	// 获取加密点表格的模型
	if (modelenc) {
		for (int row = 0; row < modelenc->rowCount(); ++row) {
			QModelIndex index = modelenc->index(row, 1); // 第1列是ID
			QString id = index.data(Qt::DisplayRole).toString();
			if (!id.isEmpty()) {
				ids.insert(id);
			}
		}
	}

	QAbstractItemModel* modelcol = ui.controlPointsLIst_TableW->model();
	if (modelcol) {
		for (int row = 0; row < modelcol->rowCount(); ++row) {
			QModelIndex index = modelcol->index(row, 1); // 第1列是ID
			QString id = index.data(Qt::DisplayRole).toString();
			if (!id.isEmpty()) {
				ids.insert(id);
			}
		}
	}

	//// 从控制点表格获取ID
	//for (int row = 0; row < ui.controlPointsLIst_TableW->rowCount(); ++row) {
	//	QTableWidgetItem* item = ui.controlPointsLIst_TableW->item(row, 1); // 第1列是ID 
	//	if (item && !item->text().isEmpty()) {
	//		ids.insert(item->text());
	//	}
	//}

	ui.ImageInfoShow_Widget->setVisiblePointIds(ids);
	ui.ImageInfoShow_Widget->setOnlyShowVisibleIds(true); // 只显示这些ID的点
}

void IMAGEPS::asyncUpdateEncData(const QSet<QString>& selectedIDs, QString flag) {
	// 在后台线程中更新数据结构 
	QtConcurrent::run([this, selectedIDs, flag]() {
		for (const QString& id : selectedIDs) {
			if (m_encryptedPoints.contains(id)) {
				m_encryptedPoints[id].state = flag;
			}
		}

		// 通知主线程更新UI 
		QMetaObject::invokeMethod(this, [this]() {
			updateVisiblePoints();
		});
	});
}

//void IMAGEPS::asyncUpdateEncData(const QSet<QString>& selectedIDs, QString flag) {
//	// 在后台线程中执行耗时操作 
//	QtConcurrent::run([this, selectedIDs, flag]() {
//		static QReadWriteLock lock;
//
//		// 构建只读的ID索引 
//		QHash<QString, int> idToDataIndex;
//		{
//			QReadLocker readLocker(&lock);
//			for (int i = 0; i < m_encListData.size(); ++i) {
//				if (m_encListData[i].size() > 1) {
//					idToDataIndex[m_encListData[i][1].toString()] = i;
//				}
//			}
//		}
//
//		// 收集需要更新的行 
//		QVector<int> rowsToUpdate;
//		for (const QString& id : selectedIDs) {
//			if (idToDataIndex.contains(id)) {
//				rowsToUpdate.append(idToDataIndex[id]);
//			}
//		}
//
//		// 批量更新（加写锁）
//		{
//			QWriteLocker writeLocker(&lock);
//			for (int row : rowsToUpdate) {
//				if (m_encListData[row].size() > 4) {
//					m_encListData[row][4] = flag;
//				}
//			}
//		}
//
//		// 通知主线程更新UI 
//		QMetaObject::invokeMethod(this, [this, selectedIDs, flag]() {
//			QAbstractItemModel* model = ui.encryptPointsList_TableW->model();
//			for (int row = 0; row < model->rowCount(); ++row) {
//				QString id = model->data(model->index(row, 1)).toString();
//				if (selectedIDs.contains(id)) {
//					model->setData(model->index(row, 4), flag, Qt::DisplayRole);
//				}
//			}
//		});
//	});
//}

void IMAGEPS::asyncUpdateColData(const QSet<QString>& selectedIDs, QString flag)
{
	QtConcurrent::run([this, selectedIDs, flag]() {
		//// 加锁确保线程安全
		//QMutexLocker locker(&m_dataMutex);

		// 更新数据结构中的状态
		for (const QString& id : selectedIDs) {
			if (m_controlPoints.contains(id)) {
				m_controlPoints[id].state = flag;
			}
		}

		// 通知主线程更新UI
		QMetaObject::invokeMethod(this, [this]() {
			m_visibleControlPointIds = m_controlPoints.keys();
			loadControlPointCurrentPage();
		});
	});
}

//void IMAGEPS::asyncUpdateColData(const QSet<QString>& selectedIDs, QString flag) {
//	// 在后台线程中执行耗时操作 
//	QtConcurrent::run([this, selectedIDs, flag]() {
//		static QReadWriteLock lock;
//
//		// 构建只读的ID索引 
//		QHash<QString, int> idToDataIndex;
//		{
//			QReadLocker readLocker(&lock);
//			for (int i = 0; i < m_colListData.size(); ++i) {
//				if (m_colListData[i].size() > 1) {
//					idToDataIndex[m_colListData[i][1].toString()] = i;
//				}
//			}
//		}
//
//		// 收集需要更新的行 
//		QVector<int> rowsToUpdate;
//		for (const QString& id : selectedIDs) {
//			if (idToDataIndex.contains(id)) {
//				rowsToUpdate.append(idToDataIndex[id]);
//			}
//		}
//
//		// 批量更新（加写锁）
//		{
//			QWriteLocker writeLocker(&lock);
//			for (int row : rowsToUpdate) {
//				if (m_colListData[row].size() > 7) {
//					m_colListData[row][7] = flag;
//				}
//			}
//		}
//
//		// 通知主线程更新UI 
//		QMetaObject::invokeMethod(this, [this, selectedIDs, flag]() {
//			QAbstractItemModel* model = ui.controlPointsLIst_TableW->model();
//			for (int row = 0; row < model->rowCount(); ++row) {
//				QString id = model->data(model->index(row, 1)).toString();
//				if (selectedIDs.contains(id)) {
//					model->setData(model->index(row, 7), flag, Qt::DisplayRole);
//				}
//			}
//		});
//	});
//}

// 更新当前可见的点 
void IMAGEPS::updateVisiblePoints()
{
	// 获取当前排序和过滤条件 
	QList<QString> filteredIds;
	filteredIds.reserve(m_encryptedPoints.size());
	int filterIndex = ui.comboBox->currentIndex();

	// 获取符合条件的点ID 
	for (auto it = m_encryptedPoints.begin(); it != m_encryptedPoints.end(); ++it) {
		const EncryptedPoint& point = it.value();
		bool showPoint = true;

		switch (filterIndex) {
		case 0: break; // 显示所有 
		case 1: showPoint = (point.overlap < 3); break;
		case 2: showPoint = (point.overlap >= 2); break;
		case 3: showPoint = (point.overlap >= 3); break;
		case 4: showPoint = (point.overlap >= 4); break;
		case 5: showPoint = (point.overlap >= 5); break;
		case 6: showPoint = (point.overlap >= 6); break;
		case 7: showPoint = (point.state == "Y"); break;
		case 8: showPoint = (point.state == "N"); break;
		default: showPoint = false; break;
		}

		if (showPoint) {
			filteredIds.append(point.id);
		}
	}

	// 按照第2列（ID）进行排序 
	std::sort(filteredIds.begin(), filteredIds.end(), [](const QString& id1, const QString& id2) {
		return id1 < id2; // 升序排序 
	});

	// 更新可见点ID列表 
	m_visiblePointIds = filteredIds;

	// 加载当前页数据 
	loadCurrentPage();
}

void IMAGEPS::loadCurrentPage()
{
	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.encryptPointsList_TableW->model());
	if (!model) {
		model = new NonEditableModel(0, 6, this);
		ui.encryptPointsList_TableW->setModel(model);
		model->setHorizontalHeaderLabels({ u8"序号", u8"ID", u8"重叠度", u8"残差", u8"状态", " " });
	}

	// 禁用视图更新
	ui.encryptPointsList_TableW->setUpdatesEnabled(false);

	int start = m_currentPage * PAGE_SIZE;
	int end = qMin(start + PAGE_SIZE, m_visiblePointIds.size());

	//if (start >= end) {
	//	//model->setRowCount(0);
	//	//ui.encryptPointsList_TableW->setUpdatesEnabled(true);
	//	//return;
	//}

	// 批量准备数据
	QList<QList<QVariant>> batchData;
	batchData.reserve(end - start);

	for (int i = start; i < end; i++) {
		const QString& id = m_visiblePointIds[i];
		const EncryptedPoint& point = m_encryptedPoints[id];

		QList<QVariant> rowData;
		rowData.reserve(6);

		rowData.append(QString::number(i + 1));           // 序号
		rowData.append(point.id);                          // ID 
		rowData.append(QString::number(point.overlap));    // 重叠度
		rowData.append(point.residual);                    // 残差
		rowData.append(point.state);                       // 状态
		rowData.append("");                                // 空列

		batchData.append(rowData);
	}

	// 批量设置数据（高性能）
	model->setDataBatch(batchData);

	ui.encryptPointsList_TableW->setUpdatesEnabled(true);
	ui.encryptPointsList_TableW->resizeColumnsToContents();

	ui.encryptPointsnum->setText(QString::fromLocal8Bit("  加密点列表数量: %1")
		.arg(m_visiblePointIds.size())
		.arg(start + 1)
		.arg(end));
}

void IMAGEPS::loadControlPointCurrentPage()
{
	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.controlPointsLIst_TableW->model());
	if (!model) {
		model = new NonEditableModel(0, 10, this);
		ui.controlPointsLIst_TableW->setModel(model);
		model->setHorizontalHeaderLabels({
			u8"序号", u8"ID", u8"类型", u8"重叠度", u8"vxy(像方)", u8"vxy(物方)", u8"vz(高程)", u8"状态", u8"权重", " "
			});
	}

	// 禁用视图更新 
	ui.controlPointsLIst_TableW->setUpdatesEnabled(false);

	int start = m_currentControlPointPage * CONTROL_POINT_PAGE_SIZE;
	int end = qMin(start + CONTROL_POINT_PAGE_SIZE, m_visibleControlPointIds.size());

	//if (start >= end) {
	//	model->setRowCount(0);
	//	ui.controlPointsLIst_TableW->setUpdatesEnabled(true);
	//	return;
	//}

	// 批量准备数据
	QList<QList<QVariant>> batchData;
	batchData.reserve(end - start);

	for (int i = start; i < end; i++) {
		const QString& id = m_visibleControlPointIds[i];
		const ControlPoint& point = m_controlPoints[id];

		QList<QVariant> rowData;
		rowData.reserve(10);

		rowData.append(QString::number(i + 1));           // 序号 
		rowData.append(point.id);                          // ID
		rowData.append(point.type);                        // 类型
		rowData.append(QString::number(point.overlap));    // 重叠度 
		rowData.append(point.imageSide);                   // 影像侧 
		rowData.append(point.objectSide);                  // 物方侧
		rowData.append(point.height);                      // 高程
		rowData.append(point.state);                       // 状态
		rowData.append(point.priority);                    // 优先级 
		rowData.append("");                                // 空列 

		batchData.append(rowData);
	}

	// 批量设置数据（高性能）
	model->setDataBatch(batchData);

	ui.controlPointsLIst_TableW->setUpdatesEnabled(true);
	ui.controlPointsLIst_TableW->resizeColumnsToContents();

	ui.controlPointsnum->setText(QString::fromLocal8Bit("   控制点列表数量: %1").arg(m_controlPoints.size()));
}

//智能镶嵌表格顺序调整
void IMAGEPS::moveSelectedRows(QTableWidget *table, int direction) {
	if (!table) return;

	// 获取选中的所有行（按顺序处理）
	QList<int> selectedRows;
	for (QTableWidgetItem *item : table->selectedItems()) {
		if (!selectedRows.contains(item->row())) {
			selectedRows.append(item->row());
		}
	}
	std::sort(selectedRows.begin(), selectedRows.end());

	// 根据移动方向调整处理顺序
	if (direction > 0) {
		std::reverse(selectedRows.begin(), selectedRows.end());
	}

	// 禁用表格更新以提高性能 
	table->setUpdatesEnabled(false);
	table->clearSelection(); // 先清除所有选中状态 

	// 存储移动后的新行号 
	QSet<int> newSelectedRows;

	// 遍历处理每行
	for (int row : selectedRows) {
		int newRow = row + direction;

		// 检查边界 
		if (newRow < 0 || newRow >= table->rowCount()) {
			newSelectedRows.insert(row); // 不能移动的行保持原选中状态 
			continue;
		}

		// 交换行内容 
		swapTableRows(table, row, newRow);
		newSelectedRows.insert(newRow);
	}

	// 恢复所有移动行的选中状态（使用item选择而非selectRow）
	for (int row = 0; row < table->rowCount(); ++row) {
		if (newSelectedRows.contains(row)) {
			for (int col = 0; col < table->columnCount(); ++col) {
				if (QTableWidgetItem* item = table->item(row, col)) {
					item->setSelected(true);
				}
			}
		}
	}

	table->setUpdatesEnabled(true);
}

//void IMAGEPS::moveSelectedRows(QTableWidget *table, int direction) {
//	if (!table) return;
//
//	// 获取选中的所有行（按顺序处理）
//	QList<int> selectedRows;
//	for (QTableWidgetItem *item : table->selectedItems()) {
//		if (!selectedRows.contains(item->row())) {
//			selectedRows.append(item->row());
//		}
//	}
//	std::sort(selectedRows.begin(), selectedRows.end());
//
//	// 根据移动方向调整处理顺序
//	if (direction > 0) {
//		std::reverse(selectedRows.begin(), selectedRows.end());
//	}
//
//	// 禁用表格更新以提高性能
//	table->setUpdatesEnabled(false);
//
//	// 遍历处理每行
//	for (int row : selectedRows) {
//		int newRow = row + direction;
//
//		// 检查边界
//		if (newRow < 0 || newRow >= table->rowCount()) {
//			continue;
//		}
//
//		// 交换行内容
//		swapTableRows(table, row, newRow);
//
//		// 更新选中状态
//		table->selectRow(newRow);
//	}
//
//	table->setUpdatesEnabled(true);
//}

void IMAGEPS::swapTableRows(QTableWidget *table, int row1, int row2) {
	if (!table || row1 == row2) return;

	// 保存行1数据 
	QList<QTableWidgetItem*> row1Items;
	for (int col = 0; col < table->columnCount(); ++col) {
		row1Items.append(table->takeItem(row1, col));
	}

	// 移动行2到行1 
	for (int col = 0; col < table->columnCount(); ++col) {
		table->setItem(row1, col, table->takeItem(row2, col));
	}

	// 将保存的行1数据放到行2 
	for (int col = 0; col < table->columnCount(); ++col) {
		table->setItem(row2, col, row1Items.at(col));
	}

	// 交换行高
	int tempHeight = table->rowHeight(row1);
	table->setRowHeight(row1, table->rowHeight(row2));
	table->setRowHeight(row2, tempHeight);
}

// 预加载的配置缓存 
QVector<SatelliteConfig> IMAGEPS::loadSatelliteConfigs() {
	QVector<SatelliteConfig> configs;
	QFile configFile("../bin/config/SrcDataParseGeoXMLConfig/SmartMosaicgetXmlConfig.csv");

	if (configFile.open(QIODevice::ReadOnly)) {
		QTextStream in(&configFile);
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (line.startsWith("#") && line.contains(":")) {
				QStringList parts = line.mid(1).split(":");
				if (parts.size() >= 2) {
					SatelliteConfig cfg;
					cfg.prefix = parts[0].trimmed().toUpper(); // 统一转为大写比较 
					cfg.xmlSuffix = parts[1].trimmed();
					cfg.timeTag = in.readLine().trimmed(); // 下一行为标签名
					configs.append(cfg);
				}
			}
		}
		configFile.close();
	}
	return configs;
}

// 高效提取时间（带星源匹配）
QString IMAGEPS::extractImageTime(const QString& imagePath, const QString& satelliteName) {
	QFileInfo imageInfo(imagePath);
	QString baseName = imageInfo.completeBaseName().toUpper(); // 转为大写比较 
	QString dirPath = imageInfo.absolutePath();

	// 1. 优先匹配星源特定配置
	for (const auto& cfg : SATELLITE_CONFIGS) {
		if (baseName.startsWith(cfg.prefix)) {
			QString xmlPath = dirPath + "/" + baseName + cfg.xmlSuffix;
			if (QFile::exists(xmlPath)) {
				if (QFile file(xmlPath); file.open(QIODevice::ReadOnly)) {
					QXmlStreamReader xml(&file);
					while (!xml.atEnd()) {
						if (xml.readNext() == QXmlStreamReader::StartElement) {
							if (xml.name().toString().compare(cfg.timeTag, Qt::CaseInsensitive) == 0) {
								QString timeStr = xml.readElementText();
								return timeStr.split('T').first().split(' ').first(); // 提取日期部分
							}
						}
					}
				}
				break; // 匹配到星源配置后立即返回
			}
		}
	}

	// 2. 通用后备方案
	QStringList suffixes;
	for (const auto& cfg : SATELLITE_CONFIGS) {
		if (!suffixes.contains(cfg.xmlSuffix))
			suffixes << cfg.xmlSuffix;
	}

	QDir dir(dirPath);
	for (const QString& suffix : suffixes) {
		QStringList xmlFiles = dir.entryList({ "*" + suffix }, QDir::Files);
		for (const QString& xmlFile : xmlFiles) {
			if (QFile file(dir.filePath(xmlFile)); file.open(QIODevice::ReadOnly)) {
				QXmlStreamReader xml(&file);
				while (!xml.atEnd()) {
					if (xml.readNext() == QXmlStreamReader::StartElement) {
						QString tag = xml.name().toString();
						// 检查通用时间标签
						if (tag.compare("StartTime", Qt::CaseInsensitive) == 0 ||
							tag.compare("TIME", Qt::CaseInsensitive) == 0 ||
							tag.compare("IMAGING_DATE", Qt::CaseInsensitive) == 0 ||
							tag.compare("firstLineTime", Qt::CaseInsensitive) == 0) {
							QString timeStr = xml.readElementText();
							return timeStr.split('T').first().split(' ').first();
						}
					}
				}
			}
		}
	}

	return QString();
}

/**
 * @brief 同步文件路径列表与表格显示顺序
 * @param filePaths 原始文件路径列表（含后缀）
 * @param table 目标表格控件
 * @return 按表格顺序排列的文件路径列表
 */
QStringList IMAGEPS::syncFileOrderWithTable(QStringList& filePaths, QTableWidget* table)
{
	// 构建文件名查找表（不含后缀）
	QHash<QString, QString> filePathMap;
	for (QString& path : filePaths) {
		filePathMap.insert(QFileInfo(path).completeBaseName(), path);
	}

	// 按表格顺序重建路径列表
	QStringList orderedPaths;
	for (int row = 0; row < table->rowCount(); ++row) {
		// 获取表格中的文件名（第1列，索引0）
		QTableWidgetItem* nameItem = table->item(row, 1);
		if (!nameItem) continue;

		QString baseName = nameItem->text();

		// 查找原始路径
		if (filePathMap.contains(baseName)) {
			orderedPaths.append(filePathMap.value(baseName));
		}
		else {
			//qWarning() << "未找到匹配文件:" << baseName;
			PROJECT_LOG_WARNING(this->CurrentConfig, QString::fromLocal8Bit("智能镶嵌 对文件列表排序时未找到对应匹配文件!"));
			//orderedPaths.append("");
		}
	}

	// 验证结果 
	if (orderedPaths.size() != table->rowCount()) {
		qWarning() << "同步后数量不一致（表格:" << table->rowCount()
			<< "文件:" << orderedPaths.size() << ")";
		PROJECT_LOG_WARNING(this->CurrentConfig, QString::fromLocal8Bit("智能镶嵌 同步后数量不一致"));
	}

	return orderedPaths;
}

//void IMAGEPS::loadControlPointCurrentPage()
//{
//	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.controlPointsLIst_TableW->model());
//	if (!model) {
//		model = new NonEditableModel(0, 10, this); // 10列 
//		ui.controlPointsLIst_TableW->setModel(model);
//
//		// 设置表头
//		model->setHorizontalHeaderLabels({
//			u8"序号", u8"ID", u8"类型", u8"重叠度", u8"vxy(像方)", u8"vxy(物方)", u8"vz(高程)", u8"状态", u8"权重", " "
//			});
//	}
//
//	// 清空现有数据
//	model->removeRows(0, model->rowCount());
//
//	int start = m_currentControlPointPage * CONTROL_POINT_PAGE_SIZE;
//	int end = qMin(start + CONTROL_POINT_PAGE_SIZE, m_visibleControlPointIds.size());
//
//	if (start >= end) {
//		return;
//	}
//
//	// 设置新的行数
//	model->setRowCount(end - start);
//
//	// 填充数据 
//	for (int i = start; i < end; i++) {
//		const QString& id = m_visibleControlPointIds[i];
//		const ControlPoint& point = m_controlPoints[id];
//
//		int row = i - start;
//
//		// 使用 setData 设置数据 
//		model->setData(model->index(row, 0), QString::number(i + 1)); // 序号 
//		model->setData(model->index(row, 1), point.id);               // ID 
//		model->setData(model->index(row, 2), point.type);             // 类型
//		model->setData(model->index(row, 3), QString::number(point.overlap));  // 重叠度
//		model->setData(model->index(row, 4), point.imageSide);        // 影像侧
//		model->setData(model->index(row, 5), point.objectSide);       // 物方侧
//		model->setData(model->index(row, 6), point.height);           // 高程 
//		model->setData(model->index(row, 7), point.state);            // 状态
//		model->setData(model->index(row, 8), point.priority);         // 优先级
//		model->setData(model->index(row, 9), "");                    // 空列 
//	}
//
//	ui.controlPointsnum->setText(QString::fromLocal8Bit("   控制点列表数量: %1").arg(m_controlPoints.size()));
//
//	// 调整列宽
//	ui.controlPointsLIst_TableW->resizeColumnsToContents();
//}

///**
// * @brief 加载当前页的控制点数据
// */
//void IMAGEPS::loadControlPointCurrentPage()
//{
//	NonEditableModel* model = qobject_cast<NonEditableModel*>(ui.controlPointsLIst_TableW->model());
//	if (!model) {
//		model = new NonEditableModel(this);
//		ui.controlPointsLIst_TableW->setModel(model);
//	}
//
//	model->setRowCount(0);
//	model->setColumnCount(10); // 序号、ID、类型、重叠度、残差、状态、优先级
//
//	int start = m_currentControlPointPage * CONTROL_POINT_PAGE_SIZE;
//	int end = qMin(start + CONTROL_POINT_PAGE_SIZE, m_visibleControlPointIds.size());
//
//	for (int i = start; i < end; i++) {
//		const QString& id = m_visibleControlPointIds[i];
//		const ControlPoint& point = m_controlPoints[id];
//
//		QList<QStandardItem*> rowItems;
//		rowItems.append(new  QStandardItem(QString::number(i + 1))); // 序号 
//		rowItems.append(new  QStandardItem(point.id));                // ID
//		rowItems.append(new  QStandardItem(point.type));              // 类型 
//		rowItems.append(new  QStandardItem(QString::number(point.overlap)));  // 重叠度
//		rowItems.append(new  QStandardItem(point.imageSide));                     // 残差(初始为空)
//		rowItems.append(new  QStandardItem(point.objectSide));                     // 影像侧 
//		rowItems.append(new  QStandardItem(point.height));                     // 物方侧 
//		rowItems.append(new  QStandardItem(point.state));             // 状态 
//		rowItems.append(new  QStandardItem(point.priority));          // 优先级
//		rowItems.append(new  QStandardItem(""));         
//
//		model->appendRow(rowItems);
//	}
//
//	// 设置表头 
//	if (model->horizontalHeaderItem(0) == nullptr) {
//		model->setHorizontalHeaderLabels({
//			u8"序号", u8"ID", u8"类型", u8"重叠度", u8"vxy(像方)", u8"vxy(物方)", u8"vz(高程)", u8"状态", u8"权重", " "
//			});
//	}
//	ui.controlPointsnum->setText(QString::fromLocal8Bit("  控制点列表数量: %1").arg(m_controlPoints.size()));
//	// 调整列宽 
//	ui.controlPointsLIst_TableW->resizeColumnsToContents();
//}

//void IMAGEPS::setUiEnabled(QWidget* window, bool enabled)
//{
//	// 设置窗口本身 
//	window->setEnabled(enabled);
//
//	// 设置所有子控件 
//	QList<QWidget*> widgets = window->findChildren<QWidget*>();
//	foreach(QWidget* widget, widgets) {
//		// 特殊处理某些不需要禁用的控件 
//		if (widget->objectName() != "alwaysEnabledWidget") {
//			widget->setEnabled(enabled);
//		}
//	}
//
//	// 视觉反馈 - 改变鼠标光标 
//	if (enabled) {
//		QApplication::restoreOverrideCursor();
//	}
//	else {
//		QApplication::setOverrideCursor(Qt::ForbiddenCursor);
//	}
//
//	// 强制界面更新 
//	qApp->processEvents();
//}