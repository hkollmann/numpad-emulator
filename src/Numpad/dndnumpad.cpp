#include "dndnumpad.h"
#include "NumpadManager.h"
#include "confbutton.h"
#include <QGridLayout>
#include <QMouseEvent>
#include <QPixmap>
#include <QMenu>
#include <QVBoxLayout>
#include <QPushButton>
#include <QHBoxLayout>
#include <QApplication>
#include <QClipboard>


DndNumpad::DndNumpad(NumpadManager *_nm, QWidget *parent) : QWidget(parent)
{
    nm = _nm;
    setWindowTitle("Configuration numpad");
    QVBoxLayout *layout = new QVBoxLayout;
    QPushButton *applyBtn = new QPushButton("Apply");
    connect(applyBtn, SIGNAL(clicked()), SLOT(slot_applyBtnClicked()));
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->addStretch(1);
    btnLayout->addWidget(applyBtn);
    btnLayout->addStretch(1);
    gridLayout = new QGridLayout;
    QLabel *helpLbl = new QLabel("Left click on the button - for drag and drop.<br>"
                                 "Right click - for copy/paste and size changing.<br>"
                                 "Drag or copy new buttons from the window \"All buttons\".<br>"
                                 "Press Apply below to use the new configuration.<br>"
                                 "Read <a href=\"Help\">Help</a> about how to use Ctrl, Shift and Alt.");
    helpLbl->setOpenExternalLinks(false);
    connect(helpLbl, SIGNAL(linkActivated(const QString&)), SLOT(slot_linkActivated(const QString&)));
    helpLbl->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    helpLbl->setMargin(10);
    layout->addWidget(helpLbl);
    layout->addLayout(gridLayout);
    layout->addLayout(btnLayout);
    setLayout(layout);
    createButtons();
    createMenu();  
}


DndNumpad::~DndNumpad()
{
    for (int i = 0; i <= rowMax; ++i)
        for (int j = 0; j <= columnMax; ++j)
        {
            if (btns[i][j])
            {
                delete btns[i][j];
            }
        }
    for (int i = 0; i <= rowMax; ++i)
    {
        delete [] btns[i];
    }
    delete [] btns;
    if (QApplication::clipboard()->mimeData()->hasFormat("application/x-cpnumpadbtn"))
    {
        QApplication::clipboard()->clear();
    }
}


void DndNumpad::createButtons()
{
    const int capacity = 100;
    QList<BtnDynamicInfo *> btnsDyInfo = nm->getCurrentBtnsConfig();
    if (btnsDyInfo.size() == 0)
    {
        rowTop = 0;
        rowBottom = 5;
        columnLeft = 0;
        columnRight = 5;
    }
    for (int i = 0; i < btnsDyInfo.size(); ++i)
    {
        BtnDynamicInfo *di = btnsDyInfo[i];
        if (i == 0)
        {
            rowTop = di->row;
            rowBottom = di->row;
            columnLeft = di->column;
            columnRight = di->column;
        }
        else
        {
            if (di->row < rowTop)
            {
                rowTop = di->row;
            }
            int r = di->shape == BigVer ? di->row + 1 : di->row;
            if (r > rowBottom)
            {
                rowBottom = r;
            }
            if (di->column < columnLeft)
            {
                columnLeft = di->column;
            }
            int c = di->shape == BigHor ? di->column + 1 : di->column;
            if (c > columnRight)
            {
                columnRight = c;
            }
        }
    }

    rowTop += capacity;
    rowBottom += capacity;
    columnLeft += capacity;
    columnRight += capacity;

    rowMax = rowBottom + capacity;
    columnMax = columnRight + capacity;

    btns = new ConfButton**[rowMax + 1];
    for (int i = 0; i <= rowMax; ++i)
    {
        btns[i] = new ConfButton*[columnMax + 1];
    }
    for (int i = 0; i <= rowMax; ++i)
        for (int j = 0; j <= columnMax; ++j)
        {
            btns[i][j] = NULL;
        }

    for (int i = rowTop - 1; i <= rowBottom + 1; ++i)
        for (int j = columnLeft - 1; j <= columnRight + 1; ++j)
        {
            btns[i][j] = newBtn(i, j);
        }

    for (int i = 0; i < btnsDyInfo.size(); ++i)
    {
        BtnDynamicInfo *di = btnsDyInfo[i];
        QString view;
        for (int j = 0; j < di->ids.size(); ++j)
        {
            int id = di->ids[j];
            view += nm->getBtnsStInfo()[id]->view;
        }

        QString text = "<center>" + view + "</center>";
        di->row += capacity;
        di->column += capacity;
        if (!btns[di->row][di->column])
            continue;
        btns[di->row][di->column]->setBtn(text, di);
        if (di->shape == BigHor)
        {
            int r = di->row;
            int c = di->column + 1;
            if (btns[r][c])
            {
                delete btns[r][c];
            }
            btns[r][c] = NULL;
        }
        else if (di->shape == BigVer)
        {
            int r = di->row + 1;
            int c = di->column;
            if (btns[r][c])
            {
                delete btns[r][c];
            }
            btns[r][c] = NULL;
        }
    }

    for (int i = rowTop - 1; i <= rowBottom + 1; ++i)
        for (int j = columnLeft - 1; j <= columnRight + 1; ++j)
        {
            if (btns[i][j])
            {
                ConfButton *btn = btns[i][j];
                if (btn->isEmpty())
                {
                    gridLayout->addWidget(btn, i, j);
                }
                else
                {
                    switch (btn->getInfo()->shape) {
                    case Sqr:
                        gridLayout->addWidget(btn, i, j);
                        break;
                    case BigHor:
                        gridLayout->addWidget(btn, i, j, 1, 2);
                        break;
                    case BigVer:
                        gridLayout->addWidget(btn, i, j, 2, 1);
                        break;
                    }
                }
            }
        }    
}


