#include "SimpleToolS.h"
#include "IMAGEPS.h"

SimpleToolS::SimpleToolS(IMAGEPS* parentImagePS, QWidget* parent)
	: QDialog(parent)
	, m_imagePS(parentImagePS)
{
	ui.setupUi(this);

	if (!m_imagePS && parent) {
		m_imagePS = qobject_cast<IMAGEPS*>(parent);
	}

	if (!systemConfig)
	{
		//systemConfig = new SystemConfig;
	}

	m_ExeName = "";
	initWidget();
	connects();
}

SimpleToolS::~SimpleToolS()
{}

void SimpleToolS::initWidget()
{
	//setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 移除帮助按钮
}

void SimpleToolS::connects()
{
	bindFileDialog(
		ui.toolButton_23,
		ui.lineEdit_33,
		nullptr,   
		tr("%1;;TIF Files(*.tif);;TIFF Files(*.tiff);;IMAGINE images Files(*.img);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img)"))
	);

	bindFileDialog(
		ui.toolButton_24,
		ui.lineEdit_34,
		nullptr, 
		tr("%1;;TIF Files(*.tif);;TIFF Files(*.tiff);;IMAGINE images Files(*.img);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img)"))
	);

	bindFileDialog(
		ui.toolButton_25,
		ui.lineEdit_35,
		nullptr,  
		tr("%1;;TIF Files(*.tif);;TIFF Files(*.tiff);;IMAGINE images Files(*.img);;All Files(*)").arg(QString::fromLocal8Bit("所有支持的影像格式(*.tif *.tiff *.img)"))
	);
}

void SimpleToolS::bindFileDialog(
	QToolButton* button,
	QLineEdit* lineEdit,
	QCheckBox* checkBox,
	const QString& filter
) {
	connect(button, &QToolButton::clicked, [=]() {
		QString path;

		// 根据CheckBox状态选择对话框类型 
		if (checkBox && checkBox->isChecked()) {
			// 文件夹选择对话框 
			path = QFileDialog::getExistingDirectory(
				this,
				"选择文件夹",
				"",
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
			);
		}
		else {
			// 文件选择对话框（支持多选）
			//QStringList files = QFileDialog::getOpenFileNames(
			path = QFileDialog::getOpenFileName(
				this,
				"选择文件",
				"",
				filter
			);
			//path = files.join(";");   // 多文件路径用分号分隔 
		}

		// 更新LineEdit
		if (!path.isEmpty()) {
			lineEdit->setText(path);
		}
	});
}

