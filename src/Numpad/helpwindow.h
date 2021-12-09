#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <QWidget>

class QTextBrowser;

class HelpWindow : public QWidget
{   
public:
    explicit HelpWindow(QWidget *parent = nullptr);
    void moveToAnchor(const QString &);

private:
    QTextBrowser *textBrowser;
};

#endif // HELPWINDOW_H