ConfButton *DndNumpad::newBtn(int row, int column)
{
    ConfButton *btn = new ConfButton(row, column);
    connect(btn, SIGNAL(btnInserted()), SLOT(slot_btnInserted()));
    connect(btn, SIGNAL(btnRemoved(BtnShape)), SLOT(slot_btnRemoved(BtnShape)));
    return btn;
}


void DndNumpad::createMenu()
{
    menu = new QMenu(this);
    toSqrBtnAction = menu->addAction("Convert to square button",
                                     this, SLOT(slot_toSqrBtn()));
    toBigVerBtnAction = menu->addAction("Convert to big vertical button",
                                        this, SLOT(slot_toBigVerBtn()));
    toBigHorBtnAction = menu->addAction("Convert to big horizontal button",
                                        this, SLOT(slot_toBigHorBtn()));
    menu->addSeparator();
    copyAction = menu->addAction("Copy", this, SLOT(slot_copy()));
    pasteAction = menu->addAction("Paste", this, SLOT(slot_paste()));
}


void DndNumpad::slot_btnInserted()
{
    ConfButton *btn = (ConfButton *)sender();
    if (!btn)
        return;
    int r = btn->getRow();
    int c = btn->getColumn();
    checkBorder(r, c);
}


void DndNumpad::checkBorder(int r, int c)
{
    if (r == rowTop - 1)
    {
        rowTop--;
        if (rowTop != 0)
        {
            int left = columnLeft == 0 ? columnLeft : columnLeft - 1;
            int right = columnRight == columnMax ? columnRight : columnRight + 1;
            for (int j = left; j <= right; ++j)
            {
                btns[rowTop - 1][j] = newBtn(rowTop - 1, j);
                gridLayout->addWidget(btns[rowTop - 1][j], rowTop - 1, j);
            }
        }
    }
    if (r == rowBottom + 1)
    {
        rowBottom++;
        if (rowBottom != rowMax)
        {
            int left = columnLeft == 0 ? columnLeft : columnLeft - 1;
            int right = columnRight == columnMax ? columnRight : columnRight + 1;
            for (int j = left; j <= right; ++j)
            {
                btns[rowBottom + 1][j] = newBtn(rowBottom + 1, j);
                gridLayout->addWidget(btns[rowBottom + 1][j], rowBottom + 1, j);
            }
        }
    }
    if (c == columnLeft - 1)
    {
        columnLeft--;
        if (columnLeft != 0)
        {
            int top = rowTop == 0 ? rowTop : rowTop - 1;
            int bottom = rowBottom == rowMax ? rowBottom : rowBottom + 1;
            for (int i = top; i <= bottom; ++i)
            {
                btns[i][columnLeft - 1] = newBtn(i, columnLeft - 1);
                gridLayout->addWidget(btns[i][columnLeft - 1], i, columnLeft - 1);
            }
        }
    }
    if (c == columnRight + 1)
    {
        columnRight++;
        if (columnRight != columnMax)
        {
            int top = rowTop == 0 ? rowTop : rowTop - 1;
            int bottom = rowBottom == rowMax ? rowBottom : rowBottom + 1;
            for (int i = top; i <= bottom; ++i)
            {
                btns[i][columnRight + 1] = newBtn(i, columnRight + 1);
                gridLayout->addWidget(btns[i][columnRight + 1], i, columnRight + 1);
            }
        }
    }
}


