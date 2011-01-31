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
#include "oauth.h"

//data member(s)
#include "consumerdata.h"

//implementation-specific data type(s)
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QStringList>
#include "userdata.h"

OAuth::OAuth(ConsumerData* consumerData)
{
    //member initialization
    this->consumerData = consumerData;
}

QString OAuth::timestampAndNonceParameters()
{
    int currentSecsSinceEpoch = QDateTime::currentMSecsSinceEpoch()/1000;

    return QString(
            "oauth_timestamp=%1&oauth_nonce=%2"
            ).arg(currentSecsSinceEpoch).arg(currentSecsSinceEpoch);
}

QString OAuth::consumerKeyParameter()
{
    return QString("oauth_consumer_key=%1").arg(consumerData->key);
}

QString OAuth::signatureMethodParameter()
{
    return QString("oauth_signature_method=%1").arg("HMAC-SHA1");
}

QString OAuth::signatureParameter(UserData* userData,
                                  QString method,
                                  QString url,
                                  QString urlPath,
                                  QString query)
{
    //url path needs to be UTF-8 encoded and percent encoded
    QStringList urlPathParts = urlPath.split("/");
    for(int i = 0; i < urlPathParts.length(); ++i)
    {
        urlPathParts[i] = urlPathParts[i].toUtf8().toPercentEncoding();
    }
    urlPath = urlPathParts.join("/");

    QByteArray percentEncodedUrlAndPath =
            (url+urlPath).toAscii().toPercentEncoding();

    QStringList queryParts = query.split("&");

    //query values need to be UTF-8 encoded
    for(int i = 0; i < queryParts.length(); ++i)
    {
        QStringList partParts = queryParts[i].split("=");
        partParts[1] = partParts[1].toUtf8();
        queryParts[i] = partParts.join("=");
    }

    //query values need to be percent encoded
    for(int i = 0; i < queryParts.length(); ++i)
    {
        QStringList partParts = queryParts[i].split("=");
        partParts[1] = partParts[1].toAscii().toPercentEncoding();
        queryParts[i] = partParts.join("=");
    }

    queryParts.sort();
    QString sortedQuery = queryParts.join("&");
    QByteArray percentEncodedAndSortedQuery =
            sortedQuery.toAscii().toPercentEncoding();

    QString base = method+
                   "&"+
                   percentEncodedUrlAndPath+
                   "&"+
                   percentEncodedAndSortedQuery;

    QString hash = hmacSha1(base,
                            consumerData->secret + "&" + userData->secret
                            );

    return QString("oauth_signature=%1").arg(QString(hash));
}

QString OAuth::userTokenParameter(UserData* userData)
{
    return QString("oauth_token=%1").arg(userData->token);
}

QString OAuth::hmacSha1(QString base, QString key)
{
    //inner pad
    QByteArray ipad;
    ipad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        ipad[i] = key[i].toAscii();

    //outer pad
    QByteArray opad;
    opad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        opad[i] = key[i].toAscii();

    //XOR operation for inner pad
    for(int i = 0; i < ipad.length(); ++i)
        ipad[i] = ipad[i] ^ 0x36;

    //XOR operation for outer pad
    for(int i = 0; i < opad.length(); ++i)
        opad[i] = opad[i] ^ 0x5c;

    //Hashes inner pad
    QByteArray innerSha1 = QCryptographicHash::hash(
            ipad + base.toAscii(),
            QCryptographicHash::Sha1
            );

    //Hashes outer pad
    QByteArray outerSha1 = QCryptographicHash::hash(
            opad + innerSha1,
            QCryptographicHash::Sha1
            );

    return outerSha1.toBase64();
}
