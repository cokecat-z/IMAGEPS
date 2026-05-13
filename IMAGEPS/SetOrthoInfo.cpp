#include "SetOrthoInfo.h"
#include "IMAGEPS.h"  

SetOrthoInfo::SetOrthoInfo(IMAGEPS* parentImagePS, QWidget* parent)
	: QDialog(parent)
	, m_imagePS(parentImagePS)
{
	ui.setupUi(this);

	if (!m_imagePS && parent) {
		m_imagePS = qobject_cast<IMAGEPS*>(parent);
	}
	setWindowTitle(QString::fromLocal8Bit("设置纠正成果属性信息"));
	resize(600, 450);
	initWidget();
	connects();
	loadConfig();
}

SetOrthoInfo::~SetOrthoInfo()
{}

void SetOrthoInfo::setconfigFile() {
	if (m_imagePS) {
		m_configFile = m_imagePS->projectdir + QString::fromLocal8Bit("ortho_settings.ini");
	}
}

void SetOrthoInfo::initWidget()
{
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 移除帮助按钮
	setSizeGripEnabled(false); // 禁用右下角的大小调整手柄（如果有）
	setFixedSize(size()); // 确保窗口大小固定 

	//ui.lineEdit_4->setText("CGCS20000");
	ui.lineEdit_3->setReadOnly(true);
}

void SetOrthoInfo::connects()
{
	connect(ui.lineEdit_3, &QLineEdit::textChanged, [this](const QString &text) {
		ui.lineEdit_3->setToolTip(text);
	});

	connect(&projectSetting, &ProjectSetting::PROJCStext,
		this, &SetOrthoInfo::setPROJCStextSlot);

	connect(ui.checkBox, &QCheckBox::stateChanged, this, [this](int state) {
		bool isChecked = (state == Qt::Checked);

		ui.label_5->setEnabled(isChecked);
		ui.label_6->setEnabled(isChecked);
		ui.label_7->setEnabled(isChecked);
		ui.comboBox->setEnabled(isChecked);
		ui.comboBox_2->setEnabled(isChecked);
		ui.comboBox_3->setEnabled(isChecked);
	});

	// 在构造函数或初始化函数中添加 
	connect(ui.comboBox_3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {

		QString currentText = ui.comboBox_3->currentText();

		if (currentText == QString::fromLocal8Bit("UTM") || currentText == QString::fromLocal8Bit("WGS84")) {
			ui.comboBox->setEnabled(false);   // 禁用 
			ui.comboBox->setCurrentIndex(1);
			ui.comboBox_2->setEnabled(false);   // 禁用 
		}
		else {
			ui.comboBox->setEnabled(true);    // 启用 
			ui.comboBox_2->setEnabled(true);    // 启用 
		}
	});
}

