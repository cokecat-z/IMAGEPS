#include "DataModelConfig.h"
#include "IMAGEPS.h"

DataModelConfig::DataModelConfig(IMAGEPS* parentImagePS, QWidget* parent)
	: QDialog(parent)
	, m_imagePS(parentImagePS)
{
	ui.setupUi(this);

	if (!m_imagePS && parent) {
		m_imagePS = qobject_cast<IMAGEPS*>(parent);
	}
	this->setWindowTitle(QString::fromLocal8Bit("模型创建"));
	//this->resize(1300, 1100);
	//this->setMinimumSize(800, 600);

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect screenGeometry = screen->geometry();
	int x = (screenGeometry.width() - this->width()) / 2;
	int y = (screenGeometry.height() - this->height()) / 2;
	this->move(x, y);
	this->installEventFilter(this);
	connects();
	
	//loadFile();
	//
	initWidget();
}

void DataModelConfig::connects()
{
	connect(ui.pushButton, &QPushButton::clicked, this, &DataModelConfig::BuildmodelSlot);

	//connect(ui.pushButton, &QPushButton::clicked, this, [=]() {
	//	QStringList filenamelist = QStringList();
	//	BuildmodelSlot(filenamelist); 
	//});
	connect(ui.pushButton_2, &QPushButton::clicked, this, &DataModelConfig::deletemodelSlot);
	//connect(ui.pushButton_3, &QPushButton::clicked, this, &DataModelConfig::okmodelSlot);
	connect(ui.pushButton_3, &QPushButton::clicked, this, &QWidget::close);

	connect(m_imagePS, &IMAGEPS::aboutToClose, this,
		[=]() {
		this->close();
	});
}

void DataModelConfig::loadFile(QStringList filenamelist)
{
	for (const QString &filename : filenamelist) {
		if (!DataModelfilenames.contains(filename)) {
			DataModelfilenames.insert(filename, false);
			DataModelBool = false;
			LoadDataModelBool = true;
		}
	}

	if (!DataModelfilenames.isEmpty() && LoadDataModelBool)
	{
		LoadDataModelBool = false;
		bool state = true;
		for (auto data = DataModelfilenames.begin(); data != DataModelfilenames.end(); ++data)
		{
			if (!data.value())
			{
				QFileInfo fileInfo(data.key());
				QString baseName = fileInfo.completeBaseName();

				// 检查是否已在ui.tableWidget 中存在 
				bool existsInTable1 = false;
				for (int i = 0; i < ui.tableWidget->rowCount(); i++)
				{
					if (fileInfo.fileName().contains(ui.tableWidget->item(i, 1)->text()))
					{
						existsInTable1 = true;
						break;
					}
				}
				if (existsInTable1)
				{
					state = true;
					continue;
				}

				//检查ui.tableWidget_2 的第3列和第7列
				bool existsInTable2 = false;
				for (int i = 0; i < ui.tableWidget_2->rowCount(); i++)
				{
					QTableWidgetItem* col3Item = ui.tableWidget_2->item(i, 2); // 第3列(索引2)
					QTableWidgetItem* col4Item = ui.tableWidget_2->item(i, 4); // 第3列(索引4)
					QTableWidgetItem* col5Item = ui.tableWidget_2->item(i, 5); // 第3列(索引5)
					QTableWidgetItem* col7Item = ui.tableWidget_2->item(i, 6); // 第7列(索引6)

					if ((col3Item && col3Item->text() == baseName) ||
						(col4Item && col4Item->text() == baseName) ||
						(col5Item && col5Item->text() == baseName) ||
						(col7Item && col7Item->text() == baseName))
					{
						existsInTable2 = true;
						break;
					}
				}

				if (existsInTable2)
				{
					state = true;
					continue;
				}

				// 插入新行
				int newRowIndex = ui.tableWidget->rowCount();
				ui.tableWidget->insertRow(newRowIndex);

				// 确保所有单元格都有 QTableWidgetItem
				for (int col = 0; col < ui.tableWidget->columnCount(); col++)
				{
					if (!ui.tableWidget->item(newRowIndex, col))
					{
						QTableWidgetItem* item = new QTableWidgetItem;
						item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
						ui.tableWidget->setItem(newRowIndex, col, item);
					}
				}
				// 设置序号（当前行号 + 1）
				ui.tableWidget->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

				// 设置文件名（去掉扩展名）
				ui.tableWidget->item(newRowIndex, 1)->setText(baseName);
				data.value() = true;
				ui.tableWidget->resizeColumnsToContents();
			}
		}
	}
}