/**
* @brief DEM相关性质检工具
*/
void SimpleToolS::DEMRelatedPropertyCheck()
{
	if (!tryLockFunction(SimModelMutex, QString::fromLocal8Bit("已有DEM相关性质检"))) {
		return;
	}
	//setProjectdir();
	//logEdit->append(QString::fromLocal8Bit("****DEM相关性质检****"));

	// 获取界面参数
	QString InputImageFilePathPS = ui.lineEdit_33->text();
	QString RefImageFilePathPS = ui.lineEdit_34->text();
	QString OutputImageFilePathPS = ui.lineEdit_35->text();
	int coefWidPS = ui.lineEdit_36->text().toInt();
	bool CreatePydPS = ui.checkBox_9->isChecked();

	// 检查是否有任务需要处理 
	if (InputImageFilePathPS.isEmpty()) {
		QMessageBox::warning(this,
			QString::fromLocal8Bit("警告"),
			QString::fromLocal8Bit("请完成路径配置!"));
		SimModelMutex.unlock();
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
	bool isDuplicate = false;

	//QFileInfo fileinfo_in(OutputImageFilePathPS);
	//QString outfilename = fileinfo_in.absolutePath() + "/" + fileinfo.fileName();
	//if (QFile::exists(outfilename)) {
	//	duplicateCount++;
	//	isDuplicate = true;
	//}
	//else {
	//	isDuplicate= false;
	//}

	// 处理重复文件情况 
	bool processAll = true;
	if (isDuplicate) {
		QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
			QString::fromLocal8Bit("路径已存在"),
			QString::fromLocal8Bit("成果文件!").arg(duplicateCount),
			QMessageBox::Yes | QMessageBox::No);

		processAll = (reply == QMessageBox::Yes);

		if (!processAll) {
			SimModelMutex.unlock();
			return;
		}
	}

	// 创建XML文档
	QDomDocument doc;
	QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"");
	doc.appendChild(instruction);

	QDomElement root = doc.createElement("XQImageCorrelatonCmd");
	root.setAttribute("version", "1.0");
	doc.appendChild(root);

	// 添加输入输出文件路径 
	QDomElement inputImageFileName = doc.createElement("InputImageFilePath");
	inputImageFileName.setAttribute("type", "string");
	inputImageFileName.appendChild(doc.createTextNode(InputImageFilePathPS));
	root.appendChild(inputImageFileName);

	QDomElement RefImageFilePath = doc.createElement("RefImageFilePath");
	RefImageFilePath.setAttribute("type", "string");
	RefImageFilePath.appendChild(doc.createTextNode(RefImageFilePathPS));
	root.appendChild(RefImageFilePath);
	
	QDomElement OutputImageFilePath = doc.createElement("OutputImageFilePath");
	OutputImageFilePath.setAttribute("type", "string");
	OutputImageFilePath.appendChild(doc.createTextNode(OutputImageFilePathPS));
	root.appendChild(OutputImageFilePath);
	
	QDomElement coefWid = doc.createElement("coefWid");
	coefWid.setAttribute("type", "string");
	coefWid.appendChild(doc.createTextNode(QString::number(coefWidPS)));
	root.appendChild(coefWid);
	
	QDomElement CreatePyd = doc.createElement("CreatePyd");
	CreatePyd.appendChild(doc.createTextNode(QVariant(CreatePydPS).toString()));
	root.appendChild(CreatePyd);

	QFileInfo fileinfo(OutputImageFilePathPS);
	// 保存XML文件
	QString xmlPath = fileinfo.absolutePath()+ "/" + fileinfo.completeBaseName() + QString::fromLocal8Bit(".xml");
	QFile file(xmlPath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::warning(this,
			QString::fromLocal8Bit("错误"),
			QString::fromLocal8Bit("创建任务单失败!"));
		SimModelMutex.unlock();
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
	task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("linux64/PSImageCorrelatonCmd.x"));
#else
	task.operatorName = QDir(m_imagePS->appDirPath).filePath(QString::fromLocal8Bit("Software/DsmMatch_64/PSImageCorrelatonCmd.exe"));
#endif
	m_taskQueue.enqueue(task);
	m_totalTasks++;

	// 开始处理任务 
	startNextTasks(QString::fromLocal8Bit("DEM相关性质检"), true);
}

