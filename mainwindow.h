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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//base class
#include <QWidget>

//data members
#include "downloaddialog.h"
#include "uploaddialog.h"
#include "userdata.h"
class QNetworkAccessManager;
class QToolButton;
class Dropbox;
class OAuth;

//member functions
class QNetworkReply;
class QListWidgetItem;
class QNetworkRequest;
class QResizeEvent;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

//shared data members
public:
    QNetworkAccessManager *networkAccessManager;
    Dropbox *dropbox;
    OAuth *oAuth;

//member functions
public:
    explicit MainWindow(
        QNetworkAccessManager *networkAccessManager,
        Dropbox *dropbox,
        OAuth *oAuth,
        QWidget *parent = 0
        );
    ~MainWindow();
protected:
    void resizeEvent(QResizeEvent *resizeEvent);
    void keyPressEvent(QKeyEvent *event);
private slots:
    //initialization
    void attemptSignIn();
    //misc
    bool s60v3();
    void setupActions();
    void setCurrentPage(QWidget *page);
    void back();
    void showContextMenu(QPoint point);
    void handleItemUploadedToDirectory(QString directory);
    void openDropboxInABrowser();
    //ui slots
    void on_doneSigningInPushButton_clicked();
    void on_filesAndFoldersListWidget_itemActivated(QListWidgetItem *item);
    void on_filesAndFoldersListWidget_itemDoubleClicked(QListWidgetItem *item);
    void on_filesAndFoldersListWidget_customContextMenuRequested(QPoint point);
    //action slots
    void navigateItem(QListWidgetItem *item);
    void signIn();
    void signOut();
    void about();
    void cut();
    void copy();
    void rename();
    void remove();
    void publicLink();
    void download();
    void propeties();
    void up();
    void refresh();
    void paste();
    void createFolder();
    void upload();
    void activeDownload();
    void activeUpload();
    //network slots
    void requestRequestToken();
    void requestAccessToken();
    void requestDirectoryListing(QString path);
    void requestAccountInfo();
    void requestFolderCreation(QString path);
    void requestCopying(QString source, QString destination);
    void requestMoving(QString source, QString destination);
    void requestDeletion(QString path);
    void requestNetworkRequest(QNetworkRequest *networkRequest);
    void globalHandleNetworkReply(QNetworkReply *networkReply);
    void handleNetworkReply(QNetworkReply *networkReply);
    void handleRequestToken(QNetworkReply *networkReply);
    void handleAccessToken(QNetworkReply *networkReply);
    void handleDirectoryListing(QNetworkReply *networkReply);
    void handleAccountInfo(QNetworkReply *networkReply);
    void handleFolderCreation(QNetworkReply* networkReply);
    void handleCopying(QNetworkReply* networkReply);
    void handleMoving(QNetworkReply* networkReply);
    void handleDeletion(QNetworkReply* networkReply);

//private data members
private:
    Ui::MainWindow *ui;
    QString requestToken;
    QString requestTokenSecret;
    DownloadDialog downloadDialog;
    UploadDialog uploadDialog;
    QString currentDirectory;
    UserData userData;
    QWidget *tempPage;
    bool shouldPreserveClipboardContents;
    bool renameOperationBeingProcessed;
    QString clipboard;

    QLayout *toolButtonsLayout;
    QToolButton *upToolButton;
    QToolButton *refreshToolButton;
    QToolButton *pasteToolButton;
    QToolButton *createFolderToolButton;
    QToolButton *uploadToolButton;

    QAction *cutAction;
    QAction *copyAction;
    QAction *renameAction;
    QAction *removeAction;
    QAction *publicLinkAction;
    QAction *downloadAction;
    QAction *propertiesAction;
    QAction *upAction;
    QAction *refreshAction;
    QAction *pasteAction;
    QAction *createFolderAction;
    QAction *uploadAction;
    QAction *activeDownloadAction;
    QAction *activeUploadAction;
    QAction *accountInfoAction;
    QAction *signOutAction;
    QAction *aboutAction;
    QAction *aboutQtAction;
    QAction *backAction;
};

#endif // MAINWINDOW_H
