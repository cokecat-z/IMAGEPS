#include "PublicFunctions.h"
#include "IMAGEPS.h"

// 将OpenCV的Mat转换为QImage
QImage PublicFunctions::cvMatToQImage(const cv::Mat &mat)
{
	// 8-bits unsigned, NO. OF CHANNELS = 1
	if (mat.type() == CV_8UC1) {
		QImage image(mat.cols, mat.rows, QImage::Format_Grayscale8);
		image.bits();
		return image;
	}
	// 8-bits unsigned, NO. OF CHANNELS = 3
	else if (mat.type() == CV_8UC3) {
		// Copy input Mat
		cv::Mat rgb;
		cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
		QImage image(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
		return image.copy();
	}
	// 8-bits unsigned, NO. OF CHANNELS = 4
	else if (mat.type() == CV_8UC4) {
		QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	}
	else {
		return QImage();
	}
}

//arg:文件地址 文件内容分割符
QList<QStringList> PublicFunctions::loadFile(QString filePath, QString splitFlag)
{
	QList<QStringList> data;
	QFile* file = new QFile(filePath);
	
	if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(nullptr, "Error", filePath + " open failed");
		return QList<QStringList>();
	}

	QTextStream* stream = new QTextStream(file);
	stream->setCodec("UTF-8");
	while (!stream->atEnd())
	{
		QString lineStr = stream->readLine();
		lineStr.remove('\n');
		data.push_back(lineStr.split(splitFlag));
	}

	file->close();
	
	return data;
}

QMap<QString, QList<QStringList>> PublicFunctions::loadConfigFile(QString filePath)
{
	QMap<QString, QList<QStringList>> data;
	QFile* file = new QFile(filePath);

	if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(nullptr, "Error", filePath + " open failed");
		return QMap<QString, QList<QStringList>>();
	}

	QTextStream* stream = new QTextStream(file);
	stream->setCodec("UTF-8");
	QString keyStr;
	while (!stream->atEnd())
	{
		QString lineStr = stream->readLine();
		lineStr.remove('\n');
		if (lineStr.contains('#'))
		{
			keyStr = lineStr.remove('#');
			data[keyStr].clear();
		}
		else
		{
			if (!keyStr.isEmpty() && !lineStr.isEmpty())
				data[keyStr].push_back(lineStr.split(","));
		}
	}

	file->close();

	return data;
}

QList<QPair<QString, QList<QStringList>>> PublicFunctions::loadConfigFileL(QString filePath)
{
	QList<QPair<QString, QList<QStringList>>> dataList;
	QFile file(filePath);

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QMessageBox::critical(nullptr, "Error", filePath + " open failed");
		return dataList;
	}

	QTextStream stream(&file);
	stream.setCodec("UTF-8");

	QString currentKey;
	QList<QStringList> currentValueList;

	while (!stream.atEnd())
	{
		QString lineStr = stream.readLine().trimmed();

		if (lineStr.contains('#'))
		{
			// 保存上一组数据（如果有）
			if (!currentKey.isEmpty()) {
				dataList.append(qMakePair(currentKey, currentValueList));
				currentValueList.clear();
			}

			// 开始新的一组 
			currentKey = lineStr.remove('#');
		}
		else if (!currentKey.isEmpty() && !lineStr.isEmpty())
		{
			// 添加当前组的数据项
			currentValueList.append(lineStr.split(","));
		}
	}

	// 添加最后一组数据
	if (!currentKey.isEmpty()) {
		dataList.append(qMakePair(currentKey, currentValueList));
	}

	file.close();
	return dataList;
}


//bool PublicFunctions::writeToCsv(const QString &filePath, const QList<QStringList> &records) {
//	QSet<QPair<QString, QString>> existingEntries;
//
//	// 读取现有文件内容
//	if (QFile::exists(filePath)) {
//		QFile readFile(filePath);
//		if (!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
//			return false;
//		}
//		QTextStream in(&readFile);
//		while (!in.atEnd()) {
//			QString line = in.readLine().trimmed();
//			QStringList fields = line.split(',');
//			if (fields.size() >= 2) {
//				QString first = fields[0].trimmed();
//				QString second = fields[1].trimmed();
//				existingEntries.insert(qMakePair(first, second));
//			}
//		}
//		readFile.close();
//	}
//
//	// 收集需要写入的新记录
//	QList<QStringList> newRecords;
//	for (const auto &sl : records) {
//		if (sl.size() >= 2) {
//			QString first = sl[0].trimmed();
//			QString second = sl[1].trimmed();
//			if (!existingEntries.contains(qMakePair(first, second))) {
//				newRecords.append({ first, second });
//				existingEntries.insert(qMakePair(first, second)); // 防止同一批重复
//			}
//		}
//	}
//
//	if (newRecords.isEmpty()) {
//		return true; // 无需写入
//	}
//
//	// 追加写入新记录
//	QFile writeFile(filePath);
//	if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
//		return false;
//	}
//
//	QTextStream out(&writeFile);
//	for (const auto &sl : newRecords) {
//		out << sl[0] << "," << sl[1] << "\n";
//	}
//
//	return true;
//}
//
//
//QHash<QString, QString>  PublicFunctions::loadFromCsv(const QString &filePath) {
//	QHash<QString, QString> fileMap;
//
//	QFile file(filePath);
//	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//		return fileMap;
//	}
//	QTextStream in(&file);
//	while (!in.atEnd()) {
//		QStringList fields = in.readLine().split(",");
//		if (fields.size() >= 2) {
//			fileMap.insert(fields[0].trimmed(),
//				fields[1].trimmed());
//		}
//	}
//
//	file.close();
//	return fileMap;
//}

