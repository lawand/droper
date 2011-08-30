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

//corresponding headers
#include "mainwindow.h"
#include "ui_mainwindow.h"

//data members
#include <QNetworkAccessManager>
#include <QToolButton>
#include "dropbox.h"
#include "oauth.h"

//member functions
#include <QNetworkReply>
#include <QResizeEvent>

//implementation-specific
#include <QSettings>
#include <QMessageBox>
#include <QResource>
#include <QMovie>
#include <QInputDialog>
#include <QMenu>
#include <QSysInfo>
#include <QDateTime>
#include <QFileDialog>
#include <QDesktopServices>
#include <QClipboard>
#include <QsKineticScroller/QsKineticScroller.h>
#include "json/json.h"

MainWindow::MainWindow(
    QNetworkAccessManager *networkAccessManager,
    Dropbox *dropbox,
    OAuth *oAuth,
    QWidget *parent
    ) :
    QWidget(parent),
    ui(new Ui::MainWindow),
    authenticationDialog(
        networkAccessManager,
        dropbox,
        oAuth
    ),
    downloadDialog(
        networkAccessManager,
        dropbox,
        oAuth,
        &userData
    ),
    uploadDialog(
        networkAccessManager,
        dropbox,
        oAuth,
        &userData
    ),
    currentDirectory("/"),
    renameOperationBeingProcessed(false),
    requestingUidAndReferralLink(false),
    toolButtonsLayout(0)
{
    //shared data members initialization
    this->networkAccessManager = networkAccessManager;
    this->dropbox = dropbox;
    this->oAuth = oAuth;

    //private data members initialization
    ui->setupUi(this);
    setCurrentPage(ui->authenticationPage);
    setupActions();
    if(! s60v3())
    {
        ui->mainPage->layout()->setMargin(0);
    }

    //QObject connections
    connect(
        this->networkAccessManager,
        SIGNAL(finished(QNetworkReply*)),
        SLOT(globalHandleNetworkReply(QNetworkReply*))
    );
    connect(ui->signUpPushButton, SIGNAL(clicked()), SLOT(signUp()));
    connect(ui->signInPushButton, SIGNAL(clicked()), SLOT(signIn()));
    connect(
        &authenticationDialog,
        SIGNAL(accepted()),
        SLOT(readTokenAndSecret())
    );
    connect(
        &uploadDialog,
        SIGNAL(itemUploadedToDirectory(QString)),
        SLOT(handleItemUploadedToDirectory(QString))
        );

    //QMovie initialization
    QMovie *loading = new QMovie(
        ":/resources/animations/loading.gif",
        QByteArray(),
        this
    );
    ui->loadingLabel->setMovie(loading);
    loading->start();

    //QsKineticScroller initialization
    QsKineticScroller *kineticScroller = new QsKineticScroller(this);
    kineticScroller->enableKineticScrollFor(ui->filesAndFoldersListWidget);

    //user data
    initializationStepOne();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *resizeEvent)
{
    if(! s60v3())
    {
        if(toolButtonsLayout != 0)
        {
            ui->mainPage->layout()->removeItem(toolButtonsLayout);
            delete toolButtonsLayout;
        }

        if(resizeEvent->size().height() > resizeEvent->size().width())
            //if the new screen orientation is a portrait one
        {
            toolButtonsLayout = new QHBoxLayout();
            toolButtonsLayout->addWidget(upToolButton);
            toolButtonsLayout->addWidget(refreshToolButton);
            toolButtonsLayout->addWidget(pasteToolButton);
            toolButtonsLayout->addWidget(createFolderToolButton);
            toolButtonsLayout->addWidget(uploadToolButton);

            upToolButton->setSizePolicy(
                QSizePolicy::Expanding,
                QSizePolicy::Preferred
                );
            refreshToolButton->setSizePolicy(
                QSizePolicy::Expanding,
                QSizePolicy::Preferred
                );
            pasteToolButton->setSizePolicy(
                QSizePolicy::Expanding,
                QSizePolicy::Preferred
                );
            createFolderToolButton->setSizePolicy(
                QSizePolicy::Expanding,
                QSizePolicy::Preferred
                );
            uploadToolButton->setSizePolicy(
                QSizePolicy::Expanding,
                QSizePolicy::Preferred
                );

            qobject_cast<QGridLayout *>( ui->mainPage->layout() )->addItem(
                toolButtonsLayout,
                1,
                0
                );
        }
        else
        {
            toolButtonsLayout = new QVBoxLayout();
            toolButtonsLayout->addWidget(upToolButton);
            toolButtonsLayout->addWidget(refreshToolButton);
            toolButtonsLayout->addWidget(pasteToolButton);
            toolButtonsLayout->addWidget(createFolderToolButton);
            toolButtonsLayout->addWidget(uploadToolButton);

            upToolButton->setSizePolicy(
                QSizePolicy::Preferred,
                QSizePolicy::Expanding
                );
            refreshToolButton->setSizePolicy(
                QSizePolicy::Preferred,
                QSizePolicy::Expanding
                );
            pasteToolButton->setSizePolicy(
                QSizePolicy::Preferred,
                QSizePolicy::Expanding
                );
            createFolderToolButton->setSizePolicy(
                QSizePolicy::Preferred,
                QSizePolicy::Expanding
                );
            uploadToolButton->setSizePolicy(
                QSizePolicy::Preferred,
                QSizePolicy::Expanding
                );

            qobject_cast<QGridLayout *>( ui->mainPage->layout() )->addItem(
                toolButtonsLayout,
                0,
                1
                );
        }
    }

    QWidget::resizeEvent(resizeEvent);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(s60v3())
    {
        if(ui->stackedWidget->currentWidget() == ui->loadingPage)
            return;

        switch(event->key())
        {
        case Qt::Key_Right:
            navigateItem(ui->filesAndFoldersListWidget->currentItem());
            break;

        case Qt::Key_Left:
            if(upAction->isEnabled())
                upAction->trigger();
            break;
        }
    }
}

