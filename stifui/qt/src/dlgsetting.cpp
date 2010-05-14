/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description: QT C++ based Class.
 * 
 */

#include "dlgsetting.h"
#include <QtGui>

DlgSetting::DlgSetting(UiSetting* settingObj, QWidget *parent)
    : QDialog(parent), setting(settingObj)
    {
    SetupUI();
    }

void DlgSetting::SetupUI()
    {
    this->setContextMenuPolicy(Qt::NoContextMenu);
    QGridLayout *mainLayout = new QGridLayout(this);
    this->setLayout(mainLayout);
    
    chkShowoutput = new QCheckBox(this);
    chkShowoutput->setText(tr("Show output in execution."));
    chkShowoutput->setChecked(setting->ReadSetting(KShowOutput) == "true");
    QWidget *toolWidget = new QWidget(this);
    QGridLayout *toolLayout = new QGridLayout(this);
    
    toolWidget->setLayout(toolLayout);
    btnOk = new QPushButton(tr("Ok"), toolWidget);
    QObject::connect(btnOk, SIGNAL(clicked()), this,
            SLOT(on_btnOk_clicked()));
    btnCancel = new QPushButton(tr("Cancel"), toolWidget);
    QObject::connect(btnCancel, SIGNAL(clicked()), this,
            SLOT(on_btnCancel_clicked()));
    toolLayout->addWidget(btnOk, 0, 0);
    toolLayout->addWidget(btnCancel, 0, 1);

    QWidget *nullWidget = new QWidget(this);
    nullWidget->setMinimumHeight(30);
    
    mainLayout->addWidget(nullWidget, 0, 0);
    mainLayout->addWidget(chkShowoutput, 1, 0);
    mainLayout->addWidget(toolWidget, 3, 0);    
    
    }

void DlgSetting::on_btnOk_clicked()
    {
    if(chkShowoutput->checkState() == Qt::Checked)
        {
        setting->SetSetting(KShowOutput, "true");
        }
    else
        {
        setting->SetSetting(KShowOutput, "false");    
        }
    this->accept();
    }

void DlgSetting::on_btnCancel_clicked()
    {
    this->reject();
    }