void SetOrthoInfo::loadConfig()
{
	PublicFunctions::loadFile(QString::fromLocal8Bit("../bin/config/systemConfig/投影配置功能列表.csv"), ",");

	QString filePath = QString::fromLocal8Bit("../bin/config/systemConfig/国家投影信息.txt");
	QFile* file = new QFile(filePath);

	if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(nullptr, u8"错误", filePath + u8" 打开失败");
		return;
	}

	QTextStream* stream = new QTextStream(file);
	stream->setCodec("UTF-8");
	QString tempKey;
	PredefineData tempPreData;
	int tempLines = -1;
	while (!stream->atEnd())
	{
		QString lineStr = stream->readLine().remove('\n');
		if (!lineStr.isEmpty() && lineStr.split(",").size() == 2 && lineStr.split(",").at(1) == "TRUE")
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

void SetOrthoInfo::on_toolButton_clicked() {
	projectSetting.setWindowFlags(Qt::Dialog | Qt::WindowStaysOnTopHint);
	projectSetting.resize(1550, 900);
	projectSetting.openSettingWidgetSlot();
}

void SetOrthoInfo::setPROJCStextSlot(QString PROJCStext)
{
	ui.lineEdit_3->setText(PROJCStext);
	ui.lineEdit_3->setCursorPosition(0);
}

// 保存所有设置到单一配置文件 
void SetOrthoInfo::saveAllSettingsToFile(const QString& configFile)
{
	QSettings settings(configFile, QSettings::IniFormat);
	settings.setIniCodec("UTF-8");

	// 获取当前UI状态 
	QMap<QString, QVariant> currentSettings = getCurrentSettings();

	foreach(const QString& filePath, m_currentFileList) {
		QFileInfo fileInfo(filePath);
		QString fileName = fileInfo.completeBaseName();

		settings.beginGroup(fileName);
		settings.setValue("GSDX", currentSettings["GSDX"]);
		settings.setValue("GSDY", currentSettings["GSDY"]);
		settings.setValue("wktHCS", currentSettings["wktHCS"]);
		settings.setValue("checkBox", currentSettings["checkBox"]);
		settings.setValue("comboBox", currentSettings["comboBox"]);
		settings.setValue("comboBox_2", currentSettings["comboBox_2"]);
		//settings.setValue("lineEdit_4", currentSettings["lineEdit_4"]);
		settings.setValue("comboBox_3", currentSettings["comboBox_3"]);
		if (ui.checkBox->isChecked()) {
			if (ui.comboBox_3->currentText() == "CGCS2000")
			{
				PredefineData AutoCalProj = findCoordinateSystem("China2000", filePath);
				settings.setValue("AutoCalProj", AutoCalProj.PROJCS);
			}
			else if (ui.comboBox_3->currentText() == "WGS84") {
				// 检查一级键是否存在 
				if (!tranData.contains("North WGS_1984")) {
					return;
				}
				// 在指定坐标系类型下查找匹配项 
				const QMap<QString, PredefineData>& typeData = tranData.value("North WGS_1984");

				const PredefineData data = typeData.value("WGS_1984_GEOGCS");
				settings.setValue("AutoCalProj", data.PROJCS);
			}
			else if (ui.comboBox_3->currentText() == "UTM") {

				PredefineData AutoCalProj = findUTMCoordinateSystem("UTM", filePath);
				settings.setValue("AutoCalProj", AutoCalProj.PROJCS);
			}
			else {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("没有找到匹配的ui.comboBox_3项"));
			}
		}
		else {
			settings.setValue("AutoCalProj", "");
		}
		settings.endGroup();
	}
}

// 从单一配置文件加载特定文件的设置 
void SetOrthoInfo::loadSettingsFromFile(const QString& configFile, const QString& fileName)
{
	QSettings settings(configFile, QSettings::IniFormat);
	settings.setIniCodec("UTF-8");

	QMap<QString, QVariant> fileSettings;

	settings.beginGroup(fileName);
	fileSettings["GSDX"] = settings.value("GSDX", "");
	fileSettings["GSDY"] = settings.value("GSDY", "");
	fileSettings["wktHCS"] = settings.value("wktHCS", "");
	fileSettings["checkBox"] = settings.value("checkBox", false);
	fileSettings["comboBox"] = settings.value("comboBox", 0);
	fileSettings["comboBox_2"] = settings.value("comboBox_2", 0);
	//fileSettings["lineEdit_4"] = settings.value("lineEdit_4", "CGCS2000");
	fileSettings["comboBox_3"] = settings.value("comboBox_3", 0);
	settings.endGroup();

	applySettingsToUI(fileSettings);

	ui.lineEdit_3->setCursorPosition(0);
}

// 从单一配置文件加载多个文件的共同设置 
void SetOrthoInfo::loadCommonSettingsFromFile(const QString& configFile, const QStringList& fileNames)
{
	if (fileNames.isEmpty())  return;

	QSettings settings(configFile, QSettings::IniFormat);
	settings.setIniCodec("UTF-8");

	// 收集所有设置 
	QList<QMap<QString, QVariant>> allSettings;

	foreach(const QString& fileName, fileNames) {
		QMap<QString, QVariant> fileSettings;

		settings.beginGroup(fileName);
		fileSettings["GSDX"] = settings.value("GSDX", "");
		fileSettings["GSDY"] = settings.value("GSDY", "");
		fileSettings["wktHCS"] = settings.value("wktHCS", "");
		fileSettings["checkBox"] = settings.value("checkBox", false);
		fileSettings["comboBox"] = settings.value("comboBox", 0);
		fileSettings["comboBox_2"] = settings.value("comboBox_2", 0);
		//fileSettings["lineEdit_4"] = settings.value("lineEdit_4", "CGCS2000");
		fileSettings["comboBox_3"] = settings.value("comboBox_3", 0);
		settings.endGroup();

		allSettings.append(fileSettings);
	}

	// 找出共同设置 
	QMap<QString, QVariant> commonSettings;
	if (!allSettings.isEmpty()) {
		commonSettings = allSettings.first();

		for (int i = 1; i < allSettings.size(); ++i) {
			QMap<QString, QVariant> current = allSettings[i];

			foreach(const QString& key, commonSettings.keys()) {
				if (current[key] != commonSettings[key]) {
					// 设置不同时，使用默认值或空值 
					if (key == "lineEdit") {
						commonSettings[key] = ""; // 文件名不同时清空 
					}
					else if (key == "checkBox") {
						commonSettings[key] = false;
					}
					else if (key.startsWith("comboBox")) {
						commonSettings[key] = 0;
					}
					else {
						commonSettings[key] = "";
					}
				}
			}
		}
	}

	applySettingsToUI(commonSettings);

	ui.lineEdit_3->setCursorPosition(0);
}

