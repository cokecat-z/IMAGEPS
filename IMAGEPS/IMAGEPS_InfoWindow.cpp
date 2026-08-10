#include "IMAGEPS_InfoWindow.h"

IMAGEPS_InfoWindow* IMAGEPS_InfoWindow::instance = nullptr;

IMAGEPS_InfoWindow::IMAGEPS_InfoWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setWindowTitle(u8"IMAGEPS DESKTOP多源遥感影像智能处理系统 2026V2.7[*]");

	instance = this;
	QString exeDir = QCoreApplication::applicationDirPath();
	QDir dir(exeDir);
	IMAGEPS_InfoWindow::instance->appDirPath = dir.absolutePath();

	if (!ImageCloudSnowShow)
	{
		ImageCloudSnowShow = new ImageCloudSnowProcessing;

        ImageCloudSnowShow->resize(400, 200);
	}

	if (!HeightAnomaly)
	{
		HeightAnomaly = new HeightAnomalyProcessing;

		//HeightAnomaly->setFixedSize(1200, 350);
		HeightAnomaly->resize(300, 150);
	}

	if (!InvalidValueHandl)
	{
		InvalidValueHandl = new InvalidValueHandling;

		InvalidValueHandl->resize(700, 180);
	}

    if (!imagesuper)
    {
        imagesuper = new ImageSuperResolution;

        imagesuper->resize(600, 600);
    }

    if (!FileDec)
    {
		FileDec = new FileDecompression;

		FileDec->resize(300, 150);
    }


	//if (!RPCTRPB)
	//{
	//	RPCTRPB = new RPC2RPB;

 	//  RPCTRPB->resize(300, 150);
	//}

	initWidget();
	connects();
	addRecentProjectItem();
}

IMAGEPS_InfoWindow::~IMAGEPS_InfoWindow()
{
}


