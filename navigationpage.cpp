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
#include "navigationpage.h"
#include "ui_navigationpage.h"

// member functions
#include <QNetworkReply>

// implementation-specific
#include <QNetworkRequest>
#include <QMessageBox>
#include <QResource>
#include <QToolBar>
#include <QDateTime>
#include <QMenu>
#include <QKeyEvent>
#include <QInputDialog>
#include <QClipboard>
#include <QSettings>
#include <QDesktopServices>
#include <QFileDialog>
#include "common.h"
#include "dropbox.h"
#include "oauth.h"
#include "userdata.h"
#include "util.h"
#include "json.h"
#include "qtscroller.h"

NavigationPage::NavigationPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NavigationPage)
{
    // private data members initialization
    ui->setupUi(this);

    // kinetic scroller for touch devices
    if(! Util::s60v3())
    {
        QtScroller::grabGesture(
            ui->filesAndFoldersListWidget->viewport(),
            QtScroller::LeftMouseButtonGesture
            );
    }

    // actions setup
    setupActions();
}

NavigationPage::~NavigationPage()
{
    delete ui;
}

void NavigationPage::focusFilesAndFoldersListWidget()
{
    ui->filesAndFoldersListWidget->setFocus();
}

QVariantMap NavigationPage::getMetadataOfCurrentDirectory()
{
    return metadataOfCurrentDirectory;
}

void NavigationPage::addItemToFilesAndFoldersListWidget(
    QVariantMap metadataMap
    )
{
    QString filePath = metadataMap["path"].toString();
    QString fileDirectory = filePath.left(
        filePath.lastIndexOf("/")
        );
    if(fileDirectory.isEmpty())
        fileDirectory = "/";

    if(fileDirectory.toLower() == currentDirectory.toLower())
    {
        QListWidgetItem *item = new QListWidgetItem(
            ui->filesAndFoldersListWidget
            );

        QString itemPath = metadataMap["path"].toString();
        QString itemName = itemPath.right(
            (itemPath.length() - itemPath.lastIndexOf("/")) - 1
            );

        if(metadataMap["is_dir"].toBool() == true)
        {
            item->setText(itemName);
        }
        else
        {
            QString size =  metadataMap["size"].toString();

            item->setText(itemName + " " + "(" + size + ")");
        }

        QResource iconResource(
            QString(":/resources/icons/%1")
            .arg(metadataMap["icon"].toString())
            + ".png"
            );

        if(metadataMap["is_dir"].toBool() == true)
        {
            if(iconResource.isValid())
                item->setIcon(QIcon(iconResource.fileName()));
            else
                item->setIcon(QIcon(":/resources/icons/folder.png"));
        }
        else
        {
            if(iconResource.isValid())
                item->setIcon(QIcon(iconResource.fileName()));
            else
                item->setIcon(QIcon(":/resources/icons/page_white.png"));
        }

        item->setData(Qt::UserRole, metadataMap);
    }
}

