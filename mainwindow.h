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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// base class
#include <QMainWindow>

// member functions
#include <QVariantMap>
class QNetworkRequest;
class QNetworkReply;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
// member functions
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
public slots:
    // network slots
    void getNetworkRequest(QNetworkRequest *networkRequest);
    void putNetworkRequest(QNetworkRequest *networkRequest, QIODevice *data);
    void handleNetworkReply(QNetworkReply *networkReply);
private slots:
    // misc
    void setupActions();
    void attemptSignIn();
    void signOut();
    void download(QVariantMap fileInfo);
    void upload(QString directory);
    void switchToNavigation();
    void switchToAccountInfo();
    void switchToFileTransfers();
    void setCurrentPage(QWidget *page);
    // action slots
    void on_aboutAction_triggered();
    void on_aboutQtAction_triggered();
    
// private data members
private:
    Ui::MainWindow *ui;
    QWidget *previousPage;
};

#endif // MAINWINDOW_H
