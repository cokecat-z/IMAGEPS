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

