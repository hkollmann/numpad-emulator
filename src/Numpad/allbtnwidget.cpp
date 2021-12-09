#include "allbtnwidget.h"
#include "NumpadManager.h"
#include "sourcebutton.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QList>
#include <QMenu>
#include <QMouseEvent>
#include <QRegExp>
#include <QRegExpValidator>


AllBtnWidget::AllBtnWidget(NumpadManager *_nm, QWidget *_parent)
    :QWidget(_parent)
{
    nm = _nm;
    setWindowTitle("All buttons");  
    anyTextLineEdit = new QLineEdit;
    anyTextLineEdit->setPlaceholderText("Any word");
    QPushButton *crBtn = new QPushButton("Create custom button");
    connect(crBtn, SIGNAL(clicked()), SLOT(slot_crBtnClicked()));  
    newBtn = NULL;
    newAnyTextBtn(" ");
    newBtn->setVisible(false);
    errMsgLbl = new QLabel("");
    errMsgLbl->setStyleSheet("color: red;");
    errMsgLbl->setOpenExternalLinks(false);
    connect(errMsgLbl, SIGNAL(linkActivated(const QString&)),
            SLOT(slot_linkActivated(const QString&)));
    QGridLayout *gridLayout = new QGridLayout;  
    gridLayout->addWidget(anyTextLineEdit, 0, 0, 1, 3);
    gridLayout->addWidget(crBtn, 0, 3, 1, 2);
    gridLayout->addWidget(newBtn, 1, 0, 1, 1);
    gridLayout->addWidget(errMsgLbl, 1, 3, 1, 17);
    QLabel *helpLbl = new QLabel("Drag and drop or copy/paste any button to "
                                 "the configuration numpad.<br>"
                                 "Create a custom button with any word or with special symbol.");
    gridLayout->addWidget(helpLbl, 0, 12, 1, 8);
    altCodeLineEdit = new QLineEdit;
    altCodeLineEdit->setPlaceholderText("Alt code");
    QRegExp altCodeRX("\\d{1,4}");
    QValidator *altCodeVal = new QRegExpValidator(altCodeRX, this);
    altCodeLineEdit->setValidator(altCodeVal);
    unicodeLineEdit = new QLineEdit;
    unicodeLineEdit->setPlaceholderText("Unicode");
    QRegExp unicodeRX("[0-9A-Fa-f]{1,4}");
    QValidator *unicodeVal = new QRegExpValidator(unicodeRX, this);
    unicodeLineEdit->setValidator(unicodeVal);
    QPushButton *crAltCodeBtn = new QPushButton("Create special symbol");
    connect(crAltCodeBtn, SIGNAL(clicked()), SLOT(slot_crAltCodeBtnClicked()));
    gridLayout->addWidget(altCodeLineEdit, 0, 5, 1, 2);
    gridLayout->addWidget(unicodeLineEdit, 0, 7, 1, 2);
    gridLayout->addWidget(crAltCodeBtn, 0, 9, 1, 3);
    setLayout(gridLayout);
    createBtns(gridLayout);
    createMenu();
    crBtn->setFocus();
}


void AllBtnWidget::createBtns(QGridLayout *_gridLayout)
{
    QList<BtnDynamicInfo *> btnsDyInfo = nm->getAllBtnsConfig();
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
        SourceButton *btn = new SourceButton(text, di->ids, this);
        _gridLayout->addWidget(btn, di->row + 2, di->column);
    }
    for (int i = 0; i < btnsDyInfo.size(); ++i)
    {
        delete btnsDyInfo[i];
    }
}


void AllBtnWidget::slot_crBtnClicked()
{
    errMsgLbl->setText("");
    if (anyTextLineEdit->text().size() == 0)
    {
        errMsgLbl->setText("Enter any word in the text box above.");
    }
    newAnyTextBtn(anyTextLineEdit->text());
}


void AllBtnWidget::newAnyTextBtn(QString text)
{
    if (text.size() == 0)
        return;
    QString unsupSym;
    QList<int> ids = nm->strToIds(text, unsupSym);
    if (ids.isEmpty())
    {      
        errMsgLbl->setText("Unsupported symbols: " + unsupSym);
        return;
    }
    QString view;
    for (int i = 0; i < ids.size(); ++i)
    {
        int id = ids[i];
        view += nm->getBtnsStInfo()[id]->view;
    }
    if (!newBtn)
    {
        newBtn = new SourceButton("<center>" + view + "</center>", ids, this);
    }
    else
    {
        newBtn->setBtn("<center>" + view + "</center>", ids);
    }
    newBtn->setVisible(true);
    if (!unsupSym.isEmpty())
    {      
        errMsgLbl->setText("Unsupported symbols: " + unsupSym);
    }
}


void AllBtnWidget::createMenu()
{
    menu = new QMenu(this);
    menu->addAction("copy", this, SLOT(slot_copy()));
}


void AllBtnWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::RightButton)
    {
        return;
    }
    rcMenuBtn = dynamic_cast<SourceButton *>(childAt(event->pos()));
    if (!rcMenuBtn)
    {
        return;
    }
    menu->move(event->globalPos());
    menu->show();
}


void AllBtnWidget::slot_copy()
{
    rcMenuBtn->copy();
}


void AllBtnWidget::closeEvent(QCloseEvent *)
{
    nm->allBtnWidClose();
}


void AllBtnWidget::slot_crAltCodeBtnClicked()
{
    errMsgLbl->setText("");
    if (altCodeLineEdit->text().size() == 0 || unicodeLineEdit->text().size() == 0)
    {
        errMsgLbl->setText("Enter both alt code and unicode. Read <a href=\"Help\">Help</a>");
    }
    newAltCodeBtn(altCodeLineEdit->text(), unicodeLineEdit->text());
}


void AllBtnWidget::newAltCodeBtn(QString altCode, QString unicode)
{
    if (altCode.size() == 0 || unicode.size() == 0)
        return;
    int id = nm->addNewBtnInfo(altCode, unicode);
    QString view = nm->getBtnsStInfo()[id]->view;
    if (!newBtn)
    {
        newBtn = new SourceButton("<center>" + view + "</center>", QList<int>()<<id, this);
    }
    else
    {
        newBtn->setBtn("<center>" + view + "</center>", QList<int>()<<id);
    }
    newBtn->setVisible(true);
}


void AllBtnWidget::slot_linkActivated(const QString&)
{
    nm->showHelp("createAltCodeBtn");
}

