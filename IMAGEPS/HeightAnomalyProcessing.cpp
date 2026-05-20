#include "HeightAnomalyProcessing.h"

//HeightAnomalyProcessing* HeightAnomalyProcessing::instance = nullptr;

HeightAnomalyProcessing::HeightAnomalyProcessing(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	setWindowTitle(QString::fromLocal8Bit("影像编辑处理"));
	setWindowIcon(QIcon(QString::fromLocal8Bit(":/resource/menu/工具/高程异常处理.png")));

	QString exeDir = QCoreApplication::applicationDirPath();
	QDir dir(exeDir);
	this->appDirPath = dir.absolutePath();

	initWidget();
	connects();
}

HeightAnomalyProcessing::~HeightAnomalyProcessing()
{}

void HeightAnomalyProcessing::initWidget()
{
	//setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint); // 移除帮助按钮

	buttonGroup_model = {
		ui.HeightAnomalyDetection_btn , ui.HeightAnomalyElimination_btn/*, ui.DSMDEMLogicalConsistencyProcessing_btn,
		ui.ParallelDSMDEMLogicalConsistencyProcessing_btn , ui.DSMDEMStandardizationCheck_btn, ui.DSMDEMEdgeMatchingCheck_btn,
		ui.DEMRelatedPropertyCheck_btn*/
	};

	QStringList iconPaths = {
		QString::fromLocal8Bit(":/resource/menu/子菜单/高程异常消除.png"),
		QString::fromLocal8Bit(":/resource/menu/子菜单/高程异常查找.png"),
		//QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM逻辑一致性处理.png"),
		//QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM逻辑一致性处理(并行).png"),
		//QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM标准化检查.png"),
		//QString::fromLocal8Bit(":/resource/menu/子菜单/DSM-DEM接边检查.png"),
		//QString::fromLocal8Bit(":/resource/menu/子菜单/DEM相关性质检工具.png")
	};

	// 为每个按钮设置图标
	for (int i = 0; i < buttonGroup_model.size(); ++i) {
		buttonGroup_model[i]->setIcon(QIcon(iconPaths[i]));
		buttonGroup_model[i]->setIconSize(QSize(32, 32));
	}
}

void HeightAnomalyProcessing::connects() {

	//m_toolConnections = {
	//	////高程异常消除
	//	//{"PSRemoveCorseValue.exe", {
	//	//	ui.HeightAnomalyDetection_btn,
	//	//	ui.HeightAnomalyDetection_btnf
	//	//}},

	//	////高程异常查找
	//	//{"PSFindCorseValueTool.exe", {
	//	//	ui.HeightAnomalyElimination_btn,
	//	//	ui.HeightAnomalyElimination_btnf
	//	//}},

	//	////DSM-DEM逻辑一致性处理
	//	//{"PSDsmDemCheckTool.exe", {
	//	//	ui.DSMDEMLogicalConsistencyProcessing_btn,
	//	//	ui.DSMDEMLogicalConsistencyProcessing_btnf
	//	//}},

	//	////DSM-DEM逻辑一致性处理（并行）
	//	//{"PSDsmDemCheckTool2.exe", {
	//	//	ui.ParallelDSMDEMLogicalConsistencyProcessing_btn,
	//	//	ui.ParallelDSMDEMLogicalConsistencyProcessing_btnf
	//	//}},

	//	////DSM-DEM标准化检查
	//	//{"PSDsmDemStandCheckTool.exe", {
	//	//	ui.DSMDEMStandardizationCheck_btn,
	//	//	ui.DSMDEMStandardizationCheck_btnf
	//	//}},

	//	////DSM-DEM接边检查
	//	//{"PSDsmDemRelCheckTool.exe", {
	//	//	ui.DSMDEMEdgeMatchingCheck_btn,
	//	//	ui.DSMDEMEdgeMatchingCheck_btnf
	//	//}},

	//	////DEM相关性质检工具
	//	//{"PSImageCorrelatonTool.exe", {
	//	//	ui.DEMRelatedPropertyCheck_btn,
	//	//	ui.DEMRelatedPropertyCheck_btnf
	//	//}},
	//};

	m_moduleConnections = {
		//高程异常消除
		{"PSRemoveCorseValue.exe", {
			ui.HeightAnomalyDetection_btn,
			ui.HeightAnomalyDetection_btnf
		}},
		//高程异常查找
		{"PSFindCorseValue.exe", {
			ui.HeightAnomalyElimination_btn,
			ui.HeightAnomalyElimination_btnf
		}},
	};

	//// 移除空指针 
	//for (auto& toolList : m_toolConnections) {
	//	toolList.erase(std::remove_if(toolList.begin(), toolList.end(),
	//		[](QObject* obj) { return obj == nullptr; }), toolList.end());
	//}

	//for (auto it = m_toolConnections.begin(); it != m_toolConnections.end(); ++it) {

	//	for (QObject* uiElement : it.value()) {
	//		if (QPushButton* button = qobject_cast<QPushButton*>(uiElement)) {
	//			if (button) {
	//				connect(button, &QPushButton::clicked, this, [=]() {
	//					QString fullToolPath = this->appDirPath +
	//						QString::fromLocal8Bit("/Software/") + it.key();
	//					QProcess* process = new QProcess(this);
	//					process->start(fullToolPath);
	//				});
	//			}
	//		}
	//	}
	//}

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
						QString fullToolPath = this->appDirPath +
							"/" + it.key();
						QProcess* process = new QProcess(this);
						process->start(fullToolPath);
					});
				}
			}
		}
	}

}