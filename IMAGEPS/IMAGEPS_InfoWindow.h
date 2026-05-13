#ifndef IMAGEPS_INFOWINDOW_H
#define IMAGEPS_INFOWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_IMAGEPS_InfoWindow.h"
#include <QSettings>
#include <QTextCodec>
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QDesktopServices>

#include "IMAGEPS.h"
#include "ImageCloudSnowProcessing.h"
#include "RPC2RPB.h"
#include "InvalidValueHandling.h"
#include "HeightAnomalyProcessing.h"
#include "ImageCropTool.h"

class IMAGEPS_InfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    IMAGEPS_InfoWindow(QWidget *parent = nullptr);
    ~IMAGEPS_InfoWindow();

	QString appDirPath;

	static IMAGEPS_InfoWindow* instance;
	Ui::IMAGEPS_InfoWindowClass ui;

signals:
	void aboutToClose_InfoWindow();

private:
    //Ui::IMAGEPS_InfoWindowClass ui;
	QSettings* settings;
	QMap<QString, QAction*> Map_mainToolBarAction;//主菜单->菜单子项
	
	QMap<QString, QList<QObject*>> m_toolConnections;
	QMap<QString, QList<QObject*>> m_moduleConnections;

	void initWidget();
	void connects();

	void newProActionSlot();//新建工程
	void openProActionSlot();//打开工程

	void addRecentProjectItem();

	void startToolProcess(const QString& toolName);

	template<typename T>
	void connectUIToTool(const QString& toolName, T* uiElement);
	void connectMultipleUIToTool(const QString& toolName, const QList<QObject*>& uiElements);
	void connectActionToTool(const QString& toolName, QAction* action);
	void connectButtonToTool(const QString& toolName, QPushButton* button);
	void connectLabelToTool(const QString& toolName, QLabel* label);

	void closeEvent(QCloseEvent* event);

	bool removeLineFromCsv(const QString &filePath,
		const QString &fullProjectPath,
		const QString &projectId);

	//IMAGEPS *window = nullptr;
	QList<IMAGEPS*> m_openWindows;
	QVector<QPushButton*> buttonGroup;
	QVector<QPushButton*> buttonGroup_model;

	ImageCloudSnowProcessing* ImageCloudSnowShow = nullptr;
	ImageCropTool* ImageCrop = nullptr;
	HeightAnomalyProcessing* HeightAnomaly = nullptr;
	InvalidValueHandling* InvalidValueHandl = nullptr;
	RPC2RPB* RPCTRPB = nullptr;
};

#endif