// 实现文件 
bool PublicFunctions::writeToCsv(const QString &filePath, const QList<QStringList> &records) {
	QSet<QPair<QString, QString>> existingEntries;

	// 读取现有文件内容 
	if (QFile::exists(filePath)) {
		QFile readFile(filePath);
		if (!readFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			return false;
		}
		QTextStream in(&readFile);
		while (!in.atEnd()) {
			QString line = in.readLine().trimmed();
			QStringList fields = parseCsvLine(line);
			if (fields.size() >= 2) {
				QString first = unescapeCsvField(fields[0]);
				QString second = unescapeCsvField(fields[1]);
				existingEntries.insert(qMakePair(first, second));
			}
		}
		readFile.close();
	}

	// 收集需要写入的新记录
	QList<QStringList> newRecords;
	for (const auto &sl : records) {
		if (sl.size() >= 2) {
			QString first = sl[0].trimmed();
			QString second = sl[1].trimmed();
			if (!existingEntries.contains(qMakePair(first, second))) {
				newRecords.append({ first, second });
				existingEntries.insert(qMakePair(first, second));
			}
		}
	}

	if (newRecords.isEmpty()) {
		return true;
	}

	// 追加写入新记录
	QFile writeFile(filePath);
	if (!writeFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
		return false;
	}

	QTextStream out(&writeFile);
	for (const auto &sl : newRecords) {
		QString first = escapeCsvField(sl[0]);
		QString second = escapeCsvField(sl[1]);
		out << first << "," << second << "\n";
	}

	return true;
}

QHash<QString, QString> PublicFunctions::loadFromCsv(const QString &filePath) {
	QHash<QString, QString> fileMap;

	QFile file(filePath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return fileMap;
	}

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine();
		QStringList fields = parseCsvLine(line);
		if (fields.size() >= 2) {
			QString key = unescapeCsvField(fields[0]);
			QString value = unescapeCsvField(fields[1]);
			fileMap.insert(key, value);
		}
	}

	file.close();
	return fileMap;
}

QStringList PublicFunctions::parseCsvLine(const QString &line) {
	QStringList result;
	QString currentField;
	bool inQuotes = false;

	for (int i = 0; i < line.length(); ++i) {
		QChar ch = line[i];

		if (ch == '\"') {
			if (inQuotes && i + 1 < line.length() && line[i + 1] == '\"') {
				// 转义的双引号 
				currentField += '\"';
				i++;
			}
			else {
				// 引号开始/结束 
				inQuotes = !inQuotes;
			}
		}
		else if (ch == ',' && !inQuotes) {
			// 字段分隔符 
			result.append(currentField);
			currentField.clear();
		}
		else {
			// 普通字符
			currentField += ch;
		}
	}

	// 添加最后一个字段
	if (!currentField.isEmpty() || line.endsWith(',')) {
		result.append(currentField);
	}

	return result;
}

QString PublicFunctions::escapeCsvField(const QString &field) {
	QString trimmed = field.trimmed();

	// 如果字段为空，直接返回
	if (trimmed.isEmpty()) {
		return trimmed;
	}

	// 检查是否需要引号包围 
	bool needsQuotes = trimmed.contains(',') ||
		trimmed.contains('\"') ||
		trimmed.contains('\n') ||
		trimmed.contains('\r') ||
		trimmed.startsWith(' ') ||
		trimmed.endsWith(' ');

	if (!needsQuotes) {
		return trimmed;
	}

	// 转义字段中的引号
	QString escaped = trimmed;
	escaped.replace('\"', "\"\"");

	// 用引号包围 
	return '\"' + escaped + '\"';
}