void MainWindow::initializationStepOne()
{
    QSettings settings;
    settings.beginGroup("user");
    if( (settings.childKeys().indexOf("token") == -1) ||
        (settings.childKeys().indexOf("secret") == -1) )
    {
        settings.clear();
    }
    else
    {
        readTokenAndSecret();
    }
}

void MainWindow::readTokenAndSecret()
{
    QSettings settings;
    userData.token = settings.value("user/token").toString();
    userData.secret = settings.value("user/secret").toString();

    initializationStepTwo();
}

void MainWindow::initializationStepTwo()
{
    QSettings settings;
    settings.beginGroup("user");
    if(settings.childKeys().indexOf("uid") == -1 ||
       settings.childKeys().indexOf("referral_link") == -1 )
    {
        requestUidAndReferralLink();
    }
    else
    {
        readUidAndReferralLink();
    }
}

void MainWindow::readUidAndReferralLink()
{
    QSettings settings;
    userData.uid = settings.value("user/uid").toString();
    userData.referralLink = settings.value("user/referral_link").toString();

    if(userData.referralLink.isEmpty())
        referralLinkAction->setEnabled(false);

    initializationStepThree();
}

void MainWindow::initializationStepThree()
{
    setCurrentPage(ui->mainPage);

    requestDirectoryListing("/");
}

bool MainWindow::s60v3()
{
    return (
        QSysInfo::s60Version() == QSysInfo::SV_S60_3_1 ||
        QSysInfo::s60Version() == QSysInfo::SV_S60_3_2
        );
}