void IMAGEPS_InfoWindow::initWidget()
{
	//this->showMaximized();
	// 延迟执行最大化并强制布局更新
	QTimer::singleShot(0, this, [this]() {
		this->showMaximized();
		this->menuBar()->setMinimumWidth(this->width());
		this->layout()->activate();
	});
	settings = new QSettings("../bin/config/mainToolBarConfig/mainToolBar_inpho.ini", QSettings::IniFormat);
	settings->setIniCodec(QTextCodec::codecForName("UTF-8"));

	QStringList List_menuBar;//一级菜单
	List_menuBar << "Project_menu" << "Stand-Alines_menu" << "tool_menu" << "Window_menu" << "Options_menu"
		<< "help_menu";

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
	//{"Threedimensionalmodelcutting_action", QString::fromLocal8Bit(":/resource/menu/工具/立体模型裁切.png")},
	{"HeightAnomalyElimination_action", QString::fromLocal8Bit(":/resource/menu/子菜单/高程异常消除.png")},
	{"HeightAnomalyDetection_action", QString::fromLocal8Bit(":/resource/menu/子菜单/高程异常查找.png")},
	//{"DSMDEMLogicalConsistencyProcessing_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM逻辑一致性处理.png")},
	//{"ParallelDSM-DEMLogicalConsistencyProcessing_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM逻辑一致性处理(并行).png")},
	//{"DSMDEMStandardizationCheck_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM标准化检查.png")},
	//{"DSMDEMEdgeMatchingCheck_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM接边检查.png")},
	//{"DEMRelatedPropertyCheck_action", QString::fromLocal8Bit(":/resource/menu/子菜单/DEM相关性质检工具.png")},
	{"Invalidvaluereplacement_action", QString::fromLocal8Bit(":/resource/menu/工具/无效值替换.png")},
	{"ImageZeroValueProcessing_action", QString::fromLocal8Bit(":/resource/menu/子菜单/影像零值处理.png")},
	{"ImageZerovalueProcessings_action", QString::fromLocal8Bit(":/resource/menu/工具/影像零值处理(白点)并行版.png")},
	{"Invalidvaluelookup_action", QString::fromLocal8Bit(":/resource/menu/工具/无效值查找.png")},
	};

	for (auto iter : List_menuBar)
	{
		if (settings->value("mainToolBarSetting/" + iter + "Enable").toBool())
		{
			QMenu* tempMenu = ui.menuBar->addMenu(settings->value("mainToolBarSetting/" + iter + "Name").toString());

			QStringList List_tempMenuActions;//一级菜单下按钮
			QMap<QString, QStringList> Map_tempChildMenus;//二级菜单
			if (iter == "Project_menu")
			{
				List_tempMenuActions << "newPro_action" << "openPro_action" << "savePro_action" << "backupsPro_action" << "closePro_action"
					<< "systemSet_action" << "openProDir_action" << "quit_action";
			}
			else if (iter == "Stand-Alines_menu")
			{
				List_tempMenuActions << "equalShow_action" << "allMapShow_action" << "amplify_action" << "narrow_action" << "translation_action";
			}
			else if (iter == "tool_menu")
			{
				List_tempMenuActions << "Buildpyramid_action" << "imageMatchCorrect_action" << "shutdownTask_action" << "ImageCloudandAndStripingDeformationReplacement_menu" << "ImageCropTool_menu"
					<< "DSMDEMInteractiveEditing_action" << "projectChange_action" << "formatChange_action" << "Rpb2Rpc_action" << "RpcToRpb_action"
					<< "Softwaretools_menu" << "RemoteSensingImageDecompression_action" << "ImageResampling_action" << "ImageRangeProduction_action" << "MapExtentGeneration_action"
					<< "DEMMosaic_action" << "ImageCoordinateProcessingTool_action" << "HeightAnomalyProcessing_menu" << "NoDataAttribute_action"/* << "GeoDataExtractorTool_action"*/
					<< "WindowsClusterService_action" << "InvalidValueHandling_menu" << "ColorItoGray_action";
				Map_tempChildMenus["matchResultBackups_menu"] << "controlPointsMatchResultBackups_action" << "connectPointsMatchResultBackups_action";

				Map_tempChildMenus["Softwaretools_menu"] << "ImageFilterTool_action" << "BandSplitTool_action" << "BandOrganizeTool_action" << "BandCompositeTool_action" << "BandMathXTool_action"
					<< "Orthorectification_action" << "ImageFusionSoftware_action" << "TrueColorConversionSoftware_action" << "ImageColorCorrectionSoftware_action"
					<< "ImageMosaicSoftware_action" << "GeoRectifyPlatform_action";

				Map_tempChildMenus["ImageCloudandAndStripingDeformationReplacement_menu"] << "ImageCloudandSnowReplacementInteractive_action" << "ImageCloudandSnowReplacementBatchProcessing_action" << "ImageCloudandSnowReplacementFullyAutomatic_action";

				Map_tempChildMenus["ImageCropTool_menu"] << "ImageCropTool_action" << "ImageCutTool_action" /*<< "Threedimensionalmodelcutting_action"*/;

				Map_tempChildMenus["HeightAnomalyProcessing_menu"] << "HeightAnomalyElimination_action" << "HeightAnomalyDetection_action" << "DSMDEMLogicalConsistencyProcessing_action"
					<< "ParallelDSM-DEMLogicalConsistencyProcessing_action" << "DSMDEMStandardizationCheck_action" << "DSMDEMEdgeMatchingCheck_action" << "DEMRelatedPropertyCheck_action";

				Map_tempChildMenus["InvalidValueHandling_menu"] << "Invalidvaluereplacement_action" << "ImageZeroValueProcessing_action" << "ImageZerovalueProcessings_action" << "Invalidvaluelookup_action";
			}
			else if (iter == "Window_menu")
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
			else if (iter == "Options_menu")
			{
				List_tempMenuActions << "dataModelConfig_action" << "connectPointsMatch_action" << "controlPointsMatch_action";
			}
			else if (iter == "help_menu")
			{
				List_tempMenuActions << "freedomNetAdjustCal_action" << "controlNetAdjustCal_action" << "controlPointsMatch_action";
			}
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

	//工具
	Map_mainToolBarAction["DEMMosaic_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/DEM镶嵌.png")));
	Map_mainToolBarAction["Rpb2Rpc_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/RPB转RPC.png")));
	//Map_mainToolBarAction["RpcToRpb_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/RPC转RPB.png")));
	//Map_mainToolBarAction["GeoDataExtractorTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/参考数据提取.png")));
	Map_mainToolBarAction["formatChange_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/格式转换.png")));
	Map_mainToolBarAction["NoDataAttribute_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/设置NoData属性.png")));
	Map_mainToolBarAction["projectChange_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/投影转换.png")));
	Map_mainToolBarAction["ImageCoordinateProcessingTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像坐标信息处理工具.png")));
	Map_mainToolBarAction["Orthorectification_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/正射纠正软件.png")));
	Map_mainToolBarAction["ImageFusionSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像融合软件.png")));
	Map_mainToolBarAction["TrueColorConversionSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/真彩色转换软件.png")));
	Map_mainToolBarAction["ImageColorCorrectionSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像匀色软件.png")));
	Map_mainToolBarAction["ImageMosaicSoftware_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像镀嵌软件.png")));
	Map_mainToolBarAction["RemoteSensingImageDecompression_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/遥感影像解压缩.png")));
	Map_mainToolBarAction["Buildpyramid_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/创建金字塔.png")));
	Map_mainToolBarAction["ImageResampling_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像重采样.png")));
	Map_mainToolBarAction["ImageRangeProduction_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/影像范围生产.png")));
	Map_mainToolBarAction["MapExtentGeneration_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/图幅范围生成.png")));
	//Map_mainToolBarAction["GeoRectifyPlatform_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/ltmxgcjdzj-icon.png")));
	Map_mainToolBarAction["BandSplitTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段拆分软件.png")));
	Map_mainToolBarAction["BandCompositeTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段合成软件.png")));
	Map_mainToolBarAction["BandMathXTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段计算软件.png")));
	Map_mainToolBarAction["BandOrganizeTool_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/波段重组软件.png")));
	Map_mainToolBarAction["ColorItoGray_action"]->setIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/gszh-icon.png")));

	//QList<QAbstractButton*> buttons = ui.buttonGroup->buttons();
	buttonGroup = { 
		ui.Buildpyramid_btn, ui.projectChange_btn, ui.formatChange_btn , ui.Rpb2Rpc_btn, ui.ImageRangeProduction_btn ,
		ui.MapExtentGeneration_btn, ui.ImageCoordinateProcessing_btn ,ui.HeightAnomalyDetection_btn, /*ui.GeoDataExtractorTool_btn ,*/
		ui.Invalidvaluelookup_btn, ui.DEMMosaic_btn , ui.ImageResampling_btn, ui.imageEqualProject_btn ,
		ui.RemoteSensingImageDecompression_btn, ui.ImageCropTool_btn,ui.NoDataAttribute_btn,  ui.ColorItoGray_btn,
		ui.Convert8to16_btn, ui.ExportWKT_btn
	};

	buttonGroup_model = {
		ui.Satellitedataprocessing_btn,ui.CloudReplaceTools_btn,ui.RDModelRadarDataIn_btn,
        ui.DSMImageProduction_btn, ui.QualityInspection_btn,ui.remotePermissionUpgradeHaspDog_btn , ui.useManual_btn, ui.ImageSuperResolution_btn, ui.Imageprocessing_btn
	};

	// 定义图片路径列表（按按钮顺序匹配）
	QStringList iconPaths = {
		QString::fromLocal8Bit(":/resource/menu/工具/创建金字塔.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/投影转换.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/格式转换.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/RPB转RPC.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/影像范围生产.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/图幅范围生成.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/影像坐标信息处理工具.png"),
		QString::fromLocal8Bit(":/resource/menu/子菜单/高程异常消除.png"),
		//QString::fromLocal8Bit(":/resource/menu/工具/参考数据提取.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/无效值替换.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/DEM镶嵌.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/影像重采样.png"),
		QString::fromLocal8Bit(":/resource/menu/数据预处理/影像赋投影.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/遥感影像解压缩.png"),
		QString::fromLocal8Bit(":/resource/menu/子菜单/影像常规战切.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/设置NoData属性.png"),
		QString::fromLocal8Bit(":/resource/menu/ldyxdscl-icon.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/gctj2.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/ysxqsc.png")
	};

	// 为每个按钮设置图标
	for (int i = 0; i < buttonGroup.size(); ++i) {
        buttonGroup[i]->setIcon(QIcon(iconPaths[i]));
        buttonGroup[i]->setIconSize(QSize(32, 32));
	}

	QStringList iconPaths_model = {
	QString::fromLocal8Bit(":/resource/menu/工具/正射纠正软件.png"),
	QString::fromLocal8Bit(":/resource/menu/drgj-icon.png"),
	QString::fromLocal8Bit(":/resource/menu/工具/影像云雪和拉花变形替换.png"),
	QString::fromLocal8Bit(":/resource/menu/15DSM成果精化.png"),
	QString::fromLocal8Bit(":/resource/menu/工具/影像匀色软件.png"),
	QString::fromLocal8Bit(":/resource/menu/gjj2.png"),
    QString::fromLocal8Bit(":/resource/menu/子菜单/使用手册.png"),
    QString::fromLocal8Bit(":/resource/menu/数据预处理/影像赋投影.png"),
    QString::fromLocal8Bit(":/resource/menu/数据预处理/影像云检测.png")
	};

	// 为每个按钮设置图标
	for (int i = 0; i < buttonGroup_model.size(); ++i) {
		buttonGroup_model[i]->setIcon(QIcon(iconPaths_model[i]));
		buttonGroup_model[i]->setIconSize(QSize(64, 64));
	}
	ui.remotePermissionUpgradeHaspDog_btn->setIconSize(QSize(48, 48));
	ui.useManual_btn->setIconSize(QSize(48, 48));
	ui.ImageSuperResolution_btnf->setVisible(false);
	ui.ImageSuperResolution_btn->setVisible(false);
	//ui.FieldstoShpfile_btnf->setVisible(false);
	//ui.FieldstoShpfile_btn->setVisible(false);

	//ui.GeoDataExtractorTool_btnf->setVisible(false);
	//ui.GeoDataExtractorTool_btn->setVisible(false);
	//QApplication::processEvents();
}

