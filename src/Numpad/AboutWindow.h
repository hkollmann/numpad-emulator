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


#ifndef _AboutWindow_h_
#define _AboutWindow_h_

#include <QWidget>

class QLabel;
class QPushButton;
class QTabWidget;
class QTextBrowser;

class AboutWindow : public QWidget
{
  Q_OBJECT

public:
  AboutWindow(QString, QWidget *p_parent = 0);
  void retranslateUi();

private:
  QTabWidget *pm_tabWidget;
  QLabel *pm_areaLbl;
  QTextBrowser *pm_textBrowser;
  QPushButton *pm_closeBtn;    
  QString version;
};

#endif

