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


#include <hbprogressdialog.h>
#include <hbmessagebox.h>

#include <hbpopup.h>

#include <QString>
#include <QDate>

#include <e32std.h>
#include <e32base.h>

#include "creator.hrh" // for command ids
#include "notifications.h"
#include "enginewrapper.h"
#include "engine.h"
#include "mainview.h"



// ---------------------------------------------------------------------------

EngineWrapper::EngineWrapper()
: iEngine(0), iProgressDialog(0)
{
}

// ---------------------------------------------------------------------------

EngineWrapper::~EngineWrapper()
{
    if (iEngine != 0) {
        delete iEngine;
    }
    if (iProgressDialog != 0) {
        delete iProgressDialog;
    }
}

// ---------------------------------------------------------------------------

bool EngineWrapper::init()
{
    TRAPD(err, iEngine = CCreatorEngine::NewL(this));
    if(err != KErrNone) {
        return false;
    }
    else {
        return true;
    }
}

// ---------------------------------------------------------------------------

MemoryDetails EngineWrapper::GetMemoryDetails()
{
    TMemoryDetails tMemoryDetails = iEngine->GetMemoryDetails();
    MemoryDetails memoryDetails;

    // Convert TMemoryDetails to MemoryDetails 
    memoryDetails.mCFree  = QString((QChar*)tMemoryDetails.iCFree.Ptr(), tMemoryDetails.iCFree.Length());
	memoryDetails.mDFree  = QString((QChar*)tMemoryDetails.iDFree.Ptr(), tMemoryDetails.iDFree.Length());
	memoryDetails.mEFree  = QString((QChar*)tMemoryDetails.iEFree.Ptr(), tMemoryDetails.iEFree.Length());
	memoryDetails.mHFree  = QString((QChar*)tMemoryDetails.iHFree.Ptr(), tMemoryDetails.iHFree.Length());
	
	memoryDetails.mCSize  = QString((QChar*)tMemoryDetails.iCSize.Ptr(), tMemoryDetails.iCSize.Length());
	memoryDetails.mDSize  = QString((QChar*)tMemoryDetails.iDSize.Ptr(), tMemoryDetails.iDSize.Length());
		memoryDetails.mHSize  = QString((QChar*)tMemoryDetails.iHSize.Ptr(), tMemoryDetails.iHSize.Length());
	
	if (tMemoryDetails.iENotAvailable == EFalse) {
		memoryDetails.mESize  = QString((QChar*)tMemoryDetails.iESize.Ptr(), tMemoryDetails.iESize.Length());
		memoryDetails.mENotAvailable = false;
	}
	else {
		memoryDetails.mENotAvailable = true;
	}
	return memoryDetails;
}


// ---------------------------------------------------------------------------

void EngineWrapper::ShowErrorMessage(const TDesC& aErrorMessage)
{
    QString errorMessage((QChar*)aErrorMessage.Ptr(),aErrorMessage.Length());
    Notifications::error(errorMessage);
 
}

// ---------------------------------------------------------------------------

void EngineWrapper::ShowNote(const TDesC& aNoteMessage, TInt aResourceId)
{
    QString note((QChar*)aNoteMessage.Ptr(),aNoteMessage.Length());
    Notifications::showGlobalNote(note, HbMessageBox::MessageTypeInformation, HbPopup::ConfirmationNoteTimeout);
}

// ---------------------------------------------------------------------------

void EngineWrapper::ShowProgressBar(const TDesC& aPrompt, int aMax)
{
	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
    iProgressDialog = Notifications::showProgressBar(text, aMax);
	connect(iProgressDialog, SIGNAL(cancelled()), this, SLOT(ProgressDialogCancelled()));
}

// ---------------------------------------------------------------------------

void EngineWrapper::IncrementProgressbarValue()
{
    iProgressDialog->setProgressValue(iProgressDialog->progressValue() + 1);
}

// ---------------------------------------------------------------------------

void EngineWrapper::CloseProgressbar()
{
    delete iProgressDialog;
    iProgressDialog = 0;
}

// ---------------------------------------------------------------------------

TBool EngineWrapper::EntriesQueryDialog(TInt& aNumberOfEntries, const TDesC& aPrompt, TBool aAcceptsZero)
    {
	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
    bool err = Notifications::entriesQueryDialog(aNumberOfEntries, text, aAcceptsZero);
	return err;
    }
	
// ---------------------------------------------------------------------------	

TBool EngineWrapper::TimeQueryDialog(TTime aTime, const TDesC& aPrompt)
    {
    // TTime to QDate
    TBuf<20> timeString;
    _LIT(KDateString,"%D%M%Y%/0%1%/1%2%/2%3%/3");
    TRAP_IGNORE( aTime.FormatL(timeString, KDateString) );
    QString temp = QString::fromUtf16(timeString.Ptr(), timeString.Length());
    temp.replace(QChar('/'), QChar('-'));
    QDate date = QDate::fromString(temp, "dd-MM-yyyy");

	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
    bool err = Notifications::timeQueryDialog(date, text);
	return err;
    }

TBool EngineWrapper::YesNoQueryDialog(const TDesC& aPrompt)
{
	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
    return Notifications::yesNoQueryDialog(text);	
}
 
// ---------------------------------------------------------------------------	

