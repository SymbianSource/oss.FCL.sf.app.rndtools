/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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

    iNotepadApi = CNotepadApi::NewL();
    }

CCreatorNotepad::~CCreatorNotepad()
    {
    LOGSTRING("Creator: CCreatorNotepad::~CCreatorNotepad");
    
    delete iNotepadApi;
    
    if (iParameters)
        delete iParameters;
    }

//----------------------------------------------------------------------------

TBool CCreatorNotepad::AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries)
    {
    LOGSTRING("Creator: CCreatorNotepad::AskDataFromUserL");

    if ( aCommand == ECmdDeleteNotes )
        {
        return iEngine->YesNoQueryDialogL( _L("Delete all Notes?") );
        }
    
    // By Creator not supported because 
    // note id is not available via Notepad API

    return iEngine->EntriesQueryDialogL(aNumberOfEntries, _L("How many entries to create?"));
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

    iNotepadApi->AddContentL(parameters->iNoteText->Des());
    
    return err;
    }

//----------------------------------------------------------------------------
void CCreatorNotepad::DeleteAllL()
    {
    LOGSTRING("Creator: CCreatorNotepad::DeleteAllL");
    // Open Notes db
    RDbs dbs;
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
    CleanupStack::PopAndDestroy( &dbs );
    }

//----------------------------------------------------------------------------
void CCreatorNotepad::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorNotepad::DeleteAllCreatedByCreatorL");
    // Not supported because note id is not available via Notepad API
    User::Leave( KErrNotSupported );
    }