void NavigationPage::setupActions()
{
    QList<QAction *> contextMenuActions;
    contextMenuActions.append(ui->cutAction);
    contextMenuActions.append(ui->copyAction);
    contextMenuActions.append(ui->renameAction);
    contextMenuActions.append(ui->deleteAction);
    contextMenuActions.append(ui->downloadAction);
    contextMenuActions.append(ui->publicLinkAction);
    contextMenuActions.append(ui->shareableLinkAction);
    contextMenuActions.append(ui->propertiesAction);
    ui->filesAndFoldersListWidget->addActions(contextMenuActions);

    QList<QAction *> folderActions;
    if(!Util::s60v3())
    {
        folderActions.append(ui->upAction);
    }
    folderActions.append(ui->refreshAction);
    folderActions.append(ui->pasteAction);
    folderActions.append(ui->createFolderAction);
    folderActions.append(ui->uploadAction);
    if(Util::s60v3())
    {
        QMenu *folderMenu = new QMenu(this);
        folderMenu->addActions(folderActions);
        QAction *folderMenuAction = new QAction("Folder", this);
        folderMenuAction->setMenu(folderMenu);
        this->addAction(folderMenuAction);
    }
    else
    {
        QToolBar *toolBar = new QToolBar(this);
        toolBar->addActions(folderActions);
        toolBar->setIconSize(QSize(24, 24));
        layout()->addWidget(toolBar);
    }

    this->addAction(ui->accountInfoAction);
    this->addAction(ui->fileTransfersAction);

    // rename actions to include keyboard shortcut
    if(!Util::s60v3())
    {
        ui->deleteAction->setText(ui->deleteAction->text() + " [Backspace]");
        ui->copyAction->setText(ui->copyAction->text() + " [Ctrl+C]");
        ui->cutAction->setText(ui->cutAction->text() + " [Ctrl+X]");
        ui->pasteAction->setText(ui->pasteAction->text() + " [Ctrl+V]");
        ui->downloadAction->setText(ui->downloadAction->text() + " [Ctrl+D]");
        ui->propertiesAction->setText(
            ui->propertiesAction->text() + " [Ctrl+E]"
            );
        ui->renameAction->setText(ui->renameAction->text() + " [Ctrl+R]");
        ui->uploadAction->setText(ui->uploadAction->text() + " [Ctrl+U]");
        ui->createFolderAction->setText(
            ui->createFolderAction->text() + " [Ctrl+F]"
            );
        ui->refreshAction->setText(ui->refreshAction->text() + " [Ctrl+Space]");
        ui->publicLinkAction->setText(
            ui->publicLinkAction->text() + " [Ctrl+P]"
            );
        ui->shareableLinkAction->setText(
            ui->shareableLinkAction->text() + " [Ctrl+S]"
            );
    }
    else
    {
        ui->deleteAction->setText(ui->deleteAction->text() + " [Backspace]");
        ui->copyAction->setText(ui->copyAction->text() + " [1]");
        ui->cutAction->setText(ui->cutAction->text() + " [2]");
        ui->pasteAction->setText(ui->pasteAction->text() + " [3]");
        ui->downloadAction->setText(ui->downloadAction->text() + " [4]");
        ui->propertiesAction->setText(
            ui->propertiesAction->text() + " [5]"
            );
        ui->renameAction->setText(ui->renameAction->text() + " [7]");
        ui->uploadAction->setText(ui->uploadAction->text() + " [8]");
        ui->createFolderAction->setText(
            ui->createFolderAction->text() + " [9]"
            );
        ui->refreshAction->setText(ui->refreshAction->text() + " [0]");
        ui->publicLinkAction->setText(
            ui->publicLinkAction->text() + " [*]"
            );
        ui->shareableLinkAction->setText(
            ui->shareableLinkAction->text() + " [#]"
            );
    }
}