void IMAGEPS_InfoWindow::connects()
{
	connect(this, &IMAGEPS_InfoWindow::aboutToClose_InfoWindow, [this]() {
		for (IMAGEPS* window : m_openWindows) {
			window->close();
		}
		ImageCloudSnowShow->close();
		HeightAnomaly->close();
        InvalidValueHandl->close();
        imagesuper->close();
		//RPCTRPB->close();
	});

	// 3. 连接点击信号
	connect(ui.listWidget, &QListWidget::itemClicked, [this](QListWidgetItem *item) {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return;
		}
		QString path = item->data(Qt::UserRole).toString();
		IMAGEPS* window = new IMAGEPS();
		window->setAttribute(Qt::WA_DeleteOnClose);

		// 连接窗口关闭信号到清理函数
		connect(window, &QObject::destroyed, [this, window]() {
			m_openWindows.removeAll(window);
		});

		// 尝试打开工程
		if (!window->openRecentProject(path)) {
			window->deleteLater();
			//QMessageBox::critical(this, "错误", "打开工程失败");
			return;
		}

		addRecentProjectItem();
		// 打开成功，显示窗口并保存引用
		m_openWindows.append(window);
		window->showMaximized();
	});

	// 4. 支持右键删除单项 
	ui.listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.listWidget, &QListWidget::customContextMenuRequested, [this](const QPoint &pos) {
		//QMenu menu;
		//QAction *deleteAction = menu.addAction(u8"删除");
		//if (deleteAction == menu.exec(ui.listWidget->mapToGlobal(pos))) {
		//	delete ui.listWidget->takeItem(ui.listWidget->row(ui.listWidget->itemAt(pos)));
		//}

		QMenu menu;
		QAction *deleteAction = menu.addAction(u8" 删除");

		if (deleteAction == menu.exec(ui.listWidget->mapToGlobal(pos))) {
			QListWidgetItem *item = ui.listWidget->itemAt(pos);
			if (!item) return;

            // 获取完整项目信息
			QString fullProjectPath = item->data(Qt::UserRole).toString();
			QString projectId = item->text();

			delete ui.listWidget->takeItem(ui.listWidget->row(item));

			QString filePath = "../bin/config/projectPathConfig/projectPath.csv";
			removeLineFromCsv(filePath, fullProjectPath, projectId);
		}
	});

	connect(ui.CloudReplaceTools_btn, &QPushButton::clicked, this, [this]() { 
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		ImageCloudSnowShow->show();
	});
	connect(ui.CloudReplaceTools_btnf, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		ImageCloudSnowShow->show();
	});

	connect(ui.HeightAnomalyDetection_btn, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		HeightAnomaly->show();
	});
	connect(ui.HeightAnomalyDetection_btnf, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		HeightAnomaly->show();
	});

	//connect(ui.ImageCropTool_btn, &QPushButton::clicked, this, [this]() {
	//	// 验证许可证
	//	if (!PublicFunctions::validateDogLicense()) {
	//		return 0;
	//	}
	//	ImageCrop->show();
	//});
	//connect(ui.ImageCropTool_btnf, &QPushButton::clicked, this, [this]() {
	//	// 验证许可证
	//	if (!PublicFunctions::validateDogLicense()) {
	//		return 0;
	//	}
	//	ImageCrop->show();
	//});

	connect(ui.Invalidvaluelookup_btn, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		InvalidValueHandl->show();
	});
	connect(ui.Invalidvaluelookup_btnf, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
        InvalidValueHandl->show();
	});

    connect(ui.ImageSuperResolution_btn, &QPushButton::clicked, this, [this]() {
        // 验证许可证
        if (!PublicFunctions::validateDogLicense()) {
            return 0;
        }
        imagesuper->show();
    });
    connect(ui.ImageSuperResolution_btnf, &QPushButton::clicked, this, [this]() {
        // 验证许可证
        if (!PublicFunctions::validateDogLicense()) {
            return 0;
        }
        imagesuper->show();
    });

	connect(ui.RemoteSensingImageDecompression_btn, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		FileDec->show();
		});
	connect(ui.RemoteSensingImageDecompression_btnf, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		FileDec->show();
		});

	connect(ui.Imageprocessing_btn, &QPushButton::clicked, this, [this]() {
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		bool ok;
		QString ip = QInputDialog::getText(this, u8"输入服务器IP", u8"请输入Web服务IP地址:", QLineEdit::Normal, "192.168.0.104", &ok);
		if (ok && !ip.isEmpty()) {
			QString urlStr = QString("http://%1:8888/web/IMAGEPS/#/login").arg(ip);
			WebBrowserDialog* webDialog = new WebBrowserDialog(QUrl::fromUserInput(urlStr), this);
			webDialog->show();
		}
	});
	connect(ui.Imageprocessing_btnf, &QPushButton::clicked, this, [this]() {
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		bool ok;
		QString ip = QInputDialog::getText(this, u8"输入服务器IP", u8"请输入Web服务IP地址:", QLineEdit::Normal, "192.168.0.104", &ok);
		if (ok && !ip.isEmpty()) {
			QString urlStr = QString("http://%1:8888/web/IMAGEPS/#/login").arg(ip);
			WebBrowserDialog* webDialog = new WebBrowserDialog(QUrl::fromUserInput(urlStr), this);
			webDialog->show();
		}
	});

	//connect(ui.Rpb2Rpc_btn, &QPushButton::clicked, this, [this]() {
	//	// 验证许可证
	//	if (!PublicFunctions::validateDogLicense()) {
	//		return 0;
	//	}
	//	RPCTRPB->show();
	//});
	//connect(ui.Rpb2Rpc_btnf, &QPushButton::clicked, this, [this]() {
	//	// 验证许可证
	//	if (!PublicFunctions::validateDogLicense()) {
	//		return 0;
	//	}
	//	RPCTRPB->show();
	//});

	connect(ui.Satellitedataprocessing_btn, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return;
		}
		IMAGEPS* window = new IMAGEPS();
		window->setAttribute(Qt::WA_DeleteOnClose);

		// 连接窗口关闭信号到清理函数
		connect(window, &QObject::destroyed, [this, window]() {
			m_openWindows.removeAll(window);
		});

		// 尝试打开工程
		if (!window->newProActionSlot()) {
			window->deleteLater();
			return;
		}
		addRecentProjectItem();
		// 打开成功，显示窗口并保存引用
		m_openWindows.append(window);
		window->showMaximized();
	});
	connect(ui.Satellitedataprocessing_btnf, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return;
		}
		IMAGEPS* window = new IMAGEPS();
		window->setAttribute(Qt::WA_DeleteOnClose);

		// 连接窗口关闭信号到清理函数
		connect(window, &QObject::destroyed, [this, window]() {
			m_openWindows.removeAll(window);
		});

		// 尝试打开工程
		if (!window->newProActionSlot()) {
			window->deleteLater();
			return;
		}
		addRecentProjectItem();
		// 打开成功，显示窗口并保存引用
		m_openWindows.append(window);
		window->showMaximized();
	});

    connect(ui.useManual_btn, &QPushButton::clicked, this, []() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		QString filePath = IMAGEPS_InfoWindow::instance->appDirPath +
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
	connect(ui.useManual_btnf, &QPushButton::clicked, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		QString filePath = IMAGEPS_InfoWindow::instance->appDirPath +
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

	connect(Map_mainToolBarAction.value("useManual_action"), &QAction::triggered, this, [this]() {
		// 验证许可证
		if (!PublicFunctions::validateDogLicense()) {
			return 0;
		}
		QString filePath = IMAGEPS_InfoWindow::instance->appDirPath +
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

	m_toolConnections = {
		// 影像云雪自动替换软件（交互式）
		{"PSCloudReplaceTool2.exe",  {
			Map_mainToolBarAction.value("ImageCloudandSnowReplacementInteractive_action",  nullptr),
		}},

		//// 影像云雪自动替换软件（批处理）
		//{"PSCloudReplaceTool.exe",  {
		//	Map_mainToolBarAction.value("ImageCloudandSnowReplacementBatchProcessing_action",  nullptr),
		//}},

		//// 影像云雪自动替换软件（全自动）
		//{"PSAutoCloudReplaceTool.exe",  {
		//	Map_mainToolBarAction.value("ImageCloudandSnowReplacementFullyAutomatic_action",  nullptr),
		//}},

		////创建金字塔
		//{"PSCreatePyramidApp.exe",  {
		//	Map_mainToolBarAction.value("Buildpyramid_action",  nullptr),
		//	ui.Buildpyramid_btn,
		//	ui.Buildpyramid_btnf
		//}},

		////投影转换
		//{"PSImageReprojectTool.exe",  {
		//	Map_mainToolBarAction.value("projectChange_action",  nullptr),
		//	ui.projectChange_btn,
		//	ui.projectChange_btnf
		//}},

		////格式转换
		//{"PSFormatTransTool2.exe",  {
		//	Map_mainToolBarAction.value("formatChange_action",  nullptr),
		//	ui.formatChange_btn,
		//	ui.formatChange_btnf
		//}},

  //      //rpb转rpc
  //      {"PSRpb2Rpc.exe",  {
  //          Map_mainToolBarAction.value("Rpb2Rpc_action",  nullptr)
  //      }},

  //      //RpcToRpb
  //      {"PSRpcToRpb.exe",  {
  //          Map_mainToolBarAction.value("RpcToRpb_action",  nullptr)
  //      }},

  //      //影像范围生成
  //      {"PSGenShpTool.exe",  {
  //          Map_mainToolBarAction.value("ImageRangeProduction_action",  nullptr),
  //          ui.ImageRangeProduction_btn,
  //          ui.ImageRangeProduction_btnf
  //      }},

		////图幅范围生成
		//{"PSMapGenTool.exe",  {
		//	Map_mainToolBarAction.value("MapExtentGeneration_action",  nullptr),
		//	ui.MapExtentGeneration_btn,
		//	ui.MapExtentGeneration_btnf
		//}},

		////影像坐标信息处理工具
		//{"PSGeoInfoRemoveTool.exe", {
		//	Map_mainToolBarAction.value("ImageCoordinateProcessingTool_action",  nullptr),
		//	ui.ImageCoordinateProcessing_btn,
		//	ui.ImageCoordinateProcessing_btnf
		//}},

        ////高程异常消除
        //{"PSRemoveCorseValueTool.exe", {
        //    Map_mainToolBarAction.value("HeightAnomalyDetection_action",  nullptr)
        //}},

        ////高程异常查找
        //{"PSFindCorseValueTool.exe", {
        //    Map_mainToolBarAction.value("HeightAnomalyElimination_action",  nullptr)
        //}},

        ////DSM-DEM逻辑一致性处理
        //{"PSDsmDemCheckTool.exe", {
        //    Map_mainToolBarAction.value("DSMDEMLogicalConsistencyProcessing_action",  nullptr)
        //}},

        ////DSM-DEM逻辑一致性处理（并行）
        //{"PSDsmDemCheckTool2.exe", {
        //    Map_mainToolBarAction.value("ParallelDSM-DEMLogicalConsistencyProcessing_action",  nullptr)
        //}},

        ////DSM-DEM标准化检查
        //{"PSDsmDemStandCheckTool.exe", {
        //    Map_mainToolBarAction.value("DSMDEMStandardizationCheck_action",  nullptr)
        //}},

        ////DSM-DEM接边检查
        //{"PSDsmDemRelCheckTool.exe", {
        //    Map_mainToolBarAction.value("DSMDEMEdgeMatchingCheck_action",  nullptr)
        //}},

        ////DEM相关性质检工具
        //{"PSImageCorrelatonTool.exe", {
        //    Map_mainToolBarAction.value("DEMRelatedPropertyCheck_action",  nullptr)
        //}},
		////设置NoData属性
		//{ "PSInvalidValueTool.exe", {
		//	Map_mainToolBarAction.value("NoDataAttribute_action",  nullptr),
		//	ui.NoDataAttribute_btn,
		//	ui.NoDataAttribute_btnf
		//} },

		////参考数据提取
		//{"PSGeoDataExtractorTool.exe", {
		//	Map_mainToolBarAction.value("GeoDataExtractorTool_action",  nullptr),
		//	ui.GeoDataExtractorTool_btn,
		//	ui.GeoDataExtractorTool_btnf
		//}},

        //无效值替换
        {"PSImageValueRelaceTool.exe", {
            Map_mainToolBarAction.value("Invalidvaluereplacement_action",  nullptr)
        }},

        //影像零值处理(白点)
        {"PSWhiteDotEraseTool.exe", {
            Map_mainToolBarAction.value("ImageZeroValueProcessing_action",  nullptr)
        }},

        //影像零值处理(白点)并行版
        {"PSWhiteDotEraseTool2.exe", {
            Map_mainToolBarAction.value("ImageZerovalueProcessings_action",  nullptr)
        }},

        //无效值查找
        {"PSHoleCheckTool.exe", {
            Map_mainToolBarAction.value("Invalidvaluelookup_action",  nullptr)
        }},

		////影像重采样
		//{"PSImageResampleTool.exe", {
		//	Map_mainToolBarAction.value("ImageResampling_action",  nullptr),
		//	ui.ImageResampling_btn,
		//	ui.ImageResampling_btnf
		//}},
		
		////遥感影像解压缩
		//{"PSUnzipTool.exe", {
		//	Map_mainToolBarAction.value("RemoteSensingImageDecompression_action",  nullptr),
		//	ui.RemoteSensingImageDecompression_btn,
		//	ui.RemoteSensingImageDecompression_btnf
		//}},

   //     //影像常规裁切
   //     {"PSImageCropTool.exe", {
   //         Map_mainToolBarAction.value("ImageCropTool_action",  nullptr),
			//ui.ImageCropTool_btn,
			//ui.ImageCropTool_btnf
   //     }},

        ////影像多模式裁切
        //{"PSImageCutTool.exe", {
        //    Map_mainToolBarAction.value("ImageCutTool_action",  nullptr)
        //}},

        ////立体模型裁切
        //{"PSImageCropTool_ZY.exe", {
        //    Map_mainToolBarAction.value("Threedimensionalmodelcutting_action",  nullptr)
        //}},

		////R-D模型雷达数据导入
		//{"PSRd2RfmTool.exe", {
		//	Map_mainToolBarAction.value("RDModelRadarDataIn_action",  nullptr),
		//	ui.RDModelRadarDataIn_btn,
		//	ui.RDModelRadarDataIn_btnf
		//}},
				
		//远程许可升级(hasp狗)
		{"RUS_HASP_IMAGEPS.exe", {
			Map_mainToolBarAction.value("remotePermissionUpgradeHaspDog_action",  nullptr),
			ui.remotePermissionUpgradeHaspDog_btn,
			ui.remotePermissionUpgradeHaspDog_btnf
		}},
		// 正射纠正软件
		{"PSOrthoTool.exe",  {
			Map_mainToolBarAction.value("Orthorectification_action",  nullptr),
		}},
		//// 影像融合软件
		//{"PSFusionTool2.exe",  {
		//	Map_mainToolBarAction.value("ImageFusionSoftware_action",  nullptr),
		//}},
		//// 真彩色转换软件
		//{"PSImageOutByteTool2.exe",  {
		//	Map_mainToolBarAction.value("TrueColorConversionSoftware_action",  nullptr),
		//}},
		//// 影像匀色软件
		//{"PSDodgingTool2.exe",  {
		//	Map_mainToolBarAction.value("ImageColorCorrectionSoftware_action",  nullptr),
		//}},
		//// 影像镶嵌软件
		//{"PSMosaicTool2.exe",  {
		//	Map_mainToolBarAction.value("ImageMosaicSoftware_action",  nullptr),
		//}},
		//// 配准纠正软件
		//{"PSGeoRectifyPlatform.exe", {
		//	Map_mainToolBarAction.value("GeoRectifyPlatform_action",  nullptr),
		//}}
	};

	m_moduleConnections = {
		// 影像质检软件
		{"QualityInspection.exe",  {
			ui.QualityInspection_btn,
			ui.QualityInspection_btnf
		}},
		
		// DSM处理
		{"DSMImageProduction.exe",  {
			ui.DSMImageProduction_btn,
			ui.DSMImageProduction_btnf
		}},

		// 灰度转换
		{"ColorItoGray.exe",  {
			Map_mainToolBarAction.value("ColorItoGray_action",  nullptr),
			ui.ColorItoGray_btn,
			ui.ColorItoGray_btnf
		}},

		// 影像融合软件
		{"PSImageInter.exe",  {
			Map_mainToolBarAction.value("ImageFusionSoftware_action",  nullptr),
		}},

		// 真彩色转换软件
		{"TrueColorConversion.exe",  {
			Map_mainToolBarAction.value("TrueColorConversionSoftware_action",  nullptr),
		}},

		// 影像匀色软件
		{"ImageColorCorrection.exe",  {
			Map_mainToolBarAction.value("ImageColorCorrectionSoftware_action",  nullptr),
		}},
		// 影像镶嵌软件
		{"ImageMosaic.exe",  {
			Map_mainToolBarAction.value("ImageMosaicSoftware_action",  nullptr),
		}},

		// 影像云雪自动替换软件（批处理）
		{"PSImageCloudandSnowProcessing.exe",  {
			Map_mainToolBarAction.value("ImageCloudandSnowReplacementBatchProcessing_action",  nullptr),
		}},

		// 影像云雪自动替换软件（全自动）
		{"PSAutoCloudReplace.exe",  {
			Map_mainToolBarAction.value("ImageCloudandSnowReplacementFullyAutomatic_action",  nullptr),
		}},

		//创建金字塔
		{"PSCreatePyramid.exe",  {
			Map_mainToolBarAction.value("Buildpyramid_action",  nullptr),
			ui.Buildpyramid_btn,
			ui.Buildpyramid_btnf
		}},

		//投影转换
		{"ProjectionTransformation.exe",  {
			Map_mainToolBarAction.value("projectChange_action",  nullptr),
			ui.projectChange_btn,
			ui.projectChange_btnf
		}},

		//格式转换
		{"FormatConversion.exe",  {
			Map_mainToolBarAction.value("formatChange_action",  nullptr),
			ui.formatChange_btn,
			ui.formatChange_btnf
		}},

		//rpb转rpc
		{"PSRfmTrans.exe",  {
			Map_mainToolBarAction.value("Rpb2Rpc_action",  nullptr),
			ui.Rpb2Rpc_btn,
			ui.Rpb2Rpc_btnf
		}},

		//影像常规裁切
		{"ImageCropTool.exe", {
			Map_mainToolBarAction.value("ImageCropTool_action",  nullptr),
			ui.ImageCropTool_btn,
			ui.ImageCropTool_btnf
		}},

		//影像范围生成
		{"PSImageRange.exe",  {
			Map_mainToolBarAction.value("ImageRangeProduction_action",  nullptr),
			ui.ImageRangeProduction_btn,
			ui.ImageRangeProduction_btnf
		}},

		//图幅范围生成
		{"PSMapNumCal.exe",  {
			Map_mainToolBarAction.value("MapExtentGeneration_action",  nullptr),
			ui.MapExtentGeneration_btn,
			ui.MapExtentGeneration_btnf
		}},

		//影像坐标信息处理工具
		{"PSExportTfw.exe", {
			Map_mainToolBarAction.value("ImageCoordinateProcessingTool_action",  nullptr),
			ui.ImageCoordinateProcessing_btn,
			ui.ImageCoordinateProcessing_btnf
		}},

		//设置NoData属性
		{ "PSSetNoData.exe", {
			Map_mainToolBarAction.value("NoDataAttribute_action",  nullptr),
			ui.NoDataAttribute_btn,
			ui.NoDataAttribute_btnf
		} },

		//高程异常消除
		{"PSRemoveCorseValue.exe", {
			Map_mainToolBarAction.value("HeightAnomalyDetection_action",  nullptr)
		}},

		//高程异常查找
		{ "PSFindCorseValue.exe", {
			Map_mainToolBarAction.value("HeightAnomalyElimination_action",  nullptr)
		} },

		//DEM镶嵌
		{"PSDemMosaic.exe", {
			Map_mainToolBarAction.value("DEMMosaic_action",  nullptr),
			ui.DEMMosaic_btn,
			ui.DEMMosaic_btnf
		}},

		//影像重采样
		{"ImageResampleTool.exe", {
			Map_mainToolBarAction.value("ImageResampling_action",  nullptr),
			ui.ImageResampling_btn,
			ui.ImageResampling_btnf
		}},

		//影像赋投影
		{"Imageprojection.exe", {
			Map_mainToolBarAction.value("imageEqualProject_action",  nullptr),
			ui.imageEqualProject_btn,
			ui.imageEqualProject_btnf
		}},

		// 波段拆分软件
		{"Bandsplitting.exe",  {
			Map_mainToolBarAction.value("BandSplitTool_action",  nullptr),
		}},

		// 波段重组软件
		{"PSBandOrganize.exe",  {
			Map_mainToolBarAction.value("BandOrganizeTool_action",  nullptr),
		}},

		// 波段合成软件
		{"PSBandComposite.exe",  {
			Map_mainToolBarAction.value("BandCompositeTool_action",  nullptr),
		}},

		// 波段计算软件
		{"PSBandMath.exe",  {
			Map_mainToolBarAction.value("BandMathXTool_action",  nullptr),
		}},

		// 影像升位
		{"PSConvert8to16.exe",  {
			ui.Convert8to16_btn,
			ui.Convert8to16_btnf
		}},

		// wkt格式输出
		{"PSExportWKT.exe",  {
			ui.ExportWKT_btn,
			ui.ExportWKT_btnf
		}},

		//R-D模型雷达数据导入
		{ "PSSarModelCreate.exe", {
			Map_mainToolBarAction.value("RDModelRadarDataIn_action",  nullptr),
			ui.RDModelRadarDataIn_btn,
			ui.RDModelRadarDataIn_btnf
		} },
	};

	// 移除空指针 
	for (auto& toolList : m_toolConnections) {
		toolList.erase(std::remove_if(toolList.begin(), toolList.end(),
			[](QObject* obj) { return obj == nullptr; }), toolList.end());
	}

	for (auto it = m_toolConnections.begin(); it != m_toolConnections.end(); ++it) {
		connectMultipleUIToTool(it.key(), it.value());
	}

	// 移除空指针 
	for (auto& toolList : m_moduleConnections) {
		toolList.erase(std::remove_if(toolList.begin(), toolList.end(),
			[](QObject* obj) { return obj == nullptr; }), toolList.end());
	}

	for (auto it = m_moduleConnections.begin(); it != m_moduleConnections.end(); ++it) {
		//connectMultipleUIToTool(it.key(), it.value());
		for (QObject* uiElement : it.value()) {
			if (QAction* action = qobject_cast<QAction*>(uiElement)) {
				if (action) {
					connect(action, &QAction::triggered, this, [=]() {
						// 验证许可证
						if (!PublicFunctions::validateDogLicense()) {
							return 0;
						}
						QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
							"/" + it.key();
						QProcess* process = new QProcess(this);
						process->start(fullToolPath);
					});
				}
			}
			else if (QPushButton* button = qobject_cast<QPushButton*>(uiElement)) {
				if (button) {
					connect(button, &QPushButton::clicked, this, [=]() {
						// 验证许可证
						if (!PublicFunctions::validateDogLicense()) {
							return 0;
						}
						QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
							"/" + it.key();
						QProcess* process = new QProcess(this);
						process->start(fullToolPath);
					});
				}
			}
		}
	}

	connect(Map_mainToolBarAction["newPro_action"], &QAction::triggered, this, &IMAGEPS_InfoWindow::newProActionSlot);
	connect(Map_mainToolBarAction["openPro_action"], &QAction::triggered, this, &IMAGEPS_InfoWindow::openProActionSlot);

}

void IMAGEPS_InfoWindow::newProActionSlot()
{
	// 验证许可证
	if (!PublicFunctions::validateDogLicense()) {
		return;
	}
	IMAGEPS* window = new IMAGEPS();
	window->setAttribute(Qt::WA_DeleteOnClose);

	// 连接窗口关闭信号到清理函数
	connect(window, &QObject::destroyed, [this, window]() {
		m_openWindows.removeAll(window);
	});

	// 尝试打开工程
	if (!window->newProActionSlot()) {
		window->deleteLater();
		return;
	}
	
	addRecentProjectItem();
	// 打开成功，显示窗口并保存引用
	m_openWindows.append(window);
	window->showMaximized();
}

void IMAGEPS_InfoWindow::openProActionSlot()
{
	// 验证许可证
	if (!PublicFunctions::validateDogLicense()) {
		return;
	}
	IMAGEPS* window = new IMAGEPS(); 
	window->setAttribute(Qt::WA_DeleteOnClose); 

	// 连接窗口关闭信号到清理函数
	connect(window, &QObject::destroyed, [this, window]() {
		m_openWindows.removeAll(window);  
	});

	// 尝试打开工程
	if (!window->openProActionSlot()) { 
		window->deleteLater(); 
		//QMessageBox::critical(this, "错误", "打开工程失败");
		return;
	}

	addRecentProjectItem();
	// 打开成功，显示窗口并保存引用
	m_openWindows.append(window);
	window->showMaximized();
}

//void IMAGEPS_InfoWindow::addRecentProjectItem() {
//	ui.listWidget->clear();
//	QString filePath = "../bin/config/projectPathConfig/projectPath.csv";
//	QHash<QString, QString> fileMap = PublicFunctions::loadFromCsv(filePath);
//
//	int count = 0;
//	const int maxRecentFiles = 10;
//
//	for (auto it = fileMap.begin(); it != fileMap.end() && count < maxRecentFiles; ++it, ++count) {
//		QString fileName = it.key();
//		QString filePath = it.value();
//		QFileInfo fileInfo(fileName);
//		QListWidgetItem *item = new QListWidgetItem(
//			QIcon(QString::fromLocal8Bit(":/resource/menu/RecentP/文件夹.png")),
//			QString("%1\n%2").arg(fileInfo.completeBaseName()).arg(filePath)
//		);
//		item->setData(Qt::UserRole, fileName);  // 存储完整路径
//
//		ui.listWidget->addItem(item);
//	}
//}

void IMAGEPS_InfoWindow::addRecentProjectItem() {
	ui.listWidget->clear();
	QString filePath = "../bin/config/projectPathConfig/projectPath.csv";

	// 获取文件内容并附带最后修改时间
	QVector<QPair<QString, QDateTime>> projectsWithTime;

	QFile file(filePath);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&file);
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (line.isEmpty()) continue;

			// 使用安全的 CSV 解析
			QStringList parts = PublicFunctions::parseCsvLine(line);

			if (parts.size() >= 2) {
				QString fileName = PublicFunctions::unescapeCsvField(parts[0]);
				QFileInfo fi(fileName);

				if (fi.exists()) {
					projectsWithTime.append(qMakePair(line, fi.lastModified()));
				}
			}
			file.close();
		}
	}
	else {
		qDebug() << u8"无法打开最近项目文件:" << filePath;
		return;
	}

	// 按修改时间降序排序
	std::sort(projectsWithTime.begin(), projectsWithTime.end(),
		[](const QPair<QString, QDateTime> &a, const QPair<QString, QDateTime> &b) {
		return a.second > b.second;
	});

	// 添加前10个最近项目
	int count = 0;
	const int maxRecentFiles = 10;

	for (const auto &project : projectsWithTime) {
		if (count++ >= maxRecentFiles) break;

		// 解析保存的CSV行
		QStringList parts = PublicFunctions::parseCsvLine(project.first);

		if (parts.size() >= 2) {
			QString fileName = PublicFunctions::unescapeCsvField(parts[0]);
			QString filePath = PublicFunctions::unescapeCsvField(parts[1]);
			QFileInfo fileInfo(fileName);

			// 创建列表项
			QListWidgetItem *item = new QListWidgetItem(
				QIcon(QString::fromLocal8Bit(":/resource/menu/RecentP/文件夹.png")),
				QString("%1\n%2").arg(fileInfo.completeBaseName()).arg(filePath)
			);
			item->setData(Qt::UserRole, fileName);
			ui.listWidget->addItem(item);
		}
	}
}

