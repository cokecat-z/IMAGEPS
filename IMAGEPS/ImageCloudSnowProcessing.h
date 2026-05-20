#ifndef IMAGECLOUDSHOWSNOWPROCESSING_H
#define IMAGECLOUDSHOWSNOWPROCESSING_H

#include <QDialog>
#include <QIcon>
#include <QAction>
#include <QProcess>
#include <QDir>

#include "ui_ImageCloudSnowProcessing.h"

class ImageCloudSnowProcessing : public QDialog
{
	Q_OBJECT

public:
	ImageCloudSnowProcessing(QWidget *parent = nullptr);
	~ImageCloudSnowProcessing();

	QString appDirPath;
	//static ImageCloudSnowProcessing* instance;

private:
	Ui::ImageCloudSnowProcessingClass ui;

	void initWidget();
	void connects();

	QVector<QPushButton*> buttonGroup_model;
	QMap<QString, QList<QObject*>> m_toolConnections;
};

#endif