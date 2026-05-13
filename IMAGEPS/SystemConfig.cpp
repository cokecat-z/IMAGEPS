#include "SystemConfig.h"
#include "IMAGEPS.h"

SystemConfig::SystemConfig(IMAGEPS* parentImagePS, QWidget* parent)
	: QDialog(parent)
	, m_imagePS(parentImagePS)
{
	ui.setupUi(this);

	if (!m_imagePS && parent) {
		m_imagePS = qobject_cast<IMAGEPS*>(parent);
	}
	this->setWindowTitle(QString::fromLocal8Bit("生产配置"));
	this->resize(1000, 800);
		// 设置窗口最小大小 
	//this->setMinimumSize(1000, 800);

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	int x = (screenGeometry.width() - this->width()) / 2;
	int y = (screenGeometry.height() - this->height()) / 2;
	this->move(x, y);
	
	loadConfig();

	initWidget();
	connects();

	if (!dataModelConfig)
	{
		dataModelConfig = new DataModelConfig(m_imagePS, m_imagePS);
		dataModelConfig->resize(1180, 500);
		dataModelConfig->setProjectdir();
	}

	if (!SetOrthoInfoConfig)
	{
		SetOrthoInfoConfig = new SetOrthoInfo(m_imagePS, m_imagePS);
		//SetOrthoInfoConfig->resize(1180, 500);
		SetOrthoInfoConfig->setconfigFile();
	}
	// 初始设置 
	setupScrollAreaLabels();

	m_maxConcurrentProcesses = ui.runEnviron_taskBox->currentText().toInt();

	loadSettings();
	setupCustomWorkflowPage(); // 初始化自定义流程页面 
	//ui.imageMatch_connectMatchModeBox->setView(new QListView());
	//ui.imageMatch_connectMatchModeBox->view()->setTextElideMode(Qt::ElideNone); // 禁止省略号
	//ui.imageMatch_connectMatchModeBox->view()->setMinimumWidth(ui.imageMatch_connectMatchModeBox->width() * 2); // 下拉框更宽
}

QSize SystemConfig::sizeHint() const
{
	if (parentWidget()) {
		return parentWidget()->size() * 0.7;
	}
	return QSize(1000, 800); // 默认大小 
}

void SystemConfig::loadConfig(QString funModule)
{
	funModuleTemp.clear();
	funListConifg.clear();

	if(funModule == QString::fromLocal8Bit("系统配置功能列表"))
		funListConifg = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/systemConfig/系统配置功能列表.csv"), ",");
	else
		funModuleTemp = PublicFunctions::loadConfigFile(QString::fromLocal8Bit("../bin/config/systemConfig/%1.csv").arg(funModule));
}

void SystemConfig::initWidget()
{
	for (auto data : funListConifg)
	{
		if (data[1] == "TRUE")
		{
			QListWidgetItem* item = new QListWidgetItem(data[0]);
			item->setTextAlignment(Qt::AlignVCenter);
			ui.funListWidget->addItem(item);
			funModuleBool[data[0]] = false;
		}
	}
	itemSlot(funModuleBool);

	ui.stackedWidget->setCurrentIndex(0);

	// 综合设置（间距+边框+背景）
	ui.funListWidget->setStyleSheet(
		"QListWidget {"
		"   background-color: #f2f2f2;"
		"   border: none;"              // 完全移除控件边框
		"   border-radius: 10px;"      // 设置圆角（即使无边框也需保留，影响背景裁剪）
		"   outline: 0px;"
		"   padding: 5px;"             // 保持内边距
		"}"
		"QListWidget::item {"
		"   margin: 5px;"              // 减少边距使更紧凑
		"   padding: 5px;"
		"   border: none;"             // 移除item边框
		"   border-radius: 4px;"
		"   color: black;"
		"   background: transparent;"  // 透明背景
		"}"
		"QListWidget::item:hover {"
		"   background-color: #00a99d;"
		"}"
		"QListWidget::item:selected {"
		"   background-color: #00a99d;"
		"   color: white;"
		"}"
	);

	ui.lineEdit_5->setReadOnly(true);
	ui.lineEdit_18->setReadOnly(true);
	QString tmp = "PROJCS[\"Transverse Mercator\",GEOGCS[\"China2000\",DATUM[\"China2000\",SPHEROID[\"CGCS2000\",6378137,298.257222101],TOWGS84[0, 0, 0,0,0,0,0]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433]],UNIT[\"meters\",1],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",75],PARAMETER[\"scale_factor\",1],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",0]]";
	ui.lineEdit_5->setText(tmp);

	imageEventColor_eventLightGroups = {
	{ui.lineEdit_15, ui.checkBox_20, ui.checkBox_22, ui.checkBox_23, ui.imageEventColor_useModuleDBCheckBox, ui.checkBox_26,  ui.checkBox_31, ui.lineEdit_26, ui.checkBox_30, ui.checkBox_32, ui.checkBox_33, ui.checkBox_35, ui.checkBox_20, ui.pushButton,ui.checkBox_36},
	{ui.checkBox_20, ui.imageEventColor_useModuleDBCheckBox, ui.checkBox_33, ui.checkBox_35},
	{ui.lineEdit_15,ui.imageEventColor_useModuleDBCheckBox, ui.checkBox_33, ui.checkBox_35},
	{ui.imageEventColor_useModuleDBCheckBox, ui.checkBox_33, ui.checkBox_35},
	{ui.checkBox_35},
	{ui.checkBox_20, ui.checkBox_22, ui.imageEventColor_useModuleDBCheckBox, ui.checkBox_30, ui.checkBox_32, ui.checkBox_33, ui.checkBox_35},
	{ui.checkBox_35},
	{ui.imageEventColor_useModuleDBCheckBox, ui.checkBox_33}
	};
	
	//imageEventColorcheckboxMap[ui.checkBox_22] = { ui.label_22, ui.label_26, ui.lineEdit_16, ui.lineEdit_17 };
	imageEventColorcheckboxMap[ui.checkBox_23] = { ui.lineEdit_19, ui.checkBox_24 };
	//imageEventColorcheckboxMap[ui.imageEventColor_useModuleDBCheckBox] = { ui.imageEventColor_useModuleDBBox, ui.lineEdit_21, ui.pushButton_2 };
	imageEventColorcheckboxMap[ui.checkBox_26] = { ui.label_30, ui.lineEdit_25, ui.pushButton_3 };
	imageEventColorcheckboxMap[ui.checkBox_31] = { ui.label_33, ui.lineEdit_27};
	imageEventColorcheckboxMap[ui.checkBox_33] = { ui.lineEdit_26};
	imageEventColorcheckboxMap[ui.checkBox_32] = { ui.label_35, ui.lineEdit_28};
	imageEventColorcheckboxMap[ui.imageEventColor_improveClarityCheckBox] = { ui.imageEventColor_improveClarityBox};

	ui.lineEdit_47->setEnabled(false);
	ui.label_72->setEnabled(false);

	ui.checkBox_66->setEnabled(ui.radioButton_4->isChecked());
	ui.checkBox_67->setEnabled(ui.checkBox_66->isChecked());
	ui.checkBox_68->setEnabled(ui.checkBox_66->isChecked());

	ui.label_45->setEnabled(ui.checkBox_40->isChecked());
	ui.label_46->setEnabled(ui.checkBox_40->isChecked());
	ui.label_47->setEnabled(ui.checkBox_40->isChecked());
	ui.lineEdit_34->setEnabled(ui.checkBox_40->isChecked());
	ui.lineEdit_35->setEnabled(ui.checkBox_40->isChecked());
	ui.lineEdit_36->setEnabled(ui.checkBox_40->isChecked());

	ui.label_49->setEnabled(ui.checkBox_42->isChecked());
	ui.label_50->setEnabled(ui.checkBox_42->isChecked());
	ui.label_51->setEnabled(ui.checkBox_42->isChecked());
	ui.lineEdit_38->setEnabled(ui.checkBox_42->isChecked());
	ui.lineEdit_39->setEnabled(ui.checkBox_42->isChecked());
	ui.lineEdit_40->setEnabled(ui.checkBox_42->isChecked());

	ui.lineEdit_44->setEnabled(ui.checkBox_47->isChecked());
	ui.lineEdit_41->setEnabled(ui.checkBox_48->isChecked());
	ui.lineEdit_57->setEnabled(ui.checkBox_48->isChecked());

	ui.trueColorConver_colorAdjustWayLabel->setEnabled(ui.checkBox_43->isChecked());
	ui.trueColorConver_colorAdjustWayBox->setEnabled(ui.checkBox_43->isChecked());
	ui.AdjustClearness_check->setEnabled(ui.checkBox_43->isChecked());
	ui.checkBox_45->setEnabled(ui.checkBox_43->isChecked());
	ui.checkBox_46->setEnabled(ui.checkBox_43->isChecked());
	ui.checkBox_48->setEnabled(ui.checkBox_43->isChecked());
	ui.AdjustClearnessType_Box->setEnabled(ui.checkBox_43->isChecked());
	ui.label_62->setEnabled(ui.checkBox_43->isChecked());
	ui.label_83->setEnabled(ui.checkBox_43->isChecked());
	ui.trueColorConver_groundObjectTypeLabel->setEnabled(ui.checkBox_43->isChecked());
	ui.trueColorConver_groundObjectTypeBox->setEnabled(ui.checkBox_43->isChecked());
	ui.checkBox_82->setEnabled(ui.checkBox_43->isChecked());
	ui.lineEdit_42->setEnabled(ui.checkBox_43->isChecked());
	ui.lineEdit_56->setEnabled(ui.checkBox_43->isChecked());
	ui.lineEdit_43->setEnabled(ui.checkBox_43->isChecked());

	// 创建一个QMovie对象并设置GIF文件
	//movie = new QMovie(QString::fromLocal8Bit("D:/giphy.gif"));
	movie_left = new QMovie(QString::fromLocal8Bit(":/resource/menu/流程/left_arrow.gif"));
	movie_right = new QMovie(QString::fromLocal8Bit(":/resource/menu/流程/right_arrow.gif"));
	movie_down = new QMovie(QString::fromLocal8Bit(":/resource/menu/流程/down_arrow.gif"));
	if (!movie_left->isValid() && !movie_right->isValid() && !movie_down->isValid()) {
		qDebug() << "Error: GIF not loaded!";
	}
	movie_right->setScaledSize(ui.label_74->size());
	movie_left->setScaledSize(ui.label_76->size());
	movie_down->setScaledSize(ui.label_82->size());

	ui.label_74->setMovie(movie_right);
	ui.label_74->setScaledContents(true);

	ui.label_75->setMovie(movie_right);
	ui.label_75->setScaledContents(true);

	ui.label_76->setMovie(movie_left);
	ui.label_76->setScaledContents(true);

	ui.label_77->setMovie(movie_left);
	ui.label_77->setScaledContents(true);

	ui.label_78->setMovie(movie_right);
	ui.label_78->setScaledContents(true);

	ui.label_79->setMovie(movie_right);
	ui.label_79->setScaledContents(true);

	ui.label_80->setMovie(movie_down);
	ui.label_80->setScaledContents(true);

	ui.label_81->setMovie(movie_down);
	ui.label_81->setScaledContents(true);

	ui.label_82->setMovie(movie_down);
	ui.label_82->setScaledContents(true);

	// 开始播放动画 
	movie_left->setCacheMode(QMovie::CacheAll);
	movie_right->setCacheMode(QMovie::CacheAll);
	movie_down->setCacheMode(QMovie::CacheAll);
	movie_left->setSpeed(50); 
	movie_right->setSpeed(50);
	movie_down->setSpeed(50); 
	movie_left->start();
	movie_right->start();
	movie_down->start();

	QPixmap icon_1(QString::fromLocal8Bit(":/resource/menu/流程/金字塔创建.png"));  
	QPixmap icon_2(QString::fromLocal8Bit(":/resource/menu/流程/连接点匹配.png"));  
	QPixmap icon_3(QString::fromLocal8Bit(":/resource/menu/流程/控制点匹配.png"));  
	QPixmap icon_4(QString::fromLocal8Bit(":/resource/menu/流程/自由网平差.png"));  
	QPixmap icon_5(QString::fromLocal8Bit(":/resource/menu/流程/控制网平差.png"));  
	QPixmap icon_6(QString::fromLocal8Bit(":/resource/menu/流程/正射纠正.png"));  
	QPixmap icon_7(QString::fromLocal8Bit(":/resource/menu/流程/影像融合.png"));  
	QPixmap icon_8(QString::fromLocal8Bit(":/resource/menu/流程/真彩色转换.png"));
	QPixmap icon_9(QString::fromLocal8Bit(":/resource/menu/流程/影像匀色.png"));  
	QPixmap icon_10(QString::fromLocal8Bit(":/resource/menu/流程/影像镶嵌.png")); 
	
	ui.label_106->setPixmap(icon_1);
	ui.label_106->setScaledContents(true);

	ui.label_108->setPixmap(icon_2);
	ui.label_108->setScaledContents(true);

	ui.label_110->setPixmap(icon_3);
	ui.label_110->setScaledContents(true);

	ui.label_112->setPixmap(icon_6);
	ui.label_112->setScaledContents(true);

	ui.label_114->setPixmap(icon_4);
	ui.label_114->setScaledContents(true);

	ui.label_116->setPixmap(icon_5);
	ui.label_116->setScaledContents(true);

	ui.label_118->setPixmap(icon_7);
	ui.label_118->setScaledContents(true);

	ui.label_120->setPixmap(icon_9);
	ui.label_120->setScaledContents(true);

	ui.label_122->setPixmap(icon_8);
	ui.label_122->setScaledContents(true);

	ui.label_124->setPixmap(icon_10);
	ui.label_124->setScaledContents(true);
}

void SystemConfig::setWidgetBackground(QWidget *widget, const QString &imagePath)
{
	// 获取设备像素比 
	qreal dpr = widget->devicePixelRatio();

	// 加载图片并设置DPI 
	QPixmap pixmap(imagePath);
	pixmap.setDevicePixelRatio(dpr);

	// 创建临时文件保存缩放后图片
	QTemporaryFile tempFile;
	if (tempFile.open()) {
		pixmap.save(&tempFile, "PNG");
		tempFile.close();

		// 设置样式表 
		widget->setStyleSheet(QString(
			"QWidget {"
			"   border-image: url(%1);"
			"   background-position: center;"
			"   background-repeat: no-repeat;"
			"   background-attachment: fixed;"
			"   background-size: %2px %3px;"
			"}"
		).arg(tempFile.fileName())
			.arg(pixmap.width() / dpr)
			.arg(pixmap.height() / dpr));
	}

	widget->setAutoFillBackground(true);
}

void SystemConfig::connects()
{
	connect(ui.okButton, &QPushButton::clicked, this, &SystemConfig::okButtonSlot);
	connect(ui.cancelButton, &QPushButton::clicked, this, &SystemConfig::cancelButtonSlot);
	connect(ui.applyButton, &QPushButton::clicked, this, &SystemConfig::applyButtonSlot);

	connect(ui.funListWidget, &QListWidget::itemClicked, this, &SystemConfig::itemswitchSlot);
	
	connect(ui.normalCorrect_projectSettingToolBut, &QPushButton::clicked, this, [this]() {
		projectSetting.setOpenerButtonName("normalCorrect");  // 标记是第一个按钮打开的 
		projectSetting.setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
		//projectSetting.resize(1180, 580);
		projectSetting.resize(1550, 900);
		projectSetting.openSettingWidgetSlot();
	});

	connect(ui.projectChange_projectSettingToolBut, &QPushButton::clicked, this, [this]() {
		projectSetting.setOpenerButtonName("projectChange");  // 标记是第二个按钮打开的
		projectSetting.setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
		//projectSetting.resize(1180, 580);
		projectSetting.resize(1550, 900);
		projectSetting.openSettingWidgetSlot();
	});

	connect(m_imagePS, &IMAGEPS::aboutToClose, this,
		[=]() {
		m_isAboutToClose = true;
		this->close();
		m_isAboutToClose = false; // 重置状态
	});

	connect(&projectSetting, &ProjectSetting::PROJCStext,
		this, &SystemConfig::setPROJCStextSlot);
	connect(ui.lineEdit_5, &QLineEdit::textChanged, [this](const QString &text) {
		ui.lineEdit_5->setToolTip(text);
	});
	connect(ui.lineEdit_18, &QLineEdit::textChanged, [this](const QString &text) {
		ui.lineEdit_18->setToolTip(text);
	});

	// 连接导出按钮
	connect(ui.pushButton_5, &QPushButton::clicked,
		this, &SystemConfig::onExportTrueColorSettings);

	// 连接导入按钮 
	connect(ui.pushButton_4, &QPushButton::clicked,
		this, &SystemConfig::onImportTrueColorSettings);

	connect(ui.checkBox_85, &QCheckBox::stateChanged, this, [this](int state) {
		bool isEnabled = (state == Qt::Checked);
		if (isEnabled)
		{
			ui.label_113->setEnabled(isEnabled);
			ui.lineEdit_60->setEnabled(isEnabled);
		}
		else
		{
			ui.label_113->setEnabled(isEnabled);
			ui.lineEdit_60->setEnabled(isEnabled);
		}
	});

	connect(ui.imageEventColor_useModuleDBBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		// 显示当前选项对应的控件组 
		if (index == 5) {
			ui.lineEdit_21->setEnabled(true);
			ui.pushButton_2->setEnabled(true);
		}
		else
		{
			ui.lineEdit_21->setEnabled(false);
			ui.pushButton_2->setEnabled(false);
		}

	});

	for (auto it = imageEventColorcheckboxMap.begin(); it != imageEventColorcheckboxMap.end(); ++it) {
		QCheckBox* checkBox = it.key();
		const QList<QWidget*>& widgets = it.value();

		connect(checkBox, &QCheckBox::stateChanged, this, [widgets](int state) {
			bool isEnabled = (state == Qt::Checked);
			for (QWidget* widget : widgets) {
				widget->setEnabled(isEnabled);
			}
		});
	}

	connect(ui.imageEventColor_useModuleDBCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
		bool isEnabled = (state == Qt::Checked);
		if (isEnabled)
		{
			ui.imageEventColor_useModuleDBBox->setEnabled(true);
			ui.lineEdit_21->setEnabled(true);
			ui.pushButton_2->setEnabled(true);
			ui.lineEdit_20->setEnabled(false);
			ui.pushButton->setEnabled(false);
		}
		else
		{
			ui.imageEventColor_useModuleDBBox->setEnabled(false);
			ui.lineEdit_21->setEnabled(false);
			ui.pushButton_2->setEnabled(false);
			ui.lineEdit_20->setEnabled(true);
			ui.pushButton->setEnabled(true);
		}
	});

	connect(ui.checkBox_22, &QCheckBox::stateChanged, this, [this](int state) {
		bool isEnabled = (state == Qt::Checked);
		if (isEnabled)
		{
			ui.label_22->setEnabled(isEnabled);
			ui.label_26->setEnabled(isEnabled);
			ui.lineEdit_16->setEnabled(isEnabled);
			ui.lineEdit_17->setEnabled(isEnabled);
			if (ui.imageEventColor_eventLightColorFunBox->currentIndex() == 0) {
				ui.label_26->setEnabled(false);
				ui.lineEdit_17->setEnabled(false);
			}
		}
		else
		{
			ui.label_22->setEnabled(isEnabled);
			ui.label_26->setEnabled(isEnabled);
			ui.lineEdit_16->setEnabled(isEnabled);
			ui.lineEdit_17->setEnabled(isEnabled);
		}
	});

	connect(ui.checkBox_73, &QCheckBox::stateChanged, this, [this](int state) {
		bool isEnabled = (state == Qt::Checked);
		if (isEnabled)
		{
			ui.lineEdit_53->setEnabled(isEnabled);;
			if (ui.imageEventColor_eventLightColorFunBox->currentIndex() == 0) {
				ui.lineEdit_53->setEnabled(false);
			}
		}
		else
		{
			ui.lineEdit_53->setEnabled(isEnabled);
		}
	});

	connect(ui.checkBox_27, &QCheckBox::stateChanged, this, [this](int state) {
		bool isEnabled = (state == Qt::Checked);
		bool isChecked_26 = ui.checkBox_26->isChecked();
		if (isEnabled)
		{
			ui.label_30->setEnabled(false);
			ui.lineEdit_25->setEnabled(false);
			ui.pushButton_3->setEnabled(false);
		}
		else
		{
			ui.label_30->setEnabled(isChecked_26);
			ui.lineEdit_25->setEnabled(isChecked_26);
			ui.pushButton_3->setEnabled(isChecked_26);
		}
	});

	connect(ui.checkBox_53, &QCheckBox::stateChanged, this, [this](int state) {
		bool isEnabled = (state == Qt::Checked);
		if (isEnabled)
		{
			ui.lineEdit_47->setEnabled(isEnabled);
			ui.label_72->setEnabled(isEnabled);
		}
		else
		{
			ui.lineEdit_47->setEnabled(isEnabled);
			ui.label_72->setEnabled(isEnabled);
		}
	});

	connect(ui.CheckBox_6, &QCheckBox::stateChanged, this, [this](int state) {
		bool isEnabled = (state == Qt::Checked);
		if (isEnabled)
		{
			ui.LineEdit_3->setEnabled(isEnabled);
			ui.Label_13->setEnabled(isEnabled);
		}
		else
		{
			ui.LineEdit_3->setEnabled(isEnabled);
			ui.Label_13->setEnabled(isEnabled);
		}
	});

	// 连接radioButton_4的状态变化信号 
	connect(ui.radioButton_4, &QRadioButton::toggled, this, [this](bool checked) {
		// 设置checkBox_66的可用状态 
		ui.checkBox_66->setEnabled(checked);

		// 如果radioButton未被选中，则禁用所有相关控件 
		if (!checked) {
			ui.checkBox_66->setChecked(false);   // 取消勾选
			ui.checkBox_67->setEnabled(false);
			ui.checkBox_68->setEnabled(false);
		}

		// 原有的其他控件状态设置保持不变 
		ui.LineEdit_3->setEnabled(checked);
		ui.Label_13->setEnabled(checked);
	});

	// 连接checkBox_66的状态变化信号
	connect(ui.checkBox_66, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		// 设置checkBox_67和checkBox_68的可用状态
		ui.checkBox_67->setEnabled(isChecked);
		ui.checkBox_68->setEnabled(isChecked);

		// 如果取消勾选，确保子复选框也被取消勾选（可选）
		if (!isChecked) {
			ui.checkBox_67->setChecked(false);
			ui.checkBox_68->setChecked(false);
		}
	});

	//影像融合
	connect(ui.checkBox_15, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.label_17->setEnabled(isChecked);
		ui.lineEdit_14->setEnabled(isChecked);
	});
	
	connect(ui.AdjustClearness_check, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.AdjustClearnessType_Box->setEnabled(isChecked);
	});

	connect(ui.checkBox_45, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.lineEdit_42->setEnabled(isChecked);
		ui.label_62->setEnabled(isChecked);
	});

	connect(ui.checkBox_46, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.lineEdit_43->setEnabled(isChecked);
	});

	connect(ui.checkBox_47, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.lineEdit_44->setEnabled(isChecked);
	});

	connect(ui.checkBox_82, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.lineEdit_56->setEnabled(isChecked);
	});

	connect(ui.checkBox_48, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.lineEdit->setEnabled(isChecked);
		ui.lineEdit_2->setEnabled(isChecked);
	});

	connect(ui.checkBox_40, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.label_45->setEnabled(isChecked);
		ui.label_46->setEnabled(isChecked);
		ui.label_47->setEnabled(isChecked);
		ui.lineEdit_34->setEnabled(isChecked);
		ui.lineEdit_35->setEnabled(isChecked);
		ui.lineEdit_36->setEnabled(isChecked);
	});

	connect(ui.checkBox_42, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.label_49->setEnabled(isChecked);
		ui.label_50->setEnabled(isChecked);
		ui.label_51->setEnabled(isChecked);
		ui.lineEdit_38->setEnabled(isChecked);
		ui.lineEdit_39->setEnabled(isChecked);
		ui.lineEdit_40->setEnabled(isChecked);
	});

	connect(ui.imageMatch_connectMatchModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, [this](int state) {

		bool masterEnabled = ui.checkBox_11->isChecked();

		ui.checkBox_44->setEnabled(state == 0 || state == 1);
		ui.checkBox_44->setEnabled(state == 0 || state == 1);
	});

	connect(ui.imageMatch_controlMatchModeBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, [this](int state) {
	
		bool masterEnabled = ui.checkBox_11->isChecked();

		ui.checkBox_11->setEnabled(state == 2);
		ui.checkBox_4->setEnabled(state == 2);

		bool groupAEnabled = masterEnabled && (state == 2);

		ui.label_41->setEnabled(groupAEnabled);
		ui.lineEdit_23->setEnabled(groupAEnabled);
		ui.toolButton->setEnabled(groupAEnabled);
	});

	connect(ui.checkBox_11, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.label_41->setEnabled(isChecked);
		ui.lineEdit_23->setEnabled(isChecked);
		ui.toolButton->setEnabled(isChecked);
	});

	//connect(ui.checkBox_43, &QCheckBox::stateChanged, this, [this](int state) {
	//	bool isChecked = (state == Qt::Checked);

	//	ui.trueColorConver_colorAdjustWayLabel->setEnabled(isChecked);
	//	ui.trueColorConver_colorAdjustWayBox->setEnabled(isChecked);
	//	ui.AdjustClearnessType_Box->setEnabled(isChecked);
	//	ui.checkBox_82->setEnabled(isChecked);
	//	ui.checkBox_45->setEnabled(isChecked);
	//	ui.checkBox_46->setEnabled(isChecked);
	//	ui.checkBox_48->setEnabled(isChecked);
	//	ui.AdjustClearness_check->setEnabled(isChecked);
	//	ui.label_62->setEnabled(isChecked);
	//	ui.trueColorConver_groundObjectTypeLabel->setEnabled(isChecked);
	//	ui.trueColorConver_groundObjectTypeBox->setEnabled(isChecked);
	//	ui.lineEdit_56->setEnabled(isChecked);
	//	ui.lineEdit_42->setEnabled(isChecked);
	//	ui.lineEdit_43->setEnabled(isChecked);
	//});
	connect(ui.checkBox_43, &QCheckBox::stateChanged,
		this, &SystemConfig::updateWidgetStates);
	connect(ui.trueColorConver_colorAdjustWayBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &SystemConfig::updateWidgetStates);

	connect(ui.checkBox_59, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.lineEdit_50->setEnabled(isChecked);
	});
	
	connect(ui.checkBox_62, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.label_89->setEnabled(isChecked);
		ui.lineEdit_51->setEnabled(isChecked);
	});

	connect(ui.imageEventColor_eventLightColorFunBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
		for (const auto& group : imageEventColor_eventLightGroups) {
			for (QWidget* widget : group) {
				widget->setEnabled(true);
			}
		}
		// 当前选项对应的控件组 
		if (index >= 0 && index < imageEventColor_eventLightGroups.size()) {
			for (QWidget* widget : imageEventColor_eventLightGroups[index]) {
				widget->setEnabled(false);
			}
		}
	});


	// 监听页面切换
	connect(ui.stackedWidget, &QStackedWidget::currentChanged, [this](int index) {
		if (ui.stackedWidget->widget(index) == ui.automationProcess1_Page) {
			setupScrollAreaLabels(); // 每次切换到该页面时重新初始化
		}
	});

	connect(this, &SystemConfig::FunctionAbnormalExit, this,
		[=](QString title) {
		
		if (!m_imagePS->getWorkProcessflag()) {
			logEdit->append(QString::fromLocal8Bit("****") + title);
		}
	});
}

void SystemConfig::updateWidgetStates()
{
	bool masterEnabled = ui.checkBox_43->isChecked();
	int mode = ui.trueColorConver_colorAdjustWayBox->currentIndex();

	// 组A控件（模式0时启用）
	bool groupAEnabled = masterEnabled && (mode == 0);


	// 组B控件（模式1时启用）
	bool groupBEnabled = masterEnabled && (mode == 1 || mode == 3);
	ui.pushButton_4->setEnabled(groupBEnabled);
	ui.pushButton_5->setEnabled(groupBEnabled);
	ui.checkBox_28->setEnabled(groupBEnabled);
	ui.lineEdit_41->setEnabled(groupBEnabled && ui.checkBox_43->isChecked());
	ui.lineEdit_57->setEnabled(groupBEnabled && ui.checkBox_43->isChecked());

	// 组C控件（模式2时启用）
	bool groupCEnabled = masterEnabled && (mode == 2);
	ui.trueColorConver_groundObjectTypeBox->setEnabled(groupCEnabled);
	ui.trueColorConver_groundObjectTypeLabel->setEnabled(groupCEnabled);
	ui.checkBox_46->setEnabled(groupCEnabled);
	ui.lineEdit_43->setEnabled(groupCEnabled && ui.checkBox_46->isChecked());


	// 不受模式影响的公共控件
	ui.trueColorConver_colorAdjustWayLabel->setEnabled(masterEnabled);
	ui.trueColorConver_colorAdjustWayBox->setEnabled(masterEnabled);
	ui.AdjustClearnessType_Box->setEnabled(masterEnabled);
	ui.checkBox_82->setEnabled(masterEnabled);
	ui.checkBox_45->setEnabled(masterEnabled);
	ui.checkBox_48->setEnabled(masterEnabled);
	ui.AdjustClearness_check->setEnabled(masterEnabled);
	ui.label_62->setEnabled(masterEnabled && ui.checkBox_45->isChecked());
	ui.lineEdit_56->setEnabled(masterEnabled && ui.checkBox_82->isChecked());
	ui.lineEdit_42->setEnabled(masterEnabled && ui.checkBox_45->isChecked());
}

void SystemConfig::setProjectdir() {
	if (m_imagePS) {
		projectdir = m_imagePS->projectdir;
	}
}

void SystemConfig::itemSlot(QMap<QString, bool>& itemfunModuleBool)
{
	for (const auto &pair : itemfunModuleBool.toStdMap()) {

		if (pair.first == QString::fromLocal8Bit("运行环境"))
		{
			ui.stackedWidget->setCurrentWidget(ui.runEnviron_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.runEnviron_runLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.runEnviron_runBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.runEnviron_taskLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.runEnviron_taskBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("影像匹配"))
		{
			ui.stackedWidget->setCurrentWidget(ui.imageMatch_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[QString::fromLocal8Bit("连接点匹配") + ui.imageMatch_connectMatchModeLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageMatch_connectMatchModeBox->addItem(childIter.at(0));
			}
			ui.imageMatch_connectMatchModeBox->setCurrentIndex(0);
			for (auto childIter : funModuleTemp[QString::fromLocal8Bit("控制点匹配") + ui.imageMatch_controlMatchModeLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageMatch_controlMatchModeBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.imageMatch_waitMatchDataBandLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageMatch_waitMatchDataBandBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.imageMatch_referenceDataBandLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageMatch_referenceDataBandBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.imageMatch_controlMatchModelLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageMatch_controlMatchModelBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("正射纠正"))
		{
			ui.stackedWidget->setCurrentWidget(ui.normalCorrect_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.normalCorrect_samplingWayLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.normalCorrect_samplingWayBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.normalCorrect_resultFormatLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.normalCorrect_resultFormatBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.label_7->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_12->setText(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.label_9->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_13->setText(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("影像融合"))
		{
			ui.stackedWidget->setCurrentWidget(ui.imageFusion_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.imageFusion_outImageFormatLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageFusion_outImageFormatBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.imageFusion_fusionWayLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageFusion_fusionWayBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("影像匀色"))
		{
			ui.stackedWidget->setCurrentWidget(ui.imageEventColor_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.imageEventColor_eventLightColorFunLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageEventColor_eventLightColorFunBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.imageEventColor_useModuleDBCheckBox->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageEventColor_useModuleDBBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.imageEventColor_improveClarityCheckBox->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageEventColor_improveClarityBox->addItem(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.label_16->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_15->setText(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.label_22->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_16->setText(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.label_26->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_17->setText(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.checkBox_27->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_24->setText(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.label_33->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_27->setText(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.label_31->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_26->setText(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.label_35->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_28->setText(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("影像镶嵌"))
		{
			ui.stackedWidget->setCurrentWidget(ui.imageInlay_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.imageInlay_resultFormatLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.imageInlay_resultFormatBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("智能镶嵌"))
		{
			ui.stackedWidget->setCurrentWidget(ui.IntelligentMosaic_page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.SamplingMode_label->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.SamplingMode_Box->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.resulttype_label->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.resulttype_Box->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.Label_9->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.ComboBox_4->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("快速镶嵌"))
		{
			ui.stackedWidget->setCurrentWidget(ui.QuickMosaic_page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.QuickMosaic_resultFormatLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.QuickMosaic_resultFormatBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("影像裁切"))
		{
			ui.stackedWidget->setCurrentWidget(ui.ImageCropping_page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.label_88->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.comboBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("配准纠正"))
		{
			ui.stackedWidget->setCurrentWidget(ui.matchCorrect_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.matchCorrect_changeModelLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.matchCorrect_changeModelBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("投影转换"))
		{
			ui.stackedWidget->setCurrentWidget(ui.projectChange_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.projectChange_samplingWayLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.projectChange_samplingWayBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.projectChange_resultFormatLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.projectChange_resultFormatBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("格式转换"))
		{
			ui.stackedWidget->setCurrentWidget(ui.FormatConversion_page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.label_93->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.resultType_Box->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.label_94->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.Bitdepth_Box->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("金字塔创建"))
		{
			ui.stackedWidget->setCurrentWidget(ui.pyramidCreate_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.pyramidCreate_samplingWayLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.pyramidCreate_samplingWayBox->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("卫片平差"))
		{
			ui.stackedWidget->setCurrentWidget(ui.sateImageAdjust_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.sateImageAdjust_adjustModelLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.sateImageAdjust_adjustModelBox->addItem(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.label_59->text()])
			{
				if (childIter.at(1) == "TRUE")
				{
					ui.lineEdit_45->setText(childIter.at(0));
					ui.lineEdit_46->setText(childIter.at(0));
				}
			}
			for (auto childIter : funModuleTemp[ui.label_72->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.lineEdit_47->setText(childIter.at(0));
			}

		}
		else if (pair.first == QString::fromLocal8Bit("真彩色转换"))
		{
			ui.stackedWidget->setCurrentWidget(ui.trueColorConver_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.trueColorConver_outImageFormatLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.trueColorConver_outImageFormatBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.trueColorConver_colorAdjustWayLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.trueColorConver_colorAdjustWayBox->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.trueColorConver_groundObjectTypeLabel->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.trueColorConver_groundObjectTypeBox->addItem(childIter.at(0));
			}
			for (auto childIter : funModuleTemp[ui.AdjustClearness_check->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.AdjustClearnessType_Box->addItem(childIter.at(0));
			}
		}
		else if (pair.first == QString::fromLocal8Bit("SAR影像滤波"))
		{
			ui.stackedWidget->setCurrentWidget(ui.trueColorConver_Page);

			if (!funModuleBool[pair.first])
				funModuleBool[pair.first] = true;
			else
				return;

			loadConfig(pair.first);

			for (auto childIter : funModuleTemp[ui.ImageType_label->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.ImageType_Box->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.Label_3->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.FilteringMethod_Box->addItem(childIter.at(0));
			}

			for (auto childIter : funModuleTemp[ui.Label_20->text()])
			{
				if (childIter.at(1) == "TRUE")
					ui.ComboBox_5->addItem(childIter.at(0));
			}

		}
	}
}

void SystemConfig::itemswitchSlot(QListWidgetItem* item)
{
	if (item->text() == QString::fromLocal8Bit("运行环境"))
	{
		ui.stackedWidget->setCurrentWidget(ui.runEnviron_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("影像云检"))
	{
		ui.stackedWidget->setCurrentWidget(ui.CloudDetection_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("影像匹配"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageMatch_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("正射纠正"))
	{
		ui.stackedWidget->setCurrentWidget(ui.normalCorrect_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("影像融合"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageFusion_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("影像匀色"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageEventColor_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("影像镶嵌"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageInlay_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("智能镶嵌"))
	{
		ui.stackedWidget->setCurrentWidget(ui.IntelligentMosaic_page);
	}
	else if (item->text() == QString::fromLocal8Bit("快速镶嵌"))
	{
		ui.stackedWidget->setCurrentWidget(ui.QuickMosaic_page);
	}
	else if (item->text() == QString::fromLocal8Bit("影像裁切"))
	{
		ui.stackedWidget->setCurrentWidget(ui.ImageCropping_page);
	}
	else if (item->text() == QString::fromLocal8Bit("配准纠正"))
	{
		ui.stackedWidget->setCurrentWidget(ui.matchCorrect_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("投影转换"))
	{
		ui.stackedWidget->setCurrentWidget(ui.projectChange_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("格式转换"))
	{
		ui.stackedWidget->setCurrentWidget(ui.FormatConversion_page);
	}
	else if (item->text() == QString::fromLocal8Bit("金字塔创建"))
	{
		ui.stackedWidget->setCurrentWidget(ui.pyramidCreate_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("卫片平差"))
	{
		ui.stackedWidget->setCurrentWidget(ui.sateImageAdjust_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("真彩色转换"))
	{
		ui.stackedWidget->setCurrentWidget(ui.trueColorConver_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("影像质检"))
	{
		ui.stackedWidget->setCurrentWidget(ui.ImageQualityInspection_page);
	}
	else if (item->text() == QString::fromLocal8Bit("SAR影像滤波"))
	{
		ui.stackedWidget->setCurrentWidget(ui.SARImageFiltering_page);
	}
	else if (item->text() == QString::fromLocal8Bit("生产配置"))
	{
		ui.stackedWidget->setCurrentWidget(ui.automationProcess1_Page);
	}
	else if (item->text() == QString::fromLocal8Bit("自定义流程"))
	{
		ui.stackedWidget->setCurrentWidget(ui.CustomWorkflow_page);
	}

}

void SystemConfig::settingShowWidget(QString title)
{
	this->show();
	// 查找匹配文本的项
	QList<QListWidgetItem*> items = ui.funListWidget->findItems(title, Qt::MatchExactly);

	if (!items.isEmpty()) {
		ui.funListWidget->setCurrentItem(items.first());
	}
	if (title == QString::fromLocal8Bit("运行环境"))
	{
		ui.stackedWidget->setCurrentWidget(ui.runEnviron_Page);
	}
	else if (title == QString::fromLocal8Bit("影像云检"))
	{
		ui.stackedWidget->setCurrentWidget(ui.CloudDetection_Page);
	}
	else if (title == QString::fromLocal8Bit("影像匹配"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageMatch_Page);
	}
	else if (title == QString::fromLocal8Bit("正射纠正"))
	{
		ui.stackedWidget->setCurrentWidget(ui.normalCorrect_Page);
	}
	else if (title == QString::fromLocal8Bit("影像融合"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageFusion_Page);
	}
	else if (title == QString::fromLocal8Bit("影像匀色"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageEventColor_Page);
	}
	else if (title == QString::fromLocal8Bit("影像镶嵌"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageInlay_Page);
	}
	else if (title == QString::fromLocal8Bit("智能镶嵌"))
	{
		ui.stackedWidget->setCurrentWidget(ui.IntelligentMosaic_page);
	}
	else if (title == QString::fromLocal8Bit("快速镶嵌"))
	{
		ui.stackedWidget->setCurrentWidget(ui.QuickMosaic_page);
	}
	else if (title == QString::fromLocal8Bit("影像裁切"))
	{
		ui.stackedWidget->setCurrentWidget(ui.ImageCropping_page);
	}
	else if (title == QString::fromLocal8Bit("影像纠正"))
	{
		ui.stackedWidget->setCurrentWidget(ui.normalCorrect_Page);

		// 查找匹配文本的项
		QList<QListWidgetItem*> items = ui.funListWidget->findItems(QString::fromLocal8Bit("正射纠正"), Qt::MatchExactly);

		if (!items.isEmpty()) {
			ui.funListWidget->setCurrentItem(items.first());
		}
	}
	else if (title == QString::fromLocal8Bit("纠正模型配准"))
	{
		ui.stackedWidget->setCurrentWidget(ui.matchCorrect_Page);

		// 查找匹配文本的项
		QList<QListWidgetItem*> items = ui.funListWidget->findItems(QString::fromLocal8Bit("配准纠正"), Qt::MatchExactly);

		if (!items.isEmpty()) {
			ui.funListWidget->setCurrentItem(items.first());
		}
	}
	else if (title == QString::fromLocal8Bit("投影转换"))
	{
		ui.stackedWidget->setCurrentWidget(ui.projectChange_Page);
	}
	else if (title == QString::fromLocal8Bit("格式转换"))
	{
		ui.stackedWidget->setCurrentWidget(ui.FormatConversion_page);
	}
	else if (title == QString::fromLocal8Bit("金字塔创建"))
	{
		ui.stackedWidget->setCurrentWidget(ui.pyramidCreate_Page);
	}
	else if (title == QString::fromLocal8Bit("卫片平差"))
	{
		ui.stackedWidget->setCurrentWidget(ui.sateImageAdjust_Page);
	}
	else if (title == QString::fromLocal8Bit("真彩色转换"))
	{
		ui.stackedWidget->setCurrentWidget(ui.trueColorConver_Page);
	}
	else if (title == QString::fromLocal8Bit("影像质检"))
	{
		ui.stackedWidget->setCurrentWidget(ui.ImageQualityInspection_page);
	}
	else if (title == QString::fromLocal8Bit("SAR影像滤波"))
	{
		ui.stackedWidget->setCurrentWidget(ui.SARImageFiltering_page);
	}
	else if (title == QString::fromLocal8Bit("生产配置"))
	{
		ui.stackedWidget->setCurrentWidget(ui.automationProcess1_Page);
	}
	else if (title == QString::fromLocal8Bit("自定义流程"))
	{
		ui.stackedWidget->setCurrentWidget(ui.CustomWorkflow_page);
	}

	/*if (title == QString::fromLocal8Bit("金字塔创建"))
	{
		ui.stackedWidget->setCurrentWidget(ui.pyramidCreate_Page);

		if (!funModuleBool[title])
			funModuleBool[title] = true;
		else
			return;

		loadConfig(title);

		for (auto childIter : funModuleTemp[ui.pyramidCreate_samplingWayLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.pyramidCreate_samplingWayBox->addItem(childIter.at(0));
		}
	}
	else if (title == QString::fromLocal8Bit("DOM连接点匹配"))
	{
		ui.stackedWidget->setCurrentWidget(ui.imageMatch_Page);

		if (!funModuleBool[QString::fromLocal8Bit("影像匹配")])
			funModuleBool[QString::fromLocal8Bit("影像匹配")] = true;
		else
			return;

		loadConfig(QString::fromLocal8Bit("影像匹配"));

		for (auto childIter : funModuleTemp[QString::fromLocal8Bit("连接点匹配") + ui.imageMatch_connectMatchModeLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.imageMatch_connectMatchModeBox->addItem(childIter.at(0));
		}

		for (auto childIter : funModuleTemp[QString::fromLocal8Bit("控制点匹配") + ui.imageMatch_controlMatchModeLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.imageMatch_controlMatchModeBox->addItem(childIter.at(0));
		}

		for (auto childIter : funModuleTemp[ui.imageMatch_waitMatchDataBandLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.imageMatch_waitMatchDataBandBox->addItem(childIter.at(0));
		}

		for (auto childIter : funModuleTemp[ui.imageMatch_referenceDataBandLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.imageMatch_referenceDataBandBox->addItem(childIter.at(0));
		}

		for (auto childIter : funModuleTemp[ui.imageMatch_controlMatchModelLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.imageMatch_controlMatchModelBox->addItem(childIter.at(0));
		}
	}*/
	
}

//////////////////////////////
/**
 * @brief 系统配置保存及加载
 */
void SystemConfig::saveSettings()
{
	QSettings settings("../bin/config/systemConfig/systemConfig.ini", QSettings::IniFormat);

	// 保存运行环境页面设置 
	settings.beginGroup("RunEnvironment");
	settings.setValue("runMode", ui.runEnviron_runBox->currentIndex());
	settings.setValue("taskMode", ui.runEnviron_taskBox->currentIndex());
	settings.setValue("showCheckBox", ui.runEnviron_showCheckBox->isChecked());
	settings.setValue("highSpeedCheckBox", ui.runEnviron_highSpeedCheckBox->isChecked());
	settings.endGroup();

	// 云检测
	settings.beginGroup(" CloudDetection");
	settings.setValue("checkBox64", ui.checkBox_64->isChecked());
	settings.setValue("checkBox65", ui.checkBox_65->isChecked());
	settings.setValue("lineEdit52", ui.lineEdit_52->text());
	settings.endGroup();

	// 保存影像匹配页面设置 
	settings.beginGroup("ImageMatch");
	settings.setValue("connectMatchMode", ui.imageMatch_connectMatchModeBox->currentIndex());
	settings.setValue("controlMatchMode", ui.imageMatch_controlMatchModeBox->currentIndex());
	settings.setValue("waitMatchDataBand", ui.imageMatch_waitMatchDataBandBox->currentIndex());
	settings.setValue("referenceDataBand", ui.imageMatch_referenceDataBandBox->currentIndex());
	settings.setValue("controlMatchModel", ui.imageMatch_controlMatchModelBox->currentIndex());
	settings.setValue("checkBox_c", ui.checkBox->isChecked());
	settings.setValue("checkBox2", ui.checkBox_2->isChecked());
	settings.setValue("checkBox3", ui.checkBox_3->isChecked());
	settings.setValue("checkBox4", ui.checkBox_4->isChecked());
	settings.setValue("checkBox5", ui.checkBox_5->isChecked());
	settings.setValue("checkBox11", ui.checkBox_11->isChecked());
	settings.setValue("checkBox72", ui.checkBox_72->isChecked());
	settings.setValue("checkBox74", ui.checkBox_74->isChecked());
	settings.setValue("checkBox44", ui.checkBox_44->isChecked());
	settings.setValue("lineEdit", ui.lineEdit->text());
	settings.setValue("lineEdit2", ui.lineEdit_2->text());
	settings.setValue("lineEdit3", ui.lineEdit_3->text());
	settings.setValue("lineEdit4", ui.lineEdit_4->text());
	settings.setValue("lineEdit6", ui.lineEdit_6->text());
	settings.setValue("lineEdit7", ui.lineEdit_7->text());
	settings.setValue("lineEdit8", ui.lineEdit_8->text());
	settings.setValue("lineEdit9", ui.lineEdit_9->text());
	settings.setValue("lineEdit10", ui.lineEdit_10->text());
	settings.setValue("lineEdit11", ui.lineEdit_11->text());
	settings.setValue("lineEdit22", ui.lineEdit_22->text());
	settings.setValue("lineEdit23", ui.lineEdit_23->text());
	settings.setValue("lineEdit48", ui.lineEdit_48->text());
	settings.endGroup();

	// 保存卫星图像调整页面设置 
	settings.beginGroup("SatelliteImageAdjust");
	settings.setValue("adjustModel", ui.sateImageAdjust_adjustModelBox->currentIndex());
	settings.setValue("checkBox49", ui.checkBox_49->isChecked());
	settings.setValue("checkBox50", ui.checkBox_50->isChecked());
	settings.setValue("checkBox51", ui.checkBox_51->isChecked());
	settings.setValue("checkBox52", ui.checkBox_52->isChecked());
	settings.setValue("checkBox53", ui.checkBox_53->isChecked());
	settings.setValue("checkBox54", ui.checkBox_54->isChecked());
	settings.setValue("checkBox55", ui.checkBox_55->isChecked());
	settings.setValue("checkBox83", ui.checkBox_83->isChecked());
	settings.setValue("checkBox84", ui.checkBox_84->isChecked());
	settings.setValue("lineEdit45", ui.lineEdit_45->text());
	settings.setValue("lineEdit46", ui.lineEdit_46->text());
	settings.setValue("lineEdit47", ui.lineEdit_47->text());
	settings.setValue("lineEdit58", ui.lineEdit_58->text());
	settings.setValue("lineEdit59", ui.lineEdit_59->text());
	settings.endGroup();

	// 保存纠正参数页面设置 
	settings.beginGroup("NormalCorrect");
	settings.setValue("resultFormat", ui.normalCorrect_resultFormatBox->currentIndex());
	settings.setValue("samplingWay", ui.normalCorrect_samplingWayBox->currentIndex());
	settings.setValue("lineEdit5", ui.lineEdit_5->text());
	settings.setValue("lineEdit12", ui.lineEdit_12->text());
	settings.setValue("lineEdit13", ui.lineEdit_13->text());
	settings.setValue("lineEdit60", ui.lineEdit_60->text());
	settings.setValue("checkBox6", ui.checkBox_6->isChecked());
	settings.setValue("checkBox7", ui.checkBox_7->isChecked());
	settings.setValue("checkBox8", ui.checkBox_8->isChecked());
	settings.setValue("checkBox9", ui.checkBox_9->isChecked());
	settings.setValue("checkBox10", ui.checkBox_10->isChecked());
	settings.setValue("checkBox12", ui.checkBox_12->isChecked());
	settings.setValue("checkBox85", ui.checkBox_85->isChecked());
	settings.endGroup();

	// 保存影像融合页面设置 
	settings.beginGroup("ImageFusion");
	settings.setValue("fusionWay", ui.imageFusion_fusionWayBox->currentIndex());
	settings.setValue("outImageFormat", ui.imageFusion_outImageFormatBox->currentIndex());
	settings.setValue("lineEdit14", ui.lineEdit_14->text());
	settings.setValue("checkBox14", ui.checkBox_14->isChecked());
	settings.setValue("checkBox15", ui.checkBox_15->isChecked());
	settings.setValue("checkBox16", ui.checkBox_16->isChecked());
	settings.setValue("checkBox17", ui.checkBox_17->isChecked());
	settings.setValue("checkBox18", ui.checkBox_18->isChecked());
	settings.setValue("checkBox19", ui.checkBox_19->isChecked());
	settings.endGroup();

	// 保存影像匀色页面设置 
	settings.beginGroup("ImageEventColor");
	settings.setValue("moduleDB", ui.imageEventColor_useModuleDBBox->currentIndex());
	settings.setValue("improveClarityLevel", ui.imageEventColor_improveClarityBox->currentIndex());
	settings.setValue("eventLightColorFun", ui.imageEventColor_eventLightColorFunBox->currentIndex());
	settings.setValue("lineEdit15", ui.lineEdit_15->text());
	settings.setValue("lineEdit16", ui.lineEdit_16->text());
	settings.setValue("lineEdit17", ui.lineEdit_17->text());
	settings.setValue("lineEdit19", ui.lineEdit_19->text());
	settings.setValue("lineEdit20", ui.lineEdit_20->text());
	settings.setValue("lineEdit21", ui.lineEdit_21->text());
	settings.setValue("lineEdit24", ui.lineEdit_24->text());
	settings.setValue("lineEdit25", ui.lineEdit_25->text());
	settings.setValue("lineEdit26", ui.lineEdit_26->text());
	settings.setValue("lineEdit27", ui.lineEdit_27->text());
	settings.setValue("lineEdit28", ui.lineEdit_28->text());
	settings.setValue("useModuleDB", ui.imageEventColor_useModuleDBCheckBox->isChecked());
	settings.setValue("improveClarity", ui.imageEventColor_improveClarityCheckBox->isChecked());
	settings.setValue("output8bImage_checkBox", ui.output8bImage_checkBox->isChecked());
	settings.setValue("checkBox20", ui.checkBox_20->isChecked());
	settings.setValue("checkBox21", ui.checkBox_21->isChecked());
	settings.setValue("checkBox22", ui.checkBox_22->isChecked());
	settings.setValue("checkBox23", ui.checkBox_23->isChecked());
	settings.setValue("checkBox24", ui.checkBox_24->isChecked());
	settings.setValue("checkBox26", ui.checkBox_26->isChecked());
	settings.setValue("checkBox27", ui.checkBox_27->isChecked());
	settings.setValue("checkBox30", ui.checkBox_30->isChecked());
	settings.setValue("checkBox31", ui.checkBox_31->isChecked());
	settings.setValue("checkBox32", ui.checkBox_32->isChecked());
	settings.setValue("checkBox33", ui.checkBox_33->isChecked());
	settings.setValue("checkBox35", ui.checkBox_35->isChecked());
	settings.setValue("checkBox36", ui.checkBox_36->isChecked());
	settings.endGroup();

	// 保存影像裁切页面设置 
	settings.beginGroup("ImageCrop");
	settings.setValue("comboBox", ui.comboBox->currentIndex());
	settings.setValue("lineEdit50", ui.lineEdit_50->text());
	settings.setValue("lineEdit51", ui.lineEdit_51->text());;
	settings.setValue("checkBox58", ui.checkBox_58->isChecked());
	settings.setValue("checkBox59", ui.checkBox_59->isChecked());
	settings.setValue("checkBox60", ui.checkBox_60->isChecked());
	settings.setValue("checkBox61", ui.checkBox_61->isChecked());
	settings.setValue("checkBox62", ui.checkBox_62->isChecked());
	settings.setValue("checkBox63", ui.checkBox_63->isChecked());
	settings.endGroup();

	// 保存影像镶嵌页面设置 
	settings.beginGroup("ImageInlay");
	settings.setValue("resultFormat", ui.imageInlay_resultFormatBox->currentIndex());
	settings.setValue("checkBox25", ui.checkBox_25->isChecked());
	settings.endGroup();

	// 保存智能镶嵌页面设置 
	settings.beginGroup("SmartMosaic");
	settings.setValue("SamplingMode", ui.SamplingMode_Box->currentIndex());
	settings.setValue("resulttype", ui.resulttype_Box->currentIndex());
	settings.setValue("ComboBox4", ui.ComboBox_4->currentIndex());
	settings.setValue("LineEdit", ui.LineEdit->text());
	settings.setValue("LineEdit2", ui.LineEdit_2->text());
	settings.setValue("LineEdit3", ui.LineEdit_3->text());
	settings.setValue("LineEdit4", ui.LineEdit_4->text());
	settings.setValue("lineEdit37", ui.lineEdit_37->text());
	settings.setValue("CheckBox", ui.CheckBox->isChecked());
	settings.setValue("CheckBox3", ui.CheckBox_3->isChecked());
	settings.setValue("CheckBox4", ui.CheckBox_4->isChecked());
	settings.setValue("CheckBox5", ui.CheckBox_5->isChecked());
	settings.setValue("CheckBox6", ui.CheckBox_6->isChecked());
	settings.setValue("CheckBox7", ui.CheckBox_7->isChecked());
	settings.endGroup();

	// 保存快速镶嵌页面设置 
	settings.beginGroup("QuickMosaic");
	settings.setValue("QresultFormatBox", ui.QuickMosaic_resultFormatBox->currentIndex());
	settings.setValue("checkBox71", ui.checkBox_71->isChecked());
	settings.endGroup();

	// 保存配准纠正页面设置 
	settings.beginGroup("MatchCorrect");
	settings.setValue("changeModel", ui.matchCorrect_changeModelBox->currentIndex());
	settings.setValue("checkBox29", ui.checkBox_29->isChecked());
	settings.setValue("checkBox34", ui.checkBox_34->isChecked());
	settings.setValue("lineEdit30", ui.lineEdit_30->text());
	settings.endGroup();

	// 保存投影变换页面设置 
	settings.beginGroup("ProjectChange");
	settings.setValue("samplingWay", ui.projectChange_samplingWayBox->currentIndex());
	settings.setValue("checkBox37", ui.checkBox_37->isChecked());
	settings.setValue("lineEdit18", ui.lineEdit_18->text());
	settings.setValue("resultFormat", ui.projectChange_resultFormatBox->currentIndex());
	settings.setValue("lineEdit29", ui.lineEdit_29->text());
	settings.setValue("lineEdit31", ui.lineEdit_31->text());
	settings.endGroup();

	// 保存格式转换页面设置
	settings.beginGroup("FormatConversion");
	settings.setValue("dataTypeImage", ui.radioButton_3->isChecked());
	settings.setValue("dataTypeDEM", ui.radioButton_4->isChecked());
	settings.setValue("resultType", ui.resultType_Box->currentIndex());
	settings.setValue("bitDepthAdjust", ui.checkBox_70->isChecked());
	settings.setValue("bitDepth", ui.Bitdepth_Box->currentIndex());
	settings.setValue("heightCorrection", ui.checkBox_66->isChecked());
	settings.setValue("ellipsoidToLevel", ui.checkBox_67->isChecked());
	settings.setValue("levelToEllipsoid", ui.checkBox_68->isChecked());
	settings.setValue("createPyramid", ui.checkBox_69->isChecked());
	settings.endGroup();

	// 保存SAR影像滤波参数页面设置 
	settings.beginGroup("SARImageFiltering");
	settings.setValue("ImageType_Box", ui.ImageType_Box->currentIndex());
	settings.setValue("FilteringMethod_Box", ui.FilteringMethod_Box->currentIndex());
	settings.setValue("LineEdit_6", ui.LineEdit_6->text());
	settings.setValue("LineEdit_5", ui.LineEdit_5->text());
	settings.setValue("LineEdit_7", ui.LineEdit_7->text());
	settings.setValue("LineEdit_8", ui.LineEdit_8->text());
	settings.setValue("ComboBox_5", ui.ComboBox_5->currentIndex());
	settings.endGroup();

	// 保存金字塔创建页面设置 
	settings.beginGroup("PyramidCreate");
	settings.setValue("samplingWay", ui.pyramidCreate_samplingWayBox->currentIndex());
	settings.endGroup();

	// 保存真彩色转换页面设置 
	settings.beginGroup("TrueColorConvert");
	settings.setValue("outImageFormat", ui.trueColorConver_outImageFormatBox->currentIndex());
	settings.setValue("colorAdjustWay", ui.trueColorConver_colorAdjustWayBox->currentIndex());
	settings.setValue("groundObjectType", ui.trueColorConver_groundObjectTypeBox->currentIndex());
	settings.setValue("AdjustClearnessType", ui.AdjustClearnessType_Box->currentIndex());
	settings.setValue("lineEdit32", ui.lineEdit_32->text());
	settings.setValue("lineEdit33", ui.lineEdit_33->text());
	settings.setValue("lineEdit34", ui.lineEdit_34->text());
	settings.setValue("lineEdit35", ui.lineEdit_35->text());
	settings.setValue("lineEdit36", ui.lineEdit_36->text());
	settings.setValue("lineEdit38", ui.lineEdit_38->text());
	settings.setValue("lineEdit39", ui.lineEdit_39->text());
	settings.setValue("lineEdit40", ui.lineEdit_40->text());
	settings.setValue("lineEdit41", ui.lineEdit_41->text());
	settings.setValue("lineEdit56", ui.lineEdit_56->text());
	settings.setValue("lineEdit42", ui.lineEdit_42->text());
	settings.setValue("lineEdit43", ui.lineEdit_43->text());
	settings.setValue("lineEdit44", ui.lineEdit_44->text());
	settings.setValue("lineEdit57", ui.lineEdit_57->text());
	settings.setValue("checkBox39", ui.checkBox_39->isChecked());
	settings.setValue("checkBox38", ui.checkBox_38->isChecked());
	settings.setValue("checkBox41", ui.checkBox_41->isChecked());
	settings.setValue("checkBox40", ui.checkBox_40->isChecked());
	settings.setValue("checkBox42", ui.checkBox_42->isChecked());
	settings.setValue("checkBox43", ui.checkBox_43->isChecked());
	settings.setValue("AdjustClearnesscheck", ui.AdjustClearness_check->isChecked());
	settings.setValue("checkBox45", ui.checkBox_45->isChecked());
	settings.setValue("checkBox46", ui.checkBox_46->isChecked());
	settings.setValue("checkBox47", ui.checkBox_47->isChecked());
	settings.setValue("checkBox48", ui.checkBox_48->isChecked());
	settings.setValue("checkBox82", ui.checkBox_82->isChecked());
	settings.setValue("checkBox28", ui.checkBox_28->isChecked());
	settings.endGroup();

	////自动化流程一
	//settings.beginGroup("LabelStates");
	//// 保存启用/禁用状态 
	//for (int i = 74; i <= 83; ++i) {
	//	QLabel* label = findChild<QLabel*>(QString("label_%1").arg(i));
	//	if (label) {
	//		settings.setValue(QString("label%1_enabled").arg(i), label->isEnabled());
	//		settings.setValue(QString("label%1_text").arg(i), label->text());
	//		settings.setValue(QString("label%1_stylesheet").arg(i), label->styleSheet());
	//	}
	//}
	//settings.endGroup();
}

void SystemConfig::loadSettings()
{
	QSettings settings("../bin/config/systemConfig/systemConfig.ini", QSettings::IniFormat);

	// 加载运行环境页面设置 
	settings.beginGroup("RunEnvironment");
	ui.runEnviron_runBox->setCurrentIndex(settings.value("runMode", 0).toInt());
	ui.runEnviron_taskBox->setCurrentIndex(settings.value("taskMode", 0).toInt());
	ui.runEnviron_showCheckBox->setChecked(settings.value("showCheckBox", false).toBool());
	ui.runEnviron_highSpeedCheckBox->setChecked(settings.value("highSpeedCheckBox", false).toBool());
	settings.endGroup();

	// 云检测
	settings.beginGroup(" CloudDetection");
	ui.checkBox_64->setChecked(settings.value("checkBox64", false).toBool());
	ui.checkBox_65->setChecked(settings.value("checkBox65", false).toBool());
	ui.lineEdit_52->setText(settings.value("lineEdit52", "").toString());
	settings.endGroup();

	// 加载影像匹配页面设置 
	settings.beginGroup("ImageMatch");
	ui.imageMatch_connectMatchModeBox->setCurrentIndex(settings.value("connectMatchMode", 0).toInt());
	ui.imageMatch_controlMatchModeBox->setCurrentIndex(settings.value("controlMatchMode", 0).toInt());
	ui.imageMatch_waitMatchDataBandBox->setCurrentIndex(settings.value("waitMatchDataBand", 0).toInt());
	ui.imageMatch_referenceDataBandBox->setCurrentIndex(settings.value("referenceDataBand", 0).toInt());
	ui.imageMatch_controlMatchModelBox->setCurrentIndex(settings.value("controlMatchModel", 0).toInt());
	ui.checkBox->setChecked(settings.value("checkBox_c", false).toBool());
	ui.checkBox_2->setChecked(settings.value("checkBox2", false).toBool());
	ui.checkBox_3->setChecked(settings.value("checkBox3", false).toBool());
	ui.checkBox_4->setChecked(settings.value("checkBox4", false).toBool());
	ui.checkBox_5->setChecked(settings.value("checkBox5", false).toBool());
	ui.checkBox_11->setChecked(settings.value("checkBox11", false).toBool());
	ui.checkBox_72->setChecked(settings.value("checkBox72", false).toBool());
	ui.checkBox_74->setChecked(settings.value("checkBox74", false).toBool());
	ui.checkBox_44->setChecked(settings.value("checkBox44", false).toBool());
	ui.lineEdit->setText(settings.value("lineEdit", "").toString());
	ui.lineEdit_2->setText(settings.value("lineEdit2", "").toString());
	ui.lineEdit_3->setText(settings.value("lineEdit3", "").toString());
	ui.lineEdit_4->setText(settings.value("lineEdit4", "").toString());
	ui.lineEdit_6->setText(settings.value("lineEdit6", "").toString());
	ui.lineEdit_7->setText(settings.value("lineEdit7", "").toString());
	ui.lineEdit_8->setText(settings.value("lineEdit8", "").toString());
	ui.lineEdit_9->setText(settings.value("lineEdit9", "").toString());
	ui.lineEdit_10->setText(settings.value("lineEdit10", "").toString());
	ui.lineEdit_11->setText(settings.value("lineEdit11", "").toString());
	ui.lineEdit_22->setText(settings.value("lineEdit22", "").toString());
	ui.lineEdit_23->setText(settings.value("lineEdit23", "").toString());
	ui.lineEdit_48->setText(settings.value("lineEdit48", "").toString());
	settings.endGroup();

	// 加载卫星图像调整页面设置 
	settings.beginGroup("SatelliteImageAdjust");
	ui.sateImageAdjust_adjustModelBox->setCurrentIndex(settings.value("adjustModel", 0).toInt());
	ui.checkBox_49->setChecked(settings.value("checkBox49", false).toBool());
	ui.checkBox_50->setChecked(settings.value("checkBox50", false).toBool());
	ui.checkBox_51->setChecked(settings.value("checkBox51", false).toBool());
	ui.checkBox_52->setChecked(settings.value("checkBox52", false).toBool());
	ui.checkBox_53->setChecked(settings.value("checkBox53", false).toBool());
	ui.checkBox_54->setChecked(settings.value("checkBox54", false).toBool());
	ui.checkBox_55->setChecked(settings.value("checkBox55", false).toBool());
	ui.checkBox_83->setChecked(settings.value("checkBox83", false).toBool());
	ui.checkBox_84->setChecked(settings.value("checkBox84", false).toBool());
	ui.lineEdit_45->setText(settings.value("lineEdit45", "").toString());
	ui.lineEdit_46->setText(settings.value("lineEdit46", "").toString());
	ui.lineEdit_47->setText(settings.value("lineEdit47", "").toString());
	ui.lineEdit_58->setText(settings.value("lineEdit58", "").toString());
	ui.lineEdit_59->setText(settings.value("lineEdit59", "").toString());
	settings.endGroup();

	// 加载纠正参数页面设置 
	settings.beginGroup("NormalCorrect");
	ui.normalCorrect_resultFormatBox->setCurrentIndex(settings.value("resultFormat", 0).toInt());
	ui.normalCorrect_samplingWayBox->setCurrentIndex(settings.value("samplingWay", 0).toInt());
	ui.lineEdit_5->setText(settings.value("lineEdit5", "").toString());
	ui.lineEdit_12->setText(settings.value("lineEdit12", "").toString());
	ui.lineEdit_13->setText(settings.value("lineEdit13", "").toString());
	ui.lineEdit_60->setText(settings.value("lineEdit60", "").toString());
	ui.checkBox_6->setChecked(settings.value("checkBox6", false).toBool());
	ui.checkBox_7->setChecked(settings.value("checkBox7", false).toBool());
	ui.checkBox_8->setChecked(settings.value("checkBox8", false).toBool());
	ui.checkBox_9->setChecked(settings.value("checkBox9", false).toBool());
	ui.checkBox_10->setChecked(settings.value("checkBox10", false).toBool());
	ui.checkBox_12->setChecked(settings.value("checkBox12", false).toBool());
	ui.checkBox_85->setChecked(settings.value("checkBox85", false).toBool());
	settings.endGroup();

	// 加载影像融合页面设置 
	settings.beginGroup("ImageFusion");
	ui.imageFusion_fusionWayBox->setCurrentIndex(settings.value("fusionWay", 0).toInt());
	ui.imageFusion_outImageFormatBox->setCurrentIndex(settings.value("outImageFormat", 0).toInt());
	ui.lineEdit_14->setText(settings.value("lineEdit14", "").toString());
	ui.checkBox_14->setChecked(settings.value("checkBox14", true).toBool());
	ui.checkBox_15->setChecked(settings.value("checkBox15", false).toBool());
	ui.checkBox_16->setChecked(settings.value("checkBox16", true).toBool());
	ui.checkBox_17->setChecked(settings.value("checkBox17", false).toBool());
	ui.checkBox_18->setChecked(settings.value("checkBox18", false).toBool());
	ui.checkBox_19->setChecked(settings.value("checkBox19", false).toBool());
	settings.endGroup();

	// 加载影像匀色页面设置 
	settings.beginGroup("ImageEventColor");
	ui.imageEventColor_useModuleDBBox->setCurrentIndex(settings.value("moduleDB", 0).toInt());
	ui.imageEventColor_eventLightColorFunBox->setCurrentIndex(settings.value("eventLightColorFun", 0).toInt());
	ui.imageEventColor_improveClarityBox->setCurrentIndex(settings.value("improveClarityLevel", 0).toInt());
	ui.lineEdit_15->setText(settings.value("lineEdit15", "").toString());
	ui.lineEdit_16->setText(settings.value("lineEdit16", "").toString());
	ui.lineEdit_17->setText(settings.value("lineEdit17", "").toString());
	ui.lineEdit_19->setText(settings.value("lineEdit19", "").toString());
	ui.lineEdit_20->setText(settings.value("lineEdit20", "").toString());
	ui.lineEdit_21->setText(settings.value("lineEdit21", "").toString()); 
	ui.lineEdit_24->setText(settings.value("lineEdit24", "").toString());
	ui.lineEdit_25->setText(settings.value("lineEdit25", "").toString());
	ui.lineEdit_26->setText(settings.value("lineEdit26", "").toString());
	ui.lineEdit_27->setText(settings.value("lineEdit27", "").toString());
	ui.lineEdit_28->setText(settings.value("lineEdit28", "").toString());
	ui.imageEventColor_useModuleDBCheckBox->setChecked(settings.value("useModuleDB", false).toBool());
	ui.imageEventColor_improveClarityCheckBox->setChecked(settings.value("improveClarity", false).toBool());
	ui.output8bImage_checkBox->setChecked(settings.value("output8bImage_checkBox", false).toBool());
	ui.checkBox_20->setChecked(settings.value("checkBox20", false).toBool());
	ui.checkBox_21->setChecked(settings.value("checkBox21", false).toBool());
	ui.checkBox_22->setChecked(settings.value("checkBox22", false).toBool());
	ui.checkBox_23->setChecked(settings.value("checkBox23", false).toBool()); 
	ui.checkBox_24->setChecked(settings.value("checkBox24", false).toBool()); 
	ui.checkBox_26->setChecked(settings.value("checkBox26", false).toBool());
	ui.checkBox_27->setChecked(settings.value("checkBox27", false).toBool());
	ui.checkBox_30->setChecked(settings.value("checkBox30", false).toBool());
	ui.checkBox_31->setChecked(settings.value("checkBox31", false).toBool());
	ui.checkBox_32->setChecked(settings.value("checkBox32", false).toBool());
	ui.checkBox_33->setChecked(settings.value("checkBox33", false).toBool());
	ui.checkBox_35->setChecked(settings.value("checkBox35", false).toBool());
	ui.checkBox_36->setChecked(settings.value("checkBox36", false).toBool());
	settings.endGroup();

	// 加载影像裁切页面设置 
	settings.beginGroup("ImageCrop");
	ui.comboBox->setCurrentIndex(settings.value("comboBox", 0).toInt());
	ui.lineEdit_50->setText(settings.value("lineEdit50", "").toString());
	ui.lineEdit_51->setText(settings.value("lineEdit51", "").toString());
	ui.checkBox_58->setChecked(settings.value("checkBox58", true).toBool());
	ui.checkBox_59->setChecked(settings.value("checkBox59", false).toBool());
	ui.checkBox_60->setChecked(settings.value("checkBox60", true).toBool());
	ui.checkBox_61->setChecked(settings.value("checkBox61", false).toBool());
	ui.checkBox_62->setChecked(settings.value("checkBox62", false).toBool());
	ui.checkBox_63->setChecked(settings.value("checkBox63", false).toBool());
	settings.endGroup();

	// 加载影像镶嵌页面设置 
	settings.beginGroup("ImageInlay");
	ui.imageInlay_resultFormatBox->setCurrentIndex(settings.value("resultFormat", 0).toInt());
	ui.checkBox_25->setChecked(settings.value("checkBox25", false).toBool());
	settings.endGroup();

	// 加载智能镶嵌页面设置 
	settings.beginGroup("SmartMosaic");
	ui.SamplingMode_Box->setCurrentIndex(settings.value("SamplingMode", 0).toInt());
	ui.resulttype_Box->setCurrentIndex(settings.value("resulttype", 0).toInt());
	ui.ComboBox_4->setCurrentIndex(settings.value("ComboBox4", 0).toInt());
	ui.LineEdit->setText(settings.value("LineEdit", "").toString());
	ui.LineEdit_2->setText(settings.value("LineEdit2", "").toString());
	ui.LineEdit_3->setText(settings.value("LineEdit3", "").toString());
	ui.LineEdit_4->setText(settings.value("LineEdit4", "").toString());
	ui.lineEdit_37->setText(settings.value("lineEdit37", "").toString());
	ui.CheckBox->setChecked(settings.value("CheckBox", false).toBool());
	ui.CheckBox_3->setChecked(settings.value("CheckBox3", false).toBool());
	ui.CheckBox_4->setChecked(settings.value("CheckBox4", false).toBool());
	ui.CheckBox_5->setChecked(settings.value("CheckBox5", false).toBool());
	ui.CheckBox_6->setChecked(settings.value("CheckBox6", false).toBool());
	ui.CheckBox_7->setChecked(settings.value("CheckBox7", false).toBool());
	settings.endGroup();

	// 加载快速镶嵌页面设置 
	settings.beginGroup("QuickMosaic");
	ui.QuickMosaic_resultFormatBox->setCurrentIndex(settings.value("QresultFormatBox", 0).toInt());
	ui.checkBox_71->setChecked(settings.value("checkBox71", false).toBool());
	settings.endGroup();

	// 加载匹配校正页面设置 
	settings.beginGroup("MatchCorrect");
	ui.matchCorrect_changeModelBox->setCurrentIndex(settings.value("changeModel", 0).toInt());
	ui.checkBox_29->setChecked(settings.value("checkBox29", false).toBool());
	ui.checkBox_34->setChecked(settings.value("checkBox34", false).toBool());
	ui.lineEdit_30->setText(settings.value("lineEdit30", "").toString());
	settings.endGroup();

	// 加载投影变换页面设置 
	settings.beginGroup("ProjectChange");
	ui.projectChange_samplingWayBox->setCurrentIndex(settings.value("samplingWay", 0).toInt());
	ui.checkBox_37->setChecked(settings.value("checkBox37", false).toBool());
	ui.lineEdit_18->setText(settings.value("lineEdit18", "").toString());
	ui.projectChange_resultFormatBox->setCurrentIndex(settings.value("resultFormat", 0).toInt());
	ui.lineEdit_29->setText(settings.value("lineEdit29", "").toString());
	ui.lineEdit_31->setText(settings.value("lineEdit31", "").toString());
	settings.endGroup();

	// 加载格式转换页面设置
	settings.beginGroup("FormatConversion");
	ui.radioButton_3->setChecked(settings.value("dataTypeImage", true).toBool());
	ui.radioButton_4->setChecked(settings.value("dataTypeDEM", false).toBool());
	ui.resultType_Box->setCurrentIndex(settings.value("resultType", 0).toInt());
	ui.checkBox_70->setChecked(settings.value("bitDepthAdjust", true).toBool());
	ui.Bitdepth_Box->setCurrentIndex(settings.value("bitDepth", 0).toInt());
	ui.checkBox_66->setChecked(settings.value("heightCorrection", false).toBool());
	ui.checkBox_67->setChecked(settings.value("ellipsoidToLevel", true).toBool());
	ui.checkBox_68->setChecked(settings.value("levelToEllipsoid", false).toBool());
	ui.checkBox_69->setChecked(settings.value("createPyramid", true).toBool());
	settings.endGroup();

	// 加载SAR影像滤波参数页面设置 
	settings.beginGroup("SARImageFiltering");
	ui.ImageType_Box->setCurrentIndex(settings.value("ImageType_Box", 0).toInt());
	ui.FilteringMethod_Box->setCurrentIndex(settings.value("FilteringMethod_Box", 0).toInt());
	ui.LineEdit_6->setText(settings.value("LineEdit_6", "").toString());
	ui.LineEdit_5->setText(settings.value("LineEdit_5", "").toString());
	ui.LineEdit_7->setText(settings.value("LineEdit_7", "").toString());
	ui.LineEdit_8->setText(settings.value("LineEdit_8", "").toString());
	ui.ComboBox_5->setCurrentIndex(settings.value("ComboBox_5", 0).toInt());
	settings.endGroup();

	// 加载金字塔创建页面设置 
	settings.beginGroup("PyramidCreate");
	ui.pyramidCreate_samplingWayBox->setCurrentIndex(settings.value("samplingWay", 0).toInt());
	settings.endGroup();

	// 加载真彩色转换页面设置 
	settings.beginGroup("TrueColorConvert");
	ui.trueColorConver_outImageFormatBox->setCurrentIndex(settings.value("outImageFormat", 0).toInt());
	ui.trueColorConver_colorAdjustWayBox->setCurrentIndex(settings.value("colorAdjustWay", 0).toInt());
	ui.trueColorConver_groundObjectTypeBox->setCurrentIndex(settings.value("groundObjectType", 0).toInt());
	ui.AdjustClearnessType_Box->setCurrentIndex(settings.value("AdjustClearnessType", 0).toInt());
	ui.lineEdit_32->setText(settings.value("lineEdit32", "").toString());
	ui.lineEdit_33->setText(settings.value("lineEdit33", "").toString());
	ui.lineEdit_34->setText(settings.value("lineEdit34", "").toString());
	ui.lineEdit_35->setText(settings.value("lineEdit35", "").toString());
	ui.lineEdit_36->setText(settings.value("lineEdit36", "").toString()); 
	ui.lineEdit_38->setText(settings.value("lineEdit38", "").toString());
	ui.lineEdit_39->setText(settings.value("lineEdit39", "").toString());
	ui.lineEdit_40->setText(settings.value("lineEdit40", "").toString());
	ui.lineEdit_41->setText(settings.value("lineEdit41", "").toString());
	ui.lineEdit_56->setText(settings.value("lineEdit56", "").toString());
	ui.lineEdit_42->setText(settings.value("lineEdit42", "").toString());
	ui.lineEdit_43->setText(settings.value("lineEdit43", "").toString());
	ui.lineEdit_44->setText(settings.value("lineEdit44", "").toString());
	ui.lineEdit_57->setText(settings.value("lineEdit57", "").toString());
	ui.checkBox_39->setChecked(settings.value("checkBox39", false).toBool());
	ui.checkBox_38->setChecked(settings.value("checkBox38", false).toBool());
	ui.checkBox_41->setChecked(settings.value("checkBox41", false).toBool());
	ui.checkBox_40->setChecked(settings.value("checkBox40", false).toBool());
	ui.checkBox_42->setChecked(settings.value("checkBox42", false).toBool());
	ui.checkBox_43->setChecked(settings.value("checkBox43", false).toBool());
	ui.AdjustClearness_check->setChecked(settings.value("AdjustClearnesscheck", false).toBool());
	ui.checkBox_45->setChecked(settings.value("checkBox45", false).toBool());
	ui.checkBox_46->setChecked(settings.value("checkBox46", false).toBool());
	ui.checkBox_47->setChecked(settings.value("checkBox47", false).toBool());
	ui.checkBox_48->setChecked(settings.value("checkBox48", false).toBool());
	ui.checkBox_82->setChecked(settings.value("checkBox82", false).toBool());
	ui.checkBox_28->setChecked(settings.value("checkBox28", false).toBool());
	settings.endGroup();

	////自动化流程一
	//settings.beginGroup("LabelStates");
	//for (int i = 74; i <= 83; ++i) {
	//	QLabel* label = findChild<QLabel*>(QString("label_%1").arg(i));
	//	if (label) {
	//		label->setEnabled(settings.value(QString("label%1_enabled").arg(i), true).toBool());
	//		label->setText(settings.value(QString("label%1_text").arg(i), "").toString());
	//		QString styleKey = QString("label%1_stylesheet").arg(i);
	//		QString defaultStyle = "background-color: rgb(0, 170, 0);";
	//		label->setStyleSheet(settings.value(styleKey, defaultStyle).toString());

	//		if (label->text() == QString::fromLocal8Bit("影像镶嵌")){
	//			m_flagMosaic = false;
	//		}
	//		else {
	//			m_flagMosaic = true;
	//		}
	//	}
	//}
	//settings.endGroup();

}

//////////////////////////////

/**
 * @brief 显示功能界面并等待用户确认
 * @param functionName 要显示的功能项名称
 * @return 用户是否点击了确认（QDialog::Accepted 或 QDialog::Rejected）
 */
int SystemConfig::showFunctionAndWait(const QString& functionName)
{
	if (functionName == QString::fromLocal8Bit("纠正模型配准"))
	{
		// 1. 设置界面状态 - 只显示指定的功能项
		QStringList visibleItems = { QString::fromLocal8Bit("运行环境"), QString::fromLocal8Bit("配准纠正"), QString::fromLocal8Bit("影像匹配") };
		for (int i = 0; i < ui.funListWidget->count(); ++i) {
			QListWidgetItem* item = ui.funListWidget->item(i);
			item->setHidden(!visibleItems.contains(item->text()));
		}
	}
	else if (functionName == QString::fromLocal8Bit("影像纠正")) {
		// 1. 设置界面状态 - 只显示指定的功能项
		QStringList visibleItems = { QString::fromLocal8Bit("运行环境"), QString::fromLocal8Bit("正射纠正") };
		for (int i = 0; i < ui.funListWidget->count(); ++i) {
			QListWidgetItem* item = ui.funListWidget->item(i);
			item->setHidden(!visibleItems.contains(item->text()));
		}
	}
	else
	{
		// 1. 设置界面状态 - 只显示指定的功能项
		QStringList visibleItems = { QString::fromLocal8Bit("运行环境"), functionName };
		for (int i = 0; i < ui.funListWidget->count(); ++i) {
			QListWidgetItem* item = ui.funListWidget->item(i);
			item->setHidden(!visibleItems.contains(item->text()));
		}
	}
	//settingShowWidget(QString::fromLocal8Bit("运行环境"));

	settingShowWidget(functionName);

	// 4. 显示模态对话框并等待用户操作 
	QEventLoop loop;
	connect(this, &SystemConfig::accepted, &loop, &QEventLoop::quit);
	connect(this, &SystemConfig::rejected, &loop, &QEventLoop::quit);

	return this->exec(); // 返回用户操作结果 
}

///**
// * @brief 影像金字塔创建
// */
//void SystemConfig::acceptModule(QString title, QStringList filenamelist)
//{
//	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有金字塔创建"))) {
//		return;
//	}
//	setProjectdir();
//	if (title == QString::fromLocal8Bit("金字塔创建"))
//	{
//		logEdit->append(QString::fromLocal8Bit("****影像金字塔创建****"));
//
//		// 初始化任务队列
//		m_taskQueue.clear();
//		m_runningProcesses.store(0);
//		m_completedTasks.store(0);
//		m_startTime = QDateTime::currentDateTime();
//		m_totalTasks = 0;
//
//		QString format = ui.pyramidCreate_samplingWayBox->currentText();
//		int skipCount = 0; // 记录跳过的文件数量
//		//QString imagePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择影像数据"), "../bin/runTestData", tr("TIF Image(*.tif);;TIFF Image(*.tiff);;ERDAS Image(*.img)"));
//		if (filenamelist.isEmpty())
//		{
//			QMessageBox::critical(
//				this,
//				QString::fromLocal8Bit("错误"),
//				QString::fromLocal8Bit("金字塔创建数据为空！"),
//				QMessageBox::Cancel  
//			);
//			emit FunctionAbnormalExit(QString::fromLocal8Bit("金字塔创建"));
//			ModelMutex.unlock();
//			return;
//		}
//
//		for (auto filename : filenamelist)
//		{
//			QFileInfo fileInfo(filename);
//
//			QString baseName = fileInfo.fileName();
//			QString suffix = fileInfo.suffix().toLower();
//			QDir dir(fileInfo.absolutePath());
//
//			// 检查是否已存在金字塔文件（原文件或同名.TIL/.til文件）
//			bool pyramidExists = false;
//
//			// 1. 首先检查原文件的金字塔文件 
//			QStringList existingFiles = dir.entryList(
//				{ baseName + ".ovr", baseName + ".OVR",
//				 baseName + ".rrd", baseName + ".RRD" },
//				QDir::Files);
//
//			if (!existingFiles.isEmpty()) {
//				pyramidExists = true;
//			}
//
//			// 2. 如果原文件是.tif/.tiff，检查同名.til文件的金字塔文件 
//			if (!pyramidExists && (suffix == "tif" || suffix == "tiff")) {
//				QString tilBaseName = baseName + ".til";
//				QString tilBaseNameUpper = baseName + ".TIL";
//
//				// 检查.til文件是否存在
//				if (dir.exists(tilBaseName) || dir.exists(tilBaseNameUpper)) {
//					// 检查.til文件的金字塔文件
//					QStringList tilPyramidFiles = dir.entryList(
//						{ tilBaseName + ".ovr", tilBaseName + ".OVR",
//						 tilBaseName + ".rrd", tilBaseName + ".RRD" },
//						QDir::Files);
//					QStringList TILPyramidFiles = dir.entryList(
//						{ tilBaseNameUpper + ".ovr", tilBaseNameUpper + ".OVR",
//						 tilBaseNameUpper + ".rrd", tilBaseNameUpper + ".RRD" },
//						QDir::Files);
//
//					if (!tilPyramidFiles.isEmpty() || !tilPyramidFiles.isEmpty()) {
//						pyramidExists = true;
//					}
//				}
//			}
//
//			// 3. 如果金字塔已存在则跳过
//			if (pyramidExists) {
//				skipCount++;
//				continue;
//			}
//
//			// 创建 QDomDocument
//			QDomDocument doc;
//
//			// 添加 XML 声明
//			QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
//			doc.appendChild(instruction);
//
//			// 创建根节点 <CreatePyramid>
//			QDomElement root = doc.createElement("CreatePyramid");
//			root.setAttribute("version", "1.0");
//			doc.appendChild(root);
//
//			// 创建 <InputImageFilePath> 子节点
//			QDomElement inputImagePath = doc.createElement("InputImageFilePath");
//			inputImagePath.setAttribute("type", "string");
//			QDomText inputImagePathText = doc.createTextNode(filename);
//			inputImagePath.appendChild(inputImagePathText);
//			root.appendChild(inputImagePath);
//
//			// 创建 <resampleMethod> 子节点
//			QDomElement resampleMethod = doc.createElement("resampleMethod");
//			resampleMethod.setAttribute("type", "string");
//			QDomText resampleMethodText = doc.createTextNode(format);
//			resampleMethod.appendChild(resampleMethodText);
//			root.appendChild(resampleMethod);
//
//			// 写入文件
//			QString xmlPath = projectdir + QString::fromLocal8Bit("imageHandleBusinessConfig/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml");
//
//			QFile file(xmlPath);
//			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//				emit FunctionAbnormalExit(QString::fromLocal8Bit("金字塔创建"));
//				ModelMutex.unlock();
//				return;
//			}
//
//			QTextStream out(&file);
//			out.setCodec("UTF-8");
//			out << doc.toString(4);  // 4 表示缩进 4 个空格，使 XML 可读性更好
//			file.close();
//
//			// 添加到任务队列 
//			TaskInfo task;
//			task.taskFilePath = xmlPath;
//			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSCreatePyramidCmd.exe");
//#ifdef Q_OS_LINUX 
//			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSCreatePyramidCmd.x"));
//#else
//			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSCreatePyramidCmd.exe"));
//#endif
//			task.outfilename = projectdir + "imageHandleBusinessConfig/" + fileInfo.completeBaseName() + QString::fromLocal8Bit(".ovr");
//			m_taskQueue.enqueue(task);
//			m_totalTasks++;
//		}
//
//		if (skipCount > 0) {
//			logEdit->append(QString::fromLocal8Bit("检测到%1个影像已存在金字塔，已跳过").arg(skipCount));
//		}
//		if (m_taskQueue.empty())
//		{
//			ModelMutex.unlock();
//			emit CreatepyramidFinished();
//
//			return;
//		}
//		// 记录开始时间 
//		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
//		// 开始处理任务 
//		startNextTasks(QString::fromLocal8Bit("****金字塔创建"), true);
//	}
//}

//bool SystemConfig::addWeakTextureNode(const QString& xmlFilePath, bool WeakTexturePS)
//{
//	// 打开并读取XML文件
//	QFile file(xmlFilePath);
//	if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
//		qDebug() << u8"无法打开文件:" << xmlFilePath;
//		return false;
//	}
//
//	// 加载XML文档 
//	QDomDocument doc;
//	QString errorMsg;
//	int errorLine, errorColumn;
//	if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
//		qDebug() << u8"XML解析错误:" << errorMsg
//			<< "at line" << errorLine << ", column" << errorColumn;
//		file.close();
//		return false;
//	}
//
//	QDomNodeList weakTextureList = doc.elementsByTagName("WeakTexture");
//	if (!weakTextureList.isEmpty()) {
//		qDebug() << u8"WeakTexture节点已存在";
//		file.close();
//		return true;
//	}
//
//	// 查找SingleModelMatch节点
//	QDomNodeList singleModelMatchList = doc.elementsByTagName("SingleModelMatch");
//	if (singleModelMatchList.isEmpty()) {
//		qDebug() << u8"未找到SingleModelMatch节点";
//		file.close();
//		return false;
//	}
//
//	QDomElement singleModelMatch = singleModelMatchList.at(0).toElement();
//
//	// 创建WeakTexture节点 
//	QDomElement weakTexture = doc.createElement("WeakTexture");
//	weakTexture.setAttribute("type", "bool");
//	QDomText text = doc.createTextNode(QVariant(WeakTexturePS).toString());
//	weakTexture.appendChild(text);
//
//	// 在SingleModelMatch后插入新节点 
//	singleModelMatch.parentNode().insertAfter(weakTexture, singleModelMatch);
//
//	// 回写文件
//	file.resize(0);  // 清空原文件内容 
//	QTextStream out(&file);
//	out.setCodec("UTF-8");
//	doc.save(out, 4); // 缩进为4个空格
//	file.close();
//
//	//qDebug() << u8"成功添加WeakTexture节点到" << xmlFilePath;
//	return true;
//}

//bool SystemConfig::addWeakTextureNode(const QString& xmlFilePath, bool WeakTexturePS)
//{
//	// 打开并读取XML文件
//	QFile file(xmlFilePath);
//	if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
//		qDebug() << u8"无法打开文件:" << xmlFilePath;
//		return false;
//	}
//
//	// 加载XML文档 
//	QDomDocument doc;
//	QString errorMsg;
//	int errorLine, errorColumn;
//	if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
//		qDebug() << u8"XML解析错误:" << errorMsg
//			<< "at line" << errorLine << ", column" << errorColumn;
//		file.close();
//		return false;
//	}
//
//	QDomNodeList weakTextureList = doc.elementsByTagName("MatchGcp");
//	if (!weakTextureList.isEmpty()) {
//		qDebug() << u8"MatchGcp节点已存在";
//		file.close();
//		return true;
//	}
//
//	// 查找WeakTexture节点
//	QDomNodeList singleModelMatchList = doc.elementsByTagName("WeakTexture");
//	if (singleModelMatchList.isEmpty()) {
//		qDebug() << u8"未找到WeakTexture节点";
//		file.close();
//		return false;
//	}
//
//	QDomElement WeakTexture = singleModelMatchList.at(0).toElement();
//
//	// 创建MatchGcp节点 
//	QDomElement MatchGcp = doc.createElement("MatchGcp");
//	MatchGcp.setAttribute("type", "bool");
//	QDomText text = doc.createTextNode(QVariant(WeakTexturePS).toString());
//	MatchGcp.appendChild(text);
//
//	// 在WeakTexture后插入新节点 
//	WeakTexture.parentNode().insertAfter(MatchGcp, WeakTexture);
//
//	// 回写文件
//	file.resize(0);  // 清空原文件内容 
//	QTextStream out(&file);
//	out.setCodec("UTF-8");
//	doc.save(out, 4); // 缩进为4个空格
//	file.close();
//
//	//qDebug() << u8"成功添加MatchGcp节点到" << xmlFilePath;
//	return true;
//}
//
//bool SystemConfig::addWeakTextureNode(const QString& xmlFilePath,
//	bool WeakTexturePS,
//	const QStringList& DEMReferencefilename,
//	const QStringList& DOMReferencefilename)
//{
//	// 打开并读取XML文件 
//	QFile file(xmlFilePath);
//	if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
//		qDebug() << u8"无法打开文件:" << xmlFilePath;
//		return false;
//	}
//
//	// 加载XML文档 
//	QDomDocument doc;
//	QString errorMsg;
//	int errorLine, errorColumn;
//	if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
//		qDebug() << u8"XML解析错误:" << errorMsg
//			<< "at line" << errorLine << ", column" << errorColumn;
//		file.close();
//		return false;
//	}
//
//	// 检查并添加DEM/DOM节点 
//	QDomNodeList gridFilterList = doc.elementsByTagName("GridFilterSize");
//	if (!gridFilterList.isEmpty()) {
//		QDomElement gridFilter = gridFilterList.at(0).toElement();
//
//		// 检查是否已存在DEM节点
//		QDomNodeList demList = doc.elementsByTagName("DEM");
//		if (demList.isEmpty() && !DEMReferencefilename.isEmpty()) {
//			// 创建DEM节点 
//			QDomElement demElement = doc.createElement("DEM");
//			for (const QString& demPath : DEMReferencefilename) {
//				QDomElement pathElement = doc.createElement("Path");
//				QDomText text = doc.createTextNode(demPath);
//				pathElement.appendChild(text);
//				demElement.appendChild(pathElement);
//			}
//			gridFilter.parentNode().insertAfter(demElement, gridFilter);
//		}
//
//		// 检查是否已存在DOM节点
//		QDomNodeList domList = doc.elementsByTagName("DOM");
//		if (domList.isEmpty() && !DOMReferencefilename.isEmpty()) {
//			// 创建DOM节点 
//			QDomElement domElement = doc.createElement("DOM");
//			for (const QString& domPath : DOMReferencefilename) {
//				QDomElement pathElement = doc.createElement("Path");
//				QDomText text = doc.createTextNode(domPath);
//				pathElement.appendChild(text);
//				domElement.appendChild(pathElement);
//			}
//
//			// 插入到DEM节点之后（如果DEM存在）或GridFilterSize之后
//			QDomNode refNode = demList.isEmpty() ? gridFilter : demList.at(0);
//			refNode.parentNode().insertAfter(domElement, refNode);
//		}
//	}
//
//	// 保留原有WeakTexture节点处理逻辑 
//	QDomNodeList weakTextureList = doc.elementsByTagName("MatchGcp");
//	if (!weakTextureList.isEmpty()) {
//		qDebug() << u8"MatchGcp节点已存在";
//		file.close();
//		return true;
//	}
//
//	QDomNodeList singleModelMatchList = doc.elementsByTagName("WeakTexture");
//	if (singleModelMatchList.isEmpty()) {
//		qDebug() << u8"未找到WeakTexture节点";
//		file.close();
//		return false;
//	}
//
//	QDomElement WeakTexture = singleModelMatchList.at(0).toElement();
//
//	// 创建MatchGcp节点 
//	QDomElement MatchGcp = doc.createElement("MatchGcp");
//	MatchGcp.setAttribute("type", "bool");
//	QDomText text = doc.createTextNode(QVariant(WeakTexturePS).toString());
//	MatchGcp.appendChild(text);
//
//	// 在WeakTexture后插入新节点 
//	WeakTexture.parentNode().insertAfter(MatchGcp, WeakTexture);
//
//	// 回写文件
//	file.resize(0);  // 清空原文件内容 
//	QTextStream out(&file);
//	out.setCodec("UTF-8");
//	doc.save(out, 4); // 缩进为4个空格
//	file.close();
//
//	return true;
//}

bool SystemConfig::addWeakTextureNode(const QString& xmlFilePath, bool WeakTexturePS, const QStringList& DOMReferencefilename)
{
	// 打开并读取XML文件 
	QFile file(xmlFilePath);
	if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
		//qDebug() << u8"无法打开文件:" << xmlFilePath;
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点匹配任务单生成, 无法打开文件"));
		return false;
	}

	// 加载XML文档 
	QDomDocument doc;
	QString errorMsg;
	int errorLine, errorColumn;
	if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
		//qDebug() << u8"XML解析错误:" << errorMsg
		//	<< "at line" << errorLine << ", column" << errorColumn;
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点匹配任务单生成, XML解析错误"));
		file.close();
		return false;
	}

	if (WeakTexturePS) {
		// 添加DOM节点（新增部分）
		QDomNodeList demList = doc.elementsByTagName("DEM");
		if (!demList.isEmpty()) {
			QDomElement lastDem = demList.at(demList.size() - 1).toElement();

			// 遍历所有DOM文件路径 
			for (const QString& domPath : DOMReferencefilename) {
				if (!domPath.isEmpty()) {
					QDomElement domElement = doc.createElement("DOM");
					QDomText domText = doc.createTextNode(domPath);
					domElement.appendChild(domText);

					// 在最后一个DEM节点后插入DOM节点 
					lastDem.parentNode().insertAfter(domElement, lastDem);
					lastDem = domElement; // 更新最后一个节点引用 
				}
			}
		}
		else {
			//qDebug() << u8"未找到DEM节点，无法添加DOM节点";
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点匹配任务单生成, 未找到DEM节点，无法添加DOM节点"));
		}
	}

	// 检查MatchGcp节点是否已存在 
	QDomNodeList weakTextureList = doc.elementsByTagName("MatchGcp");
	if (!weakTextureList.isEmpty()) {
		//qDebug() << u8"MatchGcp节点已存在";
		PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("MatchGcp节点已存在"));
		file.close();
		return true;
	}

	// 查找WeakTexture节点 
	QDomNodeList singleModelMatchList = doc.elementsByTagName("WeakTexture");
	if (singleModelMatchList.isEmpty()) {
		//qDebug() << u8"未找到WeakTexture节点";
		PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("未找到WeakTexture节点"));
		file.close();
		return false;
	}

	QDomElement WeakTexture = singleModelMatchList.at(0).toElement();

	// 创建MatchGcp节点 
	QDomElement MatchGcp = doc.createElement("MatchGcp");
	MatchGcp.setAttribute("type", "bool");
	QDomText text = doc.createTextNode(QVariant(WeakTexturePS).toString());
	MatchGcp.appendChild(text);

	// 在WeakTexture后插入新节点 
	WeakTexture.parentNode().insertAfter(MatchGcp, WeakTexture);

	
	// 回写文件 
	file.resize(0);  // 清空原文件内容 
	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4); // 缩进为4个空格 
	file.close();

	//qDebug() << u8"成功修改XML文件:" << xmlFilePath;
	return true;
}

/**
 * @brief 影像金字塔创建
 */
void SystemConfig::acceptModule(QString title, QStringList filenamelist)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有金字塔创建"))) {
		//PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("已有金字塔创建任务,请稍候."), "SystemConfig.cpp", 2091);
		return;
	}
	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("金字塔创建"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}
	setProjectdir();
	if (title == QString::fromLocal8Bit("金字塔创建"))
	{
		logEdit->append(QString::fromLocal8Bit("****影像金字塔创建****"));

		// 初始化任务队列 
		m_taskQueue.clear();
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();
		m_totalTasks = 0;

		QString format = ui.pyramidCreate_samplingWayBox->currentText();
		int skipCount = 0; // 记录跳过的文件数量

		if (filenamelist.isEmpty())
		{
			QMessageBox::critical(
				this,
				QString::fromLocal8Bit("错误"),
				QString::fromLocal8Bit("金字塔创建输入数据为空！"),
				QMessageBox::Cancel
			);
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("金字塔创建输入数据为空!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("金字塔创建输入数据为空!"));
			ModelMutex.unlock();
			return;
		}

		for (auto filename : filenamelist)
		{
			QFileInfo fileInfo(filename);

			// 使用配置文件检查金字塔是否存在 
			bool pyramidExists = m_imagePS->checkPyramidExists(fileInfo);

			// 如果金字塔已存在则跳过 
			if (pyramidExists) {
				skipCount++;
				logEdit->append(QString::fromLocal8Bit("跳过文件（已存在金字塔）: %1").arg(filename));
				continue;
			}

			// 检查文件是否为支持的图像格式
			QString suffix = fileInfo.suffix().toLower();
			bool isSupportedImage = false;
			for (const QString& imageExt : m_imagePS->m_imageFileExtensions) {
				QString cleanExt = imageExt.startsWith(".") ? imageExt.mid(1) : imageExt;
				if (suffix == cleanExt) {
					isSupportedImage = true;
					break;
				}
			}

			// 检查文件是否为支持的TIL格式
			bool isSupportedTil = false;
			for (const QString& tilExt : m_imagePS->m_tilFileExtensions) {
				QString cleanExt = tilExt.startsWith(".") ? tilExt.mid(1) : tilExt;
				if (suffix == cleanExt) {
					isSupportedTil = true;
					break;
				}
			}

			// 如果文件既不是支持的图像格式也不是TIL格式，跳过 
			if (!isSupportedImage && !isSupportedTil) {
				skipCount++;
				logEdit->append(QString::fromLocal8Bit("跳过不支持的格式: %1").arg(filename));
				continue;
			}

			// 创建 QDomDocument
			QDomDocument doc;

			// 添加 XML 声明
			QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
			doc.appendChild(instruction);

			// 创建根节点 <CreatePyramid>
			QDomElement root = doc.createElement("CreatePyramid");
			root.setAttribute("version", "1.0");
			doc.appendChild(root);

			// 创建 <InputImageFilePath> 子节点
			QDomElement inputImagePath = doc.createElement("InputImageFilePath");
			QDomText inputImagePathText = doc.createTextNode(filename);
			inputImagePath.appendChild(inputImagePathText);
			root.appendChild(inputImagePath);

			// 创建 <resampleMethod> 子节点
			QDomElement resampleMethod = doc.createElement("resampleMethod");
			QDomText resampleMethodText = doc.createTextNode(format);
			resampleMethod.appendChild(resampleMethodText);
			root.appendChild(resampleMethod);

			// 创建 <pyramidType> 子节点
			QDomElement pyramidType = doc.createElement("pyramidType");
			QDomText pyramidTypeText = doc.createTextNode("OVR");
			pyramidType.appendChild(pyramidTypeText);
			root.appendChild(pyramidType);

			// 写入文件 
			QString xmlPath = projectdir + QString::fromLocal8Bit("imageHandleBusinessConfig/") + fileInfo.completeBaseName() + QString::fromLocal8Bit(".xml");

			QFile file(xmlPath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("金字塔创建任务单创建失败!"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("金字塔创建任务单创建失败!"));
				ModelMutex.unlock();
				return;
			}

			QTextStream out(&file);
			out.setCodec("UTF-8");
			out << doc.toString(4);   // 4 表示缩进 4 个空格，使 XML 可读性更好 
			file.close();

			// 添加到任务队列 
			TaskInfo task;
			task.taskFilePath = xmlPath;
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSCreatePyramidCmd.x"));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSCreatePyramidCmd.exe"));
#endif
			task.outfilename = projectdir + "imageHandleBusinessConfig/" + fileInfo.completeBaseName() + QString::fromLocal8Bit(".ovr");
			m_taskQueue.enqueue(task);
			m_totalTasks++;
		}

		if (skipCount > 0) {
			logEdit->append(QString::fromLocal8Bit("检测到%1个影像已存在金字塔或不支持格式，已跳过").arg(skipCount));
		}

		if (m_taskQueue.empty())
		{
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("****输入数据都已有金字塔!"));
			emit CreatepyramidFinished();
			return;
		}
		// 记录开始时间 
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh 时mm分ss秒")));
		// 开始处理任务 
		startNextTasks(QString::fromLocal8Bit("****金字塔创建"), true);
	}
}

/**
 * @brief 连接点匹配实现
 */
void SystemConfig::connectPointsMatch(QStringList DEMReferencefilename, QStringList DOMReferencefilename, const QString xmldir)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有连接点匹配"))) {
		return;
	}

	//if (!m_imagePS->getWorkProcessflag()) {
	//	// 使用封装后的函数显示界面并等待用户确认 
	//	int result = showFunctionAndWait(QString::fromLocal8Bit("影像匹配"));

	//	// 用户取消操作
	//	if (result != QDialog::Accepted) {
	//		ModelMutex.unlock();
	//		return;
	//	}
	//}

	setProjectdir();
	//dataModelConfig = m_imagePS->getDataModelConfig();
	//QDateTime timestamp = PublicFunctions::readTimestampFromXml("DataModelmarker");
	//if (!timestamp.isValid()) {
	//	ModelMutex.unlock();
	//	PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请先进行数据模型配对"));
	//	emit FunctionAbnormalExit(QString::fromLocal8Bit("请先进行数据模型配对"));
	//	return;
	//}
	//QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + xmldir + "/", timestamp);

	//QString tmpPath = projectdir + xmldir + "/" + QString::fromLocal8Bit("SatModelMakerCmdout.xml");
	QString SatModelMakerCmdoutPath = projectdir + xmldir + "/" + QString::fromLocal8Bit("SatModelMakerCmdout.xml");
	//QString SatModelMakerCmdoutPath = "";
	//for (auto datafile : resultFiles)
	//{
	//	if (tmpPath == datafile)
	//		SatModelMakerCmdoutPath = tmpPath;
	//}
	//if (SatModelMakerCmdoutPath == "" || dataModelConfig->ui.tableWidget_2->rowCount() == 0)
	//{
	//	ModelMutex.unlock();
	//	PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请先进行数据模型配对"));
	//	QMessageBox::warning(this, u8"警告", QString::fromLocal8Bit("没有可用的匹配模型，请先构建连接点匹配模型！"));
	//	return;
	//}

	QDir modeldir(QFileInfo(SatModelMakerCmdoutPath).absolutePath()); // 获取所在目录
	QStringList modelFiles = modeldir.entryList(QStringList() << "model_*", QDir::Files);

	if (!modelFiles.isEmpty()) {

		int deletedCount = 0;
		foreach(const QString &fileName, modelFiles) {
			QString filePath = modeldir.absoluteFilePath(fileName);
			if (QFile::remove(filePath)) {
				deletedCount++;
			}
		}
	}

	logEdit->append(QString::fromLocal8Bit("****连接点匹配****"));

	int BlkSizeXnum = ui.lineEdit_3->text().toInt();
	int BlkSizeYnum = ui.lineEdit_4->text().toInt();
	int GridNumXnum = ui.lineEdit->text().toInt();
	int GridNumYnum = ui.lineEdit_2->text().toInt();
	int GridFilterSizeNUM = ui.lineEdit_48->text().toInt();
	bool SingleModelMatchstate = ui.checkBox_2->isChecked();
	bool WeakTexturePS = ui.checkBox_74->isChecked();
	bool MatchGcpPS = ui.checkBox_44->isChecked();

	// 创建 QDomDocument
	QDomDocument doc;

	// 添加 XML 声明
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 <XQSatATMatchTaskMaker>
	QDomElement root = doc.createElement("XQSatATMatchTaskMaker");
	//root.setAttribute("version", "1.0");
	//doc.appendChild(root);

	// 创建 <SatModelFile> 子节点
	QDomElement SatModelFile = doc.createElement("SatModelFile");
	QDomText SatModelFilePathText = doc.createTextNode(projectdir + xmldir + "/SatModelMakerCmdout.xml");
	SatModelFile.appendChild(SatModelFilePathText);
	root.appendChild(SatModelFile);

	// 创建 <OutputFile> 子节点
	QDomElement OutputFilePath = doc.createElement("OutputFile");
	//inputImagePath.setAttribute("type", "string");
	QDomText OutputFilePathText = doc.createTextNode(projectdir + "SatTiePointMatch" + "/PSsatBlockAtMatchV3.xml");
	OutputFilePath.appendChild(OutputFilePathText);
	root.appendChild(OutputFilePath);

	// 创建 <Parameters> 子节点
	QDomElement Parameters = doc.createElement("Parameters");
	//创建 <BlkSizeX> 子节点
	QDomElement BlkSizeX = doc.createElement("BlkSizeX");
	BlkSizeX.setAttribute("type", "int32");
	QDomText BlkSizeXText = doc.createTextNode(QVariant(BlkSizeXnum).toString());
	BlkSizeX.appendChild(BlkSizeXText);
	Parameters.appendChild(BlkSizeX);

	// 创建 <BlkSizeY> 子节点
	QDomElement BlkSizeY = doc.createElement("BlkSizeY");
	BlkSizeY.setAttribute("type", "int32");
	QDomText BlkSizeYText = doc.createTextNode(QVariant(BlkSizeYnum).toString());
	BlkSizeY.appendChild(BlkSizeYText);
	Parameters.appendChild(BlkSizeY);

	// 创建 <GridNumX> 子节点
	QDomElement GridNumX = doc.createElement("GridNumX");
	GridNumX.setAttribute("type", "int32");
	QDomText GridNumXText = doc.createTextNode(QVariant(GridNumXnum).toString());
	GridNumX.appendChild(GridNumXText);
	Parameters.appendChild(GridNumX);

	// 创建 <GridNumY> 子节点
	QDomElement GridNumY = doc.createElement("GridNumY");
	GridNumY.setAttribute("type", "int32");
	QDomText GridNumYText = doc.createTextNode(QVariant(GridNumYnum).toString());
	GridNumY.appendChild(GridNumYText);
	Parameters.appendChild(GridNumY);

	// 创建 <SingleModelMatch> 子节点
	QDomElement SingleModelMatch = doc.createElement("SingleModelMatch");
	SingleModelMatch.setAttribute("type", "bool");
	QDomText SingleModelMatchText = doc.createTextNode(QVariant(SingleModelMatchstate).toString());
	SingleModelMatch.appendChild(SingleModelMatchText);
	Parameters.appendChild(SingleModelMatch);

	// 创建 <WeakTexture> 子节点
	QDomElement WeakTexture = doc.createElement("WeakTexture");
	WeakTexture.setAttribute("type", "bool");
	QDomText WeakTextureText = doc.createTextNode(QVariant(WeakTexturePS).toString());
	WeakTexture.appendChild(WeakTextureText);
	Parameters.appendChild(WeakTexture);

	// 创建 <MatchGcp> 子节点
	QDomElement MatchGcp = doc.createElement("MatchGcp");
	MatchGcp.setAttribute("type", "bool");
	QDomText MatchGcpText = doc.createTextNode(QVariant(MatchGcpPS).toString());
	MatchGcp.appendChild(MatchGcpText);
	Parameters.appendChild(MatchGcp);

	// 创建 <GridFilterSize> 子节点
	QDomElement GridFilterSize = doc.createElement("GridFilterSize");
	GridFilterSize.setAttribute("type", "int32");
	QDomText GridFilterSizeText = doc.createTextNode(QVariant(GridFilterSizeNUM).toString());
	GridFilterSize.appendChild(GridFilterSizeText);
	Parameters.appendChild(GridFilterSize);

	// 创建 <DEM> 子节点
	//if (!DEMReferencefilename.empty())
	//{
	//	for (auto data : DEMReferencefilename)
	//	{
	//		QDomElement DEM = doc.createElement("DEM");
	//		QDomText DEMText = doc.createTextNode(data);
	//		DEM.appendChild(DEMText);
	//		Parameters.appendChild(DEM);
	//	}
	//}
	//else
	//{
	//	QDomElement DEM = doc.createElement("DEM");
	//	QString exeDir = QCoreApplication::applicationDirPath();
	//	QDir dir(exeDir);
	//	QString tmpPath = dir.absolutePath();

	//	QDomText DEMText = doc.createTextNode(tmpPath + "\\Software\\etc\\globaldem\\globaldem.jp2");
	//	DEM.appendChild(DEMText);
	//	Parameters.appendChild(DEM);
	//}
	//QDomElement DEM = doc.createElement("DEM");
	//QString exeDir = QCoreApplication::applicationDirPath();
	//QDir dir(exeDir);
	//QString tmpPathdir = dir.absolutePath();

	//QDomText DEMText = doc.createTextNode(tmpPathdir + "\\Software\\etc\\globaldem\\globaldem.tif");
	//DEM.appendChild(DEMText);
	//Parameters.appendChild(DEM);
	if (MatchGcpPS) {

		for (auto DEMfilePath : DEMReferencefilename) {
			// 创建 <DOM> 子节点
			QDomElement DEM = doc.createElement("DEM");
			QDomText DEMText = doc.createTextNode(DEMfilePath);
			DEM.appendChild(DEMText);
			Parameters.appendChild(DEM);
		}
		//for (auto DOMfilePath : DOMReferencefilename) {
		//	// 创建 <DOM> 子节点
		//	QDomElement DOM = doc.createElement("DOM");
		//	QDomText DOMText = doc.createTextNode(DOMfilePath);
		//	DOM.appendChild(DOMText);
		//	Parameters.appendChild(DOM);
		//}
	}
	else {
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
		bool demFilePath = ui.checkBox_72->isChecked();

		if (!demFilePath)
		{
			if (!DEMReferencefilename.empty()) {
				for (auto data : DEMReferencefilename)
				{
					QDomElement DEM = doc.createElement("DEM");
					QDomText DEMText = doc.createTextNode(data);
					DEM.appendChild(DEMText);
					Parameters.appendChild(DEM);
				}
			}
			else {
				QDomElement DEM = doc.createElement("DEM");
				QString exeDir = QCoreApplication::applicationDirPath();
				QDir dir(exeDir);
				QString tmpPath = dir.absolutePath();

				QDomText DEMText = doc.createTextNode(tmpPath + "/Software/etc/globaldem/globaldem.tif");
				DEM.appendChild(DEMText);
				Parameters.appendChild(DEM);
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
			Parameters.appendChild(DEM);
		}
	}

	// 创建 <OutputDir> 子节点
	QDomElement OutputDir = doc.createElement("OutputDir");
	QDomText OutputDirText = doc.createTextNode(projectdir + "SatTiePointMatch/");
	OutputDir.appendChild(OutputDirText);
	Parameters.appendChild(OutputDir);

	root.appendChild(Parameters);

	doc.appendChild(root);

	// 写入文件
	QString xmlPath = projectdir + QString::fromLocal8Bit("SatTiePointMatch/PSsatATMatchTaskMakerCmd.xml");

	QFile file(xmlPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点匹配任务生成程序任务单创建失败!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("连接点匹配任务生成程序任务单创建失败!"));
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << doc.toString(4);  // 4 表示缩进 4 个空格，使 XML 可读性更好
	file.close();

	int currentIndex = ui.imageMatch_connectMatchModeBox->currentIndex();

	QString operatorname = "";
	if (currentIndex == -1)
	{
		//logEdit->append(QString::fromLocal8Bit("****请完成连接点系统配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成连接点系统配置!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成连接点系统配置!!!"));
		return;
	}

	//创建调用可执行程序的类
	QProcess * process1 = new QProcess(this);
	QString authCmd;
#ifdef Q_OS_LINUX 
	switch (currentIndex)
	{
		case 0://特征匹配
			operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV3.x");
			break;
		case 1://相位匹配
			operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV6.x");
			break;
		case 2://SAR相位匹配
			operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV5.x");
			break;
		case 3://密集匹配
			operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV7.exe");
			break;
		default:
			operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV3.exe");
	}

	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSSatATMatchTaskMakerCmd.x"));
#else

	switch (currentIndex)
	{
	case 0://特征匹配
		operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV3.exe");
		break;
	case 1://相位匹配
		operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV6.exe");
		break;
	case 2://SAR相位匹配
		operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV5.exe");
		break;
	case 3://密集匹配
		operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV7.exe");
		break;
	default:
		operatorname = QString::fromLocal8Bit("PSSatBlockAtMatchV3.exe");
	}
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSSatATMatchTaskMakerCmd.exe"));
#endif
	//启动程序，并输入参数
	process1->start(authCmd, QStringList() << xmlPath);
	m_systemProcesses.append(process1);
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点匹配任务单生成程序启动..."));
	// 2. 进程结束时自动显示 
	connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		logEdit->append(QString::fromLocal8Bit("****生成连接点任务单完成！"));
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("生成连接点任务单完成！,开始获取模型数量,创建连接点匹配任务队列"));
		process1->deleteLater(); // 安全释放内存
		m_systemProcesses.removeOne(process1);
		
		bool MatchGcpPS = ui.checkBox_44->isChecked();

		addWeakTextureNode(projectdir + QString::fromLocal8Bit("SatTiePointMatch/PSsatBlockAtMatchV3.xml"), MatchGcpPS, DOMReferencefilename);

		// 初始化任务队列
		m_taskQueue.clear();
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();
		m_totalTasks = 0;

		// 解析SatModelMakerCmdout.xml文件
		QDomDocument cmdoutDoc;
		QFile cmdoutFile(projectdir + xmldir + QString::fromLocal8Bit("/SatModelMakerCmdout.xml"));
		if (!cmdoutFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配对输出文件SatModelMakerCmdout.xml打开失败!!!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配对输出文件SatModelMakerCmdout.xml打开失败!!!"));
			return;
		}
		if (!cmdoutDoc.setContent(&cmdoutFile)) {
			cmdoutFile.close();
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配对输出文件SatModelMakerCmdout.xml文件格式有误"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配对输出文件SatModelMakerCmdout.xml文件格式有误"));
			return;
		}
		cmdoutFile.close();

		// 获取模型数量
		QDomElement cmdoutRoot = cmdoutDoc.documentElement();
		QDomElement modelNumElement = cmdoutRoot.firstChildElement("ModelNum");
		int totalTasks = modelNumElement.text().toInt();

		// 遍历所有模型
		QDomElement cmdoutModels = cmdoutRoot.firstChildElement("Models");
		QDomElement cmdoutModel = cmdoutModels.firstChildElement();
		int modelIndex = 0;

		PublicFunctions::writeTimestampToXml("SatTiePointMatch", m_imagePS->projectdir);

		if (cmdoutModel.isNull())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			return;
		}

		while (!cmdoutModel.isNull()) {

			// 创建进程对象   
			QProcess *process = new QProcess(this);

			QString xmlPathv3 = projectdir + QString::fromLocal8Bit("SatTiePointMatch/PSsatBlockAtMatchV3.xml");

			// 添加到任务队列 
			TaskInfo task;
			task.taskFilePath = xmlPathv3;
			task.taskFilePath2 = QString::number(modelIndex);
			task.taskFilePath3 = "1";
			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/") + operatorname;
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/") + operatorname));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/") + operatorname);
#endif
			task.outfilename = QString::fromLocal8Bit("model_") + QString::number(modelIndex);
			m_taskQueue.enqueue(task);
			m_totalTasks++;

			//// 启动进程（异步方式，不阻塞界面）   
			//process->start(m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/") + operatorname,
			//	QStringList() << xmlPathv3 << QString::number(modelIndex) << "1");

			cmdoutModel = cmdoutModel.nextSiblingElement();
			modelIndex++;
		}
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点匹配开始执行:") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		// 记录开始时间 
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		// 开始处理任务 
		startNextTasks(QString::fromLocal8Bit("****连接点匹配"), true);
	});
		
}

/**
 * @brief 影像求交函数
 */
void SystemConfig::IntersectCmd(QStringList Referencefilename, QStringList SatelliteImagefilename, QString str, QString OutputPath, bool enableLogging) {
	if (!tryLockFunction(intersectCmdMutex, QString::fromLocal8Bit("已有模型配准"))) {
		return;
	}

	setProjectdir();
	if (enableLogging) {
		logEdit->append(QString::fromLocal8Bit("****模型配准****"));
	}
	IntersectCmdPaths.clear();
	// 初始化任务队列和相关参数 
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;
	//m_totalTasks = SatelliteImagefilename.size();

	if (SatelliteImagefilename.empty())
	{
		intersectCmdMutex.unlock();
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, OutputPath + QString::fromLocal8Bit("任务: 求交函数输入影像数据为空!"));
		emit FunctionAbnormalExit(OutputPath + QString::fromLocal8Bit("任务: 求交函数输入影像数据为空!"));
		return;
	}
	int index = 0;
	for (auto data : SatelliteImagefilename) {
		// 创建 QDomDocument 
		QDomDocument doc;

		// 添加 XML 声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <XQIntersectCmd>
		QDomElement root = doc.createElement("XQIntersectCmd");

		// 创建 <TargetFile> 子节点 
		QDomElement TargetFile = doc.createElement("TargetFile");
		QDomText TargetFilePathText = doc.createTextNode(data);
		TargetFile.appendChild(TargetFilePathText);
		root.appendChild(TargetFile);

		for (auto data1 : Referencefilename) {
			// 创建 <InputFile> 子节点 
			QDomElement InputFile = doc.createElement("InputFile");
			QDomText InputFileText = doc.createTextNode(data1);
			InputFile.appendChild(InputFileText);
			root.appendChild(InputFile);
		}

		// 创建 <OutputFilePath> 子节点 
		QDomElement OutputFilePath = doc.createElement("OutputFilePath");
		QDomText OutputFilePathText = doc.createTextNode(projectdir + "/" + OutputPath  + "/TargetFile" + QString::number(index) + QString::fromLocal8Bit("_") + str + ".txt");
		OutputFilePath.appendChild(OutputFilePathText);
		root.appendChild(OutputFilePath);

		// 创建 <ExactIntersection> 子节点 
		QDomElement ExactIntersection = doc.createElement("ExactIntersection");
		QDomText ExactIntersectionText = doc.createTextNode(QVariant(true).toString());
		ExactIntersection.appendChild(ExactIntersectionText);
		root.appendChild(ExactIntersection);

		doc.appendChild(root);

		// 写入文件 
		QString xmlPath = (projectdir + OutputPath + "/" + QString::fromLocal8Bit("PSIntersectCmd_") + QString::number(index) + QString::fromLocal8Bit("_") + str + QString::fromLocal8Bit(".xml"));

		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			intersectCmdMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, OutputPath + QString::fromLocal8Bit("任务: 求交函数任务单创建失败!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("求交函数任务单创建失败!"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);
		file.close();

		// 填充任务信息 
		TaskInfo task;
		task.taskFilePath = xmlPath;
		//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSIntersectCmd.exe");
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSIntersectCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSIntersectCmd.exe"));
#endif
		task.outfilename = projectdir + OutputPath + "/" + "TargetFile" + QString::number(index) + QString::fromLocal8Bit("_") + str + ".txt";
		m_totalTasks++;
		m_taskQueue.enqueue(task);

		index++;
	}
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, OutputPath + QString::fromLocal8Bit("任务: 求交函数开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	if (enableLogging) {
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	}

	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****模型配准"), enableLogging);
}

/**
 * @brief  控制点匹配求交函数
 */
void SystemConfig::colIntersectCmd(QStringList DOMReferencefilename, QStringList DEMReferencefilename, QStringList SatelliteImagefilename, QString OutputPath, bool enableLogging) {
	if (!tryLockFunction(colintersectCmdMutex, QString::fromLocal8Bit("已有控制点匹配求交"))) {
		return;
	}
	setProjectdir();

	// 初始化任务队列和相关参数 
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_totalTasks = 0;

	//if (SatelliteImagefilename.empty())
	//{
	//	colintersectCmdMutex.unlock();
	//	ModelMutex.unlock();
	//	QMessageBox::information(this, u8"控制点匹配", u8"待处理影像为空");
	//	return;
	//}
	//if (DOMReferencefilename.empty())
	//{
	//	QMessageBox::information(this, u8"控制点匹配", u8"待处理参考DOM影像为空");
	//	return;
	//}

	//if (DEMReferencefilename.empty())
	//{
	//	colintersectCmdMutex.unlock();
	//	QMessageBox::information(this, u8"控制点匹配", u8"待处理DEM参考影像为空");
	//	return;
	//}

	for (auto data : SatelliteImagefilename) {
		// 创建 QDomDocument 
		QDomDocument doc;

		// 添加 XML 声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <XQIntersectCmd>
		QDomElement root = doc.createElement("XQIntersectCmd");

		// 创建 <TargetFile> 子节点 
		QDomElement TargetFile = doc.createElement("TargetFile");
		QDomText TargetFilePathText = doc.createTextNode(data);
		TargetFile.appendChild(TargetFilePathText);
		root.appendChild(TargetFile);

		for (auto data1 : DOMReferencefilename) {
			// 创建 <InputFile> 子节点 
			QDomElement InputFile = doc.createElement("InputFile");
			QDomText InputFileText = doc.createTextNode(data1);
			InputFile.appendChild(InputFileText);
			root.appendChild(InputFile);
		}
		QFileInfo fileinfo(data);
		// 创建 <OutputFilePath> 子节点 
		QDomElement OutputFilePath = doc.createElement("OutputFilePath");
		QDomText OutputFilePathText = doc.createTextNode(projectdir + OutputPath + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_DOM") + ".txt");
		OutputFilePath.appendChild(OutputFilePathText);
		root.appendChild(OutputFilePath);

		// 创建 <ExactIntersection> 子节点 
		QDomElement ExactIntersection = doc.createElement("ExactIntersection");
		QDomText ExactIntersectionText = doc.createTextNode(QVariant(true).toString());
		ExactIntersection.appendChild(ExactIntersectionText);
		root.appendChild(ExactIntersection);

		doc.appendChild(root);

		// 写入文件 
		QString xmlPath = (projectdir + OutputPath + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_DOM") + QString::fromLocal8Bit(".xml"));

		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			colintersectCmdMutex.unlock();
			ModelMutex.unlock();
			//QMessageBox::information(this, u8"控制点匹配", u8"求交算子任务单打开失败!");
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配任务: 求交任务单创建失败!") + xmlPath);
			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配求交任务单创建失败!"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);
		file.close();

		// 填充任务信息 
		TaskInfo task;
		task.taskFilePath = xmlPath;
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSIntersectCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSIntersectCmd.exe"));
#endif
		task.outfilename = projectdir + OutputPath + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_DEM") + ".txt";
		m_totalTasks++;
		m_taskQueue.enqueue(task);
	}

	for (auto data : SatelliteImagefilename) {
		// 创建 QDomDocument 
		QDomDocument doc;

		// 添加 XML 声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <XQIntersectCmd>
		QDomElement root = doc.createElement("XQIntersectCmd");

		// 创建 <TargetFile> 子节点 
		QDomElement TargetFile = doc.createElement("TargetFile");
		QDomText TargetFilePathText = doc.createTextNode(data);
		TargetFile.appendChild(TargetFilePathText);
		root.appendChild(TargetFile);

		for (auto data1 : DEMReferencefilename) {
			// 创建 <InputFile> 子节点 
			QDomElement InputFile = doc.createElement("InputFile");
			QDomText InputFileText = doc.createTextNode(data1);
			InputFile.appendChild(InputFileText);
			root.appendChild(InputFile);
		}
		QFileInfo fileinfo(data);
		// 创建 <OutputFilePath> 子节点 
		QDomElement OutputFilePath = doc.createElement("OutputFilePath");
		QDomText OutputFilePathText = doc.createTextNode(projectdir + OutputPath + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_DEM") + ".txt");
		OutputFilePath.appendChild(OutputFilePathText);
		root.appendChild(OutputFilePath);

		// 创建 <ExactIntersection> 子节点 
		QDomElement ExactIntersection = doc.createElement("ExactIntersection");
		QDomText ExactIntersectionText = doc.createTextNode(QVariant(true).toString());
		ExactIntersection.appendChild(ExactIntersectionText);
		root.appendChild(ExactIntersection);

		doc.appendChild(root);

		// 写入文件 
		QString xmlPath = (projectdir + OutputPath + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_DEM") + QString::fromLocal8Bit(".xml"));

		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			colintersectCmdMutex.unlock();
			//QMessageBox::information(this, u8"控制点匹配", u8"求交算子任务单打开失败!");
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配任务: 求交任务单创建失败!") + xmlPath);
			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配求交任务单创建失败!"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);
		file.close();

		// 填充任务信息 
		TaskInfo task;
		task.taskFilePath = xmlPath;
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSIntersectCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSIntersectCmd.exe"));
#endif
		task.outfilename = projectdir + OutputPath + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_DEM") + ".txt";
		m_totalTasks++;
		m_taskQueue.enqueue(task);
	}

	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****控制点匹配求交"), false);
}

bool SystemConfig::readAndValidatePathsFile(const QString& filePath, QStringList& paths) {
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		//qDebug() << u8"无法打开文件:" << filePath;
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("求交成果文件解析失败") + filePath);
		return false;
	}

	QTextStream in(&file);
	QString firstLine = in.readLine().trimmed();

	// 检查第一行是否为0或空 
	if (firstLine.isEmpty() || firstLine == "0") {
		file.close();
		return false;
	}

	// 第一行为路径数量
	bool ok;
	int pathCount = firstLine.toInt(&ok);
	if (!ok || pathCount <= 0) {
		file.close();
		return false;
	}

	// 读取后续路径行 
	paths.clear();
	for (int i = 0; i < pathCount; ++i) {
		QString path = in.readLine().trimmed();
		if (!path.isEmpty()) {
			paths.append(path);
		}
	}

	file.close();

	// 验证读取到的路径数量是否匹配 
	if (paths.size() != pathCount) {
		//qDebug() << u8"文件" << filePath << u8"中的路径数量不匹配，声明为" << pathCount << u8"实际为" << paths.size();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, filePath + QString::fromLocal8Bit("中的路径数量不匹配，声明为") + QString::number(pathCount) + QString::fromLocal8Bit("实际为:") + paths.size());
		return false;
	}

	return true;
}

QVector<ImageDatacol> SystemConfig::colparallelProcessSatelliteImages(const QStringList& satelliteImagePaths, const QString& precisionCheckDir) {
	QVector<ImageDatacol> result;
	QMutex resultMutex;
	QElapsedTimer timer;
	timer.start();

	QtConcurrent::blockingMap(satelliteImagePaths, [&](const QString& imagePath) {
		QFileInfo info(imagePath);
		QString baseName = info.completeBaseName();

		QString demFilePath = precisionCheckDir + "/" + baseName + "_DEM.txt";
		QStringList demPaths;
		if (!readAndValidatePathsFile(demFilePath, demPaths)) {
			//return;
		}

		QString satFilePath = precisionCheckDir + "/" + baseName + "_DOM.txt";
		QStringList satPaths;
		if (!readAndValidatePathsFile(satFilePath, satPaths)) {
			return;
		}

		ImageDatacol data;
		data.imagePath = imagePath;
		data.demPaths = demPaths;
		data.satPaths = satPaths;

		QMutexLocker locker(&resultMutex);
		result.append(data);
	});

	//qDebug() << u8"并行处理完成，耗时:" << timer.elapsed() << u8"毫秒";
	return result;
}

/**
 * @brief 控制点匹配
 */
void SystemConfig::controlPointsMatch(QStringList DEMReferencefilename,
	QStringList DOMReferencefilename,
	QStringList SatelliteImagefilename,
	QMap<QString, bool>& controlPointsPath)
{
	// 1. 加锁检查 
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有控制点匹配"))) {
		return;
	}
	logEdit->append(QString::fromLocal8Bit("****控制点匹配****"));

	if (DEMReferencefilename.empty()) {
		ModelMutex.unlock();
		//logEdit->append(QString::fromLocal8Bit("****工程中没有参考DEM数据，请确认"));
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配error: 工程中没有参考DEM数据"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配时,工程中没有找到参考DEM数据，请确认"));
		return;
	}

	if (DOMReferencefilename.empty()) {
		ModelMutex.unlock();
		//logEdit->append(QString::fromLocal8Bit("****工程中没有参考DOM数据，请确认"));
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配error: 工程中没有参考DOM数据"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("工程中没有参考DOM数据，请确认"));
		return;
	}

	QStringList filteredFiles = filterSatelliteImagesByColumn(SatelliteImagefilename, 11);

	if (filteredFiles.empty()) {
		ModelMutex.unlock();
		//logEdit->append(QString::fromLocal8Bit("****工程中没有原始影像数据，请确认"));
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配error: 工程中没有原始影像数据"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("工程中没有原始影像数据，请确认"));
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("影像匹配"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}

	QObject::disconnect(this, &SystemConfig::colintersectCmdFinished, nullptr, nullptr);

	// 2. 建立新连接 
	QObject::connect(this, &SystemConfig::colintersectCmdFinished, this, [this, filteredFiles, &controlPointsPath]() {
		QObject::disconnect(this, &SystemConfig::colintersectCmdFinished, nullptr, nullptr);

		executeMainMatching(filteredFiles, controlPointsPath);
	});
	colIntersectCmd(DOMReferencefilename, DEMReferencefilename, SatelliteImagefilename, QString::fromLocal8Bit("CtlPointMatch"), false);
}

/**
 * @brief 执行主匹配逻辑
 */
void SystemConfig::executeMainMatching(const QStringList& SatelliteImagefilename,
	QMap<QString, bool>& controlPointsPath)
{
	setProjectdir();

	m_colIntersectData = colparallelProcessSatelliteImages(SatelliteImagefilename, projectdir + QString::fromLocal8Bit("CtlPointMatch"));

	int BlkSizeXnum = ui.lineEdit_10->text().toInt();
	int BlkSizeYnum = ui.lineEdit_11->text().toInt();
	int numOfBlksX = ui.lineEdit_6->text().toInt();
	int numOfBlksY = ui.lineEdit_7->text().toInt();
	int searchRANGEX = ui.lineEdit_8->text().toInt();
	int searchRANGEY = ui.lineEdit_9->text().toInt();
	int BandIndexwait = ui.imageMatch_waitMatchDataBandBox->currentIndex();
	int BandIndexreference = ui.imageMatch_referenceDataBandBox->currentIndex();
	int currentIndex = ui.imageMatch_controlMatchModeBox->currentIndex();
	bool Highdensitypoint = ui.checkBox_4->isChecked();
	bool tickPoint = ui.checkBox_5->isChecked();
	bool SpecKeyMatchRegion = ui.checkBox_11->isChecked();
	double tickPointThrehold = ui.lineEdit_22->text().toDouble();
	QString KeyMatchRegionPath = ui.lineEdit_23->text();

	if (currentIndex == -1 || BandIndexwait == -1 || BandIndexreference == -1)
	{
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成控制点系统配置!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成控制点系统配置!!!"));
		ModelMutex.unlock();
		return;
	}

	// 初始化成员变量
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime();
	m_totalTasks = 0; // 保存总任务数
	controlPointsPath.clear();

	if (SatelliteImagefilename.empty())
	{
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配输入原始影像数据为空!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配输入原始影像数据为空!"));
		return;
	}

	// 确定操作程序名称
	QString operatorName;
	int tmpindex = currentIndex;
#ifdef Q_OS_LINUX 
	switch (tmpindex) {
	case 0: case 1:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		break;
	case 2:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		currentIndex = 4;
		break;
	case 3:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		currentIndex = 5;
		break;
	case 4:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		currentIndex = 6;
		break;
	default:
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成控制点系统配置!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成控制点系统配置!!!"));
		return;
	}
#else
	switch (tmpindex) {
	case 0: case 1:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		break;
	case 2:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		currentIndex = 4;
		break;
	case 3:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		currentIndex = 5;
		break;
	case 4:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		currentIndex = 6;
		break;
	default:
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成控制点系统配置!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成控制点系统配置!!!"));
		return;
	}
#endif
	// 定义 task 文件夹的完整路径
	QString taskFolderPath = projectdir + "CtlPointMatch/task";

	// 检查 task 文件夹是否存在，不存在则创建 
	QDir dir(taskFolderPath);
	if (!dir.exists()) {
		bool created = dir.mkpath(".");
		if (created) {
			//qDebug() << "task 文件夹创建成功：" << taskFolderPath;
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: task 文件夹创建成功"));
		}
		else {
			//qWarning() << "无法创建 task 文件夹：" << taskFolderPath;
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: 无法创建 task 文件夹"));
		}
	}

	PROJECT_LOG_INFO(m_imagePS->CurrentConfig,
		QString::fromLocal8Bit("开始处理控制点匹配数据，共%1组数据").arg(m_colIntersectData.size()));

	for (int i = 0; i < m_colIntersectData.size(); ++i) {
		const ImageDatacol& data = m_colIntersectData[i];
		PROJECT_LOG_DEBUG(m_imagePS->CurrentConfig,
			QString::fromLocal8Bit("第%1组数据 - 影像路径: %2").arg(i + 1).arg(data.imagePath));

		for (int j = 0; j < data.demPaths.size(); ++j) {
			PROJECT_LOG_DEBUG(m_imagePS->CurrentConfig,
				QString::fromLocal8Bit("  DEM路径%1: %2").arg(j + 1).arg(data.demPaths[j]));
		}

		for (int j = 0; j < data.satPaths.size(); ++j) {
			PROJECT_LOG_DEBUG(m_imagePS->CurrentConfig,
				QString::fromLocal8Bit("  DOM路径%1: %2").arg(j + 1).arg(data.satPaths[j]));
		}
	}

	int index = 0;
	try {
		for (const ImageDatacol& data : m_colIntersectData)
		{
			try {
				QFileInfo fileInfo(data.imagePath);
				if (!fileInfo.exists()) {
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,
						QString::fromLocal8Bit("影像文件不存在: %1").arg(data.imagePath));
					continue;
				}

				for (auto satPathdata : data.satPaths)
				{
					try {
						QFileInfo fileInfo_sat(satPathdata);
						if (!fileInfo_sat.exists()) {
							PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,
								QString::fromLocal8Bit("DOM文件不存在: %1").arg(satPathdata));
							continue;
						}

						QString outfilenametask = (projectdir + "CtlPointMatch/task" + "/" + fileInfo.completeBaseName() +
							QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@task") + QString::number(index)
							+ QString::fromLocal8Bit(".txt"));
						QString outfilename = (projectdir + "CtlPointMatch" + "/" + fileInfo.completeBaseName()
							+ QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@")
							+ QString::number(index) + QString::fromLocal8Bit(".txt"));

						// 记录任务文件信息 
						PROJECT_LOG_DEBUG(m_imagePS->CurrentConfig,
							QString::fromLocal8Bit("创建任务文件: %1").arg(outfilenametask));

						QFile filetask(outfilenametask);
						if (!filetask.open(QIODevice::WriteOnly | QIODevice::Text)) {
							ModelMutex.unlock();
							//QMessageBox::information(this, u8"控制点匹配", u8"控制点匹配任务单创建失败!");
							PROJECT_LOG_DEBUG(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: 任务单创建失败!") + outfilenametask);
							return;
						}

						QTextStream out(&filetask);
						out << "tag: XQIMAGE MATCH V1\n";
						out << QString::fromLocal8Bit("blkSize:%1 %2\n").arg(BlkSizeXnum).arg(BlkSizeYnum);
						out << QString::fromLocal8Bit("numOfBlks:%1 %2\n").arg(numOfBlksX).arg(numOfBlksY);
						out << QString::fromLocal8Bit("searchRANGE:%1 %2\n").arg(searchRANGEX).arg(searchRANGEY);
						out << QString::fromLocal8Bit("matchMethod:%1\n").arg(currentIndex);
						out << QString::fromLocal8Bit("BandIndex:%1 %2\n").arg(BandIndexwait).arg(BandIndexreference);
						out << QString::fromLocal8Bit("confidence:%1\n").arg(0.0, 0, 'f', 6);
						out << QString::fromLocal8Bit("generateImp:%1\n").arg("false");
						out << QString::fromLocal8Bit("wallisFilter:%1\n").arg("true");
						out << QString::fromLocal8Bit("tickPoint:%1\n").arg(QVariant(tickPoint).toString());
						out << QString::fromLocal8Bit("tickPointThrehold:%1\n").arg(tickPointThrehold);
						if (!Highdensitypoint) {
							out << QString::fromLocal8Bit("SpecPtNumPerGrid:%1\n").arg("true");
							out << QString::fromLocal8Bit("PtNumPerGrid:%1\n").arg("2");
						}
						if (SpecKeyMatchRegion) {
							out << QString::fromLocal8Bit("SpecKeyMatchRegion:%1\n").arg(QVariant(SpecKeyMatchRegion).toString());
							out << QString::fromLocal8Bit("KeyMatchRegionPath:%1\n").arg(KeyMatchRegionPath);
						}
						// 写入oriIMAGE (使用data.imagePath)
						QString nativePath = QDir::toNativeSeparators(data.imagePath);
						out << QString::fromLocal8Bit("oriIMAGE:%1\n").arg(nativePath);

						out << QString::fromLocal8Bit("outRESULT:%1\n").arg(outfilename);

						// 处理DEM路径
						if (data.demPaths.size() == 1) {
							out << QString::fromLocal8Bit("DEM:%1\n").arg(data.demPaths[0]);
						}
						else if (data.demPaths.empty()) {
							out << QString::fromLocal8Bit("DEM:%1\n").arg("");
						}
						else if (data.demPaths.size() > 1) {
							out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(data.demPaths.size());
							for (const QString& demPath : data.demPaths) {
								out << QString::fromLocal8Bit("%1\n").arg(demPath);
							}
						}

						out << QString::fromLocal8Bit("refIMAGE:%1\n").arg(satPathdata);
						filetask.close();

						PROJECT_LOG_DEBUG(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: 任务单创建成功!") + outfilenametask);
						// 填充任务信息 
						TaskInfo task;
						task.taskFilePath = outfilenametask;
						task.operatorName = operatorName;
						task.outfilename = outfilename;
						m_taskQueue.enqueue(task);
						m_totalTasks++;

						controlPointsPath.insert(outfilename, false);
						index++;
					}
					catch (const std::exception& e) {
						PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,
							QString::fromLocal8Bit("处理DOM文件时发生异常: %1, 错误: %2")
							.arg(satPathdata).arg(e.what()));
						continue;
					}
				}
			}
			catch (const std::exception& e) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,
					QString::fromLocal8Bit("处理影像数据时发生异常: %1, 错误: %2")
					.arg(data.imagePath).arg(e.what()));
				continue;
			}
		}
	}
	catch (const std::exception& e) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,
			QString::fromLocal8Bit("控制点匹配主循环发生异常: %1").arg(e.what()));
		ModelMutex.unlock();
		return;
	}
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig,
		QString::fromLocal8Bit("控制点匹配任务创建完成，共创建%1个任务").arg(index));
	if (m_taskQueue.size() == 0)
	{
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig,
			QString::fromLocal8Bit("控制点匹配任务数为0,,控制点匹配已结束"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配任务数为0,控制点匹配已结束!"));
		ModelMutex.unlock();
		return;
	}

	PublicFunctions::writeTimestampToXml("CtlPointMatch", m_imagePS->projectdir);
	writeMapToXml(controlPointsPath, QString::fromLocal8Bit("controlPointsPath"));

	// 检查输出文件是否已存在
	QVector<QString> existingFiles;
	for (const auto& outfilename : controlPointsPath.keys()) {
		if (QFile::exists(outfilename)) {
			existingFiles.append(outfilename);
		}
	}

	if (!existingFiles.isEmpty()) {
		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("控制点匹配"),
			QString::fromLocal8Bit("已经存在%1个匹配成果，是否重新匹配?").arg(existingFiles.size()),
			QMessageBox::Yes | QMessageBox::No);

		int ret = (reply == QMessageBox::Yes);

		//timer.stop();

		if (!ret) {
			// 用户选择不重新处理 

			// 删除重复的任务
			QQueue<TaskInfo> filteredQueue;
			for (const auto& task : m_taskQueue) {
				if (!existingFiles.contains(task.outfilename)) {
					filteredQueue.enqueue(task);
				}
			}
			m_taskQueue = filteredQueue;

			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: 用户选择不覆盖已有成果, 剩余未处理任务数量为: ") + QString::number(m_taskQueue.size()));

			// 更新总任务数 
			m_totalTasks = m_taskQueue.size();

			// 如果所有文件都重复且用户选择不重新处理，则直接退出
			if (m_taskQueue.isEmpty()) {
				ModelMutex.unlock();
				emit controlPointsMatchFinished();
				return;
			}
		}
	}

	m_startTime = QDateTime::currentDateTime();
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****控制点匹配"));
}


///**
// * @brief 控制点匹配
// */
//void SystemConfig::controlPointsMatch(QStringList DEMReferencefilename,
//	QStringList DOMReferencefilename,
//	QStringList SatelliteImagefilename,
//	QMap<QString, bool>& controlPointsPath)
//{
//	// 1. 加锁检查 
//	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有控制点匹配"))) {
//		return;
//	}
//	logEdit->append(QString::fromLocal8Bit("****控制点匹配****"));
//
//	if (DEMReferencefilename.empty()){
//		ModelMutex.unlock();
//		//logEdit->append(QString::fromLocal8Bit("****工程中没有参考DEM数据，请确认"));
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,  QString::fromLocal8Bit("控制点匹配error: 工程中没有参考DEM数据"));
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配时,工程中没有找到参考DEM数据，请确认"));
//		return;
//	}
//
//	if (DOMReferencefilename.empty()) {
//		ModelMutex.unlock();
//		//logEdit->append(QString::fromLocal8Bit("****工程中没有参考DOM数据，请确认"));
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配error: 工程中没有参考DOM数据"));
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("工程中没有参考DOM数据，请确认"));
//		return;
//	}
//	
//	QStringList filteredFiles = filterSatelliteImagesByColumn(SatelliteImagefilename, 11);
//
//	if (filteredFiles.empty()) {
//		ModelMutex.unlock();
//		//logEdit->append(QString::fromLocal8Bit("****工程中没有原始影像数据，请确认"));
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配error: 工程中没有原始影像数据"));
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("工程中没有原始影像数据，请确认"));
//		return;
//	}
//
//	if (!m_imagePS->getWorkProcessflag()) {
//		// 使用封装后的函数显示界面并等待用户确认 
//		int result = showFunctionAndWait(QString::fromLocal8Bit("影像匹配"));
//
//		// 用户取消操作
//		if (result != QDialog::Accepted) {
//			ModelMutex.unlock();
//			return;
//		}
//	}
//
//	m_connectionId++;
//	if (m_intersectCmdConnection) {
//		disconnect(m_intersectCmdConnection);
//		m_intersectCmdConnection = QMetaObject::Connection(); // 重置为无效连接 
//	}
//	if (m_allintersectCmdConnection) {
//		disconnect(m_allintersectCmdConnection);
//		m_allintersectCmdConnection = QMetaObject::Connection(); // 重置为无效连接 
//	}
//
//	colcurrentIndex = 0; // 当前处理的图像索引 
//	//QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
//	//QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);
//		// 初始化处理 
//	if (colcurrentIndex == 0) {
//		//QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
//
//		// 建立新连接 - 每次IntersectCmd完成后处理 
//		m_intersectCmdConnection = connect(
//			this, &SystemConfig::intersectCmdFinished,
//			this, [this, DOMReferencefilename, filteredFiles, id = m_connectionId]() {
//			if (id != m_connectionId) {
//				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配error: id != m_connectionId"));
//				emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配前求交任务失败"));
//				return;
//			}
//			colcurrentIndex++;
//			if (colcurrentIndex < 2 && !DOMReferencefilename.isEmpty()) {
//				IntersectCmd(DOMReferencefilename, filteredFiles,
//					QString::fromLocal8Bit("DOM"), QString::fromLocal8Bit("CtlPointMatch"));
//			}
//			if (colcurrentIndex == 2) {
//				colcurrentIndex = 0;
//				disconnect(m_intersectCmdConnection);
//				emit allIntersectCmdsFinished();
//			}
//			
//		}
//		, Qt::UniqueConnection);
//
//		// 开始处理第一个图像
//		if (!filteredFiles.empty()) {
//			if (!DEMReferencefilename.isEmpty()) {
//				IntersectCmd(DEMReferencefilename, filteredFiles, QString::fromLocal8Bit("DEM"), QString::fromLocal8Bit("CtlPointMatch"));
//			}
//		}
//	}
//
//	//QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);
//
//	m_allintersectCmdConnection = QObject::connect(this, &SystemConfig::allIntersectCmdsFinished, this, [this, filteredFiles, &controlPointsPath, id = m_connectionId]() {
//		disconnect(m_allintersectCmdConnection);
//		if (id != m_connectionId) {
//			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配error: id != m_connectionId"));
//			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配前求交任务失败"));
//			return;
//		}
//		executeMainMatching(filteredFiles, controlPointsPath);
//	}, Qt::UniqueConnection);
//}

///**
// * @brief 执行主匹配逻辑
// */
//void SystemConfig::executeMainMatching(const QStringList& SatelliteImagefilename,
//	QMap<QString, bool>& controlPointsPath)
//{
//	setProjectdir();
//
//	int BlkSizeXnum = ui.lineEdit_10->text().toInt();
//	int BlkSizeYnum = ui.lineEdit_11->text().toInt();
//	int numOfBlksX = ui.lineEdit_6->text().toInt();
//	int numOfBlksY = ui.lineEdit_7->text().toInt();
//	int searchRANGEX = ui.lineEdit_8->text().toInt();
//	int searchRANGEY = ui.lineEdit_9->text().toInt();
//	int BandIndexwait = ui.imageMatch_waitMatchDataBandBox->currentIndex();
//	int BandIndexreference = ui.imageMatch_referenceDataBandBox->currentIndex();
//	int currentIndex = ui.imageMatch_controlMatchModeBox->currentIndex();
//	bool tickPoint = ui.checkBox_5->isChecked();
//	double tickPointThrehold = ui.lineEdit_22->text().toDouble();
//
//	if (currentIndex == -1 || BandIndexwait == -1 || BandIndexreference == -1)
//	{
//		//logEdit->append(QString::fromLocal8Bit("****请完成控制点系统配置!!!"));
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成控制点系统配置!!!"));
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成控制点系统配置!!!"));
//		ModelMutex.unlock();
//		return;
//	}
//
//	int index = 0;
//	int index1 = 0;
//	int indexDOM = 0;
//	bool firstExecution = true;
//	QDateTime startTime;
//	int totalTasks = 0;
//	static QAtomicInt completedTasks(0);
//	completedTasks.store(0);
//	controlPointsPath.clear();
//
//	if (SatelliteImagefilename.empty())
//	{
//		ModelMutex.unlock();
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配输入原始影像数据为空!"));
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配输入原始影像数据为空!"));
//		return;
//	}
//
//	// 计算总任务数   
//	for (auto data : SatelliteImagefilename) {
//		QStringList DOMpaths;
//		QString filePathDOM = (projectdir + "CtlPointMatch" + "/" + "TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DOM") + ".txt");
//		QFile fileTXTDOM(filePathDOM);
//
//		if (!fileTXTDOM.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			//qDebug() << "Failed to open file:" + filePathDOM;
//			ModelMutex.unlock();
//			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配打开求交函数结果文件失败!") + filePathDOM);
//			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配打开求交函数结果文件失败!"));
//			return;
//		}
//		QTextStream inDOM(&fileTXTDOM);
//		int lineNumberDOM = 0;
//		while (!inDOM.atEnd()) {
//			QString line = inDOM.readLine().trimmed();
//			lineNumberDOM++;
//			// 跳过第一行（数字行）
//			if (lineNumberDOM == 1) {
//				continue;
//			}
//			// 从第二行开始读取路径
//			if (!line.isEmpty()) {
//				DOMpaths.append(line);
//			}
//		}
//		fileTXTDOM.close();
//		totalTasks += DOMpaths.size();
//
//		index1++;
//	}
//	index1 = 0;
//	// 初始化成员变量
//	m_taskQueue.clear();
//	m_runningProcesses.store(0);
//	m_completedTasks.store(0);
//	m_startTime = QDateTime();
//	m_totalTasks = totalTasks; // 保存总任务数
//
//		// 确定操作程序名称
//	QString operatorName;
//	int tmpindex = currentIndex;
//#ifdef Q_OS_LINUX 
//	switch (tmpindex) {
//	case 0: case 1:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
//		break;
//	case 2:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
//		currentIndex = 4;
//		break;
//	case 3:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
//		currentIndex = 5;
//		break;
//	case 4:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
//		currentIndex = 6;
//		break;
//	default:
//		ModelMutex.unlock();
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成控制点系统配置!!!"));
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成控制点系统配置!!!"));
//		return;
//}
//#else
//	switch (tmpindex) {
//	case 0: case 1:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
//		break;
//	case 2:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
//		currentIndex = 4;
//		break;
//	case 3:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
//		currentIndex = 5;
//		break;
//	case 4:
//		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
//		currentIndex = 6;
//		break;
//	default:
//		ModelMutex.unlock();
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成控制点系统配置!!!"));
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成控制点系统配置!!!"));
//		return;
//	}
//#endif
//
//	for (auto data : SatelliteImagefilename)
//	{
//		QStringList DEMpaths;
//		QString filePathDEM = (projectdir + "CtlPointMatch/TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DEM") + ".txt");
//		QFile fileTXTDEM(filePathDEM);
//
//		if (!fileTXTDEM.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			//qDebug() << "Failed to open file:" + filePathDEM;
//			ModelMutex.unlock();
//			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配打开求交函数结果文件失败!") + filePathDEM);
//			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配打开求交函数结果文件失败!"));
//			return;
//		}
//		QTextStream inDEM(&fileTXTDEM);
//		int lineNumberDEM = 0;
//		while (!inDEM.atEnd()) {
//			QString line = inDEM.readLine().trimmed();
//			lineNumberDEM++;
//			// 跳过第一行（数字行）
//			if (lineNumberDEM == 1) {
//				continue;
//			}
//			// 从第二行开始读取路径 
//			if (!line.isEmpty()) {
//				DEMpaths.append(line);
//			}
//		}
//		fileTXTDEM.close();
//
//		QStringList DOMpaths;
//		QString filePathDOM = (projectdir + "CtlPointMatch/TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DOM") + ".txt");
//		QFile fileTXTDOM(filePathDOM);
//
//		if (!fileTXTDOM.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			//qDebug() << "Failed to open file:" + filePathDOM;
//			ModelMutex.unlock();
//			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配打开求交函数结果文件失败!") + filePathDOM);
//			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配打开求交函数结果文件失败!"));
//			return;
//		}
//		QTextStream inDOM(&fileTXTDOM);
//		int lineNumberDOM = 0;
//		while (!inDOM.atEnd()) {
//			QString line = inDOM.readLine().trimmed();
//			lineNumberDOM++;
//			// 跳过第一行（数字行）
//			if (lineNumberDOM == 1) {
//				continue;
//			}
//			// 从第二行开始读取路径
//			if (!line.isEmpty()) {
//				DOMpaths.append(line);
//			}
//		}
//		fileTXTDOM.close();
//
//		if (DOMpaths.empty())
//		{
//			/*if (index1 + 1 == SatelliteImagefilename.size()) {*/
//				//ModelMutex.unlock();
//			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: 相交的DOM数据为空!"));
//				//emit FunctionAbnormalExit(QString::fromLocal8Bit("控制点匹配"));
//			//}
//			continue;
//		}
//
//		QFileInfo fileInfo(data);
//
//		// 定义 task 文件夹的完整路径
//		QString taskFolderPath = projectdir + "CtlPointMatch/task";
//
//		// 检查 task 文件夹是否存在，不存在则创建 
//		QDir dir(taskFolderPath);
//		if (!dir.exists()) {
//			bool created = dir.mkpath(".");
//			if (created) {
//				//qDebug() << "task 文件夹创建成功：" << taskFolderPath;
//				PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: task 文件夹创建成功"));
//			}
//			else {
//				//qWarning() << "无法创建 task 文件夹：" << taskFolderPath;
//				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: 无法创建 task 文件夹"));
//			}
//		}
//
//		for (auto DOMp : DOMpaths)
//		{
//			QString outfilenametask = (projectdir + "CtlPointMatch/task" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@task") + QString::number(indexDOM) + QString::fromLocal8Bit(".txt"));
//			QString outfilename = (projectdir + "CtlPointMatch" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@") + QString::number(index) + QString::fromLocal8Bit(".txt"));
//
//			QFile filetask(outfilenametask);
//			if (!filetask.open(QIODevice::WriteOnly | QIODevice::Text)) {
//				//qDebug() << "Failed to create task filetask:" + outfilenametask;
//				ModelMutex.unlock();
//				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建控制点匹配任务单失败") + outfilenametask);
//				emit FunctionAbnormalExit(QString::fromLocal8Bit("创建控制点匹配任务单失败"));
//				return;
//			}
//			QTextStream out(&filetask);
//
//			out << "tag: XQIMAGE MATCH V1\n";
//			out << QString::fromLocal8Bit("blkSize:%1 %2\n").arg(BlkSizeXnum).arg(BlkSizeYnum);
//			out << QString::fromLocal8Bit("numOfBlks:%1 %2\n").arg(numOfBlksX).arg(numOfBlksY);
//			out << QString::fromLocal8Bit("searchRANGE:%1 %2\n").arg(searchRANGEX).arg(searchRANGEY);
//			out << QString::fromLocal8Bit("matchMethod:%1\n").arg(currentIndex);
//			out << QString::fromLocal8Bit("BandIndex:%1 %2\n").arg(BandIndexwait).arg(BandIndexreference);
//			out << QString::fromLocal8Bit("confidence:%1\n").arg(0.0, 0, 'f', 6);
//			out << QString::fromLocal8Bit("generateImp:%1\n").arg("false");
//			out << QString::fromLocal8Bit("wallisFilter:%1\n").arg("true");
//			out << QString::fromLocal8Bit("tickPoint:%1\n").arg(QVariant(tickPoint).toString());
//			out << QString::fromLocal8Bit("tickPointThrehold:%1\n").arg(tickPointThrehold);
//			out << QString::fromLocal8Bit("SpecPtNumPerGrid:%1\n").arg("true");
//			out << QString::fromLocal8Bit("PtNumPerGrid:%1\n").arg("2");
//			QString nativePath = QDir::toNativeSeparators(data);
//			out << QString::fromLocal8Bit("oriIMAGE:%1\n").arg(nativePath);
//			out << QString::fromLocal8Bit("outRESULT:%1\n").arg(outfilename);
//			if (DEMpaths.size() == 1)
//			{
//				out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());
//				out << QString::fromLocal8Bit("%1\n").arg(DEMpaths[0]);
//			}
//			else if (DEMpaths.size() <= 0)
//				out << QString::fromLocal8Bit("DEM:%1\n").arg("");
//			else if (DEMpaths.size() > 1)
//			{
//				out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());
//
//				for (auto DEMp : DEMpaths)
//					out << QString::fromLocal8Bit("%1\n").arg(DEMp);
//			}
//
//			out << QString::fromLocal8Bit("refIMAGE:%1\n").arg(DOMp);
//			filetask.close();
//
//			// 填充任务信息 
//			TaskInfo task;
//			task.taskFilePath = outfilenametask;
//			task.operatorName = operatorName;
//			task.outfilename = outfilename;
//			m_taskQueue.enqueue(task);
//
//			controlPointsPath.insert(outfilename, false);
//			indexDOM++;
//			index++;
//		}
//		index1++;
//	}
//
//	PublicFunctions::writeTimestampToXml("CtlPointMatch");
//	writeMapToXml(controlPointsPath, QString::fromLocal8Bit("controlPointsPath"));
//
//	// 检查输出文件是否已存在
//	QVector<QString> existingFiles;
//	for (const auto& outfilename : controlPointsPath.keys()) {
//		if (QFile::exists(outfilename)) {
//			existingFiles.append(outfilename);
//		}
//	}
//
//	if (!existingFiles.isEmpty()) {
//		// 弹出提示框询问用户
//		//QMessageBox msgBox;
//		//msgBox.setWindowTitle(QString::fromLocal8Bit("控制点匹配"));
//		//msgBox.setText(QString::fromLocal8Bit(" 已经存在%1个匹配成果，是否重新匹配?").arg(existingFiles.size()));
//		//msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
//		//msgBox.setDefaultButton(QMessageBox::No);
//
//		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
//			QString::fromLocal8Bit("控制点匹配"),
//			QString::fromLocal8Bit("已经存在%1个匹配成果，是否重新匹配?").arg(existingFiles.size()),
//			QMessageBox::Yes | QMessageBox::No);
//
//		// 创建定时器，15秒后自动关闭并选择"否"
//		//QTimer timer;
//		//timer.setSingleShot(true);
//		//QObject::connect(&timer, &QTimer::timeout, [&msgBox]() {
//		//	if (msgBox.isVisible()) {
//		//		msgBox.reject();  // 相当于选择"否"
//		//	}
//		//});
//		//timer.start(15000);  // 15秒超时
//
//		int ret = (reply == QMessageBox::Yes);
//
//		//timer.stop();
//
//		if (!ret) {
//			// 用户选择不重新处理 
//
//			// 删除重复的任务
//			QQueue<TaskInfo> filteredQueue;
//			for (const auto& task : m_taskQueue) {
//				if (!existingFiles.contains(task.outfilename)) {
//					filteredQueue.enqueue(task);
//				}
//			}
//			m_taskQueue = filteredQueue;
//
//			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配: 用户选择不覆盖已有成果, 剩余未处理任务数量为: ") + QString::number(m_taskQueue.size()));
//
//			// 更新总任务数 
//			m_totalTasks = m_taskQueue.size();
//
//			// 如果所有文件都重复且用户选择不重新处理，则直接退出
//			if (m_taskQueue.isEmpty()) {
//				ModelMutex.unlock();
//				emit controlPointsMatchFinished();
//				return;
//			}
//		}
//	}
//
//	m_startTime = QDateTime::currentDateTime();
//	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
//	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
//
//	// 开始处理任务 
//	startNextTasks(QString::fromLocal8Bit("****控制点匹配"));
//}

/**
 * @brief 配准模型
 */
void SystemConfig::modelMatchByInter(QStringList DEMReferencefilename,
	QStringList DOMReferencefilename,
	QStringList SatelliteImagefilename,
	QMap<QString, bool>& cmodelMatchByIntePath)
{
	// 1. 加锁检查 
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有模型配准"))) {
		return;
	}
	logEdit->append(QString::fromLocal8Bit("****模型配准****"));

	//if (DEMReferencefilename.empty()) {
	//	logEdit->append(QString::fromLocal8Bit("****工程中没有参考DEM数据，请确认"));
	//	emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配准"));
	//	ModelMutex.unlock();
	//}

	if (DOMReferencefilename.empty()) {
		ModelMutex.unlock();
		
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准error: 工程中没有参考DOM数据"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配准时,工程中没有找到参考DOM数据，请确认"));
		return;
	}

	if (SatelliteImagefilename.empty()) {
		//logEdit->append(QString::fromLocal8Bit("****工程中没有原始影像数据，请确认"));
		ModelMutex.unlock();

		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准error: 工程中没有原始影像数据"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配准时,工程中没有原始影像数据，请确认"));
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("纠正模型配准"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	m_connectionId++;
	if (m_intersectCmdConnection) {
		disconnect(m_intersectCmdConnection);
		m_intersectCmdConnection = QMetaObject::Connection(); // 重置为无效连接 
	}
	if (m_allintersectCmdConnection) {
		disconnect(m_allintersectCmdConnection);
		m_allintersectCmdConnection = QMetaObject::Connection(); // 重置为无效连接 
	}

	colcurrentIndex = 0; // 当前处理的图像索引 
	//QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
	//QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);
		// 初始化处理 
	if (colcurrentIndex == 0) {
		//QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);

		// 建立新连接 - 每次IntersectCmd完成后处理 
		m_intersectCmdConnection = connect(
			this, &SystemConfig::intersectCmdFinished,
			this, [this, DOMReferencefilename, SatelliteImagefilename, id = m_connectionId]() {
			if (id != m_connectionId) return;
			colcurrentIndex++;
			if (colcurrentIndex < 2 && !DOMReferencefilename.isEmpty()) {
				IntersectCmd(DOMReferencefilename, SatelliteImagefilename,
					QString::fromLocal8Bit("DOM"), QString::fromLocal8Bit("RegisteOrtho/Register"));
			}
			if (colcurrentIndex == 2) {
				colcurrentIndex = 0;
				disconnect(m_intersectCmdConnection);
				emit allIntersectCmdsFinished();
			}

		}
		, Qt::UniqueConnection);

		// 开始处理第一个图像
		if (!SatelliteImagefilename.empty()) {
			if (!DEMReferencefilename.isEmpty()) {
				IntersectCmd(DEMReferencefilename, SatelliteImagefilename, QString::fromLocal8Bit("DEM"), QString::fromLocal8Bit("RegisteOrtho/Register"));
			}
			else {
				emit intersectCmdFinished();
			}
		}
	}

	//QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);

	m_allintersectCmdConnection = QObject::connect(this, &SystemConfig::allIntersectCmdsFinished, this, [this, SatelliteImagefilename, &cmodelMatchByIntePath, id = m_connectionId]() {
		disconnect(m_allintersectCmdConnection);
		if (id != m_connectionId) return;
		exemodelMatchByInter(SatelliteImagefilename, cmodelMatchByIntePath);
	}, Qt::UniqueConnection);
}

/**
 * @brief 执行模型配准主匹配逻辑
 */
void SystemConfig::exemodelMatchByInter(const QStringList& SatelliteImagefilename,
	QMap<QString, bool>& cmodelMatchByIntePath)
{
	setProjectdir();

	int BlkSizeXnum = ui.lineEdit_10->text().toInt();
	int BlkSizeYnum = ui.lineEdit_11->text().toInt();
	int numOfBlksX = ui.lineEdit_6->text().toInt();
	int numOfBlksY = ui.lineEdit_7->text().toInt();
	int searchRANGEX = ui.lineEdit_8->text().toInt();
	int searchRANGEY = ui.lineEdit_9->text().toInt();
	int BandIndexwait = ui.imageMatch_waitMatchDataBandBox->currentIndex();
	int BandIndexreference = ui.imageMatch_referenceDataBandBox->currentIndex();
	int CorrectcurrentIndex = ui.matchCorrect_changeModelBox->currentIndex();
	int currentIndex = ui.imageMatch_controlMatchModeBox->currentIndex();
	bool tickPoint = ui.checkBox_34->isChecked();
	double tickPointThrehold = ui.lineEdit_30->text().toDouble();

	if (currentIndex == -1 || BandIndexwait == -1 || BandIndexreference == -1)
	{
		ModelMutex.unlock();

		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成模型配准系统配置!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成模型配准系统配置!!!"));
		return;
	}

	int index = 0;
	int index1 = 0;
	int indexDOM = 0;
	bool firstExecution = true;
	QDateTime startTime;
	int totalTasks = 0;
	cmodelMatchByIntePath.clear();

	if (SatelliteImagefilename.empty())
	{
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准输入原始影像数据为空!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配准输入原始影像数据为空!"));
		return;
	}

	// 计算总任务数   
	for (auto data : SatelliteImagefilename) {
		QStringList DOMpaths;
		QString filePathDOM = (projectdir + "RegisteOrtho/Register" + "/" + "TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DOM") + ".txt");
		QFile fileTXTDOM(filePathDOM);

		if (!fileTXTDOM.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open file:" + filePathDOM;
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准输入原始影像数据为空!") + filePathDOM);
			emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配准输入原始影像数据为空!"));
			return;
		}
		QTextStream inDOM(&fileTXTDOM);
		int lineNumberDOM = 0;
		while (!inDOM.atEnd()) {
			QString line = inDOM.readLine().trimmed();
			lineNumberDOM++;
			// 跳过第一行（数字行）
			if (lineNumberDOM == 1) {
				continue;
			}
			// 从第二行开始读取路径
			if (!line.isEmpty()) {
				DOMpaths.append(line);
			}
		}
		fileTXTDOM.close();
		totalTasks += DOMpaths.size();

		index1++;
	}
	index1 = 0;
	// 初始化成员变量
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime();
	m_totalTasks = totalTasks; // 保存总任务数

		// 确定操作程序名称
	QString operatorName;
	int tmpindex = currentIndex;
#ifdef Q_OS_LINUX 
	switch (tmpindex) {
	case 0: case 1:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		break;
	case 2:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		currentIndex = 4;
		break;
	case 3:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		currentIndex = 5;
		break;
	case 4:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/linux64/PSGcpMatchMM.x");
		currentIndex = 6;
		break;
	default:
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成模型配准系统配置!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成模型配准系统配置!!!"));
		return;
	}
#else
	switch (tmpindex) {
	case 0: case 1:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		break;
	case 2:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		currentIndex = 4;
		break;
	case 3:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		currentIndex = 5;
		break;
	case 4:
		operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
		currentIndex = 6;
		break;
	default:
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成模型配准系统配置!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成模型配准系统配置!!!"));
		return;
	}
#endif

	for (auto data : SatelliteImagefilename)
	{
		QStringList DEMpaths;
		QString filePathDEM = (projectdir + "RegisteOrtho/Register/TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DEM") + ".txt");
		QFile fileTXTDEM(filePathDEM);

		if (!fileTXTDEM.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open file:" + filePathDEM;
			//ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准打开DEM求交函数结果文件失败 ") + filePathDEM);
			//emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配准打开求交函数结果文件失败"));
			//return;
		}
		QTextStream inDEM(&fileTXTDEM);
		int lineNumberDEM = 0;
		while (!inDEM.atEnd()) {
			QString line = inDEM.readLine().trimmed();
			lineNumberDEM++;
			// 跳过第一行（数字行）
			if (lineNumberDEM == 1) {
				continue;
			}
			// 从第二行开始读取路径 
			if (!line.isEmpty()) {
				DEMpaths.append(line);
			}
		}
		fileTXTDEM.close();

		QStringList DOMpaths;
		QString filePathDOM = (projectdir + "RegisteOrtho/Register/TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DOM") + ".txt");
		QFile fileTXTDOM(filePathDOM);

		if (!fileTXTDOM.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open file:" + filePathDOM;
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准打开求交函数结果文件失败 ") + filePathDOM);
			emit FunctionAbnormalExit(QString::fromLocal8Bit("模型配准打开求交函数结果文件失败"));
			return;
		}
		QTextStream inDOM(&fileTXTDOM);
		int lineNumberDOM = 0;
		while (!inDOM.atEnd()) {
			QString line = inDOM.readLine().trimmed();
			lineNumberDOM++;
			// 跳过第一行（数字行）
			if (lineNumberDOM == 1) {
				continue;
			}
			// 从第二行开始读取路径
			if (!line.isEmpty()) {
				DOMpaths.append(line);
			}
		}
		fileTXTDOM.close();

		if (DOMpaths.empty())
		{
			if (index1 + 1 == SatelliteImagefilename.size()) {
				PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准: 相交的DOM数据为空!"));
			}
			continue;
		}

		QFileInfo fileInfo(data);

		// 定义 task 文件夹的完整路径
		QString taskFolderPath = projectdir + "RegisteOrtho/Register/task";

		// 检查 task 文件夹是否存在，不存在则创建 
		QDir dir(taskFolderPath);
		if (!dir.exists()) {
			bool created = dir.mkpath(".");
			if (created) {
				PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准: task 文件夹创建成功"));
			}
			else {
				PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准: 无法创建 task 文件夹"));
			}
		}
		if (ui.checkBox_29->isChecked()){
			QString outfilenametask = (projectdir + "RegisteOrtho/Register/task" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@task") + QString::number(indexDOM) + QString::fromLocal8Bit(".txt"));
			QString outfilename = (projectdir + "RegisteOrtho/Register" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@") + QString::number(index) + QString::fromLocal8Bit(".txt"));

			QFile filetask(outfilenametask);
			if (!filetask.open(QIODevice::WriteOnly | QIODevice::Text)) {
				//qDebug() << "Failed to create task filetask:" + outfilenametask;
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建模型配准任务单失败") + outfilenametask);
				emit FunctionAbnormalExit(QString::fromLocal8Bit("创建模型配准任务单失败"));
				return;
			}
			QTextStream out(&filetask);

			out << "tag: XQIMAGE MATCH V1\n";
			out << QString::fromLocal8Bit("blkSize:%1 %2\n").arg(BlkSizeXnum).arg(BlkSizeYnum);
			out << QString::fromLocal8Bit("numOfBlks:%1 %2\n").arg(numOfBlksX).arg(numOfBlksY);
			out << QString::fromLocal8Bit("searchRANGE:%1 %2\n").arg(searchRANGEX).arg(searchRANGEY);
			out << QString::fromLocal8Bit("matchMethod:%1\n").arg(currentIndex);
			out << QString::fromLocal8Bit("BandIndex:%1 %2\n").arg(BandIndexwait).arg(BandIndexreference);
			out << QString::fromLocal8Bit("confidence:%1\n").arg(0.0, 0, 'f', 6);
			out << QString::fromLocal8Bit("generateImp:%1\n").arg("true");
			out << QString::fromLocal8Bit("wallisFilter:%1\n").arg("true");
			out << QString::fromLocal8Bit("ImpWarpMode:%1\n").arg(CorrectcurrentIndex);
			out << QString::fromLocal8Bit("tickPoint:%1\n").arg(QVariant(tickPoint).toString());
			out << QString::fromLocal8Bit("tickPointThrehold:%1\n").arg(tickPointThrehold, 0, 'f', 6);
			out << QString::fromLocal8Bit("strictMatch:%1\n").arg("false");
			QString nativePath = QDir::toNativeSeparators(data);
			out << QString::fromLocal8Bit("oriIMAGE:%1\n").arg(nativePath);
			out << QString::fromLocal8Bit("outRESULT:%1\n").arg(outfilename);
			if (DEMpaths.size() == 1)
			{
				out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());
				out << QString::fromLocal8Bit("%1\n").arg(DEMpaths[0]);
			}
			else if (DEMpaths.size() <= 0)
				out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg("0");
			else if (DEMpaths.size() > 1)
			{
				out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());

				for (auto DEMp : DEMpaths)
					out << QString::fromLocal8Bit("%1\n").arg(DEMp);
			}

			out << QString::fromLocal8Bit("numOfRefImages:%1\n").arg(DOMpaths.size());
			for (auto DOMp : DOMpaths)
				out << QString::fromLocal8Bit("%1\n").arg(DOMp);
			
			filetask.close();

			// 填充任务信息 
			TaskInfo task;
			task.taskFilePath = outfilenametask;
			task.operatorName = operatorName;
			task.outfilename = outfilename;
			m_taskQueue.enqueue(task);

			cmodelMatchByIntePath.insert(outfilename, false);
		}
		else {
			for (auto DOMp : DOMpaths)
			{
				QString outfilenametask = (projectdir + "RegisteOrtho/Register/task" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@task") + QString::number(indexDOM) + QString::fromLocal8Bit(".txt"));
				QString outfilename = (projectdir + "RegisteOrtho/Register" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@") + QString::number(index) + QString::fromLocal8Bit(".txt"));

				QFile filetask(outfilenametask);
				if (!filetask.open(QIODevice::WriteOnly | QIODevice::Text)) {
					//qDebug() << "Failed to create task filetask:" + outfilenametask;
					ModelMutex.unlock();
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建模型配准任务单失败") + outfilenametask);
					emit FunctionAbnormalExit(QString::fromLocal8Bit("创建模型配准任务单失败"));
					return;
				}
				QTextStream out(&filetask);

				out << "tag: XQIMAGE MATCH V1\n";
				out << QString::fromLocal8Bit("blkSize:%1 %2\n").arg(BlkSizeXnum).arg(BlkSizeYnum);
				out << QString::fromLocal8Bit("numOfBlks:%1 %2\n").arg(numOfBlksX).arg(numOfBlksY);
				out << QString::fromLocal8Bit("searchRANGE:%1 %2\n").arg(searchRANGEX).arg(searchRANGEY);
				out << QString::fromLocal8Bit("matchMethod:%1\n").arg(currentIndex);
				out << QString::fromLocal8Bit("BandIndex:%1 %2\n").arg(BandIndexwait).arg(BandIndexreference);
				out << QString::fromLocal8Bit("confidence:%1\n").arg(0.0, 0, 'f', 6);
				out << QString::fromLocal8Bit("generateImp:%1\n").arg("false");
				out << QString::fromLocal8Bit("wallisFilter:%1\n").arg("true");
				out << QString::fromLocal8Bit("ImpWarpMode:%1\n").arg(CorrectcurrentIndex);
				out << QString::fromLocal8Bit("tickPoint:%1\n").arg(QVariant(tickPoint).toString());
				out << QString::fromLocal8Bit("tickPointThrehold:%1\n").arg(tickPointThrehold, 0, 'f', 6);
				out << QString::fromLocal8Bit("strictMatch:%1\n").arg("false");
				QString nativePath = QDir::toNativeSeparators(data);
				out << QString::fromLocal8Bit("oriIMAGE:%1\n").arg(nativePath);
				out << QString::fromLocal8Bit("outRESULT:%1\n").arg(outfilename);
				if (DEMpaths.size() == 1)
				{
					out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());
					out << QString::fromLocal8Bit("%1\n").arg(DEMpaths[0]);
				}
				else if (DEMpaths.size() <= 0)
					out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg("0");
				else if (DEMpaths.size() > 1)
				{
					out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());

					for (auto DEMp : DEMpaths)
						out << QString::fromLocal8Bit("%1\n").arg(DEMp);
				}

				out << QString::fromLocal8Bit("refIMAGE:%1\n").arg(DOMp);
				filetask.close();

				// 填充任务信息 
				TaskInfo task;
				task.taskFilePath = outfilenametask;
				task.operatorName = operatorName;
				task.outfilename = outfilename;
				m_taskQueue.enqueue(task);

				cmodelMatchByIntePath.insert(outfilename, false);
				indexDOM++;
				index++;
			}
		}
		index1++;
	}

	// 检查输出文件是否已存在
	QVector<QString> existingFiles;
	for (const auto& outfilename : cmodelMatchByIntePath.keys()) {
		if (QFile::exists(outfilename)) {
			existingFiles.append(outfilename);
		}
	}

	if (!existingFiles.isEmpty()) {

		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("模型配准"),
			QString::fromLocal8Bit("已经存在%1个配准成果，是否重新配准?").arg(existingFiles.size()),
			QMessageBox::Yes | QMessageBox::No);

		int ret = (reply == QMessageBox::Yes);

		//timer.stop();

		if (!ret) {
			// 用户选择不重新处理 
			logEdit->append(QString::fromLocal8Bit(" 用户取消配准已存在的成果"));
			// 删除重复的任务
			QQueue<TaskInfo> filteredQueue;
			for (const auto& task : m_taskQueue) {
				if (!existingFiles.contains(task.outfilename)) {
					filteredQueue.enqueue(task);
				}
			}
			m_taskQueue = filteredQueue;
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型配准: 用户选择不覆盖已有成果, 剩余未处理任务数量为: ") + QString::number(m_taskQueue.size()));

			// 更新总任务数 
			m_totalTasks = m_taskQueue.size();

			// 如果所有文件都重复且用户选择不重新处理，则直接退出
			if (m_taskQueue.isEmpty()) {
				ModelMutex.unlock();
				emit modelMatchByInterFinished();
				return;
			}
		}
	}
	m_startTime = QDateTime::currentDateTime();
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("配准模型开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****配准模型"));
}

// 通用的任务处理函数   
void SystemConfig::startNextTasks(QString title, bool enableLogging)
{
	// 如果是匀色任务且勾选了output8bImage_checkBox
	bool isColorCorrection = (title == QString::fromLocal8Bit("****影像匀色"));
	bool shouldConvertTo8Bit = ui.output8bImage_checkBox->isChecked() && isColorCorrection;

	QProgressBar* progressBar = m_imagePS->getProgressBar();
	m_maxConcurrentProcesses = ui.runEnviron_taskBox->currentText().toInt();
	// 初始化进度条
	if (!m_taskQueue.isEmpty() && progressBar && !m_progressBarInitialized && !ConvertTo8Bitflag) {
		if (shouldConvertTo8Bit) {
			// 匀色占50%，真彩色转换占50%
			progressBar->setRange(0, m_totalTasks * 2);
		}
		else {
			progressBar->setRange(0, m_totalTasks);
		}
		//progressBar->setRange(0, m_totalTasks);
		progressBar->setValue(0);
		progressBar->setVisible(true);
		//progressBar->show();
		m_progressBarInitialized = true;
		if(title == QString::fromLocal8Bit("****模型配准"))
			m_systemProcessesFlag = true;
		else
			m_systemProcessesFlag = false;
	}
	isFromFinishedSignal = false;
	if (isFromFinishedSignal) return;
	isFromFinishedSignal = true;
	while (!m_taskQueue.isEmpty() && m_runningProcesses.load() < m_maxConcurrentProcesses) {
		TaskInfo task = m_taskQueue.dequeue();

		QProcess* process = new QProcess(this);
		QMetaObject::Connection conn = connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[this, process, task, title, enableLogging, progressBar, shouldConvertTo8Bit](int exitCode, QProcess::ExitStatus status) {
			//isFromFinishedSignal = true;
			m_runningProcesses.fetchAndSubOrdered(1);
			m_completedTasks.fetchAndAddOrdered(1);
			//// 更新进度条
			//if (progressBar) {
			//	progressBar->setValue(m_completedTasks.load());
			//}
			if (ConvertTo8Bitflag) {
				progressBar->setValue(m_totalTasks + m_completedTasks.load()); // 设置到50%位置 
			}
			else {
				progressBar->setValue(m_completedTasks.load());
			}

			// 检查.moni文件是否存在
			QString errorstr = task.taskFilePath;
			QFileInfo taskFileInfo(task.taskFilePath);
			QString moniFilePath = taskFileInfo.absolutePath() + "/" + taskFileInfo.completeBaseName() + ".moni";
			if (enableLogging && title == QString::fromLocal8Bit("****连接点匹配"))
			{
				errorstr = task.outfilename;
				moniFilePath = taskFileInfo.absolutePath() + "/" + errorstr + ".moni";
			}
			QFile moniFile(moniFilePath);

			if (!moniFile.exists()) {
				if (enableLogging && title != QString::fromLocal8Bit("****影像云检")) {
					logEdit->append(QString::fromLocal8Bit("任务未完成: %1").arg(errorstr));
					m_systemProcesses.removeOne(process);
					//enableLogging = false;
					process->deleteLater();
					//m_taskQueue.clear();
					//return;
					if(title == QString::fromLocal8Bit("****控制点匹配"))
						m_taskQueue_col.enqueue(task);
				}
			}

			//if (enableLogging) {
			//	logEdit->append(QString::fromLocal8Bit("已完成任务：%1/%2")
			//		.arg(m_completedTasks.load())
			//		.arg(m_totalTasks));
			//}
			if (ConvertTo8Bitflag) {
				if (enableLogging) {
					logEdit->append(QString::fromLocal8Bit("已完成任务：%1/%2")
						.arg(m_totalTasks + m_completedTasks.load())
						.arg(m_totalTasks * 2));
				}
			}
			else if (!ConvertTo8Bitflag && shouldConvertTo8Bit) {
				if (enableLogging) {
					logEdit->append(QString::fromLocal8Bit("已完成任务：%1/%2")
						.arg(m_completedTasks.load())
						.arg(m_totalTasks * 2));
				}
			}
			else
			{
				if (enableLogging) {
					logEdit->append(QString::fromLocal8Bit("已完成任务：%1/%2")
						.arg(m_completedTasks.load())
						.arg(m_totalTasks));
				}
			}
			if (title == QString::fromLocal8Bit("****影像融合"))
			{
				if (!imageInterPath.contains(task.outfilename)) {
					imageInterPath.insert(task.outfilename, false);
				}
			}
			else if (title == QString::fromLocal8Bit("****模型配准"))
			{
				if (!IntersectCmdPaths.contains(task.outfilename)) {
					IntersectCmdPaths.push_back(task.outfilename);
				}
			}
			else if (title == QString::fromLocal8Bit("****创建缩略图"))
			{
				if (!ThumbnailGenerationPaths.contains(task.outfilename)) {
					ThumbnailGenerationPaths.push_back(task.outfilename);
				}
			}
			else if (title == QString::fromLocal8Bit("****影像云检"))
			{
				if (!CloudDetectionOutPaths.contains(task.outfilename)) {
					CloudDetectionOutPaths.push_back(task.outfilename);
				}
			}

			// 所有任务完成后的处理
			if (m_completedTasks.load() == m_totalTasks) {

				m_taskQueue.clear();
				m_runningProcesses.store(0);
				m_completedTasks.store(0);
				m_totalTasks = 0;

				if (shouldConvertTo8Bit && !ConvertTo8Bitflag) {
					QStringList outputFiles;
					QDir dodgingDir(projectdir + "Dodging/tmp/");
					QStringList filters = { "*.tif", "*.img", "*.pix","*.tiff" };
					QFileInfoList fileList = dodgingDir.entryInfoList(filters, QDir::Files);

					for (const QFileInfo& file : fileList) {
						outputFiles.append(file.absoluteFilePath());
					}
					ConvertTo8Bitflag = true;
					// 设置进度条范围（后50%）
					progressBar->setRange(0, m_totalTasks * 2);
					progressBar->setValue(m_totalTasks);
					ModelMutex.unlock();
					PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("开始匀色后真彩色转换... "));

					// 启动真彩色转换 
					TrueColorConversion(outputFiles);

					// 直接返回，不发送完成信号
					return;
				}
				QDateTime endTime = QDateTime::currentDateTime();

				PROJECT_LOG_INFO(m_imagePS->CurrentConfig, title + QString::fromLocal8Bit("结束时间: ") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

				if (enableLogging && m_taskQueue_col.empty()) {
					logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
					if (title == QString::fromLocal8Bit("****配准模型")){
						logEdit->append(QString::fromLocal8Bit("****模型配准") + QString::fromLocal8Bit("总耗时：") + calculateTimeDifference(m_startTime, endTime));
					}
					else {
						logEdit->append(title + QString::fromLocal8Bit("总耗时：") + calculateTimeDifference(m_startTime, endTime));
					}
				}

				m_progressBarInitialized = false;
				progressBar->setVisible(false);
				m_completedTasks.store(0);
				m_runningProcesses.store(0);
				if (title == QString::fromLocal8Bit("****控制点匹配")){
					if (m_taskQueue_col.empty()) {
						ModelMutex.unlock();
						emit controlPointsMatchFinished();
					}
					else {
						m_coltotalTasks = m_taskQueue_col.size();
						logEdit->append(QString::fromLocal8Bit("失败任务：%1个,开始重新执行")
							.arg(m_coltotalTasks));
						QDateTime tmpTime = QDateTime::currentDateTime();
						PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("失败任务：%1个,开始重新执行: ").arg(m_coltotalTasks) + tmpTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
						
						startNextTaskcols(QString::fromLocal8Bit("****控制点匹配"));
					}
					return;
				}
				else if (title == QString::fromLocal8Bit("****配准模型")){
					ModelMutex.unlock();
					emit modelMatchByInterFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****连接点匹配")){
					ModelMutex.unlock();
					emit EncryptedPointsMatchFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****金字塔创建")){
					ModelMutex.unlock();
					emit CreatepyramidFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****正射纠正")){
					// 清理.ba文件
					for (auto data : m_orthoSatelliteImages) {
						QFileInfo fileInfo(data);
						QDir directory(fileInfo.absolutePath());
						QStringList baFiles = directory.entryList({ "*.ba", "*.BA" }, QDir::Files);
						for (const QString &file : baFiles) {
							QFile::remove(directory.absoluteFilePath(file));
						}
					}
					ModelMutex.unlock();
					emit orthorectificationFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****配准纠正")){
					// 清理.ba文件
					for (auto data : m_orthoSatelliteImages) {
						QFileInfo fileInfo(data);
						QDir directory(fileInfo.absolutePath());
						QStringList baFiles = directory.entryList({ "*.ba", "*.BA" }, QDir::Files);
						for (const QString &file : baFiles) {
							QFile::remove(directory.absoluteFilePath(file));
						}
					}
					ModelMutex.unlock();
					emit AlignmentAdjustmentFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****影像融合")){
					writeMapToXml(imageInterPath, QString::fromLocal8Bit("imageInterPath"));
					ModelMutex.unlock();
					emit imageInterActionFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****真彩色转换")){
					ModelMutex.unlock();
					emit TrueColorConversionFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****影像匀色")){
					ModelMutex.unlock();
					ConvertTo8Bitflag = false;
					QString outputDir = projectdir + "Dodging/tmp";

					// 删除目录及其所有内容 
					QDir dir(outputDir);
					if (dir.exists()) {
						bool success = dir.removeRecursively();
						if (success) {
							qDebug() << "目录删除成功:" << outputDir;
						}
						else {
							qDebug() << "目录不存在:" << outputDir;
						}
					}
					emit ImageColorCorrectionFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****模型配准")){
					m_taskQueue.clear();
					if (intersectCmdMutex.tryLock()) {
						intersectCmdMutex.unlock();  
					}
					else {
						intersectCmdMutex.unlock();   
					}
					emit intersectCmdFinished();  
					return;
				}
				else if (title == QString::fromLocal8Bit("****控制点匹配求交")){
					m_taskQueue.clear();
					if (colintersectCmdMutex.tryLock()) {
						colintersectCmdMutex.unlock();
					}
					else {
						colintersectCmdMutex.unlock();
					}
					emit colintersectCmdFinished(); 
					return;
				}
				else if (title == QString::fromLocal8Bit("****智能镶嵌")){
					ModelMutex.unlock();
					emit SmartMosaicFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****绝对定位精度质检")){
					ModelMutex.unlock();
					emit ImageAbsPositPrecCheckFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****创建缩略图")){
					ModelMutex.unlock();
					emit ThumbnailGenerationFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****影像云检")){
					ModelMutex.unlock();
					emit CloudDetectionFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****投影转换")){
					ModelMutex.unlock();
					emit ProjectionTransformationFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****格式转换")){
					ModelMutex.unlock();
					emit FormatConversionFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****影像裁切")){
					ModelMutex.unlock();
					emit ImageCropFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****SAR影像滤波")){
					ModelMutex.unlock();
					emit SARImageFilterFinished();
					return;
				}
				else if (title == QString::fromLocal8Bit("****影像范围计算")){
					//ModelMutex.unlock();
					emit ImageRangeFinished();
					return;
				}
			}
			m_systemProcesses.removeOne(process);
			process->deleteLater();

				// 仅在需要时触发新任务检查
			if (m_runningProcesses.load() < m_maxConcurrentProcesses && !m_taskQueue.isEmpty()) {
				QMetaObject::invokeMethod(this, "startNextTasks",
					Qt::QueuedConnection,
					Q_ARG(QString, title),
					Q_ARG(bool, enableLogging));
			}
		});
		m_systemProcessesConn.append(conn);
		m_runningProcesses.fetchAndAddOrdered(1);
		m_systemProcesses.append(process);
		if (title == QString::fromLocal8Bit("****影像云检"))
		{
			process->start(task.operatorName, { task.taskFilePath, task.taskFilePath2 });
		}
		else if(title == QString::fromLocal8Bit("****连接点匹配"))
		{
			process->start(task.operatorName, { task.taskFilePath, task.taskFilePath2, task.taskFilePath3});
		}
		else
		{
			process->start(task.operatorName, { task.taskFilePath });
		}
	}
	isFromFinishedSignal = false;
}

//控制点匹配失败处理函数
void SystemConfig::startNextTaskcols(QString title)
{
	m_maxConcurrentProcesses = ui.runEnviron_taskBox->currentText().toInt();

	while (!m_taskQueue_col.isEmpty() && m_runningProcesses.load() < m_maxConcurrentProcesses) {
		TaskInfo task = m_taskQueue_col.dequeue();
		
		QProcess* process = new QProcess(this);
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[this, process, task, title](int exitCode, QProcess::ExitStatus status) {
			m_runningProcesses.fetchAndSubOrdered(1);
			m_completedTasks.fetchAndAddOrdered(1);

			logEdit->append(QString::fromLocal8Bit("已完成任务：%1/%2")
				.arg(m_completedTasks.load())
				.arg(m_coltotalTasks));

			// 所有任务完成后的处理
			if (m_completedTasks.load() == m_coltotalTasks) {
				QDateTime endTime = QDateTime::currentDateTime();
				logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
				logEdit->append(title + QString::fromLocal8Bit("总耗时：") + calculateTimeDifference(m_startTime, endTime));
				
				m_completedTasks.store(0);
				if (title == QString::fromLocal8Bit("****控制点匹配")) {
					ModelMutex.unlock();
					PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点匹配失败任务二次执行结束... ").arg(m_coltotalTasks) + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
					emit controlPointsMatchFinished();
				}
			
			}
			m_systemProcesses.removeOne(process);
			process->deleteLater();
			QMetaObject::invokeMethod(this, "startNextTaskcols", Qt::QueuedConnection,
				Q_ARG(QString, title));
		});

		m_runningProcesses.fetchAndAddOrdered(1);
		m_systemProcesses.append(process);

		process->start(task.operatorName, { task.taskFilePath });
	}
}

// 影像范围计算的任务处理函数（支持动态更新任务总数）
void SystemConfig::startNextTasksIRange(QString title, bool enableLogging)
{
	QProgressBar* progressBar = m_imagePS->getProgressBar_imageinfo();
	m_maxConcurrentProcesses = ui.runEnviron_taskBox->currentText().toInt();

	// 初始化或更新进度条（每次有新任务时都更新最大值）
	if (!m_taskQueueIR.isEmpty() && progressBar) {
		// 计算当前总任务数 = 已完成 + 进行中 + 队列中
		int currentTotalTasks = m_completedTasksIR.load() + m_runningProcessesIR.load() + m_taskQueueIR.size();

		// 只有当任务总数变化时才更新进度条范围
		if (currentTotalTasks != progressBar->maximum() || !m_progressBarInitializedIR) {
			progressBar->setRange(0, currentTotalTasks);
			progressBar->setValue(m_completedTasksIR.load());
			progressBar->setVisible(true);
			m_progressBarInitializedIR = true;
		}
	}

	// 防止重入
	if (isFromFinishedSignalIR) return;
	isFromFinishedSignalIR = true;

	// 启动新任务 
	while (!m_taskQueueIR.isEmpty() && m_runningProcessesIR.load() < m_maxConcurrentProcesses) {
		TaskInfo task = m_taskQueueIR.dequeue();

		QProcess* process = new QProcess(this);
		QMetaObject::Connection conn = connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[this, process, task, title, enableLogging, progressBar](int exitCode, QProcess::ExitStatus status) {
			m_runningProcessesIR.fetchAndSubOrdered(1);
			m_completedTasksIR.fetchAndAddOrdered(1);

			// 更新进度条（考虑可能新增的任务）
			if (progressBar) {
				// 计算当前总任务数 
				int currentTotal = m_completedTasksIR.load() + m_runningProcessesIR.load() + m_taskQueueIR.size();

				// 如果任务总数变化，更新范围
				if (currentTotal != progressBar->maximum()) {
					progressBar->setRange(0, currentTotal);
				}

				progressBar->setValue(m_completedTasksIR.load());
			}

			// 检查.moni文件是否存在
			QString errorstr = task.taskFilePath;
			QFileInfo taskFileInfo(task.taskFilePath);
			QString moniFilePath = taskFileInfo.absolutePath() + "/" + taskFileInfo.completeBaseName() + ".moni";

			QFile moniFile(moniFilePath);
			if (!moniFile.exists()) {
				logEdit->append(QString::fromLocal8Bit("任务未完成: %1").arg(errorstr));
			}

			if (enableLogging) {
				logEdit->append(QString::fromLocal8Bit("已完成任务：%1/%2")
					.arg(m_completedTasksIR.load())
					.arg(progressBar->maximum()));  // 显示当前总任务数 
			}

			// 所有任务完成后的处理
			if (m_taskQueueIR.isEmpty() && m_runningProcessesIR.load() == 0) {
				QDateTime endTime = QDateTime::currentDateTime();
				PROJECT_LOG_INFO(m_imagePS->CurrentConfig,
					title + QString::fromLocal8Bit("结束时间: ") +
					endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

				m_progressBarInitializedIR = false;
				progressBar->setVisible(false);
				m_completedTasksIR.store(0);
				m_runningProcessesIR.store(0);

				if (title == QString::fromLocal8Bit("****影像范围计算")) {
					m_taskQueueIR.clear();
					emit ImageRangeFinished();
					return;
				}
			}

			m_systemProcesses.removeOne(process);
			process->deleteLater();

			// 触发新任务检查 
			if (m_runningProcessesIR.load() < m_maxConcurrentProcesses && !m_taskQueueIR.isEmpty()) {
				QMetaObject::invokeMethod(this, "startNextTasksIRange",
					Qt::QueuedConnection,
					Q_ARG(QString, title),
					Q_ARG(bool, enableLogging));
			}
		});

		m_systemProcessesConn.append(conn);
		m_runningProcessesIR.fetchAndAddOrdered(1);
		m_systemProcesses.append(process);

		process->start(task.operatorName, { task.taskFilePath });
	}
	isFromFinishedSignalIR = false;
}

/**
 * @brief 自由网平差以及平差文件初始化
 */
void SystemConfig::FreeNetworkAdjust(QStringList DEMReferencefilename)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有自由网平差解算"))) {
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("卫片平差"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}
	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****自由网平差解算****"));

	QString xmlPath = projectdir + QString::fromLocal8Bit("SatBA/SatModelMakerCmd.xml");

	//创建调用可执行程序的类
	QProcess * process1 = new QProcess(this);

	QString authCmd;
#ifdef Q_OS_LINUX 
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSSatModelMakerCmd.x"));
#else
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSSatModelMakerCmd.exe"));
#endif
	//启动程序，并输入参数
	process1->start(authCmd, QStringList() << xmlPath);

	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差模型配对开始..."));
	PublicFunctions::writeTimestampToXml("FNAModelmarker", m_imagePS->projectdir);
	m_systemProcesses.append(process1);
	// 2. 进程结束时自动显示 
	connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		//logEdit->append(QString::fromLocal8Bit("****自由网任务单完成！"));
		process1->deleteLater(); // 安全释放内存
		m_systemProcesses.removeOne(process1);
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差模型配对完成,开始构建自由网平差任务单..."));

		bool IsDelTieErrorPtS = ui.checkBox_49->isChecked();
		bool IsDelGcpErrorPtS = ui.checkBox_50->isChecked();
		bool UseDemAsHeightControlS = ui.checkBox_52->isChecked();
		bool MuxAsSecondS = ui.checkBox_51->isChecked();
		bool SparseCtlBaS = ui.checkBox_53->isChecked();
		bool RdCorrectionP = ui.checkBox_84->isChecked();
		bool IsDelGcpErrorPtByGeoP = ui.checkBox_83->isChecked();
		QString DelTiePt_ThreS = ui.lineEdit_45->text();
		QString DelGcp_ThreS = ui.lineEdit_46->text();
		QString SparseCtlImgWeightS = ui.lineEdit_47->text();
		QString DelGcp_ThreXYP = ui.lineEdit_58->text();
		QString DelGcp_ThreZP = ui.lineEdit_59->text();
		int AdjustmentModelS = ui.sateImageAdjust_adjustModelBox->currentIndex();
		if (AdjustmentModelS == -1)
		{
			//logEdit->append(QString::fromLocal8Bit("****请完成自由网平差系统配置!!!"));
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成自由网平差系统配置!!!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成自由网平差系统配置!!!"));
			return;
		}

		// 创建 QDomDocument
		QDomDocument doc;

		// 添加 XML 声明
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <XQSatBA>
		QDomElement root = doc.createElement("XQSatBA");

		// 添加基本参数节点
		QDomElement isDelTieErrorPt = doc.createElement("IsDelTieErrorPt");
		isDelTieErrorPt.setAttribute("type", "bool");
		isDelTieErrorPt.appendChild(doc.createTextNode(QVariant(IsDelTieErrorPtS).toString()));
		root.appendChild(isDelTieErrorPt);

		QDomElement delTiePtThre = doc.createElement("DelTiePt_Thre");
		delTiePtThre.appendChild(doc.createTextNode(DelTiePt_ThreS));
		root.appendChild(delTiePtThre);

		QDomElement isDelGcpErrorPt = doc.createElement("IsDelGcpErrorPt");
		isDelGcpErrorPt.setAttribute("type", "bool");
		isDelGcpErrorPt.appendChild(doc.createTextNode(QVariant(IsDelGcpErrorPtS).toString()));
		root.appendChild(isDelGcpErrorPt);

		QDomElement delGcpThre = doc.createElement("DelGcp_Thre");
		delGcpThre.appendChild(doc.createTextNode(DelGcp_ThreS));
		root.appendChild(delGcpThre);

		QDomElement adjustmentModel = doc.createElement("AdjustmentModel");
		adjustmentModel.setAttribute("type", "int32");
		adjustmentModel.appendChild(doc.createTextNode(QString::number(AdjustmentModelS)));
		root.appendChild(adjustmentModel);

		QDomElement baseImgWeight = doc.createElement("BaseImgWeight");
		baseImgWeight.setAttribute("type", "float64");
		baseImgWeight.appendChild(doc.createTextNode("1.00000000"));
		root.appendChild(baseImgWeight);

		QDomElement maxNumIterations = doc.createElement("max_num_iterations");
		maxNumIterations.setAttribute("type", "int32");
		maxNumIterations.appendChild(doc.createTextNode("100"));
		root.appendChild(maxNumIterations);

		QDomElement tieFile = doc.createElement("TieFile");
		tieFile.appendChild(doc.createTextNode(projectdir + "SatBA" + "/" + "PSBundle.tie"));
		root.appendChild(tieFile);

		QDomElement gcpFile = doc.createElement("GCPfile");
		gcpFile.appendChild(doc.createTextNode(projectdir + "SatBA" + "/" + "PSBundle.gcp"));
		root.appendChild(gcpFile);

		for (auto data : DEMReferencefilename)
		{
			QDomElement dem = doc.createElement("DEM");
			QString nativePath = QDir::toNativeSeparators(data);
			dem.appendChild(doc.createTextNode(nativePath));
			root.appendChild(dem);
		}

		QDomElement useDemAsHeightControl = doc.createElement("UseDemAsHeightControl");
		useDemAsHeightControl.setAttribute("type", "bool");
		useDemAsHeightControl.appendChild(doc.createTextNode(QVariant(UseDemAsHeightControlS).toString()));
		root.appendChild(useDemAsHeightControl);

		QDomElement fastBA = doc.createElement("FastBA");
		fastBA.setAttribute("type", "bool");
		fastBA.appendChild(doc.createTextNode("false"));
		root.appendChild(fastBA);

		QDomElement outputDir = doc.createElement("OutputDir");
		outputDir.appendChild(doc.createTextNode(projectdir + "SatBA" + "/" + "NewRPCs" + "/"));
		root.appendChild(outputDir);

		QDomElement keepRpcHeader = doc.createElement("KeepRpcHeader");
		keepRpcHeader.setAttribute("type", "bool");
		keepRpcHeader.appendChild(doc.createTextNode("false"));
		root.appendChild(keepRpcHeader);

		QDomElement muxAsSecond = doc.createElement("MuxAsSecond");
		muxAsSecond.setAttribute("type", "bool");
		muxAsSecond.appendChild(doc.createTextNode(QVariant(MuxAsSecondS).toString()));
		root.appendChild(muxAsSecond);

		QDomElement sparseCtlBa = doc.createElement("SparseCtlBa");
		sparseCtlBa.setAttribute("type", "bool");
		sparseCtlBa.appendChild(doc.createTextNode(QVariant(SparseCtlBaS).toString()));
		root.appendChild(sparseCtlBa);

		QDomElement sparseCtlImgWeight = doc.createElement("SparseCtlImgWeight");
		sparseCtlImgWeight.setAttribute("type", "float64");
		sparseCtlImgWeight.appendChild(doc.createTextNode(SparseCtlImgWeightS));
		root.appendChild(sparseCtlImgWeight);

		QDomElement RdCorrection = doc.createElement("RdCorrection");
		RdCorrection.setAttribute("type", "bool");
		RdCorrection.appendChild(doc.createTextNode(QVariant(RdCorrectionP).toString()));
		root.appendChild(RdCorrection);

		QDomElement IsDelGcpErrorPtByGeo = doc.createElement("IsDelGcpErrorPtByGeo");
		IsDelGcpErrorPtByGeo.setAttribute("type", "bool");
		IsDelGcpErrorPtByGeo.appendChild(doc.createTextNode(QVariant(IsDelGcpErrorPtByGeoP).toString()));
		root.appendChild(IsDelGcpErrorPtByGeo);

		QDomElement DelGcp_ThreXY = doc.createElement("DelGcp_ThreXY");
		DelGcp_ThreXY.setAttribute("type", "float64");
		DelGcp_ThreXY.appendChild(doc.createTextNode(QVariant(DelGcp_ThreXYP).toString()));
		root.appendChild(DelGcp_ThreXY);

		QDomElement DelGcp_ThreZ = doc.createElement("DelGcp_ThreZ");
		DelGcp_ThreZ.setAttribute("type", "float64");
		DelGcp_ThreZ.appendChild(doc.createTextNode(QVariant(DelGcp_ThreZP).toString()));
		root.appendChild(DelGcp_ThreZ);

		QDomElement globalDemPath = doc.createElement("globalDemPath");
		QString exeDir = QCoreApplication::applicationDirPath();
		QDir dir(exeDir);
		QString globaldemPath = dir.absolutePath();
		globalDemPath.appendChild(doc.createTextNode(globaldemPath + QString::fromLocal8Bit("/Software/etc/globaldem/globaldem.jp2")));
		root.appendChild(globalDemPath);

		//QDateTime timestamp = PublicFunctions::readTimestampFromXml("SatTiePointMatch");
		//if (!timestamp.isValid()) {
		//	ModelMutex.unlock();
		//	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点执行时间获取无效"));

		//	emit FunctionAbnormalExit(QString::fromLocal8Bit("自由网平差获取模型配对结果失败!"));
		//	return;
		//}

		//QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("SatBA") + "/", timestamp);
		//QString tmpPath = projectdir + QString::fromLocal8Bit("SatBA/SatModelMakerCmdout.xml");
		QString SatModelMakerCmdoutPath = projectdir + QString::fromLocal8Bit("SatBA/SatModelMakerCmdout.xml");
		//QString SatModelMakerCmdoutPath = "";
		//for (auto datafile : resultFiles)
		//{
		//	if (tmpPath == datafile)
		//		SatModelMakerCmdoutPath = tmpPath;
		//}
		//if (SatModelMakerCmdoutPath == "")
		//{
		//	ModelMutex.unlock();
		//	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("平差文件夹中 没有在连接点执行时间后的SatModelMakerCmdout.xml文件"));

		//	emit FunctionAbnormalExit(QString::fromLocal8Bit("自由网平差获取模型配对结果失败!"));
		//	return;
		//}

		// 解析SatModelMakerCmdout.xml文件
		QDomDocument cmdoutDoc;
		QFile cmdoutFile(projectdir + QString::fromLocal8Bit("SatBA") + "/" + "SatModelMakerCmdout.xml");
		if (!cmdoutFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差: 打开SatModelMakerCmdout.xml文件获取模型失败..."));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("自由网平差: 打开SatModelMakerCmdout.xml文件获取模型失败..."));
			//qDebug() << "Failed to open SatModelMakerCmdout.xml";
			return;
		}
		if (!cmdoutDoc.setContent(&cmdoutFile)) {
			cmdoutFile.close();
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差: 解析SatModelMakerCmdout.xml文件获取模型失败..."));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("自由网平差: 解析SatModelMakerCmdout.xml文件获取模型失败..."));
			qDebug() << "Failed to parse SatModelMakerCmdout.xml";
			return;
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

		if (cmdoutModel.isNull())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差: 模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("自由网平差: 模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			return;
		}

		while (!cmdoutModel.isNull()) {
			QDomElement modelElement = doc.createElement(QString("Model_%1").arg(modelIndex));

			// 处理NadImage
			QDomElement nadImageName = cmdoutModel.firstChildElement("NadImageName");
			if (!nadImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("NadImageIndex").text()));
				imageElement.appendChild(imageId);
				// 将.tiff替换为.nfo
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(nadImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(nadImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("0")); // NadImage对应0
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(fwdImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(fwdImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(bwdImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(bwdImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("2")); // BwdImage对应2
				imageElement.appendChild(sensorType);
				modelElement.appendChild(imageElement);
			}

			// 处理MulImage   
			QDomElement mulImageName = cmdoutModel.firstChildElement("MulImageName");
			if (!mulImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("MulImageIndex").text()));
				imageElement.appendChild(imageId);
				// 将.tiff替换为.nfo
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(mulImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(mulImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("3")); // MulImage对应3
				imageElement.appendChild(sensorType);
				modelElement.appendChild(imageElement);
			}

			modelsElement.appendChild(modelElement);
			cmdoutModel = cmdoutModel.nextSiblingElement();
			modelIndex++;
		}

		root.appendChild(modelsElement);
		doc.appendChild(root);

		QString outputPath = projectdir + QString::fromLocal8Bit("SatBA/PSBundleCmd.xml");
		// 写入文件
		QFile file(outputPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open output file";
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差任务单创建失败..."));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("自由网平差任务单创建失败"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);  // 4 表示缩进 4 个空格，使 XML 可读性更好
		file.close();

			// 创建进程对象   
		QProcess *process = new QProcess(this);
		QDateTime startTime = QDateTime::currentDateTime();

		// 1. 立即显示开始时间（不等待）   
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		logEdit->repaint(); // 强制刷新界面

		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差开始执行:") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

		// 2. 进程结束时自动显示结束时间和耗时   
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[=](int exitCode, QProcess::ExitStatus status) {
			m_systemProcesses.removeOne(process);
			QDateTime endTime = QDateTime::currentDateTime();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差结束时间:") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

			logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			logEdit->append(QString::fromLocal8Bit("****自由网平差耗时：") + calculateTimeDifference(startTime, endTime));
			
			ModelMutex.unlock();
			emit FreeNetworkAdjustFinished();
			process->deleteLater(); // 安全释放内存
		});
		
		QString authCmd;
#ifdef Q_OS_LINUX 
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSBundleCmd.x"));
#else
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSBundleCmd.exe"));
#endif
		// 启动进程（异步方式，不阻塞界面）   
		process->start(authCmd, QStringList() << outputPath);
		m_systemProcesses.append(process);
	});
}

void SystemConfig::FreeNetworkAdjustmergeFile()
{
	setProjectdir();
	QString inputDir = projectdir + "SatTiePointMatch";  // 输入文件夹路径
	QString outputFile = projectdir + "SatBA/PSBundle.tie";  // 输出文件路径

	QStringList tieFiles;

	QDateTime timestamp = PublicFunctions::readTimestampFromXml("SatTiePointMatch", m_imagePS->projectdir);
	//if (!timestamp.isValid()) {
	//	return;
	//}

	QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/", timestamp);
	for (const QString& filePath : resultFiles) {
		QFileInfo fileInfo(filePath);
		QString suffix = fileInfo.suffix().toLower();
		QString fileName = fileInfo.fileName();

		if (fileName.endsWith(".tie", Qt::CaseInsensitive) &&
			!fileName.contains("_origin", Qt::CaseInsensitive)) {
			tieFiles << filePath;
		}
	}

	QDir dir(inputDir);
	//QStringList tieFiles = dir.entryList(QStringList() << "*.tie", QDir::Files);
	if (tieFiles.isEmpty()) {
		//qDebug() << "No .tie files found in directory:" << inputDir;
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie时未获取到连接点结果(.tie文件)"));

		return;
	}
	QFile outFile(outputFile);
	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie文件失败!"));

		//qDebug() << "Failed to open output file:" << outputFile;
		return;
	}
	QTextStream out(&outFile);
	int totalPoints = 0;
	int currentIndex = 1;  // 重新排列的序号从 1 开始
	// 第一次遍历：计算总连接点数
	for (const QString &file : tieFiles) {
		QFile inFile(dir.filePath(file));
		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie 时打开连接点成果文件失败! ") + file);

			//qDebug() << "Failed to open input file:" << file;
			continue;
		}
		QTextStream in(&inFile);
		QString firstLine = in.readLine();
		bool ok;
		int points = firstLine.toInt(&ok);
		if (ok) {
			totalPoints += points;
		}
		inFile.close();
	}
	// 写入总连接点数
	out << totalPoints << "\n";
	// 第二次遍历：合并数据并重新编号
	for (const QString &file : tieFiles) {
		QFile inFile(dir.filePath(file));
		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open input file:" << file;
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie 时打开连接点成果文件失败! ") + file);
			continue;
		}
		QTextStream in(&inFile);
		in.readLine();  // 跳过第一行（已处理）
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (line.isEmpty()) continue;
			// 分割数据（假设每行格式：序号 x y ...）
			QStringList parts = line.split(" ", Qt::SkipEmptyParts);
			if (parts.isEmpty()) continue;
			// 替换第一列为新的序号
			parts[0] = QString::number(currentIndex++);
			// 写入新行
			out << parts.join(" ") << "\n";
		}
		inFile.close();
	}
	outFile.close();
	//logEdit->append(QString::fromLocal8Bit("****序列化平差连接点文件成功！"));
}

void SystemConfig::FreeNetworkAdjustmergeFile(QString filePath, QStringList tieFilePattern)
{
	setProjectdir();
	QString inputDir = filePath + "SatTiePointMatch";  // 输入文件夹路径
	QString outputFile = filePath + "/" + "PSBundle.tie";  // 输出文件路径


	QDir dir(filePath);
	if (tieFilePattern.isEmpty()) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie时未获取到连接点结果(.tie文件)"));
		return;
	}
	QFile outFile(outputFile);
	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie文件失败!"));
		return;
	}
	QTextStream out(&outFile);
	int totalPoints = 0;
	int currentIndex = 1;  // 重新排列的序号从 1 开始
	// 第一次遍历：计算总连接点数
	for (const QString &file : tieFilePattern) {
		QFile inFile(dir.filePath(file));
		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie 时打开连接点成果文件失败! ") + file);
			continue;
		}
		QTextStream in(&inFile);
		QString firstLine = in.readLine();
		bool ok;
		int points = firstLine.toInt(&ok);
		if (ok) {
			totalPoints += points;
		}
		inFile.close();
	}
	// 写入总连接点数
	out << totalPoints << "\n";
	// 第二次遍历：合并数据并重新编号
	for (const QString &file : tieFilePattern) {
		QFile inFile(dir.filePath(file));
		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("自由网平差创建PSBundle.tie 时打开连接点成果文件失败! ") + file);
			continue;
		}
		QTextStream in(&inFile);
		in.readLine();  // 跳过第一行（已处理）
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (line.isEmpty()) continue;
			// 分割数据（假设每行格式：序号 x y ...）
			QStringList parts = line.split(" ", Qt::SkipEmptyParts);
			if (parts.isEmpty()) continue;
			// 替换第一列为新的序号
			parts[0] = QString::number(currentIndex++);
			// 写入新行
			out << parts.join(" ") << "\n";
		}
		inFile.close();
	}
	outFile.close();
	//logEdit->append(QString::fromLocal8Bit("****序列化平差连接点文件成功！"));
}

/**
 * @brief 控制网平差以及平差文件初始化
 */
void SystemConfig::controlNetAdjust(QStringList DEMReferencefilename)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有控制网平差解算"))) {
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("卫片平差"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****控制网平差解算****"));

	QString xmlPath = projectdir  + QString::fromLocal8Bit("CtlPointMatch/SatModelMakerCmd.xml");

	//创建调用可执行程序的类
	QProcess * process1 = new QProcess(this);
	QString authCmd;
#ifdef Q_OS_LINUX 
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSSatModelMakerCmd.x"));
#else
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSSatModelMakerCmd.exe"));
#endif
	//启动程序，并输入参数
	process1->start(authCmd, QStringList() << xmlPath);

	PublicFunctions::writeTimestampToXml("CTNModelmarker", m_imagePS->projectdir);

	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差模型配对开始..."));

	m_systemProcesses.append(process1);
	//进程结束时自动显示 
	connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		process1->deleteLater(); // 安全释放内存
		m_systemProcesses.removeOne(process1);
		bool IsDelTieErrorPtS = ui.checkBox_49->isChecked();
		bool IsDelGcpErrorPtS = ui.checkBox_50->isChecked();
		bool UseDemAsHeightControlS = ui.checkBox_52->isChecked();
		bool MuxAsSecondS = ui.checkBox_51->isChecked();
		bool SparseCtlBaS = ui.checkBox_53->isChecked();
		bool RdCorrectionP = ui.checkBox_84->isChecked();
		bool IsDelGcpErrorPtByGeoP = ui.checkBox_83->isChecked();
		QString DelTiePt_ThreS = ui.lineEdit_45->text();
		QString DelGcp_ThreS = ui.lineEdit_46->text();
		QString SparseCtlImgWeightS = ui.lineEdit_47->text();
		QString DelGcp_ThreXYP = ui.lineEdit_58->text();
		QString DelGcp_ThreZP = ui.lineEdit_59->text();
		int AdjustmentModelS = ui.sateImageAdjust_adjustModelBox->currentIndex();
		if (AdjustmentModelS == -1)
		{
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成控制网平差系统配置!!!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成控制网平差系统配置!!!"));
			return;
		}

		// 创建 QDomDocument
		QDomDocument doc;

		// 添加 XML 声明
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <XQSatBA>
		QDomElement root = doc.createElement("XQSatBA");

		// 添加基本参数节点
		QDomElement isDelTieErrorPt = doc.createElement("IsDelTieErrorPt");
		isDelTieErrorPt.setAttribute("type", "bool");
		isDelTieErrorPt.appendChild(doc.createTextNode(QVariant(IsDelTieErrorPtS).toString()));
		root.appendChild(isDelTieErrorPt);

		QDomElement delTiePtThre = doc.createElement("DelTiePt_Thre");
		delTiePtThre.appendChild(doc.createTextNode(DelTiePt_ThreS));
		root.appendChild(delTiePtThre);

		QDomElement isDelGcpErrorPt = doc.createElement("IsDelGcpErrorPt");
		isDelGcpErrorPt.setAttribute("type", "bool");
		isDelGcpErrorPt.appendChild(doc.createTextNode(QVariant(IsDelGcpErrorPtS).toString()));
		root.appendChild(isDelGcpErrorPt);

		QDomElement delGcpThre = doc.createElement("DelGcp_Thre");
		delGcpThre.appendChild(doc.createTextNode(DelGcp_ThreS));
		root.appendChild(delGcpThre);

		QDomElement adjustmentModel = doc.createElement("AdjustmentModel");
		adjustmentModel.setAttribute("type", "int32");
		adjustmentModel.appendChild(doc.createTextNode(QString::number(AdjustmentModelS)));
		root.appendChild(adjustmentModel);

		QDomElement baseImgWeight = doc.createElement("BaseImgWeight");
		baseImgWeight.setAttribute("type", "float64");
		baseImgWeight.appendChild(doc.createTextNode("1.00000000"));
		root.appendChild(baseImgWeight);

		QDomElement maxNumIterations = doc.createElement("max_num_iterations");
		maxNumIterations.setAttribute("type", "int32");
		maxNumIterations.appendChild(doc.createTextNode("100"));
		root.appendChild(maxNumIterations);

		QDomElement tieFile = doc.createElement("TieFile");
		tieFile.appendChild(doc.createTextNode(projectdir + "SatBA" + "/" + "PSBundle.tie"));
		root.appendChild(tieFile);

		QDomElement gcpFile = doc.createElement("GCPfile");
		gcpFile.appendChild(doc.createTextNode(projectdir + "SatBA" + "/" + "PSBundle.gcp"));
		root.appendChild(gcpFile);

		for (auto data : DEMReferencefilename)
		{
			QDomElement dem = doc.createElement("DEM");
			QString nativePath = QDir::toNativeSeparators(data);
			dem.appendChild(doc.createTextNode(nativePath));
			root.appendChild(dem);
		}

		QDomElement useDemAsHeightControl = doc.createElement("UseDemAsHeightControl");
		useDemAsHeightControl.setAttribute("type", "bool");
		useDemAsHeightControl.appendChild(doc.createTextNode(QVariant(UseDemAsHeightControlS).toString()));
		root.appendChild(useDemAsHeightControl);

		QDomElement fastBA = doc.createElement("FastBA");
		fastBA.setAttribute("type", "bool");
		fastBA.appendChild(doc.createTextNode("false"));
		root.appendChild(fastBA);

		QDomElement outputDir = doc.createElement("OutputDir");
		outputDir.appendChild(doc.createTextNode(projectdir + "SatBA/NewRPCs/"));
		root.appendChild(outputDir);

		QDomElement keepRpcHeader = doc.createElement("KeepRpcHeader");
		keepRpcHeader.setAttribute("type", "bool");
		keepRpcHeader.appendChild(doc.createTextNode("false"));
		root.appendChild(keepRpcHeader);

		QDomElement muxAsSecond = doc.createElement("MuxAsSecond");
		muxAsSecond.setAttribute("type", "bool");
		muxAsSecond.appendChild(doc.createTextNode(QVariant(MuxAsSecondS).toString()));
		root.appendChild(muxAsSecond);

		QDomElement sparseCtlBa = doc.createElement("SparseCtlBa");
		sparseCtlBa.setAttribute("type", "bool");
		sparseCtlBa.appendChild(doc.createTextNode(QVariant(SparseCtlBaS).toString()));
		root.appendChild(sparseCtlBa);

		QDomElement sparseCtlImgWeight = doc.createElement("SparseCtlImgWeight");
		sparseCtlImgWeight.setAttribute("type", "float64");
		sparseCtlImgWeight.appendChild(doc.createTextNode(SparseCtlImgWeightS));
		root.appendChild(sparseCtlImgWeight);

		QDomElement RdCorrection = doc.createElement("RdCorrection");
		RdCorrection.setAttribute("type", "bool");
		RdCorrection.appendChild(doc.createTextNode(QVariant(RdCorrectionP).toString()));
		root.appendChild(RdCorrection);

		QDomElement IsDelGcpErrorPtByGeo = doc.createElement("IsDelGcpErrorPtByGeo");
		IsDelGcpErrorPtByGeo.setAttribute("type", "bool");
		IsDelGcpErrorPtByGeo.appendChild(doc.createTextNode(QVariant(IsDelGcpErrorPtByGeoP).toString()));
		root.appendChild(IsDelGcpErrorPtByGeo);

		QDomElement DelGcp_ThreXY = doc.createElement("DelGcp_ThreXY");
		DelGcp_ThreXY.setAttribute("type", "float64");
		DelGcp_ThreXY.appendChild(doc.createTextNode(QVariant(DelGcp_ThreXYP).toString()));
		root.appendChild(DelGcp_ThreXY);

		QDomElement DelGcp_ThreZ = doc.createElement("DelGcp_ThreZ");
		DelGcp_ThreZ.setAttribute("type", "float64");
		DelGcp_ThreZ.appendChild(doc.createTextNode(QVariant(DelGcp_ThreZP).toString()));
		root.appendChild(DelGcp_ThreZ);

		QDomElement globalDemPath = doc.createElement("globalDemPath");
		QString exeDir = QCoreApplication::applicationDirPath();
		QDir dir(exeDir);
		QString globaldemPath = dir.absolutePath();
		globalDemPath.appendChild(doc.createTextNode(globaldemPath + QString::fromLocal8Bit("/Software/etc/globaldem/globaldem.jp2")));
		root.appendChild(globalDemPath);

		//QDateTime timestamp = PublicFunctions::readTimestampFromXml("CtlPointMatch");
		//if (!timestamp.isValid()) {
		//	ModelMutex.unlock();
		//	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制点执行时间获取无效"));

		//	emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差获取模型配对结果失败!"));
		//	return;
		//}

		//QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("CtlPointMatch") + "/", timestamp);
		//QString tmpPath = projectdir + QString::fromLocal8Bit("CtlPointMatch/SatModelMakerCmdout.xml");
		QString SatModelMakerCmdoutPath = projectdir + QString::fromLocal8Bit("CtlPointMatch/SatModelMakerCmdout.xml");
		//QString SatModelMakerCmdoutPath = "";
		//for (auto datafile : resultFiles)
		//{
		//	if (tmpPath == datafile)
		//		SatModelMakerCmdoutPath = tmpPath;
		//}
		//if (SatModelMakerCmdoutPath == "")
		//{
		//	ModelMutex.unlock();
		//	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("平差文件夹中 没有在控制点执行时间后的SatModelMakerCmdout.xml文件"));

		//	emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差获取模型配对结果失败!"));
		//	return;
		//}

		// 解析SatModelMakerCmdout.xml文件
		QDomDocument cmdoutDoc;
		QFile cmdoutFile(projectdir + QString::fromLocal8Bit("CtlPointMatch/SatModelMakerCmdout.xml"));
		if (!cmdoutFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open SatModelMakerCmdout.xml";
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 打开SatModelMakerCmdout.xml文件获取模型失败..."));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差: 打开SatModelMakerCmdout.xml文件获取模型失败..."));
			return;
		}
		if (!cmdoutDoc.setContent(&cmdoutFile)) {
			cmdoutFile.close();
			//qDebug() << "Failed to parse SatModelMakerCmdout.xml";
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 解析SatModelMakerCmdout.xml文件获取模型失败..."));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差: 解析SatModelMakerCmdout.xml文件获取模型失败..."));
			qDebug() << "Failed to parse SatModelMakerCmdout.xml";
			return;
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

		if (cmdoutModel.isNull())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差: 模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			return;
		}

		while (!cmdoutModel.isNull()) {
			QDomElement modelElement = doc.createElement(QString("Model_%1").arg(modelIndex));

			// 处理NadImage
			QDomElement nadImageName = cmdoutModel.firstChildElement("NadImageName");
			if (!nadImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("NadImageIndex").text()));
				imageElement.appendChild(imageId);
				// 将.tiff替换为.nfo
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(nadImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(nadImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("0")); // NadImage对应0
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(fwdImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(fwdImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(bwdImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(bwdImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("2")); // BwdImage对应2
				imageElement.appendChild(sensorType);
				modelElement.appendChild(imageElement);
			}

			// 处理MulImage   
			QDomElement mulImageName = cmdoutModel.firstChildElement("MulImageName");
			if (!mulImageName.isNull()) {
				QDomElement imageElement = doc.createElement("Image");
				QDomElement imageId = doc.createElement("ImageID");
				imageId.setAttribute("type", "int32");
				imageId.appendChild(doc.createTextNode(cmdoutModel.firstChildElement("MulImageIndex").text()));
				imageElement.appendChild(imageId);
				// 将.tiff替换为.nfo
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
				QDomElement isErrImg = doc.createElement("IsErrImg");
				isErrImg.setAttribute("type", "bool");
				isErrImg.appendChild(doc.createTextNode(QVariant(!m_imagePS->getIsCtrlIMG(mulImagePath, 4)).toString()));
				imageElement.appendChild(isErrImg);
				QDomElement isCtrlIMG = doc.createElement("IsCtrlIMG");
				isCtrlIMG.setAttribute("type", "bool");
				isCtrlIMG.appendChild(doc.createTextNode(QVariant(m_imagePS->getIsCtrlIMG(mulImagePath, 5)).toString()));
				imageElement.appendChild(isCtrlIMG);
				QDomElement sensorType = doc.createElement("SensorType");
				sensorType.setAttribute("type", "int32");
				sensorType.appendChild(doc.createTextNode("3")); // MulImage对应3
				imageElement.appendChild(sensorType);
				modelElement.appendChild(imageElement);
			}

			modelsElement.appendChild(modelElement);
			cmdoutModel = cmdoutModel.nextSiblingElement();
			modelIndex++;
		}

		root.appendChild(modelsElement);
		doc.appendChild(root);

		QString outputPath = projectdir + QString::fromLocal8Bit("SatBA/PSBundleCmd.xml");
		// 写入文件
		QFile file(outputPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open output file";
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差任务单创建失败..."));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);  // 4 表示缩进 4 个空格，使 XML 可读性更好
		file.close();
		QMap<QString, bool> tmpcontrolPointsPath = m_imagePS->getcolfileValue();
		controlNetAdjustmergeFile(tmpcontrolPointsPath);
		// 创建进程对象   
		QProcess *process = new QProcess(this);
		QDateTime startTime = QDateTime::currentDateTime();

		// 1. 立即显示开始时间（不等待）   
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		logEdit->repaint(); // 强制刷新界面

		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差开始执行:") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		// 2. 进程结束时自动显示结束时间和耗时   
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[=](int exitCode, QProcess::ExitStatus status) {
			m_systemProcesses.removeOne(process);
			QDateTime endTime = QDateTime::currentDateTime();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差结束时间:") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

			logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			logEdit->append(QString::fromLocal8Bit("****控制网平差耗时：") + calculateTimeDifference(startTime, endTime));
			ModelMutex.unlock();
			emit controlNetAdjustFinished();
			process->deleteLater(); // 安全释放内存
		});

		QString authCmd;
#ifdef Q_OS_LINUX 
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSBundleCmd.x"));
#else
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSBundleCmd.exe"));
#endif
		// 启动进程（异步方式，不阻塞界面）   
		process->start(authCmd, QStringList() << outputPath);
		m_systemProcesses.append(process);
	});
}

//void SystemConfig::controlNetAdjustmergeFile(QMap<QString, bool> controlPointsPath)
//{
//	setProjectdir();
//	QString outputFile = projectdir + "SatBA" + "/" + "PSBundle.gcp";    // 输出文件路径 
//
//	 // 1. 修改后的XML解析逻辑
//	QMap<QString, int> imageInfoToIdMap;
//	QString psBundleCmdFile = projectdir + "SatBA" + "/" + "PSBundleCmd.xml";
//	QFile xmlFile(psBundleCmdFile);
//	if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//		QDomDocument doc;
//		if (doc.setContent(&xmlFile)) {
//			QDomElement root = doc.documentElement();
//			QDomElement modelsElement = root.elementsByTagName("Models").at(0).toElement();
//
//			// 获取所有Model节点
//			QDomNodeList modelNodes = modelsElement.childNodes();
//			for (int m = 0; m < modelNodes.size(); ++m) {
//				QDomNode modelNode = modelNodes.at(m);
//				if (modelNode.isElement() && modelNode.nodeName().startsWith("Model_")) {
//					QDomNodeList images = modelNode.toElement().elementsByTagName("Image");
//					for (int i = 0; i < images.size(); ++i) {
//						QDomElement image = images.at(i).toElement();
//						QString imageInfoFile = image.elementsByTagName("ImageInfoFile").at(0).toElement().text();
//						int imageId = image.elementsByTagName("ImageID").at(0).toElement().text().toInt();
//						imageInfoToIdMap[imageInfoFile] = imageId;
//					}
//				}
//			}
//		}
//		xmlFile.close();
//	}
//	else {
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 打开PSBundleCmd.xml文件失败"));
//		//qDebug() << "Failed to open PSBundleCmd.xml  file:" << psBundleCmdFile;
//	}
//
//	QFile outFile(outputFile);
//	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
//		//qDebug() << "Failed to open output file:" << outputFile;
//		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 创建PSBundle.gcp文件失败"));
//		return;
//	}
//	QTextStream out(&outFile);
//
//	if (controlPointsPath.empty())
//		return;
//	if (m_imagePS->m_visibleControlPointIds.empty())
//	{
//		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 控制点数量为0"));
//
//		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差: 控制点数量为0"));
//		return;
//	}
//
//	int totalPoints = 0;
//	int currentIndex = 0;  
//
//	// 第一次遍历：计算总控制点数 
//	for (auto data = controlPointsPath.begin(); data != controlPointsPath.end(); ++data)
//	{
//		QFile inFile(data.key());
//		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 打开控制点成果文件失败") + data.key());
//			//qDebug() << "Failed to open input file:" << data.key();
//			continue;
//		}
//		QTextStream in(&inFile);
//		QString firstLine = in.readLine();
//		bool ok;
//		int points = firstLine.toInt(&ok);
//		if (ok) {
//			totalPoints += points;
//		}
//		inFile.close();
//	}
//
//	// 写入总控制点数 
//	out << totalPoints << "\n";
//
//	for (auto data = controlPointsPath.begin(); data != controlPointsPath.end(); ++data)
//	{
//		QFile inFile(data.key());
//		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 打开控制点成果文件失败") + data.key());
//
//			//qDebug() << "Failed to open input file:" << data.key();
//			continue;
//		}
//		QTextStream in(&inFile);
//		in.readLine();
//		while (!in.atEnd()) {
//			QString line = in.readLine().trimmed();
//			if (line.isEmpty())   continue;
//
//			QStringList parts = line.split(" ", Qt::SkipEmptyParts);
//			if (parts.isEmpty())   continue;
//
//			parts[0] = QString::number(currentIndex);
//
//			// 获取当前控制点文件对应的 ImageID 
//			int imageId = -1;
//			QFileInfo fileInfo(data.key());
//			QString baseName = fileInfo.completeBaseName();
//			QString result = baseName.section('_', 0, -2);
//
//			// 在映射中查找对应的 ImageID 
//			for (auto it = imageInfoToIdMap.begin(); it != imageInfoToIdMap.end(); ++it) {
//				QFileInfo fileInfotmp(it.key());
//				if(fileInfotmp.completeBaseName() == result){
//				//if (it.key().contains(result)) {
//					imageId = it.value();
//					break;
//				}
//			}
//			parts.removeAt(4);
//			parts.insert(1, m_imagePS->m_visibleControlPointIds[currentIndex]);
//			parts.insert(2, QString::number(imageId)); // 插入 ImageID 
//			parts[8] = "1.000";
//			parts[9] = "1";
//			currentIndex++;
//
//			out << parts.join("   ") << "\n";
//		}
//		inFile.close();
//	}
//	outFile.close();
//	logEdit->append(QString::fromLocal8Bit("****序列化平差控制点文件成功！"));
//}

void SystemConfig::controlNetAdjustmergeFile(QMap<QString, bool> controlPointsPath)
{
	setProjectdir();
	QString outputFile = projectdir + "SatBA" + "/" + "PSBundle.gcp";    // 输出文件路径 

	// 1. 修改后的XML解析逻辑
	QMap<QString, int> imageInfoToIdMap;
	QString psBundleCmdFile = projectdir + "SatBA" + "/" + "PSBundleCmd.xml";
	QFile xmlFile(psBundleCmdFile);
	if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QDomDocument doc;
		if (doc.setContent(&xmlFile)) {
			QDomElement root = doc.documentElement();
			QDomElement modelsElement = root.elementsByTagName("Models").at(0).toElement();

			// 获取所有Model节点 
			QDomNodeList modelNodes = modelsElement.childNodes();
			for (int m = 0; m < modelNodes.size(); ++m) {
				QDomNode modelNode = modelNodes.at(m);
				if (modelNode.isElement() && modelNode.nodeName().startsWith("Model_")) {
					QDomNodeList images = modelNode.toElement().elementsByTagName("Image");
					for (int i = 0; i < images.size(); ++i) {
						QDomElement image = images.at(i).toElement();
						QString imageInfoFile = image.elementsByTagName("ImageInfoFile").at(0).toElement().text();
						int imageId = image.elementsByTagName("ImageID").at(0).toElement().text().toInt();
						imageInfoToIdMap[imageInfoFile] = imageId;
					}
				}
			}
		}
		xmlFile.close();
	}
	else {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 打开PSBundleCmd.xml文件失败"));
		return;
	}

	QFile outFile(outputFile);
	if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 创建PSBundle.gcp文件失败"));
		return;
	}
	QTextStream out(&outFile);

	// 获取 SatTiePointMatch 文件夹下的所有 model_*.icp 文件
	QString tiePointDir = projectdir + QString::fromLocal8Bit("SatTiePointMatch");
	QDir dir(tiePointDir);
	QStringList icpFiles = dir.entryList(QStringList() << "model_*.icp", QDir::Files);

	// 合并控制点来源：原有controlPointsPath和icpFiles 
	QMap<QString, bool> allControlPoints = controlPointsPath;
	for (const QString& icpFile : icpFiles) {
		allControlPoints.insert(tiePointDir + "/" + icpFile, true);
	}

	if (allControlPoints.empty()) {
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 无控制点文件"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差: 无控制点文件"));
		outFile.close();
		return;
	}

	if (m_imagePS->m_visibleControlPointIds.empty()) {
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 控制点数量为0"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("控制网平差: 控制点数量为0"));
		outFile.close();
		return;
	}

	int totalPoints = 0;
	int currentIndex = 0;

	// 第一次遍历：计算总控制点数 
	for (auto data = allControlPoints.begin(); data != allControlPoints.end(); ++data) {
		QFile inFile(data.key());
		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,
				QString::fromLocal8Bit("控制网平差: 打开控制点成果文件失败") + data.key());
			continue;
		}
		QTextStream in(&inFile);

		// 检查文件类型（.txt文件有第一行点数，.icp文件没有）
		bool isIcpFile = data.key().endsWith(".icp");
		if (!isIcpFile) {
			QString firstLine = in.readLine();
			bool ok;
			int points = firstLine.toInt(&ok);
			if (ok) {
				totalPoints += points;
			}
		}
		else {
			// 对于.icp文件，需要统计行数
			int lineCount = 0;
			while (!in.atEnd()) {
				in.readLine();
				lineCount++;
			}
			totalPoints += lineCount;
			inFile.seek(0); // 重置文件指针
		}
		inFile.close();
	}

	// 写入总控制点数 
	out << totalPoints << "\n";

	// 第二次遍历：写入控制点数据 
	for (auto data = allControlPoints.begin(); data != allControlPoints.end(); ++data) {
		QFile inFile(data.key());
		if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,
				QString::fromLocal8Bit("控制网平差: 打开控制点成果文件失败") + data.key());
			continue;
		}
		QTextStream in(&inFile);

		bool isIcpFile = data.key().endsWith(".icp");
		if (!isIcpFile) {
			in.readLine(); // 跳过第一行（点数）
		}

		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (line.isEmpty()) continue;

			QStringList parts;
			if (isIcpFile) {
				// 处理.icp文件格式 
				parts = line.split(QRegExp("\\s+"), Qt::SkipEmptyParts);
				if (parts.size() < 7) continue;

				// 转换格式：从icp格式转换为gcp格式 
				QStringList newParts;
				newParts << QString::number(currentIndex)   // 0: 索引
					<< parts[0] // 1: 点ID
					<< "-1"                             // 2: ImageID (先设为-1，后面会更新)
					<< parts[1]                         // 3: X坐标 
					<< parts[2]                         // 4: Y坐标 
					<< parts[3]                         // 5: Z坐标 
					<< parts[5]                         // 8: imageX 
					<< parts[6]                         // 9: imageY
					<< "1.000"                          // 10: 权重 
					<< "1";                             // 11: 类型

		   // 获取ImageID
				QString imageName = parts[4]; // 卫星影像名称
				for (auto it = imageInfoToIdMap.begin(); it != imageInfoToIdMap.end(); ++it) {
					QFileInfo fileInfo(it.key());
					if (fileInfo.completeBaseName() == imageName) {
						newParts[2] = QString::number(it.value());
						break;
					}
				}
				parts = newParts;
			}
			else {
				// 处理原有.txt文件格式
				parts = line.split(" ", Qt::SkipEmptyParts);
				if (parts.isEmpty()) continue;

				parts[0] = QString::number(currentIndex);

				// 获取当前控制点文件对应的 ImageID 
				int imageId = -1;
				QFileInfo fileInfo(data.key());
				QString baseName = fileInfo.completeBaseName();
				QString result = baseName.section('_', 0, -2);

				for (auto it = imageInfoToIdMap.begin(); it != imageInfoToIdMap.end(); ++it) {
					QFileInfo fileInfotmp(it.key());
					if (fileInfotmp.completeBaseName() == result) {
						imageId = it.value();
						break;
					}
				}
				parts.removeAt(4);
				parts.insert(1, m_imagePS->m_visibleControlPointIds[currentIndex]);
				parts.insert(2, QString::number(imageId));
				parts[8] = "1.000";
				parts[9] = "1";
			}

			currentIndex++;
			out << parts.join("   ") << "\n";
		}
		inFile.close();
	}
	outFile.close();

	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("控制网平差: 序列化平差控制点文件成功"));
	logEdit->append(QString::fromLocal8Bit("****序列化平差控制点文件成功！"));
}

/**
 * @brief 正射纠正
 */
void SystemConfig::orthorectification(QStringList DEMReferencefilename, QStringList SatelliteImagefilename)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有正射纠正"))) {
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("正射纠正"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}
	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****正射纠正****"));

	QStringList filteredFiles = filterSatelliteImagesByColumn(SatelliteImagefilename, 9);

	if (!DEMReferencefilename.empty()){

		QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);

		// 2. 建立新连接 
		QObject::connect(this, &SystemConfig::intersectCmdFinished, this, [=]() {
			QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
			// 保存卫星影像列表到成员变量以便后续清理
			m_orthoSatelliteImages = filteredFiles;

			// 初始化任务队列和相关参数
			m_taskQueue.clear();
			m_runningProcesses.store(0);
			m_completedTasks.store(0);
			m_startTime = QDateTime::currentDateTime();
			m_totalTasks = filteredFiles.size();


			QString wktHCSP = ui.lineEdit_5->text();
			//QString ResultImageFileType = ui.normalCorrect_resultFormatBox->currentText();
			int ResultImageFileTypenum = ui.normalCorrect_resultFormatBox->currentIndex();
			QString ResultImageFileType;
			switch (ResultImageFileTypenum) {
			case 0: ResultImageFileType = QString::fromLocal8Bit("tif"); break;
			case 1: ResultImageFileType = QString::fromLocal8Bit("img"); break;
			case 2: ResultImageFileType = QString::fromLocal8Bit("pix"); break;
			default:
				//logEdit->append(QString::fromLocal8Bit("****请完成正射纠正输出文件格式配置!!!"));
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成正射纠正输出文件格式配置!!!:"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成正射纠正输出文件格式配置!!!"));
				return;
			}
			int ResamplingKernelP = ui.normalCorrect_samplingWayBox->currentIndex();
			bool ClipByDemP = ui.checkBox_8->isChecked();
			bool CreatePydP = ui.checkBox_9->isChecked();
			bool FillNodataP = ui.checkBox_10->isChecked();
			bool SetNodataTagP = ui.checkBox_85->isChecked();
			double NodataTagP = ui.lineEdit_60->text().toDouble();

			if (ResamplingKernelP == -1) {
				//logEdit->append(QString::fromLocal8Bit("****请完成正射纠正系统配置!!!"));
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成正射纠正输出文件格式配置!!!:"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成正射纠正输出文件格式配置"));
				return;
			}

			if (filteredFiles.empty())
			{
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成正射纠正输入数据为空!:"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成正射纠正输入数据为空!"));
				return;
			}

			int index = 0;
			for (auto data : filteredFiles) {
				QStringList DEMpaths;
				QString filePathDEM = (projectdir + "Ortho" + "/" + "TargetFile" + QString::number(index) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DEM") + ".txt");
				QFile fileTXTDEM(filePathDEM);

				if (!fileTXTDEM.open(QIODevice::ReadOnly | QIODevice::Text)) {
					//qDebug() << "Failed to open file:" << filePathDEM;
					ModelMutex.unlock();
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正打开求交成果文件失败: ") + filePathDEM);
					emit FunctionAbnormalExit(QString::fromLocal8Bit("正射纠正打开求交成果文件失败") + filePathDEM);
					continue;
				}

				QTextStream inDEM(&fileTXTDEM);
				int lineNumberDEM = 0;
				while (!inDEM.atEnd()) {
					QString line = inDEM.readLine().trimmed();
					lineNumberDEM++;
					if (lineNumberDEM == 1) continue;
					if (!line.isEmpty()) DEMpaths.append(line);
				}
				fileTXTDEM.close();

				QFileInfo fileInfo(data);
				QDomDocument doc;
				QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
				doc.appendChild(instruction);

				QDomElement root = doc.createElement("OrthoRectify");
				QDomElement OriginImageFileName = doc.createElement("OriginImageFileName");
				OriginImageFileName.appendChild(doc.createTextNode(data));
				root.appendChild(OriginImageFileName);

				for (auto data1 : DEMpaths) {
					QDomElement DemImageFileName = doc.createElement("DemImageFileName");
					DemImageFileName.appendChild(doc.createTextNode(data1));
					root.appendChild(DemImageFileName);
				}

				QDomElement OrthoImageFileName = doc.createElement("OrthoImageFileName");
				OrthoImageFileName.appendChild(doc.createTextNode(projectdir + "Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType));
				root.appendChild(OrthoImageFileName);

				double GSDXnum = ui.lineEdit_12->text().toDouble();
				double GSDYnum = ui.lineEdit_13->text().toDouble();

				QDomElement RectifyPara = doc.createElement("RectifyPara");
				QString imgType = getTiffImageType(data);
				if (imgType == "PAN") GSDYnum = GSDXnum;
				else GSDXnum = GSDYnum;

				//QMap<QString, QString> configInfo = m_imagePS->SetOrthoInfoWidget->getFileConfigInfo(fileInfo.completeBaseName());
				QMap<QString, QString> configInfo = SetOrthoInfoConfig->getFileConfigInfo(fileInfo.completeBaseName());
				if (!configInfo.empty()) {
					GSDXnum = configInfo["GSDX"].toDouble();
					GSDYnum = configInfo["GSDY"].toDouble();
					QString projection = configInfo["Projection"];
					if (projection != "")
						wktHCSP = projection;
				}

				QDomElement GSDX = doc.createElement("GSDX");
				GSDX.appendChild(doc.createTextNode(QString::number(GSDXnum)));
				RectifyPara.appendChild(GSDX);

				QDomElement GSDY = doc.createElement("GSDY");
				GSDY.appendChild(doc.createTextNode(QString::number(GSDYnum)));
				RectifyPara.appendChild(GSDY);

				QDomElement wktHCS = doc.createElement("wktHCS");
				wktHCS.appendChild(doc.createTextNode(wktHCSP));
				RectifyPara.appendChild(wktHCS);

				QString ResamplingKernelstr;
				switch (ResamplingKernelP) {
				case 0: ResamplingKernelstr = "Nearest"; break;
				case 1: ResamplingKernelstr = "BiLinear"; break;
				case 2: ResamplingKernelstr = "BiCubic"; break;
				}

				QDomElement ResamplingKernel = doc.createElement("ResamplingKernel");
				ResamplingKernel.appendChild(doc.createTextNode(ResamplingKernelstr));
				RectifyPara.appendChild(ResamplingKernel);

				QDomElement ClipByDem = doc.createElement("ClipByDem");
				ClipByDem.appendChild(doc.createTextNode(QVariant(ClipByDemP).toString()));
				RectifyPara.appendChild(ClipByDem);

				QDomElement CreatePyd = doc.createElement("CreatePyd");
				CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
				RectifyPara.appendChild(CreatePyd);

				QDomElement FillNodata = doc.createElement("FillNodata");
				FillNodata.appendChild(doc.createTextNode(QVariant(FillNodataP).toString()));
				RectifyPara.appendChild(FillNodata);

				QDomElement SetNodataTag = doc.createElement("SetNodataTag");
				SetNodataTag.appendChild(doc.createTextNode(QVariant(SetNodataTagP).toString()));
				RectifyPara.appendChild(SetNodataTag);

				QDomElement NodataTag = doc.createElement("NodataTag");
				NodataTag.appendChild(doc.createTextNode(QString::number(NodataTagP)));
				RectifyPara.appendChild(NodataTag);

				root.appendChild(RectifyPara);
				doc.appendChild(root);

				QString xmlPath = projectdir + "Ortho/PSOrthoCmd_" + QString::number(index) + ".xml";
				QFile file(xmlPath);
				if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
				{
					ModelMutex.unlock();
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正任务单创建失败..."));
					emit FunctionAbnormalExit(QString::fromLocal8Bit("正射纠正任务单创建失败"));
					return;
				}


				QTextStream out(&file);
				out.setCodec("UTF-8");
				out << doc.toString(4);
				file.close();

				// 将任务添加到队列
				TaskInfo task;
				task.taskFilePath = xmlPath;
				//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSOrthoCmd.exe");
#ifdef Q_OS_LINUX 
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSOrthoCmd.x"));
#else
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSOrthoCmd.exe"));
#endif
				task.outfilename = projectdir + "Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType;
				m_taskQueue.enqueue(task);

				index++;
			}
			logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正开始执行:") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			startNextTasks(QString::fromLocal8Bit("****正射纠正"));
		});

		IntersectCmd(DEMReferencefilename, filteredFiles, QString::fromLocal8Bit("DEM"), QString::fromLocal8Bit("Ortho"));
	}
	else {
		// 保存卫星影像列表到成员变量以便后续清理
		m_orthoSatelliteImages = filteredFiles;

		// 初始化任务队列和相关参数
		m_taskQueue.clear();
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();
		m_totalTasks = filteredFiles.size();

		QString wktHCSP = ui.lineEdit_5->text();
		//QString ResultImageFileType = ui.normalCorrect_resultFormatBox->currentText();
		int ResultImageFileTypenum = ui.normalCorrect_resultFormatBox->currentIndex();
		QString ResultImageFileType;
		switch (ResultImageFileTypenum) {
		case 0: ResultImageFileType = QString::fromLocal8Bit("tif"); break;
		case 1: ResultImageFileType = QString::fromLocal8Bit("img"); break;
		case 2: ResultImageFileType = QString::fromLocal8Bit("pix"); break;
		default:
			//logEdit->append(QString::fromLocal8Bit("****请完成正射纠正输出文件格式配置!!!"));
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成正射纠正输出文件格式配置!!!:"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成正射纠正输出文件格式配置!!!"));
			return;
		}
		int ResamplingKernelP = ui.normalCorrect_samplingWayBox->currentIndex();
		bool ClipByDemP = ui.checkBox_8->isChecked();
		bool CreatePydP = ui.checkBox_9->isChecked();
		bool FillNodataP = ui.checkBox_10->isChecked();
		bool SetNodataTagP = ui.checkBox_85->isChecked();
		double NodataTagP = ui.lineEdit_60->text().toDouble();

		if (ResamplingKernelP == -1) {
			//logEdit->append(QString::fromLocal8Bit("****请完成正射纠正系统配置!!!"));
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成正射纠正输出文件格式配置!!!:"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成正射纠正输出文件格式配置!!!"));
			return;
		}

		if (filteredFiles.empty())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成正射纠正输入数据为空!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成正射纠正输入数据为空!"));
			return;
		}

		int index = 0;
		for (auto data : filteredFiles) {

			QFileInfo fileInfo(data);
			QDomDocument doc;
			QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
			doc.appendChild(instruction);

			QDomElement root = doc.createElement("OrthoRectify");
			QDomElement OriginImageFileName = doc.createElement("OriginImageFileName");
			OriginImageFileName.appendChild(doc.createTextNode(data));
			root.appendChild(OriginImageFileName);

			QDomElement DemImageFileName = doc.createElement("DemImageFileName");
			DemImageFileName.appendChild(doc.createTextNode(""));
			root.appendChild(DemImageFileName);

			QDomElement OrthoImageFileName = doc.createElement("OrthoImageFileName");
			OrthoImageFileName.appendChild(doc.createTextNode(projectdir + "Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType));
			root.appendChild(OrthoImageFileName);

			double GSDXnum = ui.lineEdit_12->text().toDouble();
			double GSDYnum = ui.lineEdit_13->text().toDouble();

			QDomElement RectifyPara = doc.createElement("RectifyPara");
			QString imgType = getTiffImageType(data);
			if (imgType == "PAN") GSDYnum = GSDXnum;
			else GSDXnum = GSDYnum;

			QMap<QString, QString> configInfo = SetOrthoInfoConfig->getFileConfigInfo(fileInfo.completeBaseName());
			if (!configInfo.empty()) {
				GSDXnum = configInfo["GSDX"].toDouble();
				GSDYnum = configInfo["GSDY"].toDouble();
				QString projection = configInfo["Projection"];
				if (projection != "")
					wktHCSP = projection;
			}

			QDomElement GSDX = doc.createElement("GSDX");
			GSDX.appendChild(doc.createTextNode(QString::number(GSDXnum)));
			RectifyPara.appendChild(GSDX);

			QDomElement GSDY = doc.createElement("GSDY");
			GSDY.appendChild(doc.createTextNode(QString::number(GSDYnum)));
			RectifyPara.appendChild(GSDY);

			QDomElement wktHCS = doc.createElement("wktHCS");
			wktHCS.appendChild(doc.createTextNode(wktHCSP));
			RectifyPara.appendChild(wktHCS);

			QString ResamplingKernelstr;
			switch (ResamplingKernelP) {
			case 0: ResamplingKernelstr = "Nearest"; break;
			case 1: ResamplingKernelstr = "BiLinear"; break;
			case 2: ResamplingKernelstr = "BiCubic"; break;
			}

			QDomElement ResamplingKernel = doc.createElement("ResamplingKernel");
			ResamplingKernel.appendChild(doc.createTextNode(ResamplingKernelstr));
			RectifyPara.appendChild(ResamplingKernel);

			QDomElement ClipByDem = doc.createElement("ClipByDem");
			ClipByDem.appendChild(doc.createTextNode(QVariant(ClipByDemP).toString()));
			RectifyPara.appendChild(ClipByDem);

			QDomElement CreatePyd = doc.createElement("CreatePyd");
			CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
			RectifyPara.appendChild(CreatePyd);

			QDomElement FillNodata = doc.createElement("FillNodata");
			FillNodata.appendChild(doc.createTextNode(QVariant(FillNodataP).toString()));
			RectifyPara.appendChild(FillNodata);

			QDomElement SetNodataTag = doc.createElement("SetNodataTag");
			SetNodataTag.appendChild(doc.createTextNode(QVariant(SetNodataTagP).toString()));
			RectifyPara.appendChild(SetNodataTag);

			QDomElement NodataTag = doc.createElement("NodataTag");
			NodataTag.appendChild(doc.createTextNode(QString::number(NodataTagP)));
			RectifyPara.appendChild(NodataTag);

			root.appendChild(RectifyPara);
			doc.appendChild(root);

			QString xmlPath = projectdir + "Ortho/PSOrthoCmd_" + QString::number(index) + ".xml";
			QFile file(xmlPath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正任务单创建失败..."));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("正射纠正任务单创建失败"));
				return;
			}


			QTextStream out(&file);
			out.setCodec("UTF-8");
			out << doc.toString(4);
			file.close();

			// 将任务添加到队列
			TaskInfo task;
			task.taskFilePath = xmlPath;
			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSOrthoCmd.exe");
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSOrthoCmd.x"));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSOrthoCmd.exe"));
#endif
			task.outfilename = projectdir + "Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType;
			m_taskQueue.enqueue(task);

			index++;
		}
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正开始执行:") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		startNextTasks(QString::fromLocal8Bit("****正射纠正"));
	}
}

/**
 * @brief 配准纠正
 */
void SystemConfig::AlignmentAdjustment(QStringList DEMReferencefilename, QStringList SatelliteImagefilename)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有配准纠正"))) {
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("影像纠正"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****配准纠正****"));

	if (!DEMReferencefilename.empty()){

		QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);

		// 2. 建立新连接 
		QObject::connect(this, &SystemConfig::intersectCmdFinished, this, [=]() {
			QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
			// 保存卫星影像列表到成员变量以便后续清理
			m_orthoSatelliteImages = SatelliteImagefilename;

			// 初始化任务队列和相关参数
			m_taskQueue.clear();
			m_runningProcesses.store(0);
			m_completedTasks.store(0);
			m_startTime = QDateTime::currentDateTime();
			m_totalTasks = SatelliteImagefilename.size();


			QString wktHCSP = ui.lineEdit_5->text();
			//QString ResultImageFileType = ui.normalCorrect_resultFormatBox->currentText();
			int ResultImageFileTypenum = ui.normalCorrect_resultFormatBox->currentIndex();
			QString ResultImageFileType;
			switch (ResultImageFileTypenum) {
			case 0: ResultImageFileType = QString::fromLocal8Bit("tif"); break;
			case 1: ResultImageFileType = QString::fromLocal8Bit("img"); break;
			case 2: ResultImageFileType = QString::fromLocal8Bit("pix"); break;
			default:
				//logEdit->append(QString::fromLocal8Bit("****请完成配准纠正输出文件格式配置!!!"));
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!:"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!"));
				return;
			}
			int ResamplingKernelP = ui.normalCorrect_samplingWayBox->currentIndex();
			bool ClipByDemP = ui.checkBox_8->isChecked();
			bool CreatePydP = ui.checkBox_9->isChecked();
			bool FillNodataP = ui.checkBox_10->isChecked();
			bool SetNodataTagP = ui.checkBox_85->isChecked();
			double NodataTagP = ui.lineEdit_60->text().toDouble();

			if (ResamplingKernelP == -1) {
				//logEdit->append(QString::fromLocal8Bit("****请完成配准纠正系统配置!!!"));
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!:"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!"));
				return;
			}

			if (SatelliteImagefilename.empty())
			{
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成配准纠正输入数据为空!"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成配准纠正输入数据为空!"));
				return;
			}

			int index = 0;
			for (auto data : SatelliteImagefilename) {
				QStringList DEMpaths;
				QString filePathDEM = (projectdir + "RegisteOrtho/Ortho" + "/" + "TargetFile" + QString::number(index) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DEM") + ".txt");
				QFile fileTXTDEM(filePathDEM);

				if (!fileTXTDEM.open(QIODevice::ReadOnly | QIODevice::Text)) {
					//qDebug() << "Failed to open file:" << filePathDEM;
					ModelMutex.unlock();
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("配准纠正打开求交成果文件失败: ") + filePathDEM);
					emit FunctionAbnormalExit(QString::fromLocal8Bit("配准纠正打开求交成果文件失败") + filePathDEM);
					return;
				}

				QTextStream inDEM(&fileTXTDEM);
				int lineNumberDEM = 0;
				while (!inDEM.atEnd()) {
					QString line = inDEM.readLine().trimmed();
					lineNumberDEM++;
					if (lineNumberDEM == 1) continue;
					if (!line.isEmpty()) DEMpaths.append(line);
				}
				fileTXTDEM.close();

				QFileInfo fileInfo(data);
				QDomDocument doc;
				QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
				doc.appendChild(instruction);

				QDomElement root = doc.createElement("OrthoRectify");
				QDomElement OriginImageFileName = doc.createElement("OriginImageFileName");
				OriginImageFileName.appendChild(doc.createTextNode(data));
				root.appendChild(OriginImageFileName);

				for (auto data1 : DEMpaths) {
					QDomElement DemImageFileName = doc.createElement("DemImageFileName");
					DemImageFileName.appendChild(doc.createTextNode(data1));
					root.appendChild(DemImageFileName);
				}

				QDomElement OrthoImageFileName = doc.createElement("OrthoImageFileName");
				OrthoImageFileName.appendChild(doc.createTextNode(projectdir + "RegisteOrtho/Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType));
				root.appendChild(OrthoImageFileName);

				double GSDXnum = ui.lineEdit_12->text().toDouble();
				double GSDYnum = ui.lineEdit_13->text().toDouble();

				QDomElement RectifyPara = doc.createElement("RectifyPara");
				QString imgType = getTiffImageType(data);
				if (imgType == "PAN") GSDYnum = GSDXnum;
				else GSDXnum = GSDYnum;

				QMap<QString, QString> configInfo = SetOrthoInfoConfig->getFileConfigInfo(fileInfo.completeBaseName());
				if (!configInfo.empty()) {
					GSDXnum = configInfo["GSDX"].toDouble();
					GSDYnum = configInfo["GSDY"].toDouble();
					QString projection = configInfo["Projection"];
					if (projection != "")
						wktHCSP = projection;
				}

				QDomElement GSDX = doc.createElement("GSDX");
				GSDX.appendChild(doc.createTextNode(QString::number(GSDXnum)));
				RectifyPara.appendChild(GSDX);

				QDomElement GSDY = doc.createElement("GSDY");
				GSDY.appendChild(doc.createTextNode(QString::number(GSDYnum)));
				RectifyPara.appendChild(GSDY);

				QDomElement wktHCS = doc.createElement("wktHCS");
				wktHCS.appendChild(doc.createTextNode(wktHCSP));
				RectifyPara.appendChild(wktHCS);

				QString ResamplingKernelstr;
				switch (ResamplingKernelP) {
				case 0: ResamplingKernelstr = "Nearest"; break;
				case 1: ResamplingKernelstr = "BiLinear"; break;
				case 2: ResamplingKernelstr = "BiCubic"; break;
				}

				QDomElement ResamplingKernel = doc.createElement("ResamplingKernel");
				ResamplingKernel.appendChild(doc.createTextNode(ResamplingKernelstr));
				RectifyPara.appendChild(ResamplingKernel);

				QDomElement ClipByDem = doc.createElement("ClipByDem");
				ClipByDem.appendChild(doc.createTextNode(QVariant(ClipByDemP).toString()));
				RectifyPara.appendChild(ClipByDem);

				QDomElement CreatePyd = doc.createElement("CreatePyd");
				CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
				RectifyPara.appendChild(CreatePyd);

				QDomElement FillNodata = doc.createElement("FillNodata");
				FillNodata.appendChild(doc.createTextNode(QVariant(FillNodataP).toString()));
				RectifyPara.appendChild(FillNodata);

				QDomElement SetNodataTag = doc.createElement("SetNodataTag");
				SetNodataTag.appendChild(doc.createTextNode(QVariant(SetNodataTagP).toString()));
				RectifyPara.appendChild(SetNodataTag);

				QDomElement NodataTag = doc.createElement("NodataTag");
				NodataTag.appendChild(doc.createTextNode(QString::number(NodataTagP)));
				RectifyPara.appendChild(NodataTag);

				root.appendChild(RectifyPara);
				doc.appendChild(root);

				QString xmlPath = projectdir + "RegisteOrtho/Ortho/PSOrthoCmd_" + QString::number(index) + ".xml";
				QFile file(xmlPath);
				if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
				{
					ModelMutex.unlock();
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("配准纠正任务单创建失败...") + filePathDEM);
					emit FunctionAbnormalExit(QString::fromLocal8Bit("配准纠正任务单创建失败"));
					return;
				}


				QTextStream out(&file);
				out.setCodec("UTF-8");
				out << doc.toString(4);
				file.close();

				// 将任务添加到队列
				TaskInfo task;
				task.taskFilePath = xmlPath;
				//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSOrthoCmd.exe");
#ifdef Q_OS_LINUX 
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSOrthoCmd.x"));
#else
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSOrthoCmd.exe"));
#endif
				task.outfilename = projectdir + "RegisteOrtho/Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType;
				m_taskQueue.enqueue(task);

				index++;
			}
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("配准纠正开始执行:") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			startNextTasks(QString::fromLocal8Bit("****配准纠正"));
		});

		IntersectCmd(DEMReferencefilename, SatelliteImagefilename, QString::fromLocal8Bit("DEM"), QString::fromLocal8Bit("RegisteOrtho/Ortho"));
	}
	else {
		// 保存卫星影像列表到成员变量以便后续清理
		m_orthoSatelliteImages = SatelliteImagefilename;

		// 初始化任务队列和相关参数
		m_taskQueue.clear();
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();
		m_totalTasks = SatelliteImagefilename.size();

		QString wktHCSP = ui.lineEdit_5->text();
		//QString ResultImageFileType = ui.normalCorrect_resultFormatBox->currentText();
		int ResultImageFileTypenum = ui.normalCorrect_resultFormatBox->currentIndex();
		QString ResultImageFileType;
		switch (ResultImageFileTypenum) {
		case 0: ResultImageFileType = QString::fromLocal8Bit("tif"); break;
		case 1: ResultImageFileType = QString::fromLocal8Bit("img"); break;
		case 2: ResultImageFileType = QString::fromLocal8Bit("pix"); break;
		default:
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!:"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!"));
			return;
		}
		int ResamplingKernelP = ui.normalCorrect_samplingWayBox->currentIndex();
		bool ClipByDemP = ui.checkBox_8->isChecked();
		bool CreatePydP = ui.checkBox_9->isChecked();
		bool FillNodataP = ui.checkBox_10->isChecked();
		bool SetNodataTagP = ui.checkBox_85->isChecked();
		double NodataTagP = ui.lineEdit_60->text().toDouble();

		if (ResamplingKernelP == -1) {
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!:"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成配准纠正输出文件格式配置!!!"));
			return;
		}

		if (SatelliteImagefilename.empty())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成配准纠正输入数据为空!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成配准纠正输入数据为空!"));
			return;
		}

		int index = 0;
		for (auto data : SatelliteImagefilename) {

			QFileInfo fileInfo(data);
			QDomDocument doc;
			QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
			doc.appendChild(instruction);

			QDomElement root = doc.createElement("OrthoRectify");
			QDomElement OriginImageFileName = doc.createElement("OriginImageFileName");
			OriginImageFileName.appendChild(doc.createTextNode(data));
			root.appendChild(OriginImageFileName);

			QDomElement DemImageFileName = doc.createElement("DemImageFileName");
			DemImageFileName.appendChild(doc.createTextNode(""));
			root.appendChild(DemImageFileName);

			QDomElement OrthoImageFileName = doc.createElement("OrthoImageFileName");
			OrthoImageFileName.appendChild(doc.createTextNode(projectdir + "RegisteOrtho/Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType));
			root.appendChild(OrthoImageFileName);

			double GSDXnum = ui.lineEdit_12->text().toDouble();
			double GSDYnum = ui.lineEdit_13->text().toDouble();

			QDomElement RectifyPara = doc.createElement("RectifyPara");
			QString imgType = getTiffImageType(data);
			if (imgType == "PAN") GSDYnum = GSDXnum;
			else GSDXnum = GSDYnum;

			QMap<QString, QString> configInfo = SetOrthoInfoConfig->getFileConfigInfo(fileInfo.completeBaseName());
			if (!configInfo.empty()) {
				GSDXnum = configInfo["GSDX"].toDouble();
				GSDYnum = configInfo["GSDY"].toDouble();
				QString projection = configInfo["Projection"];
				if (projection != "")
					wktHCSP = projection;
			}

			QDomElement GSDX = doc.createElement("GSDX");
			GSDX.appendChild(doc.createTextNode(QString::number(GSDXnum)));
			RectifyPara.appendChild(GSDX);

			QDomElement GSDY = doc.createElement("GSDY");
			GSDY.appendChild(doc.createTextNode(QString::number(GSDYnum)));
			RectifyPara.appendChild(GSDY);

			QDomElement wktHCS = doc.createElement("wktHCS");
			wktHCS.appendChild(doc.createTextNode(wktHCSP));
			RectifyPara.appendChild(wktHCS);

			QString ResamplingKernelstr;
			switch (ResamplingKernelP) {
			case 0: ResamplingKernelstr = "Nearest"; break;
			case 1: ResamplingKernelstr = "BiLinear"; break;
			case 2: ResamplingKernelstr = "BiCubic"; break;
			}

			QDomElement ResamplingKernel = doc.createElement("ResamplingKernel");
			ResamplingKernel.appendChild(doc.createTextNode(ResamplingKernelstr));
			RectifyPara.appendChild(ResamplingKernel);

			QDomElement ClipByDem = doc.createElement("ClipByDem");
			ClipByDem.appendChild(doc.createTextNode(QVariant(ClipByDemP).toString()));
			RectifyPara.appendChild(ClipByDem);

			QDomElement CreatePyd = doc.createElement("CreatePyd");
			CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
			RectifyPara.appendChild(CreatePyd);

			QDomElement FillNodata = doc.createElement("FillNodata");
			FillNodata.appendChild(doc.createTextNode(QVariant(FillNodataP).toString()));
			RectifyPara.appendChild(FillNodata);

			QDomElement SetNodataTag = doc.createElement("SetNodataTag");
			SetNodataTag.appendChild(doc.createTextNode(QVariant(SetNodataTagP).toString()));
			RectifyPara.appendChild(SetNodataTag);

			QDomElement NodataTag = doc.createElement("NodataTag");
			NodataTag.appendChild(doc.createTextNode(QString::number(NodataTagP)));
			RectifyPara.appendChild(NodataTag);

			root.appendChild(RectifyPara);
			doc.appendChild(root);

			QString xmlPath = projectdir + "RegisteOrtho/Ortho/PSOrthoCmd_" + QString::number(index) + ".xml";
			QFile file(xmlPath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("配准纠正任务单创建失败..."));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("配准纠正任务单创建失败"));
				return;
			}


			QTextStream out(&file);
			out.setCodec("UTF-8");
			out << doc.toString(4);
			file.close();

			// 将任务添加到队列
			TaskInfo task;
			task.taskFilePath = xmlPath;
			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSOrthoCmd.exe");
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSOrthoCmd.x"));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSOrthoCmd.exe"));
#endif
			task.outfilename = projectdir + "RegisteOrtho/Ortho" + "/" + fileInfo.completeBaseName() + "." + ResultImageFileType;
			m_taskQueue.enqueue(task);

			index++;
		}
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("配准纠正开始执行:") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		startNextTasks(QString::fromLocal8Bit("****配准纠正"));
	}
}

/**
 * @brief 影像融合
 */
void SystemConfig::imageInterAction()
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有影像融合"))) {
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("影像融合"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}
	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****影像融合****"));

	// 准备任务数据 
	if (!m_imagePS->tmpimageInterPath.empty() && m_imagePS->tmpimageInterflag) {
		m_imagePS->tmpimageInterflag = false;
		imageInterPath = m_imagePS->tmpimageInterPath;
		m_imagePS->tmpimageInterPath.clear();
	}

	QString xmlPath = projectdir + QString::fromLocal8Bit("Fusion/SatModelMakerCmd.xml");

	// 创建并启动SatModelMakerCmd进程
	QProcess *process = new QProcess(this);
	QString authCmd;
#ifdef Q_OS_LINUX 
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSSatModelMakerCmd.x"));
#else
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSSatModelMakerCmd.exe"));
#endif
	process->start(authCmd, QStringList() << xmlPath);
	PublicFunctions::writeTimestampToXml("INAModelmarker", m_imagePS->projectdir);
	m_systemProcesses.append(process);

	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合模型配对开始..."));

	connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[this, process](int exitCode, QProcess::ExitStatus status) {
		m_systemProcesses.removeOne(process);
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合模型配对完成..."));

		//QDateTime timestamp = PublicFunctions::readTimestampFromXml("INAModelmarker");
		//if (!timestamp.isValid()) {
		//	ModelMutex.unlock();
		//	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("获取影像融合执行时间获取无效"));

		//	emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合获取模型配对结果失败!"));
		//	return;
		//}

		//QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("Fusion") + "/", timestamp);
		//QString tmpPath = projectdir + QString::fromLocal8Bit("Fusion/SatModelMakerCmdout.xml");
		QString SatModelMakerCmdoutPath = projectdir + QString::fromLocal8Bit("Fusion/SatModelMakerCmdout.xml");
		//QString SatModelMakerCmdoutPath = "";
		//for (auto datafile : resultFiles)
		//{
		//	if (tmpPath == datafile)
		//		SatModelMakerCmdoutPath = tmpPath;
		//}
		//if (SatModelMakerCmdoutPath == "")
		//{
		//	ModelMutex.unlock();
		//	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("没有获取到影像融合执行时间后的SatModelMakerCmdout.xml文件"));

		//	emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合获取模型配对结果失败!"));
		//	return;
		//}

		// 解析SatModelMakerCmdout.xml  
		QDomDocument cmdoutDoc;
		QFile cmdoutFile(SatModelMakerCmdoutPath);
		if (!cmdoutFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open SatModelMakerCmdout.xml";
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合: 打开SatModelMakerCmdout.xml文件获取模型失败..."));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合: 打开SatModelMakerCmdout.xml文件获取模型失败..."));
			//emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合"));
			return;
		}
		if (!cmdoutDoc.setContent(&cmdoutFile)) {
			cmdoutFile.close();
			//qDebug() << "Failed to parse SatModelMakerCmdout.xml";
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合: 解析SatModelMakerCmdout.xml文件获取模型失败..."));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合: 解析SatModelMakerCmdout.xml文件获取模型失败..."));
			//emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合"));
			return;
		}
		cmdoutFile.close();

		// 获取配置参数
		bool CreatePydP = ui.checkBox_14->isChecked();
		bool GreenEnhanceP = ui.checkBox_15->isChecked();
		bool SharpenP = ui.checkBox_16->isChecked();
		bool OnlyRGBP = ui.checkBox_17->isChecked();
		bool AdjustBandP = ui.checkBox_18->isChecked();
		int FusionMethod = ui.imageFusion_fusionWayBox->currentIndex();
		int ResultImageFileTypenum = ui.imageFusion_outImageFormatBox->currentIndex();
		QString ResultImageFileType;
		switch (ResultImageFileTypenum) {
		case 0: ResultImageFileType = QString::fromLocal8Bit(".tif"); break;
		case 1: ResultImageFileType = QString::fromLocal8Bit(".img"); break;
		case 2: ResultImageFileType = QString::fromLocal8Bit(".pix"); break;
		default:
			//logEdit->append(QString::fromLocal8Bit("****请完成影像融合输出文件格式配置!!!"));
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成影像融合输出文件格式配置"));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成影像融合输出文件格式配置"));
			return;
		}
		QString GreenEnhanceCoefP = ui.lineEdit_14->text();
		int ResamplingKernelP = ui.normalCorrect_samplingWayBox->currentIndex();

		// 解析模型信息并生成任务
		QDomElement cmdoutRoot = cmdoutDoc.documentElement();
		QDomElement cmdoutModels = cmdoutRoot.firstChildElement("Models");
		QDomElement cmdoutModel = cmdoutModels.firstChildElement();

		m_taskQueue.clear();
		int modelIndex = 0;

		if (cmdoutModel.isNull())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合: 模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合: 模型配对输出文件SatModelMakerCmdout.xml中模型数量为0!"));
			//emit FunctionAbnormalExit(QString::fromLocal8Bit("影像融合"));
			return;
		}

		// 检查重复文件 
		int duplicateCount = 0;
		QVector<bool> isDuplicate;
		QDomElement tempModel = cmdoutModel;
		while (!tempModel.isNull()) {
			QDomElement mulImageName = tempModel.firstChildElement("MulImageName");
			if (!mulImageName.isNull()) {
				QString mulImagePath = mulImageName.text();
				QFileInfo filenameout(mulImagePath);
				QString outputfile = projectdir + QString::fromLocal8Bit("Fusion") + "/" + filenameout.completeBaseName() + QString::number(modelIndex) + ResultImageFileType;

				//// 检查逻辑：同时检查文件存在性和imageInterPath中的值 
				//bool shouldSkip = QFile::exists(outputfile) &&
				//	imageInterPath.contains(outputfile) &&
				//	!imageInterPath.value(outputfile);
				// 检查逻辑：同时检查文件存在性和imageInterPath中的值 
				bool shouldSkip = QFile::exists(outputfile) &&
					imageInterPath.contains(outputfile);

				if (shouldSkip) {
					if (!imageInterPath.contains(outputfile)) {
						imageInterPath.insert(outputfile, false);
					}
					duplicateCount++;
					isDuplicate.append(true);
				}
				else {
					isDuplicate.append(false);
				}
			}
			modelIndex++;
			tempModel = tempModel.nextSiblingElement();
		}

		// 处理重复文件情况
		bool processAll = true;
		if (duplicateCount > 0) {  // 移除了imageInterPath的条件检查，因为已经在上面处理 
			QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
				QString::fromLocal8Bit("文件已存在"),
				QString::fromLocal8Bit("已经存在%1个匹配成果，是否重新融合?").arg(duplicateCount),
				QMessageBox::Yes | QMessageBox::No);

			processAll = (reply == QMessageBox::Yes);
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合: 用户选择不覆盖已有成果文件,已存在:总和 , ") + QString::number(duplicateCount) + " : " + QString::number(modelIndex));
			if (!processAll && duplicateCount == modelIndex) {
				// 所有文件都重复且用户选择不重新处理
				logEdit->append(QString::fromLocal8Bit(" 用户取消重新融合已存在的成果"));
				ModelMutex.unlock();
				emit imageInterActionFinished();
				return;
			}
		}

		// 重置计数器
		modelIndex = 0;
		cmdoutModel = cmdoutModels.firstChildElement();
		int duplicateIndex = 0;

		while (!cmdoutModel.isNull()) {
			// 检查是否需要跳过重复文件
			if (!processAll && isDuplicate[duplicateIndex++]) {
				cmdoutModel = cmdoutModel.nextSiblingElement();
				modelIndex++;
				continue;
			}

			// 创建 QDomDocument 
			QDomDocument doc;

			// 添加 XML 声明 
			QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
			doc.appendChild(instruction);

			// 创建根节点 <ImageFusion>
			QDomElement root = doc.createElement("ImageFusion");
			root.setAttribute("version", "1.0");

			// 处理NadImage 
			QDomElement nadImageName = cmdoutModel.firstChildElement("NadImageName");
			if (!nadImageName.isNull()) {
				QString nadImagePath = nadImageName.text();
				QDomElement imageInfoFile = doc.createElement("HRImageFilePath");
				imageInfoFile.setAttribute("type", "string");
				imageInfoFile.appendChild(doc.createTextNode(nadImagePath));
				root.appendChild(imageInfoFile);
			}

			// 处理FwdImage (如果有)   
			QDomElement fwdImageName = cmdoutModel.firstChildElement("FwdImageName");
			if (!fwdImageName.isNull()) {
				QString fwdImagePath = fwdImageName.text();
				QDomElement imageInfoFile = doc.createElement("HRImageFilePath");
				imageInfoFile.setAttribute("type", "string");
				imageInfoFile.appendChild(doc.createTextNode(fwdImagePath));
				root.appendChild(imageInfoFile);
			}

			// 处理BwdImage (如果有)   
			QDomElement bwdImageName = cmdoutModel.firstChildElement("BwdImageName");
			if (!bwdImageName.isNull()) {
				QString bwdImagePath = bwdImageName.text();
				QDomElement imageInfoFile = doc.createElement("HRImageFilePath");
				imageInfoFile.setAttribute("type", "string");
				imageInfoFile.appendChild(doc.createTextNode(bwdImagePath));
				root.appendChild(imageInfoFile);
			}

			QString mulImagePath;
			// 处理MulImage   
			QDomElement mulImageName = cmdoutModel.firstChildElement("MulImageName");
			if (!mulImageName.isNull()) {
				mulImagePath = mulImageName.text();
				QDomElement imageInfoFile = doc.createElement("MSImageFilePath");
				imageInfoFile.setAttribute("type", "string");
				imageInfoFile.appendChild(doc.createTextNode(mulImagePath));
				root.appendChild(imageInfoFile);
			}

			QDomElement ResultImageFilePath = doc.createElement("ResultImageFilePath");
			ResultImageFilePath.setAttribute("type", "string");
			QFileInfo filenameout(mulImagePath);
			//QString outputfile = projectdir + QString::fromLocal8Bit("Fusion") + "/" + filenameout.completeBaseName() + QString::number(modelIndex) + ResultImageFileType;
			QString outputfile = projectdir + QString::fromLocal8Bit("Fusion") + "/" + filenameout.completeBaseName() + ResultImageFileType;
			ResultImageFilePath.appendChild(doc.createTextNode(outputfile));
			root.appendChild(ResultImageFilePath);

			QDomElement CreatePyd = doc.createElement("CreatePyd");
			CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
			root.appendChild(CreatePyd);

			QDomElement GreenEnhance = doc.createElement("GreenEnhance");
			GreenEnhance.appendChild(doc.createTextNode(QVariant(GreenEnhanceP).toString()));
			root.appendChild(GreenEnhance);

			QDomElement Sharpen = doc.createElement("Sharpen");
			Sharpen.appendChild(doc.createTextNode(QVariant(SharpenP).toString()));
			root.appendChild(Sharpen);

			QDomElement ResamplingKernel = doc.createElement("ResamplingKernel");
			ResamplingKernel.appendChild(doc.createTextNode(QString::number(ResamplingKernelP)));
			root.appendChild(ResamplingKernel);

			QDomElement GreenEnhanceCoef = doc.createElement("GreenEnhanceCoef");
			GreenEnhanceCoef.appendChild(doc.createTextNode(GreenEnhanceCoefP));
			root.appendChild(GreenEnhanceCoef);

			QDomElement OnlyRGB = doc.createElement("OnlyRGB");
			OnlyRGB.appendChild(doc.createTextNode(QVariant(OnlyRGBP).toString()));
			root.appendChild(OnlyRGB);

			//QString BandP;
			//if (AdjustBandP)
			//	BandP = "3-2-1";
			//else
			//	BandP = "1-2-3";

			QDomElement AdjustBand = doc.createElement("AdjustBand");
			AdjustBand.appendChild(doc.createTextNode(QVariant(AdjustBandP).toString()));
			root.appendChild(AdjustBand);

			QDomElement OldVersion = doc.createElement("OldVersion");
			OldVersion.appendChild(doc.createTextNode("false"));
			root.appendChild(OldVersion);

			QDomElement OutputImageType = doc.createElement("OutputImageType");
			OutputImageType.appendChild(doc.createTextNode((QString::number(ResultImageFileTypenum))));
			root.appendChild(OutputImageType);

			QDomElement FusionMethodP = doc.createElement("FusionMethod");
			FusionMethodP.appendChild(doc.createTextNode(QString::number(FusionMethod)));
			root.appendChild(FusionMethodP);

			doc.appendChild(root);
			// 写入文件 
			QString xmlPath = (projectdir + QString::fromLocal8Bit("Fusion/PSFusionCmd") + QString::number(modelIndex) + QString::fromLocal8Bit(".xml"));

			QFile file(xmlPath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合任务单创建失败: ") + xmlPath);
				continue;
			}
			QTextStream out(&file);
			out.setCodec("UTF-8");
			out << doc.toString(4);
			file.close();

			// 添加任务到队列 
			TaskInfo task;
			task.taskFilePath = xmlPath;
			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSFusionCmd.exe");
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSFusionCmd.x"));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSFusionCmd.exe"));
#endif
			task.outfilename = outputfile;
			m_taskQueue.enqueue(task);

			modelIndex++;
			cmdoutModel = cmdoutModel.nextSiblingElement();
		}

		// 初始化任务控制变量
		m_totalTasks = modelIndex;
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();

		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合开始执行:") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		startNextTasks(QString::fromLocal8Bit("****影像融合"));

		process->deleteLater();
	});
}

/**
* @brief 真彩色转换
*/
void SystemConfig::TrueColorConversion(QStringList imagePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有真彩色转换"))) {
		return;
	}

	if ((!m_imagePS->getWorkProcessflag())&& !(ConvertTo8Bitflag && ui.output8bImage_checkBox->isChecked())) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("真彩色转换"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}

	QString outputDir;
	QString protitle;
	setProjectdir();

	if (ConvertTo8Bitflag && ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir = projectdir + "Dodging";
		protitle = QString::fromLocal8Bit("****影像匀色");
	}
	else {
		outputDir = projectdir + "OutByte";
		protitle = QString::fromLocal8Bit("****真彩色转换");
		logEdit->append(protitle + "****");
	}

	// 获取界面参数
	double minCutCoefnum = ui.lineEdit_32->text().toDouble();
	double maxCutCoefnum = ui.lineEdit_33->text().toDouble();
	bool OnlyRGBP = ui.checkBox_39->isChecked();
	bool GreenEnhanceP = ui.checkBox_40->isChecked();
	double VegGreenCoefnum = ui.lineEdit_34->text().toDouble();
	double VegRedCoefnum = ui.lineEdit_35->text().toDouble();
	double VegBlueCoefnum = ui.lineEdit_36->text().toDouble();
	bool BlueEnhanceP = ui.checkBox_42->isChecked();
	double WaterGreenCoefnum = ui.lineEdit_38->text().toDouble();
	double WaterRedCoefnum = ui.lineEdit_39->text().toDouble();
	double WaterBlueCoefnum = ui.lineEdit_40->text().toDouble();
	double BrightnessCoefnum = ui.lineEdit_56->text().toDouble();
	double ExposeControlThreholdnum = ui.lineEdit_42->text().toDouble();
	double LandCoefnum = ui.lineEdit_43->text().toDouble();
	double SaturationCoefnum = ui.lineEdit_44->text().toDouble();
	double Numbernum = ui.lineEdit_41->text().toDouble();
	double Radiusnum = ui.lineEdit_57->text().toDouble();
	bool CreatePydP = ui.checkBox_38->isChecked();
	bool AdjustBandP = ui.checkBox_41->isChecked();
	bool ExposureSuppressP = ui.checkBox_45->isChecked();
	bool AdaptiveStretchP = ui.checkBox_43->isChecked();
	bool AdjustClearnessP = ui.AdjustClearness_check->isChecked();
	bool AdjustBrightnessP = ui.checkBox_82->isChecked();
	bool LandEnhanceP = ui.checkBox_46->isChecked();
	bool SaturationAdjustP = ui.checkBox_47->isChecked();
	bool SharpenP = ui.checkBox_48->isChecked();
	bool FineStatisticalP = ui.checkBox_28->isChecked();
	int AdjustClearnessTypenum = ui.AdjustClearnessType_Box->currentIndex();
	int OptimizedMethodnum = ui.trueColorConver_colorAdjustWayBox->currentIndex();
	int CulTureTypenum = ui.trueColorConver_groundObjectTypeBox->currentIndex();

	// 获取输出文件格式
	int ResultImageFileTypenum = ui.trueColorConver_outImageFormatBox->currentIndex();
	QString outfiletype;
	switch (ResultImageFileTypenum) {
	case 0: outfiletype = QString::fromLocal8Bit(".tif"); break;
	case 1: outfiletype = QString::fromLocal8Bit(".img"); break;
	case 2: outfiletype = QString::fromLocal8Bit(".pix"); break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成真彩色转换输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成真彩色转换输出文件格式配置"));

		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成真彩色转换输出文件格式配置"));
		return;
	}

	// 准备任务数据 
	QMap<QString, bool> tmpimageInterPath;
	if (!imagePath.empty()) {
		for (auto data : imagePath) {
			tmpimageInterPath.insert(data, false);
		}
	}
	else {
		if (imageInterPath.empty() && m_imagePS->tmpimageInterflag) {
			imageInterPath = m_imagePS->tmpimageInterPath;
			m_imagePS->tmpimageInterflag = false;
		}
		tmpimageInterPath = imageInterPath;
	}

	// 检查是否有任务需要处理 
	if (tmpimageInterPath.empty()) {
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("真彩色转换输入待处理数据为空"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("真彩色转换输入待处理数据为空"));
		return;
	}
	// 初始化任务队列
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;

	QVector<bool> isDuplicate;
	bool processAll = true;
	if (!ConvertTo8Bitflag) {
		// 检查输出文件是否存在 
		int duplicateCount = 0;
		
		for (auto filename = tmpimageInterPath.begin(); filename != tmpimageInterPath.end(); ++filename) {
			if (!filename.value()) {
				QFileInfo fileinfo(filename.key());
				QString outfilename = outputDir + "/" + fileinfo.completeBaseName() + outfiletype;
				if (QFile::exists(outfilename)) {
					duplicateCount++;
					isDuplicate.append(true);
				}
				else {
					isDuplicate.append(false);
				}
			}
		}

		// 处理重复文件情况 
		if (duplicateCount > 0) {
			QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
				QString::fromLocal8Bit("文件已存在"),
				QString::fromLocal8Bit("已经存在%1个转换成果，是否重新转换?").arg(duplicateCount),
				QMessageBox::Yes | QMessageBox::No);

			processAll = (reply == QMessageBox::Yes);
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("真彩色转换: 用户选择不覆盖已有成果文件,已存在:总和 , ") + QString::number(duplicateCount) + " : " + QString::number(tmpimageInterPath.size()));

			if (!processAll && duplicateCount == tmpimageInterPath.size()) {
				// 所有文件都重复且用户选择不重新处理
				ModelMutex.unlock();
				emit TrueColorConversionFinished();
				return;
			}
		}
	}
	// 为每个图像创建任务 
	int PSindex = 0;
	int duplicateIndex = 0;
	for (auto filename = tmpimageInterPath.begin(); filename != tmpimageInterPath.end(); ++filename) {
		if (!ConvertTo8Bitflag) {
			// 检查是否需要跳过重复文件
			if (!processAll && isDuplicate[duplicateIndex++]) {
				continue;
			}
		}

			// 创建XML文档
			QDomDocument doc;
			QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
			doc.appendChild(instruction);

			QDomElement root = doc.createElement("XQImageOutByteCmd");
			root.setAttribute("version", "1.0");
			doc.appendChild(root);

			// 添加输入输出文件路径 
			QDomElement inputImageFileName = doc.createElement("inputImageFileName");
			inputImageFileName.setAttribute("type", "string");
			inputImageFileName.appendChild(doc.createTextNode(filename.key()));
			root.appendChild(inputImageFileName);

			QFileInfo fileinfo(filename.key());
			QDomElement outputFileName = doc.createElement("outputFileName");
			outputFileName.setAttribute("type", "string");
			QString outfilename = outputDir + "/" + fileinfo.completeBaseName() + outfiletype;
			outputFileName.appendChild(doc.createTextNode(outfilename));
			root.appendChild(outputFileName);

			// 添加处理参数
			auto addChildNode = [&doc, &root](const QString& tagName, const QString& value) {
				QDomElement element = doc.createElement(tagName);
				element.appendChild(doc.createTextNode(value));
				root.appendChild(element);
			};

			addChildNode("minCutCoef", QString::number(minCutCoefnum));
			addChildNode("maxCutCoef", QString::number(maxCutCoefnum));
			addChildNode("OnlyRGB", QVariant(OnlyRGBP).toString());
			addChildNode("GreenEnhance", QVariant(GreenEnhanceP).toString());
			addChildNode("GreenEnhanceCoef", QString::number(VegGreenCoefnum));
			addChildNode("VegGreenCoef", QString::number(VegGreenCoefnum));
			addChildNode("VegRedCoef", QString::number(VegRedCoefnum));
			addChildNode("VegBlueCoef", QString::number(VegBlueCoefnum));
			addChildNode("BlueEnhance", QVariant(BlueEnhanceP).toString());
			addChildNode("BlueEnhanceCoef", QString::number(WaterBlueCoefnum));
			addChildNode("WaterGreenCoef", QString::number(WaterGreenCoefnum));
			addChildNode("WaterRedCoef", QString::number(WaterRedCoefnum));
			addChildNode("WaterBlueCoef", QString::number(WaterBlueCoefnum));
			addChildNode("CreatePyd", QVariant(CreatePydP).toString());
			addChildNode("AdjustBand", QVariant(AdjustBandP).toString());
			addChildNode("ExposureSuppress", QVariant(ExposureSuppressP).toString());
			addChildNode("AdaptiveStretch", QVariant(AdaptiveStretchP).toString());
			addChildNode("Tasks", "4");
			addChildNode("AdjustContrast", "false");
			addChildNode("ContrastCoef", "0.1");
			addChildNode("Dehaze", "false");
			addChildNode("DehazeCoef", "0.95");
			addChildNode("ExposeControlThrehold", QString::number(ExposeControlThreholdnum));
			addChildNode("OptimizedMethod", QString::number(OptimizedMethodnum));
			addChildNode("CulTureType", QString::number(CulTureTypenum));
			addChildNode("LandEnhance", QVariant(LandEnhanceP).toString());
			addChildNode("LandCoef", QString::number(LandCoefnum));
			addChildNode("SaturationAdjust", QVariant(SaturationAdjustP).toString());
			addChildNode("SaturationCoef", QString::number(SaturationCoefnum));
			addChildNode("Sharpen", QVariant(SharpenP).toString());
			addChildNode("AdjustClearness", QVariant(AdjustClearnessP).toString());
			addChildNode("AdjustClearnessType", QString::number(AdjustClearnessTypenum));
			addChildNode("AdjustBrightness", QVariant(AdjustBrightnessP).toString());
			addChildNode("BrightnessCoef", QString::number(BrightnessCoefnum, 'f', 6));
			addChildNode("Number", QString::number(Numbernum, 'f', 6));
			addChildNode("Radius", QString::number(Radiusnum, 'f', 6));
			addChildNode("FineStatistical", QVariant(FineStatisticalP).toString());

			// 保存XML文件
			QString xmlPath = outputDir + "/" + "PSImageOutByteCmd" + QString::number(PSindex) + QString::fromLocal8Bit(".xml");
			QFile file(xmlPath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				qDebug() << "Failed to open file for writing";
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("生成真彩色转换任务单失败: ") + xmlPath);

				emit FunctionAbnormalExit(QString::fromLocal8Bit("生成真彩色转换任务单失败: ") + xmlPath);
				return;
			}

			QTextStream out(&file);
			out.setCodec("UTF-8");
			doc.save(out, 4);
			file.close();

			// 添加到任务队列 
			TaskInfo task;
			task.taskFilePath = xmlPath;
			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageOutByteCmd.exe");
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSImageOutByteCmd.x"));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSImageOutByteCmd.exe"));
#endif
			task.outfilename = outfilename;
			m_taskQueue.enqueue(task);
			m_totalTasks++;

			if (imagePath.empty())
			{
				// 查找文件 
				auto itnad = imageInterPath.find(filename.key());
				if (itnad != imageInterPath.end()) {
					itnad.value() = true;
				}
			}

			PSindex++;
		//}
	}
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("真彩色转换开始执行") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	if (!ConvertTo8Bitflag ) {
		// 记录开始时间 
		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	}
	// 开始处理任务 
	startNextTasks(protitle, true);
}

/**
 * @brief 影像匀色
 */
void SystemConfig::ImageColorCorrection(QStringList imagePath)
{
	if (imagePath.empty())
	{
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色输入待处理数据为空!"));

		emit FunctionAbnormalExit(QString::fromLocal8Bit("影像匀色输入数据为空"));
		return;
	}

	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有影像匀色"))) {
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("影像匀色"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}

	setProjectdir();
	
	logEdit->append(QString::fromLocal8Bit("****影像匀色****"));

	QString outputDir;
	if (ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir = projectdir + "Dodging/tmp";

		// 检查 task 文件夹是否存在，不存在则创建 
		QDir dir(outputDir);
		if (!dir.exists()) {
			dir.mkpath(".");
		}
	}
	else {
		outputDir = projectdir + "Dodging";
	}

	// 检查输出目录中是否已存在同名文件
	QString dodgingDir = projectdir + QString::fromLocal8Bit("Dodging") + "/";
	int duplicateCount = 0;
	QVector<bool> isDuplicate(imagePath.size(), false);

	for (int i = 0; i < imagePath.size(); ++i) {
		QFileInfo inputFile(imagePath[i]);
		QString outputFile = dodgingDir + inputFile.completeBaseName() + "." + inputFile.suffix();

		if (QFile::exists(outputFile)) {
			duplicateCount++;
			isDuplicate[i] = true;
		}
	}

	// 处理重复文件情况
	bool processAll = true;
	QStringList filteredImagePath;

	if (duplicateCount > 0) {
		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("文件已存在"),
			QString::fromLocal8Bit("已经存在%1个匀色成果，是否重新匀色?").arg(duplicateCount),
			QMessageBox::Yes | QMessageBox::No);

		processAll = (reply == QMessageBox::Yes);

		if (!processAll && duplicateCount == imagePath.size()) {
			// 所有文件都重复且用户选择不重新处理
			ModelMutex.unlock();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色: 用户选择不覆盖已有成果文件,已存在:总和 , ") + QString::number(duplicateCount) + " : " + QString::number(imagePath.size()));

			emit ImageColorCorrectionFinished();
			return;
		}

		// 如果选择不处理重复文件，则从输入列表中移除
		if (!processAll) {
			for (int i = 0; i < imagePath.size(); ++i) {
				if (!isDuplicate[i]) {
					filteredImagePath.append(imagePath[i]);
				}
			}
			//imagePath = filteredImagePath;
		}
		else
		{
			filteredImagePath = imagePath;
		}
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色: 已存在成果文件:总和 , ") + QString::number(duplicateCount) + " : " + QString::number(imagePath.size()));
	}
	else
	{
		filteredImagePath = imagePath;
	}

	QString tmpPath = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath();

	QString DodgingTemplateLibDir;
	int imageEventColor_useModuleDBBox = ui.imageEventColor_useModuleDBBox->currentIndex();
	switch (imageEventColor_useModuleDBBox)
	{
	case 0:
		DodgingTemplateLibDir = tmpPath + "/Software/etc/dodgingTemplate/china16_1/";
		break;
	case 1:
		DodgingTemplateLibDir = tmpPath + "/Software/etc/dodgingTemplate/china16_2/";
		break;
	case 2:
		DodgingTemplateLibDir = tmpPath + "/Software/etc/dodgingTemplate/china16_3/";
		break;
	case 3:
		DodgingTemplateLibDir = tmpPath + "/Software/etc/dodgingTemplate/china150/";
		break;
	case 4:
		DodgingTemplateLibDir = tmpPath + "/Software/etc/dodgingTemplate/global600/";
		break;
	case 5:
		DodgingTemplateLibDir = ui.lineEdit_21->text();
		break;
	default:
		logEdit->append(QString::fromLocal8Bit("****请完成影像匀色模板库文件配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成影像匀色模板库文件配置 "));

		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成影像匀色模板库文件配置  "));
		return;
	}

	QStringList xmlPath;
	QStringList NumOfTasks;
	QString freeXmlPath;
	int imageEventColor_eventLightColorFunBox = ui.imageEventColor_eventLightColorFunBox->currentIndex();
	switch (imageEventColor_eventLightColorFunBox)
	{
	case 0:
		xmlPath = createMaskDodgingFile(filteredImagePath, DodgingTemplateLibDir);
		if (xmlPath.empty())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("MASK匀光影像匀色任务单创建失败"));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("MASK匀光影像匀色任务单创建失败"));
			return;
		}
		break;
	case 1:
	case 5:
		freeXmlPath = createCalDodgingCoefFile(filteredImagePath);
		xmlPath = createFreeNetworkDodgingFile(filteredImagePath, DodgingTemplateLibDir);
		if (xmlPath.empty())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色任务单创建失败"));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("影像匀色任务单创建失败"));
			return;
		}
		break;
	case 2:
	case 3:
	case 4:
		xmlPath = createDodgingControlFile(filteredImagePath, DodgingTemplateLibDir);

		if (xmlPath.empty())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色任务单创建失败"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("影像匀色任务单创建失败"));
			return;
		}
		break; //createFreeNetworkDodgingPlusFile
	case 6:
		createFreeNetworkDodgingPlusFile(filteredImagePath, DodgingTemplateLibDir);

		break;
	default:
		logEdit->append(QString::fromLocal8Bit("****请完成影像匀色方法配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成影像匀色方法配置"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成影像匀色方法配置"));
		return;
	}
	if (imageEventColor_eventLightColorFunBox == 1)
	{
		QDateTime startTime = QDateTime::currentDateTime();
		logEdit->append(QString::fromLocal8Bit("****匀色系数计算开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

		//创建调用可执行程序的类
		QProcess * process1 = new QProcess(this);

		QString authCmd;
#ifdef Q_OS_LINUX 
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSCalDodgingCoefCmd.x"));
#else
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSCalDodgingCoefCmd.exe"));
#endif
		//启动程序，并输入参数
		process1->start(authCmd, QStringList() << freeXmlPath);
		//process1->start(m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSCalDodgingCoefCmd.exe"), QStringList() << freeXmlPath);
		m_systemProcesses.append(process1);
		// 2. 进程结束时自动显示 
		connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[=](int exitCode, QProcess::ExitStatus status) {
			m_systemProcesses.removeOne(process1);
			QDateTime endTime = QDateTime::currentDateTime();
			logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			logEdit->append(QString::fromLocal8Bit("****匀色系数计算任务总耗时：") + calculateTimeDifference(startTime, endTime));
			process1->deleteLater(); // 安全释放内存

			// 构建Dodging文件夹路径 
			QString dodgingDir = outputDir;

			// 构建完整的moni文件路径 
			QString moniFilePath = QDir(dodgingDir).filePath("CalDodgingCoefCmd.moni");

			if (QFileInfo::exists(moniFilePath))
			{
				// 初始化任务队列和相关参数
				m_taskQueue.clear();
				m_runningProcesses.store(0);
				m_completedTasks.store(0);
				m_startTime = QDateTime::currentDateTime();
				m_totalTasks = xmlPath.size();

				for (auto filename : xmlPath)
				{
					// 将任务添加到队列
					TaskInfo task;
					task.taskFilePath = filename;
					//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSDodgingCmd.exe");
#ifdef Q_OS_LINUX 
					task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSDodgingCmd.x"));
#else
					task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSDodgingCmd.exe"));
#endif
					m_taskQueue.enqueue(task);
				}
				PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
				logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
				startNextTasks(QString::fromLocal8Bit("****影像匀色"));
			}
			else {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("匀色系数计算任务执行失败,未生成对应moni文件 "));
			}
		});
	}
	else if (imageEventColor_eventLightColorFunBox == 6)
	{
		QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);
 
		QObject::connect(this, &SystemConfig::allIntersectCmdsFinished, this, [this]() {
			QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);
			if (FreeNetwork_PSDodging_plusXmlFiles.empty())
			{
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色任务单创建失败"));

				emit FunctionAbnormalExit(QString::fromLocal8Bit("影像匀色"));
				return;
			}
			// 初始化任务队列和相关参数
			m_taskQueue.clear();
			m_runningProcesses.store(0);
			m_completedTasks.store(0);
			m_startTime = QDateTime::currentDateTime();
			m_totalTasks = FreeNetwork_PSDodging_plusXmlFiles.size();

			for (auto filename : FreeNetwork_PSDodging_plusXmlFiles)
			{
				// 将任务添加到队列
				TaskInfo task;
				task.taskFilePath = filename;
				//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSDodgingCmd.exe");
#ifdef Q_OS_LINUX 
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSDodgingCmd.x"));
#else
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSDodgingCmd.exe"));
#endif
				m_taskQueue.enqueue(task);
			}
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

			logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			startNextTasks(QString::fromLocal8Bit("****影像匀色"));

		});
	}
	else
	{	
		// 初始化任务队列和相关参数
		m_taskQueue.clear();
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();
		m_totalTasks = xmlPath.size();

		for (auto filename : xmlPath)
		{
			// 将任务添加到队列
			TaskInfo task;
			task.taskFilePath = filename;
			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSDodgingCmd.exe");
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSDodgingCmd.x"));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSDodgingCmd.exe"));
#endif
			m_taskQueue.enqueue(task);
		}
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		startNextTasks(QString::fromLocal8Bit("****影像匀色"));
	}
}

/**
 * @brief 影像镶嵌
 */
void SystemConfig::ImageMosaic(QStringList imagePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有影像镶嵌"))) {
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("影像镶嵌"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****影像镶嵌****"));

	// 检查是否存在镶嵌成果文件 
	QString mosaicDir = projectdir + QString::fromLocal8Bit("Mosaic") + "/";
	QString mosaicTif = mosaicDir + "mosaic.tif";
	QString mosaicImg = mosaicDir + "mosaic.img";

	bool mosaicExists = QFile::exists(mosaicTif) || QFile::exists(mosaicImg);

	if (mosaicExists) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("镶嵌成果已存在"),
			QString::fromLocal8Bit("已经存在镶嵌成果，是否重新转换?"),
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::No) {
			ModelMutex.unlock();
			emit ImageMosaicFinished();
			return;
		}
	}

	if (imagePath.empty())
	{
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像镶嵌输入待处理数据为空!"));

		emit FunctionAbnormalExit(QString::fromLocal8Bit("影像镶嵌输入待处理数据为空"));
		return;
	}
	int ResultCreatePnum = 0;
	bool ResultCreateP = ui.checkBox_25->isChecked();
	if (ResultCreateP)
		ResultCreatePnum = 1;
	else
		ResultCreatePnum = 0;
	int ResultImageFileTypenum = ui.imageInlay_resultFormatBox->currentIndex();
	QString outfiletype;
	switch (ResultImageFileTypenum)
	{
	case 0:
		outfiletype = QString::fromLocal8Bit(".tif");
		break;
	case 1:
		outfiletype = QString::fromLocal8Bit(".img");
		break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成影像镶嵌输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成影像镶嵌输出文件格式配置!"));

		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成影像镶嵌输出文件格式配置"));
		return;
	}
	QString outputFilePath = projectdir + QString::fromLocal8Bit("Mosaic") + "/" + "mosaiclist.txt";

	QFile file(outputFilePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像镶嵌任务单创建失败"));

		emit FunctionAbnormalExit(QString::fromLocal8Bit("影像镶嵌任务单创建失败"));
		return ;
	}

	QTextStream out(&file);
	out.setCodec("ANSI");   

	// 写入每个影像路径 
	for (const QString &path : imagePath) {
		QString tmp = path;
		out << tmp << "\n";
	}

	file.close();

	static QAtomicInt completedTasks(0);
	completedTasks.store(0);
	QDateTime startTime = QDateTime::currentDateTime();
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	logEdit->repaint();

	//创建调用可执行程序的类
	QProcess * process1 = new QProcess(this);
	QString authCmd;
#ifdef Q_OS_LINUX 
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSMosaic/PSMosaic.x"));
#else
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/PSMosaic/PSMosaic.exe"));
#endif
	//启动程序，并输入参数
	process1->start(authCmd, QStringList() << outputFilePath << projectdir + QString::fromLocal8Bit("Mosaic/mosaic") + outfiletype << QString::number(ResultCreatePnum));
	m_systemProcesses.append(process1);
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像镶嵌开始执行: ") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	// 2. 进程结束时自动显示 
	connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		completedTasks.fetchAndAddOrdered(1);  // 原子递增
		m_systemProcesses.removeOne(process1);
		completedTasks.store(0);
		QDateTime endTime = QDateTime::currentDateTime();
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像镶嵌结束时间: ") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

		logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		logEdit->append(QString::fromLocal8Bit("****影像镶嵌总耗时：") + calculateTimeDifference(startTime, endTime));
		ModelMutex.unlock();
		emit ImageMosaicFinished();
		process1->deleteLater(); // 安全释放内存
	});
}

/**
 * @brief 智能镶嵌
 */
void SystemConfig::SmartMosaic(QStringList imagePath, QStringList CropFilePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有智能镶嵌"))) {
		return;
	}

	if (!m_imagePS->getWorkProcessflag()) {
		// 使用封装后的函数显示界面并等待用户确认 
		int result = showFunctionAndWait(QString::fromLocal8Bit("智能镶嵌"));

		// 用户取消操作
		if (result != QDialog::Accepted) {
			ModelMutex.unlock();
			return;
		}
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****智能镶嵌****"));
	IntersectCmdPaths.clear();
	// 检查是否存在镶嵌成果文件 
	QString mosaicDir = projectdir + QString::fromLocal8Bit("SmartMosaic") + "/";
	QString mosaicTif = mosaicDir + "SmartMosaic.tif";
	QString mosaicImg = mosaicDir + "SmartMosaic.img";
	QString mosaicTif_index = mosaicDir + "SmartMosaic_0.tif";
	QString mosaicImg_index = mosaicDir + "SmartMosaic_0.img";

	bool mosaicExists = QFile::exists(mosaicTif) || QFile::exists(mosaicImg)|| QFile::exists(mosaicTif_index)|| QFile::exists(mosaicImg_index);

	if (mosaicExists) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("镶嵌成果已存在"),
			QString::fromLocal8Bit("已经存在镶嵌成果，是否重新转换?"),
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::No) {
			ModelMutex.unlock();
			emit SmartMosaicFinished();
			return;
		}
	}

	if (imagePath.empty())
	{
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("智能镶嵌输入待处理数据为空!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("智能镶嵌输入待处理数据为空"));
		return;
	}

	QString outputFilePath = projectdir + QString::fromLocal8Bit("SmartMosaic") + "/" + "SmartMosaicList.txt";

	QFile file(outputFilePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建智能镶嵌影像列表文件SmartMosaicList.txt文件失败!"));

		emit FunctionAbnormalExit(QString::fromLocal8Bit("创建智能镶嵌影像列表文件SmartMosaicList.txt文件失败"));
		return;
	}

	QTextStream out(&file);
	out.setCodec("ANSI");

	// 写入每个影像路径 
	for (const QString &path : imagePath) {
		QString tmp = path;
		out << tmp << "\n";
	}

	file.close();

	bool mosaicToopFile = ui.CheckBox_3->isChecked();
	QString mosaicToopdir = ui.LineEdit_2->text();
	QString attriID = ui.LineEdit_3->text();

	if (!mosaicToopFile)//已经存在拓扑网文件路径
	{
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("智能镶嵌: 已设置 影像拓扑网目录"));
		if (!CropFilePath.empty())
		{
			ExcuteExeCutVector(CropFilePath[0], projectdir + QString::fromLocal8Bit("SmartMosaic") + "/", attriID);
			disconnect(this, &SystemConfig::ExcuteExeCutVectorFinished, nullptr, nullptr);
			connect(this, &SystemConfig::ExcuteExeCutVectorFinished, this,
				[=]() {
				disconnect(this, &SystemConfig::ExcuteExeCutVectorFinished, nullptr, nullptr);
				ExcuteExeMuchXQMosaicX(imagePath, mosaicToopdir);
			});

		}
		else
		{
			ExcuteExetopo(outputFilePath, mosaicToopdir);
		}
	}
	else
	{
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("智能镶嵌: 未设置 影像拓扑网目录"));

		mosaicToopdir = projectdir + QString::fromLocal8Bit("SmartMosaic/topo.shp");
		if (!CropFilePath.empty())
		{
			ExcuteExetopo(outputFilePath, mosaicToopdir, 1);
			disconnect(this, &SystemConfig::CreateTopologyFinished, nullptr, nullptr);
			connect(this, &SystemConfig::CreateTopologyFinished, this,
				[=]() {
				//ExcuteExeCutVector(projectdir + QString::fromLocal8Bit("SmartMosaic\\topo.shp"), projectdir + QString::fromLocal8Bit("SmartMosaic\\"));
				ExcuteExeCutVector(CropFilePath[0], projectdir + QString::fromLocal8Bit("SmartMosaic") + "/", attriID);
				disconnect(this, &SystemConfig::ExcuteExeCutVectorFinished, nullptr, nullptr);
				connect(this, &SystemConfig::ExcuteExeCutVectorFinished, this,
					[=]() {
					disconnect(this, &SystemConfig::ExcuteExeCutVectorFinished, nullptr, nullptr);
					ExcuteExeMuchXQMosaicX(imagePath, mosaicToopdir);
				});
			});
		}
		else
		{
			ExcuteExetopo(outputFilePath, mosaicToopdir);
		}
	}
}

/**
 * @brief 快速镶嵌
 */
void SystemConfig::QuickMosaic(QStringList imagePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有快速镶嵌"))) {
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("快速镶嵌"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****快速镶嵌****"));

	// 检查是否存在镶嵌成果文件 
	QString mosaicDir = projectdir + QString::fromLocal8Bit("Mosaic") + "/";
	QString mosaicTif = mosaicDir + "QuickMosaic.tif";
	QString mosaicImg = mosaicDir + "QuickMosaic.img";
	QString mosaicBil = mosaicDir + "QuickMosaic.bil";

	bool mosaicExists = QFile::exists(mosaicTif) || QFile::exists(mosaicImg) || QFile::exists(mosaicBil);

	if (mosaicExists) {
		QMessageBox::StandardButton reply;
		reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("镶嵌成果已存在"),
			QString::fromLocal8Bit("已经存在镶嵌成果，是否重新转换?"),
			QMessageBox::Yes | QMessageBox::No);

		if (reply == QMessageBox::No) {
			ModelMutex.unlock();
			emit ImageMosaicFinished();
			return;
		}
	}

	if (imagePath.empty())
	{
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("快速镶嵌输入待处理数据为空!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("快速镶嵌输入待处理数据为空"));
		return;
	}
	int ResultCreatePnum = 0;
	bool ResultCreateP = ui.checkBox_25->isChecked();
	if (ResultCreateP)
		ResultCreatePnum = 1;
	else
		ResultCreatePnum = 0;
	int ResultImageFileTypenum = ui.QuickMosaic_resultFormatBox->currentIndex();
	QString outfiletype;
	switch (ResultImageFileTypenum)
	{
	case 0:
		outfiletype = QString::fromLocal8Bit(".tif");
		break;
	case 1:
		outfiletype = QString::fromLocal8Bit(".img");
		break;
	case 2:
		outfiletype = QString::fromLocal8Bit(".bil");
		break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成快速镶嵌输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成快速镶嵌输出文件格式配置!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成快速镶嵌输出文件格式配置"));
		return;
	}

	bool CreatePydP = ui.checkBox_71->isChecked();

	QDomDocument doc;

	// 添加XML声明 
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 
	QDomElement root = doc.createElement("XQDemMosaicCmd");
	root.setAttribute("version", "1.0");
	doc.appendChild(root);

	// 添加各子节点
	QDomElement InputFiles = doc.createElement("InputFiles");
	root.appendChild(InputFiles);

	for (auto filepath : imagePath)
	{
		QDomElement ImageFile = doc.createElement("ImageFile");
		ImageFile.appendChild(doc.createTextNode(filepath));
		InputFiles.appendChild(ImageFile);

	}

	QDomElement outputFileName = doc.createElement("outputFileName");
	outputFileName.appendChild(doc.createTextNode(projectdir + QString::fromLocal8Bit("Mosaic") + "/" + "QuickMosaic" + outfiletype));
	root.appendChild(outputFileName);

	QDomElement CreatePyd = doc.createElement("CreatePyd");
	CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
	root.appendChild(CreatePyd);

	// 保存XML文件
	QString xmlPath = projectdir + "Mosaic/QuickMosaic.xml";
	QFile file(xmlPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		logEdit->append(QString::fromLocal8Bit("****创建快速镶嵌任务单失败"));
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建快速镶嵌任务单失败!"));

		ModelMutex.unlock();
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();

	static QAtomicInt completedTasks(0);
	completedTasks.store(0);
	QDateTime startTime = QDateTime::currentDateTime();
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	logEdit->repaint();

	//创建调用可执行程序的类
	QProcess * process1 = new QProcess(this);
	QString authCmd;
#ifdef Q_OS_LINUX 
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSDemMosaicCmd.x"));
#else
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSDemMosaicCmd.exe"));
#endif
	//启动程序，并输入参数
	process1->start(authCmd, QStringList() << xmlPath);
	m_systemProcesses.append(process1);

	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("快速镶嵌开始执行: ") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	// 2. 进程结束时自动显示 
	connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		completedTasks.fetchAndAddOrdered(1);  // 原子递增
		m_systemProcesses.removeOne(process1);
		completedTasks.store(0);
		QDateTime endTime = QDateTime::currentDateTime();
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("快速镶嵌执行结束...") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

		logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		logEdit->append(QString::fromLocal8Bit("****快速镶嵌总耗时：") + calculateTimeDifference(startTime, endTime));
		ModelMutex.unlock();
		emit ImageMosaicFinished();
		process1->deleteLater(); // 安全释放内存
	});
}


/**
 * @brief 智能镶嵌以及生成镶嵌线
 */
void SystemConfig::ExcuteExetopo(QString mosaicListFile, QString mosaicToopfile, int GenLineOnlyisNo)
{
	bool mosaicToopFile = ui.CheckBox_3->isChecked();
	bool createPyd = ui.CheckBox->isChecked();
	bool FindLine = ui.CheckBox_3->isChecked();
	bool ExtentIsEnvelope = ui.CheckBox_6->isChecked();
	bool GenLineOnly = ui.CheckBox_4->isChecked();
	bool smallOverlap = ui.CheckBox_5->isChecked();

	QString mosaicToopdir = ui.LineEdit_2->text();
	double gsd = ui.LineEdit->text().toDouble();
	int bufferWidth = ui.LineEdit_4->text().toInt();
	int resampleMethod = ui.SamplingMode_Box->currentIndex();
	int PrecisedMosaicLine = ui.ComboBox_4->currentIndex();

	int ResultImageFileTypenum = ui.resulttype_Box->currentIndex();
	QString outfiletype;
	switch (ResultImageFileTypenum)
	{
	case 0:
		outfiletype = QString::fromLocal8Bit(".tif");
		break;
	case 1:
		outfiletype = QString::fromLocal8Bit(".img");
		break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成影像镶嵌输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成影像镶嵌输出文件格式配置!"));	
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成影像镶嵌输出文件格式配置"));
		return;
	}

	QString filePath = projectdir + "SmartMosaic/SmartMosaicList.task";
	QString outfilePath = projectdir + "SmartMosaic" + "/" + "SmartMosaic" + outfiletype;
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建智能镶嵌任务单SmartMosaicList.task文件失败!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("创建智能镶嵌任务单失败"));
		//qWarning() << "无法打开文件:" << file.errorString();
		return;
	}

	QTextStream out(&file);
	out.setCodec("ANSI");   // 设置编码 

	// 根据条件写入不同内容 
	if (!mosaicToopFile && !mosaicToopdir.isEmpty()) {
		out << "mosaicListFile:" << mosaicListFile << "\n"
			<< "mosaicResult:" << outfilePath << "\n"
			<< "mosaicToopFile:" << mosaicToopdir << "\n"
			<< "gsd:" << QString::number(gsd, 'f', 6) << "\n"
			<< "createPyd:" << createPyd  << "\n"
			<< "FindLine:" << FindLine << "\n"
			<< "ExtentIsEnvelope:" << ExtentIsEnvelope << "\n"
			<< "resampleMethod:" << resampleMethod << "\n"
			<< "GenLineOnly:" << GenLineOnly << "\n"
			<< "smallOverlap:" << smallOverlap << "\n"
			<< "bufferWidth:" << bufferWidth << "\n"
			<< "PrecisedMosaicLine:" << PrecisedMosaicLine << "\n";
	}
	else if (GenLineOnlyisNo == 1) {
		out << "mosaicListFile:" << mosaicListFile << "\n"
			<< "mosaicResult:" << outfilePath << "\n"
			<< "gsd:" << QString::number(gsd, 'f', 6) << "\n"
			<< "createPyd:" << createPyd << "\n"
			<< "FindLine:" << FindLine << "\n"
			<< "ExtentIsEnvelope:" << ExtentIsEnvelope << "\n"
			<< "resampleMethod:" << resampleMethod << "\n"
			<< "GenLineOnly:" << GenLineOnlyisNo << "\n"
			<< "smallOverlap:" << smallOverlap << "\n"
			<< "bufferWidth:" << bufferWidth << "\n"
			<< "PrecisedMosaicLine:" << PrecisedMosaicLine << "\n";
	}
	else {
		out << "mosaicListFile:" << mosaicListFile << "\n"
			<< "mosaicResult:" << outfilePath << "\n"
			<< "gsd:" << QString::number(gsd, 'f', 6) << "\n"
			<< "createPyd:" << createPyd << "\n"
			<< "FindLine:" << FindLine << "\n"
			<< "ExtentIsEnvelope:" << ExtentIsEnvelope << "\n"
			<< "resampleMethod:" << resampleMethod << "\n"
			<< "GenLineOnly:" << GenLineOnly << "\n"
			<< "smallOverlap:" << smallOverlap << "\n"
			<< "bufferWidth:" << bufferWidth << "\n"
			<< "PrecisedMosaicLine:" << PrecisedMosaicLine << "\n";
	}

	file.close();


	if (mosaicToopFile && (GenLineOnlyisNo == 1 || GenLineOnly))
	{
		//创建调用可执行程序的类
		QProcess * process = new QProcess(this);
		QDateTime startTime = QDateTime::currentDateTime();
		QString authCmd;
#ifdef Q_OS_LINUX 
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSMosaic/PSMosaicX.x"));
#else
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/PSMosaic/PSMosaicX.exe"));
#endif
		//启动程序，并输入参数
		process->start(authCmd, QStringList() << filePath);
		m_systemProcesses.append(process);
		if(GenLineOnly)
			logEdit->append(QString::fromLocal8Bit("****生成镶嵌线开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("生成镶嵌线开始时间: ") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		// 2. 进程结束时自动显示 
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[=](int exitCode, QProcess::ExitStatus status) {
			m_systemProcesses.removeOne(process);
			QDateTime endTime = QDateTime::currentDateTime();
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("生成镶嵌线结束时间: ") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			if (GenLineOnly)
			{
				logEdit->append(QString::fromLocal8Bit("****生成镶嵌线结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
				logEdit->append(QString::fromLocal8Bit("****生成镶嵌线总耗时：") + calculateTimeDifference(startTime, endTime));
				ModelMutex.unlock();
				emit SmartMosaicFinished();
			}
			else
			{
				emit CreateTopologyFinished();
			}
			process->deleteLater(); // 安全释放内存
		});
	}
	else
	{
		// 初始化任务队列和相关参数
		m_taskQueue.clear();
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();
		m_totalTasks = 1;

		// 将任务添加到队列
		TaskInfo task;
		task.taskFilePath = filePath;
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSMosaic/PSMosaicX.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/PSMosaic/PSMosaicX.exe"));
#endif
		task.outfilename = outfilePath;
		m_taskQueue.enqueue(task);

		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		startNextTasks(QString::fromLocal8Bit("****智能镶嵌"));
	}
}

/**
 * @brief 分幅智能镶嵌
 */
void SystemConfig::ExcuteExeMuchXQMosaicX(QStringList imagePath, QString mosaicToopfile)
{
	bool mosaicToopFile = ui.CheckBox_3->isChecked();
	bool createPyd = ui.CheckBox->isChecked();
	bool FindLine = ui.CheckBox_3->isChecked();
	bool ExtentIsEnvelope = ui.CheckBox_6->isChecked();
	bool GenLineOnly = ui.CheckBox_4->isChecked();
	bool smallOverlap = ui.CheckBox_5->isChecked();
	bool MultipleClippingVectorsShare = ui.CheckBox_7->isChecked();

	QString mosaicToopdir = ui.LineEdit_2->text();
	double gsd = ui.LineEdit->text().toDouble();
	int bufferWidth = ui.LineEdit_4->text().toInt();
	int resampleMethod = ui.SamplingMode_Box->currentIndex();
	int PrecisedMosaicLine = ui.ComboBox_4->currentIndex();

	int ResultImageFileTypenum = ui.resulttype_Box->currentIndex();
	QString outfiletype;
	switch (ResultImageFileTypenum)
	{
	case 0:
		outfiletype = QString::fromLocal8Bit(".tif");
		break;
	case 1:
		outfiletype = QString::fromLocal8Bit(".img");
		break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成智能镶嵌输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成智能镶嵌输出文件格式配置"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成智能镶嵌输出文件格式配置"));
		return;
	}
	QString directoryName = projectdir + QString::fromLocal8Bit("SmartMosaic") + "/";
	QString mosaicToopFilename;
	// 查找拓扑文件 
	if (mosaicToopFile) {
		QString MapSplitListfileName = "topo.shp";
		QDir dir(directoryName);
		QStringList files = dir.entryList(QStringList() << MapSplitListfileName, QDir::Files);

		if (!files.isEmpty()) {
			mosaicToopFilename = dir.filePath(files.first());
		}
		else {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("未找到系统自动生成topo.shp 文件"));

			//logEdit->append(QString::fromLocal8Bit("未找到系统自动生成topo.shp 文件"));
		}
	}

	// 查找MapSplitList.txt 文件 
	QString fileName = "MapSplitList.txt";
	QDir dir(directoryName);
	QStringList files = dir.entryList(QStringList() << fileName, QDir::Files);

	if (files.isEmpty()) {
		logEdit->append(QString::fromLocal8Bit("未找到MapSplitList.txt 文件"));
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("未找到MapSplitList.txt 文件"));

	}

	QString mapSplitListPath = dir.filePath(files.first());
	QFile file(mapSplitListPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("无法打开MapSplitList.txt 文件"));
		logEdit->append(QString::fromLocal8Bit("无法打开MapSplitList.txt 文件"));
	}

	QTextStream in(&file);
	QStringList filePaths;
	bool isFirstLine = true;
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (isFirstLine) {
			isFirstLine = false;
			continue;
		}
		if (!line.isEmpty()) {
			filePaths.append(line);
		}
	}
	file.close();


	SmartMosaiccurrentIndex = 0; // 当前处理的图像索引 
	SmartMosaiclocalInputPaths = imagePath; // 保存输入路径 
	//// 初始化任务队列和相关参数
	//m_taskQueue.clear();
	//m_runningProcesses.store(0);
	//m_completedTasks.store(0);
	// 初始化处理 
	if (SmartMosaiccurrentIndex == 0) {
		ExcuteExeMuchXQMosaicXmlFiles.clear();
		QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);

		// 建立新连接 - 每次IntersectCmd完成后处理 
		QObject::connect(this, &SystemConfig::intersectCmdFinished, this, [=]() {

			QString mosaicListFile = IntersectCmdPaths[0];
			QString outputFilePath = projectdir + QString::fromLocal8Bit("SmartMosaic") + "/" + "SmartMosaicList_" + QString::number(SmartMosaiccurrentIndex) + QString::fromLocal8Bit(".txt");  // 输出文件路径 

			// 同时打开输入文件和输出文件
			QFile targetFile(mosaicListFile);
			QFile outputFile(outputFilePath);

			if (!targetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("无法打开分幅智能镶嵌求交成果文件..."));

				//qWarning() << u8"无法打开目标文件:" << targetFile.errorString();
				return;
			}

			if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("无法创建分幅智能镶嵌影像列表文件..."));

				//qWarning() << u8"无法创建输出文件:" << outputFile.errorString();
				targetFile.close();
				return;
			}

			QTextStream in(&targetFile);
			QTextStream out(&outputFile);
			out.setCodec("ANSI");  // 设置编码

			int lineCount = 0;
			int totalLines = 0;

			while (!in.atEnd()) {
				QString line = in.readLine().trimmed();
				lineCount++;

				// 第一行是文件总数（跳过不写入）
				if (lineCount == 1) {
					totalLines = line.toInt();
					continue;
				}
				// 写入SmartMosaicList.txt （每行一个路径）
				out << line << "\n";
			}

			// 关闭文件
			targetFile.close();
			outputFile.close();

			// 验证写入结果 
			if (QFileInfo(outputFilePath).size() == 0) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("分幅智能镶嵌影像列表文件写入失败,文件为空..."));

				//qWarning() << u8"输出文件写入失败，文件为空";
			}

			// 处理当前索引对应的结果
			QString filePath = projectdir + "SmartMosaic/SmartMosaicList_" + QString::number(SmartMosaiccurrentIndex) + ".task";
			QString outfilePath = projectdir + "SmartMosaic" + "/" + "SmartMosaic_" + QString::number(SmartMosaiccurrentIndex) + outfiletype;
			QFile file1(filePath);
			if (!file1.open(QIODevice::WriteOnly | QIODevice::Text)) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建分幅智能镶嵌任务单失败"));

				//qWarning() << "无法打开文件:" << file1.errorString();
				return;
			}

			QTextStream out1(&file1);
			out1.setCodec("ANSI");   // 设置编码 
			QString mosaicToopFilepath = filePaths[SmartMosaiccurrentIndex];
			if (MultipleClippingVectorsShare)
			{
				out1 << "mosaicListFile:" << outputFilePath << "\n"
					<< "mosaicResult:" << outfilePath << "\n"
					<< "extentShpFile:" << mosaicToopFilepath << "\n"
					<< "mosaicToopFile:" << mosaicToopfile << "\n"
					<< "gsd:" << QString::number(gsd, 'f', 6) << "\n"
					<< "createPyd:" << createPyd << "\n"
					<< "FindLine:" << FindLine << "\n"
					<< "ExtentIsEnvelope:" << ExtentIsEnvelope << "\n"
					<< "resampleMethod:" << resampleMethod << "\n"
					<< "GenLineOnly:" << GenLineOnly << "\n"
					<< "smallOverlap:" << smallOverlap << "\n"
					<< "bufferWidth:" << bufferWidth << "\n"
					<< "PrecisedMosaicLine:" << PrecisedMosaicLine << "\n";
			}
			else
			{
				out1 << "mosaicListFile:" << outputFilePath << "\n"
					<< "mosaicResult:" << outfilePath << "\n"
					<< "extentShpFile:" << mosaicToopFilepath << "\n"
					<< "gsd:" << QString::number(gsd, 'f', 6) << "\n"
					<< "createPyd:" << createPyd << "\n"
					<< "FindLine:" << FindLine << "\n"
					<< "ExtentIsEnvelope:" << ExtentIsEnvelope << "\n"
					<< "resampleMethod:" << resampleMethod << "\n"
					<< "GenLineOnly:" << GenLineOnly << "\n"
					<< "smallOverlap:" << smallOverlap << "\n"
					<< "bufferWidth:" << bufferWidth << "\n"
					<< "PrecisedMosaicLine:" << PrecisedMosaicLine << "\n";
			}

			file1.close();

			if (!filePath.isEmpty()) {
				ExcuteExeMuchXQMosaicXmlFiles.append(filePath);
			}

			// 处理下一个图像 
			SmartMosaiccurrentIndex++;
			if (SmartMosaiccurrentIndex < filePaths.size()) {

				QStringList remainingPaths;
				remainingPaths.append(filePaths[SmartMosaiccurrentIndex]);

				if (!remainingPaths.isEmpty()) {
					IntersectCmd(SmartMosaiclocalInputPaths, remainingPaths, QString::fromLocal8Bit("SmartMosaic"), QString::fromLocal8Bit("SmartMosaic"));
				}
			}
			else {
				QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
				// 所有处理完成，重置索引 
				SmartMosaiccurrentIndex = 0;
				emit allIntersectCmdsFinished(); // 发射所有处理完成的信号 
			}

		});

		// 开始处理第一个图像
		if (!SmartMosaiclocalInputPaths.empty()) {

			QStringList remainingPaths;
			remainingPaths.append(filePaths[SmartMosaiccurrentIndex]);

			if (!remainingPaths.isEmpty()) {
				IntersectCmd(SmartMosaiclocalInputPaths, remainingPaths, QString::fromLocal8Bit("SmartMosaic"), QString::fromLocal8Bit("SmartMosaic"));
			}
		}
	}

	QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);

	QObject::connect(this, &SystemConfig::allIntersectCmdsFinished, this, [this]() {
		QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);
		// 初始化任务队列和相关参数
		m_taskQueue.clear();
		m_runningProcesses.store(0);
		m_completedTasks.store(0);
		m_startTime = QDateTime::currentDateTime();
		m_totalTasks = ExcuteExeMuchXQMosaicXmlFiles.size();

		for (auto filename : ExcuteExeMuchXQMosaicXmlFiles)
		{
			// 将任务添加到队列
			TaskInfo task;
			task.taskFilePath = filename;
			//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/PSMosaic/PSMosaicX.exe");
#ifdef Q_OS_LINUX 
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSMosaic/PSMosaicX.x"));
#else
			task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/PSMosaic/PSMosaicX.exe"));
#endif
			m_taskQueue.enqueue(task);
		}
		if (m_taskQueue.empty())
		{
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("分幅智能镶嵌任务单创建失败??? 分幅智能镶嵌任务队列为空"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("分幅智能镶嵌任务队列为空"));
			return;
		}

		logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		startNextTasks(QString::fromLocal8Bit("****智能镶嵌"));

	});
}

/**
 * @brief 裁切矢量
 */
void SystemConfig::ExcuteExeCutVector(QString imagePath, QString OutputDir, QString attriID)
{
	setProjectdir();

	QDomDocument doc;

	// 添加XML声明 
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 
	QDomElement root = doc.createElement("XQMapSpitCmd");
	doc.appendChild(root);

	// 添加各子节点
	QDomElement inputImageFile = doc.createElement("inputShpFilePath");
	inputImageFile.appendChild(doc.createTextNode(imagePath));
	root.appendChild(inputImageFile);

	QDomElement outputDir = doc.createElement("outputDirPath");
	outputDir.appendChild(doc.createTextNode(OutputDir));
	root.appendChild(outputDir);

	QDomElement nOutPutFormat = doc.createElement("attriID");
	nOutPutFormat.appendChild(doc.createTextNode(attriID)); 
	root.appendChild(nOutPutFormat);

	// 保存XML文件
	QString xmlPath = projectdir + "SmartMosaic/PSMapSpitCmd.xml";
	QFile file(xmlPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		//logEdit->append(QString::fromLocal8Bit("****无法创建裁切矢量任务单****"));
		ModelMutex.unlock();
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("无法创建裁切矢量任务单"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("无法创建裁切矢量任务单"));
		return;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();

	//创建调用可执行程序的类
	QProcess * process1 = new QProcess(this);
	QString authCmd;
#ifdef Q_OS_LINUX 
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSMapSpitCmd.x"));
#else
	authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSMapSpitCmd.exe"));
#endif
	//启动程序，并输入参数
	process1->start(authCmd, QStringList() << xmlPath);
	m_systemProcesses.append(process1);
	// 2. 进程结束时自动显示 
	connect(process1, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
		[=](int exitCode, QProcess::ExitStatus status) {
		m_systemProcesses.removeOne(process1);
		emit ExcuteExeCutVectorFinished();
		process1->deleteLater(); // 安全释放内存
	});
}

/**
 * @brief 投影转换
 */
void SystemConfig::ProjectionTransformation(QStringList imagePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有投影转换"))) {
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("投影转换"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****投影转换****"));

	int ResultImageFileTypenum = ui.projectChange_resultFormatBox->currentIndex();
	QString ResultImageFileType;
	switch (ResultImageFileTypenum) {
	case 0: ResultImageFileType = QString::fromLocal8Bit(".tif"); break;
	case 1: ResultImageFileType = QString::fromLocal8Bit(".img"); break;
	case 2: ResultImageFileType = QString::fromLocal8Bit(".pix"); break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成投影转换输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成投影转换输出文件格式配置!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成投影转换输出文件格式配置!"));
		return;
	}
	int ResamplingKernelnum = ui.projectChange_samplingWayBox->currentIndex();
	QString ResamplingKernelstr;
	switch (ResamplingKernelnum) {
	case 0: ResamplingKernelstr = "Nearest"; break;
	case 1: ResamplingKernelstr = "BiLinear"; break;
	case 2: ResamplingKernelstr = "BiCubic"; break;
	}

	// 初始化任务队列
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;

	bool CreatePydP = ui.checkBox_37->isChecked();
	double GSDXP = ui.lineEdit_29->text().toDouble();
	double GSDYP = ui.lineEdit_31->text().toDouble();
	QString wktHCSP = ui.lineEdit_18->text();

	for (auto filepath : imagePath)
	{
		QFileInfo fileInfo(filepath);
		QDomDocument doc;

		// 添加XML声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 
		QDomElement root = doc.createElement("ImageReproject");
		root.setAttribute("version", "1.0");
		doc.appendChild(root);

		// 添加各子节点
		QDomElement InputImageFilePath = doc.createElement("InputImageFilePath");
		InputImageFilePath.appendChild(doc.createTextNode(filepath));
		root.appendChild(InputImageFilePath);

		QDomElement OutputImageFilePath = doc.createElement("OutputImageFilePath");
		OutputImageFilePath.appendChild(doc.createTextNode(projectdir + "ImageReproject" + "/" + fileInfo.completeBaseName() + ResultImageFileType));
		root.appendChild(OutputImageFilePath);

		QDomElement Param = doc.createElement("Param");
		root.appendChild(Param);

		QDomElement GSDX = doc.createElement("GSDX");
		GSDX.appendChild(doc.createTextNode(QString::number(GSDXP)));
		Param.appendChild(GSDX);

		QDomElement GSDY = doc.createElement("GSDY");
		GSDY.appendChild(doc.createTextNode(QString::number(GSDYP)));
		Param.appendChild(GSDY);

		QDomElement wktHCS = doc.createElement("wktHCS");
		wktHCS.appendChild(doc.createTextNode(wktHCSP));
		Param.appendChild(wktHCS);

		QDomElement ResamplingKernel = doc.createElement("ResamplingKernel");
		ResamplingKernel.appendChild(doc.createTextNode(ResamplingKernelstr));
		Param.appendChild(ResamplingKernel);

		QDomElement Output8bits = doc.createElement("Output8bits");
		Output8bits.appendChild(doc.createTextNode("flase"));
		Param.appendChild(Output8bits);

		QDomElement CreatePyd = doc.createElement("CreatePyd");
		CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
		Param.appendChild(CreatePyd);

		QDomElement Tasks = doc.createElement("Tasks");
		Tasks.appendChild(doc.createTextNode("1"));
		Param.appendChild(Tasks);

		// 保存XML文件
		QString xmlPath = projectdir + "ImageReproject/" + fileInfo.completeBaseName() + ".xml";
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			//logEdit->append(QString::fromLocal8Bit("****无法创建投影转换配置文件****"));
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建投影转换任务单失败!"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("创建投影转换任务单失败!"));
			ModelMutex.unlock();
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		doc.save(out, 4);
		file.close();

		TaskInfo task;
		task.taskFilePath = xmlPath;
		//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageReprojectCmd.exe");
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSImageReprojectCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSImageReprojectCmd.exe"));
#endif
		task.outfilename = projectdir + "ImageReproject" + "/" + fileInfo.completeBaseName() + ResultImageFileType;
		m_taskQueue.enqueue(task);
		m_totalTasks++;
	}
	// 记录开始时间 
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("投影转换开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****投影转换"), true);
}

/**
 * @brief 格式转换
 */
void SystemConfig::FormatConversion(QStringList imagePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有格式转换"))) {
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("格式转换"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****格式转换****"));

	int ResultImageFileTypenum = ui.resultType_Box->currentIndex();
	QString ResultImageFileType;
	switch (ResultImageFileTypenum) {
	case 0: ResultImageFileType = QString::fromLocal8Bit(".tif"); break;
	case 1: ResultImageFileType = QString::fromLocal8Bit(".img"); break;
	case 2: ResultImageFileType = QString::fromLocal8Bit(".jpg"); break;
	case 3: ResultImageFileType = QString::fromLocal8Bit(".pix"); break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成格式转换输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成格式转换输出文件格式配置!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成格式转换输出文件格式配置"));
		return;
	}
	int Depthlnum = ui.Bitdepth_Box->currentIndex();

	// 初始化任务队列
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;

	bool CreatePydP = ui.checkBox_69->isChecked();
	bool IsDemP = ui.radioButton_4->isChecked();
	bool EtmCorrectp = ui.checkBox_66->isChecked();
	//bool T2SP = ui.radioButton_4->isChecked();

	bool T2SP = false;
	if (ui.checkBox_67->isChecked()) {
		T2SP = true;
	}
	else if (ui.checkBox_68->isChecked()) {
		T2SP = false;
	}
	bool SpecifyDepthP = ui.checkBox_70->isChecked();

	for (auto filepath : imagePath)
	{
		QFileInfo fileInfo(filepath);
		QDomDocument doc;

		// 添加XML声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 
		QDomElement root = doc.createElement("XQFormatTransformCmd");
		root.setAttribute("version", "1.0");
		doc.appendChild(root);

		// 添加各子节点
		QDomElement InputImageFilePath = doc.createElement("InputImageFilePath");
		InputImageFilePath.setAttribute("type", "string");
		InputImageFilePath.appendChild(doc.createTextNode(filepath));
		root.appendChild(InputImageFilePath);

		QDomElement OutputImageFilePath = doc.createElement("OutputImageFilePath");
		OutputImageFilePath.setAttribute("type", "string");
		OutputImageFilePath.appendChild(doc.createTextNode(projectdir + "ImageFormatTransform" + "/" + fileInfo.completeBaseName() + ResultImageFileType));
		root.appendChild(OutputImageFilePath);

		QDomElement Param = doc.createElement("Param");
		root.appendChild(Param);

		QDomElement Output8bits = doc.createElement("Output8bits");
		Output8bits.setAttribute("type", "string");
		Output8bits.appendChild(doc.createTextNode("false"));
		Param.appendChild(Output8bits);

		QDomElement IsDem = doc.createElement("IsDem");
		IsDem.setAttribute("type", "string");
		IsDem.appendChild(doc.createTextNode(QVariant(IsDemP).toString()));
		Param.appendChild(IsDem);

		QDomElement CreatePyd = doc.createElement("CreatePyd");
		CreatePyd.setAttribute("type", "string");
		CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydP).toString()));
		Param.appendChild(CreatePyd);

		QDomElement EtmCorrect = doc.createElement("EtmCorrect");
		EtmCorrect.setAttribute("type", "string");
		EtmCorrect.appendChild(doc.createTextNode(QVariant(EtmCorrectp).toString()));
		Param.appendChild(EtmCorrect);

		QDomElement T2S = doc.createElement("T2S");
		T2S.setAttribute("type", "string");
		T2S.appendChild(doc.createTextNode(QVariant(T2SP).toString()));
		Param.appendChild(T2S);

		QDomElement SpecifyDepth = doc.createElement("SpecifyDepth");
		SpecifyDepth.appendChild(doc.createTextNode(QVariant(SpecifyDepthP).toString()));
		Param.appendChild(SpecifyDepth);

		QDomElement Depth = doc.createElement("Depth");
		Depth.appendChild(doc.createTextNode(QString::number(Depthlnum)));
		Param.appendChild(Depth);

		// 保存XML文件
		QString xmlPath = projectdir + "ImageFormatTransform/" + fileInfo.completeBaseName() + ".xml";
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			//logEdit->append(QString::fromLocal8Bit("****无法创建格式转换配置文件****"));
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建格式转换配置文件失败!"));

			ModelMutex.unlock();
			emit FunctionAbnormalExit(QString::fromLocal8Bit("创建格式转换配置文件失败!"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		doc.save(out, 4);
		file.close();

		TaskInfo task;
		task.taskFilePath = xmlPath;
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/XQFormatTransformCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSFormatTransformCmd.exe"));
#endif
		task.outfilename = projectdir + "ImageFormatTransform" + "/" + fileInfo.completeBaseName() + ResultImageFileType;
		m_taskQueue.enqueue(task);
		m_totalTasks++;
	}
	// 记录开始时间 
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("格式转换开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****格式转换"), true);
}

/**
 * @brief 绝对定位精度质检
 */
void SystemConfig::ImageAbsPositPrecCheck(QStringList DEMReferencefilename, QStringList DOMReferencefilename, QStringList SatelliteImagefilename, QMap<QString, bool>& AbsPositPrecCheckPath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有绝对定位精度质检"))) {
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("影像质检"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****绝对定位精度质检****"));

	// 使用成员变量替代局部变量跟踪任务状态 
	m_currentControlPointsTask = 0;

	// 使用QPointer确保安全访问 
	QPointer<SystemConfig> self(this);

	// 定义任务执行函数 
	auto executeNextTask = [self, DEMReferencefilename, DOMReferencefilename, SatelliteImagefilename, &AbsPositPrecCheckPath]() {
		if (!self) return;

		// 将连接对象声明为共享指针，使其能在Lambda中捕获 
		struct Connectors {
			QMetaObject::Connection conn1;
			QMetaObject::Connection conn2;
		};
		auto connectors = QSharedPointer<Connectors>::create();

		switch (self->m_currentControlPointsTask) {
		case 0: // 第一个任务 
			connectors->conn1 = QObject::connect(self.data(), &SystemConfig::intersectCmdFinished, self.data(),
				[self, DEMReferencefilename, DOMReferencefilename, SatelliteImagefilename, &AbsPositPrecCheckPath, connectors]() {
				if (!self) return;
				QObject::disconnect(connectors->conn1);
				self->m_currentControlPointsTask++;

				// 第二个任务
				connectors->conn2 = QObject::connect(self.data(), &SystemConfig::intersectCmdFinished, self.data(),
					[self, DOMReferencefilename, SatelliteImagefilename, &AbsPositPrecCheckPath, connectors]() {
					if (!self) return;
					QObject::disconnect(connectors->conn2);
					self->m_currentControlPointsTask++;

					int BlkSizeXnum = self->ui.lineEdit_10->text().toInt();
					int BlkSizeYnum = self->ui.lineEdit_11->text().toInt();
					int numOfBlksX = self->ui.lineEdit_6->text().toInt();
					int numOfBlksY = self->ui.lineEdit_7->text().toInt();
					int searchRANGEX = self->ui.lineEdit_8->text().toInt();
					int searchRANGEY = self->ui.lineEdit_9->text().toInt();
					int BandIndexwait = self->ui.imageMatch_waitMatchDataBandBox->currentIndex();
					int BandIndexreference = self->ui.imageMatch_referenceDataBandBox->currentIndex();
					int currentIndex = self->ui.imageMatch_controlMatchModeBox->currentIndex();
					bool tickPoint = self->ui.checkBox_5->isChecked();
					double tickPointThrehold = self->ui.lineEdit_22->text().toDouble();

					if (currentIndex == -1 || BandIndexwait == -1 || BandIndexreference == -1)
					{
						self->logEdit->append(QString::fromLocal8Bit("****请完成绝对定位精度质检系统配置!!!"));
						emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
						self->ModelMutex.unlock();
						return;
					}

					int index = 0;
					int index1 = 0;
					int indexDOM = 0;
					bool firstExecution = true;
					QDateTime startTime;
					int totalTasks = 0;
					AbsPositPrecCheckPath.clear();

					if (SatelliteImagefilename.empty())
					{
						self->ModelMutex.unlock();
						emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
						return;
					}

					// 计算总任务数   
					for (auto data : SatelliteImagefilename) {
						QStringList DOMpaths;
						QString filePathDOM = (self->projectdir + "PrecisionCheck" + "/" + "TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DOM") + ".txt");
						QFile fileTXTDOM(filePathDOM);

						if (!fileTXTDOM.open(QIODevice::ReadOnly | QIODevice::Text)) {
							qDebug() << "Failed to open file:" + filePathDOM;
							self->ModelMutex.unlock();
							emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
							return;
						}
						QTextStream inDOM(&fileTXTDOM);
						int lineNumberDOM = 0;
						while (!inDOM.atEnd()) {
							QString line = inDOM.readLine().trimmed();
							lineNumberDOM++;
							// 跳过第一行（数字行）
							if (lineNumberDOM == 1) {
								continue;
							}
							// 从第二行开始读取路径
							if (!line.isEmpty()) {
								DOMpaths.append(line);
							}
						}
						fileTXTDOM.close();
						totalTasks += DOMpaths.size();

						index1++;
					}
					index1 = 0;
					// 初始化成员变量
					self->m_taskQueue.clear();
					self->m_runningProcesses.store(0);
					self->m_completedTasks.store(0);
					self->m_startTime = QDateTime();
					self->m_totalTasks = totalTasks; // 保存总任务数

						// 确定操作程序名称
					QString operatorName;
					int tmpindex = currentIndex;
#ifdef Q_OS_LINUX 
					switch (tmpindex) {
					case 0: case 1:
						operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.x");
						break;
					case 2:
						operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.x");
						currentIndex = 4;
						break;
					case 3:
						operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.x");
						currentIndex = 5;
						break;
					case 4:
						operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.x");
						currentIndex = 6;
						break;
					default:
						self->ModelMutex.unlock();
						emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
						return;
				}
#else
					switch (tmpindex) {
					case 0: case 1:
						operatorName = self->m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
						break;
					case 2:
						operatorName = self->m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
						currentIndex = 4;
						break;
					case 3:
						operatorName = self->m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
						currentIndex = 5;
						break;
					case 4:
						operatorName = self->m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSGcpMatchMM.exe");
						currentIndex = 6;
						break;
					default:
						self->ModelMutex.unlock();
						emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
						return;
					}
#endif


					for (auto data : SatelliteImagefilename)
					{
						QStringList DEMpaths;
						QString filePathDEM = (self->projectdir + "PrecisionCheck" + "/" + "TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DEM") + ".txt");
						QFile fileTXTDEM(filePathDEM);

						if (!fileTXTDEM.open(QIODevice::ReadOnly | QIODevice::Text)) {
							qDebug() << "Failed to open file:" + filePathDEM;
							self->ModelMutex.unlock();
							emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
							return;
						}
						QTextStream inDEM(&fileTXTDEM);
						int lineNumberDEM = 0;
						while (!inDEM.atEnd()) {
							QString line = inDEM.readLine().trimmed();
							lineNumberDEM++;
							// 跳过第一行（数字行）
							if (lineNumberDEM == 1) {
								continue;
							}
							// 从第二行开始读取路径 
							if (!line.isEmpty()) {
								DEMpaths.append(line);
							}
						}
						fileTXTDEM.close();

						QStringList DOMpaths;
						QString filePathDOM = (self->projectdir + "PrecisionCheck" + "/" + "TargetFile" + QString::number(index1) + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("DOM") + ".txt");
						QFile fileTXTDOM(filePathDOM);

						if (!fileTXTDOM.open(QIODevice::ReadOnly | QIODevice::Text)) {
							qDebug() << "Failed to open file:" + filePathDOM;
							self->ModelMutex.unlock();
							emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
							return;
						}
						QTextStream inDOM(&fileTXTDOM);
						int lineNumberDOM = 0;
						while (!inDOM.atEnd()) {
							QString line = inDOM.readLine().trimmed();
							lineNumberDOM++;
							// 跳过第一行（数字行）
							if (lineNumberDOM == 1) {
								continue;
							}
							// 从第二行开始读取路径
							if (!line.isEmpty()) {
								DOMpaths.append(line);
							}
						}
						fileTXTDOM.close();

						if (DOMpaths.empty())
						{
							continue;
						}

						QFileInfo fileInfo(data);
						//for (auto DOMp : DOMpaths)
						//{
						QFileInfo fileInfoDOM(DOMpaths[0]);
						QFileInfo fileInfoDEM(DEMpaths[0]);

						QString outfilenametask = (self->projectdir + "PrecisionCheck" + "/" + "task" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("_") + QString::fromLocal8Bit("@task") + QString::number(indexDOM) + QString::fromLocal8Bit(".txt"));
						//QString outfilename = (self->projectdir + "PrecisionCheck\\" + fileInfo.completeBaseName() + QString::fromLocal8Bit("#") + fileInfoDOM.completeBaseName() + QString::fromLocal8Bit("#") + QString::fromLocal8Bit(".txt"));
						QString outfilename = (self->projectdir + "PrecisionCheck" + "/" + fileInfo.completeBaseName() + QString::fromLocal8Bit("#") + fileInfoDOM.completeBaseName() + QString::fromLocal8Bit("#") + fileInfoDEM.completeBaseName() + QString::fromLocal8Bit("#") + QString::fromLocal8Bit(".txt"));

						QFile filetask(outfilenametask);
						if (!filetask.open(QIODevice::WriteOnly | QIODevice::Text)) {
							qDebug() << "Failed to create task filetask:" + outfilenametask;
							self->ModelMutex.unlock();
							emit self->FunctionAbnormalExit(QString::fromLocal8Bit("绝对定位精度质检"));
							return;
						}
						QTextStream out(&filetask);

						out << "tag: XQIMAGE MATCH V1\n";
						out << QString::fromLocal8Bit("blkSize:%1 %2\n").arg(BlkSizeXnum).arg(BlkSizeYnum);
						out << QString::fromLocal8Bit("numOfBlks:%1 %2\n").arg(numOfBlksX).arg(numOfBlksY);
						out << QString::fromLocal8Bit("searchRANGE:%1 %2\n").arg(searchRANGEX).arg(searchRANGEY);
						out << QString::fromLocal8Bit("matchMethod:%1\n").arg(currentIndex);
						out << QString::fromLocal8Bit("BandIndex:%1 %2\n").arg(BandIndexwait).arg(BandIndexreference);
						out << QString::fromLocal8Bit("confidence:%1\n").arg(0.0, 0, 'f', 6);
						out << QString::fromLocal8Bit("generateImp:%1\n").arg("false");
						out << QString::fromLocal8Bit("wallisFilter:%1\n").arg("true");
						out << QString::fromLocal8Bit("exportGeoDsm:%1\n").arg("true");
						out << QString::fromLocal8Bit("ImpWarpMode:%1\n").arg(4);
						out << QString::fromLocal8Bit("tickPoint:%1\n").arg(QVariant(tickPoint).toString());
						out << QString::fromLocal8Bit("tickPointThrehold:%1\n").arg(tickPointThrehold);
						out << QString::fromLocal8Bit("oriIMAGE:%1\n").arg(data);
						out << QString::fromLocal8Bit("outRESULT:%1\n").arg(outfilename);
						if (DEMpaths.size() == 1)
						{
							//out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());
							out << QString::fromLocal8Bit("DEM:%1\n").arg(DEMpaths[0]);
						}
						else if (DEMpaths.size() <= 0)
							out << QString::fromLocal8Bit("DEM:%1\n").arg("");
						else if (DEMpaths.size() > 1)
						{
							out << QString::fromLocal8Bit("numOfRefDems:%1\n").arg(DEMpaths.size());

							for (auto DEMp : DEMpaths)
								out << QString::fromLocal8Bit("%1\n").arg(DEMp);
						}
						out << QString::fromLocal8Bit("numOfRefImages:%1\n").arg(DOMpaths.size());
						for (auto DOMp : DOMpaths)
						{
							out << QString::fromLocal8Bit("%1\n").arg(DOMp);
						}
						filetask.close();

						// 填充任务信息 
						TaskInfo task;
						task.taskFilePath = outfilenametask;
						task.operatorName = operatorName;
						task.outfilename = outfilename;
						self->m_taskQueue.enqueue(task);

						AbsPositPrecCheckPath.insert(outfilename, false);
						indexDOM++;
						index++;

						index1++;
					}

					// 检查输出文件是否已存在
					QVector<QString> existingFiles;
					for (const auto& outfilename : AbsPositPrecCheckPath.keys()) {
						if (QFile::exists(outfilename)) {
							existingFiles.append(outfilename);
						}
					}

					if (!existingFiles.isEmpty()) {
						// 弹出提示框询问用户
						QMessageBox msgBox;
						msgBox.setWindowTitle(QString::fromLocal8Bit("绝对定位精度质检"));
						msgBox.setText(QString::fromLocal8Bit(" 已经存在%1个质检成果，是否重新匹配?").arg(existingFiles.size()));
						msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
						msgBox.setDefaultButton(QMessageBox::No);

						// 创建定时器，15秒后自动关闭并选择"否"
						QTimer timer;
						timer.setSingleShot(true);
						QObject::connect(&timer, &QTimer::timeout, [&msgBox]() {
							if (msgBox.isVisible()) {
								msgBox.reject();  // 相当于选择"否"
							}
						});
						timer.start(15000);  // 15秒超时

						int ret = msgBox.exec();

						timer.stop();

						if (ret == QMessageBox::No) {
							// 用户选择不重新处理 

							// 删除重复的任务
							QQueue<TaskInfo> filteredQueue;
							for (const auto& task : self->m_taskQueue) {
								if (!existingFiles.contains(task.outfilename)) {
									filteredQueue.enqueue(task);
								}
							}
							self->m_taskQueue = filteredQueue;

							// 更新总任务数 
							self->m_totalTasks = self->m_taskQueue.size();

							// 如果所有文件都重复且用户选择不重新处理，则直接退出
							if (self->m_taskQueue.isEmpty()) {
								self->ModelMutex.unlock();
								emit self->ImageAbsPositPrecCheckFinished();
								return;
							}
						
							//// 清除AbsPositPrecCheckPath并重新填充不重复的文件 
							//AbsPositPrecCheckPath.clear();
							//for (const auto& task : self->m_taskQueue) {
							//	AbsPositPrecCheckPath.insert(task.outfilename, false);
							//}
						}
					}

					PublicFunctions::writeTimestampToXml("PrecisionCheck", self->m_imagePS->projectdir);
					self->writeMapToXml(AbsPositPrecCheckPath, QString::fromLocal8Bit("PrecisionCheckPath"));

					self->m_startTime = QDateTime::currentDateTime();
					self->logEdit->append(QString::fromLocal8Bit("****开始时间：") + self->m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

					// 开始处理任务 
					self->startNextTasks(QString::fromLocal8Bit("****绝对定位精度质检"));
				});
				self->IntersectCmd(DOMReferencefilename, SatelliteImagefilename, QString::fromLocal8Bit("DOM"), QString::fromLocal8Bit("PrecisionCheck"));
			});

			self->IntersectCmd(DEMReferencefilename, SatelliteImagefilename, QString::fromLocal8Bit("DEM"), QString::fromLocal8Bit("PrecisionCheck"));
			break;
		}
	};

	// 开始执行第一个任务 
	executeNextTask();
}

/**
 * @brief 影像裁切
 */
void SystemConfig::ImageCropping(QStringList imagePath, QStringList polyShpPath, QString OutputDir)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有影像裁切"))) {
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("影像裁切"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****影像裁切****"));
	IntersectCmdPaths.clear();
	ImageCroppXmlFiles.clear();
	QString attriID = ui.lineEdit_51->text();
	ExcuteExeCutVector(polyShpPath[0], projectdir + QString::fromLocal8Bit("Mosaic") + "/", attriID);

	//for (auto filepath : imagePath) {
	disconnect(this, &SystemConfig::ExcuteExeCutVectorFinished, nullptr, nullptr);
	connect(this, &SystemConfig::ExcuteExeCutVectorFinished, this,
		[=]() {
		disconnect(this, &SystemConfig::ExcuteExeCutVectorFinished, nullptr, nullptr);

		QString directoryName = projectdir + QString::fromLocal8Bit("Mosaic") + "/";
		// 查找MapSplitList.txt 文件 
		QString fileName = "MapSplitList.txt";
		QDir dir(directoryName);
		QStringList files = dir.entryList(QStringList() << fileName, QDir::Files);

		if (files.isEmpty()) {
			logEdit->append(QString::fromLocal8Bit("未找到MapSplitList.txt 文件"));
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("未找到MapSplitList.txt 文件!"));
		}

		QString mapSplitListPath = dir.filePath(files.first());
		QFile file(mapSplitListPath);
		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
			logEdit->append(QString::fromLocal8Bit("无法打开MapSplitList.txt 文件"));
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("无法打开MapSplitList.txt 文件!"));
		}

		QTextStream in(&file);
		QStringList filePaths;
		bool isFirstLine = true;
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			if (isFirstLine) {
				isFirstLine = false;
				continue;
			}
			if (!line.isEmpty()) {
				filePaths.append(line);
			}
		}
		file.close();


		ImageCropcurrentIndex = 0; // 当前处理的图像索引 
		ImageCroplocalInputPaths = imagePath; // 保存输入路径 

		if (ImageCropcurrentIndex == 0) {
			ExcuteExeMuchXQMosaicXmlFiles.clear();
			QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);

			// 建立新连接 - 每次IntersectCmd完成后处理 
			QObject::connect(this, &SystemConfig::intersectCmdFinished, this, [=, id = ImageCropId]() {
				if (id != m_connectionId) return;

				QStringList InputImageFileP;
				QString mosaicListFile = IntersectCmdPaths[0];
				QString outputFilePath = projectdir + QString::fromLocal8Bit("Mosaic/MosaicList_") + QString::number(ImageCropcurrentIndex) + QString::fromLocal8Bit(".txt");  // 输出文件路径 

				// 同时打开输入文件和输出文件
				QFile targetFile(mosaicListFile);;

				if (!targetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像裁切 打开求交成果文件失败!"));

					//qWarning() << u8"无法打开目标文件:" << targetFile.errorString();
					return;
				}

				QTextStream in(&targetFile);

				int lineCount = 0;
				int totalLines = 0;

				while (!in.atEnd()) {
					QString line = in.readLine().trimmed();
					lineCount++;

					// 第一行是文件总数（跳过不写入）
					if (lineCount == 1) {
						totalLines = line.toInt();
						continue;
					}
					// 写入SmartMosaicList.txt （每行一个路径）
					InputImageFileP.append(line);
				}

				// 关闭文件
				targetFile.close();


				int ResultImageFileTypenum = ui.comboBox->currentIndex();
				QString ResultImageFileType;
				switch (ResultImageFileTypenum) {
				case 0: ResultImageFileType = QString::fromLocal8Bit(".tif"); break;
				case 1: ResultImageFileType = QString::fromLocal8Bit(".img"); break;
				default:
					//logEdit->append(QString::fromLocal8Bit("****请完成影像裁切输出文件格式配置!!!"));
					ModelMutex.unlock();
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成影像裁切输出文件格式配置"));

					emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成影像裁切输出文件格式配置"));
					return;
				}
				bool bEnvelopeP = ui.checkBox_58->isChecked();
				bool bCreatePydP = ui.checkBox_60->isChecked();
				bool bResultIntersectP = ui.checkBox_61->isChecked();
				bool bExpanseP = ui.checkBox_59->isChecked();
				bool bLogicalMosaicP = ui.checkBox_63->isChecked();
				bool bOnlyProcessSameNameP = ui.checkBox_86->isChecked();
				int nExpansePixelsP = ui.lineEdit_50->text().toInt();

				QDomDocument doc;

				// 添加XML声明 
				QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
				doc.appendChild(instruction);

				// 创建根节点 
				QDomElement root = doc.createElement("ImageCrop");
				doc.appendChild(root);

				for (auto filepath : InputImageFileP) {
					// 添加各子节点
					QDomElement inputImageFile = doc.createElement("InputImageFile");
					inputImageFile.appendChild(doc.createTextNode(filepath));
					root.appendChild(inputImageFile);
				}

				QDomElement outputDirP = doc.createElement("OutputDir");
				outputDirP.appendChild(doc.createTextNode(projectdir + QString::fromLocal8Bit("Mosaic") + "/"));
				root.appendChild(outputDirP);

				QDomElement nOutPutFormat = doc.createElement("nOutPutFormat");
				nOutPutFormat.appendChild(doc.createTextNode(QString::number(ResultImageFileTypenum)));  // 0表示tif格式
				root.appendChild(nOutPutFormat);

				QDomElement bSingleCut = doc.createElement("bSingleCut");
				bSingleCut.appendChild(doc.createTextNode("1"));
				root.appendChild(bSingleCut);

				QDomElement bEnvelope = doc.createElement("bEnvelope");
				bEnvelope.appendChild(doc.createTextNode(QString::number(bEnvelopeP)));  // 1表示矩形裁切 
				root.appendChild(bEnvelope);

				QDomElement bCreatePyd = doc.createElement("bCreatePyd");
				bCreatePyd.appendChild(doc.createTextNode(QString::number(bCreatePydP)));  // 创建金字塔
				root.appendChild(bCreatePyd);

				QDomElement bResultIntersect = doc.createElement("bResultIntersect");
				bResultIntersect.appendChild(doc.createTextNode(QString::number(bResultIntersectP)));
				root.appendChild(bResultIntersect);

				QDomElement bExpanse = doc.createElement("bExpanse");
				bExpanse.appendChild(doc.createTextNode(QString::number(bExpanseP)));  // 启用外扩 
				root.appendChild(bExpanse);

				QDomElement nExpansePixels = doc.createElement("nExpansePixels");
				nExpansePixels.appendChild(doc.createTextNode(QString::number(nExpansePixelsP)));  // 外扩100像素 
				root.appendChild(nExpansePixels);

				QDomElement bLogicalMosaic = doc.createElement("bLogicalMosaic");
				bLogicalMosaic.appendChild(doc.createTextNode(QString::number(bLogicalMosaicP)));
				root.appendChild(bLogicalMosaic);

				QDomElement bOnlyProcessSameName = doc.createElement("bOnlyProcessSameName");
				bOnlyProcessSameName.appendChild(doc.createTextNode(QString::number(bOnlyProcessSameNameP)));
				root.appendChild(bOnlyProcessSameName);

				QDomElement cropType = doc.createElement("cropType");
				cropType.appendChild(doc.createTextNode("1"));
				root.appendChild(cropType);

				QString mosaicToopFilepath = filePaths[ImageCropcurrentIndex];
				QDomElement polyShp = doc.createElement("polyShp");
				QDomElement polyShpFilePath = doc.createElement("polyShpFilePath");
				polyShpFilePath.appendChild(doc.createTextNode(mosaicToopFilepath));  // 使用传入的裁切范围文件路径 
				polyShp.appendChild(polyShpFilePath);
				root.appendChild(polyShp);

				// 保存XML文件
				QString xmlPath = projectdir + "Mosaic/PSImageCropCmd_" + QString::number(ImageCropcurrentIndex) + ".xml";
				QFile file(xmlPath);
				if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
					//logEdit->append(QString::fromLocal8Bit("****无法创建裁切任务单失败****"));
					ModelMutex.unlock();
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建裁切任务单失败"));
					emit FunctionAbnormalExit(QString::fromLocal8Bit("创建裁切任务单失败"));
					return;
				}

				QTextStream out(&file);
				out.setCodec("UTF-8");
				doc.save(out, 4);
				file.close();

				if (!xmlPath.isEmpty()) {
					ImageCroppXmlFiles.append(xmlPath);
				}

				// 处理下一个图像 
				ImageCropcurrentIndex++;
				if (ImageCropcurrentIndex < filePaths.size()) {

					QStringList remainingPaths;
					remainingPaths.append(filePaths[ImageCropcurrentIndex]);

					if (!remainingPaths.isEmpty()) {
						IntersectCmd(ImageCroplocalInputPaths, remainingPaths, QString::fromLocal8Bit("Mosaic"), QString::fromLocal8Bit("Mosaic"));
					}
				}
				else {
					QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
					// 所有处理完成，重置索引 
					ImageCropcurrentIndex = 0;
					emit allIntersectCmdsFinished(); // 发射所有处理完成的信号 
				}

			});

			// 开始处理第一个图像
			if (!ImageCroplocalInputPaths.empty()) {

				QStringList remainingPaths;
				remainingPaths.append(filePaths[ImageCropcurrentIndex]);

				if (!remainingPaths.isEmpty()) {
					IntersectCmd(ImageCroplocalInputPaths, remainingPaths, QString::fromLocal8Bit("Mosaic"), QString::fromLocal8Bit("Mosaic"));
				}
			}
		}

		QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);

		QObject::connect(this, &SystemConfig::allIntersectCmdsFinished, this, [this, id = ImageCropId]() {
			QObject::disconnect(this, &SystemConfig::allIntersectCmdsFinished, nullptr, nullptr);
			if (id != m_connectionId) return;
			// 初始化任务队列和相关参数
			m_taskQueue.clear();
			m_runningProcesses.store(0);
			m_completedTasks.store(0);
			m_startTime = QDateTime::currentDateTime();
			m_totalTasks = ImageCroppXmlFiles.size();

			for (auto filename : ImageCroppXmlFiles)
			{
				// 将任务添加到队列
				TaskInfo task;
				task.taskFilePath = filename;
#ifdef Q_OS_LINUX 
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/XQImageCropCmd.x"));
#else
				task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/XQImageCropCmd.exe"));
#endif
				m_taskQueue.enqueue(task);
			}
			if (m_taskQueue.empty())
			{
				ModelMutex.unlock();
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像裁切任务队列为空!"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("影像裁切"));
				return;
			}
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像裁切开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			startNextTasks(QString::fromLocal8Bit("****影像裁切"));

		});

		
	}, Qt::UniqueConnection);
}

/**
* @brief SAR影像滤波
*/
void SystemConfig::SARImageFiltering(QStringList imagePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有SAR影像滤波"))) {
		return;
	}

	// 使用封装后的函数显示界面并等待用户确认 
	int result = showFunctionAndWait(QString::fromLocal8Bit("SAR影像滤波"));

	// 用户取消操作
	if (result != QDialog::Accepted) {
		ModelMutex.unlock();
		return;
	}

	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****SAR影像滤波****"));

	// 获取界面参数
	int SarFilterImageType = ui.ImageType_Box->currentIndex();
	int fileterType = ui.FilteringMethod_Box->currentIndex();

	int dampFactor = ui.LineEdit_6->text().toInt();
	int nFilterWid = ui.LineEdit_5->text().toInt();
	int nStasticWid = ui.LineEdit_7->text().toInt();
	int nLooks = ui.LineEdit_8->text().toInt();

	// 获取输出文件格式
	int ResultImageFileTypenum = ui.ComboBox_5->currentIndex();
	QString outfiletype;
	switch (ResultImageFileTypenum) {
	case 0: outfiletype = QString::fromLocal8Bit(".tif"); break;
	case 1: outfiletype = QString::fromLocal8Bit(".img"); break;
	default:
		//logEdit->append(QString::fromLocal8Bit("****请完成Sar影像滤波输出文件格式配置!!!"));
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("请完成SAR影像滤波输出文件格式配置!!!!"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("请完成SAR影像滤波输出文件格式配置!!!"));
		return;
	}

	// 检查是否有任务需要处理 
	if (imagePath.empty()) {
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("SAR影像滤波输入待处理影像为空"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("SAR影像滤波输入待处理影像为空"));
		return;
	}

	// 初始化任务队列
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;

	// 检查输出文件是否存在 
	int duplicateCount = 0;
	QVector<bool> isDuplicate;
	for (auto filename : imagePath) {
			QFileInfo fileinfo(filename);
			QString outfilename = projectdir + "ImageFilter" + "/" + fileinfo.completeBaseName() + outfiletype;
			if (QFile::exists(outfilename)) {
				duplicateCount++;
				isDuplicate.append(true);
			}
			else {
				isDuplicate.append(false);
			}
	}

	// 处理重复文件情况 
	bool processAll = true;
	if (duplicateCount > 0) {
		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("文件已存在"),
			QString::fromLocal8Bit("已经存在%1个转换成果，是否重新转换?").arg(duplicateCount),
			QMessageBox::Yes | QMessageBox::No);

		processAll = (reply == QMessageBox::Yes);

		if (!processAll && duplicateCount == imagePath.size()) {
			// 所有文件都重复且用户选择不重新处理
			ModelMutex.unlock();
			emit SARImageFilterFinished();
			return;
		}
	}

	// 为每个图像创建任务 
	int duplicateIndex = 0;
	for (auto filename : imagePath) {
		if (!processAll && isDuplicate[duplicateIndex++]) {
			continue;
		}

		// 创建XML文档
		QDomDocument doc;
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
		doc.appendChild(instruction);

		QDomElement root = doc.createElement("XQImageFilterSarCmd");
		root.setAttribute("version", "1.0");
		doc.appendChild(root);

		// 添加输入输出文件路径 
		QDomElement inputImageFileName = doc.createElement("InputImageFilePath");
		inputImageFileName.setAttribute("type", "string");
		inputImageFileName.appendChild(doc.createTextNode(filename));
		root.appendChild(inputImageFileName);

		QFileInfo fileinfo(filename);
		QDomElement outputFileName = doc.createElement("OutputImageFilePath");
		outputFileName.setAttribute("type", "string");
		QString outfilename = projectdir + "ImageFilter" + "/" + fileinfo.completeBaseName() + outfiletype;
		outputFileName.appendChild(doc.createTextNode(outfilename));
		root.appendChild(outputFileName);

		QDomElement PSfileterType = doc.createElement("fileterType");
		PSfileterType.appendChild(doc.createTextNode(QString::number(fileterType)));
		root.appendChild(PSfileterType);

		QDomElement Param = doc.createElement("Param");

		// 添加处理参数
		auto addChildNode = [&doc, &Param](const QString& tagName, const QString& value) {
			QDomElement element = doc.createElement(tagName);
			element.appendChild(doc.createTextNode(value));
			Param.appendChild(element);
		};

		addChildNode("SarFilterImageType", QString::number(SarFilterImageType));
		addChildNode("dampFactor", QString::number(dampFactor));
		addChildNode("nFilterWid", QString::number(nFilterWid));
		addChildNode("nStasticWid", QString::number(nStasticWid));
		addChildNode("nLooks", QString::number(nLooks));

		root.appendChild(Param);

		// 保存XML文件
		QString xmlPath = projectdir + QString::fromLocal8Bit("ImageFilter") + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit(".xml");
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open file for writing";
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("SAR影像滤波创建任务单失败"));

			emit FunctionAbnormalExit(QString::fromLocal8Bit("SAR影像滤波创建任务单失败"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		doc.save(out, 4);
		file.close();

		// 添加到任务队列 
		TaskInfo task;
		task.taskFilePath = xmlPath;
		//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageOutByteCmd.exe");
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSImageFilterSarCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSImageFilterSarCmd.exe"));
#endif
		task.outfilename = outfilename;
		m_taskQueue.enqueue(task);
		m_totalTasks++;
	}
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("SAR影像滤波开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	// 记录开始时间 
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****SAR影像滤波"), true);
}

/**
* @brief 创建缩略图
*/
void SystemConfig::ThumbnailGeneration(QStringList imagePath)
{
	if (imagePath.empty())
	{
		logEdit->append(QString::fromLocal8Bit("****创建缩略图失败,传入数据为空!"));
		return;
	}
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有创建缩略图"))) {
		return;
	}
	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****创建缩略图****"));

	// 获取界面参数
	double Scale = ui.lineEdit_52->text().toInt();
	bool OnlyM = ui.checkBox_65->isChecked();

	QStringList tmpimagePath;
	for (auto filename : imagePath)
	{
		QFileInfo fileinfo(filename);
		QString Type = getTiffImageType(fileinfo.absoluteFilePath());
		if (OnlyM && Type == "MSS")
			tmpimagePath.push_back(filename);
		else if(!OnlyM)
			tmpimagePath.push_back(filename);
	}
	// 检查是否有任务需要处理 
	if (tmpimagePath.empty()) {
		ModelMutex.unlock();
		emit FunctionAbnormalExit(QString::fromLocal8Bit("创建缩略图"));
		return;
	}

	// 初始化任务队列
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;

	// 检查输出文件是否存在 
	int duplicateCount = 0;
	QVector<bool> isDuplicate;
	for (auto filename : tmpimagePath) {
		QFileInfo fileinfo(filename);
		QString outfilename = projectdir + "Cld" + "/" + "OverView" + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit(".jpg");
		if (QFile::exists(outfilename)) {
			if (!ThumbnailGenerationPaths.contains(outfilename)) {
				ThumbnailGenerationPaths.append(outfilename);
			}
			duplicateCount++;
			isDuplicate.append(true);
		}
		else {
			isDuplicate.append(false);
		}
	}

	// 处理重复文件情况 
	bool processAll = true;
	if (duplicateCount > 0) {
		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("文件已存在"),
			QString::fromLocal8Bit("已经存在%1个缩略图成果，是否重新生成?").arg(duplicateCount),
			QMessageBox::Yes | QMessageBox::No);

		processAll = (reply == QMessageBox::Yes);

		if (!processAll && duplicateCount == tmpimagePath.size()) {
			// 所有文件都重复且用户选择不重新处理
			ModelMutex.unlock();
			emit ThumbnailGenerationFinished();
			return;
		}
	}

	// 为每个图像创建任务 
	int PSindex = 0;
	int duplicateIndex = 0;
	for (auto filename : tmpimagePath) {
		// 检查是否需要跳过重复文件
		if (!processAll && isDuplicate[duplicateIndex++]) {
			continue;
		}
		QFileInfo fileinfo(filename);

		// 创建XML文档
		QDomDocument doc;
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
		doc.appendChild(instruction);

		QDomElement root = doc.createElement("XQImageResampleCmd");
		root.setAttribute("version", "1.0");
		doc.appendChild(root);

		// 添加输入输出文件路径 
		QDomElement inputImageFileName = doc.createElement("inputImageFileName");
		inputImageFileName.setAttribute("type", "string");
		inputImageFileName.appendChild(doc.createTextNode(filename));
		root.appendChild(inputImageFileName);

		QDomElement outputFileName = doc.createElement("outputFileName");
		outputFileName.setAttribute("type", "string");
		QString outfilename = projectdir + "Cld" + "/" + "OverView" + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit(".jpg");
		outputFileName.appendChild(doc.createTextNode(outfilename));
		root.appendChild(outputFileName);

		// 添加处理参数
		auto addChildNode = [&doc, &root](const QString& tagName, const QString& value) {
			QDomElement element = doc.createElement(tagName);
			element.appendChild(doc.createTextNode(value));
			root.appendChild(element);
		};

		addChildNode("ResamplingKernel", "Nearest");
		//addChildNode("Scale", QString::number(Scale / Scale / Scale));
		if (!Scale) {
			addChildNode("Scale", "0.000000");
		}
		else {
			addChildNode("Scale", QString::number(Scale / Scale / Scale, 'f', 6));
			// 更高效的计算方式：1/(Scale²)
		}
		addChildNode("CreatePyd", "false");
		addChildNode("Tasks", "4");

		// 保存XML文件
		QString xmlPath = projectdir + QString::fromLocal8Bit("Cld") + "/" + "OverView" + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_createOverView") + QString::fromLocal8Bit(".xml");
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			qDebug() << "Failed to open file for writing";
			ModelMutex.unlock();
			emit FunctionAbnormalExit(QString::fromLocal8Bit("创建缩略图"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		doc.save(out, 4);
		file.close();

		// 添加到任务队列 
		TaskInfo task;
		task.taskFilePath = xmlPath;
		//task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSImageResampleCmd.exe");
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSImageResampleCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSImageResampleCmd.exe"));
#endif
		task.outfilename = outfilename;
		m_taskQueue.enqueue(task);
		m_totalTasks++;

		PSindex++;

	}

	// 记录开始时间 
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****创建缩略图"), true);
}

/**
* @brief 影像云检
*/
void SystemConfig::CloudDetection()
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有影像云检"))) {
		return;
	}
	setProjectdir();
	logEdit->append(QString::fromLocal8Bit("****影像云检****"));

	// 获取界面参数
	int Scale = ui.lineEdit_52->text().toInt();
	bool OnlyM = ui.checkBox_65->isChecked();

	// 检查是否有任务需要处理 
	if (ThumbnailGenerationPaths.empty()) {
		ModelMutex.unlock();
		emit FunctionAbnormalExit(QString::fromLocal8Bit("影像云检影像没有对应缩略图,"));
		return;
	}

	// 初始化任务队列
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;

	// 检查输出文件是否存在 
	int duplicateCount = 0;
	QVector<bool> isDuplicate;
	for (auto filename : ThumbnailGenerationPaths) {
		QFileInfo fileinfo(filename);
		QString outfilename = projectdir + "Cld" + "/" + "CloudDect" + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_cld.tif");
		if (QFile::exists(outfilename)) {
			if (!CloudDetectionOutPaths.contains(outfilename)) {
				CloudDetectionOutPaths.append(outfilename);
			}
			duplicateCount++;
			isDuplicate.append(true);
		}
		else {
			isDuplicate.append(false);
		}
	}

	// 处理重复文件情况 
	bool processAll = true;
	if (duplicateCount > 0) {
		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("文件已存在"),
			QString::fromLocal8Bit("已经存在%1个云检成果，是否重新生成?").arg(duplicateCount),
			QMessageBox::Yes | QMessageBox::No);

		processAll = (reply == QMessageBox::Yes);

		if (!processAll && duplicateCount == ThumbnailGenerationPaths.size()) {
			// 所有文件都重复且用户选择不重新处理
			ModelMutex.unlock();
			emit CloudDetectionFinished();
			return;
		}
	}

	// 为每个图像创建任务 
	int duplicateIndex = 0;
	for (auto filename : ThumbnailGenerationPaths) {
		// 检查是否需要跳过重复文件
		if (!processAll && isDuplicate[duplicateIndex++]) {
			continue;
		}
		QFileInfo fileinfo(filename);

		// 添加到任务队列 
		TaskInfo task;
		task.taskFilePath = filename;
		task.taskFilePath2 = projectdir + "Cld" + "/" + "CloudDect" + "/";
		task.operatorName = m_imagePS->appDirPath + QString::fromLocal8Bit("/Software/CLD/CLDKNL.exe");
		task.outfilename = projectdir + "Cld" + "/" + "CloudDect" + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit("_cld.tif");;
		m_taskQueue.enqueue(task);
		m_totalTasks++;
	}

	// 记录开始时间 
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****影像云检"), true);
}

/**
* @brief 影像赋投影
*/
void SystemConfig::Imageprojection(QStringList imagePath)
{
	if (!tryLockFunction(ModelMutex, QString::fromLocal8Bit("已有影像赋投影"))) {
		return;
	}
	setProjectdir();

	// 检查是否有任务需要处理 
	if (imagePath.empty()) {
		ModelMutex.unlock();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像赋投影输入待处理数据为空"));
		emit FunctionAbnormalExit(QString::fromLocal8Bit("影像赋投影输入待处理数据为空"));
		return;
	}

	// 初始化任务队列
	m_taskQueue.clear();
	m_runningProcesses.store(0);
	m_completedTasks.store(0);
	m_startTime = QDateTime::currentDateTime();
	m_totalTasks = 0;

	// 为每个图像创建任务 
	int duplicateIndex = 0;
	for (auto filename : imagePath) {

		// 创建XML文档
		QDomDocument doc;
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
		doc.appendChild(instruction);

		QDomElement root = doc.createElement("XQSetProjectionCmd");
		root.setAttribute("version", "1.0");
		doc.appendChild(root);

		// 添加输入输出文件路径 
		QDomElement inputImageFileName = doc.createElement("InputImageFilePath");
		inputImageFileName.setAttribute("type", "string");
		inputImageFileName.appendChild(doc.createTextNode(filename));
		root.appendChild(inputImageFileName);

		QFileInfo fileinfo(filename);
		QDomElement outputFileName = doc.createElement("wktHCS");
		outputFileName.setAttribute("type", "string");
		QString outfilename = projectdir + "ImageFilter" + "/" + fileinfo.completeBaseName();
		outputFileName.appendChild(doc.createTextNode(outfilename));
		root.appendChild(outputFileName);

		// 保存XML文件
		QString xmlPath = projectdir + QString::fromLocal8Bit("ImageFilter") + "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit(".xml");
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			//qDebug() << "Failed to open file for writing";
			ModelMutex.unlock();
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像赋投影任务单创建失败"));
			emit FunctionAbnormalExit(QString::fromLocal8Bit("影像赋投影任务单创建失败"));
			return;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		doc.save(out, 4);
		file.close();

		// 添加到任务队列 
		TaskInfo task;
		task.taskFilePath = xmlPath;
#ifdef Q_OS_LINUX 
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/XQSetProjectionCmd.x"));
#else
		task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/XQSetProjectionCmd.exe"));
#endif
		task.outfilename = outfilename;
		m_taskQueue.enqueue(task);
		m_totalTasks++;
	}
	PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像赋投影开始执行: ") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	// 记录开始时间 
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + m_startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("****影像赋投影"), true);
}

/**
 * @brief 影像生产
 */
void SystemConfig::OrthoRectificationFusion(QStringList DEMReferencefilename, QStringList DOMReferencefilename, QStringList SatelliteImagefilename)
{
	setProjectdir();

	// 保存所有连接的容器
	//static QVector<QMetaObject::Connection> connections;

	// 先断开所有之前的连接
	for (auto &conn : connections) {
		QObject::disconnect(conn);
	}
	connections.clear();

	logEdit->append(QString::fromLocal8Bit("********影像生产********"));

	QDateTime startTime = QDateTime::currentDateTime();
	logEdit->append(QString::fromLocal8Bit("****开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

	// 流程非正常退出连接
	connections.append(connect(this, &SystemConfig::FunctionAbnormalExit, this,
		[=](QString title) {
		QDateTime endTime = QDateTime::currentDateTime();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("****") + title + QString::fromLocal8Bit("非正常退出,影像生产结束"));
		logEdit->append(QString::fromLocal8Bit("****") + title + QString::fromLocal8Bit("非正常退出,影像生产结束"));
		logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
		logEdit->append(QString::fromLocal8Bit("********影像生产总耗时：") + calculateTimeDifference(startTime, endTime));

		// 处理完成后断开所有连接
		for (auto &conn : connections) {
			QObject::disconnect(conn);
		}
		connections.clear();
	}));

	if (!m_flagMosaic) {
		// 影像镶嵌完成信号连接
		connections.append(connect(this, &SystemConfig::ImageMosaicFinished, this,
			[=]() {
			QDateTime endTime = QDateTime::currentDateTime();
			logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			logEdit->append(QString::fromLocal8Bit("********影像生产总耗时：") + calculateTimeDifference(startTime, endTime));

			// 处理完成后断开所有连接
			for (auto &conn : connections) {
				QObject::disconnect(conn);
			}
			connections.clear();
			m_imagePS->setWorkProcessflag(false);
			setProjectdir();
			// 添加弹窗询问是否删除中间数据
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(
				m_imagePS,
				QString::fromLocal8Bit("删除确认"),
				QString::fromLocal8Bit("是否要删除中间处理数据？"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No
			);

			if (reply == QMessageBox::Yes) {
				deleteFilesInAllSubfolders(projectdir);
				logEdit->append(QString::fromLocal8Bit("已删除中间处理数据"));
			}
			else {
				logEdit->append(QString::fromLocal8Bit("保留中间处理数据"));
			}
		}));
	}
	else {
		// 影像镶嵌完成信号连接
		connections.append(connect(this, &SystemConfig::SmartMosaicFinished, this,
			[=]() {
			QDateTime endTime = QDateTime::currentDateTime();
			logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
			logEdit->append(QString::fromLocal8Bit("********影像生产总耗时：") + calculateTimeDifference(startTime, endTime));

			// 处理完成后断开所有连接
			for (auto &conn : connections) {
				QObject::disconnect(conn);
			}
			connections.clear();
			m_imagePS->setWorkProcessflag(false);
			setProjectdir();
			// 添加弹窗询问是否删除中间数据
			QMessageBox::StandardButton reply;
			reply = QMessageBox::question(
				m_imagePS,
				QString::fromLocal8Bit("删除确认"),
				QString::fromLocal8Bit("是否要删除中间处理数据？"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No
			);

			if (reply == QMessageBox::Yes) {
				deleteFilesInAllSubfolders(projectdir);
				logEdit->append(QString::fromLocal8Bit("已删除中间处理数据"));
			}
			else {
				logEdit->append(QString::fromLocal8Bit("保留中间处理数据"));
			}
		}));
	}

	// 影像匀色完成信号连接
	connections.append(connect(this, &SystemConfig::ImageColorCorrectionFinished, this,
		[=]() {
		if (ui.widget_10->isEnabled())
		{
			if (!m_flagMosaic)
			{
				QStringList filenamelist = QStringList();

				QDir dir(projectdir + QString::fromLocal8Bit("Dodging/"));

				if (!dir.exists()) {
					//qWarning() << "文件夹不存在:";
					PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色成果文件夹不存在"));
					emit FunctionAbnormalExit(QString::fromLocal8Bit("****影像匀色成果文件夹不存在"));
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
					QString Type = getTiffImageType(fileInfo.absoluteFilePath());
					filenamelist.append(fileInfo.absoluteFilePath());
				}

				ImageMosaic(filenamelist);
			}
			else
			{
				//if (ui.lineEdit_37->text().trimmed().isEmpty()) {
				//	emit FunctionAbnormalExit(QString::fromLocal8Bit("****智能镶嵌设置中裁切矢量文件(自动流程)为空!!!,请重新设置"));
				//	return;
				//}

				//智能镶嵌排序测试注释
				//QStringList filenamelist = QStringList();

				//QDir dir(projectdir + QString::fromLocal8Bit("Dodging/"));

				//if (!dir.exists()) {
				//	//qWarning() << "文件夹不存在:";
				//	PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像匀色成果文件夹不存在"));

				//	emit FunctionAbnormalExit(QString::fromLocal8Bit("****影像匀色"));
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
				//	QString Type = getTiffImageType(fileInfo.absoluteFilePath());
				//	if (Type == "MSS")
				//		filenamelist.append(fileInfo.absoluteFilePath());
				//}
				QStringList filenamelist = QStringList();
				filenamelist = m_imagePS->getSmartMosaicFilePath();

				QStringList MosaicCropFilePath;
				if (!ui.lineEdit_37->text().trimmed().isEmpty()) {
					MosaicCropFilePath.append(ui.lineEdit_37->text());
				}

				SmartMosaic(filenamelist, MosaicCropFilePath);
			}
		}
		else {
			if (!m_flagMosaic) {
				emit ImageMosaicFinished();
			}
			else {
				emit SmartMosaicFinished();
			}
		}
	}));

	// 真彩色转换完成信号连接
	connections.append(connect(this, &SystemConfig::TrueColorConversionFinished, this,
		[=]() {
		if (ui.widget_8->isEnabled())
		{
			QStringList filenamelist = QStringList();
			QString outputDir;
			if (!ConvertTo8Bitflag && ui.output8bImage_checkBox->isChecked()) {
				// 修改输出目录为Dodging目录 
				outputDir = projectdir + "Fusion/";
			}
			else {
				outputDir = projectdir + "OutByte/";
			}
			QDir dir(outputDir);

			if (!dir.exists()) {
				qWarning() << "文件夹不存在:";
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, outputDir + QString::fromLocal8Bit("成果文件夹不存在"));

				emit FunctionAbnormalExit(QString::fromLocal8Bit("****")+ outputDir + QString::fromLocal8Bit("成果文件夹不存在"));
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
			ImageColorCorrection(filenamelist);
		}
		else
			emit ImageColorCorrectionFinished();
	}));

	// 影像融合完成信号连接
	connections.append(connect(this, &SystemConfig::imageInterActionFinished, this,
		[=]() {
		if (ui.widget_9->isEnabled() && !ui.output8bImage_checkBox->isChecked())
		{
			QStringList filenamelist = QStringList();
			TrueColorConversion(filenamelist);
		}
		else
			emit TrueColorConversionFinished();
	}));

	// 正射纠正完成信号连接
	connections.append(connect(this, &SystemConfig::orthorectificationFinished, this,
		[=]() {
		if (ui.widget_7->isEnabled())
		{
			QString outputPath = projectdir + QString::fromLocal8Bit("Ortho/");
			QDir directory(outputPath);

			QStringList filters;
			filters << "*.tif" << "*.TIF"
				<< "*.img" << "*.IMG"
				<< "*.envi" << "*.ENVI"
				<< "*.pix" << "*.PIX";

			QStringList files = directory.entryList(filters, QDir::Files | QDir::NoDotAndDotDot);
			QStringList fullPaths;
			for (const QString &file : files) {
				fullPaths.append(directory.absoluteFilePath(file));
			}
			QStringList filteredFiles = filterSatelliteImagesByColumn(fullPaths, 10);
			if (!filteredFiles.isEmpty()) {
				dataModelConfig->BuildSatModelMakerCmdXML(filteredFiles, "Fusion");
				imageInterAction();
			}
			else {
				//logEdit->append(QString::fromLocal8Bit("未找到正射纠正输出文件"));
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("影像融合未找到正射纠正输出文件"));

				emit FunctionAbnormalExit(QString::fromLocal8Bit("****影像融合未找到正射纠正输出文件"));
			}
		}
		else
			emit imageInterActionFinished();
	}));

	if (ui.widget_6->isEnabled())
	{
		// 先断开之前的连接（避免重复调用）
		disconnect(this, &SystemConfig::controlNetAdjustFinished,
			m_imagePS, qOverload<>(&IMAGEPS::ConResidualFiles));
		// 重新连接信号
		connections.append(connect(this, &SystemConfig::controlNetAdjustFinished,
			m_imagePS, qOverload<>(&IMAGEPS::ConResidualFiles)));
	}

	// 控制网平差完成信号连接
	connections.append(connect(this, &SystemConfig::controlNetAdjustFinished, this,
		[=]() {
		if (ui.widget_4->isEnabled())
		{
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
			QStringList finalAlignmentPaths = SatelliteImagefilename;
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
					if (duplicateFiles.size() == SatelliteImagefilename.size()) {
						logEdit->append(QString::fromLocal8Bit(" 用户取消重新纠正已存在的成果"));
						emit orthorectificationFinished();
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
				//logEdit->append(QString::fromLocal8Bit(" 没有需要处理的影像文件"));
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正没有需要处理的影像文件"));

				emit FunctionAbnormalExit(QString::fromLocal8Bit("****正射纠正没有需要处理的影像文件"));
				return;
			}

			QString sourceDir = projectdir + QString::fromLocal8Bit("SatBA") + "/" + "NewRPCs" + "/";
			bool allSuccess = true;

			if (ui.checkBox_7->isChecked()) {
				for (auto data : finalAlignmentPaths) {
					QFileInfo fileInfo(data);
					QString destDir = fileInfo.absolutePath() + "/";

					QDir sourceDirectory(sourceDir);
					if (!sourceDirectory.exists()) {
						PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正 没有找到平差成果文件"));

						//qWarning() << "Source directory does not exist:" << sourceDir;
						//emit FunctionAbnormalExit(QString::fromLocal8Bit("****正射纠正"));
						//return;
					}

					QDir destDirectory(destDir);
					if (!destDirectory.exists()) {
						if (!destDirectory.mkpath(".")) {
							//qWarning() << "Failed to create destination directory:" << destDir;
							PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正 没有找到原始数据文件夹"));

							//emit FunctionAbnormalExit(QString::fromLocal8Bit("****正射纠正"));
							//return;
						}
					}

					QStringList filters;
					filters << "*_rpc.txt";
					QStringList files = sourceDirectory.entryList(filters, QDir::Files | QDir::Readable);
					if (files.isEmpty()) {
						PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正 没有找到平差成果文件 *.rpc"));

						//qWarning() << "No *_rpc.txt files found in source directory";
						//emit FunctionAbnormalExit(QString::fromLocal8Bit("****正射纠正"));
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
								//qWarning() << "Failed to remove existing file:" << destPath;
								allSuccess = false;
								continue;
							}
						}
						if (!QFile::copy(sourcePath, destPath)) {
							//qWarning() << "Failed to copy file from" << sourcePath << "to" << destPath;
							allSuccess = false;
						}
					}
				}
			}

			if (DEMReferencefilename.empty())
			{
				//logEdit->append(QString::fromLocal8Bit("工程中没有参考DEM数据，请确认"));
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("正射纠正 工程中没有参考DEM数据"));

				emit FunctionAbnormalExit(QString::fromLocal8Bit("****正射纠正 工程中没有参考DEM数据，请确认"));
				return;
			}
			orthorectification(DEMReferencefilename, finalAlignmentPaths);
		}
		else
			emit orthorectificationFinished();
	}));

	if (ui.widget_5->isEnabled())
	{
		// 先断开之前的连接（避免重复调用）
		disconnect(this, &SystemConfig::FreeNetworkAdjustFinished,
			m_imagePS, qOverload<>(&IMAGEPS::EncreadResidualData));
		// 重新连接信号
		connections.append(connect(this, &SystemConfig::FreeNetworkAdjustFinished,
			m_imagePS, qOverload<>(&IMAGEPS::EncreadResidualData)));
	}

	// 自由网平差完成信号连接
	connections.append(connect(this, &SystemConfig::FreeNetworkAdjustFinished, this,
		[=]() {
		if (ui.widget_6->isEnabled())
		{
			dataModelConfig->BuildSatModelMakerCmdXML(SatelliteImagefilename, "CtlPointMatch");
			//controlNetAdjustmergeFile(controlPointsPath_Or);
			controlNetAdjust(DEMReferencefilename);
		}
		else
			emit controlNetAdjustFinished();
	}));
	//static QMap<QString, bool> controlPointsPath_Or;
	// 控制点匹配完成信号连接
	connections.append(connect(this, &SystemConfig::controlPointsMatchFinished, this,
		[=]() {
		m_imagePS->controlPointsPath = controlPointsPath_Or;
		if (ui.widget_5->isEnabled())
		{
			dataModelConfig->BuildSatModelMakerCmdXML(SatelliteImagefilename, "SatBA");
			FreeNetworkAdjustmergeFile();
			FreeNetworkAdjust(DEMReferencefilename);
		}
		else
			emit FreeNetworkAdjustFinished();
	}));

	if (ui.widget_3->isEnabled())
	{
		disconnect(this, &SystemConfig::controlPointsMatchFinished,
			m_imagePS, qOverload<>(&IMAGEPS::ControlPointsList));

		connections.append(connect(this, &SystemConfig::controlPointsMatchFinished,
			m_imagePS, qOverload<>(&IMAGEPS::ControlPointsList)));
	}

	if (ui.widget_2->isEnabled())
	{
		// 先断开之前的连接（避免重复调用）
		disconnect(this, &SystemConfig::EncryptedPointsMatchFinished,
			m_imagePS, qOverload<>(&IMAGEPS::EncryptedPointsList));

		// 加密点匹配完成信号连接
		connections.append(connect(this, &SystemConfig::EncryptedPointsMatchFinished,
			m_imagePS, qOverload<>(&IMAGEPS::EncryptedPointsList)));
	}

	connections.append(connect(this, &SystemConfig::EncryptedPointsMatchFinished, this,
		[=]() {
		if (ui.widget_3->isEnabled())
		{
			controlPointsMatch(DEMReferencefilename, DOMReferencefilename, SatelliteImagefilename, controlPointsPath_Or);
		}
		else
			emit controlPointsMatchFinished();
	}));

	connections.append(connect(this, &SystemConfig::CreatepyramidFinished, this,
		[=]() {
		if (ui.widget_2->isEnabled())
		{
			dataModelConfig = m_imagePS->getDataModelConfig();

			if (dataModelConfig->ui.tableWidget_2->rowCount() == 0) {
				PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("连接点匹配 没有可用的匹配模型，请先构建连接点匹配模型！"));

				QMessageBox::warning(this, u8"警告", QString::fromLocal8Bit("没有可用的匹配模型，请先构建连接点匹配模型！"));
				emit FunctionAbnormalExit(QString::fromLocal8Bit("****没有可用的匹配模型，请先构建连接点匹配模型！"));
				return;
			}

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
				PublicFunctions::writeTimestampToXml("SatTiePointMatch", m_imagePS->projectdir);
				QDateTime startTime = QDateTime::currentDateTime();
				logEdit->append(QString::fromLocal8Bit("****连接点匹配"));
				logEdit->append(QString::fromLocal8Bit("****开始时间：") + startTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));

				logEdit->append(QString::fromLocal8Bit("****当前") + QString::number(dataModelConfig->ui.tableWidget_2->rowCount()) + QString::fromLocal8Bit("个模型已存在匹配成功数据"));

				QDateTime endTime = QDateTime::currentDateTime();
				logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
				logEdit->append(QString::fromLocal8Bit("****连接点匹配耗时：") + calculateTimeDifference(startTime, endTime));
				emit EncryptedPointsMatchFinished();
				return;
			}

			connections.append(connect(this, &SystemConfig::EncryptedPointsMatchFinished,
				dataModelConfig, [=]() {
				for (int row = 0; row < dataModelConfig->ui.tableWidget_2->rowCount(); ++row) {
					// 创建新的 QTableWidgetItem 
					QTableWidgetItem *item = new QTableWidgetItem(QString::fromLocal8Bit("是"));

					item->setTextAlignment(Qt::AlignCenter);
					// 设置该项到指定行列 
					dataModelConfig->ui.tableWidget_2->setItem(row, 3, item);
				}
			}));

			connectPointsMatch(DEMReferencefilename, DOMReferencefilename, "SatTiePointMatch");
		}
		else
			emit EncryptedPointsMatchFinished();
	}));

	if (ui.widget->isEnabled())
		acceptModule(QString::fromLocal8Bit("金字塔创建"), SatelliteImagefilename);
	else
		emit CreatepyramidFinished();
}

/**
 * @brief 自定义流程
 */
 // 执行自定义流程
void SystemConfig::executeCustomProcess(const QStringList& selectedProcesses, const QStringList& inputFiles)
{
	if (selectedProcesses.isEmpty() || inputFiles.isEmpty()) {
		QMessageBox::warning(this,
			QString::fromLocal8Bit("警告"),
			QString::fromLocal8Bit("流程选择或输入数据为空！"));
		return;
	}

	m_selectedProcesses = selectedProcesses;
	m_processInputFiles = inputFiles;
	m_currentProcessIndex = -1;
	m_processQueue.clear();

	// 初始化流程队列
	for (const auto& process : selectedProcesses) {
		m_processQueue.enqueue(process);
	}

	logEdit->append(QString::fromLocal8Bit("==== 开始执行自定义流程 ===="));
	logEdit->append(QString::fromLocal8Bit("流程顺序: ") + selectedProcesses.join("  -> "));

	// 开始执行第一个流程
	executeNextProcess();
}

// 执行下一个流程
void SystemConfig::executeNextProcess()
{
	if (m_processQueue.isEmpty()) {
		logEdit->append(QString::fromLocal8Bit("==== 所有流程执行完成 ===="));
		emit processSequenceFinished();
		return;
	}

	QString currentProcess = m_processQueue.dequeue();
	m_currentProcessIndex++;

	logEdit->append(QString::fromLocal8Bit("\n准备执行: ") + currentProcess);

	// 根据当前流程调用相应功能 
	if (currentProcess == QString::fromLocal8Bit("金字塔创建")) {
		connect(this, &SystemConfig::CreatepyramidFinished, this, &SystemConfig::onCurrentProcessFinished);
		acceptModule(currentProcess, m_processInputFiles);
	}
	else if (currentProcess == QString::fromLocal8Bit("影像融合")) {
		connect(this, &SystemConfig::imageInterActionFinished, this, &SystemConfig::onCurrentProcessFinished);
		imageInterAction();
	}
	else if (currentProcess == QString::fromLocal8Bit("真彩色转换")) {
		connect(this, &SystemConfig::TrueColorConversionFinished, this, &SystemConfig::onCurrentProcessFinished);
		TrueColorConversion(m_processInputFiles);
	}
	else if (currentProcess == QString::fromLocal8Bit("影像匀色")) {
		connect(this, &SystemConfig::ImageColorCorrectionFinished, this, &SystemConfig::onCurrentProcessFinished);
		ImageColorCorrection(m_processInputFiles);
	}
	else if (currentProcess == QString::fromLocal8Bit("影像镶嵌")) {
		connect(this, &SystemConfig::ImageMosaicFinished, this, &SystemConfig::onCurrentProcessFinished);
		ImageMosaic(m_processInputFiles);
	}
	else if (currentProcess == QString::fromLocal8Bit("影像裁切")) {
		connect(this, &SystemConfig::ImageCropFinished, this, &SystemConfig::onCurrentProcessFinished);
		ImageCropping(m_processInputFiles, {}, QDir::tempPath());
	}
	else if (currentProcess == QString::fromLocal8Bit("智能镶嵌")) {
		connect(this, &SystemConfig::SmartMosaicFinished, this, &SystemConfig::onCurrentProcessFinished);
		SmartMosaic(m_processInputFiles, {});
	}
	else if (currentProcess == QString::fromLocal8Bit("快速镶嵌")) {
		connect(this, &SystemConfig::ImageMosaicFinished, this, &SystemConfig::onCurrentProcessFinished);
		QuickMosaic(m_processInputFiles);
	}
	else if (currentProcess == QString::fromLocal8Bit("投影转换")) {
		connect(this, &SystemConfig::ProjectionTransformationFinished, this, &SystemConfig::onCurrentProcessFinished);
		ProjectionTransformation(m_processInputFiles);
	}
	else {
		logEdit->append(QString::fromLocal8Bit("未知流程: ") + currentProcess);
		executeNextProcess(); // 跳过未知流程
	}
}

// 当前流程完成处理
void SystemConfig::onCurrentProcessFinished()
{
	// 断开之前的连接 
	disconnect(this, &SystemConfig::CreatepyramidFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::imageInterActionFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::TrueColorConversionFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::ImageColorCorrectionFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::ImageMosaicFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::ImageCropFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::SmartMosaicFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::ImageMosaicFinished, this, &SystemConfig::onCurrentProcessFinished);
	disconnect(this, &SystemConfig::ProjectionTransformationFinished, this, &SystemConfig::onCurrentProcessFinished);
	// 其他类似断开操作...

	logEdit->append(QString::fromLocal8Bit("流程完成: ") +
		m_selectedProcesses.value(m_currentProcessIndex));

	// 执行下一个流程 
	executeNextProcess();
}

//////////////////
void SystemConfig::setPROJCStextSlot(QString PROJCStext)
{
	if (projectSetting.openerButtonName() == "normalCorrect") {
		ui.lineEdit_5->setText(PROJCStext);
	}
	else if (projectSetting.openerButtonName() == "projectChange") {
		ui.lineEdit_18->setText(PROJCStext);
	}
}

void SystemConfig::okButtonSlot()
{
	saveSettings(); // 保存当前配置 
	QMessageBox::information(this,
		u8"提示",
		u8"配置已保存并生效！",
		u8"确定");
	this->accept(); // 关闭对话框并返回 QDialog::Accepted 
}

void SystemConfig::cancelButtonSlot()
{
	QMessageBox msgBox(
		QMessageBox::Question,
		u8"确认",
		u8"放弃所有修改并关闭？",
		QMessageBox::Yes | QMessageBox::No,
		this
	);

	msgBox.setButtonText(QMessageBox::Yes, u8"是");
	msgBox.setButtonText(QMessageBox::No, u8"否");
	msgBox.setDefaultButton(QMessageBox::No);

	if (msgBox.exec() == QMessageBox::Yes) {
		loadSettings();
		this->reject(); // 关闭对话框并返回 QDialog::Rejected
	}
}

void SystemConfig::applyButtonSlot()
{
	saveSettings();
	QMessageBox::information(this, u8"提示", u8"配置已保存！", u8"确定");
}

void SystemConfig::closeEvent(QCloseEvent *event)
{
	// 如果是来自aboutToClose信号的关闭 
	if (m_isAboutToClose) {
		event->accept();
		return;
	}
	//if (QMessageBox::question(this, u8"确认", u8"放弃所有修改并关闭？",
	//	QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
	//{
		loadSettings();
		this->reject(); // 关闭对话框并返回 QDialog::Rejected 
	//}
}

void SystemConfig::on_pushButton_clicked()
{
	// 打开文件对话框，获取文件路径 
	QString filePath = QFileDialog::getOpenFileName(
		this,                                   // 父窗口 
		tr(u8"选择 TIFF 或 IMG 文件"),            // 对话框标题 
		QDir::homePath(),                       // 默认打开目录（用户主目录）
		tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)"))   
	);

	// 如果用户选择了文件（非空路径）
	if (!filePath.isEmpty()) {
		// 将文件路径写入 lineEdit_20 
		ui.lineEdit_20->setText(filePath);

		// 设置文本左对齐 
		ui.lineEdit_20->setAlignment(Qt::AlignLeft);
		ui.lineEdit_20->setCursorPosition(0);
	}
}

void SystemConfig::on_pushButton_2_clicked()
{
	QString dirPath = QFileDialog::getExistingDirectory(
		this,                                   // 父窗口 
		tr(u8"选择文件夹"),                       // 对话框标题 
		QDir::homePath(),                       // 默认打开目录（用户主目录）
		QFileDialog::ShowDirsOnly               // 只显示文件夹 
	);

	if (!dirPath.isEmpty()) {
		ui.lineEdit_21->setText(dirPath);

		// 设置文本左对齐 
		ui.lineEdit_21->setAlignment(Qt::AlignLeft);
		ui.lineEdit_21->setCursorPosition(0);

	}
}

void SystemConfig::on_pushButton_3_clicked()
{
	// 打开文件对话框，获取文件路径 
	QString filePath = QFileDialog::getOpenFileName(
		this,                                
		tr(u8"选择 TIFF 或 IMG 文件"),       
		QDir::homePath(),                   
		tr("%1;;TIF Image(*.tif);;TIFF Image(*.tiff);;IMAGINE images Files(*.img);;DigitalGlobe image Files(*til);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img *.til)"))
	);

	// 如果用户选择了文件（非空路径）
	if (!filePath.isEmpty()) {
		// 将文件路径写入 lineEdit_20 
		ui.lineEdit_25->setText(filePath);

		// 设置文本左对齐 
		ui.lineEdit_25->setAlignment(Qt::AlignLeft);
		ui.lineEdit_25->setCursorPosition(0);
	}
}

void SystemConfig::on_toolButton_clicked()
{
	// 打开文件对话框，获取文件路径 
	QString filePath = QFileDialog::getOpenFileName(
		this,
		tr(u8"选择矢量文件"),
		QDir::homePath(),
		tr("shapeFile(*.shp)")
	);

	// 如果用户选择了文件（非空路径）
	if (!filePath.isEmpty()) {
		// 将文件路径写入 lineEdit_20 
		ui.lineEdit_23->setText(filePath);

		// 设置文本左对齐 
		ui.lineEdit_23->setAlignment(Qt::AlignLeft);
		ui.lineEdit_23->setCursorPosition(0);
	}
}

void SystemConfig::on_toolButton_2_clicked()
{
	// 打开文件对话框，获取文件路径 
	QString filePath = QFileDialog::getOpenFileName(
		this,
		tr(u8"选择裁切矢量文件"),
		QDir::homePath(),
		tr("shapeFile(*.shp);;All Files(*.*)")
	);

	// 如果用户选择了文件（非空路径）
	if (!filePath.isEmpty()) {
		// 将文件路径写入 lineEdit_20 
		ui.lineEdit_37->setText(filePath);

		// 设置文本左对齐 
		ui.lineEdit_37->setAlignment(Qt::AlignLeft);
		ui.lineEdit_37->setCursorPosition(0);
	}
}

void SystemConfig::on_toolButton_3_clicked()
{ 
	QFileDialog fileDialog(this);

	// 设置对话框属性 
	fileDialog.setWindowTitle(tr(u8"选择文件")); // 修改对话框标题
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen); // 改为打开模式 
	fileDialog.setFileMode(QFileDialog::ExistingFile); // 只能选择已存在的文件 

	// 设置文件过滤器
	fileDialog.setNameFilter(tr(u8"Shape文件 (*.shp);;所有文件 (*.*)"));

	//显示对话框并等待用户选择 
	if (fileDialog.exec() == QDialog::Accepted) {
		
		QString selectedFile = fileDialog.selectedFiles().first();

		ui.LineEdit_2->setText(selectedFile);
	}
}

SystemConfig::~SystemConfig()
{
}

QString SystemConfig::calculateTimeDifference(const QDateTime &start, const QDateTime &end)
{
	// 计算时间差（毫秒）
	qint64 msecs = start.msecsTo(end);

	// 处理负的时间差（如果结束时间早于开始时间）
	bool isNegative = msecs < 0;
	msecs = qAbs(msecs);

	// 计算各个时间单位
	int hours = msecs / (1000 * 60 * 60);
	msecs = msecs % (1000 * 60 * 60);

	int minutes = msecs / (1000 * 60);
	msecs = msecs % (1000 * 60);

	int seconds = msecs / 1000;
	int milliseconds = msecs % 1000;

	// 格式化字符串
	QString result = QString::fromLocal8Bit("%1%2时%3分%4秒%5毫秒")
		.arg(isNegative ? "-" : "")
		.arg(hours, 2, 10, QLatin1Char('0'))
		.arg(minutes, 2, 10, QLatin1Char('0'))
		.arg(seconds, 2, 10, QLatin1Char('0'))
		.arg(milliseconds, 3, 10, QLatin1Char('0'));

	return result;
}

/**
 * @brief 获取图像类型
 */
QString SystemConfig::getTiffImageType(const QString &filePath) {

	// 初始化GDAL(只执行一次)
	static bool gdalInitialized = []() {
		GDALAllRegister();
		return true;
	}();
	Q_UNUSED(gdalInitialized);

	// 尝试打开文件
	GDALDataset *poDataset = static_cast<GDALDataset*>(GDALOpen(filePath.toUtf8().constData(), GA_ReadOnly));
	if (poDataset == nullptr) {
		PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("gdal打开影像失败,返回Invalid"));
		//qWarning() << "GDAL failed to open file:" << filePath;
		return "Invalid";
	}

	// 获取波段数量
	const int bandCount = poDataset->GetRasterCount();
	if (bandCount <= 0) {
		GDALClose(poDataset);
		PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("gdal读取影像波段数为0,返回Invalid"));

		return "Invalid";
	}

	// 获取第一个波段的信息
	GDALRasterBand *poFirstBand = poDataset->GetRasterBand(1);
	const GDALDataType dataType = poFirstBand->GetRasterDataType();

	// 收集波段描述信息
	QStringList bandDescriptions;
	for (int i = 1; i <= bandCount; ++i) {
		const char *desc = poDataset->GetRasterBand(i)->GetDescription();
		if (desc && strlen(desc) > 0) {
			bandDescriptions << QString(desc);
		}
	}

	// 关闭数据集
	GDALClose(poDataset);

	// 判断图像类型
	if (bandCount == 1) {
		// 单波段图像
		if (dataType == GDT_Byte || dataType == GDT_UInt16) {
			if (!bandDescriptions.isEmpty()) {
				return "PAN";
			}
			return "PAN";
		}
		return "Single Band (Not Panchromatic)";
	}
	else if (bandCount == 3 || bandCount == 4) {
		// 多光谱图像(RGB或RGBA)
		if (!bandDescriptions.isEmpty()) {
			return "MSS";
		}
		return "MSS";
	}
	else if (bandCount > 4) {
		// 高光谱或多光谱图像
		if (!bandDescriptions.isEmpty()) {
			return "MSS";
		}
		return "MSS";
	}

	return "Unknown";
}

/**
 * @brief 将QMap<QString, bool>中的路径和标志写入TmpOutPath.xml 文件
 * @param mapData 要写入的QMap数据
 * @param mapType 标识是controlPointsPath还是imageInterPath
 * @return 成功返回true，失败返回false
 */
bool SystemConfig::writeMapToXml(const QMap<QString, bool>& mapData, const QString& mapType)
{
	setProjectdir();
	QString xmlPath = projectdir + "TmpOutPath.xml";
	QDomDocument doc;

	// 1. 检查文件是否存在
	bool fileExists = QFile::exists(xmlPath);

	// 2. 如果文件存在，尝试读取内容 
	if (fileExists) {
		QFile file(xmlPath);
		if (!file.open(QIODevice::ReadOnly)) {
			PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("TmpOutPath.xml文件打开失败"));

			//qWarning() << "Failed to open existing file for reading:" << xmlPath;
			// 文件打开失败时仍然继续，创建新的XML结构 
		}
		else {
			if (!doc.setContent(&file)) {
				PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("TmpOutPath.xml文件解析失败"));

				//qWarning() << "Failed to parse existing XML file, will create new structure";
				// 解析失败时创建新的doc对象
				doc = QDomDocument();
			}
			file.close();
		}
	}

	// 3. 如果doc没有documentElement（文件不存在或解析失败），创建新的XML结构 
	if (doc.documentElement().isNull()) {
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);
		QDomElement root = doc.createElement("TmpPath");
		doc.appendChild(root);
	}

	// 获取根节点（此时应该一定存在）
	QDomElement root = doc.documentElement();

	// 3. 确定要操作的节点名称 
	QString elementName;
	if (mapType == "controlPointsPath") {
		elementName = "conOutPath";
	}
	else if (mapType == "imageInterPath") {
		elementName = "FusionOutPath";
	}
	else {
		qWarning() << "Invalid mapType:" << mapType;
		return false;
	}

	// 4. 查找或创建对应的父节点 
	QDomElement parentElement;
	QDomNodeList nodes = root.elementsByTagName(elementName);

	if (nodes.isEmpty()) {
		// 创建新节点
		parentElement = doc.createElement(elementName);
		root.appendChild(parentElement);
	}
	else {
		// 使用现有节点 
		parentElement = nodes.at(0).toElement();
		// 清空现有内容 
		while (parentElement.hasChildNodes()) {
			parentElement.removeChild(parentElement.firstChild());
		}
	}

	// 5. 遍历mapData，添加Path节点
	for (auto it = mapData.constBegin(); it != mapData.constEnd(); ++it) {
		QDomElement pathElement = doc.createElement("Path");

		QDomElement outPathElement = doc.createElement("OutPath");
		outPathElement.appendChild(doc.createTextNode(it.key()));
		pathElement.appendChild(outPathElement);

		QDomElement flagElement = doc.createElement("Flag");
		flagElement.appendChild(doc.createTextNode(it.value() ? "true" : "false"));
		pathElement.appendChild(flagElement);

		parentElement.appendChild(pathElement);
	}

	// 6. 写入文件 
	QFile file(xmlPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("TmpOutPath.xml文件写入保存失败"));

		//qWarning() << "Failed to open file for writing:" << xmlPath;
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	doc.save(out, 4);
	file.close();

	return true;
}

 /**
  * @brief 从TmpOutPath.xml 读取路径和标志到QMap<QString, bool>
  * @param mapData 要填充的QMap数据（输出参数）
  * @param mapType 标识是controlPointsPath还是imageInterPath
  * @return 成功返回true，失败返回false
  */
bool SystemConfig::readMapFromXml(QMap<QString, bool>& mapData, const QString& mapType)
{
	mapData.clear();
	setProjectdir();
	// 检查文件是否存在 
	QString xmlPath = projectdir + "TmpOutPath.xml";
	if (!QFile::exists(xmlPath)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("TmpOutPath.xml文件不存在"));

		//qWarning() << "XML file not found:" << xmlPath;
		return false;
	}

	// 打开XML文件 
	QFile file(xmlPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("TmpOutPath.xml文件打开失败"));

		//qWarning() << "Failed to open file for reading:" << xmlPath;
		return false;
	}

	// 解析XML内容 
	QDomDocument doc;
	QString errorMsg;
	int errorLine, errorColumn;
	if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("TmpOutPath.xml文件解析失败") + QString::number(errorLine) + "column" + QString::number(errorColumn) + ":" + errorMsg);

		//qWarning() << "XML parse error at line" << errorLine << "column" << errorColumn
		//	<< ":" << errorMsg;
		file.close();
		return false;
	}
	file.close();

	// 获取根节点 
	QDomElement root = doc.documentElement();
	if (root.isNull() || root.tagName() != "TmpPath") {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("TmpOutPath.xml文件解析失败"));

		//qWarning() << "Invalid root node or missing TmpPath tag";
		return false;
	}

	// 根据mapType决定读取哪个节点 
	QString parentTagName;
	if (mapType == "controlPointsPath") {
		parentTagName = "conOutPath";
	}
	else if (mapType == "imageInterPath") {
		parentTagName = "FusionOutPath";
	}
	else {
		qWarning() << "Invalid mapType:" << mapType;
		return false;
	}

	QDomElement parentElement = root.firstChildElement(parentTagName);
	if (parentElement.isNull()) {
		qWarning() << "Parent element not found:" << parentTagName;
		return false;
	}

	// 遍历所有Path节点 
	QDomElement pathElement = parentElement.firstChildElement("Path");
	while (!pathElement.isNull()) {
		QDomElement outPathElement = pathElement.firstChildElement("OutPath");
		QDomElement flagElement = pathElement.firstChildElement("Flag");

		if (outPathElement.isNull() || flagElement.isNull()) {
			qWarning() << "Missing OutPath or Flag element in Path node";
			pathElement = pathElement.nextSiblingElement("Path");
			continue;
		}

		QString outPath = outPathElement.text();
		QString flag = flagElement.text();

		if (outPath.isEmpty()) {
			qWarning() << "Empty OutPath found";
			pathElement = pathElement.nextSiblingElement("Path");
			continue;
		}

		// 处理Flag值（兼容大小写和空值情况）
		bool flagValue = false;
		if (!flag.isEmpty()) {
			flagValue = (flag.compare("true", Qt::CaseInsensitive) == 0);
		}

		mapData.insert(outPath, flagValue);
		pathElement = pathElement.nextSiblingElement("Path");
	}

	return true;
}

/**
 * @brief 创建匀色控制网任务单
 * @param inputImagePath 输入图像路径
 * @param DodgingTemplateLibDir 模版库文件路径
 * @return QStringList 返回xml文件路径集合
 */
QStringList SystemConfig::createDodgingControlFile(
	const QStringList& inputImagePath,
	const QString& DodgingTemplateLibDir
) {
	int ProcessingMethodnum = ui.imageEventColor_eventLightColorFunBox->currentIndex();
	int AdjustClearnessType = ui.imageEventColor_improveClarityBox->currentIndex();
	int BlockSize = ui.lineEdit_15->text().toInt();
	int MaskMethod = ui.comboBox_2->currentIndex();

	double DiffLimitCoef = ui.lineEdit_16->text().toDouble();
	double VarDiffLimitCoef = ui.lineEdit_17->text().toDouble();
	double DehazeCoef = ui.lineEdit_28->text().toDouble();
	double ExposeControlThrehold = ui.lineEdit_19->text().toDouble();
	double BrightnessCoef = ui.lineEdit_27->text().toDouble();
	//double ContrastCoef = ui.lineEdit_26->text().toDouble();
	double DarkControlThrehold = ui.lineEdit_53->text().toDouble();
	double HierarchicalFactor = ui.lineEdit_26->text().toDouble();

	QString StdFile = ui.lineEdit_20->text();
	QString CloudMaskFile = ui.lineEdit_25->text();
	
	bool UseCloudMask = ui.checkBox_26->isChecked();
	bool CreatePyd = ui.checkBox_21->isChecked();
	bool DiffLimit = ui.checkBox_22->isChecked();
	bool Stretch = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool Sharpen = ui.checkBox_30->isChecked();
	bool Dehaze = ui.checkBox_32->isChecked();
	bool UseDodgingTemplateLib = ui.imageEventColor_useModuleDBCheckBox->isChecked();
	bool SpecProcessWater = ui.checkBox_33->isChecked();
	bool ExposeControl = ui.checkBox_23->isChecked();
	bool AdjustBrightness = ui.checkBox_31->isChecked();
	bool DarkControl = ui.checkBox_73->isChecked();
	bool CreateOverViewFile = ui.checkBox_36->isChecked();
	bool IsSingleColor = ui.checkBox_20->isChecked();
	bool AdjustClearness = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool ExposeControl_UseAutoGamma = ui.checkBox_24->isChecked();

	QString outputDir;
	if (ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir = projectdir + "Dodging/tmp";
	}
	else {
		outputDir = projectdir + "Dodging";
	}

	int index = 0;
	QStringList xmlfilses;
	for (auto filename : inputImagePath)
	{
		// 创建 QDomDocument 
		QDomDocument doc;

		// 添加 XML 声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <DataUnit>
		QDomElement root = doc.createElement("DataUnit");
		doc.appendChild(root);

		// 1. 创建InputFiles节点 
		QDomElement inputFiles = doc.createElement("InputFiles");
		root.appendChild(inputFiles);


		// 添加ImageFile节点 
		QDomElement imageFile = doc.createElement("ImageFile");
		imageFile.appendChild(doc.createTextNode(filename));
		inputFiles.appendChild(imageFile);

		// 2. 创建Parameters节点 
		QDomElement parameters = doc.createElement("Parameters");
		root.appendChild(parameters);

		// 创建ColorParameters节点 
		QDomElement colorParameters = doc.createElement("ColorParameters");
		parameters.appendChild(colorParameters);

		// Lambda函数：添加参数节点 
		auto addParameter = [&](const QString& name, const QString& type, const QString& value) {
			QDomElement elem = doc.createElement(name);
			elem.setAttribute("type", type);
			elem.appendChild(doc.createTextNode(value));
			colorParameters.appendChild(elem);
		};

		// Lambda函数：添加参数节点 
		auto addParameterNO = [&](const QString& name, const QString& value) {
			QDomElement elem = doc.createElement(name);
			elem.appendChild(doc.createTextNode(value));
			colorParameters.appendChild(elem);
		};

		// 添加各种颜色处理参数 
		addParameter("IsSingleColor", "int32", QString::number(IsSingleColor));
		addParameter("SingleColorCoef", "float64", "0.30000000");
		addParameter("ProcessingMethod", "int32", QString::number(ProcessingMethodnum));
		addParameter("BlockSize", "int32", QString::number(BlockSize));
		addParameterNO("StdFile", StdFile);
		addParameter("UseCloudMask", "int32", QString::number(UseCloudMask));
		addParameterNO("CloudMaskFile", CloudMaskFile);
		addParameter("CreatePyd", "int32", QString::number(CreatePyd));
		addParameter("DiffLimit", "int32", QString::number(DiffLimit));
		addParameter("DiffLimitCoef", "float64", QString::number(DiffLimitCoef));
		addParameter("VarDiffLimitCoef", "float64", QString::number(VarDiffLimitCoef));
		addParameter("Stretch", "int32", QString::number(Stretch));
		addParameter("Sharpen", "int32", QString::number(Sharpen));
		addParameter("CreateOverViewFile", "int32", QString::number(CreateOverViewFile));
		addParameter("Dehaze", "int32", QString::number(Dehaze));
		addParameter("DehazeCoef", "float64", QString::number(DehazeCoef));
		addParameter("UseDodgingTemplateLib", "int32", QString::number(UseDodgingTemplateLib));
		addParameterNO("DodgingTemplateLibDir",  DodgingTemplateLibDir);
		addParameter("SpecProcessWater", "int32", QString::number(SpecProcessWater));
		addParameter("MinCoef", "float64", "0.25000000");
		addParameter("MaxCoef", "float64", "0.25000000");
		addParameter("ExposeControl", "int32", QString::number(ExposeControl));
		addParameter("ExposeControl_UseAutoGamma", "int32", QString::number(ExposeControl_UseAutoGamma));
		addParameter("ExposeControlThrehold", "float64", QString::number(ExposeControlThrehold));
		addParameter("DarkControl", "int32", QString::number(DarkControl));
		addParameter("DarkControlThrehold", "float64", QString::number(DarkControlThrehold));
		addParameter("HierarchicalFactor", "float64", QString::number(HierarchicalFactor));
		addParameter("AdjustBrightness", "int32", QString::number(AdjustBrightness));
		addParameter("BrightnessCoef", "float64", QString::number(BrightnessCoef));
		addParameter("AdjustContrast", "int32", QString::number(0));
		addParameter("ContrastCoef", "float64", QString::number(0.10000));
		addParameter("AdjustClearness", "int32", QString::number(AdjustClearness));
		addParameter("AdjustClearnessType", "int32", QString::number(AdjustClearnessType));
		addParameter("MaskMethod", "int32", QString::number(MaskMethod));
		addParameter("Tasks", "int32", "4");

		// 3. 创建OutPutFiles节点 
		QDomElement outPutFiles = doc.createElement("OutPutFiles");
		root.appendChild(outPutFiles);

		QString tmp = outputDir + "/";
		QString outputPath = tmp;
		// 添加OutPath节点 
		QDomElement outPath = doc.createElement("OutPath");
		outPath.appendChild(doc.createTextNode(outputPath));
		outPutFiles.appendChild(outPath);

		// 添加注释 
		QDomComment comment = doc.createComment(QString::fromLocal8Bit("控制网匀色"));
		//root.appendChild(comment);

		// 写入文件 
		QString xmlPath = outputDir + "/control_PSDodgingCmd" + QString::number(index) + ".xml";
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, xmlPath + QString::fromLocal8Bit("任务单创建失败"));

			//qWarning() << "Failed to open file for writing:" << xmlPath;
			return xmlfilses;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);
		out << "\n" << comment.toComment();
		file.close();
		xmlfilses.push_back(xmlPath);
		index++;
	}

	return xmlfilses;
}


/**
 * @brief 创建自由网匀色Plus任务单(FreeNetwork_PSDodgingCmd_plus.xml)
 * @param inputImagePaths 输入图像路径列表
 * @param DodgingTemplateLibDir 模版库文件路径
 * @return QStringList 返回xml文件路径集合
 */
void SystemConfig::createFreeNetworkDodgingPlusFile(
	const QStringList& inputImagePaths,
	const QString& DodgingTemplateLibDir)
{
	createFNDPlusFilecurrentIndex = 0; // 当前处理的图像索引 
	createFNDPluslocalInputPaths = inputImagePaths; // 保存输入路径 
	createFNDPluslocalDodgingTemplateLibDir = DodgingTemplateLibDir; // 保存模板库路径

	QString outputDir;
	if (ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir =  "Dodging/tmp";
	}
	else {
		outputDir =  "Dodging";
	}

	// 初始化处理 
	if (createFNDPlusFilecurrentIndex == 0) {
		FreeNetwork_PSDodging_plusXmlFiles.clear();
		QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);

		// 建立新连接 - 每次IntersectCmd完成后处理 
		QObject::connect(this, &SystemConfig::intersectCmdFinished, this, [this, outputDir]() {
			// 处理当前索引对应的结果
			QString xmlPath = processCurrentImage(createFNDPlusFilecurrentIndex, createFNDPluslocalInputPaths, createFNDPluslocalDodgingTemplateLibDir);
			if (!xmlPath.isEmpty()) {
				FreeNetwork_PSDodging_plusXmlFiles.append(xmlPath);
			}
			// 处理下一个图像 
			createFNDPlusFilecurrentIndex++;
			if (createFNDPlusFilecurrentIndex < createFNDPluslocalInputPaths.size()) {
				QStringList mainImagePath;
				mainImagePath.append(createFNDPluslocalInputPaths[createFNDPlusFilecurrentIndex]);

				QStringList remainingPaths = createFNDPluslocalInputPaths;
				remainingPaths.removeAt(createFNDPlusFilecurrentIndex);

				if (!remainingPaths.isEmpty()) {
					IntersectCmd(remainingPaths, mainImagePath, QString::fromLocal8Bit("Image"), outputDir);
				}
			}
			else {
				QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
				// 所有处理完成，重置索引 
				createFNDPlusFilecurrentIndex = 0;
				emit allIntersectCmdsFinished(); // 发射所有处理完成的信号 
			}
		});

		// 开始处理第一个图像
		if (!createFNDPluslocalInputPaths.empty()) {
			QStringList mainImagePath;
			mainImagePath.append(createFNDPluslocalInputPaths[createFNDPlusFilecurrentIndex]);

			QStringList remainingPaths = createFNDPluslocalInputPaths;
			remainingPaths.removeAt(createFNDPlusFilecurrentIndex);

			if (!remainingPaths.isEmpty()) {
				IntersectCmd(remainingPaths, mainImagePath, QString::fromLocal8Bit("Image"), outputDir);
			}
		}
	}
}

/**
 * @brief 处理当前图像并生成XML文件
 * @param index 当前图像索引
 * @param inputImagePaths 所有输入图像路径
 * @param DodgingTemplateLibDir 模板库路径
 */
QString  SystemConfig::processCurrentImage(int index, const QStringList& inputImagePaths, const QString& DodgingTemplateLibDir)
{
	// 从UI控件获取参数值
	int ProcessingMethod = ui.imageEventColor_eventLightColorFunBox->currentIndex();
	int AdjustClearnessType = ui.imageEventColor_improveClarityBox->currentIndex();

	int BlockSize = ui.lineEdit_15->text().toInt();

	double DiffLimitCoef = ui.lineEdit_16->text().toDouble();
	double VarDiffLimitCoef = ui.lineEdit_17->text().toDouble();
	double DehazeCoef = ui.lineEdit_28->text().toDouble();
	double ExposeControlThrehold = ui.lineEdit_19->text().toDouble();
	double BrightnessCoef = ui.lineEdit_27->text().toDouble();
	double DarkControlThrehold = ui.lineEdit_53->text().toDouble();
	double HierarchicalFactor = ui.lineEdit_26->text().toDouble();

	QString StdFile = ui.lineEdit_20->text();
	QString CloudMaskFile = ui.lineEdit_25->text();

	bool UseCloudMask = ui.checkBox_26->isChecked();
	bool CreatePyd = ui.checkBox_21->isChecked();
	bool DiffLimit = ui.checkBox_22->isChecked();
	bool Stretch = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool Sharpen = ui.checkBox_30->isChecked();
	bool Dehaze = ui.checkBox_32->isChecked();
	bool UseDodgingTemplateLib = ui.imageEventColor_useModuleDBCheckBox->isChecked();
	bool SpecProcessWater = ui.checkBox_33->isChecked();
	bool ExposeControl = ui.checkBox_23->isChecked();
	bool AdjustBrightness = ui.checkBox_31->isChecked();
	bool DarkControl = ui.checkBox_73->isChecked();
	bool CreateOverViewFile = ui.checkBox_36->isChecked();
	bool IsSingleColor = ui.checkBox_20->isChecked();
	bool AdjustClearness = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool ExposeControl_UseAutoGamma = ui.checkBox_24->isChecked();

	QString mainImagePath = inputImagePaths[index];

	QString outputDir;
	if (ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir = "Dodging/tmp";
	}
	else {
		outputDir = "Dodging";
	}

	// 创建 QDomDocument 
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 <DataUnit>
	QDomElement root = doc.createElement("DataUnit");
	doc.appendChild(root);

	// 1. 创建InputFiles节点 
	QDomElement inputFiles = doc.createElement("InputFiles");
	root.appendChild(inputFiles);

	// 添加主ImageFile节点 
	QDomElement imageFile = doc.createElement("ImageFile");
	imageFile.appendChild(doc.createTextNode(mainImagePath));
	inputFiles.appendChild(imageFile);

	// 添加对应的ImageFileAdj节点 
	//QStringList remainingPaths = inputImagePaths;
	//remainingPaths.removeAt(index);
	//for (const QString& path : remainingPaths) {
		// 读取IntersectCmd生成的结果文件 
	QString targetFilePath = projectdir + outputDir + "/TargetFile" + QString::number(0) + "_Image.txt";
	QFile targetFile(targetFilePath);
	if (targetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QTextStream in(&targetFile);
		int lineCount = 0;
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			lineCount++;
			if (lineCount > 1) { // 跳过第一行（文件数量行）
				QDomElement adjFile = doc.createElement("ImageFileAdj");
				adjFile.appendChild(doc.createTextNode(line));
				inputFiles.appendChild(adjFile);
			}
		}
		targetFile.close();
	}
	else {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig,  QString::fromLocal8Bit("打开求交成果文件失败") + targetFilePath);

		//qWarning() << "无法打开目标文件:" << targetFilePath;
	}
	//}

	// 2. 创建Parameters节点
	QDomElement parameters = doc.createElement("Parameters");
	root.appendChild(parameters);

	// 创建ColorParameters节点 
	QDomElement colorParameters = doc.createElement("ColorParameters");
	parameters.appendChild(colorParameters);

	// Lambda函数：添加参数节点 
	auto addParameter = [&](const QString& name, const QString& type, const QString& value) {
		QDomElement elem = doc.createElement(name);
		elem.setAttribute("type", type);
		elem.appendChild(doc.createTextNode(value));
		colorParameters.appendChild(elem);
	};

	auto addParameterNO = [&](const QString& name, const QString& value) {
		QDomElement elem = doc.createElement(name);
		elem.appendChild(doc.createTextNode(value));
		colorParameters.appendChild(elem);
	};

	// 添加各种颜色处理参数
	addParameter("IsSingleColor", "int32", QString::number(IsSingleColor));
	addParameter("SingleColorCoef", "float64", "0.30000000");
	addParameter("ProcessingMethod", "int32", QString::number(ProcessingMethod));
	addParameter("BlockSize", "int32", QString::number(BlockSize));
	addParameterNO("StdFile", StdFile);
	addParameter("UseCloudMask", "int32", QString::number(UseCloudMask));
	addParameterNO("CloudMaskFile", CloudMaskFile);
	addParameter("CreatePyd", "int32", QString::number(CreatePyd));
	addParameter("DiffLimit", "int32", QString::number(DiffLimit));
	addParameter("DiffLimitCoef", "float64", QString::number(DiffLimitCoef));
	addParameter("VarDiffLimitCoef", "float64", QString::number(VarDiffLimitCoef));
	addParameter("Stretch", "int32", QString::number(Stretch));
	//addParameter("UseDodgingTemplateLib", "int32", QString::number(UseDodgingTemplateLib));
	//addParameterNO("DodgingTemplateLibDir", DodgingTemplateLibDir);
	//addParameter("SpecProcessWater", "int32", QString::number(SpecProcessWater));
	addParameter("MinCoef", "float64", "0.25000000");
	addParameter("MaxCoef", "float64", "0.25000000");
	addParameter("ExposeControl", "int32", QString::number(ExposeControl));
	addParameter("ExposeControl_UseAutoGamma", "int32", QString::number(ExposeControl_UseAutoGamma));
	addParameter("ExposeControlThrehold", "float64", QString::number(ExposeControlThrehold));
	addParameter("DarkControl", "int32", QString::number(DarkControl));
	addParameter("DarkControlThrehold", "float64", QString::number(DarkControlThrehold));
	addParameter("AdjustBrightness", "int32", QString::number(AdjustBrightness));
	addParameter("BrightnessCoef", "float64", QString::number(BrightnessCoef));
	addParameter("AdjustContrast", "int32", QString::number(0));
	addParameter("ContrastCoef", "float64", QString::number(0.10000));
	addParameter("BundleProcessInner", "float64", QString::number(0));
	addParameter("Sharpen", "int32", QString::number(Sharpen));
	addParameter("CreateOverViewFile", "int32", QString::number(CreateOverViewFile));
	addParameter("Dehaze", "int32", QString::number(Dehaze));
	addParameter("DehazeCoef", "float64", QString::number(DehazeCoef));
	//addParameter("AdjustClearness", "float64", QString::number(AdjustClearness));
	//addParameter("AdjustClearnessType", "float64", QString::number(AdjustClearnessType));
	addParameter("Tasks", "int32", QString::number(inputImagePaths.size()));

	// 3. 创建OutPutFiles节点 
	QDomElement outPutFiles = doc.createElement("OutPutFiles");
	root.appendChild(outPutFiles);

	QString tmp = projectdir + outputDir + "/";
	QString outputPath = tmp;

	// 添加OutPath节点
	QDomElement outPath = doc.createElement("OutPath");
	outPath.appendChild(doc.createTextNode(outputPath));
	outPutFiles.appendChild(outPath);

	// 创建注释节点
	QDomComment comment = doc.createComment(QString::fromLocal8Bit(" 自由网匀色Plus任务单"));

	// 写入文件 
	QString xmlPath = QDir(projectdir).filePath(outputDir + "/FreeNetwork_PSDodgingCmd_plus" + QString::number(index) + ".xml");
	QFile file(xmlPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, xmlPath + QString::fromLocal8Bit("任务单创建失败"));

		//qWarning() << "无法打开文件进行写入:" << xmlPath;
		return "";
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << doc.toString(4);
	out << "\n" << comment.toComment();
	file.close();

	return xmlPath;
}

// /**
//  * @brief 创建自由网匀色Plus任务单(FreeNetwork_PSDodgingCmd_plus.xml)
//  * @param inputImagePaths 输入图像路径列表
//  * @param DodgingTemplateLibDir 模版库文件路径
//  * @return QStringList 返回xml文件路径集合
//  */
//QStringList SystemConfig::createFreeNetworkDodgingPlusFile(
//	const QStringList& inputImagePaths,
//	const QString& DodgingTemplateLibDir)
//{
//	QStringList xmlFiles;
//	
//	// 1. 循环调用IntersectCmd，每个图像作为主图像一次 
//	if (!inputImagePaths.empty())
//	{
//		for (int i = 0; i < inputImagePaths.size(); ++i)
//		{
//			QStringList mainImagePath;
//			mainImagePath.append(inputImagePaths[i]);
//
//			// 获取剩余图像路径列表 
//			QStringList remainingPaths = inputImagePaths;
//			remainingPaths.removeAt(i);
//
//			if (!remainingPaths.isEmpty())
//			{
//				// 调用IntersectCmd，传入主图像和剩余图像 
//				IntersectCmd(remainingPaths, mainImagePath, QString::fromLocal8Bit("Image"), QString::fromLocal8Bit("Dodging"));
//			}
//		}
//	}
//
//	// 断开旧连接
//	QObject::disconnect(this, &SystemConfig::intersectCmdFinished, nullptr, nullptr);
//
//	// 2. 建立新连接 
//	QObject::connect(this, &SystemConfig::intersectCmdFinished, this, [&xmlFiles, this, inputImagePaths, DodgingTemplateLibDir]() {
//		int ProcessingMethod = ui.imageEventColor_eventLightColorFunBox->currentIndex();
//		int BlockSize = ui.lineEdit_15->text().toInt();
//		int CloudThrehold = ui.lineEdit_34->text().toInt();
//
//		double DiffLimitCoef = ui.lineEdit_16->text().toDouble();
//		double VarDiffLimitCoef = ui.lineEdit_17->text().toDouble();
//		double DehazeCoef = ui.lineEdit_32->text().toDouble();
//		double ExposeControlThrehold = ui.lineEdit_19->text().toDouble();
//		double BrightnessCoef = ui.lineEdit_27->text().toDouble();
//		double ContrastCoef = ui.lineEdit_26->text().toDouble();
//
//		QString StdFile = ui.lineEdit_20->text();
//		QString CloudMaskFile = ui.lineEdit_25->text();
//
//		bool UseCloudThrehold = ui.checkBox_27->isChecked();
//		bool CreatePyd = ui.checkBox_21->isChecked();
//		bool DiffLimit = ui.checkBox_22->isChecked();
//		bool Stretch = ui.imageEventColor_improveClarityCheckBox->isChecked();
//		bool Sharpen = ui.checkBox_30->isChecked();
//		bool Dehaze = ui.checkBox_32->isChecked();
//		bool UseDodgingTemplateLib = ui.imageEventColor_useModuleDBCheckBox->isChecked();
//		bool SpecProcessWater = ui.checkBox_33->isChecked();
//		bool ExposeControl = ui.checkBox_23->isChecked();
//		bool AdjustBrightness = ui.checkBox_31->isChecked();
//		bool AdjustContrast = ui.checkBox_28->isChecked();
//		bool CreateOverViewFile = ui.checkBox_36->isChecked();
//		bool IsSingleColor = ui.checkBox_20->isChecked();
//		bool BundleProcessInner = ui.checkBox_29->isChecked();
//
//		QStringList localXmlFiles;
//		int index = 0;
//
//		// 遍历每个主图像
//		for (int i = 0; i < inputImagePaths.size(); ++i) {
//			QString mainImagePath = inputImagePaths[i];
//
//			// 创建 QDomDocument 
//			QDomDocument doc;
//			QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
//			doc.appendChild(instruction);
//
//			// 创建根节点 <DataUnit>
//			QDomElement root = doc.createElement("DataUnit");
//			doc.appendChild(root);
//
//			// 1. 创建InputFiles节点
//			QDomElement inputFiles = doc.createElement("InputFiles");
//			root.appendChild(inputFiles);
//
//			// 添加主ImageFile节点
//			QDomElement imageFile = doc.createElement("ImageFile");
//			imageFile.appendChild(doc.createTextNode(mainImagePath));
//			inputFiles.appendChild(imageFile);
//
//			// 添加对应的ImageFileAdj节点 
//			QStringList remainingPaths = inputImagePaths;
//			remainingPaths.removeAt(i);
//			for (const QString& path : remainingPaths) {
//				// 读取IntersectCmd生成的结果文件
//				QString targetFilePath = projectdir + "Dodging/TargetFile" + QString::number(i) + "_Image.txt";
//				QFile targetFile(targetFilePath);
//				if (targetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//					QTextStream in(&targetFile);
//					int lineCount = 0;
//					while (!in.atEnd()) {
//						QString line = in.readLine().trimmed();
//						lineCount++;
//						if (lineCount > 1) { // 跳过第一行（文件数量行）
//							QDomElement adjFile = doc.createElement("ImageFileAdj");
//							adjFile.appendChild(doc.createTextNode(line));
//							inputFiles.appendChild(adjFile);
//						}
//					}
//					targetFile.close();
//				}
//				else {
//					qWarning() << "无法打开目标文件:" << targetFilePath;
//				}
//			}
//
//			// 2. 创建Parameters节点 
//			QDomElement parameters = doc.createElement("Parameters");
//			root.appendChild(parameters);
//
//			// 创建ColorParameters节点 
//			QDomElement colorParameters = doc.createElement("ColorParameters");
//			parameters.appendChild(colorParameters);
//
//			// Lambda函数：添加参数节点 
//			auto addParameter = [&](const QString& name, const QString& type, const QString& value) {
//				QDomElement elem = doc.createElement(name);
//				elem.setAttribute("type", type);
//				elem.appendChild(doc.createTextNode(value));
//				colorParameters.appendChild(elem);
//			};
//
//			auto addParameterNO = [&](const QString& name, const QString& value) {
//				QDomElement elem = doc.createElement(name);
//				elem.appendChild(doc.createTextNode(value));
//				colorParameters.appendChild(elem);
//			};
//
//			// 添加各种颜色处理参数（与之前相同）
//			addParameter("IsSingleColor", "int32", QString::number(IsSingleColor));
//			addParameter("SingleColorCoef", "float64", "0.30000000");
//			addParameter("ProcessingMethod", "int32", QString::number(ProcessingMethod));
//			addParameter("BlockSize", "int32", QString::number(BlockSize));
//			addParameterNO("StdFile", StdFile);
//			addParameter("UseCloudThrehold", "int32", QString::number(UseCloudThrehold));
//			addParameter("CloudThrehold", "int32", QString::number(CloudThrehold));
//			addParameterNO("CloudMaskFile", CloudMaskFile);
//			addParameter("CreatePyd", "int32", QString::number(CreatePyd));
//			addParameter("DiffLimit", "int32", QString::number(DiffLimit));
//			addParameter("DiffLimitCoef", "float64", QString::number(DiffLimitCoef));
//			addParameter("VarDiffLimitCoef", "float64", QString::number(VarDiffLimitCoef));
//			addParameter("Stretch", "int32", QString::number(Stretch));
//			addParameter("Sharpen", "int32", QString::number(Sharpen));
//			addParameter("CreateOverViewFile", "int32", QString::number(CreateOverViewFile));
//			addParameter("Dehaze", "int32", QString::number(Dehaze));
//			addParameter("DehazeCoef", "float64", QString::number(DehazeCoef));
//			addParameter("UseDodgingTemplateLib", "int32", QString::number(UseDodgingTemplateLib));
//			addParameterNO("DodgingTemplateLibDir", DodgingTemplateLibDir);
//			addParameter("SpecProcessWater", "int32", QString::number(SpecProcessWater));
//			addParameter("MinCoef", "float64", "0.25000000");
//			addParameter("MaxCoef", "float64", "0.25000000");
//			addParameter("ExposeControl", "int32", QString::number(ExposeControl));
//			addParameter("ExposeControlThrehold", "float64", QString::number(ExposeControlThrehold));
//			addParameter("AdjustBrightness", "int32", QString::number(AdjustBrightness));
//			addParameter("BrightnessCoef", "float64", QString::number(BrightnessCoef));
//			addParameter("AdjustContrast", "int32", QString::number(AdjustContrast));
//			addParameter("ContrastCoef", "float64", QString::number(ContrastCoef));
//			addParameter("BundleProcessInner", "int32", QString::number(BundleProcessInner));
//			addParameter("Tasks", "int32", QString::number(inputImagePaths.size()));
//
//			// 3. 创建OutPutFiles节点
//			QDomElement outPutFiles = doc.createElement("OutPutFiles");
//			root.appendChild(outPutFiles);
//
//			QString tmp = projectdir + QString::fromLocal8Bit("Dodging/");
//			QString outputPath = tmp;
//
//			// 添加OutPath节点
//			QDomElement outPath = doc.createElement("OutPath");
//			outPath.appendChild(doc.createTextNode(outputPath));
//			outPutFiles.appendChild(outPath);
//
//			// 创建注释节点 
//			QDomComment comment = doc.createComment(QString::fromLocal8Bit(" 自由网匀色Plus任务单"));
//
//			// 写入文件
//			QString xmlPath = QDir(projectdir).filePath("Dodging/FreeNetwork_PSDodgingCmd_plus" + QString::number(i) + ".xml");
//			QFile file(xmlPath);
//			if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
//				qWarning() << "无法打开文件进行写入:" << xmlPath;
//				continue;
//			}
//
//			QTextStream out(&file);
//			out.setCodec("UTF-8");
//			out << doc.toString(4);
//			out << "\n" << comment.toComment();
//			file.close();
//
//			xmlFiles.append(xmlPath);
//			index++;
//		}
//
//		//xmlFiles = localXmlFiles;
//	});
//
//	return xmlFiles;
//}

/**
 * @brief 创建Mask匀色任务单(_DodgingCmd.xml)
 * @param inputImagePaths 输入图像路径列表
 * @param DodgingTemplateLibDir 模版库文件路径
 * @return QStringList 返回xml文件路径集合
 */
QStringList SystemConfig::createMaskDodgingFile(
	const QStringList& inputImagePaths,
	const QString& DodgingTemplateLibDir)
{
	// 从UI控件获取参数值 
	int ProcessingMethod = ui.imageEventColor_eventLightColorFunBox->currentIndex();
	int AdjustClearnessType = ui.imageEventColor_improveClarityBox->currentIndex();
	int MaskMethod = ui.comboBox_2->currentIndex();

	int DarkControlThrehold = ui.lineEdit_53->text().toInt();
	int BlockSize = ui.lineEdit_15->text().toInt();

	double DiffLimitCoef = ui.lineEdit_16->text().toDouble();
	double VarDiffLimitCoef = ui.lineEdit_17->text().toDouble();
	double DehazeCoef = ui.lineEdit_28->text().toDouble();
	double ExposeControlThrehold = ui.lineEdit_19->text().toDouble();
	double BrightnessCoef = ui.lineEdit_27->text().toDouble();
	//double DarkControlThrehold = ui.lineEdit_53->text().toDouble();
	double HierarchicalFactor = ui.lineEdit_26->text().toDouble();

	QString StdFile = ui.lineEdit_20->text();
	QString CloudMaskFile = ui.lineEdit_25->text();

	bool UseCloudMask = ui.checkBox_26->isChecked();
	bool CreatePyd = ui.checkBox_21->isChecked();
	bool DiffLimit = ui.checkBox_22->isChecked();
	bool Stretch = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool Sharpen = ui.checkBox_30->isChecked();
	bool Dehaze = ui.checkBox_32->isChecked();
	bool UseDodgingTemplateLib = ui.imageEventColor_useModuleDBCheckBox->isChecked();
	bool SpecProcessWater = ui.checkBox_33->isChecked();
	bool ExposeControl = ui.checkBox_23->isChecked();
	bool DarkControl = ui.checkBox_73->isChecked();
	bool AdjustBrightness = ui.checkBox_31->isChecked();
	//bool DarkControl = ui.checkBox_73->isChecked();
	bool CreateOverViewFile = ui.checkBox_36->isChecked();
	bool IsSingleColor = ui.checkBox_20->isChecked();
	bool AdjustClearness = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool ExposeControl_UseAutoGamma = ui.checkBox_24->isChecked();

	QString outputDir;
	if (ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir =  "Dodging/tmp";
	}
	else {
		outputDir =  "Dodging";
	}

	int index = 0;
	QStringList xmlFiles;
	for (auto filename : inputImagePaths)
	{
		// 创建 QDomDocument 
		QDomDocument doc;

		// 添加 XML 声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <DataUnit>
		QDomElement root = doc.createElement("DataUnit");
		doc.appendChild(root);

		// 1. 创建InputFiles节点 
		QDomElement inputFiles = doc.createElement("InputFiles");
		root.appendChild(inputFiles);

		// 添加ImageFile节点 
		QDomElement imageFile = doc.createElement("ImageFile");
		imageFile.appendChild(doc.createTextNode(filename));
		inputFiles.appendChild(imageFile);

		// 2. 创建Parameters节点 
		QDomElement parameters = doc.createElement("Parameters");
		root.appendChild(parameters);

		// 创建ColorParameters节点 
		QDomElement colorParameters = doc.createElement("ColorParameters");
		parameters.appendChild(colorParameters);

		// Lambda函数：添加参数节点 
		auto addParameter = [&](const QString& name, const QString& type, const QString& value) {
			QDomElement elem = doc.createElement(name);
			elem.setAttribute("type", type);
			elem.appendChild(doc.createTextNode(value));
			colorParameters.appendChild(elem);
		};

		// Lambda函数：添加无类型参数节点 
		auto addParameterNO = [&](const QString& name, const QString& value) {
			QDomElement elem = doc.createElement(name);
			elem.appendChild(doc.createTextNode(value));
			colorParameters.appendChild(elem);
		};

		// 添加各种颜色处理参数 
		addParameter("IsSingleColor", "int32", QString::number(IsSingleColor));
		addParameter("SingleColorCoef", "float64", "0.30000000");
		addParameter("ProcessingMethod", "int32", QString::number(ProcessingMethod));
		addParameter("BlockSize", "int32", QString::number(BlockSize));
		addParameterNO("StdFile", StdFile);
		addParameter("UseCloudMask", "int32", QString::number(UseCloudMask));
		addParameterNO("CloudMaskFile", CloudMaskFile);
		addParameter("CreatePyd", "int32", QString::number(CreatePyd));
		addParameter("DiffLimit", "int32", QString::number(DiffLimit));
		addParameter("DiffLimitCoef", "float64", QString::number(DiffLimitCoef));
		addParameter("VarDiffLimitCoef", "float64", QString::number(VarDiffLimitCoef));
		addParameter("Stretch", "int32", QString::number(Stretch));
		addParameter("Sharpen", "int32", QString::number(Sharpen));
		addParameter("CreateOverViewFile", "int32", QString::number(CreateOverViewFile));
		addParameter("Dehaze", "int32", QString::number(Dehaze));
		addParameter("DehazeCoef", "float64", QString::number(DehazeCoef));
		addParameter("UseDodgingTemplateLib", "int32", QString::number(UseDodgingTemplateLib));
		addParameterNO("DodgingTemplateLibDir", DodgingTemplateLibDir);
		addParameter("SpecProcessWater", "int32", QString::number(SpecProcessWater));
		addParameter("MinCoef", "float64", "0.25000000");
		addParameter("MaxCoef", "float64", "0.25000000");
		addParameter("ExposeControl", "int32", QString::number(ExposeControl));
		addParameter("ExposeControl_UseAutoGamma", "int32", QString::number(ExposeControl_UseAutoGamma));
		addParameter("ExposeControlThrehold", "float64", QString::number(ExposeControlThrehold));
		addParameter("DarkControl", "int32", QString::number(DarkControl));
		addParameter("DarkControlThrehold", "float64", QString::number(DarkControlThrehold));
		addParameter("HierarchicalFactor", "float64", QString::number(HierarchicalFactor));
		addParameter("AdjustBrightness", "int32", QString::number(AdjustBrightness));
		addParameter("BrightnessCoef", "float64", QString::number(BrightnessCoef));
		addParameter("AdjustContrast", "int32", QString::number(0));
		addParameter("ContrastCoef", "float64", QString::number(0.10000));
		addParameter("AdjustClearness", "float64", QString::number(AdjustClearness));
		addParameter("AdjustClearnessType", "float64", QString::number(AdjustClearnessType));
		addParameter("MaskMethod", "int32", QString::number(MaskMethod));
		addParameter("Tasks", "int32", "4");

		// 3. 创建OutPutFiles节点 
		QDomElement outPutFiles = doc.createElement("OutPutFiles");
		root.appendChild(outPutFiles);

		QString tmp = projectdir + outputDir + "/";
		QString outputPath = tmp;

		// 添加OutPath节点 
		QDomElement outPath = doc.createElement("OutPath");
		outPath.appendChild(doc.createTextNode(outputPath));
		outPutFiles.appendChild(outPath);

		// 添加注释 
		QDomComment comment = doc.createComment(QString::fromLocal8Bit(" 适用于Mask匀光匀色方法"));

		QFileInfo fileinfo(filename);
		// 写入文件 
		QString xmlPath = QDir(projectdir).filePath(outputDir + "/") + fileinfo.completeBaseName() + ".xml";
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, xmlPath + QString::fromLocal8Bit("任务单创建失败"));

			//qWarning() << "无法打开文件进行写入:" << xmlPath;
			return xmlFiles;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);
		out << "\n" << comment.toComment();
		file.close();
		xmlFiles.push_back(xmlPath);
		index++;
	}

	return xmlFiles;
}

/**
 * @brief 创建自由网匀色任务单(FreeNetwork_DodgingCmd.xml)
 * @param inputImagePaths 输入图像路径列表
 * @param DodgingTemplateLibDir 模版库文件路径
 * @return QStringList 返回xml文件路径集合
 */
QStringList SystemConfig::createFreeNetworkDodgingFile(
	const QStringList& inputImagePaths,
	const QString& DodgingTemplateLibDir)
{
	// 从UI控件获取参数值 
	int ProcessingMethod = ui.imageEventColor_eventLightColorFunBox->currentIndex();
	int AdjustClearnessType = ui.imageEventColor_improveClarityBox->currentIndex();
	int MaskMethod = ui.comboBox_2->currentIndex();

	int BlockSize = ui.lineEdit_15->text().toInt();

	double DiffLimitCoef = ui.lineEdit_16->text().toDouble();
	double VarDiffLimitCoef = ui.lineEdit_17->text().toDouble();
	double DehazeCoef = ui.lineEdit_28->text().toDouble();
	double ExposeControlThrehold = ui.lineEdit_19->text().toDouble();
	double BrightnessCoef = ui.lineEdit_27->text().toDouble();
	double DarkControlThrehold = ui.lineEdit_53->text().toDouble();
	double HierarchicalFactor = ui.lineEdit_26->text().toDouble();

	QString StdFile = ui.lineEdit_20->text();
	QString CloudMaskFile = ui.lineEdit_25->text();

	bool UseCloudMask = ui.checkBox_26->isChecked();
	bool CreatePyd = ui.checkBox_21->isChecked();
	bool DiffLimit = ui.checkBox_22->isChecked();
	bool Stretch = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool Sharpen = ui.checkBox_30->isChecked();
	bool Dehaze = ui.checkBox_32->isChecked();
	bool UseDodgingTemplateLib = ui.imageEventColor_useModuleDBCheckBox->isChecked();
	bool SpecProcessWater = ui.checkBox_33->isChecked();
	bool ExposeControl = ui.checkBox_23->isChecked();
	bool AdjustBrightness = ui.checkBox_31->isChecked();
	bool DarkControl = ui.checkBox_73->isChecked();
	bool CreateOverViewFile = ui.checkBox_36->isChecked();
	bool IsSingleColor = ui.checkBox_20->isChecked();
	bool AdjustClearness = ui.imageEventColor_improveClarityCheckBox->isChecked();
	bool ExposeControl_UseAutoGamma = ui.checkBox_24->isChecked();

	QString outputDir;
	if (ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir = "Dodging/tmp";
	}
	else {
		outputDir = "Dodging";
	}

	int index = 0;
	QStringList xmlFiles;
	for (auto filename : inputImagePaths)
	{
		// 创建 QDomDocument 
		QDomDocument doc;

		// 添加 XML 声明 
		QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
		doc.appendChild(instruction);

		// 创建根节点 <DataUnit>
		QDomElement root = doc.createElement("DataUnit");
		doc.appendChild(root);

		// 1. 创建InputFiles节点 
		QDomElement inputFiles = doc.createElement("InputFiles");
		root.appendChild(inputFiles);

		// 添加ImageFile节点 
		QDomElement imageFile = doc.createElement("ImageFile");
		imageFile.appendChild(doc.createTextNode(filename));
		inputFiles.appendChild(imageFile);

		// 2. 创建Parameters节点 
		QDomElement parameters = doc.createElement("Parameters");
		root.appendChild(parameters);

		// 创建ColorParameters节点 
		QDomElement colorParameters = doc.createElement("ColorParameters");
		parameters.appendChild(colorParameters);

		// Lambda函数：添加参数节点 
		auto addParameter = [&](const QString& name, const QString& type, const QString& value) {
			QDomElement elem = doc.createElement(name);
			elem.setAttribute("type", type);
			elem.appendChild(doc.createTextNode(value));
			colorParameters.appendChild(elem);
		};

		// Lambda函数：添加无类型参数节点 
		auto addParameterNO = [&](const QString& name, const QString& value) {
			QDomElement elem = doc.createElement(name);
			elem.appendChild(doc.createTextNode(value));
			colorParameters.appendChild(elem);
		};

		// 添加各种颜色处理参数 
		addParameter("IsSingleColor", "int32", QString::number(IsSingleColor));
		addParameter("SingleColorCoef", "float64", "0.30000000");
		addParameter("ProcessingMethod", "int32", QString::number(ProcessingMethod));
		addParameter("BlockSize", "int32", QString::number(BlockSize));
		addParameterNO("StdFile", StdFile);
		addParameter("UseCloudMask", "int32", QString::number(UseCloudMask));
		addParameterNO("CloudMaskFile", CloudMaskFile);
		addParameter("CreatePyd", "int32", QString::number(CreatePyd));
		addParameter("DiffLimit", "int32", QString::number(DiffLimit));
		addParameter("DiffLimitCoef", "float64", QString::number(DiffLimitCoef));
		addParameter("VarDiffLimitCoef", "float64", QString::number(VarDiffLimitCoef));
		addParameter("Stretch", "int32", QString::number(Stretch));
		addParameter("Sharpen", "int32", QString::number(Sharpen));
		addParameter("CreateOverViewFile", "int32", QString::number(CreateOverViewFile));
		addParameter("Dehaze", "int32", QString::number(Dehaze));
		addParameter("DehazeCoef", "float64", QString::number(DehazeCoef));
		addParameter("UseDodgingTemplateLib", "int32", QString::number(UseDodgingTemplateLib));
		addParameterNO("DodgingTemplateLibDir", DodgingTemplateLibDir);
		addParameter("SpecProcessWater", "int32", QString::number(SpecProcessWater));
		addParameter("MinCoef", "float64", "0.25000000");
		addParameter("MaxCoef", "float64", "0.25000000");
		addParameter("ExposeControl", "int32", QString::number(ExposeControl));
		addParameter("ExposeControl_UseAutoGamma", "int32", QString::number(ExposeControl_UseAutoGamma));
		addParameter("ExposeControlThrehold", "float64", QString::number(ExposeControlThrehold));
		addParameter("DarkControl", "int32", QString::number(DarkControl));
		addParameter("DarkControlThrehold", "float64", QString::number(DarkControlThrehold));
		addParameter("HierarchicalFactor", "float64", QString::number(HierarchicalFactor));
		addParameter("AdjustBrightness", "int32", QString::number(AdjustBrightness));
		addParameter("BrightnessCoef", "float64", QString::number(BrightnessCoef));
		addParameter("AdjustContrast", "int32", QString::number(0));
		addParameter("ContrastCoef", "float64", QString::number(0.10000));
		addParameter("AdjustClearness", "float64", QString::number(AdjustClearness));
		addParameter("AdjustClearnessType", "float64", QString::number(AdjustClearnessType));
		addParameter("MaskMethod", "int32", QString::number(MaskMethod));
		addParameter("Tasks", "int32", "4");

		// 3. 创建OutPutFiles节点 
		QDomElement outPutFiles = doc.createElement("OutPutFiles");
		root.appendChild(outPutFiles);

		QString tmp = projectdir + outputDir + "/";
		QString outputPath = tmp;

		// 添加OutPath节点 
		QDomElement outPath = doc.createElement("OutPath");
		outPath.appendChild(doc.createTextNode(outputPath));
		outPutFiles.appendChild(outPath);

		// 添加注释 
		QDomComment comment = doc.createComment(QString::fromLocal8Bit(" 适用于自由网匀色方法、控制网匀色Plus"));

		// 写入文件 
		QString xmlPath = QDir(projectdir).filePath(outputDir + "/FreeNetwork_DodgingCmd" + QString::number(index) + ".xml");
		QFile file(xmlPath);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, xmlPath + QString::fromLocal8Bit("任务单创建失败"));

			//qWarning() << "无法打开文件进行写入:" << xmlPath;
			return xmlFiles;
		}

		QTextStream out(&file);
		out.setCodec("UTF-8");
		out << doc.toString(4);
		out << "\n" << comment.toComment();
		file.close();
		xmlFiles.push_back(xmlPath);
		index++;
	}

	return xmlFiles;
}

/**
 * @brief 创建匀色系数计算任务单(CalDodgingCoefCmd.xml)
 * @param imagePaths 输入图像路径列表
 * @param DodgingTemplateLibDir 模版库文件路径
 * @return QStringList 返回xml文件路径集合
 */
QString SystemConfig::createCalDodgingCoefFile(
	const QStringList& imagePaths)
{
	int index = 0;
	QString xmlFiles;

	QString outputDir;
	if (ui.output8bImage_checkBox->isChecked()) {
		// 修改输出目录为Dodging目录 
		outputDir = "Dodging/tmp";
	}
	else {
		outputDir = "Dodging";
	}

	// 创建 QDomDocument 
	QDomDocument doc;

	// 添加 XML 声明 
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 <XQCalDodgingCoefCmd>
	QDomElement root = doc.createElement("XQCalDodgingCoefCmd");
	doc.appendChild(root);

	// 1. 创建InputFiles节点 
	QDomElement inputFiles = doc.createElement("InputFiles");
	root.appendChild(inputFiles);

	for (auto imagePath : imagePaths)
	{
		// 添加ImageFile节点 
		QDomElement imageFile = doc.createElement("ImageFile");
		imageFile.appendChild(doc.createTextNode(imagePath));
		inputFiles.appendChild(imageFile);
	}
	// 2. 添加OutDir节点 
	QDomElement outDir = doc.createElement("OutDir");
	QString tmp = projectdir + outputDir + "/";
	QDir dir = QFileInfo(imagePaths.at(0)).absoluteDir();
	if (!dir.exists()) {
		return QString();
	}

	QString path = dir.absolutePath();
	//QString outputPath = tmp;
	outDir.appendChild(doc.createTextNode(path + "/"));
	root.appendChild(outDir);

	// 3. 添加CreateOverViewFile节点 
	QDomElement createOverViewFile = doc.createElement("CreateOverViewFile");
	createOverViewFile.setAttribute("type", "int32");
	createOverViewFile.appendChild(doc.createTextNode(QString::number(0)));
	root.appendChild(createOverViewFile);

	//// 4. 添加AddSaturation节点 
	//QDomElement addSaturationNode = doc.createElement("AddSaturation");
	//addSaturationNode.appendChild(doc.createTextNode(QString::number(1)));
	//root.appendChild(addSaturationNode);

	// 写入文件 
	xmlFiles = QDir(projectdir).filePath(outputDir + "/CalDodgingCoefCmd.xml");
	QFile file(xmlFiles);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, xmlFiles + QString::fromLocal8Bit("任务单创建失败"));

		//qWarning() << u8"无法打开文件进行写入:" << xmlFiles;
		return xmlFiles;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << doc.toString(4);
	file.close();
	//xmlFiles.push_back(xmlPath);
	index++;

	return xmlFiles;
}

/**
 * @brief 创建并行任务管理配置文件(PSTaskParallelManagerCmd.xml)
 * @param softwareDir 软件运行目录路径
 * @param jobType 任务类型
 * @param tasks 任务列表（每个元素是任务命令行）
 * @param outputPath 输出XML文件路径
 * @return bool 成功返回true，失败返回false
 */
bool SystemConfig::createTaskParallelManagerFile(
	const QString& softwareDir,
	const QString& jobType,
	const QStringList& tasks,
	const QString& outputPath)
{
	// 参数校验 
	if (softwareDir.isEmpty() || jobType.isEmpty() || tasks.isEmpty()) {
		qWarning() << u8"参数不合法：软件目录、任务类型或任务列表不能为空";
		return false;
	}

	// 创建 QDomDocument 
	QDomDocument doc;

	// 添加 XML 声明 
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 <XQTaskParallelManagerCmd>
	QDomElement root = doc.createElement("XQTaskParallelManagerCmd");
	doc.appendChild(root);

	// 添加注释：并行任务数 
	QDomComment parallelComment = doc.createComment(u8" 并行任务数，可以根据机器配置情况指定，最小值为1");
	root.appendChild(parallelComment);

	// 1. 添加并行任务数（默认设置为2）
	QDomElement parallelNumElem = doc.createElement("nParallelNum");
	parallelNumElem.appendChild(doc.createTextNode("2"));  // 默认并行数 
	root.appendChild(parallelNumElem);

	// 添加注释：任务监视文件目录 
	QDomComment monitorComment = doc.createComment(u8" 任务监视文件目录，相关任务完成后，会在该目录生成任务完成标记文件");
	root.appendChild(monitorComment);

	// 2. 添加任务监视目录（使用软件目录下的JobMonitor子目录）
	QDomElement monitorFolderElem = doc.createElement("JobMonitorFolderPath");
	monitorFolderElem.appendChild(doc.createTextNode(QDir(projectdir).filePath("Temp/JobMoni/")));
	root.appendChild(monitorFolderElem);

	// 添加注释：任务类型 
	QDomComment jobTypeComment = doc.createComment(u8" 任务类型，可以根据任务情况任意指定");
	root.appendChild(jobTypeComment);

	// 3. 添加任务类型 
	QDomElement jobTypeElem = doc.createElement("JobType");
	jobTypeElem.appendChild(doc.createTextNode(jobType));
	root.appendChild(jobTypeElem);

	// 4. 添加软件运行目录 
	QDomElement softwareDirElem = doc.createElement("SoftwareDir");
	QString tmpname = softwareDir;
	softwareDirElem.appendChild(doc.createTextNode(tmpname));
	root.appendChild(softwareDirElem);

	// 5. 添加总任务数 
	QDomElement numOfTasksElem = doc.createElement("NumOfTasks");
	numOfTasksElem.setAttribute("type", "int32");
	numOfTasksElem.appendChild(doc.createTextNode(QString::number(tasks.size())));
	root.appendChild(numOfTasksElem);

	// 6. 创建ExecuteScript节点 
	QDomElement executeScriptElem = doc.createElement("ExecuteScript");
	root.appendChild(executeScriptElem);

	// 添加所有任务 
	for (int i = 0; i < tasks.size(); ++i) {
		QString taskName = QString("Task_%1").arg(i + 1);
		QString taskId = QString("Task%1").arg(i + 1);

		QDomElement taskElem = doc.createElement(taskName);

		// 添加TaskID（自动生成）
		QDomElement taskIdElem = doc.createElement("TaskID");
		taskIdElem.appendChild(doc.createTextNode(taskId));
		taskElem.appendChild(taskIdElem);

		// 添加Task（从参数传入）
		QDomElement taskCmdElem = doc.createElement("Task");
		taskCmdElem.appendChild(doc.createTextNode(tasks.at(i)));
		taskElem.appendChild(taskCmdElem);

		executeScriptElem.appendChild(taskElem);
	}

	// 确保输出目录存在 
	QDir().mkpath(QFileInfo(outputPath).absolutePath());

	// 写入文件 
	QFile file(outputPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		qWarning() << "无法打开文件进行写入:" << outputPath;
		return false;
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << doc.toString(4);
	file.close();

	return true;
}

void SystemConfig::setupScrollAreaLabels()
{
	ui.scrollArea_10->viewport()->installEventFilter(this);

	QList<QLabel*> labels = ui.scrollArea_10->viewport()->findChildren<QLabel*>();
	foreach(QLabel *label, labels) {
		label->setAttribute(Qt::WA_TransparentForMouseEvents, false);
	}
}

bool SystemConfig::eventFilter(QObject *watched, QEvent *event)
{
	// 只处理目标scrollArea视口及其子控件 
	if (watched == ui.scrollArea_10->viewport() ||
		watched->parent() == ui.scrollArea_10->viewport())
	{
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
			if (mouseEvent && mouseEvent->button() == Qt::RightButton) {
				// 获取实际点击的Label（考虑滚动偏移）
				QPoint viewportPos = ui.scrollArea_10->viewport()->mapFromParent(mouseEvent->pos());
				if (QLabel *label = qobject_cast<QLabel*>(ui.scrollArea_10->viewport()->childAt(viewportPos))) {
					handleLabelRightClick(label, viewportPos);
					return true; // 事件已处理
				}
			}
				if (mouseEvent && mouseEvent->button() == Qt::LeftButton) {
				// 获取实际点击的Label（考虑滚动偏移）
				QPoint viewportPos = ui.scrollArea_10->viewport()->mapFromParent(mouseEvent->pos());
				if (QWidget *widget = qobject_cast<QWidget*>(ui.scrollArea_10->viewport()->childAt(viewportPos))) {
					handlewidgetRightClick(widget, viewportPos);
					return true; // 事件已处理
				}
			}
		}
	}

	QPushButton* button = qobject_cast<QPushButton*>(watched);
	if (button && m_visibleButtons.contains(button)) {
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() == Qt::LeftButton) {
				// 记录拖拽开始位置 
				m_dragStartPosition = mouseEvent->pos();
				m_draggedButton = button;
				return true;
			}
			else if (mouseEvent->button() == Qt::RightButton) {
				// 记录右键点击的按钮 
				m_contextMenuButton = button;

				// 显示右键菜单 
				QMenu contextMenu;
				QAction* deleteAction = new QAction(u8"删除处理节点", &contextMenu);
				connect(deleteAction, &QAction::triggered, this, &SystemConfig::deleteProcessingNode);
				contextMenu.addAction(deleteAction);
				contextMenu.exec(button->mapToGlobal(mouseEvent->pos()));
				return true;
			}
		}
		else if (event->type() == QEvent::MouseMove && m_draggedButton == button) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->buttons() & Qt::LeftButton) {
				// 移动按钮 
				button->move(button->pos() + (mouseEvent->pos() - m_dragStartPosition));
				updateVisualConnections();
				return true;
			}
		}
		else if (event->type() == QEvent::MouseButtonRelease && m_draggedButton == button) {
			m_draggedButton = nullptr;
			return true;
		}
	}

	// 为新创建的滚动条动态添加中文菜单支持 
	if (event->type() == QEvent::ChildAdded) {
		QChildEvent* childEvent = static_cast<QChildEvent*>(event);
		if (QScrollBar* scrollBar = qobject_cast<QScrollBar*>(childEvent->child())) {
			scrollBar->installEventFilter(this);
			scrollBar->setContextMenuPolicy(Qt::CustomContextMenu);
			connect(scrollBar, &QScrollBar::customContextMenuRequested,
				this, [this, scrollBar](const QPoint &pos) {
				m_imagePS->showChineseScrollBarContextMenu(scrollBar, pos);
			});
		}
	}

	return QDialog::eventFilter(watched, event);
}

//自动化流程一设置
void SystemConfig::handleLabelRightClick(QLabel *label, const QPoint &viewportPos)
{
	QMenu menu;
	if (label == ui.label_124)
	{
		//// 添加菜单项 
		//menu.addAction(u8" 参与运算", [=]() {
		//	label->setStyleSheet("background-color: rgb(0, 170, 0); ");
		//	label->setEnabled(true);
		//});

		//menu.addAction(u8" 不参与运算", [=]() {
		//	label->setStyleSheet(
		//		"QLabel:disabled {"
		//		"   background-color: gray;"
		//		"   color: darkGray;"
		//		"}"
		//	);
		//	label->setEnabled(false);
		//});
		menu.addAction(u8" 使用常规镶嵌", [=]() {
			//label->setStyleSheet("background-color: rgb(0, 170, 0); ");
			//label->setEnabled(true);
			//label->setText(QString::fromLocal8Bit("影像镶嵌"));

			if (!ui.widget_10->isEnabled()) {
				QPixmap icon_10(QString::fromLocal8Bit(":/resource/menu/流程/影像镶嵌_灰.png"));
				ui.label_124->setPixmap(icon_10);
				ui.label_124->setScaledContents(true);
			}
			else {
				QPixmap icon_10(QString::fromLocal8Bit(":/resource/menu/流程/影像镶嵌.png"));
				ui.label_124->setPixmap(icon_10);
				ui.label_124->setScaledContents(true);
			}

			m_flagMosaic = false;
		});
		menu.addAction(u8" 使用智能镶嵌", [=]() {
			//label->setStyleSheet("background-color: rgb(0, 170, 0); ");
			//label->setEnabled(true);
			//label->setText(QString::fromLocal8Bit("智能镶嵌"));
			if (!ui.widget_10->isEnabled()) {
				QPixmap icon_10(QString::fromLocal8Bit(":/resource/menu/流程/智能镶嵌_灰.png"));
				ui.label_124->setPixmap(icon_10);
				ui.label_124->setScaledContents(true);
			}
			else {
				QPixmap icon_10(QString::fromLocal8Bit(":/resource/menu/流程/智能镶嵌.png"));
				ui.label_124->setPixmap(icon_10);
				ui.label_124->setScaledContents(true);
			}
			m_flagMosaic = true;
		});
	}
	else{
		//// 添加菜单项 
		//menu.addAction(u8" 参与运算", [=]() {
		//	label->setStyleSheet("background-color: rgb(0, 170, 0); ");
		//	label->setEnabled(true);
		//});

		//menu.addAction(u8" 不参与运算", [=]() {
		//	label->setStyleSheet(
		//		"QLabel:disabled {"
		//		"   background-color: gray;"
		//		"   color: darkGray;"
		//		"}"
		//	);
		//	label->setEnabled(false);
		//});
		//QWidget *parent = label->parentWidget();
		//if (label->isEnabled() && parent != ui.automationProcess1_Page && parent != ui.scrollAreaWidgetContents) {
		//	parent->setEnabled(false);
		//	//label->setEnabled(false);
		//}
		//else {
		//	parent->setEnabled(true);

		//	//label->setEnabled(true);
		//}
	}

	// 显示菜单（转换为全局坐标）
	menu.exec(ui.scrollArea_10->viewport()->mapToGlobal(viewportPos));
}

//自动化流程一设置
void SystemConfig::handlewidgetRightClick(QWidget *widget, const QPoint &viewportPos)
{
	if (qobject_cast<QLabel*>(widget)) {
		widget = widget->parentWidget();
	}

	if (widget && widget->isEnabled() &&
		widget != ui.automationProcess1_Page  &&
		widget != ui.scrollAreaWidgetContents) {
		widget->setEnabled(false);

		if (widget->objectName() == "widget1") {
			QPixmap icon_1(QString::fromLocal8Bit(":/resource/menu/流程/金字塔创建_灰.png"));
			ui.label_106->setPixmap(icon_1);
			ui.label_106->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_2") {
			QPixmap icon_2(QString::fromLocal8Bit(":/resource/menu/流程/连接点匹配_灰.png"));
			ui.label_108->setPixmap(icon_2);
			ui.label_108->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_3") {
			QPixmap icon_3(QString::fromLocal8Bit(":/resource/menu/流程/控制点匹配_灰.png"));
			ui.label_110->setPixmap(icon_3);
			ui.label_110->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_4") {
			QPixmap icon_6(QString::fromLocal8Bit(":/resource/menu/流程/正射纠正_灰.png"));
			ui.label_112->setPixmap(icon_6);
			ui.label_112->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_5") {
			QPixmap icon_4(QString::fromLocal8Bit(":/resource/menu/流程/自由网平差_灰.png"));
			ui.label_114->setPixmap(icon_4);
			ui.label_114->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_6") {
			QPixmap icon_5(QString::fromLocal8Bit(":/resource/menu/流程/控制网平差_灰.png"));
			ui.label_116->setPixmap(icon_5);
			ui.label_116->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_7") {
			QPixmap icon_7(QString::fromLocal8Bit(":/resource/menu/流程/影像融合_灰.png"));
			ui.label_118->setPixmap(icon_7);
			ui.label_118->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_8") {
			QPixmap icon_9(QString::fromLocal8Bit(":/resource/menu/流程/影像匀色_灰.png"));
			ui.label_120->setPixmap(icon_9);
			ui.label_120->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_9") {
			QPixmap icon_8(QString::fromLocal8Bit(":/resource/menu/流程/真彩色转换_灰.png"));
			ui.label_122->setPixmap(icon_8);
			ui.label_122->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_10") {
			QPixmap icon_10(QString::fromLocal8Bit(":/resource/menu/流程/影像镶嵌_灰.png"));
			ui.label_124->setPixmap(icon_10);
			ui.label_124->setScaledContents(true);
		}
	}
	else if (widget) {
		widget->setEnabled(true);

		if (widget->objectName() == "widget1") {
			QPixmap icon_1(QString::fromLocal8Bit(":/resource/menu/流程/金字塔创建.png"));
			ui.label_106->setPixmap(icon_1);
			ui.label_106->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_2") {
			QPixmap icon_2(QString::fromLocal8Bit(":/resource/menu/流程/连接点匹配.png"));
			ui.label_108->setPixmap(icon_2);
			ui.label_108->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_3") {
			QPixmap icon_3(QString::fromLocal8Bit(":/resource/menu/流程/控制点匹配.png"));
			ui.label_110->setPixmap(icon_3);
			ui.label_110->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_4") {
			QPixmap icon_6(QString::fromLocal8Bit(":/resource/menu/流程/正射纠正.png"));
			ui.label_112->setPixmap(icon_6);
			ui.label_112->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_5") {
			QPixmap icon_4(QString::fromLocal8Bit(":/resource/menu/流程/自由网平差.png"));
			ui.label_114->setPixmap(icon_4);
			ui.label_114->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_6") {
			QPixmap icon_5(QString::fromLocal8Bit(":/resource/menu/流程/控制网平差.png"));
			ui.label_116->setPixmap(icon_5);
			ui.label_116->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_7") {
			QPixmap icon_7(QString::fromLocal8Bit(":/resource/menu/流程/影像融合.png"));
			ui.label_118->setPixmap(icon_7);
			ui.label_118->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_8") {
			QPixmap icon_9(QString::fromLocal8Bit(":/resource/menu/流程/影像匀色.png"));
			ui.label_120->setPixmap(icon_9);
			ui.label_120->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_9") {
			QPixmap icon_8(QString::fromLocal8Bit(":/resource/menu/流程/真彩色转换.png"));
			ui.label_122->setPixmap(icon_8);
			ui.label_122->setScaledContents(true);
		}
		else if (widget->objectName() == "widget_10") {
			QPixmap icon_10(QString::fromLocal8Bit(":/resource/menu/流程/影像镶嵌.png"));
			ui.label_124->setPixmap(icon_10);
			ui.label_124->setScaledContents(true);
		}
	}
}

/////////////////////////////////

// 设置自定义流程页面
void SystemConfig::setupCustomWorkflowPage()
{
	// 获取所有业务按钮（假设已在Qt Designer中命名为button_1到button_12）
	QList<QPushButton*> businessButtons = {
		ui.pushButton_7,  ui.pushButton_8,  ui.pushButton_9,  ui.pushButton_10,  ui.pushButton_11,
		ui.pushButton_12,  ui.pushButton_13,  ui.pushButton_14,  ui.pushButton_15
	};

	// 为业务按钮连接点击信号 
	for (QPushButton* button : businessButtons) {
		connect(button, &QPushButton::clicked, this, &SystemConfig::handleBusinessButtonClicked);
	}

	// 为功能按钮连接信号
	connect(ui.startButton, &QPushButton::clicked, this, &SystemConfig::handleStartProcessClicked);
	connect(ui.pathButton, &QPushButton::clicked, this, &SystemConfig::handlePathSettingClicked);
	connect(ui.stopButton, &QPushButton::clicked, this, &SystemConfig::handleStopProcessClicked);

	// 创建路径设置对话框 
	createPathSettingsDialog();

	// 设置上方显示区域为可拖放区域
	ui.workflowDisplayArea->setAcceptDrops(true);
	ui.workflowDisplayArea->setMouseTracking(true);
	ui.workflowDisplayArea->installEventFilter(this);
}

// 业务按钮点击处理
void SystemConfig::handleBusinessButtonClicked()
{
	QPushButton* clickedButton = qobject_cast<QPushButton*>(sender());
	if (!clickedButton) return;

	// 如果按钮未选择，则添加到选择列表
	if (!m_selectedButtons.contains(clickedButton)) {
		m_selectedButtons.append(clickedButton);
		clickedButton->setEnabled(false); // 置灰

		// 在上方显示区域创建对应的按钮 
		QPushButton* displayButton = new QPushButton(clickedButton->text(), ui.workflowDisplayArea);
		displayButton->setProperty("sourceButton", QVariant::fromValue(clickedButton));
		displayButton->setMinimumSize(100, 30);
		displayButton->move(20 + m_visibleButtons.size() * 120, 20);
		displayButton->show();
		displayButton->installEventFilter(this); // 安装事件过滤器用于拖拽 

		m_visibleButtons.append(displayButton);

		// 更新连接线
		updateVisualConnections();
	}
}

// 更新按钮间的连接线 
void SystemConfig::updateVisualConnections()
{
	// 清除现有连接线 
	for (QFrame* line : m_connectionLines) {
		delete line;
	}
	m_connectionLines.clear();

	// 为相邻按钮创建连接线 
	for (int i = 1; i < m_visibleButtons.size(); i++) {
		QPushButton* prevButton = m_visibleButtons.at(i - 1);
		QPushButton* currButton = m_visibleButtons.at(i);

		QFrame* line = new QFrame(ui.workflowDisplayArea);
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		line->setLineWidth(2);
		line->setMidLineWidth(2);
		line->setStyleSheet("background-color: #555;");

		int startX = prevButton->geometry().right();
		int endX = currButton->geometry().left();
		int y = prevButton->geometry().center().y();

		line->setGeometry(startX, y, endX - startX, 2);
		line->show();

		m_connectionLines.append(line);
	}
}

// 启动处理流程 
void SystemConfig::handleStartProcessClicked()
{
	if (m_selectedButtons.isEmpty()) {
		QMessageBox::warning(this, "警告", "请先选择至少一个处理流程");
		return;
	}

	// 执行选中的业务流程 
	QStringList processNames;
	for (QPushButton* button : m_selectedButtons) {
		processNames << button->text();
	}

	logEdit->append("开始执行自定义流程: " + processNames.join("  -> "));
	// 这里可以添加实际的流程执行代码 
}

// 路径设置按钮点击 
void SystemConfig::handlePathSettingClicked()
{
	if (m_selectedButtons.isEmpty()) {
		QMessageBox::warning(this, "警告", "请先选择至少一个处理流程");
		return;
	}

	m_pathSettingsDialog->show();
}

// 创建路径设置对话框 
void SystemConfig::createPathSettingsDialog()
{
	m_pathSettingsDialog = new QDialog(this);
	m_pathSettingsDialog->setWindowTitle("数据自动加载路径设置");

	QVBoxLayout* mainLayout = new QVBoxLayout(m_pathSettingsDialog);
	QScrollArea* scrollArea = new QScrollArea(m_pathSettingsDialog);
	QWidget* container = new QWidget(scrollArea);
	QVBoxLayout* containerLayout = new QVBoxLayout(container);

	// 动态添加路径设置控件 
	for (int i = 0; i < 9; i++) { // 对应9个业务 
		QGroupBox* groupBox = new QGroupBox("业务 " + QString::number(i + 1), container);
		QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);

		QLabel* nameLabel = new QLabel("业务名称:", groupBox);
		QLineEdit* nameEdit = new QLineEdit(groupBox);
		nameEdit->setReadOnly(true);

		QCheckBox* autoLoadCheck = new QCheckBox("自动加载数据", groupBox);
		QLabel* pathLabel = new QLabel("数据路径:", groupBox);
		QLineEdit* pathEdit = new QLineEdit(groupBox);
		QPushButton* browseButton = new QPushButton("浏览...", groupBox);

		connect(browseButton, &QPushButton::clicked, [pathEdit]() {
			QString dir = QFileDialog::getExistingDirectory(nullptr, "选择数据路径");
			if (!dir.isEmpty()) {
				pathEdit->setText(dir);
			}
		});

		connect(autoLoadCheck, &QCheckBox::toggled, [pathLabel, pathEdit, browseButton](bool checked) {
			pathLabel->setEnabled(checked);
			pathEdit->setEnabled(checked);
			browseButton->setEnabled(checked);
		});

		groupLayout->addWidget(nameLabel);
		groupLayout->addWidget(nameEdit);
		groupLayout->addWidget(autoLoadCheck);
		groupLayout->addWidget(pathLabel);
		groupLayout->addWidget(pathEdit);
		groupLayout->addWidget(browseButton);
		groupBox->setLayout(groupLayout);
		groupBox->setProperty("businessIndex", i);

		containerLayout->addWidget(groupBox);
	}

	// 添加确定和取消按钮 
	QHBoxLayout* buttonLayout = new QHBoxLayout();
	QPushButton* okButton = new QPushButton("确定", m_pathSettingsDialog);
	QPushButton* cancelButton = new QPushButton("取消", m_pathSettingsDialog);

	connect(okButton, &QPushButton::clicked, this, &SystemConfig::savePathSettings);
	connect(cancelButton, &QPushButton::clicked, m_pathSettingsDialog, &QDialog::reject);

	buttonLayout->addStretch();
	buttonLayout->addWidget(okButton);
	buttonLayout->addWidget(cancelButton);

	container->setLayout(containerLayout);
	scrollArea->setWidget(container);
	scrollArea->setWidgetResizable(true);

	mainLayout->addWidget(scrollArea);
	mainLayout->addLayout(buttonLayout);

	m_pathSettingsDialog->setLayout(mainLayout);
	m_pathSettingsDialog->resize(500, 600);
}

// 保存路径设置
void SystemConfig::savePathSettings()
{
	// 这里实现保存路径设置的逻辑 
	// 可以遍历对话框中的控件，获取用户设置并保存

	m_pathSettingsDialog->accept();
	logEdit->append("数据自动加载路径设置已保存");
}

// 终止处理流程
void SystemConfig::handleStopProcessClicked()
{
	// 实现终止处理流程的逻辑 
	logEdit->append("自定义流程已终止");

	// 重置界面状态 
	for (QPushButton* button : m_selectedButtons) {
		button->setEnabled(true);
	}
	m_selectedButtons.clear();

	for (QPushButton* button : m_visibleButtons) {
		delete button;
	}
	m_visibleButtons.clear();

	for (QFrame* line : m_connectionLines) {
		delete line;
	}
	m_connectionLines.clear();
}

// 显示右键菜单
void SystemConfig::showContextMenu(const QPoint& pos)
{
	QWidget* widget = qobject_cast<QWidget*>(sender());
	if (!widget) return;

	QPushButton* button = qobject_cast<QPushButton*>(widget);
	if (button && m_visibleButtons.contains(button)) {
		m_contextMenuButton = button;

		QMenu contextMenu;
		QAction* deleteAction = new QAction("删除处理节点", &contextMenu);
		connect(deleteAction, &QAction::triggered, this, &SystemConfig::deleteProcessingNode);
		contextMenu.addAction(deleteAction);

		contextMenu.exec(widget->mapToGlobal(pos));
	}
}

// 删除处理节点
void SystemConfig::deleteProcessingNode()
{
	if (!m_contextMenuButton) return;

	// 找到对应的源按钮并恢复
	QPushButton* sourceButton = m_contextMenuButton->property("sourceButton").value<QPushButton*>();
	if (sourceButton) {
		sourceButton->setEnabled(true);
		m_selectedButtons.removeOne(sourceButton);
	}

	// 删除显示按钮
	m_visibleButtons.removeOne(m_contextMenuButton);
	delete m_contextMenuButton;

	// 更新连接线 
	updateVisualConnections();
}
/////////////////////////////////

QStringList SystemConfig::filterSatelliteImagesByColumn(const QStringList& fileList, int col) const
{
	if (col < 0 || col >= m_imagePS->ui.sateImageDataList_TableW->columnCount()) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("无效的卫星影像列表 列序号, 将返回原始列表"));

		//qWarning() << "Invalid column index:" << col;
		return fileList; // 返回原始列表
	}

	QStringList result;

	for (const QString& filename : fileList) {
		QFileInfo fileInfo(filename);
		QString baseName = fileInfo.completeBaseName();
		bool found = false;
		bool keepFile = true;

		for (int row = 0; row < m_imagePS->ui.sateImageDataList_TableW->rowCount(); ++row) {
			QTableWidgetItem* nameItem = m_imagePS->ui.sateImageDataList_TableW->item(row, 1);
			if (nameItem && nameItem->text() == baseName) {
				found = true;
				QTableWidgetItem* targetItem = m_imagePS->ui.sateImageDataList_TableW->item(row, col);
				if (!targetItem || targetItem->text().compare(QString::fromLocal8Bit("否"), Qt::CaseInsensitive) == 0) {
					keepFile = false;
				}
				break;
			}
		}

		if (!found) {
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("文件在列表中未找到!!!!"));

			//qDebug() << "File not found in table:" << baseName;
			keepFile = false;
		}

		if (keepFile) {
			result.append(filename);
		}
	}

	return result;
}

void SystemConfig::terminateAllProcesses()
{
	// 断开所有保存的连接
	for (const QMetaObject::Connection& conn : m_systemProcessesConn) {
		disconnect(conn);
	}
	m_systemProcessesConn.clear();  // 清空连接列表

	for (QProcess* process : m_systemProcesses) {
		if (process && process->state() != QProcess::NotRunning) {
			process->kill();
			process->waitForFinished(1000);
		}
	}
	m_progressBarInitialized = false;
	m_progressBarInitializedIR = false;
	m_systemProcesses.clear();

	// 处理完成后断开所有连接
	for (auto &conn : connections) {
		QObject::disconnect(conn);
	}
	connections.clear();
	m_imagePS->setWorkProcessflag(false);

	if (!ModelMutex.tryLock()) {
		// 如果tryLock失败，说明锁已经被占用 
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("Mutex was locked, now unlocking"));

		//qDebug() << "Mutex was locked, now unlocking";
		ModelMutex.unlock();   // 解锁 
	}
	else {
		// 如果tryLock成功，说明锁未被占用，需要立即释放 
		//qDebug() << "Mutex was not locked";
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("Mutex was not locked"));
		ModelMutex.unlock();
	}
	if (m_systemProcessesFlag)
	{
		if (!intersectCmdMutex.tryLock()) {
			intersectCmdMutex.unlock();   // 锁未被持有时主动释放
		}
		else {
			intersectCmdMutex.unlock();   // 确保锁被释放
		}
	}

	//ModelMutex.unlock();
	m_taskQueue.clear();
}

QString SystemConfig::getGlobalShpFileValue() const {
	return ui.lineEdit_37->text();
}

bool SystemConfig::getGlobalDEM() const {
	return ui.checkBox_72->isChecked();
}

bool SystemConfig::getprogressBarInitialized() {
	return this->m_progressBarInitialized;
}

void SystemConfig::writeGlobalShpFileValue(QString filePath) const {
	ui.lineEdit_37->setText(filePath);
}

QStringList SystemConfig::getThumbnailGenerationFilePath(QString flag) const {
	QStringList filePath;
	if(flag == "OverView")
		return ThumbnailGenerationPaths;
	else if(flag == "CloudDect")
		return CloudDetectionOutPaths;
	return filePath;
}

/**
 * @brief 删除中间数据
 */
//bool SystemConfig::deleteFilesInAllSubfolders(const QString &folderPath)
//{
//	QDir dir(folderPath);
//
//	if (!dir.exists()) {
//		qWarning() << u8"文件夹不存在:" << folderPath;
//		return false;
//	}
//
//	// 获取所有流程Label的状态 
//	QVector<bool> labelStatus = {
//		!ui.label_106->isEnabled(),  // 金字塔创建 
//		!ui.label_108->isEnabled(),  // 连接点匹配 
//		!ui.label_110->isEnabled(),  // 控制点匹配 
//		!ui.label_114->isEnabled(),  // 自由网平差 
//		!ui.label_116->isEnabled(),  // 控制网平差 
//		!ui.label_112->isEnabled(),  // 正射纠正 
//		!ui.label_118->isEnabled(),  // 影像融合 
//		!ui.label_122->isEnabled(),  // 真彩色转换 
//		!ui.label_120->isEnabled(),  // 影像匀色 
//		!ui.label_124->isEnabled()   // 影像镶嵌 
//	};
//
//	// 找到最后一个不可用(disabled)的Label索引 
//	int lastDisabledIndex = -1;
//	for (int i = labelStatus.size() - 1; i >= 0; --i) {
//		if (labelStatus[i]) {
//			lastDisabledIndex = i;
//			break;
//		}
//	}
//
//	// 对应的子目录名称列表（按照Label顺序）
//	QStringList subDirNames = {
//		"imageHandleBusinessConfig",
//		"SatTiePointMatch",
//		"CtlPointMatch",
//		"SatBA",
//		"SatBA",
//		"Ortho",
//		"Fusion",
//		"OutByte",
//		"Dodging",
//		"Mosaic"  // 注意：这里只列出与Label直接对应的目录 
//	};
//
//	bool success = true;
//
//	// 先处理当前目录的子目录（递归）
//	QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
//	foreach(const QString &subDirName, subDirs) {
//		QString subDirPath = dir.filePath(subDirName);
//
//		// 检查是否是需要保留的目录 
//		bool isDirToKeep = false;
//
//		// 1. 检查是否是最后一个不可用Label对应的目录 
//		if (lastDisabledIndex >= 0 && subDirName == subDirNames[lastDisabledIndex]) {
//			isDirToKeep = true;
//		}
//
//		// 2. 如果是镶嵌阶段（最后一个Label），额外保留SmartMosaic目录
//		if (lastDisabledIndex == 9 && subDirName == "SmartMosaic") { // 9是影像镶嵌的索引
//			isDirToKeep = true;
//		}
//
//		if (!isDirToKeep) {
//			if (!deleteFilesInAllSubfolders(subDirPath)) {
//				success = false;
//			}
//		}
//		else {
//			qDebug() << u8"保留目录中的文件:" << subDirPath;
//		}
//	}
//
//	// 如果不是根目录，删除当前目录中的文件 
//	if (dir != QDir(folderPath)) {
//		QStringList files = dir.entryList(QDir::Files);
//		foreach(const QString &fileName, files) {
//			QString filePath = dir.filePath(fileName);
//			QFile file(filePath);
//
//			if (!file.remove()) {
//				qWarning() << u8"删除文件失败:" << filePath
//					<< u8"错误:" << file.errorString();
//				success = false;
//			}
//			else {
//				qDebug() << u8"已删除文件:" << filePath;
//			}
//		}
//	}
//
//	return success;
//}

bool SystemConfig::deleteFilesInAllSubfolders(const QString &folderPath)
{
	QDir dir(folderPath);

	if (!dir.exists()) {
		qWarning() << u8"文件夹不存在:" << folderPath;
		return false;
	}

	// 获取所有流程Label的状态 
	QVector<bool> labelStatus = {
		ui.label_106->isEnabled(),  // 金字塔创建 
		ui.label_108->isEnabled(),  // 连接点匹配 
		ui.label_110->isEnabled(),  // 控制点匹配 
		ui.label_114->isEnabled(),  // 自由网平差 
		ui.label_116->isEnabled(),  // 控制网平差 
		ui.label_112->isEnabled(),  // 正射纠正 
		ui.label_118->isEnabled(),  // 影像融合 
		ui.label_122->isEnabled(),  // 真彩色转换 
		ui.label_120->isEnabled(),  // 影像匀色 
		ui.label_124->isEnabled()   // 影像镶嵌 
	};

	// 找到最后一个不可用(disabled)的Label索引 
	int lastDisabledIndex = -1;
	for (int i = labelStatus.size() - 1; i >= 0; --i) {
		if (labelStatus[i]) {
			lastDisabledIndex = i;
			break;
		}
	}

	// 对应的子目录名称列表（按照Label顺序）
	QStringList subDirNames = {
		"imageHandleBusinessConfig",
		"SatTiePointMatch",
		"CtlPointMatch",
		"SatBA",
		"SatBA",
		"Ortho",
		"Fusion",
		"OutByte",
		"Dodging",
		"Mosaic"
	};

	bool success = true;

	// 不处理第一层目录的文件（保持不变）

	// 处理子目录
	QStringList subDirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	foreach(const QString &subDirName, subDirs) {
		QString subDirPath = dir.filePath(subDirName);

		// 检查是否是需要保留的目录 
		bool isDirToKeep = false;

		// 1. 检查是否是最后一个不可用Label对应的目录 
		if (lastDisabledIndex >= 0 && subDirName == subDirNames[lastDisabledIndex]) {
			isDirToKeep = true;
		}

		// 2. 如果是镶嵌阶段（最后一个Label），额外保留SmartMosaic目录
		if (lastDisabledIndex == 9 && subDirName == "SmartMosaic") {
			isDirToKeep = true;
		}

		if (isDirToKeep) {
			qDebug() << u8"保留目录中的文件:" << subDirPath;
			continue; // 跳过保留目录 
		}

		// 对于非保留目录，递归删除其中的所有文件 
		QDir subDir(subDirPath);
		QStringList files = subDir.entryList(QDir::Files | QDir::NoDotAndDotDot);
		foreach(const QString &fileName, files) {
			QString filePath = subDir.filePath(fileName);
			QFile file(filePath);

			if (!file.remove()) {
				qWarning() << u8"删除文件失败:" << filePath
					<< u8"错误:" << file.errorString();
				success = false;
			}
			else {
				qDebug() << u8"已删除文件:" << filePath;
			}
		}

		// 递归处理子目录的子目录
		QStringList childDirs = subDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		foreach(const QString &childDirName, childDirs) {
			if (!deleteFilesInAllSubfolders(subDir.filePath(childDirName))) {
				success = false;
			}
		}
	}

	return success;
}


void SystemConfig::onExportTrueColorSettings()
{
	// 获取保存文件路径
	QString filePath = QFileDialog::getSaveFileName(this,
		tr(u8"导出真彩色模版参数"),
		QDir::currentPath(),
		tr(u8"真彩色模版参数文件 (*.PStrueColor)"));

	if (filePath.isEmpty()) {
		return;
	}

	// 确保文件扩展名正确
	if (!filePath.endsWith(".PStrueColor", Qt::CaseInsensitive)) {
		filePath += ".PStrueColor";
	}

	// 创建XML文档
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根元素 
	QDomElement root = doc.createElement("trueColor");
	doc.appendChild(root);

	// 添加各参数节点 
	addXmlElement(doc, root, "Start", QString::number(ui.lineEdit_32->text().toDouble()), "float64");
	addXmlElement(doc, root, "End", QString::number(ui.lineEdit_33->text().toDouble()), "float64");

	addXmlElement(doc, root, "USMSharpen", ui.checkBox_48->isChecked() ? "true" : "false");
	addXmlElement(doc, root, "Number", QString::number(ui.lineEdit->text().toDouble()), "float64");
	addXmlElement(doc, root, "Radius", QString::number(ui.lineEdit_2->text().toDouble()), "float64");

	addXmlElement(doc, root, "VegEnhance", ui.checkBox_40->isChecked() ? "true" : "false");
	addXmlElement(doc, root, "R_Veg", QString::number(ui.lineEdit_35->text().toDouble()), "float64");
	addXmlElement(doc, root, "G_Veg", QString::number(ui.lineEdit_34->text().toDouble()), "float64");
	addXmlElement(doc, root, "B_Veg", QString::number(ui.lineEdit_36->text().toDouble()), "float64");

	addXmlElement(doc, root, "WaterEnhance", ui.checkBox_42->isChecked() ? "true" : "false");
	addXmlElement(doc, root, "R_Water", QString::number(ui.lineEdit_39->text().toDouble()), "float64");
	addXmlElement(doc, root, "G_Water", QString::number(ui.lineEdit_38->text().toDouble()), "float64");
	addXmlElement(doc, root, "B_Water", QString::number(ui.lineEdit_40->text().toDouble()), "float64");

	addXmlElement(doc, root, "AdaptiveStretch", ui.checkBox_48->isChecked() ? "true" : "false");
	addXmlElement(doc, root, "AdaptiveStretchMethod", QString::number(ui.trueColorConver_colorAdjustWayBox->currentIndex()), "int32");
	addXmlElement(doc, root, "FineStatistical", ui.checkBox_2->isChecked() ? "true" : "false");

	// 写入文件 
	QFile file(filePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr(u8"错误"), tr(u8"无法创建文件: %1").arg(filePath));
		return;
	}

	QTextStream out(&file);
	doc.save(out, 4); // 缩进4个空格 
	file.close();

	QMessageBox::information(this, tr(u8"成功"), tr(u8"真彩色模板参数已导出到: %1").arg(filePath));
}

// 辅助函数：添加XML元素 
void SystemConfig::addXmlElement(QDomDocument& doc, QDomElement& parent,
	const QString& name, const QString& value,
	const QString& type)
{
	QDomElement element = doc.createElement(name);
	if (!type.isEmpty()) {
		element.setAttribute("type", type);
	}
	QDomText text = doc.createTextNode(value);
	element.appendChild(text);
	parent.appendChild(element);
}

void SystemConfig::onImportTrueColorSettings()
{
	// 获取打开文件路径
	QString filePath = QFileDialog::getOpenFileName(this,
		tr(u8"导入真彩色模版参数"),
		QDir::currentPath(),
		tr(u8"真彩色模版参数文件 (*.PStrueColor)"));

	if (filePath.isEmpty()) {
		return;
	}

	// 读取XML文件 
	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::warning(this, tr(u8"错误"), tr(u8"无法打开文件: %1").arg(filePath));
		return;
	}

	QDomDocument doc;
	if (!doc.setContent(&file)) {
		file.close();
		QMessageBox::warning(this, tr(u8"错误"), tr(u8"文件格式错误: %1").arg(filePath));
		return;
	}
	file.close();

	// 解析XML内容 
	QDomElement root = doc.documentElement();
	if (root.tagName() != "trueColor") {
		QMessageBox::warning(this, tr(u8"错误"), tr(u8"无效的真彩色设置文件"));
		return;
	}

	// 读取各参数并设置到界面控件
	ui.lineEdit_32->setText(getXmlValue(root, "Start"));
	ui.lineEdit_33->setText(getXmlValue(root, "End"));

	ui.checkBox_48->setChecked(getXmlValue(root, "USMSharpen") == "true");
	ui.lineEdit->setText(getXmlValue(root, "Number"));
	ui.lineEdit_2->setText(getXmlValue(root, "Radius"));

	ui.checkBox_40->setChecked(getXmlValue(root, "VegEnhance") == "true");
	ui.lineEdit_35->setText(getXmlValue(root, "R_Veg"));
	ui.lineEdit_34->setText(getXmlValue(root, "G_Veg"));
	ui.lineEdit_36->setText(getXmlValue(root, "B_Veg"));

	ui.checkBox_42->setChecked(getXmlValue(root, "WaterEnhance") == "true");
	ui.lineEdit_39->setText(getXmlValue(root, "R_Water"));
	ui.lineEdit_38->setText(getXmlValue(root, "G_Water"));
	ui.lineEdit_40->setText(getXmlValue(root, "B_Water"));

	ui.checkBox_48->setChecked(getXmlValue(root, "AdaptiveStretch") == "true");
	ui.trueColorConver_colorAdjustWayBox->setCurrentIndex(getXmlValue(root, "AdaptiveStretchMethod").toInt());
	ui.checkBox_2->setChecked(getXmlValue(root, "FineStatistical") == "true");

	QMessageBox::information(this, tr(u8"成功"), tr(u8"真彩色模板参数已从文件导入: %1").arg(filePath));
}

// 辅助函数：获取XML元素值
QString SystemConfig::getXmlValue(const QDomElement& parent, const QString& name)
{
	QDomNodeList nodes = parent.elementsByTagName(name);
	if (nodes.isEmpty()) {
		return "";
	}
	return nodes.at(0).toElement().text();
}