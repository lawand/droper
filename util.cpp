/****************************************************************************
**
** Copyright 2011, 2012 Omar Lawand Dalatieh.
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
#include "util.h"

// implementation-specific
#include <QSysInfo>

QString Util::bytesToString(qlonglong bytes)
{
    qreal size = (qreal)bytes;
    QString unit;
    if (size < 1024) {
        unit = "Bytes";
    } else if (size < 1024*1024) {
        size /= 1024;
        unit = "KiB";
    } else if (size < 1024*1024*1024){
        size /= 1024*1024;
        unit = "MiB";
    } else {
        size /= 1024*1024*1024;
        unit = "GiB";
    }

    return QString("%1%2")
        .arg(size, 0, 'f', 1)
        .arg(unit)
            ;
}

bool Util::s60v3()
{
    return (
#ifdef Q_OS_SYMBIAN
        QSysInfo::s60Version() == QSysInfo::SV_S60_3_1 ||
        QSysInfo::s60Version() == QSysInfo::SV_S60_3_2
#else
        false
#endif
        );
}
