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


#include "NumpadManager.h"
#include "KeyboardHookMsg.h"
#include "Numpad.h"
#include "SettingsDialog.h"
#include "AboutWindow.h"
#include "ShowHideKey.h"
#include "msgbox.h"
#include "helpwindow.h"
#include "dndnumpad.h"
#include "allbtnwidget.h"
#include <QMenu>
#include <QApplication>
#include <QColor>
#include <QString>
#include <QCursor>
#include <QDesktopWidget>
#include <QFontMetrics>
#include <QSettings>
#include <QAction>
#include <QDir>
#include <QTimer>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <Shlobj.h>
#include <QFile>
#include <QFileDialog>
#include <QDateTime>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QMimeData>
#include <QClipboard>


#pragma comment(lib,"kernel32.lib")
#pragma comment(lib,"user32.lib")
#pragma comment(lib, "Shell32.lib")

NumpadManager::NumpadManager(QWidget *p_parent/*= 0*/)
: QWidget(p_parent), lastStInfoIndex(164), curStInfoIndex(lastStInfoIndex + 1)
{
    version = "1.8.0";
    appName = "Numpad_" + version;
    confFileName = "NumpadConfig.txt";
    checkConfFile();
    checkExamples();

  setWindowTitle("NumpadManager_" + version);
  winId();

  QApplication::setWindowIcon(QIcon(":/numpad_icon.png"));
  
  QApplication::setQuitOnLastWindowClosed(false);

  connect(qApp, SIGNAL(aboutToQuit()), SLOT(slot_quit()));

  m_defaultButtonsSize = 60;
  m_defaultSpacing = 5;

  m_defaultBtnNotPressColor = QColor(94, 94, 94).name();
  m_defaultBtnTextColor = QColor(Qt::white).name();
  m_defaultBackgroundColor = QColor(57, 57, 57).name();
  m_defaultBtnPressColor = QColor(95, 170, 75).name();

  m_defaultFont.setPixelSize(20);
   
  pm_settings = new QSettings("Numpad", appName, this);

  pm_autoRunSettings = new QSettings ("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\"
                     "CurrentVersion\\Run", QSettings::NativeFormat, this);

  createSystemTray();
  createNumpadMenu();

  loadBtnsStaticInfo();

  createNumpad();
  
  pm_systemTray->show();
                              
  pm_settingsDialog = NULL;
  pm_aboutWindow = NULL;
  helpWindow = NULL;
  dndNumpad = NULL;
  allBtnWid = NULL;
  
  QFontMetrics numpadFont(pm_numpad->font());
  m_diffPosNumpadAndCursor = numpadFont.horizontalAdvance("num");
 
  m_keyboardHookSetGood = false;
  m_hookKeyboard = NULL;    

  m_keyboardHookDll = LoadLibrary(TEXT("KeyboardHook.dll"));
  if (m_keyboardHookDll)
  {
    typedef void (*SetHwnd) (HWND);
    SetHwnd setHwnd = (SetHwnd) GetProcAddress(m_keyboardHookDll, "setAppHwnd");
    if (setHwnd)
    {
      setHwnd((HWND)winId());
      HOOKPROC keyboardHookProc = (HOOKPROC) GetProcAddress(m_keyboardHookDll, 
                                                           "_LowLevelKeyboardProc@12");
      if (keyboardHookProc)
      {
        m_hookKeyboard = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProc,
                                          m_keyboardHookDll, 0);
        if (m_hookKeyboard)
        {
          m_keyboardHookSetGood = true;
        }        
      }
    }
  }
               
  loadShowHideKeysInfo();
  
  m_defaultShowHideKey = "F9";
  setShowHideKey(readKeyFromSettings());
  
//  pm_systemTray->showMessage("", "Numpad");

  pm_errMsgBox = NULL;
  showNewNumpad();
}  


void NumpadManager::showNewNumpad()
{
    if (pm_errMsgBox)
    {
        delete pm_errMsgBox;
        pm_errMsgBox = NULL;
    }
    pm_numpad->setNoActivateStyle();
    pm_numpad->show();    
    if (m_isConfErr)
    {
        pm_errMsgBox = new MsgBox(m_confErrMsg);
        pm_errMsgBox->show();
    }
}

////////////////////////////////////////////////////////////////////////////////

NumpadManager::~NumpadManager()
{
  if (m_hookKeyboard)
  {
    UnhookWindowsHookEx(m_hookKeyboard);
  }
  if (m_keyboardHookDll)
  {
    FreeLibrary(m_keyboardHookDll);
  }

  std::vector<ShowHideKey *>::iterator iter = m_showHideKeysInfo.begin();
  while (iter != m_showHideKeysInfo.end())
  {
    delete *iter;
    iter++;
  }
  pm_settings->setValue("/Settings/xPr", pm_numpad->pos().x());
  pm_settings->setValue("/Settings/yPr", pm_numpad->pos().y());
  delete pm_numpad;

  QList<BtnStaticInfo *> list = m_btnsStInfo.values();
  for (int i = 0; i < list.size(); ++i)
  {
      delete list[i];
  }
}  


