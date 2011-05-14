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

#ifndef OAUTH_H
#define OAUTH_H

//data members
class ConsumerData;

//member functions
#include <QPair>
#include <QString>
#include <QNetworkRequest>
class QUrl;
class UserData;

class OAuth
{
public:
    OAuth(ConsumerData* consumerData);

public:
    void signRequest(
        UserData* userData,
        QString method,
        QNetworkRequest* networkRequest
        );
    void addConsumerKeyQueryItem(QNetworkRequest* networkRequest);

public: //shared objects
    ConsumerData* consumerData;

private:
    QString timestampAndNonceItems();
    QString consumerKeyItem();
    QString signatureMethodItem();
    QString userTokenItem(UserData* userData);
    QString versionItem();
    QString signatureItem(
        UserData* userData,
        QString method,
        QUrl* url,
        QString oAuthHeader
        );

    //calculates a HMAC-SHA1 checksum
    QString hmacSha1(QString base, QString key);
};

#endif // OAUTH_H