void NavigationPage::keyPressEvent(QKeyEvent *event)
{
    if(! Util::s60v3())
    {
        switch(event->modifiers())
        {
        case Qt::ControlModifier:
            switch(event->key())
            {
            case Qt::Key_C:
                ui->copyAction->trigger();
                break;

            case Qt::Key_X:
                ui->cutAction->trigger();
                break;

            case Qt::Key_V:
                ui->pasteAction->trigger();
                break;

            case Qt::Key_D:
                ui->downloadAction->trigger();
                break;

            case Qt::Key_E:
                ui->propertiesAction->trigger();
                break;

            case Qt::Key_R:
                ui->renameAction->trigger();
                break;

            case Qt::Key_U:
                ui->uploadAction->trigger();
                break;

            case Qt::Key_F:
                ui->createFolderAction->trigger();
                break;

            case Qt::Key_Space:
                ui->refreshAction->trigger();
                break;

            case Qt::Key_P:
                ui->publicLinkAction->trigger();
                break;

            case Qt::Key_S:
                ui->shareableLinkAction->trigger();
                break;

            default:
                QWidget::keyPressEvent(event);
                break;
            }
            break;

        default:
            switch(event->key())
            {
            case Qt::Key_Right:
                ui->navigateAction->trigger();
                break;

            case Qt::Key_Left:
                ui->upAction->trigger();
                break;

            case Qt::Key_Backspace:
                ui->deleteAction->trigger();
                break;

            default:
                QWidget::keyPressEvent(event);
                break;
            }
            break;
        }
    }
    else
    {
        switch(event->key())
        {
        case Qt::Key_Right:
            ui->navigateAction->trigger();
            break;

        case Qt::Key_Left:
            ui->upAction->trigger();
            break;

        case Qt::Key_Backspace:
            ui->deleteAction->trigger();
            break;

        case Qt::Key_1:
            ui->copyAction->trigger();
            break;

        case Qt::Key_2:
            ui->cutAction->trigger();
            break;

        case Qt::Key_3:
            ui->pasteAction->trigger();
            break;

        case Qt::Key_4:
            ui->downloadAction->trigger();
            break;

        case Qt::Key_5:
            ui->propertiesAction->trigger();
            break;

        case Qt::Key_7:
            ui->renameAction->trigger();
            break;

        case Qt::Key_8:
            ui->uploadAction->trigger();
            break;

        case Qt::Key_9:
            ui->createFolderAction->trigger();
            break;

        case Qt::Key_0:
            ui->refreshAction->trigger();
            break;

        case Qt::Key_Asterisk:
            ui->publicLinkAction->trigger();
            break;

        case Qt::Key_NumberSign:
            ui->shareableLinkAction->trigger();
            break;

        default:
            QWidget::keyPressEvent(event);
            break;
        }
    }
}

void NavigationPage::requestMetadata(QString path)
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::METADATA).toString() + path;
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    emit networkRequestGetNeeded(&networkRequest);
}

void NavigationPage::requestMove(QString source, QString destination)
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_MOVE);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("from_path", source);
    url.addQueryItem("to_path", destination);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    emit networkRequestGetNeeded(&networkRequest);
}

void NavigationPage::requestCopy(QString source, QString destination)
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_COPY);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("from_path", source);
    url.addQueryItem("to_path", destination);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    emit networkRequestGetNeeded(&networkRequest);
}

void NavigationPage::requestDelete(QString path)
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_DELETE);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("path", path);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    emit networkRequestGetNeeded(&networkRequest);
}

void NavigationPage::requestCreateFolder(QString path)
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::FILEOPS_CREATEFOLDER);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("path", path);
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    emit networkRequestGetNeeded(&networkRequest);
}

void NavigationPage::requestShares(QString path)
{
    QUrl url = Common::dropbox->apiToUrl(Dropbox::SHARES).toString() + path;
    QNetworkRequest networkRequest(url);
    Common::oAuth->signRequestHeader("GET", &networkRequest);

    emit networkRequestGetNeeded(&networkRequest);
}

void NavigationPage::handleNetworkReply(QNetworkReply *networkReply)
{
    Dropbox::Api api = Common::dropbox->urlToApi(networkReply->url());
    switch(api)
    {
    case Dropbox::METADATA:
        handleMetadata(networkReply);
        break;

    case Dropbox::FILEOPS_MOVE:
    case Dropbox::FILEOPS_COPY:
    case Dropbox::FILEOPS_DELETE:
    case Dropbox::FILEOPS_CREATEFOLDER:
        handleFileops(networkReply);
        break;

    case Dropbox::SHARES:
        handleShares(networkReply);
        break;

    default:
        break;
    }
}

