#ifndef PROJECTSETTING_H
#define PROJECTSETTING_H

#include "ui_projectSetting.h"
#include <QDialog>
#include <QStackedWidget>
#include <QComboBox>
#include <QDebug>
#include <QRegularExpression>
#include <QGuiApplication>
#include <QScreen>
#include "PublicFunctions.h"

class IMAGEPS;

//预定义投影详细信息
struct PredefineData
{
	QString Datum;
	QString SPHEROID;
	int SemiMajor;
	double InFlattening;
	QString PrimeMeridian;
	QList<int> List_TOWGS84;
	QList<int> List_projectInfo;
	QString PROJCS; //PROJCS
};

class ProjectSetting : public QDialog
{
	Q_OBJECT

public:
	ProjectSetting(QDialog *parent = nullptr, IMAGEPS* imagePSInstance = nullptr);
	//ProjectSetting(QDialog *parent = nullptr);
	~ProjectSetting();

	void initWidget();
	void loadConfig(QString funfunModule = QString::fromLocal8Bit("投影配置功能列表"));
	void connects();
	QString m_openerButtonName;
	void setOpenerButtonName(const QString& name) {
		m_openerButtonName = name;
	}
	QString openerButtonName() const {
		return m_openerButtonName;
	}

public slots:
	void openSettingWidgetSlot();
	void funListWidgetItemSlot(QListWidgetItem* item);
	void customProject_projectTypeBoxSlot(QString text);
	void projectNameListWidgetItemSlot(QListWidgetItem* item);
	void projectTypeBoxSlot(QString text);
	void okButtonSlot();
	void applyButtonSlot();
	void onListModified();

signals:
	void PROJCStext(QString PROJCS); //空间参考信号

private:
	Ui::ProjectSetting ui;
	IMAGEPS* m_imagePS;

	QMap<QString, QMap<QString,PredefineData>> tranData;//QMap<坐标系类型，QMap<坐标系名称 详细信息>>
	QList<QStringList> funListConifg;//投影配置功能列表
	QMap<QString, bool> funModuleBool;//投影配置功能模块控制 避免重复点击反复读取
	QMap<QString, QList<QStringList>> funModuleTemp;//组件内下拉菜单使能控制信息
	QMap<QString, QStringList> projectTypetoInfo;//信息种属 投影类型表 
};

#endif