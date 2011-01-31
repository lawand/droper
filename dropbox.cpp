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

//corresponding header file(s)
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
                "https://api.dropbox.com/%1/token/"
                ).arg(apiVersion);
        break;

    case Dropbox::ACCOUNT_INFO:
        return QString(
                "https://api.dropbox.com/%1/account/info/"
                ).arg(apiVersion);
        break;

    case Dropbox::ACCOUNT:
        return QString(
                "https://api.dropbox.com/%1/account/"
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
                "https://api.dropbox.com/%1/fileops/copy/"
                ).arg(apiVersion);
        break;

    case Dropbox::FILEOPS_CREATEFOLDER:
        return QString(
                "https://api.dropbox.com/%1/fileops/create_folder/"
                ).arg(apiVersion);
        break;

    case Dropbox::FILEOPS_DELETE:
        return QString(
                "https://api.dropbox.com/%1/fileops/delete/"
                ).arg(apiVersion);
        break;

    case Dropbox::FILEOPS_MOVE:
        return QString(
                "https://api.dropbox.com/%1/fileops/move/"
                ).arg(apiVersion);
        break;
    }
}

Dropbox::Api Dropbox::urlStringToApi(QString urlString)
{

    if(urlString.startsWith("https://api.dropbox.com/"))
    {
        urlString.remove("https://api.dropbox.com/");

        if(urlString.startsWith(QString("%1").arg(apiVersion) + "/"))
        {
            urlString.remove(QString("%1").arg(apiVersion) + "/");

            if(urlString.startsWith("token/"))
                return Dropbox::TOKEN;

            if(urlString.startsWith("account/info/"))
                return Dropbox::ACCOUNT_INFO;

            if(urlString.startsWith("account/"))
                return Dropbox::ACCOUNT;

            if(urlString.startsWith("metadata/dropbox"))
                return Dropbox::METADATA;

            if(urlString.startsWith("fileops/"))
            {
                urlString.remove("fileops/");

                if(urlString.startsWith("copy/"))
                    return Dropbox::FILEOPS_COPY;

                if(urlString.startsWith("create_folder/"))
                    return Dropbox::FILEOPS_CREATEFOLDER;

                if(urlString.startsWith("delete/"))
                    return Dropbox::FILEOPS_DELETE;

                if(urlString.startsWith("move/"))
                    return Dropbox::FILEOPS_MOVE;
            }
        }
    }

    if(urlString.startsWith("https://api-content.dropbox.com/"))
    {
        urlString.remove("https://api-content.dropbox.com/");

        if(urlString.startsWith(QString("%1").arg(apiVersion) + "/"))
        {
            urlString.remove(QString("%1").arg(apiVersion) + "/");

            if(urlString.startsWith("files/dropbox"))
                return Dropbox::FILES;

            if(urlString.startsWith("thumbnails/dropbox"))
                return Dropbox::THUMBNAILS;
        }
    }
}

QString Dropbox::extractMetaDataPath(QString urlString)
{
    urlString.remove(
            QString(
                    "https://api.dropbox.com/%1/metadata/dropbox"
                    ).arg(apiVersion)
            );

    return urlString;
}

QString Dropbox::extractFilePath(QString urlString)
{
    urlString.remove(
            QString(
                    "https://api-content.dropbox.com/%1/files/dropbox"
                    ).arg(apiVersion)
            );

    return urlString;
}
