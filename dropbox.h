/****************************************************************************
**
** Copyright 2011 Omar Lawand Dalatieh.
** Contact: see the README file.
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

#ifndef DROPBOX_H
#define DROPBOX_H

#include <QString>

class Dropbox
{

public:
    enum Api
    {
        TOKEN,
        ACCOUNT_INFO,
        ACCOUNT,
        FILES,
        METADATA,
        THUMBNAILS,
        FILEOPS_COPY,
        FILEOPS_CREATEFOLDER,
        FILEOPS_DELETE,
        FILEOPS_MOVE
    };

public:
    Dropbox(int apiVersion = 0);

    QString apiToUrlString(Dropbox::Api api);

private:
    int apiVersion;
};

#endif // DROPBOX_H