bool EngineWrapper::ExecuteOptionsMenuCommand(int commandId)
{
	TInt err = KErrNone;
	if (commandId == ECmdCreateFromFile) {
		TRAP(err, iEngine->RunScriptL());
	}
	else if (commandId == ECmdSelectRandomDataFile) {
		TFileName filename;
		TBool ret = EFalse;
		TRAP(err, ret = iEngine->GetRandomDataFilenameL(filename));
		if (err != KErrNone) {
			Notifications::error("Error in getting random data.");
			return false;
		}		
		if (ret == true) {
			TRAP(err, iEngine->GetRandomDataFromFileL(filename));
		}
	}
    else {
		TRAP(err, iEngine->ExecuteOptionsMenuCommandL(commandId));
	}
	// error handling
    if(err != KErrNone) {
        return false;
    }
    else {
        return true;
    }
}

// ---------------------------------------------------------------------------	

bool EngineWrapper::PopupListDialog(const TDesC& aPrompt, CDesCArray* aFileNameArray, TInt& aIndex) 
{
	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
	QStringList itemList;

	for (int i = 0; i < aFileNameArray->Count(); i++) {
		itemList.append(QString::fromUtf16(
				  aFileNameArray->MdcaPoint(i).Ptr(),
				  aFileNameArray->MdcaPoint(i).Length()));
	}
	// TODO: HbSelectionDialog handle close & user choice
	Notifications::popupListDialog(text, itemList, HbAbstractItemView::SingleSelection);
	return false;
}

// ---------------------------------------------------------------------------	

bool EngineWrapper::DirectoryQueryDialog(const TDesC& aPrompt, TFileName& aDirectory)
{
	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
	QString directory = QString((QChar*)aDirectory.Ptr(), aDirectory.Length());
	bool ret = Notifications::directoryQueryDialog(text, directory);
	if (ret == true) {
		aDirectory = TFileName(directory.utf16());
	}
	return ret;

}


// ---------------------------------------------------------------------------

void EngineWrapper::ProgressDialogCancelled()
{
	CloseProgressbar();
	TRAPD(err, iEngine->ProgressDialogCancelledL());
	// error handling
    if(err != KErrNone) {
        Notifications::error("Error in operation cancel.");
    }	
}

// ---------------------------------------------------------------------------

bool EngineWrapper::ListQueryDialog(const TDesC& aPrompt, TListQueryId aId, TInt& aIndex)
{
	bool ret = false;
	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
	QStringList itemList;
	
	switch (aId) {
		case R_CONTACT_CREATION_TYPE_QUERY: {
			itemList << "Default fields" << "Define...";
			break;
		}
		case R_GROUP_CREATION_TYPE_QUERY: {
			itemList << "Random" << "Define...";
			break;
		}
		case R_MESSAGE_TYPE_QUERY: {
			itemList << "SMS" << "MMS" << "AMS" << "Email" << "Smart Message" << "IR Message" << "BT Message";
			break;
		}
		case R_FOLDER_TYPE_QUERY: {
			itemList << "Inbox" << "Drafts" << "Outbox" << "Sent";
			break;
		}
		case R_UNREAD_QUERY: {
			itemList << "Read" << "New";
			break;
		}
		case R_ATTACHMENT_MULTI_SELECTION_QUERY: {
			Notifications::error("Not supported resource id."); 
			break;
		}
		case R_AMS_ATTACHMENT_SINGLE_SELECTION_QUERY: {
			itemList << "AMR 20kB";
			break;
		}
		case R_ATTACHMENT_SINGLE_SELECTION_QUERY: {
			itemList << "JPEG 25kB" << "JPEG 300kB" << "JPEG 500kB" << "PNG 15kB" << "GIF 2kB" << "RNG 1kB" 
			<< "MIDI 10kB" << "WAVE 20kB" << "AMR 20kB" << "Excel 15kB" << "Word 20kB" << "PowerPoint 40kB" 
			<< "Text 10kB" << "Text 70kB" << "3GPP 70kB" << "MP3 250kB" << "AAC 100kB" << "RM 95kB";
			break;
		}
		default: {
			Notifications::error("Error in resource id.");
			return ret;
		}
	}
    // TODO: HbSelectionDialog handle close & user choice
    Notifications::popupListDialog(text, itemList, HbAbstractItemView::SingleSelection);
    return false;

}

bool EngineWrapper::ListQueryDialog(const TDesC& aPrompt, TListQueryId aId, CArrayFixFlat<TInt>* aIndexes)
{
	bool ret = false;
	QString text((QChar*)aPrompt.Ptr(), aPrompt.Length());
	QStringList itemList;
	QList<int> indexes;
	if (aId == R_ATTACHMENT_MULTI_SELECTION_QUERY) {
		itemList << "None" << "JPEG 25kB" << "JPEG 300kB" << "JPEG 500kB" << "PNG 15kB" << "GIF 2kB" << "RNG 1kB" 
			<< "MIDI 10kB" << "WAVE 20kB" << "AMR 20kB" << "Excel 15kB" << "Word 20kB" << "PowerPoint 40kB" 
			<< "Text 10kB" << "Text 70kB" << "3GPP 70kB" << "MP3 250kB" << "AAC 100kB" << "RM 95kB";

		//ret = Notifications::popupListDialog(text, itemList, indexes);
		// TODO: HbSelectionDialog handle close & user choice
		Notifications::popupListDialog(text, itemList, HbAbstractItemView::MultiSelection);

		if (ret == true) {
			aIndexes->Reset();
			for (int i = 0; i < indexes.count(); i++) {
				aIndexes->AppendL(indexes.at(i));
			}
		}
	}
	return ret;			
}

void EngineWrapper::CloseCreatorApp()
{
	MainView::closeApp();
}
