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

// corresponding headers
#include "consumerdata.h"

ConsumerData::ConsumerData(
    ) :
    key(XXXX),
    secret(XXXX)
    // key and secret were intentionally set to XXXX for security reasons
    // you should request a Dropbox key/secret pair and use them instead
    // of these dummy values.
    // for more info, lookup OAuth and Dropbox API
    //
    // feel free to contact me by email (Lawand Dalatieh - <lawand87@gmail.com>)
    // in case you had any questions about this :)
{
}
