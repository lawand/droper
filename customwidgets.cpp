/****************************************************************************
**
** Copyright 2011, 2012, 2013, 2014 Omar Lawand Dalatieh.
**
** This file is part of Droper.
**
** Droper is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** Droper is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Droper.  If not, see <http://www.gnu.org/licenses/>.
**
** For more information, check out the GNU General Public license found
** in the COPYING file and the README file.
**
****************************************************************************/

// corresponding headers
#include "customwidgets.h"

// member functions
#include <QKeyEvent>

// implementation-specific
#include "util.h"

CustomListWidget::CustomListWidget(QWidget *parent) :
    QListWidget(parent)
{
}

void CustomListWidget::keyPressEvent(QKeyEvent *event)
{
    if(! Util::s60v3())
    {
        switch(event->modifiers())
        {
        case Qt::ControlModifier:
            switch(event->key())
            {
            case Qt::Key_C:
            case Qt::Key_X:
            case Qt::Key_V:
            case Qt::Key_D:
            case Qt::Key_E:
            case Qt::Key_R:
            case Qt::Key_U:
            case Qt::Key_F:
            case Qt::Key_Space:
            case Qt::Key_S:
                QWidget::keyPressEvent(event);
                break;

            default:
                QListWidget::keyPressEvent(event);
                break;
            }
            break;

        default:
            switch(event->key())
            {
            case Qt::Key_Right:
            case Qt::Key_Left:
            case Qt::Key_Backspace:
                QWidget::keyPressEvent(event);
                break;

            default:
                QListWidget::keyPressEvent(event);
                break;
            }
            break;
        }
    }
    else
    {
        switch(event->key())
        {
        case Qt::Key_Right:
        case Qt::Key_Left:
        case Qt::Key_Backspace:
        case Qt::Key_1:
        case Qt::Key_2:
        case Qt::Key_3:
        case Qt::Key_4:
        case Qt::Key_5:
        case Qt::Key_7:
        case Qt::Key_8:
        case Qt::Key_9:
        case Qt::Key_0:
        case Qt::Key_NumberSign:
            QWidget::keyPressEvent(event);
            break;

        default:
            QListWidget::keyPressEvent(event);
            break;
        }
    }
}
