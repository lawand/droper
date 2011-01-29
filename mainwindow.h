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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//base class(es)
#include <QMainWindow>

//data member(s) forward declaration(s)
class QNetworkAccessManager;
class OAuth;
class UserData;
class Dropbox;

//member-function(s)-related forward declaration(s)
class QNetworkReply;
class QTreeWidgetItem;

//member-function(s)-related includes

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QNetworkAccessManager* networkAccessManager,
                        OAuth* oAuth,
                        UserData* userData,
                        Dropbox* dropbox,
                        QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString currentDirectory;
    QString clipboard;
    bool shouldPreserveClipboardContents;

public: //shared objects
    QNetworkAccessManager* networkAccessManager;
    OAuth* oAuth;
    UserData* userData;
    Dropbox* dropbox;

private slots:
    void handleNetworkReply(QNetworkReply* networkReply);
    void requestAccountInformation();
    void handleAccountInformation(QNetworkReply* networkReply);
    void requestDirectoryListing(QString path);
    void handleDirectoryListing(QNetworkReply* networkReply);
    void requestFile(QString path);
    void handleFile(QNetworkReply* networkReply);
    void requestCopying(QString source, QString destination);
    void handleCopying(QNetworkReply* networkReply);
    void requestMoving(QString source, QString destination);
    void handleMoving(QNetworkReply* networkReply);
    void requestDeleting(QString path);
    void handleDeleting(QNetworkReply* networkReply);
    void requestFolderCreation(QString path);
    void handleFolderCreation(QNetworkReply* networkReply);
    void about();
    void refreshCurrentDirectory();
    void on_filesAndFoldersTreeWidget_itemDoubleClicked(
            QTreeWidgetItem* item
            );
    void on_upPushButton_clicked();
    void on_refreshPushButton_clicked();
    void on_createFolderPushButton_clicked();
    void on_cutPushButton_clicked();
    void on_copyPushButton_clicked();
    void on_pastePushButton_clicked();
    void on_renamePushButton_clicked();
    void on_deletePushButton_clicked();
};

#endif // MAINWINDOW_H
