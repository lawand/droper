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
#include "oauth.h"

// data members
#include "consumerdata.h"

// member functions
#include <QUrl>
#include "userdata.h"

// implementation-specific
#include <QByteArray>
#include <QCryptographicHash>
#include <QDateTime>
#include <QStringList>

OAuth::OAuth(ConsumerData *consumerData)
{
    // shared data members initialization
    this->consumerData = consumerData;

    // qrand seed
    qsrand(QDateTime::currentDateTime().toTime_t());
}

void OAuth::signRequestHeader(
    QString method,
    QNetworkRequest *networkRequest,
    UserData *userData
    )
{
    QString header = "OAuth ";

    header += timestampAndNonceHeaderItems() + ",";
    header += consumerKeyHeaderItem() + ",";
    header += signatureMethodHeaderItem() + ",";
    if(userData != 0)
    {
        header += userTokenHeaderItem(userData) + ",";
    }
    header += versionHeaderItem() + ",";

    QUrl url = networkRequest->url();
    header += signatureHeaderItem(
        userData,
        method,
        &url,
        header
        ) + ",";

    // remove the last ","
    header.chop(1);

    networkRequest->setRawHeader("Authorization", header.toAscii());
}

QString OAuth::timestampAndNonceHeaderItems()
{
    int currentSecsSinceEpoch = QDateTime::currentDateTime().toUTC().toTime_t();

    return QString("%1=\"%2\"")
        .arg("oauth_timestamp")
        .arg(currentSecsSinceEpoch)
        +
        ","
        +
        QString("%1=\"%2\"")
        .arg("oauth_nonce")
        .arg(QString("%1").arg(qrand()))
        ;
}

QString OAuth::consumerKeyHeaderItem()
{
    return QString("%1=\"%2\"")
        .arg("oauth_consumer_key")
        .arg(consumerData->key)
        ;
}

QString OAuth::signatureMethodHeaderItem()
{
    return QString("%1=\"%2\"")
        .arg("oauth_signature_method")
        .arg("HMAC-SHA1")
        ;
}

QString OAuth::userTokenHeaderItem(UserData *userData)
{
    return QString("%1=\"%2\"")
        .arg("oauth_token")
        .arg(userData->token)
        ;
}

QString OAuth::versionHeaderItem()
{
    return QString("%1=\"%2\"")
        .arg("oauth_version")
        .arg("1.0")
        ;
}

QString OAuth::signatureHeaderItem(
    UserData *userData,
    QString method,
    QUrl *url,
    QString oAuthHeader
    )
{
    // prepare URL
    QString urlSchemeAndHost = url->toString(
        QUrl::RemovePort |
        QUrl::RemovePath |
        QUrl::RemoveQuery |
        QUrl::RemoveFragment
        );
    QString urlPath = url->path();

    // url path parts need to be UTF-8 encoded and percent encoded
    QStringList urlPathParts = urlPath.split("/");
    for(int i = 0; i < urlPathParts.length(); ++i)
    {
        urlPathParts[i] = urlPathParts[i].toUtf8().toPercentEncoding();
    }
    urlPath = urlPathParts.join("/");

    QByteArray readyForUseUrl =
        (urlSchemeAndHost+urlPath).toAscii().toPercentEncoding();

    // prepare parameters
    QList< QPair<QString,QString> > parameters;

    parameters.append(url->queryItems());

    // extract header parameters and add them to the parameters list
    oAuthHeader.remove("OAuth ");
    QStringList oAuthParameters =
        oAuthHeader.split(",", QString::SkipEmptyParts);
    foreach(QString oAuthParameter, oAuthParameters)
    {
        QStringList oAuthParameterParts = oAuthParameter.split("=");
        QString first = oAuthParameterParts.at(0);
        QString second = oAuthParameterParts.at(1);
        second.remove("\"");
        QPair<QString, QString> parameter = qMakePair(
            first,
            second
            );
        parameters.append(parameter);
    }

    // parameters need to be UTF-8 encoded and percent encoded
    for(int i = 0; i < parameters.length(); ++i)
    {
        QPair<QString,QString> parameter = parameters[i];
        parameter.second = parameter.second.toUtf8().toPercentEncoding();
        parameters[i] = parameter;
    }

    qSort(parameters);

    QString parametersString;
    QPair<QString,QString> parameter;
    foreach(parameter, parameters)
    {
        parametersString += parameter.first + "=" + parameter.second + "&";
    }
    // remove last "&"
    parametersString.chop(1);

    QString readyForUseParametersString =
        parametersString.toAscii().toPercentEncoding();

    // generate base string
    QString base = method+
        "&"+
        readyForUseUrl+
        "&"+
        readyForUseParametersString;

    // calculate the hash
    QString hash;
    if(userData != 0)
    {
        hash = hmacSha1(
                base,
                consumerData->secret + "&" + userData->secret
                );
    }
    else
    {
        hash = hmacSha1(
                base,
                consumerData->secret + "&"
                );
    }

    // return the result
    return QString("%1=\"%2\"")
        .arg("oauth_signature")
        .arg(hash)
        ;
}

QString OAuth::hmacSha1(QString base, QString key)
{
    // inner pad
    QByteArray ipad;
    ipad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        ipad[i] = key[i].toAscii();

    // outer pad
    QByteArray opad;
    opad.fill(char(0), 64);
    for(int i = 0; i < key.length(); ++i)
        opad[i] = key[i].toAscii();

    // XOR operation for inner pad
    for(int i = 0; i < ipad.length(); ++i)
        ipad[i] = ipad[i] ^ 0x36;

    // XOR operation for outer pad
    for(int i = 0; i < opad.length(); ++i)
        opad[i] = opad[i] ^ 0x5c;

    // Hashes inner pad
    QByteArray innerSha1 = QCryptographicHash::hash(
        ipad + base.toAscii(),
        QCryptographicHash::Sha1
        );

    // Hashes outer pad
    QByteArray outerSha1 = QCryptographicHash::hash(
        opad + innerSha1,
        QCryptographicHash::Sha1
        );

    return outerSha1.toBase64();
}
