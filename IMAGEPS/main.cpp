//#include "IMAGEPS.h"
#include "IMAGEPS_InfoWindow.h"
#include <QtWidgets/QApplication>

#include <gdal.h>   
#include <gdal_priv.h>   
#include <ogr_spatialref.h>   
#include <QDebug>
#include <QFontDatabase>

#include <QFileInfo>

int main(int argc, char *argv[])
{
	//QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	//QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
	//QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
	//QApplication::setHighDpiScaleFactorRoundingPolicy(
	//	Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QApplication a(argc, argv);

	// 设置字体
	QString exeDir = QCoreApplication::applicationDirPath();
	QString fontPath = exeDir + "/resource/fonts/SourceHanSansSC-Medium.otf";
	int fontId = QFontDatabase::addApplicationFont(fontPath);
	if (fontId >= 0) {
		QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
		a.setFont(QFont(family));
	}

	// 验证许可证
	if (!PublicFunctions::validateDogLicense()) {
		return 0;
	}

	IMAGEPS_InfoWindow w;
	w.show();
	return a.exec();
}

//void restartApplication()
//{
//	QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList());
//	QCoreApplication::quit();
//}
//
//int main(int argc, char *argv[])
//{
//	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//	QApplication a(argc, argv);
//
//	QProcess process;
//	QProcess processDog;
//	QString exeDir = QCoreApplication::applicationDirPath();
//	QDir dir(exeDir);
//	QString exePath = dir.absolutePath();
//	int fontId = QFontDatabase::addApplicationFont("D:/CokeWork/VsProject/IMAGEPS/bin/resource/fonts/SourceHanSansSC-Medium.otf");
//	if (fontId < 0) {
//		// 字体加载失败
//		//qWarning("Failed to load font");
//	}
//	else {
//		QString family = QFontDatabase::applicationFontFamilies(fontId).at(0);
//		//qDebug() << "Registered font families:" << family;
//		a.setFont(QFont(family));
//	}
//
//	// 根据操作系统类型选择不同的算子 
//	QString authCmd;
//	QString authCmddir;
//#ifdef Q_OS_LINUX 
//	authCmddir = "/linux64";
//	QString libPath = exePath + "/linux64";
//	QString libPath_Mosaic = exePath + "/linux64";
//	qputenv("LD_LIBRARY_PATH",
//		QByteArray(libPath.toLocal8Bit() + ":"
//			libPath_Mosaic.toLocal8Bit() +
//			":" + qgetenv("LD_LIBRARY_PATH")));
//#else 
//	authCmddir = "/Software/DsmMatch_64";
//	authCmd = exePath + QString::fromLocal8Bit("/Software/DsmMatch_64/PSAuthorizeInquiryCmd.exe");
//#endif 
//
//	process.start(authCmd, QStringList() << "15");
//
//	// 等待进程完成，最多等待30秒 
//	if (!process.waitForFinished(30000)) {
//		QMessageBox msgBox(QMessageBox::Critical,  // 使用错误图标
//			u8"错误",
//			u8"许可证检查进程执行超时",
//			QMessageBox::NoButton,   // 先不设置标准按钮 
//			nullptr);
//		msgBox.setIcon(QMessageBox::Critical);      // 显式设置错误图标 
//		QPushButton *remoteUpgradeBtn = msgBox.addButton(u8"远程许可升级(hasp狗)", QMessageBox::ActionRole);
//		QPushButton *restartBtn = msgBox.addButton(u8" 重新启动", QMessageBox::ActionRole);
//		QPushButton *cancelBtn = msgBox.addButton(u8" 取消", QMessageBox::RejectRole);
//		msgBox.setDefaultButton(restartBtn);
//
//		msgBox.exec();
//
//		if (msgBox.clickedButton() == remoteUpgradeBtn) {
//			// 执行远程许可升级逻辑 
//			processDog.start(exePath + QString::fromLocal8Bit("/Software/RUS_HASP.exe"));
//			if (!processDog.waitForStarted(3000)) {
//				QMessageBox::critical(nullptr, u8"错误", u8"无法启动远程升级程序");
//			}
//			processDog.waitForFinished();
//		}
//		else if (msgBox.clickedButton() == restartBtn) {
//			restartApplication();
//		}
//		return 0;
//	}
//
//	// 检查进程退出状态 
//	if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
//		QMessageBox msgBox(QMessageBox::Critical,
//			u8"错误",
//			u8"许可证检查进程执行失败",
//			QMessageBox::NoButton,
//			nullptr);
//		msgBox.setIcon(QMessageBox::Critical);
//		QPushButton *remoteUpgradeBtn = msgBox.addButton(u8" 远程许可升级(hasp狗)", QMessageBox::ActionRole);
//		QPushButton *restartBtn = msgBox.addButton(u8" 重新启动", QMessageBox::ActionRole);
//		QPushButton *cancelBtn = msgBox.addButton(u8" 取消", QMessageBox::RejectRole);
//		msgBox.setDefaultButton(restartBtn);
//
//		msgBox.exec();
//
//		if (msgBox.clickedButton() == remoteUpgradeBtn) {
//			// 执行远程许可升级逻辑 
//			processDog.start(exePath + QString::fromLocal8Bit("/Software/RUS_HASP.exe"));
//			if (!processDog.waitForStarted(3000)) {
//				QMessageBox::critical(nullptr, u8"错误", u8"无法启动远程升级程序");
//			}
//			processDog.waitForFinished();
//		}
//		else if (msgBox.clickedButton() == restartBtn) {
//			restartApplication();
//		}
//		return 0;
//	}
//
//	QFile file(exePath + authCmddir + QString::fromLocal8Bit("/XQAuthorize.lic"));
//	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//		QMessageBox msgBox(QMessageBox::Critical,
//			u8"错误",
//			u8"无法打开许可证文件",
//			QMessageBox::NoButton,
//			nullptr);
//		msgBox.setIcon(QMessageBox::Critical);
//		QPushButton *remoteUpgradeBtn = msgBox.addButton(u8" 远程许可升级(hasp狗)", QMessageBox::ActionRole);
//		QPushButton *restartBtn = msgBox.addButton(u8" 重新启动", QMessageBox::ActionRole);
//		QPushButton *cancelBtn = msgBox.addButton(u8" 取消", QMessageBox::RejectRole);
//		msgBox.setDefaultButton(restartBtn);
//
//		msgBox.exec();
//
//		if (msgBox.clickedButton() == remoteUpgradeBtn) {
//			// 执行远程许可升级逻辑 
//			processDog.start(exePath + QString::fromLocal8Bit("/Software/RUS_HASP.exe"));
//			if (!processDog.waitForStarted(3000)) {
//				QMessageBox::critical(nullptr, u8"错误", u8"无法启动远程升级程序");
//			}
//			processDog.waitForFinished();
//		}
//		else if (msgBox.clickedButton() == restartBtn) {
//			restartApplication();
//		}
//		return 0;
//	}
//
//	bool hasSuperDog = false;
//	bool hasHaspDog = false;
//	int superDogValue = 0;
//	int haspDogValue = 0;
//	QTextStream in(&file);
//	while (!in.atEnd()) {
//		QString line = in.readLine().trimmed();
//		if (line.isEmpty())  continue;
//		QStringList parts = line.split(':');
//		if (parts.size() != 2) continue;
//		QString key = parts[0].trimmed();
//		QString valueStr = parts[1].trimmed();
//		bool ok;
//		int value = valueStr.toInt(&ok);
//		if (ok && (value == 0 || value == 1)) {
//			if (key == "SUPERDOG") {
//				superDogValue = value;
//				hasSuperDog = true;
//	}
//			else if (key == "HASPDOG") {
//				haspDogValue = value;
//				hasHaspDog = true;
//			}
//}
//	}
//	file.close();
//
//	if (!hasSuperDog || !hasHaspDog) {
//		QMessageBox msgBox(QMessageBox::Critical,
//			u8"错误",
//			u8"无效的许可证格式",
//			QMessageBox::NoButton,
//			nullptr);
//		msgBox.setIcon(QMessageBox::Critical);
//		QPushButton *remoteUpgradeBtn = msgBox.addButton(u8" 远程许可升级(hasp狗)", QMessageBox::ActionRole);
//		QPushButton *restartBtn = msgBox.addButton(u8" 重新启动", QMessageBox::ActionRole);
//		QPushButton *cancelBtn = msgBox.addButton(u8" 取消", QMessageBox::RejectRole);
//		msgBox.setDefaultButton(restartBtn);
//
//		msgBox.exec();
//
//		if (msgBox.clickedButton() == remoteUpgradeBtn) {
//			// 执行远程许可升级逻辑 
//			processDog.start(exePath + QString::fromLocal8Bit("/Software/RUS_HASP.exe"));
//			if (!processDog.waitForStarted(3000)) {
//				QMessageBox::critical(nullptr, u8"错误", u8"无法启动远程升级程序");
//			}
//			processDog.waitForFinished();
//		}
//		else if (msgBox.clickedButton() == restartBtn) {
//			restartApplication();
//		}
//		return 0;
//	}
//
//	if (!(superDogValue == 1 || haspDogValue == 1)) {
//		QMessageBox msgBox(QMessageBox::Critical,
//			u8"错误",
//			u8"无效的许可证格式",
//			QMessageBox::NoButton,
//			nullptr);
//		msgBox.setIcon(QMessageBox::Critical);
//		QPushButton *remoteUpgradeBtn = msgBox.addButton(u8" 远程许可升级(hasp狗)", QMessageBox::ActionRole);
//		QPushButton *restartBtn = msgBox.addButton(u8" 重新启动", QMessageBox::ActionRole);
//		QPushButton *cancelBtn = msgBox.addButton(u8" 取消", QMessageBox::RejectRole);
//		msgBox.setDefaultButton(restartBtn);
//
//		msgBox.exec();
//
//		if (msgBox.clickedButton() == remoteUpgradeBtn) {
//			// 执行远程许可升级逻辑
//			processDog.start(exePath + QString::fromLocal8Bit("/Software/RUS_HASP.exe"));
//			if (!processDog.waitForStarted(3000)) {
//				QMessageBox::critical(nullptr, u8"错误", u8"无法启动远程升级程序");
//			}
//			processDog.waitForFinished();
//		}
//		else if (msgBox.clickedButton() == restartBtn) {
//			restartApplication();
//		}
//		return 0;
//	}
//
//	IMAGEPS_InfoWindow w;
//	w.show();
//	return a.exec();
//}
