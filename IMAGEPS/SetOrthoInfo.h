#ifndef SETORTHOINFO_H
#define SETORTHOINFO_H

#include <QDialog>
#include <QSettings>
#include <QMap>
#include <QString>
#include "ui_SetOrthoInfo.h"
#include "ProjectSetting.h"

class IMAGEPS;

class SetOrthoInfo : public QDialog
{
	Q_OBJECT

public:
	explicit SetOrthoInfo(IMAGEPS* parentImagePS = nullptr, QWidget* parent = nullptr);

	//explicit SetOrthoInfo(QWidget *parent = nullptr);
	~SetOrthoInfo();
	Ui::SetOrthoInfo ui;

	QString m_configFile = "ortho_settings.ini";  // 统一配置文件路径 
	void setconfigFile();

	// 保存所有文件设置到单一配置文件 
	void saveAllSettingsToFile(const QString& configFile);

	// 从单一配置文件加载特定文件的设置 
	void loadSettingsFromFile(const QString& configFile, const QString& fileName);

	// 从单一配置文件加载多个文件的共同设置 
	void loadCommonSettingsFromFile(const QString& configFile, const QStringList& fileNames);

	// 设置当前文件列表 
	void setCurrentFileList(const QStringList& filePaths);

	// 获取当前文件列表 
	QStringList getCurrentFileList() const;

	//获取文件配置信息 
	QMap<QString, QString> getFileConfigInfo(const QString& fileName);

private slots:
	void on_toolButton_clicked();

private:
	IMAGEPS* m_imagePS;
	ProjectSetting  projectSetting;
	QStringList m_filePath;  // 存储文件路径
	QStringList m_currentFileList;
	QMap<QString, QMap<QString, PredefineData>> tranData;//QMap<坐标系类型，QMap<坐标系名称 详细信息>>

	void initWidget();
	void connects();
	void loadConfig();

	void setPROJCStextSlot(QString PROJCStext);

	// 获取当前控件状态 
	QMap<QString, QVariant> getCurrentSettings() const;

	// 应用设置到UI 
	void applySettingsToUI(const QMap<QString, QVariant>& settings);

	// 查找对应的投影坐标系
	PredefineData findCoordinateSystem(const QString& coordType, const QString& imagePath);
	PredefineData findUTMCoordinateSystem(const QString& coordType, const QString& imagePath);

};

#endif
