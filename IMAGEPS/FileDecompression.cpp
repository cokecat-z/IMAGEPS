#include "FileDecompression.h"


FileDecompression::FileDecompression(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("文件解压缩"));
	setWindowIcon(QIcon(u8":/resource/menu/工具/遥感影像解压缩.png"));

	QString exeDir = QCoreApplication::applicationDirPath();
	QDir dir(exeDir);
	this->appDirPath = dir.absolutePath();

	initWidget();
	connects();
}

FileDecompression::~FileDecompression()
{}

void FileDecompression::initWidget()
{
	//setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 移除帮助按钮

	buttonGroup_model = {
		ui.FileDecompressionTool_btn, ui.DataScheduledDecompression_btn 
	};

	QStringList iconPaths = {
		QString::fromLocal8Bit(":/resource/menu/工具/DEM镶嵌.png"),
		QString::fromLocal8Bit(":/resource/menu/工具/遥感影像解压缩.png")
	};

	// 为每个按钮设置图标
	for (int i = 0; i < buttonGroup_model.size(); ++i) {
		buttonGroup_model[i]->setIcon(QIcon(iconPaths[i]));
		buttonGroup_model[i]->setIconSize(QSize(32, 32));
	}
}

void FileDecompression::connects() {

	m_toolConnections = {
		//无效值替换
		{"FileDecompressionTool.exe", {
			ui.FileDecompressionTool_btn,
			ui.FileDecompressionTool_btnf
		}},

		////影像零值处理(白点)
		//{"PSWhiteDotEraseTool.exe", {
		//	ui.ImageZeroValueProcessing_btn,
		//	ui.ImageZeroValueProcessing_btnf
		//}},

		//////影像零值处理(白点)并行版
		////{"PSWhiteDotEraseTool2.exe", {
		////	ui.ImageZerovalueProcessings_btn,
		////	ui.ImageZerovalueProcessings_btnf
		////}},

		////无效值查找
		//{"PSHoleCheckTool.exe", {
		//	ui.Invalidvaluelookup_btn,
		//	ui.Invalidvaluelookup_btnf
		//}}
	};

	m_moduleConnections = {
		//影像零值处理(白点)
		{"DataScheduledDecompression.exe", {
			ui.DataScheduledDecompression_btn,
			ui.DataScheduledDecompression_btnf
		}}
	};

	// 移除空指针 
	for (auto& toolList : m_toolConnections) {
		toolList.erase(std::remove_if(toolList.begin(), toolList.end(),
			[](QObject* obj) { return obj == nullptr; }), toolList.end());
	}

	for (auto it = m_toolConnections.begin(); it != m_toolConnections.end(); ++it) {

		for (QObject* uiElement : it.value()) {
			if (QPushButton* button = qobject_cast<QPushButton*>(uiElement)) {
				if (button) {
					connect(button, &QPushButton::clicked, this, [=]() {
						QString fullToolPath = this->appDirPath +
							QString::fromLocal8Bit("/FileDecompression/standalone/") + it.key();
						QProcess* process = new QProcess(this);
						process->start(fullToolPath);
					});
				}
			}
		}
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
						QString fullToolPath = this->appDirPath +
							"/FileDecompression/timer/" + it.key();
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
						QString fullToolPath = this->appDirPath +
							"/FileDecompression/timer/" + it.key();
						QProcess* process = new QProcess(this);
						process->start(fullToolPath);
					});
				}
			}
		}
	}
}