void NumpadManager::loadShowHideKeysInfo()
{
    m_showHideKeysInfo.push_back(new ShowHideKey("F1", VK_F1, Qt::Key_F1));
    m_showHideKeysInfo.push_back(new ShowHideKey("F2", VK_F2, Qt::Key_F2));
    m_showHideKeysInfo.push_back(new ShowHideKey("F3", VK_F3, Qt::Key_F3));
    m_showHideKeysInfo.push_back(new ShowHideKey("F4", VK_F4, Qt::Key_F4));
    m_showHideKeysInfo.push_back(new ShowHideKey("F5", VK_F5, Qt::Key_F5));
    m_showHideKeysInfo.push_back(new ShowHideKey("F6", VK_F6, Qt::Key_F6));
    m_showHideKeysInfo.push_back(new ShowHideKey("F7", VK_F7, Qt::Key_F7));
    m_showHideKeysInfo.push_back(new ShowHideKey("F8", VK_F8, Qt::Key_F8));
    m_showHideKeysInfo.push_back(new ShowHideKey("F9", VK_F9, Qt::Key_F9));
    m_showHideKeysInfo.push_back(new ShowHideKey("F10", VK_F10, Qt::Key_F10));
    m_showHideKeysInfo.push_back(new ShowHideKey("F11", VK_F11, Qt::Key_F11));
    m_showHideKeysInfo.push_back(new ShowHideKey("F12", VK_F12, Qt::Key_F12));
    m_showHideKeysInfo.push_back(new ShowHideKey("0", 0x30, Qt::Key_0));
    m_showHideKeysInfo.push_back(new ShowHideKey("1", 0x31, Qt::Key_1));
    m_showHideKeysInfo.push_back(new ShowHideKey("2", 0x32, Qt::Key_2));
    m_showHideKeysInfo.push_back(new ShowHideKey("3", 0x33, Qt::Key_3));
    m_showHideKeysInfo.push_back(new ShowHideKey("4", 0x34, Qt::Key_4));
    m_showHideKeysInfo.push_back(new ShowHideKey("5", 0x35, Qt::Key_5));
    m_showHideKeysInfo.push_back(new ShowHideKey("6", 0x36, Qt::Key_6));
    m_showHideKeysInfo.push_back(new ShowHideKey("7", 0x37, Qt::Key_7));
    m_showHideKeysInfo.push_back(new ShowHideKey("8", 0x38, Qt::Key_8));
    m_showHideKeysInfo.push_back(new ShowHideKey("9", 0x39, Qt::Key_9));
    m_showHideKeysInfo.push_back(new ShowHideKey("A", 0x41, Qt::Key_A));
    m_showHideKeysInfo.push_back(new ShowHideKey("B", 0x42, Qt::Key_B));
    m_showHideKeysInfo.push_back(new ShowHideKey("C", 0x43, Qt::Key_C));
    m_showHideKeysInfo.push_back(new ShowHideKey("D", 0x44, Qt::Key_D));
    m_showHideKeysInfo.push_back(new ShowHideKey("E", 0x45, Qt::Key_E));
    m_showHideKeysInfo.push_back(new ShowHideKey("F", 0x46, Qt::Key_F));
    m_showHideKeysInfo.push_back(new ShowHideKey("G", 0x47, Qt::Key_G));
    m_showHideKeysInfo.push_back(new ShowHideKey("H", 0x48, Qt::Key_H));
    m_showHideKeysInfo.push_back(new ShowHideKey("I", 0x49, Qt::Key_I));
    m_showHideKeysInfo.push_back(new ShowHideKey("J", 0x4A, Qt::Key_J));
    m_showHideKeysInfo.push_back(new ShowHideKey("K", 0x4B, Qt::Key_K));
    m_showHideKeysInfo.push_back(new ShowHideKey("L", 0x4C, Qt::Key_L));
    m_showHideKeysInfo.push_back(new ShowHideKey("M", 0x4D, Qt::Key_M));
    m_showHideKeysInfo.push_back(new ShowHideKey("N", 0x4E, Qt::Key_N));
    m_showHideKeysInfo.push_back(new ShowHideKey("O", 0x4F, Qt::Key_O));
    m_showHideKeysInfo.push_back(new ShowHideKey("P", 0x50, Qt::Key_P));
    m_showHideKeysInfo.push_back(new ShowHideKey("Q", 0x51, Qt::Key_Q));
    m_showHideKeysInfo.push_back(new ShowHideKey("R", 0x52, Qt::Key_R));
    m_showHideKeysInfo.push_back(new ShowHideKey("S", 0x53, Qt::Key_S));
    m_showHideKeysInfo.push_back(new ShowHideKey("T", 0x54, Qt::Key_T));
    m_showHideKeysInfo.push_back(new ShowHideKey("U", 0x55, Qt::Key_U));
    m_showHideKeysInfo.push_back(new ShowHideKey("V", 0x56, Qt::Key_V));
    m_showHideKeysInfo.push_back(new ShowHideKey("W", 0x57, Qt::Key_W));
    m_showHideKeysInfo.push_back(new ShowHideKey("X", 0x58, Qt::Key_X));
    m_showHideKeysInfo.push_back(new ShowHideKey("Y", 0x59, Qt::Key_Y));
    m_showHideKeysInfo.push_back(new ShowHideKey("Z", 0x5A, Qt::Key_Z));
}


void NumpadManager::createNumpad()
{    
  pm_numpad = new Numpad(this);
  connect(pm_numpad, SIGNAL(menuActivated()), this, SLOT(slot_numpadMenuActivated()));

 // pm_numpad->setAltCodeLblMode(readAltCodeLblMode());

  pm_numpad->setBtnNotPressColor(readBtnNotPressColorFromSettings());
  pm_numpad->setBtnTextColor(readBtnTextColorFromSettings());
  pm_numpad->setBackgroundColor(readBackgroundColorFromSettings());
  pm_numpad->setBtnPressColor(readBtnPressColorFromSettings());
  
  readSizeFromSettings();
  pm_numpad->setButtonsSize(m_initButtonsSize, false);
  pm_numpad->setSpacing(m_initSpacing);
  QFont font = readFontFromSettings();
  pm_numpad->setButtonsFont(font);

  pm_numpad->setMenuVisible(readMenuVisibleFromSettings());

  int xPrev = pm_settings->value("/Settings/xPr", 200).toInt();
  int yPrev = pm_settings->value("/Settings/yPr", 200).toInt();
  pm_numpad->move(xPrev, yPrev);
}

////////////////////////////////////////////////////////////////////////////////  

void NumpadManager::createSystemTray()
{
  pm_systemTray = new QSystemTrayIcon(QIcon(":/numpad_icon.png"));
  pm_sysTrayMenu = new QMenu();
  pm_showHideNumpadAction = pm_sysTrayMenu->addAction("Show/Hide numpad", this,
                                                  SLOT(slot_showHideNumpad()));
  pm_sysTrayMenu->addAction("Settings", this, SLOT(slot_numpadSettings()));
  pm_sysTrayMenu->addAction("Add/Remove buttons", this, SLOT(slot_configure()));  
  pm_sysTrayMenu->addAction("Help", this, SLOT(slot_help()));
  pm_sysTrayMenu->addAction("About", this, SLOT(slot_about())); 
  pm_sysTrayMenu->addAction("Quit", qApp, SLOT(quit()));
  connect(pm_systemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          SLOT(slot_systemTrayActivated(QSystemTrayIcon::ActivationReason)));
  pm_systemTray->setToolTip("Numpad");
}

////////////////////////////////////////////////////////////////////////////////

