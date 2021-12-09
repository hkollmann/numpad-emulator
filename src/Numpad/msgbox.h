#ifndef MSGBOX_H
#define MSGBOX_H

#include <QWidget>
#include <QLabel>

class MsgBox : public QWidget
{
public:
    MsgBox(QString msg, QWidget *parent = 0);
    void setMsg(QString msg) { pm_msg->setText(msg); }
private:
    QLabel *pm_msg;
};

#endif // MSGBOX_H