void DataModelConfig::initWidget()
{
	ui.checkBox_3->setEnabled(false);
	ui.tableWidget->verticalHeader()->setVisible(false); // 隐藏行号  
	ui.tableWidget_2->verticalHeader()->setVisible(false); // 隐藏行号  
	ui.tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget_2->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
	ui.tableWidget_2->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents); //完整显示
	ui.tableWidget_2->horizontalHeader()->setSectionResizeMode(6, QHeaderView::ResizeToContents);
	ui.tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui.tableWidget_2->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//最后一列强制拉伸
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	ui.tableWidget_2->horizontalHeader()->setStretchLastSection(true);
	DataModelBool = true;
	LoadDataModelBool = false;
}

void DataModelConfig::setProjectdir() {
	if (m_imagePS) {
		projectdir = m_imagePS->projectdir;
	}
}

bool DataModelConfig::removeFileIfExists(const QString &filePath) {
	QFileInfo fi(filePath);
	if (fi.exists()) {
		QFile file(filePath);
		if (!file.remove()) {
			PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("删除文件失败! ") + filePath);
			return false;
		}
		PROJECT_LOG_WARNING(m_imagePS->CurrentConfig, QString::fromLocal8Bit("删除文件成功! ") + filePath);
	}
	return true;
}

QString DataModelConfig::BuildSatModelMakerCmdXML(QStringList filenamelist, const QString outdir)
{
	setProjectdir();
	bool threedimensionalModel = ui.checkBox->isChecked();
	bool multispectralModel = ui.checkBox_2->isChecked();

	// 创建 QDomDocument
	QDomDocument doc;

	// 添加 XML 声明
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
	doc.appendChild(instruction);

	// 创建根节点 <SatImages>
	QDomElement root = doc.createElement("XQSatImages");
	//root.setAttribute("version", "1.0");
	//doc.appendChild(root);

	// 创建 <InputFiles> 子节点
	QDomElement InputFiles = doc.createElement("InputFiles");

	if (filenamelist.isEmpty())
	{
		for (auto data = DataModelfilenames.begin(); data != DataModelfilenames.end(); ++data)
		{
			if (data.value())
			{
				filenamelist.push_back(data.key());
			}
		}
	}

	for (auto data : filenamelist)
	{
			// 创建 <InputImageFilePath> 子节点
			QDomElement InputFilesPath = doc.createElement("ImageFile");
			//inputImagePath.setAttribute("type", "string");
			QString tmp = data;
			QDomText inputImagePathText = doc.createTextNode(tmp);
			InputFilesPath.appendChild(inputImagePathText);
			InputFiles.appendChild(InputFilesPath);
	}
	root.appendChild(InputFiles);

	// 创建 <OutputFile> 子节点
	QDomElement Outputfile = doc.createElement("OutputFile");
	QDomText OutputfileText = doc.createTextNode(projectdir + outdir + "/" + "SatModelMakerCmdout.xml");
	Outputfile.appendChild(OutputfileText);
	root.appendChild(Outputfile);

	// 创建 <Parameters> 子节点
	QDomElement Parameters = doc.createElement("Parameters");
	// 创建 <StereoModel> 子节点
	QDomElement StereoModel = doc.createElement("StereoModel");
	StereoModel.setAttribute("type", "int32");
	QDomText StereoModelText = doc.createTextNode(QString::number(threedimensionalModel));
	StereoModel.appendChild(StereoModelText);
	Parameters.appendChild(StereoModel);

	// 添加说明注释   
	QDomComment comment1 = doc.createComment(QString::fromLocal8Bit("false为单片，true为立体"));
	Parameters.appendChild(comment1);

	// 创建 <MuxAsSingleModel> 子节点
	QDomElement MuxAsSingleModel = doc.createElement("MuxAsSingleModel");
	MuxAsSingleModel.setAttribute("type", "int32");
	QDomText MuxAsSingleModelText = doc.createTextNode(QString::number(multispectralModel));
	MuxAsSingleModel.appendChild(MuxAsSingleModelText);
	Parameters.appendChild(MuxAsSingleModel);

	// 添加说明注释   
	QDomComment comment2 = doc.createComment(QString::fromLocal8Bit("多光谱作为独立模型,也就是不和全色配对"));
	Parameters.appendChild(comment2);

	QDomElement MakeModelByKeyWord = doc.createElement("MakeModelByKeyWord");
	MakeModelByKeyWord.setAttribute("type", "int32");
	MakeModelByKeyWord.appendChild(doc.createTextNode("0"));
	Parameters.appendChild(MakeModelByKeyWord);

	QDomElement PanImageKeyWord = doc.createElement("PanImageKeyWord");
	PanImageKeyWord.appendChild(doc.createTextNode("PAN"));
	Parameters.appendChild(PanImageKeyWord);

	QDomElement MuxImageKeyWord = doc.createElement("MuxImageKeyWord");
	MuxImageKeyWord.appendChild(doc.createTextNode("MSS"));
	Parameters.appendChild(MuxImageKeyWord);

	root.appendChild(Parameters);

	doc.appendChild(root);

	// 写入文件
	QString xmlPath = projectdir + outdir + "/" + QString::fromLocal8Bit("SatModelMakerCmd.xml");

	QFile file(xmlPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("创建模型配对任务单失败!"));
		return "";
	}

	QTextStream out(&file);
	out.setCodec("UTF-8");
	out << doc.toString(4);  // 4 表示缩进 4 个空格，使 XML 可读性更好
	file.close();

	return xmlPath;
}

