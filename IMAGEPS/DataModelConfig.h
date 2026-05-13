#ifndef DATAMODELCONFIG_H
#define DATAMODELCONFIG_H

#include <QDialog>
#include "ui_DataModelConfig.h"
#include <QStackedWidget>
#include <QScrollArea>
#include <QFileDialog>
#include <QDomDocument>
#include <QFile>
#include <QProcess>
#include <qtextedit.h>
#include <QDateTime>
#include <QXmlStreamReader>
#include <QMutex>
#include <QtWidgets/QApplication>

#include "PublicFunctions.h"

class IMAGEPS;

class DataModelConfig : public QDialog
{
	Q_OBJECT

public:
	explicit DataModelConfig(IMAGEPS* parentImagePS = nullptr, QWidget* parent = nullptr);

	DataModelConfig(QWidget *parent = nullptr);
	~DataModelConfig();
	void setProjectdir();

	bool removeFileIfExists(const QString & filePath);

	void initWidget();
	void loadFile(QStringList filenamelist);
	QString BuildSatModelMakerCmdXML(QStringList filenamelist, const QString outdir);
	

	QTextEdit * logEdit;
	Ui::DataModelConfigClass ui;
	QMap<QString, bool> DataModelfilenames; //模型创建文件及状态


private:
	void connects();


public slots:
	void BuildmodelSlot();
	void deletemodelSlot();
	void okmodelSlot();

	bool writeMatchModelFromSatModelMakerCmdout();

	bool loadMatchModelToTable();

	void move_to_end()
	{
		if (logEdit)
			logEdit->moveCursor(QTextCursor::End);
	}


private:
	IMAGEPS* m_imagePS;
	QString projectdir;//工程文件目录
	QList<QStringList> funListConifg;//数据模型配置列表
	QMap<QString, QList<QStringList>> funModuleTemp;//数据模型配置功能临时列表
	bool DataModelBool;//模型构建功能控制 避免重复点击反复读取
	bool LoadDataModelBool;//模型构建功能控制 避免重复点击反复读取

	QMutex DataModeMutex;

	// 辅助函数，用于检查并获取锁 
	bool tryLockFunction(QMutex& mutex, const QString& functionName) {
		if (!mutex.tryLock()) {
			//qDebug() << functionName << "is already running, aborting";
			logEdit->append(functionName + QString::fromLocal8Bit("任务在执行,请稍后再试!!!"));
			return false;
		}
		return true;
	}

	bool eventFilter(QObject *watched, QEvent *event);

};

#endif