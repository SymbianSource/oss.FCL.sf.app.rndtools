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

#include <hbdevicemessagebox.h>
#include <hblabel.h>
#include <hbdeviceprogressdialog.h>

#include "notifications.h"



// ---------------------------------------------------------------------------

void Notifications::imageCaptured()
{
    showGlobalNote("Screen shot saved to Media Gallery", HbMessageBox::MessageTypeInformation);
}

// ---------------------------------------------------------------------------

void Notifications::about()
{

  HbMessageBox dlg;
    
	dlg.setText("Version 5.0.0 - March 10th 2010. Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved. Licensed under Eclipse Public License v1.0.");
    HbLabel header("About Screen Grabber");
    dlg.setHeadingWidget(&header);
	dlg.setTimeout(HbPopup::NoTimeout);
    dlg.exec();
}

// ---------------------------------------------------------------------------

void Notifications::error(const QString& errorMessage)
{
    showGlobalNote(errorMessage, HbMessageBox::MessageTypeWarning);

}

// ---------------------------------------------------------------------------

void Notifications::sequantialImagesCaptured(int amount){
    // Code below launches a global note
    QString text;
    text.setNum(amount, 10);
    text.append(" screen shots saved to Media Gallery");
    
    showGlobalNote(text, HbMessageBox::MessageTypeInformation);
 
}

// ---------------------------------------------------------------------------

void Notifications::videoCaptured()
{
    showGlobalNote("Video saved to Media Gallery", HbMessageBox::MessageTypeInformation);

}

// ---------------------------------------------------------------------------

HbDeviceProgressDialog* Notifications::showProgressBar(int max)
{
    HbDeviceProgressDialog *note = new HbDeviceProgressDialog(HbProgressDialog::ProgressDialog);
    note->setText("Saving...");
    note->setMaximum(max);
    note->show();
    return note;
}

// ---------------------------------------------------------------------------

void Notifications::showGlobalNote(const QString& text, HbMessageBox::MessageBoxType type)
{
    // Code below launches a global note

	HbDeviceMessageBox note(text, type);
	note.information(text);

}

// ---------------------------------------------------------------------------
