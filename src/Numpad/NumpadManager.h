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


#ifndef _NumpadManager_h_
#define _NumpadManager_h_

#include <QWidget>
#include <QSystemTrayIcon>
#include <vector>
#include <Windows.h>
#include <QMap>
#include "btninfo.h"

class SettingsDialog;
class AboutWindow;
struct ShowHideKey;
class QSettings;
class QAction;
class MsgBox;
class HelpWindow;
class Numpad;
class DndNumpad;
class AllBtnWidget;

class NumpadManager : public QWidget
{
  Q_OBJECT

public:
  NumpadManager(QWidget *p_widget = 0);
  ~NumpadManager();
  bool setShowHideKey(const QString &);
  std::vector<ShowHideKey *> getShowHideKeysInfo() { return m_showHideKeysInfo; }
  ShowHideKey *getShowHideKey() { return pm_showHideKey; }
  bool keyboardHookIsSetGood() { return m_keyboardHookSetGood; }
  bool isAutoRunSet();
  void setAutoRun(bool state);
  void writeBtnNotPressColorToSettings();
  void writeBtnPressColorToSettings();
  void writeBtnTextColorToSettings();
  void writeBackgroundColorToSettings();
  QString readBtnNotPressColorFromSettings();
  QString readBtnPressColorFromSettings();
  QString readBtnTextColorFromSettings();
  QString readBackgroundColorFromSettings();
  void writeButtonsSizeToSettings();
  void writeSpacingToSettings();
  void writeFontToSettings();
  void writeAltCodeLblMode();
  void writeKeyToSettings();  
  void configure(); 
  void setMenuVisible(bool);
  void writeMenuVisibleToSettings(bool);
  bool readMenuVisibleFromSettings();
  void openConfFileFolder(); 
  void loadOtherConfig();
  QMap<int,BtnStaticInfo *> & getBtnsStInfo() { return m_btnsStInfo; }
  QList<BtnDynamicInfo *> getCurrentBtnsConfig();
  QList<BtnDynamicInfo *> getAllBtnsConfig();
  void applyVisualConfig(QList<BtnDynamicInfo *>);
  void dndNumClose();
  void allBtnWidClose();
  QList<int> strToIds(QString, QString &);
  void showHelp(const QString &);
  int addNewBtnInfo(QString, QString);

protected:
  // The method for processing native events from the OS in Qt
  bool nativeEvent(const QByteArray &eventType, void *message, long *result);

protected slots:
  void slot_showHideNumpad();
  void slot_numpadSettings();
  void slot_about();
  void slot_systemTrayActivated(QSystemTrayIcon::ActivationReason);  
  void slot_configure();
  void slot_reloadConfig();
  void slot_numpadMenuActivated();
  void slot_quit();
  void slot_help(const QString &anchor = "start");

private:
  void createNumpad();
  void createSystemTray();  
  void readSizeFromSettings();
  QString readKeyFromSettings();  
  QFont readFontFromSettings();
  bool readAltCodeLblMode();
  void showHideNumpadThroPressKey();
  void showNewNumpad();
  void loseFocus();
  void createNumpadMenu();
  void checkConfFile();
  void loadShowHideKeysInfo();
  void checkExamples();
  void loadBtnsStaticInfo();
  QList<BtnDynamicInfo *> readBtnsDynamicInfo(QString);
  QList<BtnDynamicInfo *> loadStandardNmpdInfo();  
  int getVirtCode(int);

  Numpad *pm_numpad;
  QSystemTrayIcon *pm_systemTray;  
  SettingsDialog *pm_settingsDialog;
  AboutWindow *pm_aboutWindow;
  int m_initButtonsSize;
  int m_initSpacing;
  QString m_defaultBtnNotPressColor;
  QString m_defaultBtnPressColor;
  QString m_defaultBtnTextColor;
  QString m_defaultBackgroundColor;
  int m_defaultButtonsSize;
  int m_defaultSpacing;
  QFont m_defaultFont;
  QSettings *pm_settings;
  QSettings *pm_autoRunSettings;
  int m_diffPosNumpadAndCursor;
  HHOOK m_hookKeyboard; 
  HINSTANCE m_keyboardHookDll; 
  ShowHideKey *pm_showHideKey;
  std::vector<ShowHideKey *> m_showHideKeysInfo;
  QString m_defaultShowHideKey;
  bool m_keyboardHookSetGood;
  QAction *pm_showHideNumpadAction;
  QMenu *pm_sysTrayMenu;
  MsgBox *pm_errMsgBox;
  QMenu *pm_numpadMenu;
  QAction *pm_hideNumpadAction;
  QString version;
  QString appName;
  QString confFileName;
  QString confFileCommonPath;
  QString fullConfFileName;
  HelpWindow *helpWindow;
  QMap<int,BtnStaticInfo *> m_btnsStInfo;
  QString m_confErrMsg;
  bool m_isConfErr;
  DndNumpad *dndNumpad;
  AllBtnWidget *allBtnWid;
  const int lastStInfoIndex;
  int curStInfoIndex;
};

#endif
