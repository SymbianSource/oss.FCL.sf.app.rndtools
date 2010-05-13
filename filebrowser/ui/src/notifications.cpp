/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/


#include <hblabel.h>
//#include <hbdeviceprogressdialog.h>
#include <hbmessagebox.h>

#include "notifications.h"


// ---------------------------------------------------------------------------

void Notifications::showMessageBox(HbMessageBox::MessageBoxType type, const QString &text, const QString &label, int timeout)
{
    HbMessageBox *messageBox = new HbMessageBox(type);
    messageBox->setText(text);
    if(label.length())
        {
        HbLabel *header = new HbLabel(label, messageBox);
        messageBox->setHeadingWidget(header);
        }
    messageBox->setAttribute(Qt::WA_DeleteOnClose);
    messageBox->setTimeout(timeout);
    messageBox->open();
}

void Notifications::about()
{
    showMessageBox(
        HbMessageBox::MessageTypeInformation, 
        "Version 5.0.0 - 23h April 2010. Copyright © 2010 Nokia Corporation"
            "and/or its subsidiary(-ies). All rights reserved."
            "Licensed under Eclipse Public License v1.0.", 
        "About File Browser"
        );
}

// ---------------------------------------------------------------------------

//HbDeviceProgressDialog* Notifications::showWaitDialog(const QString &text)
//{
//    HbDeviceProgressDialog *note = new HbDeviceProgressDialog( HbProgressDialog::WaitDialog );
//    note->setText( text );
//    note->show();
//    return note;
//}

// ---------------------------------------------------------------------------

void Notifications::showInformationNote(const QString &text)
{
    showMessageBox(HbMessageBox::MessageTypeInformation, text, "", false);
}

// ---------------------------------------------------------------------------

void Notifications::showErrorNote(const QString &text, bool noTimeout)
{
    showMessageBox(HbMessageBox::MessageTypeWarning, text, "", noTimeout ? HbPopup::NoTimeout : 3000);
}

// ---------------------------------------------------------------------------

void Notifications::showConfirmationNote(const QString &text, bool noTimeout)
{
    showMessageBox(HbMessageBox::MessageTypeWarning, text, "", noTimeout ? HbPopup::NoTimeout : HbPopup::ConfirmationNoteTimeout);
}

// ---------------------------------------------------------------------------

bool Notifications::showConfirmationQuery(const QString &aText)
{
    return HbMessageBox::question(aText);
}

// ---------------------------------------------------------------------------
