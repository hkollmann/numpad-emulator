#ifndef ALLBTNWIDGET_H
#define ALLBTNWIDGET_H

#include <QWidget>

class NumpadManager;
class QGridLayout;
class QLineEdit;
class SourceButton;
class QLabel;
class QMenu;

class AllBtnWidget : public QWidget
{
    Q_OBJECT
public:
    AllBtnWidget(NumpadManager *, QWidget *_parent = 0);

protected:
    void mousePressEvent(QMouseEvent *);
    void closeEvent(QCloseEvent *);

protected slots:
    void slot_crBtnClicked();
    void slot_copy();
    void slot_crAltCodeBtnClicked();
    void slot_linkActivated(const QString&);

private:
    NumpadManager *nm;
    QLineEdit *anyTextLineEdit;
    SourceButton *newBtn;
    QLabel *errMsgLbl;
    QMenu *menu;
    SourceButton *rcMenuBtn;
    QLineEdit *altCodeLineEdit;
    QLineEdit *unicodeLineEdit;
    void createBtns(QGridLayout *);
    void newAnyTextBtn(QString text);
    void createMenu();
    void newAltCodeBtn(QString, QString);
};

#endif // ALLBTNWIDGET_H
