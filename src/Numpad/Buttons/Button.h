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


#ifndef _Button_h_
#define _Button_h_

#include <QLabel>
#include <QTimer>
#include <QList>

class QString;

class Button : public QLabel
{
    Q_OBJECT
public:
  Button(const QString &, QList<int>, QWidget *p_wid = 0);
  ~Button();
  virtual void setSize(int);
  void setPressColor(QString);
  void setNotPressColor(QString);
  void setTextColor(QString);
  void setCheckable(bool);
  void setChecked(bool);
  bool isChecked();
  void setAutoRepeat(bool);

signals:
  void pressed(QList<int>);
  void toggled(int,bool);
private:
  void setPressedView();
  void setNotPressedView();  
  void setStyles();  

  QString m_notPressColor;
  QString m_pressColor;
  QString m_textColor;
  QString m_currentMainColor;

  bool m_checkable;
  bool m_checked;
  bool m_autoRepeated;
  int m_autoRepeatInterval;
  int m_autoRepeatDelay;
  QTimer *pm_delayTimer;
  QTimer *pm_intervalTimer;
  bool m_pressed;  
  QList<int> m_ids;

protected:
  void mousePressEvent(QMouseEvent *);
  void mouseReleaseEvent(QMouseEvent *);
protected slots:
  void intervalTimeout();
  void delayTimeout();
};

#endif
