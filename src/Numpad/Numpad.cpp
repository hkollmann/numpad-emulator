/*
 * This source file is part of Numpad.
 * Copyright (C) 2013 Timur Abdrazyakov
 * abtimal@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */
 

#include "Numpad.h"
#include "NumpadManager.h"
#include "Buttons/Button.h"
#include "Buttons/BigVerticalButton.h"
#include "Buttons/BigHorizontalButton.h"
#include <typeinfo>
#include <vector>
#include <QString>
#include <QPaintEvent>
#include <QPalette>
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <QLabel>
#include <QTimer>
#include <ctime>
#include <cstdlib>
#include <QFile>
#include <QString>
#include <QSet>
#include <WindowsX.h>
#include <QMenuBar>


#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"user32.lib")

Numpad::Numpad(NumpadManager *_nm, QWidget *p_wid/*= 0*/)
: QWidget(p_wid, Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    nm = _nm;    
  srand(time(0));

  setWindowTitle("Numpad"); 

  pm_gridLayout = new QGridLayout; 

  pm_altBtn = NULL; 
  createButtons();

  pm_buttonsBox = new QWidget;
  pm_buttonsBox->setLayout(pm_gridLayout);

  QHBoxLayout *p_middleLayout = new QHBoxLayout;
  p_middleLayout->addWidget(pm_buttonsBox);
  p_middleLayout->addStretch(1);
  p_middleLayout->setMargin(0);
   
  QVBoxLayout *p_mainLayout = new QVBoxLayout;

  p_mainLayout->addLayout(p_middleLayout);
  p_mainLayout->addStretch(1);
  p_mainLayout->setMargin(0);
   
  setLayout(p_mainLayout);

  QMenuBar *menuBar = new QMenuBar();
  QAction *action = new QAction(QIcon(":/gear.png"), "", menuBar);
  connect(action, SIGNAL(triggered()), this, SIGNAL(menuActivated()));
  menuBar->addAction(action);
  this->layout()->setMenuBar(menuBar);
 
  m_minSpacing = 5;
  m_maxSpacing = 30;

  m_minButtonsSize = 20;
  
  m_maxButtonsSize = QApplication::desktop()->height() / 7;  

  pm_altCodeLbl = NULL;

  m_altCodeLblMode = false;

  setNoActivateStyle();

  pm_rstrTimer = new QTimer(this);
  connect(pm_rstrTimer, SIGNAL(timeout()), this, SLOT(slot_rstrTimeout()));
  connect(qApp, &QCoreApplication::aboutToQuit, [this](){
     allowClose = true;
  });
}        

////////////////////////////////////////////////////////////////////////////////

