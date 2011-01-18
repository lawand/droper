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

#include "dropbox.h"

Dropbox::Dropbox(int apiVersion)
{
    this->apiVersion = apiVersion;
}

QString Dropbox::apiToUrlString(Dropbox::Api api)
{
    switch(api)
    {
    case Dropbox::TOKEN:
        return QString(
                "https://api.dropbox.com/%1/token"
                ).arg(apiVersion);
        break;

    case Dropbox::ACCOUNT_INFO:
        return QString(
                "https://api.dropbox.com/%1/account/info"
                ).arg(apiVersion);
        break;

    case Dropbox::ACCOUNT:
        return QString(
                "https://api.dropbox.com/%1/account"
                ).arg(apiVersion);
        break;

    case Dropbox::FILES:
        return QString(
                "https://api-content.dropbox.com/%1/files/dropbox"
                ).arg(apiVersion);
        break;

    case Dropbox::METADATA:
        return QString(
                "https://api.dropbox.com/%1/metadata/dropbox"
                ).arg(apiVersion);
        break;

    case Dropbox::THUMBNAILS:
        return QString(
                "https://api-content.dropbox.com/%1/thumbnails/dropbox"
                ).arg(apiVersion);
        break;

    case Dropbox::FILEOPS_COPY:
        return QString(
                "https://api.dropbox.com/%1/fileops/copy"
                ).arg(apiVersion);
        break;

    case Dropbox::FILEOPS_CREATEFOLDER:
        return QString(
                "https://api.dropbox.com/%1/fileops/create_folder"
                ).arg(apiVersion);
        break;

    case Dropbox::FILEOPS_DELETE:
        return QString(
                "https://api.dropbox.com/%1/fileops/delete"
                ).arg(apiVersion);
        break;

    case Dropbox::FILEOPS_MOVE:
        return QString(
                "https://api.dropbox.com/%1/fileops/move"
                ).arg(apiVersion);
        break;
    }

}
