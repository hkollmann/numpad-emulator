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


#ifndef _Numpad_h_
#define _Numpad_h_

#include <QWidget>
#include <QGridLayout>
#include <vector>
#include <Windows.h>
#include <QVector>
#include <QMap>

class Button;
class BigVerticalButton;
class BigHorizontalButton;
class QString;
class QPaintEvent;
class QLabel;
class QTimer;
class NumpadManager;

class Numpad : public QWidget
{
  Q_OBJECT

public:
  Numpad(NumpadManager *, QWidget *pwid = 0);
  ~Numpad();
  void setNoActivateStyle();
  void unsetNoActivateStyle();   
  void setButtonsSize(int, bool correctFontSizeMode);
  void setSpacing(int);
  void setBtnNotPressColor(QString);
  void setBtnPressColor(QString);
  void setBtnTextColor(QString);
  void setBackgroundColor(QString);
  QString getBtnNotPressColor() { return m_btnNotPressColor; }
  QString getBtnPressColor() { return m_btnPressColor; }
  QString getBtnTextColor() { return m_btnTextColor; }
  QString getBackgroundColor() { return m_backgroundColor; }
  int getButtonsSize() { return m_buttonsSize; }
  int getSpacing() { return pm_gridLayout->spacing(); }
  int getMinButtonsSize() { return m_minButtonsSize; }
  int getMaxButtonsSize() { return m_maxButtonsSize; }
  int getMinSpacing() { return m_minSpacing; }
  int getMaxSpacing() { return m_maxSpacing; } 
  QFont getButtonsFont();
  void setButtonsFont(QFont font);
  bool getAltCodeLblMode() { return m_altCodeLblMode; }
  void setAltCodeLblMode(bool) { m_altCodeLblMode = false; } 
  void setMenuVisible(bool);  

signals:  
  void menuActivated();

protected:
  // The method for processing native events from the OS in Qt
  bool nativeEvent(const QByteArray &eventType, void *message, long *result);
  void paintEvent(QPaintEvent *);
  void hideEvent(QHideEvent *);  
  void closeEvent(QCloseEvent *);

protected slots:
  void slot_btnToggled(int id, bool checked);
  void slot_showAltCodeLblTimeout();
  void slot_btnPressed(QList<int>);
  void slot_rstrTimeout(); 

private:
  void createButtons();
  Button *createBtn(QString, int, int, QList<int>);
  BigHorizontalButton *createBigHorBtn(QString, int, int, QList<int>);
  BigVerticalButton *createBigVerBtn(QString, int, int, QList<int>);
  void correctBigButtonsSize();
  void setButtonsColor();
  void simulatePressKey(int);
  void simulateReleaseKey(int);  
  void setWarningColorForAltBtn();
  void setRegularColorForAltBtn();  
  void initAltCodesList();
  void correctAltCodeLblFontSize();
  void createAltCodeLbl();   
  bool loseFocus();

  std::vector<Button *> m_buttons;
  QGridLayout *pm_gridLayout;
  QString m_btnNotPressColor;
  QString m_btnPressColor;
  QString m_btnTextColor;
  QString m_backgroundColor;
  int m_buttonsSize;
  QWidget *pm_buttonsBox;
  int m_minButtonsSize;
  int m_maxButtonsSize;
  int m_minSpacing;
  int m_maxSpacing;
  HWND m_lastFocusWindow;
  bool m_noActivateStyleSet; 
  Button *pm_altBtn;  
  QLabel *pm_altCodeLbl;
  QTimer *pm_showAltCodeLblTimer;
  QVector<QString> m_altCodes;  
  bool m_altCodeLblMode;      
  QMap<int,bool> m_checkedBtns;
  QTimer *pm_rstrTimer;   
  NumpadManager *nm;

  bool allowClose = false;

};

#endif
