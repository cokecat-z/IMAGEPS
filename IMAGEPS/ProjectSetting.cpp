#include "ProjectSetting.h"
#include "IMAGEPS.h"

ProjectSetting::ProjectSetting(QDialog *parent, IMAGEPS* imagePSInstance)
	: QDialog(parent)
	, m_imagePS(imagePSInstance)
{
	ui.setupUi(this);
	this->setWindowTitle(QString::fromLocal8Bit("投影设置"));
	//this->resize(1500, 800);
	//this->setMinimumSize(800, 600);

	loadConfig();

	initWidget();

	connects();
}

void ProjectSetting::loadConfig(QString funModule)
{
	funListConifg.clear();

	if (funModule == QString::fromLocal8Bit("投影配置功能列表"))
		funListConifg = PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/systemConfig/投影配置功能列表.csv"), ",");
	else if (funModule == QString::fromLocal8Bit("预定义坐标系统"))
	{
		QString filePath = QString::fromLocal8Bit("../bin/config/systemConfig/国家投影信息.txt");
		QFile* file = new QFile(filePath);

		if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
		{
			QMessageBox::critical(nullptr, u8"错误", filePath + u8" 打开失败");
			return ;
		}

		QTextStream* stream = new QTextStream(file);
		stream->setCodec("UTF-8");
		QString tempKey;
		PredefineData tempPreData;
		int tempLines = -1;
		while (!stream->atEnd())
		{
			QString lineStr = stream->readLine().remove('\n');
			if(!lineStr.isEmpty() && lineStr.split(",").size() == 2 && lineStr.split(",").at(1) == "TRUE")
			{
				tempKey = lineStr.split(",").at(0);
			}
			else if (!lineStr.isEmpty() && !tempKey.isEmpty())
			{
				tempLines = lineStr.toInt();
				for (int i = 0; i < tempLines; i++)
				{
					//提取投影名称一行数据
					QString projectName = stream->readLine().remove('\n');
					//提取详细信息一行数据
					lineStr = stream->readLine().remove('\n');
					
					if (!projectName.isEmpty() && !lineStr.isEmpty())
					{
						//PROJCS["Transverse  GEOGCS["China2000", DATUM["China2000", SPHEROID["CGCS2000", 6378137, 298.257222101], TOWGS84[0, 0, 0, 0, 0, 0, 0]], PRIMEM["Greenwich", 0], UNIT["degree", 0.0174532925199433]], UNIT["meters", 1], PROJECTION["Transverse_Mercator"], PARAMETER["latitude_of_origin", 0], PARAMETER["central_meridian", 75], PARAMETER["scale_factor", 1], PARAMETER["false_easting", 500000], PARAMETER["false_northing", 0]]
						if (lineStr.left(6) == QString::fromLocal8Bit("PROJCS") || lineStr.left(6) == QString::fromLocal8Bit("GEOGCS"))
							tempPreData.PROJCS = lineStr;
						
						// 单独匹配 DAM[
						QRegularExpression reDam(R"(DATUM\[\"([^\"]+)\")");
						QRegularExpressionMatch matchDam = reDam.match(lineStr);
						if (matchDam.hasMatch())
						{
							tempPreData.Datum = matchDam.captured(1);
						}

						// 单独匹配 SPID[
						QRegularExpression reSpid(R"(SPHEROID\[\"([^\"]+)\",\s*([^,]+),\s*([^\]]+))");
						QRegularExpressionMatch matchSpid = reSpid.match(lineStr);
						if (matchSpid.hasMatch()) 
						{
							tempPreData.SPHEROID = matchSpid.captured(1);
							tempPreData.SemiMajor = matchSpid.captured(2).trimmed().toInt();
							tempPreData.InFlattening = matchSpid.captured(3).trimmed().toDouble();
						}

						// 单独匹配 TOWGS84[
						QRegularExpression reTowgs84(R"(TOWGS84\[([^]]+)\])");
						QRegularExpressionMatch matchTowgs84 = reTowgs84.match(lineStr);
						if (matchTowgs84.hasMatch()) {
							QString towgs84Content = matchTowgs84.captured(1); // 获取 TOWGS84[ 后的内容
							QStringList numbers = towgs84Content.split(",");   // 按逗号分割数字

							for (const QString& number : numbers) {
								tempPreData.List_TOWGS84.push_back(number.trimmed().toInt());
							}
						}

						// 单独匹配 PRIMEM[
						QRegularExpression rePrime(R"(PRIMEM\[\"([^\"]+)\",(\d+))");
						QRegularExpressionMatch matchPrime = rePrime.match(lineStr);
						if (matchPrime.hasMatch())
						{
							tempPreData.PrimeMeridian = matchPrime.captured(1) + "," + matchPrime.captured(2).trimmed();
						}

						// 单独匹配 PARAMETER[
						QRegularExpression reParam(R"(PARAMETER\[\"([^\"]+)\",\s*([^\]]+))");
						QRegularExpressionMatchIterator itParam = reParam.globalMatch(lineStr);
						int paramCount = 0;
						while (itParam.hasNext())
						{
							QRegularExpressionMatch matchParam = itParam.next();
							tempPreData.List_projectInfo.push_back(matchParam.captured(2).trimmed().toInt());
						}

						tranData[tempKey][projectName] = tempPreData;
						tempPreData.List_projectInfo.clear();
						tempPreData.List_TOWGS84.clear();
					}					
 				}
			}
			
		}

		file->close();
	}
	else if (funModule == QString::fromLocal8Bit("自定义投影坐标系"))
		funModuleTemp = PublicFunctions::loadConfigFile(QString::fromLocal8Bit("../bin/config/systemConfig/%1.csv").arg(funModule));
}