void IMAGEPS_InfoWindow::startToolProcess(const QString& toolName)
{
	// 验证许可证
	if (!PublicFunctions::validateDogLicense()) {
		return;
	}
#ifdef Q_OS_LINUX 
	// 构建完整的工具路径 
	QString toolPath = IMAGEPS_InfoWindow::instance->appDirPath +
		QString::fromLocal8Bit("/linux64/") + toolName;
#else
	// 构建完整的工具路径 
	QString toolPath = IMAGEPS_InfoWindow::instance->appDirPath +
		QString::fromLocal8Bit("/Software/") + toolName;
#endif

	QProcess* process = new QProcess(this);
	process->start(toolPath);
}

template<typename T>
void IMAGEPS_InfoWindow::connectUIToTool(const QString& toolName, T* uiElement)
{
	if (!uiElement) {
		qWarning() << "UI element is null for tool:" << toolName;
		return;
	}

	// 根据UI元素类型进行连接 
	if (QAction* action = qobject_cast<QAction*>(uiElement)) {
		connectActionToTool(toolName, action);
	}
	else if (QPushButton* button = qobject_cast<QPushButton*>(uiElement)) {
		connectButtonToTool(toolName, button);
	}
	else if (QLabel* label = qobject_cast<QLabel*>(uiElement)) {
		connectLabelToTool(toolName, label);
	}
	else {
		qWarning() << "Unsupported UI element type for tool:" << toolName;
	}
}