bool NumpadManager::readAltCodeLblMode()
{
    return pm_settings->value("/Settings/AltCodeLblMode", true).toBool();
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::writeAltCodeLblMode()
{
    pm_settings->setValue("/Settings/AltCodeLblMode", pm_numpad->getAltCodeLblMode());
}

////////////////////////////////////////////////////////////////////////////////


QString NumpadManager::readBtnNotPressColorFromSettings()
{
    return pm_settings->value("/Settings/Colors/BtnNotPress", m_defaultBtnNotPressColor).toString();
}


QString NumpadManager::readBtnPressColorFromSettings()
{
    return pm_settings->value("/Settings/Colors/BtnPress", m_defaultBtnPressColor).toString();
}


QString NumpadManager::readBtnTextColorFromSettings()
{
    return pm_settings->value("/Settings/Colors/BtnText", m_defaultBtnTextColor).toString();
}


QString NumpadManager::readBackgroundColorFromSettings()
{
    return pm_settings->value("/Settings/Colors/Background", m_defaultBackgroundColor).toString();
}
    
////////////////////////////////////////////////////////////////////////////////                                           

void NumpadManager::writeBtnNotPressColorToSettings()
{
    pm_settings->setValue("/Settings/Colors/BtnNotPress", pm_numpad->getBtnNotPressColor());
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::writeBtnPressColorToSettings()
{
    pm_settings->setValue("/Settings/Colors/BtnPress", pm_numpad->getBtnPressColor());
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::writeBtnTextColorToSettings()
{
    pm_settings->setValue("/Settings/Colors/BtnText", pm_numpad->getBtnTextColor());
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::writeBackgroundColorToSettings()
{
    pm_settings->setValue("/Settings/Colors/Background", pm_numpad->getBackgroundColor());
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::readSizeFromSettings()
{
  m_initButtonsSize = pm_settings->value("/Settings/Sizes/Buttons", 
                                             m_defaultButtonsSize).toInt(); 
  m_initSpacing = pm_settings->value("/Settings/Sizes/Spacing", 
                                         m_defaultSpacing).toInt();
}


////////////////////////////////////////////////////////////////////////////////    

void NumpadManager::writeButtonsSizeToSettings()
{
    pm_settings->setValue("/Settings/Sizes/Buttons", pm_numpad->getButtonsSize());
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::writeSpacingToSettings()
{
    pm_settings->setValue("/Settings/Sizes/Spacing", pm_numpad->getSpacing());
}

////////////////////////////////////////////////////////////////////////////////

QString NumpadManager::readKeyFromSettings()
{
  return pm_settings->value("/Settings/KeyForShowHideNumpad", 
                            m_defaultShowHideKey).toString();
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::writeKeyToSettings()
{
  pm_settings->setValue("/Settings/KeyForShowHideNumpad", 
                        pm_showHideKey->strRepresent);
}

///////////////////////////////////////////////////////////////////////////////

QFont NumpadManager::readFontFromSettings()
{
    QVariant variant = pm_settings->value("/Settings/Font", m_defaultFont);
    return variant.value<QFont>();
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::writeFontToSettings()
{
    pm_settings->setValue("/Settings/Font", pm_numpad->getButtonsFont());
}

////////////////////////////////////////////////////////////////////////////////

bool NumpadManager::setShowHideKey(const QString & strRepresent)
{
  std::vector<ShowHideKey *>::iterator iter = m_showHideKeysInfo.begin();
  while (iter != m_showHideKeysInfo.end())
  {
    if (strRepresent == (*iter)->strRepresent)
    {
      pm_showHideKey = *iter;
      if (m_keyboardHookSetGood)
      {        
        pm_showHideNumpadAction->setShortcut(pm_showHideKey->qtCode);
        pm_hideNumpadAction->setShortcut(pm_showHideKey->qtCode);
      }         
      return true;
    }
    iter++;
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////                                    

void NumpadManager::slot_showHideNumpad()
{
    if (pm_numpadMenu->isVisible())
    {
        pm_numpadMenu->hide();
    }
  if (pm_numpad->isVisible() && !pm_numpad->isMinimized())
  {
    pm_numpad->unsetNoActivateStyle();
  	pm_numpad->hide();
  }
  else if (pm_numpad->isVisible() && pm_numpad->isMinimized())
  {
      pm_numpad->showNormal();
  }
  else
  {
      pm_numpad->setNoActivateStyle();
      pm_numpad->show();
  }
}  

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::slot_numpadSettings()
{    
  if (!pm_settingsDialog)
  {
    pm_settingsDialog = new SettingsDialog(this, pm_numpad, m_initButtonsSize,
                                           m_initSpacing);
  }
  pm_settingsDialog->show();
  loseFocus();
}


void NumpadManager::slot_configure()
{
    int minSide;
    if (QApplication::desktop()->height() < QApplication::desktop()->width())
    {
        minSide = QApplication::desktop()->height();
    }
    else
    {
        minSide = QApplication::desktop()->width();
    }
    if (!allBtnWid)
    {
        allBtnWid = new AllBtnWidget(this);
        allBtnWid->move(200, 20);        
        allBtnWid->resize(minSide, (minSide / 20.0) * 12);
        allBtnWid->show();
    }
    else
    {
        if (allBtnWid->isVisible() && allBtnWid->isMinimized())
        {
            allBtnWid->showNormal();
        }
        else
        {
            allBtnWid->show();
        }
    }
    allBtnWid->activateWindow();
    allBtnWid->raise();
    if (!dndNumpad)
    {
        dndNumpad = new DndNumpad(this);
        dndNumpad->move(20, 10);
        int widthP, heightP;
        dndNumpad->getRatio(widthP, heightP);
        int maxP = widthP > heightP ? widthP : heightP;
        float oneP = (float)(minSide - 200) / maxP;
        dndNumpad->resize(widthP * oneP, heightP * oneP + 100);
        dndNumpad->show();
    }
    else
    {
        if (dndNumpad->isVisible() && dndNumpad->isMinimized())
        {
            dndNumpad->showNormal();
        }
        else
        {
            dndNumpad->show();
        }
    }
    dndNumpad->activateWindow();
    dndNumpad->raise();   
}


void NumpadManager::slot_reloadConfig()
{
    if (pm_settingsDialog)
    {
        pm_settingsDialog->setEnabled(false);
    }
    int xNumpad = pm_numpad->pos().x();
    int yNumpad = pm_numpad->pos().y();
    delete pm_numpad;
    createNumpad();
    pm_numpad->move(xNumpad, yNumpad);
    showNewNumpad();
    if (pm_settingsDialog)
    {
        pm_settingsDialog->setNumpad(pm_numpad);
        pm_settingsDialog->setEnabled(true);
    }
}

///////////////////////////////////////////////////////////////////////////////

void NumpadManager::slot_about()
{
  if (!pm_aboutWindow)
  {
    pm_aboutWindow = new AboutWindow(version);
  }
  pm_aboutWindow->show();
  loseFocus();
}


void NumpadManager::configure()
{
    slot_configure();
}


void NumpadManager::slot_help(const QString &anchor)
{
  if (!helpWindow)
  {
    helpWindow = new HelpWindow;
  }
  helpWindow->show();
  helpWindow->moveToAnchor(anchor);
  loseFocus();
}

////////////////////////////////////////////////////////////////////////////////    

void NumpadManager::slot_systemTrayActivated(QSystemTrayIcon::ActivationReason)
{    
    int stX = pm_systemTray->geometry().x();
    int stY = pm_systemTray->geometry().y();
    pm_sysTrayMenu->resize(pm_sysTrayMenu->sizeHint());
    pm_sysTrayMenu->move(stX, stY - pm_sysTrayMenu->height());
    pm_sysTrayMenu->show();
    pm_sysTrayMenu->raise();
    pm_sysTrayMenu->activateWindow();
}  

/////////////////////////////////////////////////////////////////////////////////

bool NumpadManager::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)
    // Transform the message pointer to the MSG WinAPI
    MSG* p_msg = reinterpret_cast<MSG*>(message);

    if (p_msg->message == WM_QUERYENDSESSION) {
        qApp->quit();
        return true;
    }

    if (p_msg->message == KEYBOARDHOOKMSG)
    {
      if (p_msg->wParam == pm_showHideKey->winCode)
      {
          slot_showHideNumpad();
      }
      return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////

void NumpadManager::showHideNumpadThroPressKey()
{
  if (pm_numpad->isVisible())
  {
    pm_numpad->unsetNoActivateStyle();
  	pm_numpad->hide();
  }
  else
  {
    int widthScreen = QApplication::desktop()->width();
    int heightScreen = QApplication::desktop()->height();
    int xCenter =  widthScreen / 2;
    int yCenter =  heightScreen / 2;
    int xCursor = QCursor::pos().x();
    int yCursor = QCursor::pos().y();
    
    int xNumpad;
    int yNumpad;
             
    RECT rectNumpad;
    GetWindowRect((HWND)pm_numpad->winId(), &rectNumpad);
    int widthNumpad = rectNumpad.right - rectNumpad.left;
    int heightNumpad = rectNumpad.bottom - rectNumpad.top;
              
    if (xCursor < xCenter && yCursor <= yCenter)
    {
      xNumpad = xCursor + m_diffPosNumpadAndCursor;
      yNumpad = yCursor + m_diffPosNumpadAndCursor;
    }
    if (xCursor >= xCenter && yCursor < yCenter)
    {
      xNumpad = xCursor - m_diffPosNumpadAndCursor - widthNumpad;
      yNumpad = yCursor + m_diffPosNumpadAndCursor;
    }
    if (xCursor > xCenter && yCursor >= yCenter)
    {
      xNumpad = xCursor - m_diffPosNumpadAndCursor - widthNumpad;
      yNumpad = yCursor - m_diffPosNumpadAndCursor - heightNumpad;
    }
    if (xCursor <= xCenter && yCursor > yCenter)
    {
      xNumpad = xCursor + m_diffPosNumpadAndCursor;
      yNumpad = yCursor - m_diffPosNumpadAndCursor - heightNumpad;
    }
    if (xCursor == xCenter && yCursor == yCenter)
    {
      xNumpad = xCursor + m_diffPosNumpadAndCursor;
      yNumpad = yCursor + m_diffPosNumpadAndCursor;
    }    
    
    if (xNumpad + widthNumpad > widthScreen)
    {
      xNumpad = widthScreen - widthNumpad;
    }
    if (yNumpad + heightNumpad > heightScreen)
    {
      yNumpad = heightScreen - heightNumpad;
    }  
    if (xNumpad < 0)
    {
      xNumpad = 0;
    }
    if (yNumpad < 0)
    {
      yNumpad = 0;
    } 
    
    pm_numpad->move(xNumpad, yNumpad);
    pm_numpad->setNoActivateStyle();
  	pm_numpad->show();
  }
}  

////////////////////////////////////////////////////////////////////////////////

bool NumpadManager::isAutoRunSet()
{
    if (pm_autoRunSettings->contains(appName))
    {
        QString autoRunPath =
                pm_autoRunSettings->value(appName).toString();
        QString correctAutoRunPath =
                QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        if (autoRunPath == correctAutoRunPath)
        {
            return true;
        }
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////

void NumpadManager::setAutoRun(bool state)
{
    if (state)
    {
        pm_autoRunSettings->setValue(appName,
             QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    }
    else
    {
        pm_autoRunSettings->remove(appName);
    }
}


void NumpadManager::loseFocus()
{
    HWND hwndTaskbar;
    hwndTaskbar = FindWindow(TEXT("Shell_TrayWnd"), NULL);
    if (hwndTaskbar)
    {
        SetForegroundWindow(hwndTaskbar);
    }
}


void NumpadManager::slot_numpadMenuActivated()
{
    pm_numpadMenu->resize(pm_numpadMenu->sizeHint());
    pm_numpadMenu->move(pm_numpad->pos().x(), pm_numpad->pos().y());
    pm_numpadMenu->show();
    pm_numpadMenu->raise();
    pm_numpadMenu->activateWindow();
}


void NumpadManager::createNumpadMenu()
{
    pm_numpadMenu = new QMenu();
    pm_numpadMenu->addAction("Settings", this, SLOT(slot_numpadSettings()));
    pm_numpadMenu->addAction("Add/Remove buttons", this, SLOT(slot_configure()));   
    pm_numpadMenu->addAction("Help", this, SLOT(slot_help()));
    pm_numpadMenu->addAction("About", this, SLOT(slot_about()));
    pm_numpadMenu->addSeparator();
    pm_hideNumpadAction = pm_numpadMenu->addAction("Hide in system tray", this,
                                                    SLOT(slot_showHideNumpad()));
    pm_numpadMenu->addAction("Quit", qApp, SLOT(quit()));
}


void NumpadManager::setMenuVisible(bool visible)
{
    pm_numpad->setMenuVisible(visible);
    writeMenuVisibleToSettings(visible);
}


void NumpadManager::writeMenuVisibleToSettings(bool visible)
{
    pm_settings->setValue("/Settings/MenuVisible", visible);
}


bool NumpadManager::readMenuVisibleFromSettings()
{
    return pm_settings->value("/Settings/MenuVisible", true).toBool();
}


void NumpadManager::checkConfFile()
{
    wchar_t path[MAX_PATH];
    if(SUCCEEDED(SHGetFolderPath(NULL,
                                 CSIDL_LOCAL_APPDATA,
                                 NULL,
                                 SHGFP_TYPE_CURRENT,
                                 path)))
    {
        QString appDataPath = QDir::fromNativeSeparators(QString::fromWCharArray(path));
        confFileCommonPath = appDataPath + "/Numpad/" + version + "/Buttons";
        fullConfFileName = confFileCommonPath + "/Current/" + confFileName;
        if (!QFile::exists(fullConfFileName))
        {
            QDir dir;
            dir.mkpath(confFileCommonPath + "/Current");
            QFile embedConfFile(":/" + confFileName);
            embedConfFile.copy(fullConfFileName);
            QFile confFile(fullConfFileName);
            QFile::Permissions perm = confFile.permissions();
            perm = perm | QFile::WriteOwner | QFile::WriteUser
                    | QFile::WriteGroup | QFile::WriteOther;
            confFile.setPermissions(perm);
        }
    }   
}


void NumpadManager::checkExamples()
{
    QList<QString> exampleNames;
    exampleNames << "All buttons" << "Any text" << "Phone" << "Standard numpad"
                 << "Numpad with Alt" << "Numpad with arrows";
    for (int i = 0; i < exampleNames.size(); ++i)
    {
        QString examplePath = confFileCommonPath + "/Examples/" + exampleNames[i];
        QString fullExampleName = examplePath + "/" + confFileName;
        if (!QFile::exists(fullExampleName))
        {
            QDir dir;
            dir.mkpath(examplePath);
            QFile embedFile(":/Examples/" + exampleNames[i] + "/" + confFileName);
            embedFile.copy(fullExampleName);
            QFile confFile(fullExampleName);
            QFile::Permissions perm = confFile.permissions();
            perm = perm | QFile::WriteOwner | QFile::WriteUser
                    | QFile::WriteGroup | QFile::WriteOther;
            confFile.setPermissions(perm);
        }
    }
}


void NumpadManager::openConfFileFolder()
{
    bool isOpen = QDesktopServices::openUrl(QUrl::fromLocalFile(confFileCommonPath));
    if (!isOpen)
    {
        MsgBox *msgBox = new MsgBox("Unfortunately, the program can not open "
                                    "the configuration file folder " + confFileCommonPath);
        msgBox->setAttribute(Qt::WA_DeleteOnClose);
        msgBox->show();
    }
}


void NumpadManager::slot_quit()
{
    pm_systemTray->hide();
    if (QApplication::clipboard()->mimeData()->hasFormat("application/x-cpnumpadbtn"))
    {
        QApplication::clipboard()->clear();
    }
}


void NumpadManager::loadOtherConfig()
{    
    QString fileName = QFileDialog::getOpenFileName(pm_settingsDialog, "Open configuration file",
                                                    confFileCommonPath, "*.txt");
    if (!fileName.isEmpty())
    {        
        if (fileName == fullConfFileName)
        {
            return;
        }
        QFile curConfFile(fullConfFileName);
        if (curConfFile.exists())
        {
            QMessageBox::StandardButton answer = QMessageBox::question(pm_settingsDialog,
                                                                       "Save configuration",
                                                                       "Do you want to save your current configuration?",
                                                                       QMessageBox::Yes | QMessageBox::No);
            if (answer == QMessageBox::Yes)
            {
                QString folder = QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss");
                QString savedFileDir = confFileCommonPath + "/Saved/" + folder;
                QDir dir;
                dir.mkpath(savedFileDir);
                curConfFile.copy(savedFileDir + "/" + confFileName);
            }
            curConfFile.remove();
        }
        QFile otherConfFile(fileName);
        otherConfFile.copy(fullConfFileName);
        slot_reloadConfig();
    }
}


void NumpadManager::loadBtnsStaticInfo()
{
    m_btnsStInfo[0] = new BtnStaticInfo("0", false, QList<int>() << VK_NUMPAD0);
    m_btnsStInfo[1] = new BtnStaticInfo("1", false, QList<int>() << VK_NUMPAD1);
    m_btnsStInfo[2] = new BtnStaticInfo("2", false, QList<int>() << VK_NUMPAD2);
    m_btnsStInfo[3] = new BtnStaticInfo("3", false, QList<int>() << VK_NUMPAD3);
    m_btnsStInfo[4] = new BtnStaticInfo("4", false, QList<int>() << VK_NUMPAD4);
    m_btnsStInfo[5] = new BtnStaticInfo("5", false, QList<int>() << VK_NUMPAD5);
    m_btnsStInfo[6] = new BtnStaticInfo("6", false, QList<int>() << VK_NUMPAD6);
    m_btnsStInfo[7] = new BtnStaticInfo("7", false, QList<int>() << VK_NUMPAD7);
    m_btnsStInfo[8] = new BtnStaticInfo("8", false, QList<int>() << VK_NUMPAD8);
    m_btnsStInfo[9] = new BtnStaticInfo("9", false, QList<int>() << VK_NUMPAD9);
    m_btnsStInfo[10] = new BtnStaticInfo("A", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD5);
    m_btnsStInfo[11] = new BtnStaticInfo("B", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD6);
    m_btnsStInfo[12] = new BtnStaticInfo("C", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD7);
    m_btnsStInfo[13] = new BtnStaticInfo("D", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD8);
    m_btnsStInfo[14] = new BtnStaticInfo("E", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD9);
    m_btnsStInfo[15] = new BtnStaticInfo("F", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD0);
    m_btnsStInfo[16] = new BtnStaticInfo("G", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD1);
    m_btnsStInfo[17] = new BtnStaticInfo("H", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD2);
    m_btnsStInfo[18] = new BtnStaticInfo("I", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD3);
    m_btnsStInfo[19] = new BtnStaticInfo("J", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD4);
    m_btnsStInfo[20] = new BtnStaticInfo("K", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD5);
    m_btnsStInfo[21] = new BtnStaticInfo("L", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD6);
    m_btnsStInfo[22] = new BtnStaticInfo("M", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD7);
    m_btnsStInfo[23] = new BtnStaticInfo("N", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD8);
    m_btnsStInfo[24] = new BtnStaticInfo("O", true, QList<int>() << VK_NUMPAD7 << VK_NUMPAD9);
    m_btnsStInfo[25] = new BtnStaticInfo("P", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD0);
    m_btnsStInfo[26] = new BtnStaticInfo("Q", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD1);
    m_btnsStInfo[27] = new BtnStaticInfo("R", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD2);
    m_btnsStInfo[28] = new BtnStaticInfo("S", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD3);
    m_btnsStInfo[29] = new BtnStaticInfo("T", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD4);
    m_btnsStInfo[30] = new BtnStaticInfo("U", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD5);
    m_btnsStInfo[31] = new BtnStaticInfo("V", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD6);
    m_btnsStInfo[32] = new BtnStaticInfo("W", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD7);
    m_btnsStInfo[33] = new BtnStaticInfo("X", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD8);
    m_btnsStInfo[34] = new BtnStaticInfo("Y", true, QList<int>() << VK_NUMPAD8 << VK_NUMPAD9);
    m_btnsStInfo[35] = new BtnStaticInfo("Z", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD0);
    m_btnsStInfo[36] = new BtnStaticInfo("a", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD7);
    m_btnsStInfo[37] = new BtnStaticInfo("b", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD8);
    m_btnsStInfo[38] = new BtnStaticInfo("c", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD9);
    m_btnsStInfo[39] = new BtnStaticInfo("d", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD0);
    m_btnsStInfo[40] = new BtnStaticInfo("e", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD1);
    m_btnsStInfo[41] = new BtnStaticInfo("f", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD2);
    m_btnsStInfo[42] = new BtnStaticInfo("g", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD3);
    m_btnsStInfo[43] = new BtnStaticInfo("h", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD4);
    m_btnsStInfo[44] = new BtnStaticInfo("i", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD5);
    m_btnsStInfo[45] = new BtnStaticInfo("j", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD6);
    m_btnsStInfo[46] = new BtnStaticInfo("k", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD7);
    m_btnsStInfo[47] = new BtnStaticInfo("l", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD8);
    m_btnsStInfo[48] = new BtnStaticInfo("m", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD0 << VK_NUMPAD9);
    m_btnsStInfo[49] = new BtnStaticInfo("n", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD0);
    m_btnsStInfo[50] = new BtnStaticInfo("o", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD1);
    m_btnsStInfo[51] = new BtnStaticInfo("p", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD2);
    m_btnsStInfo[52] = new BtnStaticInfo("q", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD3);
    m_btnsStInfo[53] = new BtnStaticInfo("r", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD4);
    m_btnsStInfo[54] = new BtnStaticInfo("s", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD5);
    m_btnsStInfo[55] = new BtnStaticInfo("t", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD6);
    m_btnsStInfo[56] = new BtnStaticInfo("u", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD7);
    m_btnsStInfo[57] = new BtnStaticInfo("v", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD8);
    m_btnsStInfo[58] = new BtnStaticInfo("w", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD1 << VK_NUMPAD9);
    m_btnsStInfo[59] = new BtnStaticInfo("x", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD0);
    m_btnsStInfo[60] = new BtnStaticInfo("y", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD1);
    m_btnsStInfo[61] = new BtnStaticInfo("z", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD2);
    m_btnsStInfo[62] = new BtnStaticInfo(".<br>dcml", false, QList<int>() << VK_DECIMAL);
    m_btnsStInfo[63] = new BtnStaticInfo("enter", false, QList<int>() << VK_RETURN);
    m_btnsStInfo[64] = new BtnStaticInfo("+", false, QList<int>() << VK_ADD);
    m_btnsStInfo[65] = new BtnStaticInfo("-", false, QList<int>() << VK_SUBTRACT);
    m_btnsStInfo[66] = new BtnStaticInfo("*", false, QList<int>() << VK_MULTIPLY);
    m_btnsStInfo[67] = new BtnStaticInfo("/", false, QList<int>() << VK_DIVIDE);
    m_btnsStInfo[68] = new BtnStaticInfo("&#x2190;", false, QList<int>() << VK_BACK);
    m_btnsStInfo[69] = new BtnStaticInfo("alt", false, QList<int>() << VK_MENU);
    m_btnsStInfo[70] = new BtnStaticInfo("=", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD1);
    m_btnsStInfo[71] = new BtnStaticInfo("~", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD6);
    m_btnsStInfo[72] = new BtnStaticInfo("space", false, QList<int>() << VK_SPACE);
    m_btnsStInfo[73] = new BtnStaticInfo("!", true, QList<int>() << VK_NUMPAD3 << VK_NUMPAD3);
    m_btnsStInfo[74] = new BtnStaticInfo("@", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD4);
    m_btnsStInfo[75] = new BtnStaticInfo("\"", true, QList<int>() << VK_NUMPAD3 << VK_NUMPAD4);
    m_btnsStInfo[76] = new BtnStaticInfo("#", true, QList<int>() << VK_NUMPAD3 << VK_NUMPAD5);
    m_btnsStInfo[77] = new BtnStaticInfo("$", true, QList<int>() << VK_NUMPAD3 << VK_NUMPAD6);
    m_btnsStInfo[78] = new BtnStaticInfo("%", true, QList<int>() << VK_NUMPAD3 << VK_NUMPAD7);
    m_btnsStInfo[79] = new BtnStaticInfo(",", true, QList<int>() << VK_NUMPAD4 << VK_NUMPAD4);
    m_btnsStInfo[80] = new BtnStaticInfo("(", true, QList<int>() << VK_NUMPAD4 << VK_NUMPAD0);
    m_btnsStInfo[81] = new BtnStaticInfo(")", true, QList<int>() << VK_NUMPAD4 << VK_NUMPAD1);
    m_btnsStInfo[82] = new BtnStaticInfo("{", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD3);
    m_btnsStInfo[83] = new BtnStaticInfo("}", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD5);
    m_btnsStInfo[84] = new BtnStaticInfo("[", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD1);
    m_btnsStInfo[85] = new BtnStaticInfo("]", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD3);
    m_btnsStInfo[86] = new BtnStaticInfo("'", true, QList<int>() << VK_NUMPAD3 << VK_NUMPAD9);
    m_btnsStInfo[87] = new BtnStaticInfo("`", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD6);
    m_btnsStInfo[88] = new BtnStaticInfo("&", true, QList<int>() << VK_NUMPAD3 << VK_NUMPAD8);
    m_btnsStInfo[89] = new BtnStaticInfo(":", true, QList<int>() << VK_NUMPAD5 << VK_NUMPAD8);
    m_btnsStInfo[90] = new BtnStaticInfo(";", true, QList<int>() << VK_NUMPAD5 << VK_NUMPAD9);
    m_btnsStInfo[91] = new BtnStaticInfo("&#60;", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD0); // <
    m_btnsStInfo[92] = new BtnStaticInfo("&#62;", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD2); // >
    m_btnsStInfo[93] = new BtnStaticInfo("?", true, QList<int>() << VK_NUMPAD6 << VK_NUMPAD3);
    m_btnsStInfo[94] = new BtnStaticInfo("\\", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD2);
    m_btnsStInfo[95] = new BtnStaticInfo("^", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD4);
    m_btnsStInfo[96] = new BtnStaticInfo("_", true, QList<int>() << VK_NUMPAD9 << VK_NUMPAD5);
    m_btnsStInfo[97] = new BtnStaticInfo("|", true, QList<int>() << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD4);
    m_btnsStInfo[98] = new BtnStaticInfo("F1", false, QList<int>() << VK_F1);
    m_btnsStInfo[99] = new BtnStaticInfo("F2", false, QList<int>() << VK_F2);
    m_btnsStInfo[100] = new BtnStaticInfo("F3", false, QList<int>() << VK_F3);
    m_btnsStInfo[101] = new BtnStaticInfo("F4", false, QList<int>() << VK_F4);
    m_btnsStInfo[102] = new BtnStaticInfo("F5", false, QList<int>() << VK_F5);
    m_btnsStInfo[103] = new BtnStaticInfo("F6", false, QList<int>() << VK_F6);
    m_btnsStInfo[104] = new BtnStaticInfo("F7", false, QList<int>() << VK_F7);
    m_btnsStInfo[105] = new BtnStaticInfo("F8", false, QList<int>() << VK_F8);
    m_btnsStInfo[106] = new BtnStaticInfo("F9", false, QList<int>() << VK_F9);
    m_btnsStInfo[107] = new BtnStaticInfo("F10", false, QList<int>() << VK_F10);
    m_btnsStInfo[108] = new BtnStaticInfo("F11", false, QList<int>() << VK_F11);
    m_btnsStInfo[109] = new BtnStaticInfo("F12", false, QList<int>() << VK_F12);
    m_btnsStInfo[110] = new BtnStaticInfo("esc", false, QList<int>() << VK_ESCAPE);
    m_btnsStInfo[111] = new BtnStaticInfo("tab", false, QList<int>() << VK_TAB);
    m_btnsStInfo[112] = new BtnStaticInfo("print<br>scrn", false, QList<int>() << VK_SNAPSHOT);
    m_btnsStInfo[113] = new BtnStaticInfo("scrl<br>lock", false, QList<int>() << VK_SCROLL);
    m_btnsStInfo[114] = new BtnStaticInfo("pause", false, QList<int>() << VK_PAUSE);
    m_btnsStInfo[115] = new BtnStaticInfo("home", false, QList<int>() << VK_HOME);
    m_btnsStInfo[116] = new BtnStaticInfo("end", false, QList<int>() << VK_END);
    m_btnsStInfo[117] = new BtnStaticInfo("dlt", false, QList<int>() << VK_DELETE);
    m_btnsStInfo[118] = new BtnStaticInfo("page<br>up", false, QList<int>() << VK_PRIOR);
    m_btnsStInfo[119] = new BtnStaticInfo("page<br>down", false, QList<int>() << VK_NEXT);
    m_btnsStInfo[120] = new BtnStaticInfo("&#x25B2;", false, QList<int>() << VK_UP);
    m_btnsStInfo[121] = new BtnStaticInfo("&#x25BC;", false, QList<int>() << VK_DOWN);
    m_btnsStInfo[122] = new BtnStaticInfo("&#x25C4;", false, QList<int>() << VK_LEFT);
    m_btnsStInfo[123] = new BtnStaticInfo("&#x25BA;", false, QList<int>() << VK_RIGHT);
    m_btnsStInfo[124] = new BtnStaticInfo("ctrl", false, QList<int>() << VK_CONTROL);
    m_btnsStInfo[125] = new BtnStaticInfo("shift", false, QList<int>() << VK_SHIFT);
    m_btnsStInfo[127] = new BtnStaticInfo("0 key", false, QList<int>() << 0x30);
    m_btnsStInfo[128] = new BtnStaticInfo("1 key", false, QList<int>() << 0x31);
    m_btnsStInfo[129] = new BtnStaticInfo("2 key", false, QList<int>() << 0x32);
    m_btnsStInfo[130] = new BtnStaticInfo("3 key", false, QList<int>() << 0x33);
    m_btnsStInfo[131] = new BtnStaticInfo("4 key", false, QList<int>() << 0x34);
    m_btnsStInfo[132] = new BtnStaticInfo("5 key", false, QList<int>() << 0x35);
    m_btnsStInfo[133] = new BtnStaticInfo("6 key", false, QList<int>() << 0x36);
    m_btnsStInfo[134] = new BtnStaticInfo("7 key", false, QList<int>() << 0x37);
    m_btnsStInfo[135] = new BtnStaticInfo("8 key", false, QList<int>() << 0x38);
    m_btnsStInfo[136] = new BtnStaticInfo("9 key", false, QList<int>() << 0x39);
    m_btnsStInfo[137] = new BtnStaticInfo("A key", false, QList<int>() << 0x41);
    m_btnsStInfo[138] = new BtnStaticInfo("B key", false, QList<int>() << 0x42);
    m_btnsStInfo[139] = new BtnStaticInfo("C key", false, QList<int>() << 0x43);
    m_btnsStInfo[140] = new BtnStaticInfo("D key", false, QList<int>() << 0x44);
    m_btnsStInfo[141] = new BtnStaticInfo("E key", false, QList<int>() << 0x45);
    m_btnsStInfo[142] = new BtnStaticInfo("F key", false, QList<int>() << 0x46);
    m_btnsStInfo[143] = new BtnStaticInfo("G key", false, QList<int>() << 0x47);
    m_btnsStInfo[144] = new BtnStaticInfo("H key", false, QList<int>() << 0x48);
    m_btnsStInfo[145] = new BtnStaticInfo("I key", false, QList<int>() << 0x49);
    m_btnsStInfo[146] = new BtnStaticInfo("J key", false, QList<int>() << 0x4A);
    m_btnsStInfo[147] = new BtnStaticInfo("K key", false, QList<int>() << 0x4B);
    m_btnsStInfo[148] = new BtnStaticInfo("L key", false, QList<int>() << 0x4C);
    m_btnsStInfo[149] = new BtnStaticInfo("M key", false, QList<int>() << 0x4D);
    m_btnsStInfo[150] = new BtnStaticInfo("N key", false, QList<int>() << 0x4E);
    m_btnsStInfo[151] = new BtnStaticInfo("O key", false, QList<int>() << 0x4F);
    m_btnsStInfo[152] = new BtnStaticInfo("P key", false, QList<int>() << 0x50);
    m_btnsStInfo[153] = new BtnStaticInfo("Q key", false, QList<int>() << 0x51);
    m_btnsStInfo[154] = new BtnStaticInfo("R key", false, QList<int>() << 0x52);
    m_btnsStInfo[155] = new BtnStaticInfo("S key", false, QList<int>() << 0x53);
    m_btnsStInfo[156] = new BtnStaticInfo("T key", false, QList<int>() << 0x54);
    m_btnsStInfo[157] = new BtnStaticInfo("U key", false, QList<int>() << 0x55);
    m_btnsStInfo[158] = new BtnStaticInfo("V key", false, QList<int>() << 0x56);
    m_btnsStInfo[159] = new BtnStaticInfo("W key", false, QList<int>() << 0x57);
    m_btnsStInfo[160] = new BtnStaticInfo("X key", false, QList<int>() << 0x58);
    m_btnsStInfo[161] = new BtnStaticInfo("Y key", false, QList<int>() << 0x59);
    m_btnsStInfo[162] = new BtnStaticInfo("Z key", false, QList<int>() << 0x5A);
    m_btnsStInfo[163] = new BtnStaticInfo(".", true, QList<int>() << VK_NUMPAD4 << VK_NUMPAD6);
    m_btnsStInfo[164] = new BtnStaticInfo(" ", false, QList<int>() << VK_SPACE);
}


QList<BtnDynamicInfo *> NumpadManager::readBtnsDynamicInfo(QString fileName)
{
    QFile confFile(fileName);
    if (!confFile.exists())
    {
        m_confErrMsg = "The configuration file " + fileName + " not found. "
                       "The standard Numpad has been loaded.";
        m_isConfErr = true;
        return loadStandardNmpdInfo();
    }
    if (!confFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_confErrMsg = "The program can not open the configuration file " + fileName + ". "
                       "The standard Numpad has been loaded.";
        m_isConfErr = true;
        return loadStandardNmpdInfo();
    }
    QTextStream stream(&confFile);
    QString str;
    while (!stream.atEnd())
    {
        str = stream.readLine();
        if (str.contains("[BUTTONS]"))
        {
           break;
        }
        if (stream.atEnd())
        {
            m_confErrMsg = "The configuration file " + confFileName + " is incorrect: "
                           "missing section [BUTTONS]. "
                           "The standard Numpad has been loaded.";
            m_isConfErr = true;
            return loadStandardNmpdInfo();
        }
    }
    m_confErrMsg = "Errors in configuration file " + confFileName + ":\n\n";
    m_isConfErr = false;
    QList<BtnDynamicInfo *> btnsDyInfo;
    while (!stream.atEnd())
    {
        str = stream.readLine();
        QList<int> ids;
        QString word;
        QRegExp rx;
        rx.setPattern("(\".+\")");
        int pos = rx.indexIn(str);
        if (pos != -1)
        {
            word = rx.cap(1);
            pos += rx.matchedLength();
            QString unsupSym;
            word = word.remove(0, 1);
            word = word.remove(word.size() - 1, 1);
            ids = strToIds(word, unsupSym);
            if (!unsupSym.isEmpty())
            {
                m_confErrMsg += ("Unsupported symbols: " + unsupSym + "\n");
                m_isConfErr = true;
            }
            if (ids.isEmpty())
            {                
                continue;
            }
        }
        else
        {
            rx.setPattern("(ALT.+ALT)");
            pos = rx.indexIn(str);
            if (pos != -1)
            {
                QString word = rx.cap(1);
                QList<int> codes;
                pos = 0;
                rx.setPattern("(\\d+)");
                while ((pos = rx.indexIn(word, pos)) != -1)
                {
                    codes << rx.cap(1).toInt();
                    pos += rx.matchedLength();
                }
                rx.setPattern("(UNI.+UNI)");
                pos = rx.indexIn(str);
                if (pos != -1)
                {
                    QString view = rx.cap(1);
                    pos += rx.matchedLength();
                    view = view.remove(0, 3);
                    view = view.remove(view.size() - 3, 3);
                    m_btnsStInfo[curStInfoIndex] = new BtnStaticInfo(view, true, codes);
                    ids << curStInfoIndex;
                    ++curStInfoIndex;
                }
            }
        }
        rx.setPattern("(\\d+)");
        QStringList list;
        if (pos == -1)
        {
            pos = 0;
        }
        while ((pos = rx.indexIn(str, pos)) != -1) {
            list << rx.cap(1);
            pos += rx.matchedLength();
        }
        QString confRec(word + " ");
        for (int i = 0; i < list.size(); ++i)
        {
            confRec += (list[i] + " ");
        }
        if (list.size() == 0)
        {
            continue;
        }
        if ((ids.isEmpty() && list.size() != 4) || (!ids.isEmpty() && list.size() != 3))
        {
            m_confErrMsg += ("Incorrect record " + confRec + ".\n");
            m_isConfErr = true;
            continue;
        }
        pos = 0;
        if (ids.isEmpty())
        {
            int id = list[0].toInt();
            if (!m_btnsStInfo.contains(id))
            {
                m_confErrMsg += ("Incorrect ID " + list[0] + " in record " + confRec + ".\n");
                m_isConfErr = true;
                continue;
            }
            ids << id;
            ++pos;
        }
        int shapeId = list[pos].toInt();
        if (shapeId < 1 || shapeId > 3)
        {
            m_confErrMsg += ("Incorrect number " + list[pos] + " for shape in record " + confRec + ".\n");
            m_isConfErr = true;
            continue;
        }
        shapeId--;
        BtnShape shape = (BtnShape)shapeId;
        ++pos;
        int row = list[pos].toInt();
        ++pos;
        int column = list[pos].toInt();
        if (row < 1 || column < 1)
        {
            m_confErrMsg += ("Incorrect position " + list[pos - 1] + " " + list[pos] + " in record " + confRec + ".\n");
            m_isConfErr = true;
            continue;
        }
        row--;
        column--;
        btnsDyInfo << new BtnDynamicInfo(ids, shape, row, column);
    }
    confFile.close();    
    return btnsDyInfo;
}


QList<int> NumpadManager::strToIds(QString word, QString &unsupSym)
{
    QList<int> ids;
    for (int i = 0; i < word.size(); ++i)
    {
        QString c(word[i]);
        QMap<int, BtnStaticInfo *>::const_iterator iter = m_btnsStInfo.constBegin();
        while (iter != m_btnsStInfo.constEnd())
        {
            if (c == iter.value()->view)
            {
                int id = iter.key();
                ids << id;
                break;
            }
            ++iter;
        }
        if (iter == m_btnsStInfo.constEnd())
        {
            unsupSym = unsupSym.append(c);
        }
    }
    return ids;
}


QList<BtnDynamicInfo *>  NumpadManager::loadStandardNmpdInfo()
{
    QList<BtnDynamicInfo *> btnsDyInfo;
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 0, BigHor, 4, 0);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 1, Sqr, 3, 0);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 2, Sqr, 3, 1);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 3, Sqr, 3, 2);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 4, Sqr, 2, 0);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 5, Sqr, 2, 1);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 6, Sqr, 2, 2);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 7, Sqr, 1, 0);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 8, Sqr, 1, 1);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 9, Sqr, 1, 2);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 62, Sqr, 4, 2);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 63, BigVer, 3, 3);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 64, BigVer, 1, 3);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 65, Sqr, 0, 3);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 66, Sqr, 0, 2);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 67, Sqr, 0, 1);
    btnsDyInfo << new BtnDynamicInfo(QList<int>() << 68, Sqr, 0, 0);
    return btnsDyInfo;
}


