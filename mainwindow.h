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

//base class
#include <QMainWindow>

//data members
#include <downloaddialog.h>
#include <uploaddialog.h>
class QNetworkAccessManager;
class QSettings;
class QToolBar;
class OAuth;
class UserData;
class Dropbox;

//member functions
class QNetworkReply;
class QListWidgetItem;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
            QNetworkAccessManager* networkAccessManager,
            OAuth* oAuth,
            UserData* userData,
            Dropbox* dropbox,
            QSettings* settings,
            QWidget *parent = 0
            );
    ~MainWindow();

public: //shared objects
    QNetworkAccessManager* networkAccessManager;
    OAuth* oAuth;
    UserData* userData;
    Dropbox* dropbox;
    QSettings* settings;

private:
    Ui::MainWindow *ui;
    QString currentDirectory;
    QString clipboard;
    bool shouldPreserveClipboardContents;
    DownloadDialog downloadDialog;
    bool renameOperationBeingProcessed;
    UploadDialog uploadDialog;
#ifdef Q_OS_SYMBIAN
    bool kineticScrollingEnabled;
#endif

private slots:
    void handleNetworkReply(QNetworkReply* networkReply);
    void requestAccountInformation();
    void handleAccountInformation(QNetworkReply* networkReply);
    void requestDirectoryListing(QString path);
    void handleDirectoryListing(QNetworkReply* networkReply);
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
#ifdef Q_OS_SYMBIAN
    void on_filesAndFoldersListWidget_itemClicked(
#else
    void on_filesAndFoldersListWidget_itemDoubleClicked(
#endif
            QListWidgetItem* item
            );
    void up();
    void refresh();
    void cut();
    void copy();
    void paste();
    void rename();
    void del();
    void download();
    void upload();
    void createFolder();
    void showProperties();
    void on_filesAndFoldersListWidget_customContextMenuRequested(QPoint point);
    void showLoadingAnimation();
    void hideLoadingAnimation();
    void showFileMenu();
    void showOptionsMenu();
    void showInfoMenu();
    void handleUploadDone(QString folder);
    void showSettings();
};

#endif // MAINWINDOW_H
