#ifndef SIMPLETOOLS_H
#define SIMPLETOOLS_H

#include <QDialog>
#include <QFileDialog>
#include <QDebug>
#include "ui_SimpleToolS.h"

#include "SystemConfig.h"

class IMAGEPS;

class SimpleToolS : public QDialog
{
	Q_OBJECT

public:
	explicit SimpleToolS(IMAGEPS* parentImagePS = nullptr, QWidget* parent = nullptr);

	SimpleToolS(QWidget *parent = nullptr);
	~SimpleToolS();
	Ui::SimpleToolSClass ui;
	QTextEdit * logEdit;
	QProgressBar* m_ProgressBar;

	QString m_ExeName;

	void DEMRelatedPropertyCheck();

private slots:
	void on_pushButton_46_clicked();

	void on_pushButton_47_clicked();

private:
	void initWidget();
	void connects();

	// 通用绑定函数
	void bindFileDialog(
		QToolButton* button,          // 绑定的按钮 
		QLineEdit* lineEdit,          // 显示路径的输入框
		QCheckBox* checkBox = nullptr,// 可选：控制选择文件/文件夹 
		const QString& filter = "所有文件 (*.*)"  // 文件类型过滤 
	);
	void startNextTasks(QString title, bool enableLogging);
	void terminateAllProcesses();  // 终止所有进程 

	// 辅助函数，用于检查并获取锁 
	bool tryLockFunction(QMutex& mutex, const QString& functionName) {
		if (!mutex.tryLock()) {
				QMessageBox::warning(this,
				QString::fromLocal8Bit("警告"),
				QString::fromLocal8Bit("已有任务正在运行中，请等待任务结束"));
			return false;
		}
		return true;
	}

	IMAGEPS* m_imagePS;

	struct TaskInfo {
		QString taskFilePath;
		QString operatorName;
		QString outfilename;
		QString taskFilePath2;
		QString taskFilePath3;
	};
	QQueue<TaskInfo> m_taskQueue;
	QQueue<TaskInfo> m_taskQueue_col;
	QAtomicInt m_runningProcesses;
	QAtomicInt m_completedTasks;
	QDateTime m_startTime;
	int m_totalTasks = 0;
	int m_maxConcurrentProcesses = 1; // 根据CPU核心数调整并发数
	bool m_progressBarInitialized = false;//进度条初始化标识
	QMutex SimModelMutex;

	bool isFromFinishedSignal = false;

	QList<QProcess*> m_SimProcesses;  // 存储SimpleToolS启动的进程

	SystemConfig* systemConfig = nullptr;
};

#endif