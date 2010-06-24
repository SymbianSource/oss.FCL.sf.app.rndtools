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
    iNotepadWrapper = CCreatorNotepadWrapper::NewL();
    }

CCreatorNotepad::~CCreatorNotepad()
    {
    LOGSTRING("Creator: CCreatorNotepad::~CCreatorNotepad");
    
    if (iParameters)
        {
        delete iParameters;
        iParameters;
        }
    
    if (iNotepadWrapper)
    	{
		delete iNotepadWrapper;
    	}
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

    iNotepadWrapper->CreateNoteL(parameters->iNoteText->Des());
    
    return err;
    }

//----------------------------------------------------------------------------
void CCreatorNotepad::DeleteAllL()
    {
    LOGSTRING("Creator: CCreatorNotepad::DeleteAllL");
    iNotepadWrapper->DeleteAllL();
    }

//----------------------------------------------------------------------------
void CCreatorNotepad::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorNotepad::DeleteAllCreatedByCreatorL");
    // Not supported because note id is not available via Notepad API
    User::Leave( KErrNotSupported );
    }
