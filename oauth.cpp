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

//corresponding headers
#include "oauth.h"

//data members
#include "consumerdata.h"

//member functions
#include "userdata.h"

//implementation-specific
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QStringList>

OAuth::OAuth(ConsumerData* consumerData)
{
    //member initialization
    this->consumerData = consumerData;
}

QPair<QString,QString> OAuth::timestampQueryItem()
{
    int currentSecsSinceEpoch = QDateTime::currentDateTime().toUTC().toTime_t();

    return qMakePair(
            QString("oauth_timestamp"),
            QString("%1").arg(currentSecsSinceEpoch)
            );
}

QPair<QString,QString> OAuth::nonceQueryItem(qint64 timestamp)
{
    return qMakePair(
            QString("oauth_nonce"),
            QString("%1").arg(timestamp)
            );
}

QPair<QString,QString> OAuth::consumerKeyQueryItem()
{
    return qMakePair(
            QString("oauth_consumer_key"),
            QString("%1").arg(consumerData->key)
            );
}

QPair<QString,QString> OAuth::signatureMethodQueryItem()
{
    return qMakePair(
            QString("oauth_signature_method"),
            QString("%1").arg("HMAC-SHA1")
            );
}

QPair<QString,QString> OAuth::signatureQueryItem(UserData* userData,
                                  QString method,
                                  QUrl url)
{
    //prepare URL
        QString urlSchemeAndHost = url.toString(QUrl::RemovePort |
                                                QUrl::RemovePath |
                                                QUrl::RemoveQuery |
                                                QUrl::RemoveFragment);
        QString urlPath = url.path();

        //url path parts need to be UTF-8 encoded and percent encoded
        QStringList urlPathParts = urlPath.split("/");
        for(int i = 0; i < urlPathParts.length(); ++i)
        {
            urlPathParts[i] = urlPathParts[i].toUtf8().toPercentEncoding();
        }
        urlPath = urlPathParts.join("/");

        QByteArray readyForUseUrlPart1 =
               (urlSchemeAndHost+urlPath).toAscii().toPercentEncoding();

    //prepare Query
        QList< QPair<QString,QString> > queryItems = url.queryItems();

        //query values need to be UTF-8 encoded and percent encoded
        for(int i = 0; i < queryItems.length(); ++i)
        {
            QPair<QString,QString> queryItem = queryItems[i];
            queryItem.second = queryItem.second.toUtf8().toPercentEncoding();
            queryItems[i] = queryItem;
        }

        qSort(queryItems);

        QString newQuery;
        QPair<QString,QString> queryItem;
        foreach(queryItem, queryItems)
        {
            newQuery += queryItem.first + "=" + queryItem.second + "&";
        }
        //remove last "&"
        newQuery.chop(1);

        QString readyForUseUrlPart2 = newQuery.toAscii().toPercentEncoding();

    //generate base string
        QString base = method+
                       "&"+
                       readyForUseUrlPart1+
                       "&"+
                       readyForUseUrlPart2;

    //calculate the hash
        QString hash = hmacSha1(base,
                                consumerData->secret + "&" + userData->secret
                                );

    //return the result
        return qMakePair(
                QString("oauth_signature"),
                QString(hash)
                );
}

QPair<QString,QString> OAuth::userTokenQueryItem(UserData* userData)
{
    return qMakePair(
            QString("oauth_token"),
            QString("%1").arg(userData->token)
            );
}

void OAuth::updateRequest(UserData* userData,
                          QString method,
                          QUrl* url)
{
    QList< QPair<QString,QString> > queryItems = url->queryItems();

    //replace timestamp item
    for(int i = 0; i < queryItems.length(); ++i)
        if(queryItems[i].first == "oauth_timestamp")
            queryItems.removeAt(i);
    queryItems.append(timestampQueryItem());

    //replace nonce item
    for(int i = 0; i < queryItems.length(); ++i)
        if(queryItems[i].first == "oauth_nonce")
            queryItems.removeAt(i);
    //dirty trick: I know that the timestamp item is the last item :-)
    queryItems.append(nonceQueryItem(queryItems.last().second.toLongLong()));

    //remove signature item
    for(int i = 0; i < queryItems.length(); ++i)
        if(queryItems[i].first == "oauth_signature")
            queryItems.removeAt(i);

    //prepare for signature generation
    url->setQueryItems(queryItems);

    //add new signature item
    queryItems.append(signatureQueryItem(userData, method, *url));

    //update to include the signature
    url->setQueryItems(queryItems);
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