void DndNumpad::slot_btnRemoved(BtnShape shape)
{
    if (shape == BigHor || shape == BigVer)
    {
        ConfButton *btn = (ConfButton *)sender();
        if (!btn)
            return;
        to2Sqr(btn, shape);
    }
}


void DndNumpad::to2Sqr(ConfButton *btn, BtnShape shape)
{
    gridLayout->removeWidget(btn);
    gridLayout->addWidget(btn, btn->getRow(), btn->getColumn());
    int row = shape == BigHor ? btn->getRow() : btn->getRow() + 1;
    int column = shape == BigHor ? btn->getColumn() + 1 : btn->getColumn();
    ConfButton *emptyBtn = newBtn(row, column);
    btns[row][column] = emptyBtn;
    gridLayout->addWidget(emptyBtn, row, column);
}


void DndNumpad::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::RightButton)
    {
        return;
    }
    rcMenuBtn = dynamic_cast<ConfButton *>(childAt(event->pos()));
    if (!rcMenuBtn)
    {
        return;
    }
    if (!rcMenuBtn->isEmpty())
    {
        copyAction->setEnabled(true);
        BtnShape shape = rcMenuBtn->getInfo()->shape;
        if (shape == BigVer || shape == BigHor)
        {
            toSqrBtnAction->setEnabled(true);
            toBigVerBtnAction->setEnabled(false);
            toBigHorBtnAction->setEnabled(false);
        }
        else
        {
            toSqrBtnAction->setEnabled(false);

            int r = rcMenuBtn->getRow();
            int c = rcMenuBtn->getColumn();
            if ((r - 1 >= 0 && btns[r - 1][c] && btns[r - 1][c]->isEmpty()) ||
                    (r + 1 <= rowMax && btns[r + 1][c] && btns[r + 1][c]->isEmpty()))
            {
                toBigVerBtnAction->setEnabled(true);
            }
            else
            {
                toBigVerBtnAction->setEnabled(false);
            }
            if ((c - 1 >= 0 && btns[r][c - 1] && btns[r][c - 1]->isEmpty()) ||
                    (c + 1 <= columnMax && btns[r][c + 1] && btns[r][c + 1]->isEmpty()))
            {
                toBigHorBtnAction->setEnabled(true);
            }
            else
            {
                toBigHorBtnAction->setEnabled(false);
            }
        }
    }
    else
    {
        toSqrBtnAction->setEnabled(false);
        toBigVerBtnAction->setEnabled(false);
        toBigHorBtnAction->setEnabled(false);
        copyAction->setEnabled(false);
    }

    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    if (mimeData->hasFormat("application/x-cpnumpadbtn"))
    {
        pasteAction->setEnabled(true);
    }
    else
    {
        pasteAction->setEnabled(false);
    }

    menu->move(event->globalPos());
    menu->show();
}


void DndNumpad::slot_toSqrBtn()
{
    BtnShape shape = rcMenuBtn->getInfo()->shape;
    if (shape == Sqr)
    {
        return;
    }
    rcMenuBtn->getInfo()->shape = Sqr;
    to2Sqr(rcMenuBtn, shape);
}