QList<BtnDynamicInfo *> NumpadManager::getCurrentBtnsConfig()
{
    return readBtnsDynamicInfo(fullConfFileName);
}


QList<BtnDynamicInfo *> NumpadManager::getAllBtnsConfig()
{
    return readBtnsDynamicInfo(":/Examples/All buttons/" + confFileName);
}


void NumpadManager::applyVisualConfig(QList<BtnDynamicInfo *> _btnsDyInfo)
{    
    QFile confFile(fullConfFileName);
    if (!confFile.exists())
    {
        checkConfFile();
    }
    if (!confFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        MsgBox *msg = new MsgBox("The program can not open the configuration file "
                                 + fullConfFileName);
        msg->setAttribute(Qt::WA_DeleteOnClose);
        msg->show();
        return;
    }
    QTextStream fileStream(&confFile);
    QString str;
    QTextStream textStream(&str);
    if (fileStream.atEnd())
    {
        textStream << "[BUTTONS]" << Qt::endl;
    }
    while (!fileStream.atEnd())
    {
        QString s = fileStream.readLine();
        textStream << s << Qt::endl;
        if (s.contains("[BUTTONS]"))
        {
           break;
        }
        if (fileStream.atEnd())
        {
            textStream << "[BUTTONS]" << Qt::endl;
        }
    }
    for (int i = 0; i < _btnsDyInfo.size(); ++i)
    {
        BtnDynamicInfo *di = _btnsDyInfo[i];        
        if (di->ids.size() == 1)
        {
            int id = di->ids[0];
            if (id <= lastStInfoIndex)
            {
                textStream << id << " ";
            }
            else
            {
                textStream << "ALT ";
                QList<int> codes = m_btnsStInfo[id]->codes;
                for (int i = 0; i < codes.size(); ++i)
                {
                    textStream << codes[i] << " ";
                }
                textStream << "ALT UNI" << m_btnsStInfo[id]->view << "UNI ";
            }
        }
        else
        {
            textStream << "\"";
            for (int j = 0; j < di->ids.size(); ++j)
            {
                int id = di->ids[j];
                textStream << m_btnsStInfo[id]->view;
            }
            textStream << "\" ";
        }        
        int shape = di->shape;
        ++shape;
        textStream << shape << " ";
        int row = di->row;
        ++row;
        textStream << row << " ";
        int column = di->column;
        ++column;
        textStream << column << Qt::endl;
    }
    confFile.resize(0);
    fileStream << str;
    confFile.close();
    slot_reloadConfig();
    for (int i = 0; i < _btnsDyInfo.size(); ++i)
    {
        delete _btnsDyInfo[i];
    }
}


void NumpadManager::dndNumClose()
{
    delete dndNumpad;
    dndNumpad = NULL;    
}


void NumpadManager::allBtnWidClose()
{
    delete allBtnWid;
    allBtnWid = NULL;
}


void NumpadManager::showHelp(const QString &anchor)
{
    slot_help(anchor);
}


int NumpadManager::getVirtCode(int num)
{
    QVector<int> codes;
    codes << VK_NUMPAD0 << VK_NUMPAD1 << VK_NUMPAD2 << VK_NUMPAD3 << VK_NUMPAD4
          << VK_NUMPAD5 << VK_NUMPAD6 << VK_NUMPAD7 << VK_NUMPAD8 << VK_NUMPAD9;
    return codes[num];
}


int NumpadManager::addNewBtnInfo(QString altCode, QString unicode)
{
    QList<int> codes;
    for (int i = 0; i < altCode.size(); ++i)
    {
        int n = QString(altCode[i]).toInt();
        codes << getVirtCode(n);
    }
    QString view = "&#x" + unicode + ";";
    m_btnsStInfo[curStInfoIndex] = new BtnStaticInfo(view, true, codes);
    ++curStInfoIndex;
    return curStInfoIndex - 1;
}


