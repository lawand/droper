/****************************************************************************
**
** Copyright 2011 Omar Lawand Dalatieh.
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
//shared data members
public:
    ConsumerData *consumerData;

//member functions
public:
    OAuth(ConsumerData *consumerData);
    void signRequestHeader(
        QString method,
        QNetworkRequest *networkRequest,
        UserData *userData = 0
        );
    void signRequestUrl(
        QString method,
        QNetworkRequest *networkRequest,
        UserData *userData = 0
        );
private:
    QString timestampAndNonceHeaderItems();
    QString consumerKeyHeaderItem();
    QString signatureMethodHeaderItem();
    QString userTokenHeaderItem(UserData *userData);
    QString versionHeaderItem();
    QString signatureHeaderItem(
        UserData *userData,
        QString method,
        QUrl *url,
        QString oAuthHeader
        );
    void timestampAndNonceUrlItems(QUrl *url);
    void consumerKeyUrlItem(QUrl *url);
    void signatureMethodUrlItem(QUrl *url);
    void userTokenUrlItem(QUrl *url, UserData *userData);
    void versionUrlItem(QUrl *url);
    void signatureUrlItem(
        QUrl *url,
        UserData *userData,
        QString method
        );
    QString hmacSha1(QString base, QString key); //HMAC-SHA1 checksum
};

#endif // OAUTH_H