void MainWindow::setupActions()
{
    if(! s60v3())
    {
        //create tool buttons
        upToolButton = new QToolButton(ui->mainPage);
        refreshToolButton = new QToolButton(ui->mainPage);
        pasteToolButton = new QToolButton(ui->mainPage);
        createFolderToolButton = new QToolButton(ui->mainPage);
        uploadToolButton = new QToolButton(ui->mainPage);
    }

    //create actions
    cutAction = new QAction("Cut", this);
    copyAction = new QAction("Copy", this);
    renameAction = new QAction("Rename", this);
    removeAction = new QAction("Remove", this);
    publicLinkAction = new QAction("Public Link", this);
    downloadAction = new QAction("Download", this);
    propertiesAction = new QAction("Properties", this);
    upAction = new QAction("Up", this);
    upAction->setIcon(QIcon(":/resources/actions/up.png"));
    upAction->setEnabled(false);
    refreshAction = new QAction("Refresh", this);
    refreshAction->setIcon(QIcon(":/resources/actions/refresh.png"));
    pasteAction = new QAction("Paste", this);
    pasteAction->setIcon(QIcon(":/resources/actions/paste.png"));
    pasteAction->setEnabled(false);
    createFolderAction = new QAction("Create Folder", this);
    createFolderAction->setIcon(QIcon(":/resources/actions/create-folder.png"));
    uploadAction = new QAction("Upload", this);
    uploadAction->setIcon(QIcon(":/resources/actions/upload.png"));
    activeDownloadAction = new QAction("Active Download", this);
    activeUploadAction = new QAction("Active Upload", this);
    accountInfoAction = new QAction("Account Info", this);
    referralLinkAction = new QAction("Referral Link", this);
    signOutAction = new QAction("Sign Out", this);
    aboutAction = new QAction("About", this);
    aboutQtAction = new QAction("About Qt", this);

    //connect them with their corresponding slots
    connect(cutAction, SIGNAL(triggered()), SLOT(cut()));
    connect(copyAction, SIGNAL(triggered()), SLOT(copy()));
    connect(renameAction, SIGNAL(triggered()), SLOT(rename()));
    connect(removeAction, SIGNAL(triggered()), SLOT(remove()));
    connect(publicLinkAction, SIGNAL(triggered()), SLOT(publicLink()));
    connect(downloadAction, SIGNAL(triggered()), SLOT(download()));
    connect(propertiesAction, SIGNAL(triggered()), SLOT(propeties()));
    connect(upAction, SIGNAL(triggered()), SLOT(up()));
    connect(refreshAction, SIGNAL(triggered()), SLOT(refresh()));
    connect(pasteAction, SIGNAL(triggered()), SLOT(paste()));
    connect(createFolderAction, SIGNAL(triggered()), SLOT(createFolder()));
    connect(uploadAction, SIGNAL(triggered()), SLOT(upload()));
    connect(activeDownloadAction, SIGNAL(triggered()), SLOT(activeDownload()));
    connect(activeUploadAction, SIGNAL(triggered()), SLOT(activeUpload()));
    connect(
        accountInfoAction,
        SIGNAL(triggered()),
        SLOT(requestAccountInfo())
        );
    connect(referralLinkAction, SIGNAL(triggered()), SLOT(referralLink()));
    connect(signOutAction, SIGNAL(triggered()), SLOT(signOut()));
    connect(aboutAction, SIGNAL(triggered()), SLOT(about()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

    if(! s60v3())
    {
        //tool buttons initialization

        upToolButton->setDefaultAction(upAction);
        refreshToolButton->setDefaultAction(refreshAction);
        pasteToolButton->setDefaultAction(pasteAction);
        createFolderToolButton->setDefaultAction(createFolderAction);
        uploadToolButton->setDefaultAction(uploadAction);

        upToolButton->setAutoRaise(true);
        refreshToolButton->setAutoRaise(true);
        pasteToolButton->setAutoRaise(true);
        createFolderToolButton->setAutoRaise(true);
        uploadToolButton->setAutoRaise(true);

        upToolButton->setIconSize(QSize(32, 32));
        refreshToolButton->setIconSize(QSize(32, 32));
        pasteToolButton->setIconSize(QSize(32, 32));
        createFolderToolButton->setIconSize(QSize(32, 32));
        uploadToolButton->setIconSize(QSize(32, 32));
    }

    //softkey menu initialization

    QAction *optionsMenuAction = new QAction("Options", ui->mainPage);
    optionsMenuAction->setSoftKeyRole(QAction::PositiveSoftKey);
    ui->mainPage->addAction(optionsMenuAction);
    QMenu *optionsMenu = new QMenu(ui->mainPage);
    optionsMenuAction->setMenu(optionsMenu);

    if(s60v3())
    {
        QAction *currentFolderMenuAction = new QAction(
            "Current Folder",
            ui->mainPage
            );
        optionsMenu->addAction(currentFolderMenuAction);
        QMenu *folderMenu = new QMenu(ui->mainPage);
        currentFolderMenuAction->setMenu(folderMenu);
        folderMenu->addAction(refreshAction);
        folderMenu->addAction(pasteAction);
        folderMenu->addAction(createFolderAction);
        folderMenu->addAction(uploadAction);
    }

    QAction *activeTransferMenuAction = new QAction(
        "Active Transfer",
        ui->mainPage
        );
    optionsMenu->addAction(activeTransferMenuAction);
    QMenu *activeTransferMenu = new QMenu(ui->mainPage);
    activeTransferMenuAction->setMenu(activeTransferMenu);
    activeTransferMenu->addAction(activeDownloadAction);
    activeTransferMenu->addAction(activeUploadAction);

    QAction *accountMenuAction = new QAction(
        "Account",
        ui->mainPage
        );
    optionsMenu->addAction(accountMenuAction);
    QMenu *accountMenu = new QMenu(ui->mainPage);
    accountMenuAction->setMenu(accountMenu);
    accountMenu->addAction(accountInfoAction);
    accountMenu->addAction(referralLinkAction);
    accountMenu->addAction(signOutAction);

    QAction *helpMenuAction = new QAction(
        "Help",
        ui->mainPage
        );
    optionsMenu->addAction(helpMenuAction);
    QMenu *helpMenu = new QMenu(ui->mainPage);
    helpMenuAction->setMenu(helpMenu);
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

void MainWindow::readUserDataAndSignIn()
{
    QSettings settings;
    userData.token = settings.value("user/token").toString();
    userData.secret = settings.value("user/secret").toString();

    requestUidAndReferralLink();
}

void MainWindow::setCurrentPage(QWidget *page)
{
    ui->stackedWidget->setCurrentWidget(page);

    // <I don't know why this is necessary>
    if(page == ui->mainPage)
    {
        ui->filesAndFoldersListWidget->setFocus();
    }
    // </I don't know why this is necessary>
}

void MainWindow::showContextMenu(QPoint point)
{
    if( ui->filesAndFoldersListWidget->selectedItems().isEmpty() )
        return;

    QMenu menu(this);
    menu.addAction(cutAction);
    menu.addAction(copyAction);
    menu.addAction(renameAction);
    menu.addAction(removeAction);

    QListWidgetItem *item = ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map = item->data(Qt::UserRole).toMap();
    if(map["is_dir"].toBool() != true)
        //if the item is not a directory
    {
        menu.addAction(publicLinkAction);
        menu.addAction(downloadAction);
    }

    menu.addAction(propertiesAction);
    menu.exec(point);
}

void MainWindow::handleItemUploadedToDirectory(QString directory)
{
    if(directory == currentDirectory)
        refresh();
}

void MainWindow::on_filesAndFoldersListWidget_itemActivated(
    QListWidgetItem *item
    )
{
    if(s60v3())
    {
        showContextMenu(
            ui->filesAndFoldersListWidget->geometry().center()
            );
    }
}

void MainWindow::on_filesAndFoldersListWidget_itemDoubleClicked(
    QListWidgetItem *item
    )
{
    if(! s60v3())
    {
        navigateItem(item);
    }
}

void MainWindow::on_filesAndFoldersListWidget_customContextMenuRequested(
    QPoint point
    )
{
    if(! s60v3())
    {
        showContextMenu(
            ui->filesAndFoldersListWidget->viewport()->mapToGlobal(point)
            );
    }
}

void MainWindow::navigateItem(QListWidgetItem *item)
{
    if(item == 0)
        return;

    QVariantMap map = item->data(Qt::UserRole).toMap();

    if(map["is_dir"].toBool() == true)
        //if the item is a directory
    {
        //navigate to a sub directory
        requestDirectoryListing(
            map["path"].toString()
        );
    }
}

void MainWindow::signUp()
{
    authenticationDialog.switchToSignUp();

    authenticationDialog.showMaximized();
}

void MainWindow::signIn()
{
    authenticationDialog.switchToSignIn();

    authenticationDialog.showMaximized();
}

void MainWindow::referralLink()
{
    QApplication::clipboard()->setText(userData.referralLink);

    QMessageBox::information(
        this,
        "Droper",
        "Referral link copied to clipboard."
        );
}

void MainWindow::signOut()
{
    QMessageBox::StandardButton result = QMessageBox::question(
        this,
        "Droper",
        "Are you sure you want to sign out?",
        QMessageBox::Yes|QMessageBox::No,
        QMessageBox::No
        );
    if(result != QMessageBox::Yes)
    {
        return;
    }

    //remove old user data
    QSettings settings;
    settings.remove("user");
    userData.token.clear();
    userData.secret.clear();

    //return to the authentication page
    setCurrentPage(ui->authenticationPage);
}

void MainWindow::about()
{
    QMessageBox messageBox(this);

    messageBox.setWindowTitle("Droper");

    messageBox.setText(
        "<b>Droper v0.4.0</b> - "
        "<a href=\"http://www.dropbox.com/\">Dropbox</a> Client"
        );

    messageBox.setInformativeText(
        "Copyright 2011 Omar Lawand Dalatieh <br><br>"

        "Licensed under the GNU GPLv3 license <br><br>"

        "<a href=\"http://coinonedge.com/droper/\">"
        "http://coinonedge.com/droper/"
        "</a>"
        );

    QIcon droper(":/droper.svg");
    messageBox.setIconPixmap(droper.pixmap(QSize(32, 32)));

    messageBox.exec();
}

void MainWindow::cut()
{
    //mark the operation as a cut operation
    shouldPreserveClipboardContents = false;

    //fill the clipboard
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();
    clipboard = map["path"].toString();
    pasteAction->setEnabled(true);
}

void MainWindow::copy()
{
    //mark the operation as a copy operation
    shouldPreserveClipboardContents = true;

    //fill the clipboard
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();
    clipboard = map["path"].toString();
    pasteAction->setEnabled(true);
}

void MainWindow::rename()
{
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();
    QString path = map["path"].toString();
    QString oldName = path.right(
        (path.length() - path.lastIndexOf("/")) - 1
        );

    bool inputOk = false;
    QString newName = oldName;
    while(!inputOk)
    {
        newName = QInputDialog::getText(
            this,
            "Rename",
            "Enter a new name:",
            QLineEdit::Normal,
            newName
            );

        //trim whitespace
        newName = newName.trimmed();

        //these symbols aren't allowed by Dropbox
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
            inputOk = true;
        }
    }

    //if no new value was entered, do nothing
    if(newName.isEmpty())
        return;

    //perform the rename operation
    if(currentDirectory == "/")
    {
        requestMoving(
            currentDirectory + oldName,
            currentDirectory + newName
            );
    }
    else
    {
        requestMoving(
            currentDirectory + "/" + oldName,
            currentDirectory + "/" + newName
            );
    }

    //mark the current operation as a rename
    renameOperationBeingProcessed = true;
}

void MainWindow::remove()
{
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

        requestDeletion(path);
    }
}

void MainWindow::publicLink()
{
    //get raw info
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();

    QString path = map["path"].toString();

    if(! path.startsWith("/Public/"))
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
            ).arg(userData.uid).arg(percentEncodedPathWithPublicAsRoot);

        QApplication::clipboard()->setText(publicLink);

        QMessageBox::information(
            this,
            "Droper",
            "Public link copied to clipboard."
            );
    }
}