void ProjectSetting::initWidget()
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
	ui.stackedWidget->setCurrentIndex(0);
	ui.okButton_2->setEnabled(false);

}

void ProjectSetting::connects()
{
	connect(ui.funListWidget, &QListWidget::itemClicked, this, &ProjectSetting::funListWidgetItemSlot);

	connect(ui.predefineCoordinate_projectTypeBox,SIGNAL(currentIndexChanged(const QString &)), this, SLOT(projectTypeBoxSlot(QString)));

	connect(ui.predefineCoordinate_projectNameListWidget, &QListWidget::itemClicked, this, &ProjectSetting::projectNameListWidgetItemSlot);

	connect(ui.predefineCoordinate_basicImageBut, &QPushButton::clicked, this, [=]
	{
		QString imagePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("打开影像"), "./", tr("TIF Image(*.tif);;TIFF Image(*.tiff);;ERDAS Image(*.img);;BIL Image(*.bil);;JPG Image(*.jpg);;JPEG Image(*.jpeg);;BMP Image(*.bmp);;CoSAR(*.cos);;PCI(*.pix);;ArcInfo(*.adf);;Envi HDR(*.hdr);;HDF4 Image(*.hdf)"));
		QFile newProFile(imagePath);
		if (imagePath.isEmpty())
			return;
		else if (!newProFile.open(QIODevice::ReadOnly | QIODevice::Text) || imagePath.isEmpty())
		{
			QMessageBox::critical(this, u8"错误", QString::fromLocal8Bit("打开失败"));
			return;
		}
		 
	});

	connect(ui.customProject_projectTypeBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(customProject_projectTypeBoxSlot(QString)));
	
	connect(ui.okButton, &QPushButton::clicked, this, &ProjectSetting::okButtonSlot);

	// 监听 listWidget1 的变化
	connect(ui.predefineCoordinate_projectNameListWidget, &QListWidget::itemChanged, this, &ProjectSetting::onListModified);
	connect(ui.predefineCoordinate_projectNameListWidget->model(), &QAbstractItemModel::rowsInserted, this, &ProjectSetting::onListModified);
	connect(ui.predefineCoordinate_projectNameListWidget->model(), &QAbstractItemModel::rowsRemoved, this, &ProjectSetting::onListModified);
	connect(ui.predefineCoordinate_projectNameListWidget, &QListWidget::currentItemChanged,
		this, [this](QListWidgetItem *current, QListWidgetItem *previous) {
		Q_UNUSED(previous);
		if (current) {
			this->onListModified();  // 选择变化视为修改
		}
	});

	// 应用按钮点击后恢复禁用状态
	connect(ui.okButton_2, &QPushButton::clicked, this, [this]() {
		ui.okButton_2->setEnabled(false);  // 保存后禁用
		applyButtonSlot();

	});

	connect(m_imagePS, &IMAGEPS::aboutToClose, this,
		[=]() {
		this->close();
	});
}

