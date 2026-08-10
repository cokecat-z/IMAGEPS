#ifndef WEBBROWSERDIALOG_H
#define WEBBROWSERDIALOG_H

#include <QDialog>
#include <QWebEngineView>
#include <QVBoxLayout>
#include <QUrl>

class WebBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WebBrowserDialog(const QUrl& url, QWidget *parent = nullptr);
    ~WebBrowserDialog();

private:
    QWebEngineView* webView;
};

#endif // WEBBROWSERDIALOG_H