void MainWindow::download()
{
    if(downloadDialog.isDownloading())
    {
        QMessageBox::information(
            this,
            "Droper",
            "Another file is being downloaded."
            );

        downloadDialog.exec();
    }
    else
    {
        //get raw info
        QListWidgetItem *currentItem =
            ui->filesAndFoldersListWidget->currentItem();
        QVariantMap map =
            currentItem->data(Qt::UserRole).toMap();

        //prepare folder
        bool ok = false;
        QString folderPath;
        while(!ok)
        {
            folderPath = QFileDialog::getExistingDirectory(
                0,
                "Droper",
                QDesktopServices::storageLocation(
                    QDesktopServices::DesktopLocation
                    )
                );

            //if no directory selected, do nothing
            if(folderPath.isEmpty())
            {
                return;
            }

            QString filePath = map["path"].toString();
            QString fileName = filePath.right(
                (filePath.length() -
                 filePath.lastIndexOf("/")) - 1
                );
            if(QFile(folderPath + "/" + fileName).exists())
            {
                QMessageBox::information(
                    this,
                    "Droper",
                    QString(
                        "This directory already has a file "
                        "named '%1'. Choose another one."
                        ).arg(fileName)
                    );
            }
            else
            {
                ok = true;
            }
        }

        downloadDialog.setFileAndFolderInformation(map, folderPath);

        downloadDialog.exec();
    }
}