// 获取当前控件状态 
QMap<QString, QVariant> SetOrthoInfo::getCurrentSettings() const
{
	QMap<QString, QVariant> settings;
	settings["GSDX"] = ui.lineEdit->text();
	settings["GSDY"] = ui.lineEdit_2->text();
	settings["wktHCS"] = ui.lineEdit_3->text();
	settings["checkBox"] = ui.checkBox->isChecked();
	settings["comboBox"] = ui.comboBox->currentIndex();
	settings["comboBox_2"] = ui.comboBox_2->currentIndex();
	settings["comboBox_3"] = ui.comboBox_3->currentIndex();
	//settings["lineEdit_4"] = ui.lineEdit_4->text();
	return settings;
}

// 应用设置到UI 
void SetOrthoInfo::applySettingsToUI(const QMap<QString, QVariant>& settings)
{
	ui.lineEdit->setText(settings["GSDX"].toString());
	ui.lineEdit_2->setText(settings["GSDY"].toString());
	ui.lineEdit_3->setText(settings["wktHCS"].toString());
	ui.checkBox->setChecked(settings["checkBox"].toBool());
	ui.comboBox->setCurrentIndex(settings["comboBox"].toInt());
	ui.comboBox_2->setCurrentIndex(settings["comboBox_2"].toInt());
	ui.comboBox_3->setCurrentIndex(settings["comboBox_3"].toInt());
	//ui.lineEdit_4->setText(settings["lineEdit_4"].toString());
}

void SetOrthoInfo::setCurrentFileList(const QStringList& filePaths)
{
	m_currentFileList = filePaths;
	if (!filePaths.isEmpty()) {
		// 提取纯文件名 
		QStringList fileNames;
		foreach(const QString& path, filePaths) {
			fileNames.append(QFileInfo(path).completeBaseName());
		}

		if (filePaths.size() == 1) {
			loadSettingsFromFile(m_configFile, fileNames.first());
		}
		else {
			loadCommonSettingsFromFile(m_configFile, fileNames);
		}
	}
}

QStringList SetOrthoInfo::getCurrentFileList() const
{
	return m_currentFileList;
}

PredefineData SetOrthoInfo::findCoordinateSystem(const QString& coordType, const QString& imagePath)
{
	// 检查一级键是否存在 
	if (!tranData.contains(coordType)) {
		return PredefineData();
	}

	// 从UI控件获取参数 
	QString degreeType = ui.comboBox->currentIndex() == 0 ? "3" : "6";
	bool hasZoneNumber = ui.comboBox_2->currentIndex() == 1;

	// 获取图像元数据 
	ImageGeoMetadata* metadata = m_imagePS->getImageMetadata(imagePath);
	if (!metadata) {
		return PredefineData();
	}

	// 计算中心经度
	double centerLon = (metadata->Corners[0] + metadata->Corners[2]) / 2;

	// 在指定坐标系类型下查找匹配项 
	const QMap<QString, PredefineData>& typeData = tranData.value(coordType);

	foreach(const QString& coordName, typeData.keys()) {
		const PredefineData& data = typeData.value(coordName);
		// 提取括号内的内容 
		int start = coordName.indexOf("(");
		int end = coordName.indexOf(")");
		if (start == -1 || end == -1 || start >= end) continue;

		QString params = coordName.mid(start + 1, end - start - 1);
		QStringList parts = params.split(" ");
		if (parts.size() < 4) continue;

		// 检查度带类型 
		if (parts[0] != degreeType) continue;

		// 检查带号信息 
		QString zoneInfo = parts[3];
		if ((hasZoneNumber && zoneInfo.toUtf8() != u8"有带号") ||
			(!hasZoneNumber && zoneInfo.toUtf8() != u8"无带号")) {
			continue;
		}

		// 检查中央经线 
		QString centralMeridianStr = parts[2].toUtf8();
		if (!centralMeridianStr.startsWith(u8"中央经线=")) continue;

		bool ok = false;
		double centralMeridian = centralMeridianStr.mid(5).toDouble(&ok);
		if (!ok) continue;

		// 根据度带类型设置不同的经度范围 
		double range = (degreeType == "3") ? 1.5 : 3.0; // 3度带±1.5°，6度带±3°
		// 检查中心经度是否在范围内 
		if (qAbs(centerLon - centralMeridian) <= range) {
			return data; // 找到匹配项 
		}
	}

	return PredefineData(); // 未找到匹配项 
}

