#ifndef FILEDECOMPRESSION_H
#define FILEDECOMPRESSION_H

#include <QDialog>
#include <QIcon>
#include <QAction>
#include <QProcess>
#include <QDir>

#include "PublicFunctions.h"

#include "ui_FileDecompression.h"

class FileDecompression : public QDialog
{
	Q_OBJECT

public:
	FileDecompression(QWidget *parent = nullptr);
	~FileDecompression();

	QString appDirPath;

private:
	Ui::FileDecompressionClass ui;

	void initWidget();
	void connects();

	QVector<QPushButton*> buttonGroup_model;
	QMap<QString, QList<QObject*>> m_toolConnections;
	QMap<QString, QList<QObject*>> m_moduleConnections;
};

#endif