#include "msgbox.h"
#include <QVBoxLayout>
#include <Windows.h>

MsgBox::MsgBox(QString msg, QWidget *parent)
    : QWidget(parent, Qt::WindowTitleHint | Qt::WindowStaysOnTopHint)
{
    setWindowTitle("Numpad");
    pm_msg = new QLabel(msg);
    pm_msg->setMargin(30);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(pm_msg);
    setLayout(layout);
    setFixedSize(sizeHint());

    HWND hwnd = winId();
    LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
    SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_NOACTIVATE);
}