void DataModelConfig::BuildmodelSlot()
{
	if (!DataModelBool)
	{
		if (!tryLockFunction(DataModeMutex, QString::fromLocal8Bit("已有模型构建"))) {
			return;
		}

		int index_flag = 0;
		for (auto data = DataModelfilenames.begin(); data != DataModelfilenames.end(); ++data)
		{
			if (data.value())
			{
				index_flag++;
			}
		}
		if (index_flag < 1) {
			DataModeMutex.unlock();
			return;
		}

		QString RSatModelMakerCmdoutPath = projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/" + "SatModelMakerCmdout.xml";

		if (!removeFileIfExists(RSatModelMakerCmdoutPath)) {
			QMessageBox::critical(nullptr,
				QString::fromLocal8Bit("错误"),
				QString::fromLocal8Bit("无法清理SatModelMakerCmdout.xml旧文件，请删除文件后重新操作"));
			PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型创建界面 无法清理SatModelMakerCmdout.xml旧文件，请删除文件后重新操作"));
			return;
		}

		QStringList filenamelist = QStringList();
		QString xmlPath = BuildSatModelMakerCmdXML(filenamelist, "SatTiePointMatch");

		PublicFunctions::writeTimestampToXml(QString::fromLocal8Bit("DataModelmarker"), m_imagePS->projectdir);

		//创建调用可执行程序的类
		QProcess * process = new QProcess(this);
		QString authCmd;
#ifdef Q_OS_LINUX 
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSSatModelMakerCmd.x"));
#else
		authCmd = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSSatModelMakerCmd.exe"));
#endif
		//启动程序，并输入参数
		process->start(authCmd, QStringList() << xmlPath);
		PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型创建界面 模型配对任务开始执行!"));

		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[=](int exitCode, QProcess::ExitStatus status) {
			
			process->deleteLater(); // 安全释放内存
			PROJECT_LOG_INFO(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型创建界面 模型配对任务执行结束!"));
			//QDateTime timestamp = PublicFunctions::readTimestampFromXml("DataModelmarker");
			//if (!timestamp.isValid()) {
			//	DataModeMutex.unlock();
			//	return;
			//}

			//QStringList resultFiles = PublicFunctions::getNewlyCreatedFiles(projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/", timestamp);
			//QString tmpPath = projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/" + "SatModelMakerCmdout.xml";
			QString SatModelMakerCmdoutPath = projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/" + "SatModelMakerCmdout.xml";

			/*QString SatModelMakerCmdoutPath = "";
			for (auto datafile : resultFiles)
			{
				if (tmpPath == datafile)
					SatModelMakerCmdoutPath = tmpPath;
			}
			if (SatModelMakerCmdoutPath == "")
			{
				QMessageBox::warning(this, u8"警告", QString::fromLocal8Bit("打开SatModelMakerCmdout.xml文件失败！"));
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("模型创建界面 没有读取到SatModelMakerCmdout.xml文件"));
				DataModeMutex.unlock();
				return;
			}*/

			DataModelBool = true;
			QFile file(SatModelMakerCmdoutPath);
			if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("打开SatModelMakerCmdout.xml文件失败"));
				QMessageBox::critical(nullptr, u8"错误", u8"打开SatModelMakerCmdout.xml文件失败 " + file.errorString());
				DataModeMutex.unlock();
				DataModelBool = false;
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

						// 插入新行
						int newRowIndex = ui.tableWidget_2->rowCount();
						ui.tableWidget_2->insertRow(newRowIndex);

						// 确保所有单元格都有 QTableWidgetItem
						for (int col = 0; col < ui.tableWidget_2->columnCount(); col++)
						{
							if (!ui.tableWidget_2->item(newRowIndex, col))
							{
								QTableWidgetItem* item = new QTableWidgetItem;
								item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
								ui.tableWidget_2->setItem(newRowIndex, col, item);
							}
						}
						// 设置基准影像和多光谱影像文件名 
						QFileInfo fileInfoNadImageName(modelData[QString::fromLocal8Bit("NadImageName")]);
						QString nadBaseName = fileInfoNadImageName.completeBaseName();
						QFileInfo fileInfoMulImageName(modelData[QString::fromLocal8Bit("MulImageName")]);
						QString mulBaseName = fileInfoMulImageName.completeBaseName();
						QFileInfo fileInfofwdImageName(modelData[QString::fromLocal8Bit("FwdImageName")]);
						QString fwdBaseName = fileInfofwdImageName.completeBaseName();
						QFileInfo fileInfobwdImageName(modelData[QString::fromLocal8Bit("BwdImageName")]);
						QString bwdBaseName = fileInfobwdImageName.completeBaseName();

						// 检查第2列和第6列是否有重复 
						bool isDuplicate = false;
						for (int row = 0; row < ui.tableWidget_2->rowCount(); ++row) {
							QString existingNad = ui.tableWidget_2->item(row, 2) ? ui.tableWidget_2->item(row, 2)->text() : "";
							QString existingMul = ui.tableWidget_2->item(row, 6) ? ui.tableWidget_2->item(row, 6)->text() : "";
							QString existingFwd = ui.tableWidget_2->item(row, 4) ? ui.tableWidget_2->item(row, 4)->text() : "";
							QString existingBwd = ui.tableWidget_2->item(row, 5) ? ui.tableWidget_2->item(row, 5)->text() : "";

							if (existingNad == nadBaseName && existingMul == mulBaseName && existingFwd == fwdBaseName && existingBwd == bwdBaseName) {
								isDuplicate = true;
								break;
							}
						}

						// 只有不重复时才添加数据 
						if (!isDuplicate) {
							// 设置序号（当前行号 + 1）
							ui.tableWidget_2->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

							// 设置模型号 
							ui.tableWidget_2->item(newRowIndex, 1)->setText(modelData[QString::fromLocal8Bit("ModelID")]);

							ui.tableWidget_2->item(newRowIndex, 3)->setText(QString::fromLocal8Bit("否"));

							// 设置基准影像和多光谱影像文件名 
							ui.tableWidget_2->item(newRowIndex, 2)->setText(nadBaseName);
							ui.tableWidget_2->item(newRowIndex, 6)->setText(mulBaseName);
							ui.tableWidget_2->item(newRowIndex, 4)->setText(fwdBaseName);
							ui.tableWidget_2->item(newRowIndex, 5)->setText(bwdBaseName);

							// 查找文件 
							auto itnad = DataModelfilenames.find(modelData[QString::fromLocal8Bit("NadImageName")]);
							if (itnad != DataModelfilenames.end()) {
								itnad.value() = false;
							}

							auto itmul = DataModelfilenames.find(modelData[QString::fromLocal8Bit("MulImageName")]);
							if (itmul != DataModelfilenames.end()) {
								itmul.value() = false;
							}

							auto itfwd = DataModelfilenames.find(modelData[QString::fromLocal8Bit("FwdImageName")]);
							if (itfwd != DataModelfilenames.end()) {
								itfwd.value() = false;
							}

							auto itbwd = DataModelfilenames.find(modelData[QString::fromLocal8Bit("BwdImageName")]);
							if (itbwd != DataModelfilenames.end()) {
								itbwd.value() = false;
							}

							// 查找所有匹配的项并删除对应行 
							QList<QTableWidgetItem*> itemsNad = ui.tableWidget->findItems(nadBaseName, Qt::MatchExactly);
							foreach(QTableWidgetItem* item, itemsNad) {
								if (item->column() == 1) {
									ui.tableWidget->removeRow(item->row());
								}
							}

							QList<QTableWidgetItem*> itemsMul = ui.tableWidget->findItems(mulBaseName, Qt::MatchExactly);
							foreach(QTableWidgetItem* item, itemsMul) {
								if (item->column() == 1) {
									ui.tableWidget->removeRow(item->row());
								}
							}

							QList<QTableWidgetItem*> itemsFwd = ui.tableWidget->findItems(fwdBaseName, Qt::MatchExactly);
							foreach(QTableWidgetItem* item, itemsFwd) {
								if (item->column() == 1) {
									ui.tableWidget->removeRow(item->row());
								}
							}

							QList<QTableWidgetItem*> itemsBwd = ui.tableWidget->findItems(bwdBaseName, Qt::MatchExactly);
							foreach(QTableWidgetItem* item, itemsBwd) {
								if (item->column() == 1) {
									ui.tableWidget->removeRow(item->row());
								}
							}
						}
						else {
							// 如果是重复数据，移除刚添加的行 
							ui.tableWidget_2->removeRow(newRowIndex);
						}
						ui.tableWidget_2->resizeColumnsToContents();
					}
				}
			}

			if (xml.hasError()) {
				PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("SatModelMakerCmdout.xml文件格式有误!"));
				QMessageBox::critical(nullptr, u8"SatModelMakerCmdout.xml文件格式有误!", xml.errorString());
			}

			file.close();
			DataModeMutex.unlock();

		});
	}
}

