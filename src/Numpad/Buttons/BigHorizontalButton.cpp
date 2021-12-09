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


#include "Buttons/BigHorizontalButton.h"
#include <QGridLayout>
#include <QString>

BigHorizontalButton::BigHorizontalButton(QGridLayout *p_gridLayout, 
                                     const QString &text, QList<int> _ids,
                                         QWidget *p_wid/*= 0*/)
: AbstractBigButton(p_gridLayout, text, _ids, p_wid)
{ }

////////////////////////////////////////////////////////////////////////////////

void BigHorizontalButton::setSize(int size)
{
  setFixedSize(2 * size + lengthBetweenButtons(), size);
}

////////////////////////////////////////////////////////////////////////////////

void BigHorizontalButton::correctSize()
{  
  setSize(height());
}                   