void IMAGEPS_InfoWindow::connectMultipleUIToTool(const QString& toolName, const QList<QObject*>& uiElements)
{
	for (QObject* uiElement : uiElements) {
		connectUIToTool(toolName, uiElement);
	}
}

void IMAGEPS_InfoWindow::connectActionToTool(const QString& toolName, QAction* action)
{
	if (action) {
		connect(action, &QAction::triggered, this, [=]() {
			// 验证许可证
			if (!PublicFunctions::validateDogLicense()) {
				return 0;
			}
			//QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
			//	QString::fromLocal8Bit("/Software/") + toolName;
#ifdef Q_OS_LINUX 
			// 构建完整的工具路径 
			QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
				QString::fromLocal8Bit("/linux64/") + toolName;
#else
			// 构建完整的工具路径 
			QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
				QString::fromLocal8Bit("/Software/") + toolName;
#endif
			QProcess* process = new QProcess(this);
			process->start(fullToolPath);
		});
	}
}

void IMAGEPS_InfoWindow::connectButtonToTool(const QString& toolName, QPushButton* button)
{
	if (button) {
		connect(button, &QPushButton::clicked, this, [=]() {
			// 验证许可证
			if (!PublicFunctions::validateDogLicense()) {
				return 0;
			}
			//QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
			//	QString::fromLocal8Bit("/Software/") + toolName;
#ifdef Q_OS_LINUX 
			// 构建完整的工具路径 
			QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
				QString::fromLocal8Bit("/linux64/") + toolName;
#else
			// 构建完整的工具路径 
			QString fullToolPath = IMAGEPS_InfoWindow::instance->appDirPath +
				QString::fromLocal8Bit("/Software/") + toolName;
#endif
			QProcess* process = new QProcess(this);
			process->start(fullToolPath);
		});
	}
}