void MainWindow::propeties()
{
    //get raw info
    QListWidgetItem* currentItem =
        ui->filesAndFoldersListWidget->currentItem();
    QVariantMap map =
        currentItem->data(Qt::UserRole).toMap();

    //size
    QString size = map["size"].toString();
    if(!size.endsWith("bytes"))
    {
        QString bytes = map["bytes"].toString();
        size += QString(" (%1 bytes)").arg(bytes);
    }

    //path and name
    QString path = map["path"].toString();
    QString name = path.right(
        (path.length() - path.lastIndexOf("/")) - 1
        );

    //modified date and time
    QString modifiedString = map["modified"].toString();
    modifiedString.chop(6);     //chop() removes the time zone
    QDateTime modifiedTimeDate = QDateTime::fromString(
        modifiedString,
        "ddd, dd MMM yyyy HH:mm:ss"
        );
    modifiedTimeDate.setTimeSpec(Qt::UTC);
    QDateTime current = QDateTime::currentDateTime().toUTC();
    int secs = modifiedTimeDate.secsTo(current);
    int mins = secs/60.0;
    int hours = mins/60.0;    //using non-integer division to be able to
    int days = hours/24.0;    //compile for S60v3 using Qt SDK 1.1.2
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

    //show results

    QMessageBox messageBox(this);
    messageBox.setWindowTitle("Droper");
    messageBox.setText(
        QString(
            "Showing details for file: \n"
            "%1"
            ).arg(name)
        );

    QString informativeText;

    if(map["is_dir"].toBool() != true)
        //if the item is not a directory
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

void MainWindow::up()
{
    //generate new directory
    QStringList parts = currentDirectory.split("/");
    parts.removeLast();
    QString newDirectory = parts.join("/");

    //handle root
    if(newDirectory.isEmpty())
        newDirectory = "/";

    requestDirectoryListing(newDirectory);
}

void MainWindow::refresh()
{
    requestDirectoryListing(currentDirectory);
}

void MainWindow::paste()
{
    //get file or folder name
    QString name = clipboard.right(
        (clipboard.length() - clipboard.lastIndexOf("/")) - 1
        );

    if(shouldPreserveClipboardContents)
        //if this is a copy operation
    {
        if(currentDirectory == "/")
        {
            requestCopying(clipboard, currentDirectory + name);
        }
        else
        {
            requestCopying(clipboard, currentDirectory + "/" + name);
        }
    }
    else
    {
        if(currentDirectory == "/")
        {
            requestMoving(clipboard, currentDirectory + name);
        }
        else
        {
            requestMoving(clipboard, currentDirectory + "/" + name);
        }
    }
}

void MainWindow::createFolder()
{
    bool inputOk = false;
    QString folderName;
    while(!inputOk)
    {
        folderName = QInputDialog::getText(
            this,
            "Droper",
            "Enter the folder's name:",
            QLineEdit::Normal,
            folderName
            );

        //trim whitespace
        folderName = folderName.trimmed();

        //these symbols aren't allowed by Dropbox
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
            inputOk = true;
        }
    }

    //if no folderName was entered, do nothing
    if( folderName.isEmpty() )
        return;

    if(currentDirectory == "/")
    {
        requestFolderCreation(currentDirectory + folderName);
    }
    else
    {
        requestFolderCreation(currentDirectory + "/" + folderName);
    }
}