QString PublicFunctions::unescapeCsvField(const QString &field) {
	QString result = field.trimmed();

	if (result.isEmpty()) {
		return result;
	}

	// 移除引号包围
	if (result.startsWith('\"') && result.endsWith('\"')) {
		result = result.mid(1, result.length() - 2);
		// 处理转义的双引号 
		result.replace("\"\"", "\"");
	}

	return result;
}

QString PublicFunctions::findFileDir(const QHash<QString, QString> &fileMap,
	const QString &filename) {
	return fileMap.value(filename, "");  // 找不到返回空字符串    
}

QStringList PublicFunctions::getNewlyCreatedFiles(const QString& dirPath, const QDateTime & markerFileTime)
{
	QDir dir(dirPath);
	//QFileInfo markerInfo(markerFile);
	QStringList newFiles;

	// 获取标记文件创建时间后生成的文件 
	foreach(const QFileInfo& fileInfo, dir.entryInfoList(QDir::Files)) {
		//if (fileInfo.lastModified() >= markerInfo.created() &&
		//	!fileInfo.fileName().startsWith("processing_") &&
		//	fileInfo.fileName().startsWith("model_") &&
		//	fileInfo.fileName().endsWith(".tp")&& 
		//	fileInfo.fileName().endsWith(".tie")) {
		//	newFiles << fileInfo.absoluteFilePath();
		//}
		//if (fileInfo.lastModified() >= markerFileTime &&
		//	!fileInfo.fileName().startsWith("processing_")){
			newFiles << fileInfo.absoluteFilePath();
		//}
	}

	return newFiles;
}

/**
 * @brief 将ControlPoint.xml 中的对应内容复制回PSTimestamp.xml
 * @param projectFolder 项目文件夹路径
 * @return 成功返回true，失败返回false
 */
bool PublicFunctions::copyControlPointToPSTimestamp(const QString& projectDir)
{
	// 1. 读取ControlPoint.xml 中的标记数据 
	QDomDocument controlPointDoc;
	QFile controlPointFile(projectDir + "ControlPoint.xml");
	if (!controlPointFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open ControlPoint.xml";
		return false;
	}

	if (!controlPointDoc.setContent(&controlPointFile)) {
		qDebug() << "Failed to parse ControlPoint.xml";
		controlPointFile.close();
		return false;
	}
	controlPointFile.close();

	// 2. 获取标记节点数据 
	QDomElement controlPointRoot = controlPointDoc.documentElement();
	QString satTiePointTime, ctlPointTime, dataModelTime,
		FNAModelmarkerTime, CTNModelmarkerTime,
		INAModelmarkerTime, FUModelmarkerTime;

	QDomElement satTiePointElem = controlPointRoot.firstChildElement("SatTiePointMatch");
	if (!satTiePointElem.isNull()) {
		satTiePointTime = satTiePointElem.text();
	}

	QDomElement ctlPointElem = controlPointRoot.firstChildElement("CtlPointMatch");
	if (!ctlPointElem.isNull()) {
		ctlPointTime = ctlPointElem.text();
	}

	QDomElement dataModelElem = controlPointRoot.firstChildElement("DataModelmarker");
	if (!dataModelElem.isNull()) {
		dataModelTime = dataModelElem.text();
	}

	QDomElement FNAModelmarkerElem = controlPointRoot.firstChildElement("FNAModelmarker");
	if (!FNAModelmarkerElem.isNull()) {
		FNAModelmarkerTime = FNAModelmarkerElem.text();
	}

	QDomElement CTNModelmarkerElem = controlPointRoot.firstChildElement("CTNModelmarker");
	if (!CTNModelmarkerElem.isNull()) {
		CTNModelmarkerTime = CTNModelmarkerElem.text();
	}

	QDomElement INAModelmarkerElem = controlPointRoot.firstChildElement("INAModelmarker");
	if (!INAModelmarkerElem.isNull()) {
		INAModelmarkerTime = INAModelmarkerElem.text();
	}

	QDomElement FUModelmarkerElem = controlPointRoot.firstChildElement("FUModelmarker");
	if (!FUModelmarkerElem.isNull()) {
		FUModelmarkerTime = FUModelmarkerElem.text();
	}

	// 3. 读取现有的PSTimestamp.xml 内容（保留其他节点）
	QDomDocument timestampDoc;
	QFile timestampFile(projectDir + "PSTimestamp.xml");
	if (!timestampFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open PSTimestamp.xml  for reading";
		return false;
	}

	if (!timestampDoc.setContent(&timestampFile)) {
		qDebug() << "Failed to parse PSTimestamp.xml";
		timestampFile.close();
		return false;
	}
	timestampFile.close();

	// 4. 更新PSTimestamp.xml 中的对应节点 
	QDomElement timestampRoot = timestampDoc.documentElement();

	// 更新或创建节点 
	updateOrCreateElement(timestampDoc, timestampRoot, "SatTiePointMatch", satTiePointTime);
	updateOrCreateElement(timestampDoc, timestampRoot, "CtlPointMatch", ctlPointTime);
	updateOrCreateElement(timestampDoc, timestampRoot, "DataModelmarker", dataModelTime);
	updateOrCreateElement(timestampDoc, timestampRoot, "FNAModelmarker", FNAModelmarkerTime);
	updateOrCreateElement(timestampDoc, timestampRoot, "CTNModelmarker", CTNModelmarkerTime);
	updateOrCreateElement(timestampDoc, timestampRoot, "INAModelmarker", INAModelmarkerTime);
	updateOrCreateElement(timestampDoc, timestampRoot, "FUModelmarker", FUModelmarkerTime);

	// 5. 写回PSTimestamp.xml  
	if (!timestampFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		qDebug() << "Failed to open PSTimestamp.xml  for writing";
		return false;
	}

	QTextStream out(&timestampFile);
	out.setCodec("UTF-8");
	timestampDoc.save(out, 4);
	timestampFile.close();

	return true;
}