void IMAGEPS_InfoWindow::connectLabelToTool(const QString& toolName, QLabel* label)
{
	if (label) {
		// 设置为可点击的超链接样式 
		label->setTextFormat(Qt::RichText);
		label->setText(QString("<a href=\"%1\">%2</a>").arg(toolName).arg(label->text()));
		label->setOpenExternalLinks(false);
		label->setCursor(Qt::PointingHandCursor);

		// 直接连接信号 
		connect(label, &QLabel::linkActivated, this, &IMAGEPS_InfoWindow::startToolProcess);
	}
}

// 在关闭主窗口时发射信号
void IMAGEPS_InfoWindow::closeEvent(QCloseEvent* event)
{
	emit aboutToClose_InfoWindow();
	QMainWindow::closeEvent(event);
}

/**
 * @brief 删除列表中的最近打开工程
 */
bool IMAGEPS_InfoWindow::removeLineFromCsv(const QString &filePath,
	const QString &fullProjectPath,
	const QString &projectId)
{
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "无法打开文件:" << filePath;
		return false;
	}

	QTextStream in(&file);
	in.setCodec("UTF-8");
	QStringList linesToKeep;

	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.isEmpty()) continue;

		// 简单检查：如果行中包含完整项目路径，则删除
		bool shouldRemove = line.contains(fullProjectPath);

		if (!shouldRemove) {
			linesToKeep.append(line);
		}
	}
	file.close();

	// 重新写入文件
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		qDebug() << "无法写入文件:" << filePath;
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	for (const QString &line : linesToKeep) {
		out << line << "\n";
	}
	file.close();

	return true;
}