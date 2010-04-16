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

#if SYMBIAN_VERSION_SUPPORT < SYMBIAN_4
#ifdef __PRESENCE

#include "creator_imps.h" 
#include "creator_traces.h"

#include <cimpssapsettingslist.h>

static const TInt KIMPSFieldLength = 128;

_LIT(KCreatorIMPSPrefixName, "CR_");


//----------------------------------------------------------------------------

CIMPSParameters::CIMPSParameters()
    {
    LOGSTRING("Creator: CIMPSParameters::CIMPSParameters");

    iServerName = HBufC::New(KIMPSFieldLength);
    iServerURL = HBufC::New(KIMPSFieldLength);
    iServerUsername = HBufC::New(KIMPSFieldLength);
    iServerPassword = HBufC::New(KIMPSFieldLength);
    iServerAccessPointName = HBufC::New(KIMPSFieldLength);
    // User needs to set connectionmethod manually to be able to use the IMPS setting
    iServerAccessPointName->Des().Copy(_L("DEFAULT-IAP"));
    }

CIMPSParameters::~CIMPSParameters()
    {
    LOGSTRING("Creator: CIMPSParameters::~CIMPSParameters");

    delete iServerAccessPointName;
    delete iServerPassword;
    delete iServerUsername;
    delete iServerURL;
    delete iServerName;
    }

void CIMPSParameters::SetRandomNameL(CCreatorEngine& aEngine)
    {
    if(iServerName == 0)
        {
        iServerName = HBufC::New(KIMPSFieldLength);
        }
    TBuf<160> company = aEngine.RandomString(CCreatorEngine::ECompany);
    iServerName->Des() = KCreatorIMPSPrefixName;
    iServerName->Des().Append( company );
    iServerName->Des().Append( _L(" #") );
    iServerName->Des().AppendNum( aEngine.RandomNumber(1000, 9999) );    
    }

void CIMPSParameters::SetRandomUrlL(CCreatorEngine& aEngine)
    {
    if(iServerURL == 0)
        {
        iServerURL = HBufC::New(KIMPSFieldLength);
        }
    TBuf<160> company = aEngine.RandomString(CCreatorEngine::ECompany);
    iServerURL->Des() = _L("http://imps.");
    iServerURL->Des().Append( company );
    iServerURL->Des().Append( _L(".com:18080/CSP/csp") );
    }

void CIMPSParameters::SetRandomUsernameL(CCreatorEngine& aEngine)
    {
    iServerUsername->Des() = aEngine.RandomString(CCreatorEngine::EFirstName);
    iServerUsername->Des().LowerCase();
    }

void CIMPSParameters::SetRandomPasswordL(CCreatorEngine& aEngine)
    {
    iServerPassword->Des() = aEngine.RandomString(CCreatorEngine::ESurname);
    iServerPassword->Des().LowerCase();
    }
void CIMPSParameters::SetDefaultAccessPointL()
    {
    iServerAccessPointName->Des().Copy(_L("DEFAULT-IAP"));
    }
//----------------------------------------------------------------------------

CCreatorIMPS* CCreatorIMPS::NewL(CCreatorEngine* aEngine)
    {
    CCreatorIMPS* self = CCreatorIMPS::NewLC(aEngine);
    CleanupStack::Pop(self);
    return self;
    }

CCreatorIMPS* CCreatorIMPS::NewLC(CCreatorEngine* aEngine)
    {
    CCreatorIMPS* self = new (ELeave) CCreatorIMPS;
    CleanupStack::PushL(self);
    self->ConstructL(aEngine);
    return self;
    }

CCreatorIMPS::CCreatorIMPS()
    {
    }

void CCreatorIMPS::ConstructL(CCreatorEngine* aEngine)
    {
    LOGSTRING("Creator: CCreatorIMPS::ConstructL");

    iEngine = aEngine;

    iIMPSSAPSettingsStore = CIMPSSAPSettingsStore::NewL();    
    }

CCreatorIMPS::~CCreatorIMPS()
    {
    LOGSTRING("Creator: CCreatorIMPS::~CCreatorIMPS");

    if ( iEngine && iEntryIds.Count() )
        {
        TRAP_IGNORE( iEngine->WriteEntryIdsToStoreL( iEntryIds, KUidDictionaryUidIMPS ) );
        }
    iEntryIds.Reset();
    iEntryIds.Close();
    
    delete iIMPSSAPSettingsStore;
    
    if (iParameters)
        delete iParameters;
    }