Numpad::~Numpad()
{
    QMap<int, bool>::const_iterator iter = m_checkedBtns.constBegin();
    while (iter != m_checkedBtns.constEnd())
    {
        if (iter.value() == true)
        {
            simulateReleaseKey(iter.key());
        }
        ++iter;
    }    
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::createButtons()
{    
    QList<BtnDynamicInfo *> btnsDyInfo = nm->getCurrentBtnsConfig();
    QSet<int> checkableBtnIds;
    checkableBtnIds << 69 /*Alt*/ << 124 /*Ctrl*/ << 125 /*Shift*/ ;
    Button *p_btn;    
    for (int i = 0; i < btnsDyInfo.size(); ++i)
    {
        BtnDynamicInfo *di = btnsDyInfo[i];
        QString view;
        for (int j = 0; j < di->ids.size(); ++j)
        {
            int id = di->ids[j];
            view += nm->getBtnsStInfo()[id]->view;
        }

        switch (di->shape) {
        case Sqr:
            p_btn = createBtn("<center>" + view + "</center>",
                              di->row, di->column, di->ids);
            break;
        case BigHor:
            p_btn = createBigHorBtn("<center>" + view + "</center>",
                              di->row, di->column, di->ids);
            break;
        case BigVer:
            p_btn = createBigVerBtn("<center>" + view + "</center>",
                              di->row, di->column, di->ids);
            break;        
        }
        if (checkableBtnIds.contains(di->ids[0]))
        {
            if (di->ids[0] == 69)
            {
                pm_altBtn = p_btn;
            }
            p_btn->setCheckable(true);
            p_btn->setChecked(false);
            connect(p_btn, SIGNAL(toggled(int,bool)), SLOT(slot_btnToggled(int,bool)));
            p_btn->setAutoRepeat(false);
        } else
        {
            connect(p_btn, SIGNAL(pressed(QList<int>)), SLOT(slot_btnPressed(QList<int>)));
            p_btn->setAutoRepeat(true);
        }
    }
    for (int i = 0; i < btnsDyInfo.size(); ++i)
    {
        delete btnsDyInfo[i];
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::createAltCodeLbl()
{    
    initAltCodesList();
    int altCodeMsgIndex = rand() % m_altCodes.size();
    pm_altCodeLbl = new QLabel(m_altCodes[altCodeMsgIndex]);   
    pm_gridLayout->addWidget(pm_altCodeLbl, 5, 2, 1, 2);
}

////////////////////////////////////////////////////////////////////////////////

Button *Numpad::createBtn(QString text, int row, int column, QList<int> ids)
{
  Button *p_btn = new Button(text, ids);
  m_buttons.push_back(p_btn);
  pm_gridLayout->addWidget(p_btn, row, column);
  return p_btn;
}
  
////////////////////////////////////////////////////////////////////////////////

BigHorizontalButton *Numpad::createBigHorBtn(QString text, int row, int column,
                                             QList<int> ids)
{
  BigHorizontalButton *p_btn = new BigHorizontalButton(pm_gridLayout, text, ids);
  m_buttons.push_back(p_btn);
  pm_gridLayout->addWidget(p_btn, row, column, 1, 2);
  return p_btn;
}  

////////////////////////////////////////////////////////////////////////////////

BigVerticalButton *Numpad::createBigVerBtn(QString text, int row, int column,
                                           QList<int> ids)
{
  BigVerticalButton *p_btn = new BigVerticalButton(pm_gridLayout, text, ids);
  m_buttons.push_back(p_btn);
  pm_gridLayout->addWidget(p_btn, row, column, 2, 1);
  return p_btn;
}  

////////////////////////////////////////////////////////////////////////////////

void Numpad::setButtonsSize(int size, bool correctFontSizeMode)
{
  m_buttonsSize = size;
  std::vector<Button *>::iterator iter = m_buttons.begin();
  while (iter != m_buttons.end())
  {    
    (*iter)->setSize(m_buttonsSize);
    iter++;
  }  

  correctAltCodeLblFontSize();

  pm_gridLayout->setMargin(m_buttonsSize / 20);

  if (correctFontSizeMode)
  {
      QFont buttonsFont = getButtonsFont();
      int ratioButtonsAndFontSizes = 3;
      buttonsFont.setPixelSize(m_buttonsSize / ratioButtonsAndFontSizes);
      setButtonsFont(buttonsFont);
  }
}    

////////////////////////////////////////////////////////////////////////////////

void Numpad::setSpacing(int spacing)
{
   pm_gridLayout->setSpacing(spacing);
   correctBigButtonsSize();
}  

///////////////////////////////////////////////////////////////////////////////

void Numpad::correctBigButtonsSize()
{
  std::vector<Button *>::iterator iter = m_buttons.begin();
  AbstractBigButton *p_bigButton;
  while (iter != m_buttons.end())
  {
    p_bigButton = dynamic_cast<AbstractBigButton *> (*iter);
    if (p_bigButton)
    {
      p_bigButton->correctSize();
    }    
    iter++;
  }  
}

////////////////////////////////////////////////////////////////////////////////

      
void Numpad::setBtnNotPressColor(QString color)
{
    m_btnNotPressColor = color;
    QString colorStyle = "background-color: " + m_btnNotPressColor + ";";
    std::vector<Button *>::iterator iter = m_buttons.begin();
    while (iter != m_buttons.end())
    {
      (*iter)->setNotPressColor(colorStyle);
      iter++;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::setBtnPressColor(QString color)
{
    m_btnPressColor = color;
    QString colorStyle = "background-color: " + m_btnPressColor + ";";
    std::vector<Button *>::iterator iter = m_buttons.begin();
    while (iter != m_buttons.end())
    {
      (*iter)->setPressColor(colorStyle);
      iter++;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::setBtnTextColor(QString color)
{
    m_btnTextColor = color;
    QString colorStyle = "color: " + m_btnTextColor + ";";
    std::vector<Button *>::iterator iter = m_buttons.begin();
    while (iter != m_buttons.end())
    {
      (*iter)->setTextColor(colorStyle);
      iter++;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::setBackgroundColor(QString color)
{
  m_backgroundColor = color;  
  QPalette pal = palette();
  pal.setColor(backgroundRole(), QColor(m_backgroundColor));
  setPalette(pal);
}  
    
////////////////////////////////////////////////////////////////////////////////

QFont Numpad::getButtonsFont()
{
    return m_buttons[0]->font();
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::setButtonsFont(QFont font)
{
    std::vector<Button *>::iterator iter = m_buttons.begin();
    while (iter != m_buttons.end())
    {
        (*iter)->setFont(font);
        iter++;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::correctAltCodeLblFontSize()
{
    if (pm_altCodeLbl)
    {
        QFont font;
        font.setPixelSize(m_buttonsSize / 3);
        pm_altCodeLbl->setFont(font);
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::simulatePressKey(int keyCode)
{
    keybd_event(keyCode, MapVirtualKey(keyCode, 0), 0, 0);
}

////////////////////////////////////////////////////////////////////////////////  

void Numpad::simulateReleaseKey(int keyCode)
{    
    keybd_event(keyCode, MapVirtualKey(keyCode, 0), KEYEVENTF_KEYUP, 0);   
}


void Numpad::slot_btnPressed(QList<int> ids)
{
    for (int i = 0; i < ids.size(); ++i)
    {
        bool useAltCode = nm->getBtnsStInfo()[ids[i]]->useAltCode;
        QList<int> codes = nm->getBtnsStInfo()[ids[i]]->codes;
        if (!useAltCode)
        {
            simulatePressKey(codes[0]);
            simulateReleaseKey(codes[0]);
        } else
        {
            if (pm_altBtn && pm_altBtn->isChecked())
            {
                return;
            }
            simulatePressKey(VK_MENU);
            for (int j = 0 ; j < codes.size(); ++j)
            {
                simulatePressKey(codes[j]);
                simulateReleaseKey(codes[j]);
            }
            simulateReleaseKey(VK_MENU);
        }
    }
}


////////////////////////////////////////////////////////////////////////////////


void Numpad::slot_btnToggled(int id, bool checked)
{    
    int keyCode = nm->getBtnsStInfo()[id]->codes[0];
    if (checked)
    {
        simulatePressKey(keyCode);
        m_checkedBtns[keyCode] = true;
    }
    else
    {
        simulateReleaseKey(keyCode);
        m_checkedBtns[keyCode] = false;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::paintEvent(QPaintEvent *)
{
  resize(pm_buttonsBox->width(), pm_buttonsBox->height());
  setFixedSize(sizeHint());
}

////////////////////////////////////////////////////////////////////////////////   
    
bool Numpad::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
  Q_UNUSED(eventType)
  Q_UNUSED(result)
  // Transform the message pointer to the MSG WinAPI
  MSG* p_msg = reinterpret_cast<MSG*>(message);

  if (p_msg->message == WM_NCMOUSEMOVE && m_noActivateStyleSet)
  {
      int ncBotY = this->geometry().y();
      int curY = GET_Y_LPARAM(p_msg->lParam);
      if (curY < ncBotY)
      {
          m_lastFocusWindow = GetForegroundWindow();
          unsetNoActivateStyle();
      }
  }
  if (p_msg->message == WM_NCLBUTTONDOWN)
  {
    if (m_noActivateStyleSet)
    {
        m_lastFocusWindow = GetForegroundWindow();
        unsetNoActivateStyle();
        SetForegroundWindow((HWND)winId());
    } 
  }
  if (p_msg->message == WM_NCMOUSELEAVE && !this->isMinimized())
  {
    SHORT leftButtonPress = GetAsyncKeyState(VK_LBUTTON);
  	if (!m_noActivateStyleSet && (leftButtonPress == 1 || leftButtonPress == 0))
    {
      setNoActivateStyle();
      if (m_lastFocusWindow != (HWND)winId())
      {
          SetForegroundWindow(m_lastFocusWindow);
      }
      else
      {
          loseFocus();
      }
    }
  }
  if (p_msg->message == WM_SIZE && p_msg->wParam == SIZE_MINIMIZED)
  {      
      unsetNoActivateStyle();
  }
  if (p_msg->message == WM_SIZE && p_msg->wParam == SIZE_RESTORED)
  {      
      pm_rstrTimer->start(300);
  }
  return false;
}	

////////////////////////////////////////////////////////////////////////////////

void Numpad::setNoActivateStyle()
{
  HWND hwnd = (HWND)winId();
  LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
  SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_NOACTIVATE);
  m_noActivateStyleSet = true;
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::unsetNoActivateStyle()
{
  HWND hwnd = (HWND)winId();
  LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
  SetWindowLong(hwnd, GWL_EXSTYLE, styles & ~WS_EX_NOACTIVATE);
  m_noActivateStyleSet = false;
}   

////////////////////////////////////////////////////////////////////////////////

void Numpad::hideEvent(QHideEvent *)
{
    std::vector<Button *>::iterator iter = m_buttons.begin();
    while (iter != m_buttons.end())
    {
        Button *btn = *iter;
        if (btn->isChecked())
        {
            btn->setChecked(false);
        }
        iter++;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::slot_showAltCodeLblTimeout()
{
    pm_showAltCodeLblTimer->stop();
    if (pm_altCodeLbl)
    {
        pm_gridLayout->removeWidget(pm_altCodeLbl);
        delete pm_altCodeLbl;
        pm_altCodeLbl = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////

void Numpad::initAltCodesList()
{    
    m_altCodes << "<center>alt 1 alt</center>"
                  "<center>&#x263A;</center>"
               << "<center>alt 2 alt</center>"
                  "<center>&#x263B;</center>"
               << "<center>alt 3 alt</center>"
                  "<center>&#x2665;</center>"
               << "<center>alt 4 alt</center>"
                  "<center>&#x2666;</center>"
               << "<center>alt 5 alt</center>"
                  "<center>&#x2663;</center>"
               << "<center>alt 6 alt</center>"
                  "<center>&#x2660;</center>"
               << "<center>alt 8 alt</center>"
                  "<center>&#x25D8;</center>"
               << "<center>alt 10 alt</center>"
                  "<center>&#x25D9;</center>"
               << "<center>alt 11 alt</center>"
                  "<center>&#x2642;</center>"
               << "<center>alt 12 alt</center>"
                  "<center>&#x2640;</center>"
               << "<center>alt 13 alt</center>"
                  "<center>&#x266A;</center>"
               << "<center>alt 14 alt</center>"
                  "<center>&#x266B;</center>"
               << "<center>alt 15 alt</center>"
                  "<center>&#x263C;</center>"
               << "<center>alt 16 alt</center>"
                  "<center>&#x25BA;</center>"
               << "<center>alt 17 alt</center>"
                  "<center>&#x25C4;</center>"
               << "<center>alt 18 alt</center>"
                  "<center>&#x2195;</center>"
               << "<center>alt 19 alt</center>"
                  "<center>&#x203C;</center>"
               << "<center>alt 24 alt</center>"
                  "<center>&#x2191;</center>"
               << "<center>alt 25 alt</center>"
                  "<center>&#x2193;</center>"
               << "<center>alt 26 alt</center>"
                  "<center>&#x2192;</center>"
               << "<center>alt 27 alt</center>"
                  "<center>&#x2190;</center>"
               << "<center>alt 28 alt</center>"
                  "<center>&#x221F;</center>"
               << "<center>alt 29 alt</center>"
                  "<center>&#x2194;</center>"
               << "<center>alt 30 alt</center>"
                  "<center>&#x25B2;</center>"
               << "<center>alt 31 alt</center>"
                  "<center>&#x25BC;</center>"
               << "<center>alt 127 alt</center>"
                  "<center>&#x2302;</center>";
}


void Numpad::closeEvent(QCloseEvent *ce)
{
    if (allowClose)
      return;

    ce->ignore();
    if (m_lastFocusWindow == (HWND)winId())
    {
        if (loseFocus())
        {
            setNoActivateStyle();
            return;
        }
    }
    unsetNoActivateStyle();
    this->hide();
}


void Numpad::slot_rstrTimeout()
{
    pm_rstrTimer->stop();
    setNoActivateStyle();
    loseFocus();
}


bool Numpad::loseFocus()
{
    HWND hwndTaskbar;
    hwndTaskbar = FindWindow(TEXT("Shell_TrayWnd"), NULL);
    if (hwndTaskbar)
    {
        SetForegroundWindow(hwndTaskbar);
        return true;
    }
    return false;
}


void Numpad::setMenuVisible(bool visible)
{
    this->layout()->menuBar()->setVisible(visible);
}

