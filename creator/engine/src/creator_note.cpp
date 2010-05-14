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


#include "engine.h"
#include "enginewrapper.h"

#include "creator_note.h" 
#include "creator_traces.h"

const TInt KCreatorDiskSpaceNeededForSingleDeletion( 8192 );
_LIT( KCreatorNotepadFile, "c:Notepad.dat" );

// @see \s60\app\organizer\notepad\notepad1\LibSrc\NpdCoreModel.cpp KSecureUid.Name()
_LIT( KCreatorNotepadFormat, "SECURE[101F8878]" ); 
_LIT( KCreatorNotepadDeleteAllSQL, "DELETE FROM Table1");

//----------------------------------------------------------------------------

CNotepadParameters::CNotepadParameters()
    {
    LOGSTRING("Creator: CNotepadParameters::CNotepadParameters");

    iNoteText = HBufC::New(KNotepadFieldLength);
    }

CNotepadParameters::~CNotepadParameters()
    {
    LOGSTRING("Creator: CNotepadParameters::~CNotepadParameters");

    delete iNoteText;
    }

//----------------------------------------------------------------------------

CCreatorNotepad* CCreatorNotepad::NewL(CCreatorEngine* aEngine)
    {
    CCreatorNotepad* self = CCreatorNotepad::NewLC(aEngine);
    CleanupStack::Pop(self);
    return self;
    }

CCreatorNotepad* CCreatorNotepad::NewLC(CCreatorEngine* aEngine)
    {
    CCreatorNotepad* self = new (ELeave) CCreatorNotepad;
    CleanupStack::PushL(self);
    self->ConstructL(aEngine);
    return self;
    }

CCreatorNotepad::CCreatorNotepad() : iFs ( CEikonEnv::Static()->FsSession() )
    {
    }

void CCreatorNotepad::ConstructL(CCreatorEngine* aEngine)
    {
    LOGSTRING("Creator: CCreatorNotepad::ConstructL");

    iEngine = aEngine;

    iNotepadApi = new NotesEditor();
    //iNotepadApi = CNotepadApi::NewL();
    }

CCreatorNotepad::~CCreatorNotepad()
    {
    LOGSTRING("Creator: CCreatorNotepad::~CCreatorNotepad");
    
    // TODO DELETE!!!
    //delete iNotepadApi;
    
    if (iParameters)
        delete iParameters;
    }

//----------------------------------------------------------------------------

TBool CCreatorNotepad::AskDataFromUserL(TInt aCommand)
    {
    LOGSTRING("Creator: CCreatorNotepad::AskDataFromUserL");

    CCreatorModuleBase::AskDataFromUserL(aCommand);
        
    if ( aCommand == ECmdDeleteNotes )
        {
        return iEngine->GetEngineWrapper()->YesNoQueryDialog( _L("Delete all Notes?"), this, ECreatorModuleDelete );
        }
    
    // By Creator not supported because 
    // note id is not available via Notepad API

    return iEngine->GetEngineWrapper()->EntriesQueryDialog( &iEntriesToBeCreated, _L("How many entries to create?"), EFalse,  this, ECreatorModuleStart );
    }


//----------------------------------------------------------------------------

TInt CCreatorNotepad::CreateNoteEntryL(CNotepadParameters *aParameters)
    {
    LOGSTRING("Creator: CCreatorNotepad::CreateNoteEntryL");

    // clear any existing parameter definations
    delete iParameters;
    iParameters = NULL;
    
    CNotepadParameters* parameters = aParameters;
    
    // random data needed if no predefined data available
    if (!parameters)
        {
        iParameters = new(ELeave) CNotepadParameters;
        parameters = iParameters;
        parameters->iNoteText->Des() = iEngine->RandomString(CCreatorEngine::EMessageText);
        }
    
    TInt err = KErrNone;

    //iNotepadApi->AddContentL(parameters->iNoteText->Des());
    QString textNote = QString::fromUtf16(parameters->iNoteText->Ptr(),parameters->iNoteText->Length());
    iNotepadApi->edit(textNote);
    iNotepadApi->close(NotesEditor::CloseWithSave);
    
    return err;
    }

//----------------------------------------------------------------------------
void CCreatorNotepad::DeleteAllL()
    {
    LOGSTRING("Creator: CCreatorNotepad::DeleteAllL");
    QList<AgendaEntry> ael;
    AgendaUtil::FilterFlags filter = AgendaUtil::FilterFlags(AgendaUtil::IncludeNotes);

    iAgendaUtil = new AgendaUtil();
    ael = iAgendaUtil->fetchAllEntries(filter);
    for(int i=0 ; i<ael.count() ; i++)
    	{
        iAgendaUtil->deleteEntry(ael[i].id());
    	}
    delete iAgendaUtil;
    // Open Notes db
 /*   RDbs dbs;
    User::LeaveIfError( dbs.Connect() );
    CleanupClosePushL( dbs );
    RDbNamedDatabase db;
    TInt openErr( db.Open( dbs, KCreatorNotepadFile, KCreatorNotepadFormat ) );
    CleanupClosePushL( db );
    
    if ( openErr && openErr !=  KErrNotFound )
        {
        User::Leave( openErr );
        }
    
    // do not leave if openErr == KErrNotFound, 
    // it means there is no notes (file) created -> no need to delete
    
    if ( openErr !=  KErrNotFound )
        {
        TInt retval = iFs.ReserveDriveSpace( KDefaultDrive, KCreatorDiskSpaceNeededForSingleDeletion );
        if ( retval == KErrNone )
            {
            retval = iFs.GetReserveAccess( KDefaultDrive );
            }

        // Delete all Notes. Ignore rowCount returnvalue
        db.Execute( KCreatorNotepadDeleteAllSQL );            
        
        User::LeaveIfError( db.Compact() );
        
        if ( retval == KErrNone )
            {
            retval = iFs.ReleaseReserveAccess( KDefaultDrive );
            }
        }
    
    CleanupStack::PopAndDestroy( &db );
    CleanupStack::PopAndDestroy( &dbs );*/
    }

//----------------------------------------------------------------------------
void CCreatorNotepad::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorNotepad::DeleteAllCreatedByCreatorL");
    // Not supported because note id is not available via Notepad API
    User::Leave( KErrNotSupported );
    }
