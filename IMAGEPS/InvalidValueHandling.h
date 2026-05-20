#ifndef INVALIDVALUEHANDLING_H
#define INVALIDVALUEHANDLING_H

#include <QDialog>
#include <QIcon>
#include <QAction>
#include <QProcess>
#include <QDir>

#include "PublicFunctions.h"

#include "ui_InvalidValueHandling.h"

class InvalidValueHandling : public QDialog
{
	Q_OBJECT

public:
	InvalidValueHandling(QWidget *parent = nullptr);
	~InvalidValueHandling();

	QString appDirPath;
	//static InvalidValueHandling* instance;

private:
	Ui::InvalidValueHandlingClass ui;

	void initWidget();
	void connects();

	QVector<QPushButton*> buttonGroup_model;
	QMap<QString, QList<QObject*>> m_toolConnections;
	QMap<QString, QList<QObject*>> m_moduleConnections;
};

#endif