void ProjectSetting::funListWidgetItemSlot(QListWidgetItem* item)
{
	if (item->text() == QString::fromLocal8Bit("预定义坐标系统"))
	{
		ui.stackedWidget->setCurrentWidget(ui.predefineCoordinate_Page);

		if (!funModuleBool[item->text()])
			funModuleBool[item->text()] = true;
		else
			return;

		loadConfig(item->text());

		for (auto iter = tranData.begin(); iter != tranData.end(); iter++)
		{
			ui.predefineCoordinate_projectTypeBox->addItem(iter.key());	
		}
		
	}
	else if (item->text() == QString::fromLocal8Bit("自定义投影坐标系"))
	{
		ui.stackedWidget->setCurrentWidget(ui.customProject_Page);

		if (!funModuleBool[item->text()])
			funModuleBool[item->text()] = true;
		else
			return;

		loadConfig(item->text());

		projectTypetoInfo["PTZZWW_Page"] << "Mercator Projection" << "Transverse Mercator Projection" << "Stereographic Projection";
		projectTypetoInfo["PDN_Page"] << "Universal Transverse Mercator (UTM) Projection";
		projectTypetoInfo["PTZDZWW_Page"] << "Oblique Mercator Projection";
		projectTypetoInfo["PTZWW_Page"] << "Miller Cylindrical Projection" << "Cassini Projection" << "Gnomonic Projection" << "Orthographic Projection" << "Lambert Azimuthal Equal Area Projection" << "Azimuthal Equidistant Projection";
		projectTypetoInfo["PBZWW_Page"] << "Lambert Cylindrical Equal Area Projection";
		projectTypetoInfo["PZWW_Page"] << "Gall (Stereographic) Projection" << "Sinusoidal Projection" << "Mollweide Projection" << "Robinson Projection" << "Eckert TV Projection" << "Eckert VI Projection" << "Van der Grinten I Projection";
		projectTypetoInfo["PZZWW_Page"] << "Equidistant Cylindrical Projection";
		projectTypetoInfo["PTTTWW_Page"] << "Lambert_Conformal_Conic_1SP";
		projectTypetoInfo["PWWTTWW_Page"] << "Lambert_Conformal_Conic_2SP" << "Equidistant Conic Projection" << "Albers Equal Area Projection";
		projectTypetoInfo["PTTWW_Page"] << "Polyconic (American) Projection" << "";

		for (auto childIter : funModuleTemp[ui.customProject_projectTypeLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.customProject_projectTypeBox->addItem(childIter.at(0));
		}

		for (auto childIter : funModuleTemp[ui.customProject_ellipsoidNameLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.customProject_ellipsoidNameBox->addItem(childIter.at(0));
		}
	}
}

void ProjectSetting::customProject_projectTypeBoxSlot(QString text)
{
	if (projectTypetoInfo["PTZZWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PTZZWW_Page);

		for (auto childIter : funModuleTemp[ui.PTZZWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PTZZWW_PMBox->addItem(childIter.at(0));
		}		
	}
	else if (projectTypetoInfo["PDN_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PDN_Page);

		for (auto childIter : funModuleTemp[ui.PDN_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PDN_PMBox->addItem(childIter.at(0));
		}
	}
	else if (projectTypetoInfo["PTZDZWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PTZDZWW_Page);
		for (auto childIter : funModuleTemp[ui.PTZDZWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PTZDZWW_PMBox->addItem(childIter.at(0));
		}
	}		
	else if (projectTypetoInfo["PTZWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PTZWW_Page);
		for (auto childIter : funModuleTemp[ui.PTZWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PTZWW_PMBox->addItem(childIter.at(0));
		}
	}		
	else if (projectTypetoInfo["PBZWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PBZWW_Page);
		for (auto childIter : funModuleTemp[ui.PBZWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PBZWW_PMBox->addItem(childIter.at(0));
		}
	}		
	else if (projectTypetoInfo["PZWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PZWW_Page);
		for (auto childIter : funModuleTemp[ui.PZWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PZWW_PMBox->addItem(childIter.at(0));
		}
	}		
	else if (projectTypetoInfo["PZZWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PZZWW_Page);
		for (auto childIter : funModuleTemp[ui.PZZWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PZZWW_PMBox->addItem(childIter.at(0));
		}
	}
	else if (projectTypetoInfo["PTTTWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PTTTWW_Page);
		for (auto childIter : funModuleTemp[ui.PTTTWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PTTTWW_PMBox->addItem(childIter.at(0));
		}
	}
	else if (projectTypetoInfo["PWWTTWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PWWTTWW_Page);
		for (auto childIter : funModuleTemp[ui.PWWTTWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PWWTTWW_PMBox->addItem(childIter.at(0));
		}
	}
	else if (projectTypetoInfo["PTTWW_Page"].contains(text))
	{
		ui.stackedWidget_2->setCurrentWidget(ui.PTTWW_Page);
		for (auto childIter : funModuleTemp[ui.PTTWW_PMLabel->text()])
		{
			if (childIter.at(1) == "TRUE")
				ui.PTTWW_PMBox->addItem(childIter.at(0));
		}
	}
}

void ProjectSetting::projectNameListWidgetItemSlot(QListWidgetItem* item)
{
	QString boxText = ui.predefineCoordinate_projectTypeBox->currentText();

	ui.predefineCoordinate_DatumEdit->setText(tranData[boxText][item->text()].Datum);
	ui.predefineCoordinate_SPHEROIDEdit->setText(tranData[boxText][item->text()].SPHEROID);
	ui.predefineCoordinate_SemiMajorEdit->setText(QString::number(tranData[boxText][item->text()].SemiMajor));
	ui.predefineCoordinate_InvFlatteningEdit->setText(QString::number(tranData[boxText][item->text()].InFlattening, 'f', 9));
	ui.predefineCoordinate_PrimerMeridianEdit->setText(tranData[boxText][item->text()].PrimeMeridian);

	if (!tranData[boxText][item->text()].List_TOWGS84.isEmpty())
	{
		ui.predefineCoordinate_XtranslationEdit->setText(QString::number(tranData[boxText][item->text()].List_TOWGS84[0]));
		ui.predefineCoordinate_YtranslationEdit->setText(QString::number(tranData[boxText][item->text()].List_TOWGS84[1]));
		ui.predefineCoordinate_ZtranslationEdit->setText(QString::number(tranData[boxText][item->text()].List_TOWGS84[2]));
		ui.predefineCoordinate_XRotateEdit->setText(QString::number(tranData[boxText][item->text()].List_TOWGS84[3]));
		ui.predefineCoordinate_YRotateEdit->setText(QString::number(tranData[boxText][item->text()].List_TOWGS84[4]));
		ui.predefineCoordinate_ZRotateEdit->setText(QString::number(tranData[boxText][item->text()].List_TOWGS84[5]));
		ui.predefineCoordinate_ScaleEdit->setText(QString::number(tranData[boxText][item->text()].List_TOWGS84[6]));
	}

	if (!tranData[boxText][item->text()].List_projectInfo.isEmpty())
	{
		ui.predefineCoordinate_LatitudeEdit->setText(QString::number(tranData[boxText][item->text()].List_projectInfo[0]));
		ui.predefineCoordinate_CentralEdit->setText(QString::number(tranData[boxText][item->text()].List_projectInfo[1]));
		ui.predefineCoordinate_ScaleFactorEdit->setText(QString::number(tranData[boxText][item->text()].List_projectInfo[2]));
		ui.predefineCoordinate_FalseEastEdit->setText(QString::number(tranData[boxText][item->text()].List_projectInfo[3]));
		ui.predefineCoordinate_FalseNorthEdit->setText(QString::number(tranData[boxText][item->text()].List_projectInfo[4]));
	}
}

void ProjectSetting::projectTypeBoxSlot(QString text)
{
	ui.predefineCoordinate_projectNameListWidget->clear();
	for (auto iter = tranData[text].begin(); iter != tranData[text].end(); iter++)
	{
		ui.predefineCoordinate_projectNameListWidget->addItem(iter.key());
	}
}

void ProjectSetting::openSettingWidgetSlot()
{
	this->show();
}

void ProjectSetting::okButtonSlot()
{
	QListWidgetItem *selectedItem = ui.predefineCoordinate_projectNameListWidget->currentItem();
	if (selectedItem) {
		QString selectedText = ui.predefineCoordinate_projectTypeBox->currentText();
		QString text = selectedItem->text(); // 获取选中项的文本

		auto selectedit = tranData.find(selectedText);
		if (selectedit != tranData.end()) {
			auto it = selectedit.value().find(text);  // 获取对应的 value
			if (it != selectedit.value().end())
				emit PROJCStext(it.value().PROJCS);
		}
		else {
			qDebug() << "Key not found!";
		}
	}
	this->close();
}

void ProjectSetting::applyButtonSlot()
{
	QListWidgetItem *selectedItem = ui.predefineCoordinate_projectNameListWidget->currentItem();
	if (selectedItem) {
		QString selectedText = ui.predefineCoordinate_projectTypeBox->currentText();
		QString text = selectedItem->text(); // 获取选中项的文本

		auto selectedit = tranData.find(selectedText);
		if (selectedit != tranData.end()) {
			auto it = selectedit.value().find(text);  // 获取对应的 value
			if (it != selectedit.value().end())
				emit PROJCStext(it.value().PROJCS);
		}
		else {
			qDebug() << "Key not found!";
		}
	}
}

void ProjectSetting::onListModified() {
	ui.okButton_2->setEnabled(true);  // 只要任一列表修改，就启用应用按钮
}

ProjectSetting::~ProjectSetting()
{

}