#ifndef SOURCEBUTTON_H
#define SOURCEBUTTON_H

#include <QLabel>
#include <QList>

class SourceButton : public QLabel
{
public:
    SourceButton(QString, QList<int>, QWidget *_parent = 0);
    void setBtn(QString, QList<int>);
    void copy();

private:
    QList<int> m_ids;

protected:
    void mousePressEvent(QMouseEvent *);
};

#endif // SOURCEBUTTON_H