void NavigationPage::handleMetadata(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(dirJson, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
            this,
            "Droper",
            "JSON parsing failed."
            );

        metadataOfCurrentDirectory.clear();

        return;
    }

    // set metadata of current directory
    metadataOfCurrentDirectory = jsonResult;

    // reset scroll bar
    ui->filesAndFoldersListWidget->setCurrentRow(0);

    // prepare to change current directory
    ui->filesAndFoldersListWidget->clear();

    // set current directory's icon
    if(jsonResult["path"] == "/")
    {
        ui->currentFolderIconLabel->setPixmap(
            QIcon(":/resources/icons/dropbox.png").pixmap(16, 16)
            );
    }
    else
    {
        QResource iconResource(
            QString(":/resources/icons/%1")
            .arg(jsonResult["icon"].toString())
            + ".png"
            );
        if(iconResource.isValid())
            ui->currentFolderIconLabel->setPixmap(
                QIcon(iconResource.fileName()).pixmap(16, 16)
                );
        else
            ui->currentFolderIconLabel->setPixmap(
                QIcon(":/resources/icons/folder.png").pixmap(16, 16)
                );
    }

    // update currentDirectory and ui->currentFolderLabel
    currentDirectory = Common::dropbox->metaDataPathFromUrl(
        networkReply->url()
        );
    QString currentFolder = currentDirectory.right(
        (currentDirectory.length() - currentDirectory.lastIndexOf("/")) - 1
        );
    if(!currentFolder.isEmpty())
        ui->currentFolderLabel->setText(currentFolder);
    else
        ui->currentFolderLabel->setText("Dropbox");

    // add folders
    foreach(const QVariant &itemJson, jsonResult["contents"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();

        if(itemMap["is_dir"].toBool() == true)
        {
            addItemToFilesAndFoldersListWidget(itemMap);
        }
    }

    // add files
    foreach(const QVariant &itemJson, jsonResult["contents"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();

        if(itemMap["is_dir"].toBool() == false)
        {
            addItemToFilesAndFoldersListWidget(itemMap);
        }
    }
}

void NavigationPage::handleFileops(QNetworkReply *)
{
    ui->refreshAction->trigger();
}

void NavigationPage::handleShares(QNetworkReply *networkReply)
{
    QString json = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = QtJson::Json::parse(json, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
            this,
            "Droper",
            "JSON parsing failed."
            );

        return;
    }

    QApplication::clipboard()->setText(jsonResult["url"].toString());

    QMessageBox::information(
        this,
        "Droper",
        QString("The shareable link \"%1\" was copied to the clipboard. Note "
                "that this link expires after about a month, more specifically "
                "on: %2")
                .arg(jsonResult["url"].toString())
                .arg(jsonResult["expires"].toString())
        );
}

void NavigationPage::on_filesAndFoldersListWidget_itemActivated(
    QListWidgetItem *
    )
{
    if(! Util::s60v3())
    {
        ui->navigateAction->trigger();
    }
}

void NavigationPage::on_navigateAction_triggered()
{
    QListWidgetItem *currentItem = ui->filesAndFoldersListWidget->currentItem();

    if(currentItem == 0)
        return;

    QVariantMap map = currentItem->data(Qt::UserRole).toMap();

    if(map["is_dir"].toBool() == true)
        // if the item is a directory
    {
        // navigate to a sub directory
        requestMetadata(
            map["path"].toString()
        );
    }
}

void NavigationPage::on_cutAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    // fill the clipboard
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();
    clipboard.first = map["path"].toString();

    // mark the operation as a cut operation
    // (preserve clipboard content == false)
    clipboard.second = false;
}

void NavigationPage::on_copyAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    // fill the clipboard
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();
    clipboard.first = map["path"].toString();

    // mark the operation as a copy operation
    // (preserve clipboard content == true)
    clipboard.second = true;
}

void NavigationPage::on_renameAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();
    QString path = map["path"].toString();
    QString oldName = path.right(
        (path.length() - path.lastIndexOf("/")) - 1
        );

    QString newName = oldName;
    while(true)
    {
        newName = QInputDialog::getText(
            this,
            "Rename",
            "Enter a new name:",
            QLineEdit::Normal,
            newName
            );

        // trim whitespace
        newName = newName.trimmed();

        // these symbols aren't allowed by Dropbox
        QRegExp disallowedSymbols("[/:?*<>\"|]");
        if(newName.contains(disallowedSymbols) || newName.contains("\\") ||
            newName == "." || newName == ".."
            )
        {
            QMessageBox::information(
                this,
                "Droper",
                "The following characters aren't allowed by Dropbox: \n"
                "\\ / : ? * < > \" | \n"
                "And you can't name a file or folder . or .."
                );
        }
        else
        {
            break;
        }
    }

    // if no new value was entered, do nothing
    if(newName.isEmpty())
        return;

    // perform the rename operation
    if(currentDirectory == "/")
    {
        requestMove(
            currentDirectory + oldName,
            currentDirectory + newName
            );
    }
    else
    {
        requestMove(
            currentDirectory + "/" + oldName,
            currentDirectory + "/" + newName
            );
    }
}

