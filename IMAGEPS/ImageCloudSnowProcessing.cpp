#include "ImageCloudSnowProcessing.h"

//ImageCloudSnowProcessing* ImageCloudSnowProcessing::instance = nullptr;

ImageCloudSnowProcessing::ImageCloudSnowProcessing(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("影像编辑处理"));
	setWindowIcon(QIcon(u8":/resource/menu/工具/影像云雪和拉花变形替换.png"));
	
	QString exeDir = QCoreApplication::applicationDirPath();
	QDir dir(exeDir);
	this->appDirPath = dir.absolutePath();

	initWidget();
	connects();
}

ImageCloudSnowProcessing::~ImageCloudSnowProcessing()
{}

void ImageCloudSnowProcessing::initWidget()
{
	//setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 移除帮助按钮

	buttonGroup_model = {
		/*ui.CloudReplaceTool2_btn ,*/ ui.CloudReplaceTool_btn, ui.AutoCloudReplaceTool_btn
	};

	QStringList iconPaths = {
		/*QString::fromLocal8Bit(":/resource/menu/子菜单/影像云雪自动替换软件(交互式).png"),*/
		QString::fromLocal8Bit(":/resource/menu/子菜单/影像云宣自动替换软件(批处理).png"),
		QString::fromLocal8Bit(":/resource/menu/子菜单/影像云雪自动替换软件(全自动).png")
	};

	// 为每个按钮设置图标
	for (int i = 0; i < buttonGroup_model.size(); ++i) {
		buttonGroup_model[i]->setIcon(QIcon(iconPaths[i]));
		buttonGroup_model[i]->setIconSize(QSize(32, 32));
	}

	ui.CloudReplaceTool2_btnf->setVisible(false);
	ui.CloudReplaceTool2_btn->setVisible(false);
}

void ImageCloudSnowProcessing::connects() {

	m_toolConnections = {
		//// 影像云雪自动替换软件（交互式）
		//{"PSCloudReplaceTool2.exe",  {
		//	ui.CloudReplaceTool2_btn,
		//	ui.CloudReplaceTool2_btnf
		//}},

		// 影像云雪自动替换软件（批处理）
		{"PSImageCloudandSnowProcessing.exe",  {
			ui.CloudReplaceTool_btn,
			ui.CloudReplaceTool_btnf
		}},

		// 影像云雪自动替换软件（全自动）
		{"PSAutoCloudReplace.exe",  {
			ui.AutoCloudReplaceTool_btn,
			ui.AutoCloudReplaceTool_btnf
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
							QString::fromLocal8Bit("/") + it.key();
						QProcess* process = new QProcess(this);
						process->start(fullToolPath);
					});
				}
			}
		}
	}
}