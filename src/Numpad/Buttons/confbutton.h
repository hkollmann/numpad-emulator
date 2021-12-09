#ifndef CONFBUTTON_H
#define CONFBUTTON_H

#include <QLabel>
#include "btninfo.h"

class ConfButton : public QLabel
{
    Q_OBJECT
public:
    ConfButton(int, int, QWidget *_parent = 0);
    ~ConfButton();
    void setBtn(QString, BtnDynamicInfo *);
    bool isEmpty() { return m_isEmpty; }
    int getRow() { return m_row; }
    int getColumn() { return m_column; }
    BtnDynamicInfo *getInfo() { return m_dyInfo; }
    void setEmpty();
    void copy();
    void paste();

signals:
    void btnInserted();
    void btnRemoved(BtnShape);

protected:
    void mousePressEvent(QMouseEvent *);
    void dragEnterEvent(QDragEnterEvent *event);   
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

private:
    bool m_isEmpty;
    BtnDynamicInfo *m_dyInfo;
    int m_row;
    int m_column;
    void updateBorder();
    void insertBtn(QByteArray);
};

#endif // CONFBUTTON_H
