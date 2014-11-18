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

#ifndef COMMON_H
#define COMMON_H

// static shared data members
class QNetworkAccessManager;
class ConsumerData;
class Dropbox;
class OAuth;
class UserData;

class Common
{
// static shared data members
public:
    static QNetworkAccessManager *networkAccessManager;
    static ConsumerData *consumerData;
    static Dropbox *dropbox;
    static OAuth *oAuth;
    static UserData *userData;
};

#endif // COMMON_H