//----------------------------------------------------------------------------

TBool CCreatorIMPS::AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries)
    {
    LOGSTRING("Creator: CCreatorIMPS::AskDataFromUserL");
    if ( aCommand == ECmdDeleteIMPSs )
        {
        return iEngine->YesNoQueryDialogL( _L("Delete all IMPS server entries?") );
        }
    else if ( aCommand ==  ECmdDeleteCreatorIMPSs )
        {
        return iEngine->YesNoQueryDialogL( _L("Delete all IMPS server entries created with Creator?") );
        }
    return iEngine->EntriesQueryDialogL(aNumberOfEntries, _L("How many entries to create?"));
    }


//----------------------------------------------------------------------------

TInt CCreatorIMPS::CreateIMPSServerEntryL(CIMPSParameters *aParameters)
    {
    LOGSTRING("Creator: CCreatorIMPS::CreateIMPSServerEntryL");

    // clear any existing parameter definations
    delete iParameters;
    iParameters = NULL;
    
    CIMPSParameters* parameters = aParameters;
    
    // random data needed if no predefined data available
    if (!parameters)
        {
        iParameters = new(ELeave) CIMPSParameters;
        parameters = iParameters;                
        parameters->SetRandomNameL(*iEngine);
        parameters->SetRandomUrlL(*iEngine);
        parameters->SetRandomUsernameL(*iEngine);
        parameters->SetRandomPasswordL(*iEngine);
        parameters->SetDefaultAccessPointL();
        }

    TInt err = KErrNone;

    // set data to save item
    CIMPSSAPSettings* settings = CIMPSSAPSettings::NewLC();
    TUint32 iapId = iEngine->AccessPointNameToIdL(parameters->iServerAccessPointName->Des(), ETrue );
    err = settings->SetAccessPoint( iapId );
    if( err != KErrNone )
    	{
    	LOGSTRING("Creator: CCreatorIMPS::CreateIMPSServerEntryL - Unable to resolve access point.");
    	err = KErrNone;
    	}
    settings->SetSAPNameL( parameters->iServerName->Des() );
    settings->SetSAPAddressL( parameters->iServerURL->Des() );
    settings->SetSAPUserIdL( parameters->iServerUsername->Des() );
    settings->SetSAPUserPasswordL( parameters->iServerPassword->Des() );
    
    // save the item
    TRAP(err, iIMPSSAPSettingsStore->StoreNewSAPL( settings, EIMPSIMAccessGroup ));
    
    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( settings->Uid() );
    
    TRAP(err, iIMPSSAPSettingsStore->StoreNewSAPL( settings, EIMPSPECAccessGroup ));

    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( settings->Uid() );
    
    CleanupStack::PopAndDestroy(); //settings
    return err;
    }


//----------------------------------------------------------------------------
void CCreatorIMPS::DeleteAllL()
    {
    LOGSTRING("Creator: CCreatorIMPS::DeleteAllL");
    CIMPSSAPSettingsList* imps = CIMPSSAPSettingsList::NewLC();
    iIMPSSAPSettingsStore->PopulateSAPSettingsListL( *imps, EIMPSAccessFilterAll );
    for ( TInt i = 0; i < imps->Count(); i++ )
        {
        iIMPSSAPSettingsStore->DeleteSAPL( imps->UidForIndex( i ) ); 
        }
    CleanupStack::PopAndDestroy( imps );
    
    // reset must be done here, because iEntryIds is stored in destructor
    iEntryIds.Reset();
    
    // all entries deleted, remove the IMPS related registry
    iEngine->RemoveStoreL( KUidDictionaryUidIMPS );
    }

//----------------------------------------------------------------------------
void CCreatorIMPS::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorIMPS::DeleteAllCreatedByCreatorL");
    iEntryIds.Reset();
    
    // fetch ids of entries created by Creator
    iEngine->ReadEntryIdsFromStoreL( iEntryIds, KUidDictionaryUidIMPS );
    
    // delete entries
    for ( TInt i = 0; i < iEntryIds.Count(); i++ )
        {
        TRAP_IGNORE( iIMPSSAPSettingsStore->DeleteSAPL( iEntryIds[i] ) ); 
        }
    // reset must be done here, because iEntryIds is stored in destructor
    iEntryIds.Reset();
    
    // all entries deleted, remove the IMPS related registry
    iEngine->RemoveStoreL( KUidDictionaryUidIMPS );
    }

#endif //__PRESENCE
#endif