void NavigationPage::on_deleteAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    QMessageBox::StandardButton response = QMessageBox::question(
        this,
        "Droper",
        "Are you sure you want to delete the file/folder?",
        QMessageBox::No|QMessageBox::Yes,
        QMessageBox::No
        );

    if(response == QMessageBox::Yes)
    {
        QListWidgetItem* currentItem =
            ui->filesAndFoldersListWidget->currentItem();
        QVariantMap map =
            currentItem->data(Qt::UserRole).toMap();
        QString path = map["path"].toString();

        requestDelete(path);
    }
}

void NavigationPage::on_downloadAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    // get raw info
    QListWidgetItem *currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();

    emit downloadRequested(map);
}

void NavigationPage::on_publicLinkAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    // get raw info
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();

    QString path = map["path"].toString();

    if( !path.startsWith("/Public/") || map["is_dir"].toBool() == true)
    {
        QMessageBox::information(
            this,
            "Droper",
            "Can only get public links to files inside the public folder."
            );
    }
    else
    {
        QString pathWithPublicAsRoot = path;
        pathWithPublicAsRoot.remove(0, 8);
        QString percentEncodedPathWithPublicAsRoot =
                pathWithPublicAsRoot.toUtf8().toPercentEncoding("", "~");

        QString publicLink = QString(
            "http://dl.dropbox.com/u/%1/%2"
            )
            .arg(Common::userData->uid)
            .arg(percentEncodedPathWithPublicAsRoot);

        QApplication::clipboard()->setText(publicLink);

        QMessageBox::information(
            this,
            "Droper",
            QString("The public link \"%1\" was copied to the clipboard.")
                    .arg(publicLink)
            );
    }
}

void NavigationPage::on_shareableLinkAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    // get path
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();
    QString path = map["path"].toString();

    requestShares(path);
}

void NavigationPage::on_propertiesAction_triggered()
{
    // make sure an item is selected
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
    {
        QMessageBox::information(
            this,
            "Droper",
            "No item selected. Select one and try again."
            );
        return;
    }

    // get raw info
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();

    // size
    QString size = map["size"].toString();
    if(!size.endsWith("bytes"))
    {
        QString bytes = map["bytes"].toString();
        size += QString(" (%1 bytes)").arg(bytes);
    }

    // path and name
    QString path = map["path"].toString();
    QString name = path.right(
        (path.length() - path.lastIndexOf("/")) - 1
        );

    // modified date and time
    QString modifiedString = map["modified"].toString();
    modifiedString.chop(6);     // chop() removes the time zone
    QDateTime modifiedTimeDate = QDateTime::fromString(
        modifiedString,
        "ddd, dd MMM yyyy HH:mm:ss"
        );
    modifiedTimeDate.setTimeSpec(Qt::UTC);
    QDateTime current = QDateTime::currentDateTime().toUTC();
    int secs = modifiedTimeDate.secsTo(current);
    int mins = secs/60.0;
    int hours = mins/60.0;    // using non-integer division to be able to
    int days = hours/24.0;    // compile for S60v3 using Qt SDK 1.1.2
    int months = days/30.0;
    int years = months/12.0;
    QString friendlyModifiedString;
    if(secs < 60)
    {
        friendlyModifiedString = QString(
            "about %1 second(s) ago"
            ).arg(secs);
    }
    else if(mins < 60)
    {
        friendlyModifiedString = QString(
            "about %1 minute(s) ago"
            ).arg(mins);
    }
    else if(hours < 24)
    {
        friendlyModifiedString = QString(
            "about %1 hour(s) ago"
            ).arg(hours);
    }
    else if(days < 30)
    {
        friendlyModifiedString = QString(
            "about %1 day(s) ago"
            ).arg(days);
    }
    else if(months < 12)
    {
        friendlyModifiedString = QString(
            "about %1 month(s) ago"
            ).arg(months);
    }
    else
    {
        friendlyModifiedString = QString(
            "about %1 year(s) ago"
            ).arg(years);
    }
    modifiedString += QString(" (%1)").arg(friendlyModifiedString);

    // show results

    QMessageBox messageBox(this);
    messageBox.setWindowTitle("Droper");

    if(map["is_dir"].toBool() != true)
        // if the item is not a directory
    {
        messageBox.setText(
            QString(
                "Showing details for file: \n"
                "%1"
                ).arg(name)
            );
    }
    else
    {
        messageBox.setText(
            QString(
                "Showing details for folder: \n"
                "%1"
                ).arg(name)
            );
    }

    QString informativeText;

    if(map["is_dir"].toBool() != true)
        // if the item is not a directory
    {
        informativeText += QString("Size: %1\n\n").arg(size);
    }

    informativeText += QString(
        "Path: %2 \n\n"
        "Modified (in UTC): %3"
        ).arg(path).arg(modifiedString);

    messageBox.setInformativeText(informativeText);

    messageBox.exec();
}

