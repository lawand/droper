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

#ifndef ACCOUNTINFOPAGE_H
#define ACCOUNTINFOPAGE_H

// base class(es)
#include <QWidget>

// member functions
class QNetworkRequest;
class QNetworkReply;

namespace Ui {
class AccountInfoPage;
}

class AccountInfoPage : public QWidget
{
    Q_OBJECT

// member functions
public:
    explicit AccountInfoPage(QWidget *parent = 0);
    ~AccountInfoPage();
    // misc
private:
    void setupActions();
signals:
    // signals
    void networkRequestGetNeeded(QNetworkRequest *request);
    void backRequested();
    void signOutRequested();
public slots:
    // network slots
    void requestAccountInfo();
    void handleNetworkReply(QNetworkReply *networkReply);
    void handleAccountInfo(QNetworkReply *networkReply);
private slots:
    // action slots
    void on_referralLinkAction_triggered();
    void on_backAction_triggered();
    void on_signOutAction_triggered();

// private data members
private:
    Ui::AccountInfoPage *ui;
    QString referralLink;
};

#endif // ACCOUNTINFOPAGE_H
