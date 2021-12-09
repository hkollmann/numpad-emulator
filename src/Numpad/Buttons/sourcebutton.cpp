#include "sourcebutton.h"
#include <QMimeData>
#include <QDrag>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>


SourceButton::SourceButton(QString _text, QList<int> _ids, QWidget *_parent)
    :QLabel(_text, _parent)
{
    m_ids = _ids;
    setStyleSheet("border-style: solid;"
                  "border-width: 2px;"
                  "border-color: gray;");
    setAcceptDrops(true);
    QFont font = this->font();
    font.setPointSize(11);
    setFont(font);
}


void SourceButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        event->ignore();
        return;
    }
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << text() << m_ids;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dndnumpadbtn", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap::grabWidget(this));
    drag->setHotSpot(event->pos());

    drag->exec(Qt::MoveAction, Qt::MoveAction);
}


void SourceButton::setBtn(QString text, QList<int> _ids)
{
    setText(text);
    m_ids = _ids;
}


void SourceButton::copy()
{
    QClipboard *clipboard = QApplication::clipboard();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << text() << m_ids;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-cpnumpadbtn", itemData);
    clipboard->clear();
    clipboard->setMimeData(mimeData);
}