void DataModelConfig::deletemodelSlot()
{
	QList<QTableWidgetItem*> selectedItems = ui.tableWidget_2->selectedItems();
	if (selectedItems.isEmpty()) {
		QMessageBox::warning(this, u8"警告", QString::fromLocal8Bit("未选中要删除的模型!"));
		return;
	}
	QSet<int> rowsToProcess;
	for (QTableWidgetItem* item : selectedItems) {
		rowsToProcess.insert(item->row());
	}
	QList<int> rows = rowsToProcess.values();
	std::sort(rows.begin(), rows.end(), std::greater<int>());
	for (int row : rows) {
		QTableWidgetItem* thirdColumnItem = ui.tableWidget_2->item(row, 3);
		if (!thirdColumnItem) {
			continue; // 跳过空单元格
		}
		DataModelBool = false;
		QString value = thirdColumnItem->text().trimmed();
		if (value == QString::fromLocal8Bit("否")) {
			for (auto data = DataModelfilenames.begin(); data != DataModelfilenames.end(); ++data)
			{
				QString tmpnad = ui.tableWidget_2->item(row, 2)->text().trimmed();
				QString tmpmul = ui.tableWidget_2->item(row, 6)->text().trimmed();
				QString tmpfwd = ui.tableWidget_2->item(row, 4)->text().trimmed();
				QString tmpbwd = ui.tableWidget_2->item(row, 5)->text().trimmed();
				QFileInfo fileInfodata(data.key());
				if (fileInfodata.completeBaseName() == tmpnad || fileInfodata.completeBaseName() == tmpmul || fileInfodata.completeBaseName() == tmpfwd || fileInfodata.completeBaseName() == tmpbwd)
				{
					bool state = true;

					QFileInfo fileInfo(data.key());
					for (int i = 0; i < ui.tableWidget->rowCount(); i++)
					{
						if (fileInfo.fileName().contains(ui.tableWidget->item(i, 1)->text()))
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
					int newRowIndex = ui.tableWidget->rowCount();
					ui.tableWidget->insertRow(newRowIndex);

					// 确保所有单元格都有 QTableWidgetItem
					for (int col = 0; col < ui.tableWidget->columnCount(); col++)
					{
						if (!ui.tableWidget->item(newRowIndex, col))
						{
							QTableWidgetItem* item = new QTableWidgetItem;
							item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
							ui.tableWidget->setItem(newRowIndex, col, item);
						}
					}
					// 设置序号（当前行号 + 1）
					ui.tableWidget->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

					// 设置文件名（去掉扩展名）
					ui.tableWidget->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

					data.value() = true;
					ui.tableWidget->resizeColumnsToContents();
				}
					//data.value() = false;
			}
			ui.tableWidget_2->removeRow(row);
		}
		else {
			/*QString message = QString::fromLocal8Bit("当前选择模型存在已匹配成果数据，是否依然删除？").arg(row + 1);
			if (QMessageBox::question(this, QString::fromLocal8Bit("删除模型"), message,
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {*/
				for (auto data = DataModelfilenames.begin(); data != DataModelfilenames.end(); ++data)
				{
					QString tmpnad = ui.tableWidget_2->item(row, 2)->text().trimmed();
					QString tmpmul = ui.tableWidget_2->item(row, 6)->text().trimmed();
					QString tmpfwd = ui.tableWidget_2->item(row, 4)->text().trimmed();
					QString tmpbwd = ui.tableWidget_2->item(row, 5)->text().trimmed();
					QFileInfo fileInfodata(data.key());
					if (fileInfodata.completeBaseName() == tmpnad || fileInfodata.completeBaseName() == tmpmul || fileInfodata.completeBaseName() == tmpfwd || fileInfodata.completeBaseName() == tmpbwd)
					{
						bool state = true;

						QFileInfo fileInfo(data.key());
						for (int i = 0; i < ui.tableWidget->rowCount(); i++)
						{
							if (fileInfo.fileName().contains(ui.tableWidget->item(i, 1)->text()))
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
						int newRowIndex = ui.tableWidget->rowCount();
						ui.tableWidget->insertRow(newRowIndex);

						// 确保所有单元格都有 QTableWidgetItem
						for (int col = 0; col < ui.tableWidget->columnCount(); col++)
						{
							if (!ui.tableWidget->item(newRowIndex, col))
							{
								QTableWidgetItem* item = new QTableWidgetItem;
								item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
								ui.tableWidget->setItem(newRowIndex, col, item);
							}
						}
						// 设置序号（当前行号 + 1）
						ui.tableWidget->item(newRowIndex, 0)->setText(QString::number(newRowIndex + 1));

						// 设置文件名（去掉扩展名）
						ui.tableWidget->item(newRowIndex, 1)->setText(fileInfo.completeBaseName());

						data.value() = true;
						ui.tableWidget->resizeColumnsToContents();
					}
						//data.value() = false;
				}
				ui.tableWidget_2->removeRow(row);
			//}
		}
	}

	for (int i = 0; i < ui.tableWidget_2->rowCount(); ++i) {
		// 确保第一列存在 QTableWidgetItem
		QTableWidgetItem* item = ui.tableWidget_2->item(i, 0);
		if (!item) {
			item = new QTableWidgetItem();
			item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
			ui.tableWidget_2->setItem(i, 0, item);
		}
		// 设置序号为当前行号 + 1
		item->setText(QString::number(i + 1));
	}
}

void DataModelConfig::okmodelSlot()
{
	//connect(ui.pushButton_3, &QPushButton::clicked, this, &QWidget::close);
}

/**
 * @brief 将SatModelMakerCmdout.xml 中的SatImageModels标签内容复制到TPMatchModels中
 *        并添加Matched、MulMatched和JoinMatch标签
 * @return bool 成功返回true，失败返回false
 */
bool DataModelConfig::writeMatchModelFromSatModelMakerCmdout()
{
	setProjectdir();
	// 1. 读取SatModelMakerCmdout.xml  文件 
	QFile satFile(projectdir + QString::fromLocal8Bit("SatTiePointMatch") + "/" + "SatModelMakerCmdout.xml");
	if (!satFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("打开SatModelMakerCmdout.xml文件失败"));

		//qDebug() << "Failed to open SatModelMakerCmdout.xml";
		return false;
	}

	QDomDocument satDoc;
	if (!satDoc.setContent(&satFile)) {
		satFile.close();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("解析SatModelMakerCmdout.xml文件失败"));

		//qDebug() << "Failed to parse SatModelMakerCmdout.xml";
		return false;
	}
	satFile.close();

	// 2. 创建新的MatchModel.xml  文件结构 
	QDomDocument matchDoc;
	QFile matchFile(projectdir + "MatchModel.xml");

	// 创建基本结构 
	QDomProcessingInstruction instruction = matchDoc.createProcessingInstruction(
		"xml", "version=\"1.0\" encoding=\"UTF-8\"");
	matchDoc.appendChild(instruction);

	QDomElement root = matchDoc.createElement("TPMatchModels");
	matchDoc.appendChild(root);

	// 3. 获取SatImageModels节点 
	QDomElement satRoot = satDoc.documentElement();

	// 3.1 复制ModelNum 
	QDomElement satModelNum = satRoot.firstChildElement("ModelNum");
	if (!satModelNum.isNull()) {
		QDomElement modelNum = matchDoc.createElement("ModelNum");
		modelNum.setAttribute("type", satModelNum.attribute("type", "int32"));
		modelNum.appendChild(matchDoc.createTextNode(satModelNum.text()));
		root.appendChild(modelNum);
	}
	else {
		// 如果源文件中没有ModelNum，创建默认值 
		QDomElement modelNum = matchDoc.createElement("ModelNum");
		modelNum.setAttribute("type", "int32");
		modelNum.appendChild(matchDoc.createTextNode("0"));
		root.appendChild(modelNum);
	}

	QDomElement satModels = satRoot.firstChildElement("Models");
	if (satModels.isNull()) {
		//qDebug() << "No Models element in SatModelMakerCmdout.xml";
		return false;
	}

	// 4. 创建Models节点 
	QDomElement matchModels = matchDoc.createElement("Models");
	root.appendChild(matchModels);

	// 5. 复制所有Model节点并添加新标签 
	QDomNodeList modelList = satModels.childNodes();
	int modelCount = 0;

	for (int i = 0; i < modelList.size(); ++i) {
		QDomNode node = modelList.at(i);
		if (!node.isElement())  continue;

		QDomElement model = node.toElement();
		if (model.tagName().startsWith("Model_")) {
			// 创建新的Model节点 
			QString modelName = QString("Model_%1").arg(modelCount);
			QDomElement newModel = matchDoc.createElement(modelName);

			// 复制原有属性 
			QDomNamedNodeMap attributes = model.attributes();
			for (int j = 0; j < attributes.size(); ++j) {
				QDomAttr attr = attributes.item(j).toAttr();
				newModel.setAttribute(attr.name(), attr.value());
			}

			// 复制原有子节点 
			QDomNodeList childNodes = model.childNodes();
			for (int j = 0; j < childNodes.size(); ++j) {
				QDomNode childNode = childNodes.at(j);
				if (childNode.isElement()) {
					QDomElement childElement = childNode.toElement();
					// 跳过不需要复制的节点（如果有）
					newModel.appendChild(childNode.cloneNode(true));
				}
			}

			// 添加Matched标签（从表格第4列获取）
			QDomElement matched = matchDoc.createElement("Matched");
			matched.setAttribute("type", "bool");
			bool isMatched = false;
			if (ui.tableWidget_2  && modelCount < ui.tableWidget_2->rowCount()) {
				QTableWidgetItem* item = ui.tableWidget_2->item(modelCount, 3); // 第4列 
				if (item) {
					isMatched = (item->text() == QString::fromLocal8Bit("是"));
				}
			}
			matched.appendChild(matchDoc.createTextNode(isMatched ? "true" : "false"));
			newModel.appendChild(matched);

			// 添加MulMatched标签（默认false）
			QDomElement mulMatched = matchDoc.createElement("MulMatched");
			mulMatched.setAttribute("type", "bool");
			mulMatched.appendChild(matchDoc.createTextNode("false"));
			newModel.appendChild(mulMatched);

			// 添加JoinMatch标签（默认true）
			QDomElement joinMatch = matchDoc.createElement("JoinMatch");
			joinMatch.setAttribute("type", "bool");
			joinMatch.appendChild(matchDoc.createTextNode("true"));
			newModel.appendChild(joinMatch);

			matchModels.appendChild(newModel);
			modelCount++;
		}
	}

	// 6. 更新ModelNum为实际复制的模型数量 
	QDomElement modelNumToUpdate = root.firstChildElement("ModelNum");
	if (!modelNumToUpdate.isNull()) {
		QDomText numText = matchDoc.createTextNode(QString::number(modelCount));
		if (modelNumToUpdate.firstChild().isNull()) {
			modelNumToUpdate.appendChild(numText);
		}
		else {
			modelNumToUpdate.replaceChild(numText, modelNumToUpdate.firstChild());
		}
	}

	// 7. 保存MatchModel.xml  文件 
	if (!matchFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("保存MatchModel.xml文件失败!"));

		//qDebug() << "Failed to open MatchModel.xml  for writing";
		return false;
	}

	QTextStream out(&matchFile);
	out.setCodec("UTF-8");
	matchDoc.save(out, 4); // 缩进4个空格 
	matchFile.close();

	return true;
}
/**
 * @brief 将MatchModel.xml 中数据读取并显示到tableWidget_2表中
 * @return bool 成功返回true，失败返回false
 */
