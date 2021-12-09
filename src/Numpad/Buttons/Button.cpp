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


#include "Buttons/Button.h"
#include <QString>


Button::Button(const QString &text, QList<int> _ids, QWidget *p_wid/*= 0*/)
: QLabel(text, p_wid)
{    
    setNotPressedView();
    setCheckable(false);
    m_autoRepeated = false;
    m_autoRepeatDelay = 600;
    m_autoRepeatInterval = 50;
    pm_delayTimer = NULL;
    pm_intervalTimer = NULL;
    m_ids = _ids;
}

Button::~Button()
{

}

void Button::setSize(int size)
{
  setFixedSize(size, size);
}


void Button::setPressedView()
{   
    m_currentMainColor = m_pressColor;   
    setStyles();
}


void Button::setNotPressedView()
{   
    m_currentMainColor = m_notPressColor;   
    setStyles();
}


void Button::setNotPressColor(QString str)
{
    m_notPressColor = str;
    if (!(m_checkable && m_checked))
    {
        m_currentMainColor = m_notPressColor;
        setStyles();
    }
}


void Button::setPressColor(QString str)
{
    m_pressColor = str;
    if (m_checkable && m_checked)
    {
        m_currentMainColor = m_pressColor;
        setStyles();
    }
}


void Button::setTextColor(QString str)
{
    m_textColor = str;
    setStyles();
}


void Button::setStyles()
{   
    setStyleSheet(m_currentMainColor + m_textColor);
}


void Button::mousePressEvent(QMouseEvent *)
{
    setPressedView();    

    emit pressed(m_ids);
    m_pressed = true;
    if (m_autoRepeated)
    {
        pm_delayTimer->start(m_autoRepeatDelay);
    }
}


void Button::mouseReleaseEvent(QMouseEvent *)
{
    m_pressed = false;
    if (!m_checkable)
    {
        setNotPressedView();        
    }
    else
    {
        m_checked = !m_checked;
        if (!m_checked)
        {
            setNotPressedView();
        }
        emit toggled(m_ids[0], m_checked);
    }
    if (m_autoRepeated)
    {
        pm_delayTimer->stop();
        pm_intervalTimer->stop();
    }
}


void Button::setCheckable(bool mode)
{
    m_checkable = mode;
    m_checked = false;
}


void Button::setChecked(bool checked)
{
    m_checked = checked;
    m_checked ? setPressedView() : setNotPressedView();
    emit toggled(m_ids[0], m_checked);
}


bool Button::isChecked()
{
    return m_checked;
}


void Button::setAutoRepeat(bool mode)
{
    m_autoRepeated = mode;
    if (m_autoRepeated)
    {
        if (!pm_delayTimer)
        {
            pm_delayTimer = new QTimer();
            connect(pm_delayTimer, SIGNAL(timeout()),
                    this, SLOT(delayTimeout()));
            pm_intervalTimer = new QTimer();
            connect(pm_intervalTimer, SIGNAL(timeout()),
                    this, SLOT(intervalTimeout()));
        }
    }
    else if (pm_delayTimer)
    {
        delete pm_delayTimer;
        pm_delayTimer = NULL;
        delete pm_intervalTimer;
        pm_intervalTimer = NULL;
    }
}


void Button::delayTimeout()
{
    pm_delayTimer->stop();
    if (m_pressed)
    {
        pm_intervalTimer->start(m_autoRepeatInterval);
    }
}


void Button::intervalTimeout()
{
    if (m_pressed)
    {
        emit pressed(m_ids);
    }    
    else
    {
        pm_intervalTimer->stop();
    }
}
