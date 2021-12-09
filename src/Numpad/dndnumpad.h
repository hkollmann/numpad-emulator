#ifndef DNDNUMPAD_H
#define DNDNUMPAD_H

#include <QWidget>
#include "btninfo.h"

class QGridLayout;
class NumpadManager;
class ConfButton;
class QMenu;
class QAction;

class DndNumpad : public QWidget
{
    Q_OBJECT
public:
    DndNumpad(NumpadManager *, QWidget *parent = nullptr);
    ~DndNumpad();
    void getRatio(int &, int &);

signals:

protected slots:
    void slot_btnInserted();
    void slot_btnRemoved(BtnShape);
    void slot_toSqrBtn();
    void slot_toBigVerBtn();
    void slot_toBigHorBtn();
    void slot_applyBtnClicked();    
    void slot_copy();
    void slot_paste();
    void slot_linkActivated(const QString&);

protected:
    void mousePressEvent(QMouseEvent *);
    void closeEvent(QCloseEvent *);

private:
    QGridLayout *gridLayout;
    NumpadManager *nm;
    ConfButton ***btns;
    int rowTop;
    int rowBottom;
    int columnLeft;
    int columnRight;
    int columnMax;
    int rowMax;
    QMenu *menu;
    QAction *toSqrBtnAction;
    QAction *toBigVerBtnAction;
    QAction *toBigHorBtnAction;
    QAction *copyAction;
    QAction *pasteAction;
    ConfButton *rcMenuBtn;

    void createButtons();
    void createMenu();
    void to2Sqr(ConfButton *, BtnShape);
    ConfButton *newBtn(int,int);
    void checkBorder(int,int);
    QList<BtnDynamicInfo *> config();
};

#endif // DNDNUMPAD_H
