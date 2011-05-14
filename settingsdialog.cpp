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

//corresponding headers
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

//data members
#include <QSettings>

//implementation-specific
#include <QMessageBox>

SettingsDialog::SettingsDialog(
    QSettings* settings,
    QWidget *parent
    ) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    //member initialization
    this->settings = settings;

    //GUI initialization
    ui->setupUi(this);
    ui->currentAccountLabel->setText(settings->value("user/email").toString());
#ifdef Q_OS_SYMBIAN
    ui->kineticScrollingCheckBox->setChecked(
        settings->value(
            "gui/kinetic_scrolling",
            false
            ).toBool()
        );

    ui->singleTapCheckBox->setChecked(
        settings->value(
            "gui/single_tap",
            true
            ).toBool()
        );
#else
    ui->guiGroupBox->setVisible(false);
    ui->buttonBox->setVisible(false);
    resize(baseSize());
#endif
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_kineticScrollingCheckBox_clicked()
{
    if(ui->kineticScrollingCheckBox->isChecked() == true)
    {
        QMessageBox::information(
            this,
            "Droper",
            "This is an expirmental feature and might not "
            "be as usable as other stable features, and might even not "
            "work at all. \n"
            "Note: Disabling the single tap option might help."
            );
    }
}

void SettingsDialog::on_forgetAuthenticationPushButton_clicked()
{
    QMessageBox::StandardButton response = QMessageBox::question(
        this,
        "Droper",
        "Are you sure you want to forget authentication? \n"
        "(This means that you'll have to login the next time you use "
        "Droper)",
        QMessageBox::No|QMessageBox::Yes,
        QMessageBox::No
        );

    if(response == QMessageBox::Yes)
        settings->remove("user");
}

void SettingsDialog::reject()
{
    //write settings
#ifdef Q_OS_SYMBIAN
    settings->setValue(
        "gui/kinetic_scrolling",
        ui->kineticScrollingCheckBox->isChecked()
        );

    settings->setValue(
        "gui/single_tap",
        ui->singleTapCheckBox->isChecked()
        );
#endif

    QDialog::reject();
}
