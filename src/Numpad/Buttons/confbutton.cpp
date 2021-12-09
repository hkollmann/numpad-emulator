#include "confbutton.h"
#include <QMimeData>
#include <QDrag>
#include <QMouseEvent>
#include <QPixmap>
#include <QApplication>
#include <QClipboard>


ConfButton::ConfButton(int _row, int _column, QWidget *_parent)
    : QLabel("  ", _parent)
{
    setAcceptDrops(true);
    m_isEmpty = true;
    m_dyInfo = NULL;
    m_row = _row;
    m_column = _column;
    updateBorder();   
    QFont font = this->font();
    font.setPointSize(11);
    setFont(font);
}


void ConfButton::updateBorder()
{
    if (m_isEmpty)
    {
        setStyleSheet("border-style: dotted;"
                      "border-width: 2px;"
                      "border-color: gray;");
    }
    else
    {
        setStyleSheet("border-style: solid;"
                      "border-width: 2px;"
                      "border-color: gray;");
    }
}


void ConfButton::setBtn(QString _text, BtnDynamicInfo *_dyInfo)
{
    setText(_text);
    if (m_dyInfo)
    {
        delete m_dyInfo;
    }
    m_dyInfo = _dyInfo;
    m_isEmpty = false;
    updateBorder();
}


void ConfButton::setEmpty()
{
    setText("");
    if (m_dyInfo)
    {
        delete m_dyInfo;
    }
    m_dyInfo = NULL;
    m_isEmpty = true;
    updateBorder();
}


ConfButton::~ConfButton()
{
    if (m_dyInfo)
    {
        delete m_dyInfo;
    }
}


void ConfButton::mousePressEvent(QMouseEvent *event)
{    
    if (event->button() == Qt::RightButton)
    {
        event->ignore();
        return;
    }
    if (isEmpty())
    {
        return;
    }

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << text() << m_dyInfo->ids;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dndnumpadbtn", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap::grabWidget(this));
    drag->setHotSpot(event->pos());

    BtnShape shape = m_dyInfo->shape;
    setEmpty();
    emit btnRemoved(shape);

    drag->exec(Qt::MoveAction, Qt::MoveAction);
}


void ConfButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dndnumpadbtn"))
    {
        event ->acceptProposedAction();      
    }
    else
    {
        event->ignore();
    }
}

void ConfButton::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dndnumpadbtn"))
    {
        event ->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}


void ConfButton::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-dndnumpadbtn"))
    {
        QByteArray itemData = event->mimeData()->data("application/x-dndnumpadbtn");
        insertBtn(itemData);
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}


void ConfButton::insertBtn(QByteArray itemData)
{
    QDataStream dataStream(&itemData, QIODevice::ReadOnly);
    QString text;
    QList<int> ids;
    dataStream >> text >> ids;
    BtnShape shape = m_isEmpty ? Sqr : m_dyInfo->shape;
    BtnDynamicInfo *dyInfo = new BtnDynamicInfo(ids, shape, m_row, m_column);
    setBtn(text, dyInfo);
    emit btnInserted();
}


void ConfButton::copy()
{
    QClipboard *clipboard = QApplication::clipboard();
    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << text() << m_dyInfo->ids;
    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-cpnumpadbtn", itemData);
    clipboard->clear();
    clipboard->setMimeData(mimeData);
}


void ConfButton::paste()
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasFormat("application/x-cpnumpadbtn"))
    {
        QByteArray itemData = mimeData->data("application/x-cpnumpadbtn");
        insertBtn(itemData);
    }
}

