#include "WebBrowserDialog.h"
#include <QDebug>

WebBrowserDialog::WebBrowserDialog(const QUrl& url, QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(u8"影像处理Web服务");
    setMinimumSize(1200, 800);
    
    webView = new QWebEngineView(this);
    webView->setUrl(url);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(webView);
    
    setLayout(layout);
}

WebBrowserDialog::~WebBrowserDialog()
{
    delete webView;
}