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

#ifndef NAVIGATIONPAGE_H
#define NAVIGATIONPAGE_H

// base class(es)
#include <QWidget>

// member functions
#include <QVariantMap>
class QNetworkRequest;
class QNetworkReply;
class QListWidgetItem;

namespace Ui {
class NavigationPage;
}

class NavigationPage : public QWidget
{
    Q_OBJECT
    
// member functions
public:
    explicit NavigationPage(QWidget *parent = 0);
    ~NavigationPage();
    // misc
public:
    void focusFilesAndFoldersListWidget();
    QVariantMap getMetadataOfCurrentDirectory();
public slots:
    void addItemToFilesAndFoldersListWidget(QVariantMap metadataMap);
private:
    void setupActions();
protected:
    // reimplemented
    void keyPressEvent(QKeyEvent *event);
signals:
    // signals
    void networkRequestGetNeeded(QNetworkRequest *request);
    void accountInfoRequested();
    void downloadRequested(QVariantMap fileInfo);
    void uploadRequested(QString directory);
    void switchToFileTransfersRequested();
public slots:
    // network slots
    void requestMetadata(QString path);
    void requestMove(QString source, QString destination);
    void requestCopy(QString source, QString destination);
    void requestDelete(QString path);
    void requestCreateFolder(QString path);
    void requestShares(QString path);
    void handleNetworkReply(QNetworkReply *networkReply);
    void handleMetadata(QNetworkReply *networkReply);
    void handleFileops(QNetworkReply *);
    void handleShares(QNetworkReply *networkReply);
private slots:
    // ui slots
    void on_filesAndFoldersListWidget_itemActivated(QListWidgetItem *item);
    // action slots
    void on_navigateAction_triggered();
    void on_cutAction_triggered();
    void on_copyAction_triggered();
    void on_renameAction_triggered();
    void on_deleteAction_triggered();
    void on_downloadAction_triggered();
    void on_shareableLinkAction_triggered();
    void on_propertiesAction_triggered();
    void on_upAction_triggered();
    void on_refreshAction_triggered();
    void on_pasteAction_triggered();
    void on_createFolderAction_triggered();
    void on_uploadAction_triggered();
    void on_accountInfoAction_triggered();
    void on_fileTransfersAction_triggered();

// private data members
private:
    Ui::NavigationPage *ui;
    QString currentDirectory;
    QVariantMap metadataOfCurrentDirectory;
    QPair<QString, bool> clipboard;
};

#endif // NAVIGATIONPAGE_H