/**
 * @brief 辅助函数：更新或创建XML元素
 * @param doc XML文档对象
 * @param parent 父元素
 * @param tagName 标签名
 * @param text 文本内容
 */
void PublicFunctions::updateOrCreateElement(QDomDocument& doc, QDomElement& parent,
	const QString& tagName, const QString& text)
{
	if (text.isEmpty()) {
		return; // 如果内容为空则不更新 
	}

	QDomElement element = parent.firstChildElement(tagName);
	if (element.isNull()) {
		// 创建新元素 
		element = doc.createElement(tagName);
		element.appendChild(doc.createTextNode(text));
		parent.appendChild(element);
	}
	else {
		// 更新现有元素 
		if (element.firstChild().isText()) {
			element.firstChild().setNodeValue(text);
		}
		else {
			element.appendChild(doc.createTextNode(text));
		}
	}
}

// 写入PSTimestamp.xml
bool PublicFunctions::writeTimestampToXml(const QString& nodeName, const QString& projectDir)
{
	QDateTime currentTime = QDateTime::currentDateTime();
	QString timestampStr = currentTime.toString("yyyy-MM-dd  hh:mm:ss");

	QString filePath = projectDir + "PSTimestamp.xml";
	QFile file(filePath);

	//// 第一次进入时清空并重构文件 
	//static bool firstRun = true;
	//if (firstRun) {
	//	firstRun = false;
	//	if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
	//		QXmlStreamWriter writer(&file);
	//		writer.setAutoFormatting(true);
	//		writer.setCodec("UTF-8");

	//		writer.writeStartDocument("1.0");
	//		writer.writeStartElement("Timestamps");
	//		writer.writeEndElement();  // Timestamps 
	//		writer.writeEndDocument();

	//		file.close();
	//		copyControlPointToPSTimestamp(projectDir);
	//	}
	//}
	static QMap<QString, bool> initializedProjects;

	if (!initializedProjects.contains(projectDir) || !initializedProjects[projectDir]) {
		initializedProjects[projectDir] = true;

		if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
			QXmlStreamWriter writer(&file);
			writer.setAutoFormatting(true);
			writer.setCodec("UTF-8");

			writer.writeStartDocument("1.0");
			writer.writeStartElement("Timestamps");
			writer.writeEndElement();  // Timestamps 
			writer.writeEndDocument();

			file.close();
			copyControlPointToPSTimestamp(projectDir);
		}
	}

	// 读取现有XML内容 
	QDomDocument doc;
	if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		if (!doc.setContent(&file)) {
			qDebug() << "Failed to parse XML file, recreating...";
			doc.clear();
			// 重建文档结构 
			doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
			QDomElement root = doc.createElement("Timestamps");
			doc.appendChild(root);
		}
		file.close();
	}
	else {
		qDebug() << "Failed to open file for reading";
		return false;
	}

	// 处理XML文档 
	QDomElement root = doc.documentElement();
	if (root.isNull() || root.tagName() != "Timestamps") {
		doc.clear();
		doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
		root = doc.createElement("Timestamps");
		doc.appendChild(root);
	}

	// 查找或创建节点 
	QDomElement node = root.firstChildElement(nodeName);
	if (node.isNull()) {
		node = doc.createElement(nodeName);
		root.appendChild(node);
	}

	// 设置节点文本内容 
	QDomText text = node.firstChild().toText();
	if (text.isNull()) {
		text = doc.createTextNode(timestampStr);
		node.appendChild(text);
	}
	else {
		text.setData(timestampStr);
	}

	// 写入格式化后的XML 
	if (file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
		QTextStream out(&file);
		out.setCodec("UTF-8");

		// 设置缩进格式 (4个空格)
		const int indentSize = 4;
		QString xml = doc.toString(indentSize);

		// 修正多余换行 
		xml.replace("\n\n", "\n");
		out << xml;

		file.close();
		return true;
	}

	return false;
}

