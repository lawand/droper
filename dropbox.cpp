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
#include "dropbox.h"

Dropbox::Dropbox(int apiVersion)
{
    this->apiVersion = apiVersion;
}

QUrl Dropbox::apiToUrl(Dropbox::Api api)
{
    switch(api)
    {
    case Dropbox::OAUTH_AUTHORIZE:
        return QUrl(
            QString(
            "https://www.dropbox.com/%1/oauth/authorize"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::OAUTH_REQUESTTOKEN:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/oauth/request_token"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::OAUTH_ACCESSTOKEN:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/oauth/access_token"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::ACCOUNT_INFO:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/account/info"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::METADATA:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/metadata/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::REVISIONS:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/revisions/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::RESTORE:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/restore/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::SEARCH:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/search/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::SHARES:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/shares/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::MEDIA:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/media/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::FILEOPS_COPY:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/fileops/copy"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::FILEOPS_CREATEFOLDER:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/fileops/create_folder"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::FILEOPS_DELETE:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/fileops/delete"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::FILEOPS_MOVE:
        return QUrl(
            QString(
            "https://api.dropbox.com/%1/fileops/move"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::FILES:
        return QUrl(
            QString(
            "https://api-content.dropbox.com/%1/files/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::FILESPUT:
        return QUrl(
            QString(
            "https://api-content.dropbox.com/%1/files_put/dropbox"
            ).arg(apiVersion)
            );
        break;

    case Dropbox::THUMBNAILS:
        return QUrl(
            QString(
            "https://api-content.dropbox.com/%1/thumbnails/dropbox"
            ).arg(apiVersion)
            );
        break;
    }
}

Dropbox::Api Dropbox::urlToApi(QUrl url)
{
    if(url.toString().startsWith("https://www.dropbox.com"))
    {
        QString path = url.path();

        if(path.startsWith("/" + QString("%1").arg(apiVersion)))
        {
            path.remove("/" + QString("%1").arg(apiVersion));

            if(path.startsWith("/oauth/authorize"))
                return OAUTH_AUTHORIZE;
        }
    }

    if(url.toString().startsWith("https://api.dropbox.com"))
    {
        QString path = url.path();

        if(path.startsWith("/" + QString("%1").arg(apiVersion)))
        {
            path.remove("/" + QString("%1").arg(apiVersion));

            if(path.startsWith("/oauth/request_token"))
                return Dropbox::OAUTH_REQUESTTOKEN;

            if(path.startsWith("/oauth/access_token"))
                return Dropbox::OAUTH_ACCESSTOKEN;

            if(path.startsWith("/account/info"))
                return Dropbox::ACCOUNT_INFO;

            if(path.startsWith("/metadata"))
                return Dropbox::METADATA;

            if(path.startsWith("/revisions"))
                return Dropbox::REVISIONS;

            if(path.startsWith("/restore"))
                return Dropbox::RESTORE;

            if(path.startsWith("/search"))
                return Dropbox::SEARCH;

            if(path.startsWith("/shares"))
                return Dropbox::SHARES;

            if(path.startsWith("/media"))
                return Dropbox::MEDIA;

            if(path.startsWith("/fileops/copy"))
                return Dropbox::FILEOPS_COPY;

            if(path.startsWith("/fileops/create_folder"))
                return Dropbox::FILEOPS_CREATEFOLDER;

            if(path.startsWith("/fileops/delete"))
                return Dropbox::FILEOPS_DELETE;

            if(path.startsWith("/fileops/move"))
                return Dropbox::FILEOPS_MOVE;
        }
    }

    if(url.toString().startsWith("https://api-content.dropbox.com"))
    {
        QString path = url.path();

        if(path.startsWith("/" + QString("%1").arg(apiVersion)))
        {
            path.remove("/" + QString("%1").arg(apiVersion));

            if(path.startsWith("/files_put"))
                return Dropbox::FILESPUT;

            if(path.startsWith("/files"))
                return Dropbox::FILES;

            if(path.startsWith("/thumbnails"))
                return Dropbox::THUMBNAILS;
        }
    }
}

QString Dropbox::metaDataPathFromUrl(QUrl url)
{
    QString path = url.path();

    QString metaDataPath = path;
    metaDataPath = metaDataPath.remove(
        QString("/%1/metadata/dropbox").arg(apiVersion)
        );

    return metaDataPath;
}

QString Dropbox::filePathFromUrl(QUrl url)
{
    QString path = url.path();

    QString filePath = path;
    filePath = filePath.remove(
        QString("/%1/files/dropbox").arg(apiVersion)
        );

    return filePath;
}
