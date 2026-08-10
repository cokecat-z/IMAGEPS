#include "TabbedDockWidget.h"

TabbedDockWidget::TabbedDockWidget(QWidget* parent)
	: TabbedDockWidget("Untitled", parent) 
{
	m_isImageDisplayMode = false;
}

TabbedDockWidget::TabbedDockWidget(const QString& title, QWidget* parent)
	: QDockWidget(title, parent),
	m_isImageDisplayMode(false)
{
	// 初始化 StackedWidget
	stackedWidget = new QStackedWidget(this);
	setWidget(stackedWidget);

	// 使用默认标题栏，不进行自定义
	// 只需要连接默认的关闭按钮信号
	connect(this, &QDockWidget::topLevelChanged, [this](bool floating) {
		//closeCurrentPage();
	});

	// 如果需要拦截关闭事件，可以这样连接 
	connect(toggleViewAction(), &QAction::triggered, this, [this]() {
		closeCurrentPage();  // 调用自定义关闭逻辑
	});
}

void TabbedDockWidget::setImageDisplayMode(bool isImageDisplay) {
	m_isImageDisplayMode = isImageDisplay;
}

void TabbedDockWidget::closeEvent(QCloseEvent* event) {
	closeCurrentPage();
	event->ignore(); // 阻止 DockWidget 关闭
}

void TabbedDockWidget::closeCurrentPage() {
	// 如果处于影像显示模式，发出完整关闭请求信号
	if (m_isImageDisplayMode) {
		emit imageCloseRequested();
	}

	QWidget* mainWidget = this->widget();
	if (!mainWidget) return;

	QTabWidget* tabWidget = mainWidget->findChild<QTabWidget*>();
	if (!tabWidget || tabWidget->count() == 0) {
		hide();
		return;
	}

	int currentIndex = tabWidget->currentIndex();
	if (currentIndex >= 0) {
		tabWidget->setTabEnabled(currentIndex, false);

		tabWidget->setStyleSheet(
			"QTabBar::tab:disabled { width: 0; color: transparent; }"
			"QTabBar::scroller { width: 0; }"
			"QTabBar::tab:selected { background: #00a99d; color: white; border-bottom: 2px solid #008080; }"
		);
	}

	bool allDisabled = true;
	for (int i = 0; i < tabWidget->count(); ++i) {
		if (tabWidget->isTabEnabled(i)) {
			allDisabled = false;
			break;
		}
	}

	if (allDisabled) {
		hide(); // 全部禁用则隐藏DockWidget 
	}
	else {
		// 查找并切换到下一个可用标签页
		int nextIndex = -1;
		for (int i = currentIndex + 1; i < tabWidget->count(); ++i) {
			if (tabWidget->isTabEnabled(i)) {
				nextIndex = i;
				break;
			}
		}
		if (nextIndex == -1) {
			for (int i = 0; i < currentIndex; ++i) {
				if (tabWidget->isTabEnabled(i)) {
					nextIndex = i;
					break;
				}
			}
		}
		if (nextIndex >= 0) {
			tabWidget->setCurrentIndex(nextIndex);
		}
	}
}