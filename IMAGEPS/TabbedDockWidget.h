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

class TabbedDockWidget : public QDockWidget {
	Q_OBJECT
public:
	explicit TabbedDockWidget(QWidget* parent = nullptr); // 默认构造 
	explicit TabbedDockWidget(const QString& title, QWidget* parent = nullptr); // 带标题构造

	// 获取当前页面数量
	int pageCount() const { return stackedWidget->count(); }

signals:
	void pageClosed(int index); // 页面关闭时发出信号 

protected:
	void closeEvent(QCloseEvent* event) override;

private slots:
	void closeCurrentPage();

private:
	QStackedWidget* stackedWidget;
	QLabel* titleLabel; // 显示当前页面标题
};

#endif // TABBEDDOCKWIDGET_H 