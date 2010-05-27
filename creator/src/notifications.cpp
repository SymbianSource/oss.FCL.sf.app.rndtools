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
#include <hbdatetimepicker.h>

#include <QString>
#include <QDate>
#include <QRegExp>
#include <QList>

#include "notifications.h"

// ---------------------------------------------------------------------------

void Notifications::showMessageBox(HbMessageBox::MessageBoxType type, const QString &text, const QString &label, int timeout )
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

// ---------------------------------------------------------------------------

void Notifications::about()
{
    showMessageBox(HbMessageBox::MessageTypeInformation,
        "Version 6.0.0 - April 23rd 2010. Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved. Licensed under Eclipse Public License v1.0.",
        "About Creator", 
        HbPopup::NoTimeout
        );
}

// ---------------------------------------------------------------------------

void Notifications::error(const QString& errorMessage)
{
    showMessageBox(HbMessageBox::MessageTypeWarning, errorMessage, QString("Error"), 3000);
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

void Notifications::showGlobalNote(const QString& text, HbMessageBox::MessageBoxType type, HbPopup::DefaultTimeout timeout)
{
    showMessageBox(type, text, QString("Creator"), timeout);
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
    popup->setAttribute(Qt::WA_DeleteOnClose);
    // TODO: handle dialog close & user input
    popup->open();

	// continue if ok selected and valid user input exists in line editor
    /*if (action && action->text() == "Ok" && edit->text() != "") {
		directory = edit->text();
		err = true;
	}*/
	return err;
}

// ---------------------------------------------------------------------------

CreatorYesNoDialog::CreatorYesNoDialog(MUIObserver* observer, int userData) : 
    HbMessageBox(HbMessageBox::MessageTypeQuestion, NULL),
    CreatorDialog(observer, userData)
{
}

void CreatorYesNoDialog::launch(const QString& text, const QString& label, MUIObserver* observer, int userData) throw(std::exception)
{
    CreatorYesNoDialog* dlg = new CreatorYesNoDialog(observer, userData);
    dlg->setText(text);
    if(label.length())
        dlg->setHeadingWidget(new HbLabel(label, dlg));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setTimeout(HbPopup::NoTimeout);
    dlg->open(dlg, SLOT(DialogClosed(HbAction*)));
}

void CreatorYesNoDialog::DialogClosed(HbAction *action)
{
    TBool PositiveAction(EFalse);
    if(action && (action->softKeyRole()==QAction::PositiveSoftKey || !action->text().compare("yes", Qt::CaseInsensitive))){
        PositiveAction = ETrue;
    }
    NotifyObserver(PositiveAction);
}

CreatorInputDialog::CreatorInputDialog(int* value, MUIObserver* module, int userData) : 
    HbInputDialog(NULL),
    CreatorDialog(module, userData),
    mValue(value)
{
    if(!value)
        throw std::invalid_argument("value cannot be the null!");
}

void CreatorInputDialog::launch(const QString& label, int* value, bool acceptsZero, MUIObserver* observer, int userData) throw( std::exception )
{
    CreatorInputDialog* dlg = new CreatorInputDialog(value, observer, userData);
    dlg->setPromptText(label);
    dlg->setInputMode(IntInput);
    HbValidator *validator = new HbValidator(dlg);
    QString tmp;
    if (acceptsZero == false) {
        tmp.append("[1-9]{1}\\d{1,4}");
    }
    else {
        tmp.append("^[0-9]{5}");
    }
        
    QRegExp rxBasic(tmp);
    validator->addField(new QRegExpValidator(rxBasic, 0), "");
    dlg->setValidator(validator);
    dlg->setValue(QVariant(*value));
    dlg->lineEdit()->setSelection(0, dlg->value().toString().length());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(dlg, SLOT(DialogClosed(HbAction*)));
}

void CreatorInputDialog::DialogClosed(HbAction *action)
{
    TBool PositiveAction(EFalse);
    if(action && (action->softKeyRole()==QAction::PositiveSoftKey || !action->text().compare("ok", Qt::CaseInsensitive))){
        bool ok = false;
        *mValue = value().toInt(&ok);
        PositiveAction = ok ? ETrue : EFalse;
    }
    NotifyObserver(PositiveAction);
}

CreatorSelectionDialog::CreatorSelectionDialog(int* selectedItem, MUIObserver* observer, int userData) throw(std::exception) : 
    HbSelectionDialog(NULL),
    CreatorDialog(observer, userData),
    mSelectedItem(selectedItem),
    mSelectedItems(NULL)
{
    if(!mSelectedItem)
        throw std::invalid_argument("selectedItem cannot be null!");
}

CreatorSelectionDialog::CreatorSelectionDialog(CArrayFixFlat<TInt>* selectedItems, MUIObserver* observer, int userData) throw(std::exception) : 
    HbSelectionDialog(NULL),
    CreatorDialog(observer, userData),
    mSelectedItem(NULL),
    mSelectedItems(selectedItems)
{
    if(!mSelectedItems)
        throw std::invalid_argument("selectedItems cannot be null!");
}

void CreatorSelectionDialog::launch(const QString& label, const QStringList& items, int* selectedItem, MUIObserver* observer, int userData) throw(std::exception)
{
    CreatorSelectionDialog* dlg = new CreatorSelectionDialog(selectedItem, observer, userData);
    if(label.length())
        dlg->setHeadingWidget(new HbLabel(label, dlg));
    dlg->setStringItems(items);
    dlg->setSelectionMode(HbAbstractItemView::SingleSelection); 
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(dlg, SLOT(DialogClosed(HbAction*)));
}

void CreatorSelectionDialog::launch(const QString& label, const QStringList& items, CArrayFixFlat<TInt>* selectedItems, MUIObserver* observer, int userData) throw(std::exception)
{
    CreatorSelectionDialog* dlg = new CreatorSelectionDialog(selectedItems, observer, userData);
    if(label.length())
        dlg->setHeadingWidget(new HbLabel(label, dlg));
    dlg->setStringItems(items);
    dlg->setSelectionMode(HbAbstractItemView::MultiSelection); 
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->open(dlg, SLOT(DialogClosed(HbAction*)));
}

void CreatorSelectionDialog::DialogClosed(HbAction *action)
{
    TBool PositiveAction(EFalse);
    //CreatorSelectionDialog* dlg = qobject_cast<CreatorSelectionDialog*>(sender());
    if(!action || (action && action->softKeyRole()==QAction::SelectSoftKey) || (action && !action->text().compare("ok", Qt::CaseInsensitive))){
        if(selectedItems().count()){
            try{
                if( selectionMode() == HbAbstractItemView::SingleSelection && mSelectedItem){
                    bool ok(false);
                    *mSelectedItem = selectedItems().at(0).toInt(&ok);
                    if(!ok)
                        throw std::invalid_argument("cannot obtain selected item!");
                }
                if( selectionMode() == HbAbstractItemView::MultiSelection && mSelectedItems){
                    QList<QVariant> items = selectedItems();
                    foreach( QVariant item, items){
                        QT_TRAP_THROWING( mSelectedItems->AppendL( item.toInt() ) );
                    }
                }
                PositiveAction = ETrue;
            }
            catch (std::exception& e){
                Notifications::error( QString("exception: ")+e.what() );
            }
        }
    }
    NotifyObserver(PositiveAction);
}

CreatorDateTimeDialog::CreatorDateTimeDialog(TTime* value, MUIObserver* observer, int userData) : 
    HbDialog(NULL),
    CreatorDialog(observer, userData),
    mValue(value)
{
}

void CreatorDateTimeDialog::launch(const QString& label, TTime* value, MUIObserver* observer, int userData) throw( std::exception )
{
    CreatorDateTimeDialog* dlg = new CreatorDateTimeDialog(value, observer, userData);
    if(!value)
        throw std::invalid_argument("value cannot be the null!");
    if(label.length())
        dlg->setHeadingWidget( new HbLabel(label, dlg) );
    
    // TTime to QDate
    TBuf<20> timeString;
    _LIT(KDateString,"%D%M%Y%/0%1%/1%2%/2%3%/3");
    TRAP_IGNORE( value->FormatL(timeString, KDateString) );
    QString temp = QString::fromUtf16(timeString.Ptr(), timeString.Length());
    temp.replace(QChar('/'), QChar('-'));
    QDate date = QDate::fromString(temp, "dd-MM-yyyy");
    HbDateTimePicker* widget = new HbDateTimePicker( date, dlg );
    dlg->setContentWidget( widget );
    dlg->addAction(new HbAction("Ok", dlg));
    dlg->addAction(new HbAction("Cancel", dlg));
    dlg->setModal(true); // Dialog is modal  
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setTimeout(HbPopup::NoTimeout);
    dlg->open(dlg, SLOT(DialogClosed(HbAction*)));
}

void CreatorDateTimeDialog::DialogClosed(HbAction *action)
{
    TBool PositiveAction(EFalse);
    if(action && (action->softKeyRole()==QAction::PositiveSoftKey || !action->text().compare("ok", Qt::CaseInsensitive))){
        QString str = qobject_cast<HbDateTimePicker*>(contentWidget())->date().toString(Qt::ISODate);
        str.remove('-');
        str += ":000000";// 0h 0m 0s
        TBuf<30> dateTimeString(str.utf16());
        mValue->Set(dateTimeString);
        PositiveAction = ETrue;
    }
    NotifyObserver(PositiveAction);
}
//End of File
