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


#include <hbmessagebox.h>
#include <hblabel.h>
#include <hbaction.h>
#include <hbpopup.h>
#include <hblineedit.h>
#include <hbvalidator.h>
#include <hbabstractitemview.h>
#include <hbprogressdialog.h>

#include <QString>
#include <QDate>
#include <QRegExp>

#include "notifications.h"



// ---------------------------------------------------------------------------

void Notifications::about()
{
    HbMessageBox *messageBox = new HbMessageBox(HbMessageBox::MessageTypeInformation);
        messageBox->setText("Version 6.0.0 - April 23rd 2010. Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved. Licensed under Eclipse Public License v1.0.");
        HbLabel *header = new HbLabel("About Creator", messageBox);
        messageBox->setHeadingWidget(header);
        messageBox->setAttribute(Qt::WA_DeleteOnClose);
        messageBox->setTimeout(HbPopup::NoTimeout);
        messageBox->open();
}

// ---------------------------------------------------------------------------

void Notifications::error(const QString& errorMessage)
{
    HbMessageBox::warning(errorMessage, new HbLabel("Error"));
}


// ---------------------------------------------------------------------------


HbProgressDialog* Notifications::showProgressBar(const QString& text, int max)
{
	HbProgressDialog *note = new HbProgressDialog(HbProgressDialog::ProgressDialog);
    note->setText(text);
    note->setMaximum(max);

    note->show();
    return note;

}

// ---------------------------------------------------------------------------

void Notifications::showGlobalNote(const QString& text, HbMessageBox::MessageBoxType type, HbPopup::DefaultTimeout /*timeout*/)
{
	if (type == HbMessageBox::HbMessageBox::MessageTypeInformation)
		HbMessageBox::information(text, new HbLabel("Information"));
	else
		HbMessageBox::information(text, new HbLabel("Information"));
}

// ---------------------------------------------------------------------------

bool Notifications::entriesQueryDialog(int& numberOfEntries, const QString& text, bool acceptsZero)
{
	bool err = false;
	HbDialog *popup = new HbDialog();
    popup->setDismissPolicy(HbPopup::TapOutside);
    popup->setTimeout(HbPopup::NoTimeout);
	
	HbLabel *title = new HbLabel();
    HbLineEdit *edit = new HbLineEdit();
	HbAction *actionOk = new HbAction("Ok");
	HbAction *actionCancel = new HbAction("Cancel");
	
	title->setPlainText(text);
	popup->setHeadingWidget(title);
	popup->setContentWidget(edit);
	
	HbValidator *validator = new HbValidator();
	QString tmp;
    if (acceptsZero == false) {
		tmp.append("[1-9]{1}\\d{1,4}");
	}
	else {
		tmp.append("^[0-9]{5}");
	}
        
	QRegExp rxBasic(tmp);
	validator->addField(new QRegExpValidator(rxBasic, 0), "");
    edit->setValidator(validator);
	edit->setText(QString::number(numberOfEntries));
	edit->setSelection(0, QString::number(numberOfEntries).length());
	
	popup->setPrimaryAction(actionOk);
    popup->setSecondaryAction(actionCancel);

    // Launch popup syncronously
    HbAction *action = 0;
    action = popup->exec();

	// continue if ok selected and valid user input exists in line editor
    if (action && action->text() == "Ok" && edit->text() != "") {
		numberOfEntries = edit->text().toInt(&err, 10);
	}
	return err;
}

// ---------------------------------------------------------------------------
bool Notifications::timeQueryDialog(QDate& date, const QString& text)
{
	bool err = false;
    HbDialog *popup = new HbDialog();
    popup->setDismissPolicy(HbPopup::TapOutside);
    popup->setTimeout(HbPopup::NoTimeout);
	
	HbLabel *title = new HbLabel();
    HbLineEdit *edit = new HbLineEdit();
	HbAction *actionOk = new HbAction("Ok");
	HbAction *actionCancel = new HbAction("Cancel");
    
	title->setPlainText(text);
	popup->setHeadingWidget(title);
	popup->setContentWidget(edit);

    HbValidator *validator =new HbValidator;
    QString dateString("(0[1-9]|[12][0-9]|3[01])[/](0[1-9]|1[012])[/](19|20)\\d\\d");
	QRegExp rxDate(dateString);
	validator->addField(new QRegExpValidator(rxDate, 0), "");
    edit->setValidator(validator);
	edit->setText(date.toString());
	edit->setSelection(0, date.toString().length());

    
	popup->setPrimaryAction(actionOk);
    popup->setSecondaryAction(actionCancel);

    // Launch popup syncronously
    HbAction *action = 0;
    action = popup->exec();

	// continue if ok selected and valid user input exists in line editor
    if (action && action->text() == "Ok" && edit->text() != "") {
		date = QDate::fromString(edit->text());
        err = true;
	}
	return err;    
}

// ---------------------------------------------------------------------------

bool Notifications::yesNoQueryDialog(const QString& text)
{
    return HbMessageBox::question(text);
}

// ---------------------------------------------------------------------------

void Notifications::popupListDialog(const QString& text, QStringList& items, HbAbstractItemView::SelectionMode mode, QObject* receiver, const char* member)
{
	HbSelectionDialog *dlg = new HbSelectionDialog;
	dlg->setHeadingWidget(new HbLabel(text, dlg));
	dlg->setStringItems(items);
	dlg->setSelectionMode(mode); 
	dlg->setAttribute(Qt::WA_DeleteOnClose);
	dlg->open(receiver, member);
}

// ---------------------------------------------------------------------------

bool Notifications::directoryQueryDialog(const QString& text, QString& directory)
{
	bool err = false;
    HbDialog *popup = new HbDialog();
    popup->setDismissPolicy(HbPopup::TapOutside);
    popup->setTimeout(HbPopup::NoTimeout);
	
	HbLabel *title = new HbLabel();
    HbLineEdit *edit = new HbLineEdit();
	HbAction *actionOk = new HbAction("Ok");
	HbAction *actionCancel = new HbAction("Cancel");
	
	title->setPlainText(text);
	popup->setHeadingWidget(title);
	popup->setContentWidget(edit);
	edit->setMaxLength(256);
	edit->setText(directory);
	edit->setSelection(0, directory.length());
	
	popup->setPrimaryAction(actionOk);
    popup->setSecondaryAction(actionCancel);

    // Launch popup syncronously
    HbAction *action = 0;
    action = popup->exec();

	// continue if ok selected and valid user input exists in line editor
    if (action && action->text() == "Ok" && edit->text() != "") {
		directory = edit->text();
		err = true;
	}
	return err;
}

// ---------------------------------------------------------------------------