void MainWindow::upload()
{
    if(uploadDialog.isUploading())
    {
        QMessageBox::information(
            this,
            "Droper",
            "Another file is being uploaded."
            );

        uploadDialog.exec();
    }
    else
    {
        //prepare file
        bool ok = false;
        QString filePath;
        while(!ok)
        {
            filePath = QFileDialog::getOpenFileName(
                0,
                "Droper",
                QDesktopServices::storageLocation(
                    QDesktopServices::DesktopLocation
                    )
                );

            //if no file selected, do nothing
            if(filePath.isEmpty())
            {
                return;
            }

            QString fileName = filePath.right(
                (filePath.length() - filePath.lastIndexOf("/")) - 1
                );

            //these symbols aren't allowed by Dropbox
            QRegExp disallowedSymbols("[/:?*<>\"|]");
            if(fileName.contains(disallowedSymbols) ||
                fileName.contains("\\") || fileName == "." || fileName == ".."
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
                ok = true;
            }
        }

        //compute fileSize
        QFileInfo fileInfo(filePath);
        int bytes = fileInfo.size();

        qreal size = (qreal)bytes;
        QString sizeUnit;
        if (size < 1024) {
            sizeUnit = "bytes";
        } else if (size < 1024*1024) {
            size /= 1024;
            sizeUnit = "kB";
        } else if (size < 1024*1024*1024){
            size /= 1024*1024;
            sizeUnit = "MB";
        } else {
            size /= 1024*1024*1024;
            sizeUnit = "GB";
        }

        QString fileSize = QString("%1%2")
            .arg(size, 0, 'f', 1)
            .arg(sizeUnit)
            ;

        //make sure there isn't any folder that has the same name that the
        //file to be uploaded has and if there was a file with the same name,
        //tell the user that this upload modifies that file and that the
        //revision history can be viewed at dropbox.com
        for(int row = 0; row < ui->filesAndFoldersListWidget->count(); ++row)
        {
            QListWidgetItem *item = ui->filesAndFoldersListWidget->item(row);

            QVariantMap map = item->data(Qt::UserRole).toMap();
            QString path = map["path"].toString();
            QString name = path.right(
                (path.length() - path.lastIndexOf("/")) - 1
                );
            QString fileName = filePath.right(
                (filePath.length() - filePath.lastIndexOf("/")) - 1
                );
            if(name.toLower() == fileName.toLower())
            {
                if(map["is_dir"].toBool() == true)
                {
                    QMessageBox::critical(
                        this,
                        "Droper",
                        "There is a folder with the same name in the current "
                        "directory."
                        );

                    return;
                }
                else
                {
                    QMessageBox::information(
                        this,
                        "Droper",
                        "There is a file with the same name in the current "
                        "directory, if you continue with the upload "
                        "process, it will be overwritten. \n"
                        "You can visit dropbox.com to see file revisions "
                        "and restore earlier ones."
                        );
                }
            }
        }

        uploadDialog.setFileAndFolderInformation(
            filePath,
            fileSize,
            currentDirectory
            );

        uploadDialog.exec();
    }
}

void MainWindow::activeDownload()
{
    downloadDialog.exec();
}

void MainWindow::activeUpload()
{
    uploadDialog.exec();
}

void MainWindow::requestUidAndReferralLink()
{
    requestingUidAndReferralLink = true;

    QUrl url = dropbox->apiToUrl(Dropbox::ACCOUNT_INFO);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(&userData, "GET", &networkRequest);

    requestNetworkRequest(&networkRequest);
}

void MainWindow::requestDirectoryListing(QString path)
{
    QUrl url = dropbox->apiToUrl(Dropbox::METADATA).toString() + path;
    url.addQueryItem("list", "true");

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(&userData, "GET", &networkRequest);

    requestNetworkRequest(&networkRequest);
}

void MainWindow::requestAccountInfo()
{
    QUrl url = dropbox->apiToUrl(Dropbox::ACCOUNT_INFO);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(&userData, "GET", &networkRequest);

    requestNetworkRequest(&networkRequest);
}

void MainWindow::requestFolderCreation(QString path)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_CREATEFOLDER);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("path", path);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(&userData, "GET", &networkRequest);

    requestNetworkRequest(&networkRequest);
}

void MainWindow::requestCopying(QString source, QString destination)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_COPY);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("from_path", source);
    url.addQueryItem("to_path", destination);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(&userData, "GET", &networkRequest);

    requestNetworkRequest(&networkRequest);
}

void MainWindow::requestMoving(QString source, QString destination)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_MOVE);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("from_path", source);
    url.addQueryItem("to_path", destination);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(&userData, "GET", &networkRequest);

    requestNetworkRequest(&networkRequest);
}

void MainWindow::requestDeletion(QString path)
{
    QUrl url = dropbox->apiToUrl(Dropbox::FILEOPS_DELETE);
    url.addQueryItem("root", "dropbox");
    url.addQueryItem("path", path);

    QNetworkRequest networkRequest(url);

    oAuth->signRequest(&userData, "GET", &networkRequest);

    requestNetworkRequest(&networkRequest);
}

void MainWindow::requestNetworkRequest(QNetworkRequest *networkRequest)
{
    networkAccessManager->get(*networkRequest);

    //show the loading animation
    if(ui->stackedWidget->currentWidget() != ui->loadingPage)
    {
        tempPage = ui->stackedWidget->currentWidget();
        setCurrentPage(ui->loadingPage);
    }
}