void NavigationPage::on_upAction_triggered()
{
    if(currentDirectory == "/")
        return;

    // generate new directory
    QStringList parts = currentDirectory.split("/");
    parts.removeLast();
    QString newDirectory = parts.join("/");

    // handle root
    if(newDirectory.isEmpty())
        newDirectory = "/";

    requestMetadata(newDirectory);
}

void NavigationPage::on_refreshAction_triggered()
{
    requestMetadata(currentDirectory);
}

void NavigationPage::on_pasteAction_triggered()
{
    // make sure the clipboard is not empty
    if(clipboard.first.isEmpty())
    {
        QMessageBox::information(
            this,
            "Droper",
            "The clipboard is empty. Cut or copy an item and try again."
            );
        return;
    }

    // get file or folder name
    QString name = clipboard.first.right(
        (clipboard.first.length() - clipboard.first.lastIndexOf("/")) - 1
        );

    if(clipboard.second)
        // if this is a copy operation
    {
        if(currentDirectory == "/")
        {
            requestCopy(clipboard.first, currentDirectory + name);
        }
        else
        {
            requestCopy(clipboard.first, currentDirectory + "/" + name);
        }
    }
    else
    {
        if(currentDirectory == "/")
        {
            requestMove(clipboard.first, currentDirectory + name);
        }
        else
        {
            requestMove(clipboard.first, currentDirectory + "/" + name);
        }

        clipboard.first.clear();
    }
}

void NavigationPage::on_createFolderAction_triggered()
{
    QString folderName;
    while(true)
    {
        folderName = QInputDialog::getText(
            this,
            "Droper",
            "Enter the folder's name:",
            QLineEdit::Normal,
            folderName
            );

        // trim whitespace
        folderName = folderName.trimmed();

        // these symbols aren't allowed by Dropbox
        QRegExp disallowedSymbols("[/:?*<>\"|]");
        if(folderName.contains(disallowedSymbols) ||
            folderName.contains("\\") || folderName == "." ||
            folderName == "..")
        {
            QMessageBox::information(
                this,
                "Droper",
                "The following characters aren't allowed by Dropbox: \n"
                "\\ / : ? * < > \" | \n"
                "And you can't name a file or folder . or .."
                );
        }
        else
        {
            break;
        }
    }

    // if no folderName was entered, do nothing
    if( folderName.isEmpty() )
        return;

    if(currentDirectory == "/")
    {
        requestCreateFolder(currentDirectory + folderName);
    }
    else
    {
        requestCreateFolder(currentDirectory + "/" + folderName);
    }
}

void NavigationPage::on_uploadAction_triggered()
{
    emit uploadRequested(currentDirectory);
}

void NavigationPage::on_accountInfoAction_triggered()
{
    emit accountInfoRequested();
}

void NavigationPage::on_fileTransfersAction_triggered()
{
    emit switchToFileTransfersRequested();
}
