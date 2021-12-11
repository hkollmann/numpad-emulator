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


#include "AboutWindow.h"
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <fstream>
#include <QApplication>
#include <QDir>
#include <Windows.h>

AboutWindow::AboutWindow(QString _version, QWidget *p_parent /*= 0*/)
: QWidget(p_parent, Qt::WindowTitleHint | Qt::WindowStaysOnTopHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint)
{
    version = _version;

  pm_tabWidget = new QTabWidget;
  
  pm_areaLbl = new QLabel(pm_tabWidget);
  pm_areaLbl->setOpenExternalLinks(true);
  pm_areaLbl->setMargin(10);
  pm_tabWidget->addTab(pm_areaLbl, "");
  
  pm_textBrowser = new QTextBrowser(pm_tabWidget);
  pm_textBrowser->setOpenExternalLinks(true);
  
  QString fileName(QApplication::applicationDirPath() + QDir::separator() + 
                   "license.html");
  QByteArray fileNameByteArray = fileName.toLocal8Bit();
  char *fileNameCStr = fileNameByteArray.data();                   
  
  std::ifstream fileStream(fileNameCStr);
  if (fileStream)
  {
    QString fileContent;
    char chr;
    while (!fileStream.eof())
    {
      chr = fileStream.get();
      if (!fileStream.eof())
      {
        fileContent += chr;  
      }  
    }
      
    fileStream.close();  
    pm_textBrowser->setHtml(fileContent);  
  }
  else
  {
    pm_textBrowser->setHtml("<a href=\"http://www.gnu.org/licenses/gpl-3.0.html\">"
                               "http://www.gnu.org/licenses/gpl-3.0.html</a>");
  }    
  
  pm_tabWidget->addTab(pm_textBrowser, ""); 
      
  pm_closeBtn = new QPushButton;
  
  QHBoxLayout *p_btnLayout = new QHBoxLayout;
  p_btnLayout->addStretch(1);
  p_btnLayout->addWidget(pm_closeBtn);
  p_btnLayout->addStretch(1);
    
  QVBoxLayout *p_vLayout = new QVBoxLayout;
  
  p_vLayout->addWidget(pm_tabWidget);
  p_vLayout->addLayout(p_btnLayout);
  
  setLayout(p_vLayout);
  
  connect(pm_closeBtn, SIGNAL(clicked()), SLOT(close()));
      
  retranslateUi();

  setFixedSize(sizeHint());

  HWND hwnd = (HWND)winId();
  LONG styles = GetWindowLong(hwnd, GWL_EXSTYLE);
  SetWindowLong(hwnd, GWL_EXSTYLE, styles | WS_EX_NOACTIVATE);
}
 
////////////////////////////////////////////////////////////////////////////////

void AboutWindow::retranslateUi()
{
  setWindowTitle(tr("About Numpad"));
  
  pm_tabWidget->setTabText(0, QString(tr("About")));
  pm_tabWidget->setTabText(1, QString(tr("License")));
  
  pm_areaLbl->setText(QString(
    "<br>"
    "<center><img src=\":/numpad_icon.png\" width=\"50\"></center>"
    "<center><H2>Numpad " + version + "<H2></center>"   
    "<center><a href=\"https://sourceforge.net/projects/numpad-emulator/\">sourceforge.net/projects/numpad-emulator</a></center>"
    "<br>"
    "<center>&#169; 2019 ") + QString("Timur Abdrazyakov") + QString("</center>"
    "<center><a href=\"mailto:abtimal@gmail.com\">abtimal@gmail.com</a></center>")
    + QString(
    "<br>"
    "<center><a href=\"https://github.com/hkollmann/numpad-emulator/\">github.com/hkollmann/numpad-emulator</a></center>")
   );
       
  pm_closeBtn->setText(tr("Close"));
}    
