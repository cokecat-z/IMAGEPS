#ifndef TABBEDDOCKWIDGET_H 
#define TABBEDDOCKWIDGET_H

#include <QDockWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCloseEvent>
#include <QTabWidget>
#include <QAction>
#include <QString>

class TabbedDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit TabbedDockWidget(QWidget* parent = nullptr); // 默认构造 
	explicit TabbedDockWidget(const QString& title, QWidget* parent = nullptr); // 带标题构造

	// 获取当前页面数量
	int pageCount() const { return stackedWidget->count(); }

	// 设置影像显示模式
	void setImageDisplayMode(bool isImageDisplay);

signals:
	void pageClosed(int index); // 页面关闭时发出信号 
	
	// 影像关闭时发出的信号，通知主窗口执行完整清理
	void imageCloseRequested();

protected:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void closeCurrentPage();

private:
	QStackedWidget* stackedWidget;
	QLabel* titleLabel; // 显示当前页面标题
	bool m_isImageDisplayMode; // 是否处于影像显示模式
};

#endif // TABBEDDOCKWIDGET_H 