// 读取PSTimestamp.xml
QDateTime PublicFunctions::readTimestampFromXml(const QString& nodeName, const QString& projectDir)
{
	QFile file(projectDir + "PSTimestamp.xml");
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qDebug() << "Failed to open PSTimestamp.xml";
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

void PublicFunctions::restartApplication()
{
	QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList());
	QCoreApplication::quit();
}

// 验证狗许可证的完整函数 
bool PublicFunctions::validateDogLicense()
{
	QString exePath = QCoreApplication::applicationDirPath();
	QProcess process;
	QProcess processDog;
	QString authCmd;
	QString authCmddir;

#ifdef Q_OS_LINUX 
	authCmddir = "/linux64";
	QString libPath = exePath + "/linux64";
	QString libPath_Mosaic = exePath + "/linux64";
	qputenv("LD_LIBRARY_PATH",
		QByteArray(libPath.toLocal8Bit() + ":"
			+ libPath_Mosaic.toLocal8Bit() +
			":" + qgetenv("LD_LIBRARY_PATH")));
#else 
	authCmddir = "/Software/DsmMatch_64";
	authCmd = exePath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSAuthorizeInquiryCmd.exe");
#endif 

	// 执行验证程序 
	process.start(authCmd, QStringList() << "15");

	// 处理各种验证失败情况 
	auto handleError = [&](const QString& errorMsg) -> bool {
		QMessageBox msgBox(QMessageBox::Critical,
			u8"错误",
			errorMsg,
			QMessageBox::NoButton,
			nullptr);
		msgBox.setIcon(QMessageBox::Critical);
		QPushButton *remoteUpgradeBtn = msgBox.addButton(u8"远程许可升级(hasp狗)", QMessageBox::ActionRole);
		QPushButton *restartBtn = msgBox.addButton(u8"重新启动", QMessageBox::ActionRole);
		QPushButton *cancelBtn = msgBox.addButton(u8"取消", QMessageBox::RejectRole);
		msgBox.setDefaultButton(restartBtn);

		msgBox.exec();

		if (msgBox.clickedButton() == remoteUpgradeBtn) {
			processDog.start(exePath + QString::fromLocal8Bit("/Software/RUS_HASP_IMAGEPS.exe"));
			if (!processDog.waitForStarted(3000)) {
				QMessageBox::critical(nullptr, u8"错误", u8"无法启动远程升级程序");
			}
			processDog.waitForFinished();
		}
		else if (msgBox.clickedButton() == restartBtn) {
			restartApplication();
		}
		return false;
	};

	// 等待进程完成 
	if (!process.waitForFinished(30000)) {
		return handleError(u8"许可证检查进程执行超时");
	}

	// 检查进程退出状态 
	if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
		return handleError(u8"许可证检查进程执行失败");
	}

	// 检查许可证文件 
	QFile file(exePath + authCmddir + QString::fromLocal8Bit("/XQAuthorize.lic"));
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return handleError(u8"无法打开许可证文件");
	}

	// 解析许可证文件内容 
	bool hasSuperDog = false, hasHaspDog = false;
	int superDogValue = 0, haspDogValue = 0;
	QTextStream in(&file);

	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.isEmpty()) continue;

		QStringList parts = line.split(':');
		if (parts.size() != 2) continue;

		QString key = parts[0].trimmed();
		QString valueStr = parts[1].trimmed();
		bool ok;
		int value = valueStr.toInt(&ok);

		if (ok && (value == 0 || value == 1)) {
			if (key == "SUPERDOG") {
				superDogValue = value;
				hasSuperDog = true;
			}
			else if (key == "HASPDOG") {
				haspDogValue = value;
				hasHaspDog = true;
			}
		}
	}
	file.close();

	// 验证许可证内容 
	if (!hasSuperDog || !hasHaspDog) {
		return handleError(u8"无效的许可证格式");
	}

	if (!(superDogValue == 1 || haspDogValue == 1)) {
		return handleError(u8"无效的许可证格式");
	}

	return true;
}