// 通用的任务处理函数   
void SimpleToolS::startNextTasks(QString title, bool enableLogging)
{
	//if (m_runningProcesses.load() > 0) {
	//	return; // 避免重复处理 
	//}
	QProgressBar* progressBar = m_ProgressBar;
	// 初始化进度条
	if (!m_taskQueue.isEmpty() && progressBar && !m_progressBarInitialized) {
		progressBar->setRange(0, m_totalTasks);
		progressBar->setValue(0);
		progressBar->setVisible(true);
		//progressBar->show();
		m_progressBarInitialized = true;
	}
	isFromFinishedSignal = false;
	if (isFromFinishedSignal) {
		SimModelMutex.unlock();
		return;
	}
	isFromFinishedSignal = true;
	while (!m_taskQueue.isEmpty() && m_runningProcesses.load() < m_maxConcurrentProcesses) {
		TaskInfo task = m_taskQueue.dequeue();

		QProcess* process = new QProcess(this);
		connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
			[this, process, task, title, enableLogging, progressBar](int exitCode, QProcess::ExitStatus status) {
			//isFromFinishedSignal = true;
			m_runningProcesses.fetchAndSubOrdered(1);
			m_completedTasks.fetchAndAddOrdered(1);
			// 更新进度条
			if (progressBar) {
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
					m_SimProcesses.removeOne(process);
					//enableLogging = false;
					process->deleteLater();
					//m_taskQueue.clear();
					//return;
					if (title == QString::fromLocal8Bit("****控制点匹配"))
						m_taskQueue_col.enqueue(task);
				}
			}

			//if (enableLogging) {
			//	logEdit->append(QString::fromLocal8Bit("已完成任务：%1/%2")
			//		.arg(m_completedTasks.load())
			//		.arg(m_totalTasks));
			//}

			// 所有任务完成后的处理
			if (m_completedTasks.load() == m_totalTasks) {
				QDateTime endTime = QDateTime::currentDateTime();
				if (enableLogging && m_taskQueue_col.empty()) {
					//QMessageBox::StandardButton reply = QMessageBox::question(nullptr,
					//	title + QString::fromLocal8Bit("已完成"),
					//	QMessageBox::Yes | QMessageBox::No);
					m_ExeName = "";
					SimModelMutex.unlock();
					QMessageBox msgBox;
					msgBox.setWindowTitle(title);
					msgBox.setText(title + QString::fromLocal8Bit("已完成"));

					QPushButton *yesButton = msgBox.addButton(QString::fromLocal8Bit("确认"), QMessageBox::YesRole);

					msgBox.exec();

					if (msgBox.clickedButton() == yesButton) {
						return;
					}
					//logEdit->append(QString::fromLocal8Bit("****结束时间：") + endTime.toString(QString::fromLocal8Bit("hh时mm分ss秒")));
					//logEdit->append(title + QString::fromLocal8Bit("总耗时：") + systemConfig->calculateTimeDifference(m_startTime, endTime));
				}

				m_progressBarInitialized = false;
				progressBar->setVisible(false);
				m_completedTasks.store(0);
				m_runningProcesses.store(0);
				//if (title == QString::fromLocal8Bit("****控制点匹配")) {
				//
				//}
				//else if (title == QString::fromLocal8Bit("****连接点匹配")) {
				//	//ModelMutex.unlock();
				//	//emit EncryptedPointsMatchFinished();
				//	//return;
				//}
			
			}
			m_SimProcesses.removeOne(process);
			process->deleteLater();

				// 仅在需要时触发新任务检查
			if (m_runningProcesses.load() < m_maxConcurrentProcesses && !m_taskQueue.isEmpty()) {
				QMetaObject::invokeMethod(this, "startNextTasks",
					Qt::QueuedConnection,
					Q_ARG(QString, title),
					Q_ARG(bool, enableLogging));
			}
		});

		m_runningProcesses.fetchAndAddOrdered(1);
		m_SimProcesses.append(process);
		//if (title == QString::fromLocal8Bit("****影像云检"))
		//{
		//	process->start(task.operatorName, { task.taskFilePath, task.taskFilePath2 });
		//}
		//else if (title == QString::fromLocal8Bit("****连接点匹配"))
		//{
		//	process->start(task.operatorName, { task.taskFilePath, task.taskFilePath2, task.taskFilePath3 });
		//}
		//else
		//{
			process->start(task.operatorName, { task.taskFilePath });
		//}
	}
	isFromFinishedSignal = false;
}

void SimpleToolS::terminateAllProcesses()
{
	for (QProcess* process : m_SimProcesses) {
		if (process && process->state() != QProcess::NotRunning) {
			process->kill();
			process->waitForFinished(1000);
		}
	}
	m_SimProcesses.clear();
}


void SimpleToolS::on_pushButton_46_clicked()
{
	if (m_ExeName == u8"DEM相关性质检")
	{
		DEMRelatedPropertyCheck();
	}
}

void SimpleToolS::on_pushButton_47_clicked()
{

}