void MainWindow::globalHandleNetworkReply(QNetworkReply *networkReply)
{
    if(networkReply->error() != QNetworkReply::NoError &&
       networkReply->error() != QNetworkReply::OperationCanceledError)
    {
        QString replyData = networkReply->readAll();
        QVariantMap jsonResult = Json::parse(replyData).toMap();

        if(jsonResult.contains("error"))
        {
            if(jsonResult["error"].type() != QVariant::Map)
            {
                if(jsonResult["error"].toString().contains("Invalid signature"))
                {
                    QMessageBox::critical(
                        this,
                        "Droper",
                        "Droper currently has problems dealing with these five "
                        "symbols ; + ~ # %"
                        );
                }
                else
                {
                    QMessageBox::critical(
                        this,
                        "Droper",
                        jsonResult["error"].toString()
                        );
                }
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    QString(
                        jsonResult["error"].toMap().keys().first() +
                        ": " +
                        jsonResult["error"].toMap()
                                .values().first().toString()
                        )
                    );
            }
        }
        else
        {
            if(!replyData.isEmpty())
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    replyData
                );
            }
            else
            {
                QMessageBox::critical(
                    this,
                    "Droper",
                    networkReply->errorString()
                    );
            }
        }
    }

    Dropbox::Api api = dropbox->urlToApi(networkReply->url());
    switch(api)
    {
    case Dropbox::FILES:
        downloadDialog.handleNetworkReply(networkReply);
        uploadDialog.handleNetworkReply(networkReply);
        break;

    case Dropbox::TOKEN:
    case Dropbox::ACCOUNT:
        authenticationDialog.handleNetworkReply(networkReply);
        break;

    default:
        this->handleNetworkReply(networkReply);
        break;
    }

    networkReply->deleteLater();
}

void MainWindow::handleNetworkReply(QNetworkReply *networkReply)
{
    //stop the loading animation
    setCurrentPage(tempPage);

    if(networkReply->error() != QNetworkReply::NoError)
        return;

    Dropbox::Api api = dropbox->urlToApi(networkReply->url());
    switch(api)
    {
    case Dropbox::METADATA:
        handleDirectoryListing(networkReply);
        break;

    case Dropbox::ACCOUNT_INFO:
        if(requestingUidAndReferralLink)
            handleUidAndReferralLink(networkReply);
        else
            handleAccountInfo(networkReply);
        break;

    case Dropbox::FILEOPS_CREATEFOLDER:
        handleFolderCreation(networkReply);
        break;

    case Dropbox::FILEOPS_COPY:
        handleCopying(networkReply);
        break;

    case Dropbox::FILEOPS_MOVE:
        handleMoving(networkReply);
        break;

    case Dropbox::FILEOPS_DELETE:
        handleDeletion(networkReply);
        break;

    default:
        break;
    }
}

void MainWindow::handleUidAndReferralLink(QNetworkReply* networkReply)
{
    QString jsonData = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = Json::parse(jsonData, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
            this,
            "Droper",
            "JSON parsing failed."
            );

        return;
    }

    QSettings settings;
    settings.setValue("user/uid", jsonResult["uid"].toString());
    settings.setValue(
        "user/referral_link",
        jsonResult["referral_link"].toString()
        );

    requestingUidAndReferralLink = false;

    readUidAndReferralLink();
}

void MainWindow::handleDirectoryListing(QNetworkReply *networkReply)
{
    QString dirJson = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = Json::parse(dirJson, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
            this,
            "Droper",
            "JSON parsing failed."
            );

        return;
    }

    //prepare to change current directory
    ui->filesAndFoldersListWidget->clear();
    ui->filesAndFoldersListWidget->scrollToTop();

    //set current directory's icon
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

    //update currentDirectory and ui->currentFolderLabel
    currentDirectory = dropbox->metaDataPathFromUrl(networkReply->url());
    QString currentFolder = currentDirectory.right(
        (currentDirectory.length() - currentDirectory.lastIndexOf("/")) - 1
        );
    if(!currentFolder.isEmpty())
        ui->currentFolderLabel->setText(currentFolder);
    else
        ui->currentFolderLabel->setText("Dropbox");

    //disable the up action if we are at root, enable it otherwise
    if(currentDirectory == "/")
        upAction->setEnabled(false);
    else
        upAction->setEnabled(true);

    //add folders
    foreach(const QVariant &itemJson, jsonResult["contents"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();

        if(itemMap["is_dir"].toBool() == true)
        {
            QListWidgetItem *item = new QListWidgetItem(
                ui->filesAndFoldersListWidget
                );

            QString itemPath = itemMap["path"].toString();
            QString itemName = itemPath.right(
                (itemPath.length() - itemPath.lastIndexOf("/")) - 1
                );

            item->setText(itemName);

            QResource iconResource(
                QString(":/resources/icons/%1")
                .arg(itemMap["icon"].toString())
                + ".png"
                );

            if(iconResource.isValid())
                item->setIcon(QIcon(iconResource.fileName()));
            else
                item->setIcon(QIcon(":/resources/icons/folder.png"));

            item->setData(Qt::UserRole, itemMap);
        }
    }

    //add files
    foreach(const QVariant &itemJson, jsonResult["contents"].toList())
    {
        QVariantMap itemMap = itemJson.toMap();

        if(itemMap["is_dir"].toBool() == false)
        {
            QListWidgetItem *item = new QListWidgetItem(
                ui->filesAndFoldersListWidget
                );

            QString itemPath = itemMap["path"].toString();
            QString itemName = itemPath.right(
                (itemPath.length() - itemPath.lastIndexOf("/")) - 1
                );

            QString size =  itemMap["size"].toString();

            item->setText(itemName + " " + "(" + size + ")");

            QResource iconResource(
                QString(":/resources/icons/%1")
                .arg(itemMap["icon"].toString())
                + ".png"
                );

            if(iconResource.isValid())
                item->setIcon(QIcon(iconResource.fileName()));
            else
                item->setIcon(QIcon(":/resources/icons/page_white.png"));

            item->setData(Qt::UserRole, itemMap);
        }
    }

    if(s60v3())
    {
        if(ui->filesAndFoldersListWidget->count() != 0)
            ui->filesAndFoldersListWidget->setCurrentRow(0);
    }
}