void DndNumpad::slot_toBigVerBtn()
{
    if (rcMenuBtn->getInfo()->shape != Sqr)
    {
        return;
    }
    int r = rcMenuBtn->getRow();
    int c = rcMenuBtn->getColumn();
    if (r + 1 <= rowMax && btns[r + 1][c] && btns[r + 1][c]->isEmpty())
    {
        gridLayout->removeWidget(btns[r + 1][c]);
        delete btns[r + 1][c];
        btns[r + 1][c] = NULL;
        gridLayout->removeWidget(btns[r][c]);
        gridLayout->addWidget(btns[r][c], r, c, 2, 1);
        btns[r][c]->getInfo()->shape = BigVer;
        checkBorder(r + 1, c);
    }
    else if (r - 1 >= 0 && btns[r - 1][c] && btns[r - 1][c]->isEmpty())
    {
        BtnDynamicInfo *di = new BtnDynamicInfo(btns[r][c]->getInfo()->ids, BigVer,
                                               r - 1, c);
        btns[r - 1][c]->setBtn(btns[r][c]->text(), di);
        gridLayout->removeWidget(btns[r][c]);
        delete btns[r][c];
        btns[r][c] = NULL;
        gridLayout->removeWidget(btns[r - 1][c]);
        gridLayout->addWidget(btns[r - 1][c], r - 1, c, 2, 1);
        checkBorder(r - 1, c);
    }
}


void DndNumpad::slot_toBigHorBtn()
{
    if (rcMenuBtn->getInfo()->shape != Sqr)
    {
        return;
    }
    int r = rcMenuBtn->getRow();
    int c = rcMenuBtn->getColumn();
    if (c + 1 <= columnMax && btns[r][c + 1] && btns[r][c + 1]->isEmpty())
    {
        gridLayout->removeWidget(btns[r][c + 1]);
        delete btns[r][c + 1];
        btns[r][c + 1] = NULL;
        gridLayout->removeWidget(btns[r][c]);
        gridLayout->addWidget(btns[r][c], r, c, 1, 2);
        btns[r][c]->getInfo()->shape = BigHor;
        checkBorder(r, c + 1);
    }
    else if (c - 1 >= 0 && btns[r][c - 1] && btns[r][c - 1]->isEmpty())
    {
        BtnDynamicInfo *di = new BtnDynamicInfo(btns[r][c]->getInfo()->ids, BigHor,
                                               r, c - 1);
        btns[r][c - 1]->setBtn(btns[r][c]->text(), di);
        gridLayout->removeWidget(btns[r][c]);
        delete btns[r][c];
        btns[r][c] = NULL;
        gridLayout->removeWidget(btns[r][c - 1]);
        gridLayout->addWidget(btns[r][c - 1], r, c - 1, 1, 2);
        checkBorder(r, c - 1);
    }
}


QList<BtnDynamicInfo *> DndNumpad::config()
{
    QList<BtnDynamicInfo *> btnsDyInfo;
    for (int i = rowTop; i <= rowBottom; ++i)
        for (int j = columnLeft; j <= columnRight; ++j)
        {
            ConfButton *btn = btns[i][j];
            if (btn && !btn->isEmpty())
            {
                BtnDynamicInfo *di = btn->getInfo();
                btnsDyInfo << new BtnDynamicInfo(di->ids, di->shape,
                                                 di->row - rowTop,
                                                 di->column - columnLeft);
            }
        }
    return btnsDyInfo;
}


void DndNumpad::slot_applyBtnClicked()
{
    nm->applyVisualConfig(config());
}


void DndNumpad::closeEvent(QCloseEvent *)
{    
    nm->dndNumClose();
}


void DndNumpad::slot_copy()
{
    rcMenuBtn->copy();
}


void DndNumpad::slot_paste()
{
    rcMenuBtn->paste();
}


void DndNumpad::getRatio(int &width, int &height)
{
    width = columnRight + 1 - (columnLeft - 1) + 1;
    height = rowBottom + 1 - (rowTop - 1) + 1;
}


void DndNumpad::slot_linkActivated(const QString &)
{
    nm->showHelp("ctrlShiftAlt");
}

