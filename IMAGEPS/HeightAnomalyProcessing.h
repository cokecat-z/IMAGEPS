#ifndef HEIGHTANOMALYPROCESSING_H
#define HEIGHTANOMALYPROCESSING_H

#include <QDialog>
#include <QIcon>
#include <QAction>
#include <QProcess>
#include <QDir>

#include "PublicFunctions.h"

#include "ui_HeightAnomalyProcessing.h"

class HeightAnomalyProcessing : public QDialog
{
	Q_OBJECT

public:
	HeightAnomalyProcessing(QWidget *parent = nullptr);
	~HeightAnomalyProcessing();

	QString appDirPath;
	//static HeightAnomalyProcessing* instance;

private:
	Ui::HeightAnomalyProcessingClass ui;

	void initWidget();
	void connects();

	QVector<QPushButton*> buttonGroup_model;
	QMap<QString, QList<QObject*>> m_toolConnections;
	QMap<QString, QList<QObject*>> m_moduleConnections;
};

#endif