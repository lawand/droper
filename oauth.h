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

#ifndef OAUTH_H
#define OAUTH_H

// member functions
#include <QString>
#include "common.h"
class QNetworkRequest;
class QUrl;
class UserData;

class OAuth
{
// member functions
public:
    OAuth();
    void signRequestHeader(
        QString method,
        QNetworkRequest *networkRequest,
        UserData *userData = Common::userData
        );
private:
    QString timestampAndNonceHeaderItems();
    QString consumerKeyHeaderItem();
    QString signatureMethodHeaderItem();
    QString userTokenHeaderItem(UserData *userData);
    QString versionHeaderItem();
    QString signatureHeaderItem(
        QString method,
        QUrl *url,
        QString oAuthHeader,
        UserData *userData
        );
    QString hmacSha1(QString base, QString key); // HMAC-SHA1 checksum
};

#endif // OAUTH_H