bool DataModelConfig::loadMatchModelToTable()
{
	if (!ui.tableWidget_2) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("传入无效的ui.tableWidget_2表格指针"));

		//qDebug() << "Invalid table widget";
		return false;
	}
	setProjectdir();

	// 1. 读取MatchModel.xml 文件 
	QFile matchFile(projectdir + "MatchModel.xml");
	if (!matchFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("打开MatchModel.xml文件失败"));

		//qDebug() << "Failed to open MatchModel.xml";
		return false;
	}

	QDomDocument matchDoc;
	if (!matchDoc.setContent(&matchFile)) {
		matchFile.close();
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("解析MatchModel.xml文件失败"));

		//qDebug() << "Failed to parse MatchModel.xml";
		return false;
	}
	matchFile.close();

	// 2. 清空表格内容（保持列名不变）
	ui.tableWidget_2->clearContents();
	ui.tableWidget_2->setRowCount(0);

	// 3. 获取Models节点 
	QDomElement root = matchDoc.documentElement();
	QDomElement models = root.firstChildElement("Models");
	if (models.isNull()) {
		PROJECT_LOG_ERROR(m_imagePS->CurrentConfig, QString::fromLocal8Bit("MatchModel.xml文件中没有模型数据"));

		//qDebug() << "No Models element in MatchModel.xml";
		return true; // 没有数据不算错误 
	}

	// 4. 遍历所有Model节点 
	QDomNodeList modelList = models.childNodes();
	for (int i = 0; i < modelList.size(); ++i) {
		QDomNode node = modelList.at(i);
		if (!node.isElement() || !node.nodeName().startsWith("Model_")) {
			continue;
		}

		QDomElement model = node.toElement();

		// 添加新行 
		int row = ui.tableWidget_2->rowCount();
		ui.tableWidget_2->insertRow(row);

		// 第1列：序号（行号）
		QTableWidgetItem* indexItem = new QTableWidgetItem(QString::number(row + 1));
		indexItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_2->setItem(row, 0, indexItem);

		// 第2列：模型号（ModelID）
		QDomElement modelId = model.firstChildElement("ModelID");
		QString modelIdText = modelId.isNull() ? "" : modelId.text();
		QTableWidgetItem* modelIdItem = new QTableWidgetItem(modelIdText);
		modelIdItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_2->setItem(row, 1, modelIdItem);

		// 第3列：基准（NadImageName的文件名部分）
		QDomElement nadImageName = model.firstChildElement("NadImageName");
		QString nadFileName = "";
		if (!nadImageName.isNull()) {
			QString nadImagePath = nadImageName.text();
			nadFileName = QFileInfo(nadImagePath).completeBaseName();
		}
		QTableWidgetItem* nadItem = new QTableWidgetItem(nadFileName);
		nadItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_2->setItem(row, 2, nadItem);

		// 第4列：Matched的值（true->是，false->否）
		QDomElement matched = model.firstChildElement("Matched");
		QString matchedText = matched.isNull() ? "false" : matched.text();
		QString matchedDisplay = (matchedText.compare("true", Qt::CaseInsensitive) == 0) ?
			QString::fromLocal8Bit("是") : QString::fromLocal8Bit("否");
		QTableWidgetItem* matchedItem = new QTableWidgetItem(matchedDisplay);
		matchedItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_2->setItem(row, 3, matchedItem);

		// 第5列：基准（FwdImageName的文件名部分）
		QDomElement fwdImageName = model.firstChildElement("FwdImageName");
		QString fwdFileName = "";
		if (!fwdImageName.isNull()) {
			QString fwdImagePath = fwdImageName.text();
			fwdFileName = QFileInfo(fwdImagePath).completeBaseName();
		}
		QTableWidgetItem* fwdItem = new QTableWidgetItem(fwdFileName);
		fwdItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_2->setItem(row, 4, fwdItem);

		// 第6列：基准（BwdImageName的文件名部分）
		QDomElement bwdImageName = model.firstChildElement("BwdImageName");
		QString bwdFileName = "";
		if (!bwdImageName.isNull()) {
			QString bwdImagePath = bwdImageName.text();
			bwdFileName = QFileInfo(bwdImagePath).completeBaseName();
		}
		QTableWidgetItem* bwdItem = new QTableWidgetItem(bwdFileName);
		bwdItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_2->setItem(row, 5, bwdItem);

		// 第7列：MulImageName的文件名部分 
		QDomElement mulImageName = model.firstChildElement("MulImageName");
		QString mulFileName = "";
		if (!mulImageName.isNull()) {
			QString mulImagePath = mulImageName.text();
			mulFileName = QFileInfo(mulImagePath).completeBaseName();
		}
		QTableWidgetItem* mulItem = new QTableWidgetItem(mulFileName);
		mulItem->setTextAlignment(Qt::AlignCenter);
		ui.tableWidget_2->setItem(row, 6, mulItem);
	}

	// 自动调整列宽（可选）
	ui.tableWidget_2->resizeColumnsToContents();

	return true;
}

// 事件过滤器
bool DataModelConfig::eventFilter(QObject *watched, QEvent *event) {

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

	if (event->type() == QEvent::MouseButtonPress) {
		// 清除所有 QTableWidget 的选中状态
		QList<QTableWidget*> tableWidgets = findChildren<QTableWidget*>();
		for (QTableWidget* tableWidget : tableWidgets) {
			tableWidget->clearSelection();
		}
	}

	return QDialog::eventFilter(watched, event);
}

DataModelConfig::~DataModelConfig()
{}