void MainWindow::handleAccountInfo(QNetworkReply *networkReply)
{
    QString jsonData = networkReply->readAll();

    bool ok;
    QVariantMap jsonResult = Json::parse(jsonData, ok).toMap();
    if(!ok)
    {
        QMessageBox::information(
            this,
            "Droper",
            "JSON parsing failed."
            );

        return;
    }

    QVariantMap quotaInfo = jsonResult["quota_info"].toMap();

    qreal normalFiles = quotaInfo["normal"].toReal();
    QString normalFilesUnit;
    if (normalFiles < 1024) {
        normalFilesUnit = "bytes";
    } else if (normalFiles < 1024*1024) {
        normalFiles /= 1024;
        normalFilesUnit = "kB";
    } else if (normalFiles < 1024*1024*1024){
        normalFiles /= 1024*1024;
        normalFilesUnit = "MB";
    } else {
        normalFiles /= 1024*1024*1024;
        normalFilesUnit = "GB";
    }
    QString normalFilesString = QString("%1%2")
        .arg(normalFiles, 0, 'f', 1)
        .arg(normalFilesUnit)
        ;

    qreal sharedFiles = quotaInfo["shared"].toReal();
    QString sharedFilesUnit;
    if (sharedFiles < 1024) {
        sharedFilesUnit = "bytes";
    } else if (sharedFiles < 1024*1024) {
        sharedFiles /= 1024;
        sharedFilesUnit = "kB";
    } else if (sharedFiles < 1024*1024*1024){
        sharedFiles /= 1024*1024;
        sharedFilesUnit = "MB";
    } else {
        sharedFiles /= 1024*1024*1024;
        sharedFilesUnit = "GB";
    }
    QString sharedFilesString = QString("%1%2")
        .arg(sharedFiles, 0, 'f', 1)
        .arg(sharedFilesUnit)
        ;

    qreal quota = quotaInfo["quota"].toReal();
    QString quotaUnit;
    if (quota < 1024) {
        quotaUnit = "bytes";
    } else if (quota < 1024*1024) {
        quota /= 1024;
        quotaUnit = "kB";
    } else if (quota < 1024*1024*1024){
        quota /= 1024*1024;
        quotaUnit = "MB";
    } else {
        quota /= 1024*1024*1024;
        quotaUnit = "GB";
    }
    QString quotaString = QString("%1%2")
        .arg(quota, 0, 'f', 1)
        .arg(quotaUnit)
        ;

    qreal unused = quotaInfo["quota"].toReal() -
                   (quotaInfo["normal"].toReal() +
                   quotaInfo["shared"].toReal());
    QString unusedUnit;
    if (unused < 1024) {
        unusedUnit = "bytes";
    } else if (unused < 1024*1024) {
        unused /= 1024;
        unusedUnit = "kB";
    } else if (unused < 1024*1024*1024){
        unused /= 1024*1024;
        unusedUnit = "MB";
    } else {
        unused /= 1024*1024*1024;
        unusedUnit = "GB";
    }
    QString unusedString = QString("%1%2")
        .arg(unused, 0, 'f', 1)
        .arg(unusedUnit)
        ;

    QMessageBox messageBox(this);

    messageBox.setWindowTitle("Droper");

    messageBox.setText(
        QString("Showing account information for:\n%1").arg(
            jsonResult["email"].toString()
            )
        );

    messageBox.setInformativeText(
        QString(
            "Name: %1 \n"
            "Country: %2 \n"
            "UID: %3 \n"
            "\n"
            "Total Space: %4 \n"
            "Unused Space: %5 \n"
            "Regular files: %6 \n"
            "Shared files: %7 \n"
            "\n"
            "Total Space = Regular files + "
            "Shared files + Unused Space"
            )
            .arg(jsonResult["display_name"].toString())
            .arg(jsonResult["country"].toString())
            .arg(jsonResult["uid"].toString())
            .arg(quotaString)
            .arg(unusedString)
            .arg(normalFilesString)
            .arg(sharedFilesString)
        );

    messageBox.exec();
}

void MainWindow::handleFolderCreation(QNetworkReply* networkReply)
{
    refresh();
}

void MainWindow::handleCopying(QNetworkReply* networkReply)
{
    refresh();
}

void MainWindow::handleMoving(QNetworkReply* networkReply)
{
    refresh();

    if(!renameOperationBeingProcessed)
    {
        clipboard.clear();
        pasteAction->setEnabled(false);
    }
    else
    {
        renameOperationBeingProcessed = false;
    }
}

void MainWindow::handleDeletion(QNetworkReply* networkReply)
{
    refresh();
}