PredefineData SetOrthoInfo::findUTMCoordinateSystem(const QString& coordType, const QString& imagePath)
{
	// 检查一级键是否存在
	if (!tranData.contains(coordType)) {
		return PredefineData();
	}

	// 获取图像元数据
	ImageGeoMetadata* metadata = m_imagePS->getImageMetadata(imagePath);
	if (!metadata) {
		return PredefineData();
	}

	// 计算中心经度和判断南北半球
	double centerLon = (metadata->Corners[0] + metadata->Corners[2]) / 2;
	bool isNorthHemisphere = metadata->Corners[1] >= 0 && metadata->Corners[3] >= 0;

	// 在指定坐标系类型下查找匹配项 
	const QMap<QString, PredefineData>& typeData = tranData.value(coordType);

	for (auto it = typeData.constBegin(); it != typeData.constEnd(); ++it) {
		const QString& coordName = it.key();
		const PredefineData& data = it.value();

		// 分割字符串，查找 "Zone" 后的下一个词 
		QStringList parts = coordName.split(" ", Qt::SkipEmptyParts);
		int zoneIndex = parts.indexOf("Zone");
		if (zoneIndex == -1 || zoneIndex + 1 >= parts.size()) {
			continue; // 没有找到 "Zone" 或后面没有词 
		}

		// 提取 "Zone" 后的下一个词（比如 "14N"）
		QString zoneInfo = parts[zoneIndex + 1];
		if (zoneInfo.isEmpty()) {
			continue;
		}

		// 提取 N/S 标识（最后一个字符）
		QChar hemisphereChar = zoneInfo.back();
		if (hemisphereChar != 'N' && hemisphereChar != 'S') {
			continue; // 不是有效的 N/S 标识 
		}

		// 检查南北半球是否匹配
		if ((isNorthHemisphere && hemisphereChar != 'N') ||
			(!isNorthHemisphere && hemisphereChar != 'S')) {
			continue;
		}

		// 检查中央经线是否匹配（从 PROJCS 参数获取）
		double centralMeridian = data.List_projectInfo[2]; // Central_Meridian 存储在 List_projectInfo[2]
		if (qAbs(centerLon - centralMeridian) <= 3.0) { // +_3度
			return data;
		}
	}

	return PredefineData(); // 未找到匹配
}

QMap<QString, QString> SetOrthoInfo::getFileConfigInfo(const QString& fileName)
{
	setconfigFile();

	QMap<QString, QString> result;

	//检查配置文件是否存在
	if (!QFile::exists(m_configFile)) {
		return result; // 返回空的QMap 
	}

	//从配置文件中读取数据
	QSettings settings(m_configFile, QSettings::IniFormat);
	settings.setIniCodec("UTF-8");

	// 检查是否存在该文件名的配置组 
	if (!settings.childGroups().contains(fileName)) {
		return result; // 返回空的QMap 
	}

	settings.beginGroup(fileName);

	// 获取GSDX和GSDY 
	QString gsdx = settings.value("GSDX", "").toString();
	QString gsdy = settings.value("GSDY", "").toString();

	//根据checkBox状态获取投影信息
	QString projectionInfo;
	if (settings.value("checkBox", "").toString() == "true") {
		// 如果checkBox选中，获取AutoCalProj
		projectionInfo = settings.value("AutoCalProj", "").toString();
	}
	else {
		// 如果checkBox未选中，获取wktHCS 
		projectionInfo = settings.value("wktHCS", "").toString();
	}

	settings.endGroup();

	// 检查是否所有值都为空（理论上不会发生，因为已经检查了组存在）
	if (gsdx.isEmpty() && gsdy.isEmpty() && projectionInfo.isEmpty()) {
		return result; // 返回空的QMap
	}

	// 填充返回结果 
	result.insert("GSDX", gsdx);
	result.insert("GSDY", gsdy);
	result.insert("Projection", projectionInfo);

	return result;
}