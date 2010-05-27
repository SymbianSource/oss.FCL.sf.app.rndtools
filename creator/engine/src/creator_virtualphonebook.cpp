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
#include "creator_virtualphonebook.h" 
#include "creator_traces.h"
#include "creator_contactsetcache.h"

#include <cvpbkcontactstoreuriarray.h>
#include <vpbkcontactstoreuris.h>
#include <tvpbkcontactstoreuriptr.h>
#include <mvpbkcontactgroup.h>
#include <mvpbkcontactstore.h>
#include <cvpbkcontactmanager.h>
#include <mvpbkcontactstorelist.h>
#include <mvpbkcontactstorelistobserver.h>
#include <mvpbkcontactoperationbase.h>
#include <mvpbkstorecontact.h>
#include <mvpbkcontactstoreproperties.h>
#include <mvpbkcontactfielddata.h>
#include <mvpbkfieldtype.h>
#include <mvpbkcontactfieldtextdata.h>
#include <mvpbkcontactfieldbinarydata.h>
#include <mvpbkcontactfielddatetimedata.h>
#include <CVPbkContactViewDefinition.h>
#include <mvpbkcontactview.h>

#include <vpbkcontactview.hrh>
#include <vpbkeng.rsg>

typedef struct {
TInt iFieldCode;
TInt iRandomType;
} FieldInfo;
static const TInt RND_TYPE_UNDEF = -99;
FieldInfo CreatorVPbkMiscTextFields[] = {        
        {R_VPBK_FIELD_TYPE_FIRSTNAME, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_FIRSTNAMEREADING, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_LASTNAME, (TInt) CCreatorEngine::ESurname},
        {R_VPBK_FIELD_TYPE_LASTNAMEREADING, (TInt) CCreatorEngine::ESurname},
        {R_VPBK_FIELD_TYPE_COMPANYNAME, (TInt) CCreatorEngine::ECompany},
        {R_VPBK_FIELD_TYPE_JOBTITLE, (TInt) CCreatorEngine::EJobTitle},
        {R_VPBK_FIELD_TYPE_PREFIX, (TInt) CCreatorEngine::EPrefix},
        {R_VPBK_FIELD_TYPE_SUFFIX, (TInt) CCreatorEngine::ESuffix},
        {R_VPBK_FIELD_TYPE_SECONDNAME, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_ADDRLABELGEN, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRPOGEN, (TInt) CCreatorEngine::EPobox},
        {R_VPBK_FIELD_TYPE_ADDREXTGEN, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRSTREETGEN, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRLOCALGEN, (TInt) CCreatorEngine::ECity},
        {R_VPBK_FIELD_TYPE_ADDRREGIONGEN, (TInt) CCreatorEngine::EState},
        {R_VPBK_FIELD_TYPE_ADDRPOSTCODEGEN, (TInt) CCreatorEngine::EPostcode},
        {R_VPBK_FIELD_TYPE_ADDRCOUNTRYGEN, (TInt) CCreatorEngine::ECountry},
        {R_VPBK_FIELD_TYPE_ADDRLABELHOME, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRPOHOME, (TInt) CCreatorEngine::EPobox},
        {R_VPBK_FIELD_TYPE_ADDREXTHOME, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRSTREETHOME, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRLOCALHOME, (TInt) CCreatorEngine::ECity},
        {R_VPBK_FIELD_TYPE_ADDRREGIONHOME, (TInt) CCreatorEngine::EState},
        {R_VPBK_FIELD_TYPE_ADDRPOSTCODEHOME, (TInt) CCreatorEngine::EPostcode},
        {R_VPBK_FIELD_TYPE_ADDRCOUNTRYHOME, (TInt) CCreatorEngine::ECountry},
        {R_VPBK_FIELD_TYPE_ADDRLABELWORK, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRPOWORK, (TInt) CCreatorEngine::EPobox},
        {R_VPBK_FIELD_TYPE_ADDREXTWORK, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRSTREETWORK, (TInt) CCreatorEngine::EAddress},
        {R_VPBK_FIELD_TYPE_ADDRLOCALWORK, (TInt) CCreatorEngine::ECity},
        {R_VPBK_FIELD_TYPE_ADDRREGIONWORK, (TInt) CCreatorEngine::EState},
        {R_VPBK_FIELD_TYPE_ADDRPOSTCODEWORK, (TInt) CCreatorEngine::EPostcode},
        {R_VPBK_FIELD_TYPE_ADDRCOUNTRYWORK, (TInt) CCreatorEngine::ECountry},
        {R_VPBK_FIELD_TYPE_POC, (TInt) CCreatorEngine::EPhoneNumber},
        {R_VPBK_FIELD_TYPE_SWIS, (TInt) CCreatorEngine::EPhoneNumber},
        {R_VPBK_FIELD_TYPE_SIP, (TInt) CCreatorEngine::EPhoneNumber},
        {R_VPBK_FIELD_TYPE_DTMFSTRING, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_NOTE,(TInt)  CCreatorEngine::EMemoText},
        {R_VPBK_FIELD_TYPE_MIDDLENAME, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_DEPARTMENT, (TInt) CCreatorEngine::ECompany},
        {R_VPBK_FIELD_TYPE_ASSTNAME, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_SPOUSE, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_CHILDREN, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_SYNCCLASS, RND_TYPE_UNDEF},
        {R_VPBK_FIELD_TYPE_LOCPRIVACY, RND_TYPE_UNDEF},
        {R_VPBK_FIELD_TYPE_GENLABEL, (TInt) CCreatorEngine::EFirstName},
        {R_VPBK_FIELD_TYPE_WVADDRESS, (TInt) CCreatorEngine::EPhoneNumber},
        {R_VPBK_FIELD_TYPE_RINGTONE, RND_TYPE_UNDEF},
        {R_VPBK_FIELD_TYPE_THUMBNAILPIC, RND_TYPE_UNDEF},
        {R_VPBK_FIELD_TYPE_CALLEROBJTEXT, (TInt) CCreatorEngine::EFirstName}
        };
                    
TInt CreatorVPbkBinaryFields[] = {
        R_VPBK_FIELD_TYPE_CALLEROBJIMG//,
        //R_VPBK_FIELD_TYPE_THUMBNAILPATH
        };

TInt CreatorVPbkDateTimeFields[] = {
        R_VPBK_FIELD_TYPE_ANNIVERSARY
        };

//----------------------------------------------------------------------------
TInt CreatorVPbkPhoneNumberFields[] =
    {
    R_VPBK_FIELD_TYPE_LANDPHONEGEN,
    R_VPBK_FIELD_TYPE_LANDPHONEHOME,
    R_VPBK_FIELD_TYPE_LANDPHONEWORK,                        
    R_VPBK_FIELD_TYPE_MOBILEPHONEGEN,
    R_VPBK_FIELD_TYPE_MOBILEPHONEHOME,
    R_VPBK_FIELD_TYPE_MOBILEPHONEWORK,
    R_VPBK_FIELD_TYPE_FAXNUMBERGEN,
    R_VPBK_FIELD_TYPE_FAXNUMBERHOME,
    R_VPBK_FIELD_TYPE_FAXNUMBERWORK,
    R_VPBK_FIELD_TYPE_PAGERNUMBER,           
    R_VPBK_FIELD_TYPE_VIDEONUMBERGEN,
    R_VPBK_FIELD_TYPE_VIDEONUMBERHOME,
    R_VPBK_FIELD_TYPE_VIDEONUMBERWORK,
    R_VPBK_FIELD_TYPE_VOIPGEN,
    R_VPBK_FIELD_TYPE_VOIPHOME,
    R_VPBK_FIELD_TYPE_VOIPWORK,
    R_VPBK_FIELD_TYPE_ASSTPHONE,
    R_VPBK_FIELD_TYPE_CARPHONE
    };

TInt CreatorVPbkUrlFields[] =
    {
    R_VPBK_FIELD_TYPE_URLGEN,
    R_VPBK_FIELD_TYPE_URLHOME,
    R_VPBK_FIELD_TYPE_URLWORK
    };

TInt CreatorVPbkEmailFields[] =
    {
    R_VPBK_FIELD_TYPE_EMAILGEN,
    R_VPBK_FIELD_TYPE_EMAILHOME,
    R_VPBK_FIELD_TYPE_EMAILWORK
    };

//----------------------------------------------------------------------------

CVirtualPhonebookParameters::CVirtualPhonebookParameters()
    {
    LOGSTRING("Creator: CVirtualPhonebookParameters::CVirtualPhonebookParameters");
    iGroupName = HBufC::New(KPhonebookFieldLength);  
    }

CVirtualPhonebookParameters::~CVirtualPhonebookParameters()
    {
    LOGSTRING("Creator: CVirtualPhonebookParameters::~CVirtualPhonebookParameters");

    delete iGroupName;   
    iContactFields.ResetAndDestroy();
    iContactFields.Close();
    iLinkIds.Reset();
    iLinkIds.Close();
    }

void CVirtualPhonebookParameters::ParseL(CCommandParser* /*parser*/, TParseParams /*aCase = 0*/)
	{
	}

TInt CVirtualPhonebookParameters::ScriptLinkId() const
    {
    return iLinkId;
    }

void CVirtualPhonebookParameters::SetScriptLinkId(TInt aLinkId)
    {
    iLinkId = aLinkId;
    }

//----------------------------------------------------------------------------

CCreatorVirtualPhonebook* CCreatorVirtualPhonebook::NewL(CCreatorEngine* aEngine)
    {
    CCreatorVirtualPhonebook* self = CCreatorVirtualPhonebook::NewLC(aEngine);
    CleanupStack::Pop(self);
    return self;
    }

CCreatorVirtualPhonebook* CCreatorVirtualPhonebook::NewLC(CCreatorEngine* aEngine)
    {
    CCreatorVirtualPhonebook* self = new (ELeave) CCreatorVirtualPhonebook;
    CleanupStack::PushL(self);
    self->ConstructL(aEngine);
    return self;
    }

CCreatorVirtualPhonebook::CCreatorVirtualPhonebook()
    {    
    iAddAllFields = EFalse;
    }

void CCreatorVirtualPhonebook::ConstructL(CCreatorEngine* aEngine)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::ConstructL");

    iContactLinkArray = CVPbkContactLinkArray::NewL();
    iContactsToDelete = CVPbkContactLinkArray::NewL();
    iContactGroupsToDelete = CVPbkContactLinkArray::NewL();
    
    iEngine = aEngine;
    SetDefaultParameters();
    
    // initialize virtual phonebook
    CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
    uriArray->AppendL( TVPbkContactStoreUriPtr(VPbkContactStoreUris::DefaultCntDbUri()));
    iContactManager = CVPbkContactManager::NewL( *uriArray, &CCoeEnv::Static()->FsSession());
    CleanupStack::PopAndDestroy(uriArray);
    
    //When the contact manager is created, the stores is opened
    MVPbkContactStoreList& storeList = iContactManager->ContactStoresL();
    
    iWaiter = CAsyncWaiter::NewL();

    //MVPbkContactStoreListObserver must give as parameter    
    storeList.OpenAllL(*this);
    // wait for OpenComplete() callback
    iWaiter->StartAndWait();
    
    _LIT(dbUri, "cntdb://c:contacts.cdb");
    const TVPbkContactStoreUriPtr uri = TVPbkContactStoreUriPtr(dbUri);
       
    iStore = storeList.Find(uri);    
    iOpCounter = 0;    
    }

TBool CCreatorVirtualPhonebook::IsActive()
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::IsActive");
    return iOperation != NULL;
    }

void CCreatorVirtualPhonebook::CancelOperation()
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::CancelOperation");
    iCancelCbRequested = ETrue;    
    }

CCreatorVirtualPhonebook::~CCreatorVirtualPhonebook()
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::~CCreatorVirtualPhonebook");
    
    // this is done only once per phonebook operation
    if ( iContactsToDelete && iContactsToDelete->Count() )
        {
        TRAP_IGNORE( StoreLinksForDeleteL( *iContactsToDelete, KUidDictionaryUidContacts ) );
        }
    delete iContactsToDelete;
    if ( iContactGroupsToDelete && iContactGroupsToDelete->Count() )
        {
        TRAP_IGNORE( StoreLinksForDeleteL( *iContactGroupsToDelete, KUidDictionaryUidContactGroups ) );
        }
    delete iContactGroupsToDelete;
    
    iPreviousDeleteLinks.ResetAndDestroy();
    
    delete iOperation;
    
    TInt err = 0;
    TRAP(err, CompactPbkDatabaseL( ETrue ));

    if(iContactResults)
    	{
    	delete iContactResults;
    	}
    
    delete iContactLinkArray;
    delete iContactGroupsInStore;
    
    if(iContactManager)
    	{
    	TRAP(err, iContactManager->ContactStoresL().CloseAll(*this));
    	delete iContactManager;
    	}
    if(iWaiter)
    	{
    	delete iWaiter;    
    	}
    if (iParameters)
    	{
        delete iParameters;
    	}
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::DeleteAllL()
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::DeleteAllL");
    
    // Delete all contacts, not contact groups
    
    //get field types
    const MVPbkFieldTypeList& fieldList = iStore->StoreProperties().SupportedFields();
        
    // get all contact links, results will be set to iContactResults
    MVPbkContactOperationBase* operation = iContactManager->FindL( _L(""), fieldList , *this );
    if ( operation )
        {
        iWaiter->StartAndWait(); //Making asynchronous FindL to synchronous
        delete operation;
        }
    
    while( iContactResults->Count() && 
           !iCancelCbRequested &&
           HasOtherThanGroupsL( iContactResults ) )
        {
        // delete all found contacts
        DeleteContactsL( iContactResults, EFalse );
        
        if ( !iCancelCbRequested )
            {
            // find next set of contacts to delete
            MVPbkContactOperationBase* operation = iContactManager->FindL( _L(""), fieldList , *this );
            if ( operation )
                {
                iWaiter->StartAndWait(); //Making asynchronous FindL to synchronous
                delete operation;
                }
            }
        }
    
    if ( iCancelCbRequested && iEngine )
        {
        // User cancelled, must callback to finish terminatio sequence
        iEngine->CancelComplete();
        }
    else
        {
        // contacts deleted, remove the Creator internal contact registry
        // (no callback required)
        CDictionaryFileStore* store = iEngine->FileStoreLC();
        if ( store )
            {
            store->Remove( KUidDictionaryUidContacts );
            store->CommitL();        
            }
        CleanupStack::PopAndDestroy( store );
        }    
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::DeleteAllCreatedByCreatorL");
    DeleteItemsCreatedWithCreatorL( KUidDictionaryUidContacts );
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::DeleteAllGroupsL()
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::DeleteAllGroupsL");
    User::LeaveIfNull( iStore );
    MVPbkContactLinkArray* groups = iStore->ContactGroupsLC();
    DeleteContactsL( groups, ETrue );
    CleanupStack::PopAndDestroy(); // cannot use groups as parameter
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::DeleteAllGroupsCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::DeleteAllGroupsCreatedByCreatorL");
    DeleteItemsCreatedWithCreatorL( KUidDictionaryUidContactGroups );
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::DeleteContactsL( MVPbkContactLinkArray* aContacts, TBool aGroup )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::DeleteContactsL");
    const MVPbkContactLink* link( NULL );
    delete iOperation;
    iOperation = NULL;
    TInt i(0);
    while ( aContacts && i < aContacts->Count() && !iCancelCbRequested )
        {
        link = &aContacts->At( i++ );
        if ( aGroup == IsContactGroupL( *link ) )
            {
            iOperation = iContactManager->RetrieveContactL( *link, *this );
            // see VPbkSingleContactOperationComplete
            if ( iOperation )
                {
                iWaiter->StartAndWait();
                delete iOperation;
                iOperation = NULL;
                }            
            }
        link = NULL;
        }
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::DeleteItemsCreatedWithCreatorL( TUid aStoreUid )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::DeleteItemsCreatedWithCreatorL");
    __ASSERT_ALWAYS( aStoreUid == KUidDictionaryUidContacts || 
                     aStoreUid == KUidDictionaryUidContactGroups,
                     User::Panic( _L("CCreatorVPb"), KErrArgument ) );
    CDictionaryFileStore* store = iEngine->FileStoreLC();
    User::LeaveIfNull( store );
    if ( store->IsPresentL( aStoreUid ) )
        {
        TRAP_IGNORE( DoDeleteItemsCreatedWithCreatorL( aStoreUid, store ) );

        if ( iCancelCbRequested && iEngine )
            {
            iEngine->CancelComplete();
            }
        else
            {
            // contacts deleted, remove the Creator internal contact registry
            store->Remove( aStoreUid );
            store->CommitL();            
            }
        }
    CleanupStack::PopAndDestroy( store );
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::DoDeleteItemsCreatedWithCreatorL( TUid aStoreUid, CDictionaryFileStore* aStore )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::DoDeleteItemsCreatedWithCreatorL");
    RDictionaryReadStream in;
    in.OpenLC( *aStore, aStoreUid );
    MVPbkContactLinkArray* contactsToDelete = NULL;
    // fetch contact links from store
    while ( ( contactsToDelete = iContactManager->CreateLinksLC( in ) ) != NULL && // will leave with KErrEof
            !iCancelCbRequested ) 
        {
        DeleteContactsL( contactsToDelete, aStoreUid == KUidDictionaryUidContactGroups );
        // PopAndDestroy for contactsToDelete causes E32USER-CBase:90
        // however there is no mem-leak even if not deleting contactsToDelete object
        }
    CleanupStack::PopAndDestroy(); // in
    }

//----------------------------------------------------------------------------
TBool CCreatorVirtualPhonebook::HasOtherThanGroupsL( MVPbkContactLinkArray* aContacts )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::HasOtherThanGroupsL");
    TBool result( EFalse );
    TInt i(0);
    while ( aContacts && i < aContacts->Count() && !iCancelCbRequested && !result )
        {
        if ( !IsContactGroupL( aContacts->At( i++ ) ) )
            {
            result = ETrue;
            }
        }
    return result;
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::VPbkSingleContactOperationComplete(
                MVPbkContactOperationBase& /*aOperation*/,
                MVPbkStoreContact* aContact )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::VPbkSingleContactOperationComplete");
    
    // This callback is currently used only for deleting a contact
    // See DeleteContactsL
    
    TRAP_IGNORE( 
            aContact->PushL();
            aContact->DeleteL( *this );
            CleanupStack::PopAndDestroy( aContact );
        );
    }

void CCreatorVirtualPhonebook::VPbkSingleContactOperationFailed(
                MVPbkContactOperationBase& /*aOperation*/, 
                TInt /*aError*/ )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::VPbkSingleContactOperationFailed");
    iWaiter->Cancel();
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::StoreLinksForDeleteL( MVPbkContactLinkArray& aLinks, TUid aStoreUid )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::StoreLinksForDeleteL");
    CDictionaryFileStore* store = iEngine->FileStoreLC();
    User::LeaveIfNull( store );
    
    // backup previous contact links from store
    // otherwise they would be overwritten when calling out.WriteL
    MVPbkContactLinkArray* previousLinks( NULL );
    if ( store->IsPresentL( aStoreUid ) )
        {
        RDictionaryReadStream in;
        in.OpenLC( *store, aStoreUid );        
        TRAP_IGNORE( 
            while ( ( previousLinks = iContactManager->CreateLinksLC( in )) != NULL ) // will leave with KErrEof
                {
                CleanupStack::Pop(); // previousLinks
                iPreviousDeleteLinks.Append( previousLinks );
                previousLinks = NULL;
                }
            );
        CleanupStack::PopAndDestroy(); // in
        }

    RDictionaryWriteStream out;       
    out.AssignLC( *store, aStoreUid );
    
    // restore previous links
    for ( TInt i = 0; i < iPreviousDeleteLinks.Count(); i++ )
        {
        out.WriteL( *iPreviousDeleteLinks[i]->PackLC() );
        CleanupStack::PopAndDestroy(); // PackLC            
        }
    iPreviousDeleteLinks.ResetAndDestroy();
    
    // write new links
    out.WriteL( *aLinks.PackLC() );
    out.CommitL();
    CleanupStack::PopAndDestroy(); // aLinks.PackLC
    CleanupStack::PopAndDestroy(); // out
    
    store->CommitL();
    CleanupStack::PopAndDestroy( store );
    }

//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::AddFieldToParamsL(TInt aFieldType, const TDesC& aData)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::AddFieldToParamsL(TInt aFieldType, const TDesC& aData)");
    if(iParameters == 0)
        return;
    CCreatorContactField* field = CCreatorContactField::NewL(aFieldType, aData);
    CleanupStack::PushL(field);
    iParameters->iContactFields.AppendL(field);
    CleanupStack::Pop();
    }

void CCreatorVirtualPhonebook::AddFieldToParamsL(TInt aFieldType, const TDesC8& aData)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::AddFieldToParamsL(TInt aFieldType, const TDesC8& aData)");
    if(iParameters == 0)
        return;
    CCreatorContactField* field = CCreatorContactField::NewL(aFieldType, aData);
    CleanupStack::PushL(field);
    iParameters->iContactFields.AppendL(field);
    CleanupStack::Pop();
    }

void CCreatorVirtualPhonebook::AddFieldToParamsL(TInt aFieldType, const TTime& aData)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::AddFieldToParamsL(TInt aFieldType, const TTime& aData)");
    if(iParameters == 0)
        return;
    CCreatorContactField* field = CCreatorContactField::NewL(aFieldType, aData);
    CleanupStack::PushL(field);
    iParameters->iContactFields.AppendL(field);
    CleanupStack::Pop();
    }
       
//----------------------------------------------------------------------------
TInt CCreatorVirtualPhonebook::iPhoneNumberFields[] =
    {
    R_VPBK_FIELD_TYPE_LANDPHONEGEN,
    R_VPBK_FIELD_TYPE_LANDPHONEHOME,
    R_VPBK_FIELD_TYPE_LANDPHONEWORK,                        
    R_VPBK_FIELD_TYPE_MOBILEPHONEGEN,
    R_VPBK_FIELD_TYPE_MOBILEPHONEHOME,
    R_VPBK_FIELD_TYPE_MOBILEPHONEWORK,
    R_VPBK_FIELD_TYPE_FAXNUMBERGEN,
    R_VPBK_FIELD_TYPE_FAXNUMBERHOME,
    R_VPBK_FIELD_TYPE_FAXNUMBERWORK,
    R_VPBK_FIELD_TYPE_PAGERNUMBER,           
    R_VPBK_FIELD_TYPE_VIDEONUMBERGEN,
    R_VPBK_FIELD_TYPE_VIDEONUMBERHOME,
    R_VPBK_FIELD_TYPE_VIDEONUMBERWORK,
    R_VPBK_FIELD_TYPE_VOIPGEN,
    R_VPBK_FIELD_TYPE_VOIPHOME,
    R_VPBK_FIELD_TYPE_VOIPWORK,
    R_VPBK_FIELD_TYPE_ASSTPHONE,
    R_VPBK_FIELD_TYPE_CARPHONE
    };

TInt CCreatorVirtualPhonebook::iUrlFields[] =
    {
    R_VPBK_FIELD_TYPE_URLGEN,
    R_VPBK_FIELD_TYPE_URLHOME,
    R_VPBK_FIELD_TYPE_URLWORK
    };

TInt CCreatorVirtualPhonebook::iEmailFields[] =
    {
    R_VPBK_FIELD_TYPE_EMAILGEN,
    R_VPBK_FIELD_TYPE_EMAILHOME,
    R_VPBK_FIELD_TYPE_EMAILWORK
    };

void CCreatorVirtualPhonebook::InitializeContactParamsL(/*CCreatorModuleBaseParameters* aParameters*/)
    {
    LOGSTRING("Creator: CCreatorPhonebook::InitializeContactParamsL");        
    iParameters = new (ELeave) CVirtualPhonebookParameters;
        
    iParameters->iNumberOfPhoneNumberFields = iNumberOfPhoneNumberFields;
    iParameters->iNumberOfURLFields = iNumberOfURLFields;
    iParameters->iNumberOfEmailAddressFields = iNumberOfEmailAddressFields;       
        
    TPtrC firstname = iEngine->RandomString(CCreatorEngine::EFirstName);
    TPtrC lastname  = iEngine->RandomString(CCreatorEngine::ESurname);               
                    
    if( iAddAllFields )
        {
        // Add text fields:
        TInt textFieldCount = sizeof(CreatorVPbkMiscTextFields) / sizeof(FieldInfo);
        for( TInt tfIndex = 0; tfIndex < textFieldCount; ++tfIndex )
            {                
            CCreatorContactField* field = CCreatorContactField::NewL(CreatorVPbkMiscTextFields[tfIndex].iFieldCode, KNullDesC);
            CleanupStack::PushL(field);
            field->SetRandomParametersL(CCreatorContactField::ERandomLengthDefault);
            iParameters->iContactFields.AppendL(field);
            CleanupStack::Pop(field);
            }
            
        // Add binary fields:
        TInt binFieldCount = sizeof(CreatorVPbkBinaryFields) / sizeof(TInt);
        for( TInt bfIndex = 0; bfIndex < binFieldCount; ++bfIndex )
            {                
            CCreatorContactField* field = CCreatorContactField::NewL(CreatorVPbkBinaryFields[bfIndex], KNullDesC8);
            CleanupStack::PushL(field);
            field->SetRandomParametersL(CCreatorContactField::ERandomLengthDefault);
            iParameters->iContactFields.AppendL(field);
            CleanupStack::Pop(field);
            }
            
        // Add date-time fields:
        TInt dtFieldCount = sizeof(CreatorVPbkDateTimeFields) / sizeof(TInt);
        for( TInt dtIndex = 0; dtIndex < dtFieldCount; ++dtIndex )
            {
            AddFieldToParamsL(CreatorVPbkDateTimeFields[dtIndex], iEngine->RandomDate(CCreatorEngine::EDateFuture));
            }

        AddFieldToParamsL(R_VPBK_FIELD_TYPE_CALLEROBJIMG, KNullDesC8);        
        AddFieldToParamsL(R_VPBK_FIELD_TYPE_THUMBNAILPIC, KNullDesC8);        
        AddFieldToParamsL(R_VPBK_FIELD_TYPE_CALLEROBJTEXT, firstname);
        }
    else
        {                      
        AddFieldToParamsL(R_VPBK_FIELD_TYPE_FIRSTNAME, firstname);            
        AddFieldToParamsL(R_VPBK_FIELD_TYPE_LASTNAME, lastname);            
        }
                    
    // Phone numbers:
    TInt arraySize = sizeof(iPhoneNumberFields)/sizeof(TInt);
    TInt index = 0;
    for (TInt i=0; i<iParameters->iNumberOfPhoneNumberFields; i++ )
        {            
        if( index >= arraySize )
            {
            index = 0;
            }
        if( arraySize > 0  )
            {
            CCreatorContactField* field = CCreatorContactField::NewL(iPhoneNumberFields[index], KNullDesC);
            CleanupStack::PushL(field);
            field->SetRandomParametersL(CCreatorContactField::ERandomLengthDefault);
            iParameters->iContactFields.AppendL(field);
            CleanupStack::Pop(field);
            }
        ++index;
        }
            
       
    // URLs:
    arraySize = sizeof(iUrlFields)/sizeof(TInt);
    index = 0;
    for( TInt i = 0; i < iParameters->iNumberOfURLFields; ++i)
        {
        if( index >= arraySize )
            {
            index = 0;
            }
        if( arraySize > 0  )
            {
            CCreatorContactField* field = CCreatorContactField::NewL(iUrlFields[index], KNullDesC);
            CleanupStack::PushL(field);
            field->SetRandomParametersL(CCreatorContactField::ERandomLengthDefault);
            iParameters->iContactFields.AppendL(field);
            CleanupStack::Pop(field);            
            }
        ++index;
        }
                    
    // EMail addresses:
    arraySize = sizeof(iEmailFields)/sizeof(TInt);
    index = 0;
    for( TInt i = 0; i < iParameters->iNumberOfEmailAddressFields; ++i)
        {
        if( index >= arraySize )
            {
            index = 0;
            }
        if( arraySize > 0  )
            {
            CCreatorContactField* field = CCreatorContactField::NewL(iEmailFields[index], KNullDesC);
            CleanupStack::PushL(field);
            field->SetRandomParametersL(CCreatorContactField::ERandomLengthDefault);
            iParameters->iContactFields.AppendL(field);
            CleanupStack::Pop(field);               
            }
        ++index;
        }
    }

TInt CCreatorVirtualPhonebook::CreateContactEntryL(CCreatorModuleBaseParameters *aParameters)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::CreateContactEntryL");
    
    delete iParameters;
    iParameters = 0;
    CVirtualPhonebookParameters* parameters = 0;
    
    if( aParameters == 0 )
        {
        InitializeContactParamsL();
        parameters = iParameters;
        }
    else
        {
        parameters = (CVirtualPhonebookParameters*) aParameters;
        }
    
    TInt err = KErrNone;

    // create a new contact item
    MVPbkStoreContact* newContact = iStore->CreateNewContactLC();
    for( TInt i = 0; i < parameters->iContactFields.Count(); ++i )
        {
        CCreatorContactField* cField = parameters->iContactFields[i];
        if( cField )
            {
            cField->AddToContactL(iEngine, *newContact, this);
            }
        }

    RArray<MVPbkStoreContact*> contacts;
    CleanupClosePushL( contacts );    
    contacts.Append(newContact);        
    
    delete iOperation;
    iOperation = NULL;
    iOperation = iContactManager->CommitContactsL( contacts.Array(), *this );       
    if ( iOperation )
        {

        iWaiter->StartAndWait();//Making asynchronous CommitContctsL to synchronous
        delete iOperation;
        iOperation = NULL;
        }    
    
    // store the link to contact, so that Creator is able to delete
    // it when user requests deletion of contacts that are created by Creator 
    MVPbkContactLink* linkToContact = newContact->CreateLinkLC();
    iContactsToDelete->AppendL( linkToContact );
    CleanupStack::Pop(); // linkToContact
    
    // If this contact has a link id in script (i.e. belongs to a contact-set), we must cache the contact id:
    if( parameters->ScriptLinkId() > 0 )
        {                
        RPointerArray<CCreatorContactSet>& contactsets = ContactLinkCache::Instance()->ContactSets();
        TBool setFound(EFalse);
        for(TInt i = 0; i < contactsets.Count(); ++i )
            {
            if( contactsets[i]->LinkId() == parameters->ScriptLinkId())
                {
                MVPbkContactLink* linkToContact = newContact->CreateLinkLC();
                if( linkToContact )
                    {
                    contactsets[i]->AppendL(linkToContact);
                    iContactLinkArray->AppendL(linkToContact);
                    CleanupStack::Pop(); // linkToContact
                    }
                setFound = ETrue;
                break;
                }
            }
        if( !setFound )
        	{
        	LOGSTRING2("Error: Contact set id %d not found.", parameters->ScriptLinkId());
        	}
        }
    
    CleanupStack::Pop();//contacts
    contacts.Close();
    CompactPbkDatabaseL();
    CleanupStack::PopAndDestroy();//newContact
    if ( iCancelCbRequested && iEngine ) iEngine->CancelComplete();
    return err;
    }


//----------------------------------------------------------------------------

void CCreatorVirtualPhonebook::AddImageToContactL(MVPbkStoreContact& aContact, TInt aFieldResId, const TDesC& aFileName)
    {
    LOGSTRING("Creator: CCreatorPhonebook::AddImageToContactL");
    CPbk2ImageManager* imageMgr = CPbk2ImageManager::NewL(*iContactManager);
    CleanupStack::PushL(imageMgr);
    const MVPbkFieldTypeList& fieldList = aContact.ParentStore().StoreProperties().SupportedFields();        
    const MVPbkFieldType* fieldType = fieldList.Find(aFieldResId);
    if( fieldType == 0 )
        {
        return;
        }
    MPbk2ImageOperation* imageOp = 0;
    TRAPD(err, imageOp = imageMgr->SetImageAsyncL(aContact, *fieldType, *this, aFileName));   
    if( err == KErrNone)
        {
        if (imageOp)
            {
            CleanupDeletePushL( imageOp );
            iWaiter->StartAndWait(); // Making asynchronous SetImageAsyncL to synchronous
            CleanupStack::Pop(imageOp);
            delete imageOp;
            }
        }    
    else if(err != KErrNotSupported)
        {
        User::Leave(err);
        }
    CleanupStack::PopAndDestroy(imageMgr);        
    }

// Checks if the link is a group or not
TBool CCreatorVirtualPhonebook::IsContactGroupL(const MVPbkContactLink& aLink)
	{
	LOGSTRING("Creator: CCreatorPhonebook::IsContactGroupL");
	if( iStore )
		{
		delete iContactGroupsInStore;
		iContactGroupsInStore = 0;
		// Save contact group links here
		iContactGroupsInStore = iStore->ContactGroupsLC();
		CleanupStack::Pop();
		}
		
	if( iContactGroupsInStore )
		{
		// Loop all the groups and check if any of them matches to the given link.
		for( TInt i = 0; i < iContactGroupsInStore->Count(); ++i )
			{
			if( (*iContactGroupsInStore)[i].IsSame(aLink) )
				return ETrue;
			}
		}
	return EFalse;
	}

TInt CCreatorVirtualPhonebook::CreateGroupEntryL(CCreatorModuleBaseParameters *aParameters)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::CreateGroupEntryL");
    delete iParameters;
    iParameters = 0;
    
    CVirtualPhonebookParameters* parameters = (CVirtualPhonebookParameters*) aParameters;
    
    if( !parameters )
        {
        iParameters = new (ELeave) CVirtualPhonebookParameters;
        iParameters->iContactsInGroup = iContactsInGroup;
        iParameters->iGroupName->Des().Copy( iEngine->RandomString(CCreatorEngine::EGroupName) );
        iParameters->iGroupName->Des().Append( _L(" #") );
        iParameters->iGroupName->Des().AppendNum( iEngine->RandomNumber(1000, 9999) );
        parameters = iParameters;
        }

    TInt err = KErrNone;
 
    // create a new contact group    
    MVPbkContactGroup* newGroup = iStore->CreateNewContactGroupLC();
    
    newGroup->SetGroupLabelL(parameters->iGroupName->Des());
        
    // define amounts of contacts to be added to the group
    TInt amountOfContactsToBeAdded = 0;
    if (parameters->iContactsInGroup == KCreateRandomAmountOfGroups)
    	{
        amountOfContactsToBeAdded = iEngine->RandomNumber(30);
    	}
    else 
    	{
        amountOfContactsToBeAdded = parameters->iContactsInGroup;
    	}
    
    if( parameters->iLinkIds.Count() > 0 )
        {
        for( TInt i = 0; i < parameters->iLinkIds.Count(); ++i )
            {
            const CCreatorContactSet& set = ContactLinkCache::Instance()->ContactSet(parameters->iLinkIds[i].iLinkId);
            
            const RPointerArray<MVPbkContactLink>& links = set.ContactLinks();
            TInt numberOfExplicitLinks = links.Count(); // Number of defined contacts in contact-set
            TInt numberOfExistingContacts = set.NumberOfExistingContacts(); // Number of existing contacts in contact-set
            TInt maxAmount = numberOfExplicitLinks + numberOfExistingContacts;
            
            if( parameters->iLinkIds[i].iLinkAmount > 0 )
                maxAmount = parameters->iLinkIds[i].iLinkAmount; // Max amount is limited
            
            TInt addedMembers = 0;
            
            for( TInt j = 0; j < links.Count() && addedMembers < maxAmount; ++j )
                {
                MVPbkContactLink* link = links[j]; 
                if( link && IsContactGroupL(*link) == EFalse )
                    {
                    TRAPD(err, newGroup->AddContactL(*link));
                    if( err != KErrAlreadyExists )
                    	{
                    	// Ignore "allready exists" -error
                    	User::LeaveIfError(err);
                    	++addedMembers;
                    	}                    
                    }
                }
            if( addedMembers < maxAmount )
            	{
            	// Add existing contacts, withing the limits set by maxAmount:
            	amountOfContactsToBeAdded += maxAmount - addedMembers;
            	}
            }
        }
    if( amountOfContactsToBeAdded > 0 )
        {    
        CVPbkContactViewDefinition* viewDef = CVPbkContactViewDefinition::NewL();
        CleanupStack::PushL( viewDef );
        viewDef->SetType( EVPbkContactsView );
        MVPbkContactView* contactView = iStore->CreateViewLC( 
                *viewDef, 
                *this, 
                iStore->StoreProperties().SupportedFields() );
        iWaiter->StartAndWait();//Making asynchronous CreateViewLC to synchronous

        TInt cnt = 0;
        // assign some random contacts to the group
        for ( TInt y = 0; cnt < amountOfContactsToBeAdded && y < contactView->ContactCountL(); ++y )
            {
            MVPbkContactLink* link = contactView->CreateLinkLC( y );
            TRAPD( err, newGroup->AddContactL( *link ) );
            CleanupStack::PopAndDestroy(); // link
            if( err != KErrAlreadyExists )
                {
                // Ignore "allready exists" -error
                User::LeaveIfError( err );
                ++cnt;
                }
            }
        CleanupStack::PopAndDestroy(); // contactView
        CleanupStack::PopAndDestroy( viewDef );
        }
    
    newGroup->CommitL(*this);
    iWaiter->StartAndWait(); //Making asynchronous CommitL to synchronous

    // store the link to contact, so that Creator is able to delete
    // it when user requests deletion of contacts that were created with Creator 
    MVPbkContactLink* linkToContact = newGroup->CreateLinkLC();
    iContactGroupsToDelete->AppendL( linkToContact );
    CleanupStack::Pop(); // linkToContact
    
    CleanupStack::PopAndDestroy(); // newGroup
   
    CompactPbkDatabaseL();

    return err;

    }

//----------------------------------------------------------------------------

TInt CCreatorVirtualPhonebook::CreateSubscribedContactEntryL(CCreatorModuleBaseParameters* /*aParameters*/)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::CreateSubscribedContactEntryL");

    return KErrNotSupported;
    }

//----------------------------------------------------------------------------

void CCreatorVirtualPhonebook::AddFieldToContactItemL(
		MVPbkStoreContact& newContact, TPbkFieldId aFieldId, const TDesC& aFieldText)
	{
    LOGSTRING("Creator: CCreatorVirtualPhonebook::AddFieldToContactItemL");

    // Get field type object
    const MVPbkFieldTypeList& fieldList = newContact.ParentStore().StoreProperties().SupportedFields();        
    const MVPbkFieldType* fieldType = fieldList.Find(aFieldId);
    if( fieldType == 0 )
        return; // field type is not supported
    
    // Create a new field
	MVPbkStoreContactField* newField = newContact.CreateFieldLC(*fieldType);	
	if( !newField )
	    {
	    User::Leave(KErrNotSupported);
	    }
	
	MVPbkContactFieldData& fieldData = newField->FieldData();
	
	//data type must be text in this case
	TVPbkFieldStorageType dataType = fieldData.DataType();

	if(dataType == EVPbkFieldStorageTypeText)
	    {
		//set text to field	
		MVPbkContactFieldTextData& tmpData = MVPbkContactFieldTextData::Cast(fieldData);
		tmpData.SetTextL(aFieldText);
		//set field to contact		
		newContact.AddFieldL(newField);//This object takes ownership of the field.
		CleanupStack::Pop(newField);			
	    }
	else
		{
		User::Leave(KErrArgument);
		}
	
    }

//----------------------------------------------------------------------------

void CCreatorVirtualPhonebook::AddFieldToContactItemL(
		MVPbkStoreContact& newContact, TPbkFieldId aFieldId, const TDesC8& aFieldText)
	{
    LOGSTRING("Creator: CCreatorVirtualPhonebook::AddFieldToContactItemL");

	//get field types
	const MVPbkFieldTypeList& fieldList = newContact.ParentStore().StoreProperties().SupportedFields();
	
	//get wanted field type
	const MVPbkFieldType* fieldType = fieldList.Find(aFieldId);
	
	//create a new field
	MVPbkStoreContactField* newField =
	            newContact.CreateFieldLC(*fieldType);
	//get field data	
	MVPbkContactFieldData& fieldData = newField->FieldData();
	
	//data type must be text in this case
	TVPbkFieldStorageType dataType = fieldData.DataType();

	if(dataType == EVPbkFieldStorageTypeBinary){
		//set bindata to field	
		MVPbkContactFieldBinaryData& tmpData = MVPbkContactFieldBinaryData::Cast(fieldData);		
		tmpData.SetBinaryDataL(aFieldText);
		//set field to contact		
		newContact.AddFieldL(newField);//This object takes ownership of the field.
		CleanupStack::Pop(newField);			
	}
	else
		{
		CleanupStack::PopAndDestroy(newField);
		}
	
    }

void CCreatorVirtualPhonebook::AddFieldToContactItemL(
        MVPbkStoreContact& newContact, TPbkFieldId aFieldId, const TTime& aFieldText)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::AddFieldToContactItemL");
    //get field types
    const MVPbkFieldTypeList& fieldList = newContact.ParentStore().StoreProperties().SupportedFields();
        
    //get wanted field type
    const MVPbkFieldType* fieldType = fieldList.Find(aFieldId);
        
    //create a new field
    MVPbkStoreContactField* newField = newContact.CreateFieldLC(*fieldType);
    //get field data    
    MVPbkContactFieldData& fieldData = newField->FieldData();
        
    //data type must be datetime in this case
    TVPbkFieldStorageType dataType = fieldData.DataType();

    if(dataType == EVPbkFieldStorageTypeDateTime)
        {
        //set bindata to field  
        MVPbkContactFieldDateTimeData& tmpData = MVPbkContactFieldDateTimeData::Cast(fieldData);        
        tmpData.SetDateTime(aFieldText);
        //set field to contact      
        newContact.AddFieldL(newField);//This object takes ownership of the field.
        CleanupStack::Pop(newField);            
        }
    else
        {
        CleanupStack::PopAndDestroy(newField);
        }
    }

//----------------------------------------------------------------------------

void CCreatorVirtualPhonebook::CompactPbkDatabaseL(TBool aCompressImmediately)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::CompactPbkDatabase");

    iOpCounter++;

    // compress the database immediately or after each 100th operation
    if (aCompressImmediately || iOpCounter%100 == 0)
        {
        MVPbkContactOperationBase* operation= iContactManager->CompressStoresL(*this);
        if (operation)
            {
            CleanupDeletePushL( operation );
            iWaiter->StartAndWait();//Making asynchronous CompressStoresL to synchronous
            CleanupStack::Pop(operation);
            delete operation;
            }             		 
        }
    }

//----------------------------------------------------------------------------
//Observer implementations for MVPbkContactStoreListObserver 
//(and it's super class MVPbkContactStoreObserver)

void CCreatorVirtualPhonebook::OpenComplete()
	{
	LOGSTRING("Creator: CCreatorVirtualPhonebook::OpenComplete()");
    iWaiter->Cancel();
	}

void CCreatorVirtualPhonebook::StoreReady(MVPbkContactStore& /*aContactStore*/)
	{
	LOGSTRING("Creator: CCreatorVirtualPhonebook::StoreReady()");
	}


void CCreatorVirtualPhonebook::StoreUnavailable(MVPbkContactStore& /*aContactStore*/, 
        TInt /*aReason*/)
	{
	LOGSTRING("Creator: CCreatorVirtualPhonebook::StoreUnavailable()");
	}


void CCreatorVirtualPhonebook::HandleStoreEventL(
        MVPbkContactStore& /*aContactStore*/, 
        TVPbkContactStoreEvent /*aStoreEvent*/)
	{
	LOGSTRING("Creator: CCreatorVirtualPhonebook::HandleStoreEventL()");
	}


 TAny* CCreatorVirtualPhonebook::ContactStoreObserverExtension(
		 TUid /*aExtensionUid*/) 
        
 { 
 LOGSTRING("Creator: CCreatorVirtualPhonebook::ContactStoreObserverExtension()");
 return NULL; 
 }
 
 
//----------------------------------------------------------------------------
//Observer implementation for MVPbkBatchOperationObserver

void CCreatorVirtualPhonebook::StepComplete( 
     MVPbkContactOperationBase& /*aOperation*/,
     TInt /*aStepSize*/ )
	{
    LOGSTRING("Creator: CCreatorVirtualPhonebook::StepComplete" );	
	}


TBool CCreatorVirtualPhonebook::StepFailed(
     MVPbkContactOperationBase& /*aOperation*/,
     TInt /*aStepSize*/, TInt /*aError*/ )
{
	LOGSTRING("Creator: CCreatorVirtualPhonebook::StepFailed" );
	iWaiter->Cancel();
	return EFalse;
}


void CCreatorVirtualPhonebook::OperationComplete( 
     MVPbkContactOperationBase& /*aOperation*/ )
	{
    LOGSTRING("Creator: CCreatorVirtualPhonebook::OperationComplete" );
    //Canceling waiter when operation is done.
    iWaiter->Cancel();
	}


TAny* CCreatorVirtualPhonebook::BatchOperationObserverExtension( 
     TUid /*aExtensionUid*/ ) 
     {
     LOGSTRING("Creator: CCreatorVirtualPhonebook::BatchOperationObserverExtension" );
     return NULL;
     }

//----------------------------------------------------------------------------
//Observer implementation for MVPbkContactFindObserver
void CCreatorVirtualPhonebook::FindCompleteL( MVPbkContactLinkArray* aResults )
  	 {
  	 LOGSTRING("Creator: CCreatorVirtualPhonebook::FindCompleteL" );
  	 if(iContactResults)
  		 {
  		 delete iContactResults;
  		 iContactResults = NULL;
  		 }
  	 iContactResults = aResults;
  	 iWaiter->Cancel();
   	 }

void CCreatorVirtualPhonebook::FindFailed( TInt aError )
 	 {
 	LOGSTRING2("Creator: CCreatorVirtualPhonebook::FindFailed - Error: %d", aError );
 	iWaiter->Cancel();
  	 }

TAny* CCreatorVirtualPhonebook::ContactFindObserverExtension( 
	                TUid /*aExtensionUid*/ )
	 {
	 LOGSTRING("Creator: CCreatorVirtualPhonebook::ContactFindObserverExtension" );
	 return NULL;
	 }	                


//Observer implementation for MVPbkContactObserver
//----------------------------------------------------------------------------

void CCreatorVirtualPhonebook::ContactOperationCompleted(TContactOpResult aResult)
	{
    LOGSTRING("Creator: CCreatorVirtualPhonebook::ContactOperationCompleted" );
            
	TContactOp op = aResult.iOpCode; 		 
    switch ( op )
        {
        case EContactCommit:
        case EContactDelete:
            {
            iWaiter->Cancel();
            break;
            }
        case EContactLock:
        default: break;
        }
	}
	
void CCreatorVirtualPhonebook::ContactOperationFailed
			(TContactOp /*aOpCode*/, TInt aErrorCode, TBool /*aErrorNotified*/)
	{
    LOGSTRING2("Creator: CCreatorVirtualPhonebook::ContactOperationFailed - Error: %d", aErrorCode );
    iWaiter->Cancel();
	}
	
TAny* CCreatorVirtualPhonebook::ContactObserverExtension(TUid /*aExtensionUid*/) 
	{
    LOGSTRING("Creator: CCreatorVirtualPhonebook::ContactObserverExtension" );
    return NULL;
	}

void CCreatorVirtualPhonebook::Pbk2ImageSetComplete(MPbk2ImageOperation& /*aOperation*/)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::Pbk2ImageSetComplete" );
    //Canceling waiter when operation is done.
    iWaiter->Cancel();
    }

void CCreatorVirtualPhonebook::Pbk2ImageSetFailed(MPbk2ImageOperation& /*aOperation*/, TInt /*aError*/)
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::Pbk2ImageSetFailed" );
    //Canceling waiter when operation is done.
    iWaiter->Cancel();
    }


//Observer implementation for MVPbkContactViewObserver
//----------------------------------------------------------------------------
void CCreatorVirtualPhonebook::ContactViewReady( MVPbkContactViewBase& /*aView*/ )
    {
    LOGSTRING("Creator: CCreatorVirtualPhonebook::ContactViewReady" );
    iWaiter->Cancel();
    }

void CCreatorVirtualPhonebook::ContactViewUnavailable(
        MVPbkContactViewBase& /*aView*/ )
    {
    iWaiter->Cancel();
    }

void CCreatorVirtualPhonebook::ContactAddedToView(
        MVPbkContactViewBase& /*aView*/, 
        TInt /*aIndex*/, 
        const MVPbkContactLink& /*aContactLink*/ )
    {
    }

void CCreatorVirtualPhonebook::ContactRemovedFromView(
        MVPbkContactViewBase& /*aView*/, 
        TInt /*aIndex*/, 
        const MVPbkContactLink& /*aContactLink*/ )
    {
    }
        
void CCreatorVirtualPhonebook::ContactViewError(
        MVPbkContactViewBase& /*aView*/, 
        TInt /*aError*/, 
        TBool /*aErrorNotified*/ )
    {
    iWaiter->Cancel();
    }

/**
 * CCreatorContactFieldImpl Base class for contact field implementations
 */
class CCreatorContactFieldImpl : public CBase
    {
public:        
    virtual ~CCreatorContactFieldImpl() {};
    
    virtual void AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk);
    virtual void SetRandomParametersL(CCreatorContactField::TRandomLengthType aRandomLenType = CCreatorContactField::ERandomLengthDefault, TInt aRandomLen = 0 );
    virtual void ParseL(CCommandParser* parser);
     
    virtual TInt FieldType();
protected:
    CCreatorContactFieldImpl(TInt aFieldType);    
    TInt iFieldType;    
    CCreatorContactField::TRandomLengthType iRandomLenType;
    TInt iRandomLen;
    };
    
void CCreatorContactFieldImpl::AddToContactL(CCreatorEngine* /*aEngine*/, MVPbkStoreContact& /*aContact*/, CCreatorVirtualPhonebook* /*aVPbk*/ )
    {}

void CCreatorContactFieldImpl::SetRandomParametersL(CCreatorContactField::TRandomLengthType aRandomLenType, TInt aRandomLen )
    {    
    iRandomLenType = aRandomLenType;
    iRandomLen = aRandomLen;
    }

void CCreatorContactFieldImpl::ParseL(CCommandParser* /*parser*/)
    {}
    
CCreatorContactFieldImpl::CCreatorContactFieldImpl(TInt aFieldType)
    {
    iFieldType = aFieldType;
    }

TInt CCreatorContactFieldImpl::FieldType()
    {
    return iFieldType;    
    }


/**
 * CCreatorContactTextField Text field implementation
 */
class CCreatorContactTextField : public CCreatorContactFieldImpl
    {
    public:
    static CCreatorContactTextField* NewL(TInt aFieldType, const TDesC& aData);
    ~CCreatorContactTextField();
    virtual void AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk);    
    virtual void ParseL(CCommandParser* parser);
    
    protected:
    virtual void SetDefaultRandomContentL(CCreatorEngine* aEngine);
    virtual void SetExplicitRandomContentL(CCreatorEngine* aEngine, TInt aRandomLen);
    
    private:    
    CCreatorContactTextField(TInt aFieldType);
    void ConstructL(const TDesC& aData);
    HBufC* iPtr;    
    };
    
CCreatorContactTextField::CCreatorContactTextField(TInt aFieldType) 
: CCreatorContactFieldImpl(aFieldType)
    { }
CCreatorContactTextField::~CCreatorContactTextField()
    { 
    delete iPtr; 
    }
 void CCreatorContactTextField::AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk)
     {
     LOGSTRING("Creator: CCreatorContactTextField::AddToContactL"); 
     if( aVPbk == 0 )
         User::Leave(KErrArgument);
               
     if( iPtr == 0 || iPtr->Length() == 0)
         {
         switch( iRandomLenType )
         {
         case CCreatorContactField::ERandomLengthDefault:
             {
             SetDefaultRandomContentL(aEngine);
             break;
             }
         case CCreatorContactField::ERandomLengthMax:
             {
             const MVPbkFieldTypeList& fieldList = aContact.ParentStore().StoreProperties().SupportedFields();
             const MVPbkFieldType* fieldType = fieldList.Find(iFieldType);
             if( fieldType == 0 )
                 return; // field type is not supported
                 
             // Create a new field
             MVPbkStoreContactField* newField = aContact.CreateFieldLC(*fieldType);    
             if( !newField )
                 {
                 User::Leave(KErrNotSupported);
                 }
                 
             MVPbkContactFieldData& fieldData = newField->FieldData();  
             //data type must be text in this case
             TVPbkFieldStorageType dataType = fieldData.DataType();

             if(dataType == EVPbkFieldStorageTypeText)
                 {
                 //set text to field 
                 MVPbkContactFieldTextData& tmpData = MVPbkContactFieldTextData::Cast(fieldData);
                 iRandomLen = tmpData.MaxLength();
                 if( iRandomLen == KVPbkUnlimitedFieldLength )
                     iRandomLen = 128;
                 }
             CleanupStack::PopAndDestroy(newField);       
             SetExplicitRandomContentL(aEngine, iRandomLen);
             break;
             }
         case CCreatorContactField::ERandomLengthExplicit:
             {
             SetExplicitRandomContentL(aEngine, iRandomLen);
             break;
             }
         default:
             break;
         }
         }
     if( iPtr )
         {
         if( iFieldType == R_VPBK_FIELD_TYPE_THUMBNAILPIC )
             {
             if( iPtr && iPtr->Length() > 0 )
            	 {
            	 aVPbk->AddImageToContactL(aContact, iFieldType, iPtr->Des());
            	 }
             }
         //Thumbnail id handling
         /*else if( iFieldType == FIELD_TYPE_THUMBNAILPIC_ID )
             {
             if( iPtr && iPtr->Length() > 0 )
                 {
                 TInt id = aEngine->GetAttachmentIdL(iPtr->Des());
                 if( id != KUndef )
                     {
                     //get picture path by id
                     TFileName sourceFileName = aEngine->TestDataPathL((CCreatorEngine::TTestDataPath)id);// (CCreatorEngine::TTestDataPath) parameters.iAttachments->At(i)
                     //set image to contact
                     aVPbk->AddImageToContactL(aContact, R_VPBK_FIELD_TYPE_THUMBNAILPIC, sourceFileName);                     
                     }
                 }
             }*/         
         else
             {
             if( iPtr && iPtr->Length() > 0 )
            	 {
            	 aVPbk->AddFieldToContactItemL(aContact, iFieldType, iPtr->Des());
            	 }
             }
         }
     }

 void CCreatorContactTextField::SetDefaultRandomContentL(CCreatorEngine* aEngine)
     {
     LOGSTRING("Creator: CCreatorContactTextField::SetDefaultRandomContentL");
     if( aEngine == 0 )
         return;
     
     if( iPtr )
         {
         delete iPtr;
         iPtr = 0;
         }
     TPtrC contentData;
     
     TInt textFieldCount = sizeof(CreatorVPbkMiscTextFields) / sizeof(FieldInfo);
     TBool found = EFalse;
     for( TInt tfIndex = 0; tfIndex < textFieldCount; ++tfIndex )
         {
         if( iFieldType == CreatorVPbkMiscTextFields[tfIndex].iFieldCode )
             {
             found = ETrue;
             if( CreatorVPbkMiscTextFields[tfIndex].iRandomType != RND_TYPE_UNDEF )
                 {
                 contentData.Set(aEngine->RandomString((CCreatorEngine::TRandomStringType) CreatorVPbkMiscTextFields[tfIndex].iRandomType));
                 }
             else
                 {
                 switch( iFieldType )
                 {
                 case R_VPBK_FIELD_TYPE_RINGTONE:
                     {
                     contentData.Set(aEngine->CreateRandomSoundFileL());
                     break;                 
                     }
                 //Thumbnail (path) and thumbnailid handling with random is the same, only in script they have different handling
                 case R_VPBK_FIELD_TYPE_THUMBNAILPIC:                 
                     {
                     contentData.Set(aEngine->RandomPictureFileL());
                     break;
                     }
                 case R_VPBK_FIELD_TYPE_SYNCCLASS:
                 case R_VPBK_FIELD_TYPE_LOCPRIVACY:
                     {
                     _LIT(KPublic, "Public");
                     contentData.Set(KPublic);
                     break;
                     }
                 default:
                     break;
                 }
                 }
             break;
             }
         }
     
     if( found == EFalse )
         {
         TInt phNumFieldCount = sizeof(CreatorVPbkPhoneNumberFields) / sizeof(TInt);
         for( TInt phIndex = 0; phIndex < phNumFieldCount; ++phIndex )
             {
             if( iFieldType == CreatorVPbkPhoneNumberFields[phIndex] )
                 {
                 contentData.Set(aEngine->RandomString(CCreatorEngine::EPhoneNumber));
                 found = ETrue;
                 break;
                 }
             }
         }
     
     if( found == EFalse )
         {
         TInt emailFieldCount = sizeof(CreatorVPbkEmailFields) / sizeof(TInt);
         for( TInt eIndex = 0; eIndex < emailFieldCount; ++eIndex )
             {
             if( iFieldType == CreatorVPbkEmailFields[eIndex] )
                 {
                 iPtr = aEngine->CreateEmailAddressLC();
                 CleanupStack::Pop();
                 return;
                 }
             }
         }
     iPtr = HBufC::NewL(contentData.Length());
     iPtr->Des().Copy(contentData);
     }
 
 void CCreatorContactTextField::SetExplicitRandomContentL(CCreatorEngine* aEngine, TInt aRandomLen)
     {     
     LOGSTRING("Creator: CCreatorContactTextField::SetExplicitRandomContentL");
     if( iPtr )
         {
         delete iPtr;
         iPtr = 0;
         }
     iPtr = aEngine->CreateRandomStringLC(aRandomLen);
     CleanupStack::Pop(); // iPtr
     }
 
 void CCreatorContactTextField::ParseL(CCommandParser* parser)
     {
     parser->StrParserL(iPtr, '|');
     }
 
 void CCreatorContactTextField::ConstructL(const TDesC& aData)
      {     
      iPtr = HBufC::New(KPhonebookFieldLength);
      iPtr->Des().Copy(aData);
      }
 
 CCreatorContactTextField* CCreatorContactTextField::NewL(TInt aFieldType, const TDesC& aData)
     {
     CCreatorContactTextField* self = new (ELeave) CCreatorContactTextField(aFieldType);
     CleanupStack::PushL(self);
     self->ConstructL(aData);
     CleanupStack::Pop();
     return self;
     }
 /**
  * CCreatorContactBinaryField Binary field implementation
  */
 class CCreatorContactBinaryField : public CCreatorContactFieldImpl
    {
public:
    static CCreatorContactBinaryField* NewL(TInt aFieldType, const TDesC8& aData);
    ~CCreatorContactBinaryField();  
    virtual void AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk);
    virtual void ParseL(CCommandParser* parser);
    virtual void SetDefaultRandomContentL(CCreatorEngine* aEngine);
private:
    CCreatorContactBinaryField(TInt aFieldType);
    void ConstructL(const TDesC8& aData);
    HBufC8* iPtr;    
    };
 
 CCreatorContactBinaryField::CCreatorContactBinaryField(TInt aFieldType) 
 : CCreatorContactFieldImpl(aFieldType)
     { }
 CCreatorContactBinaryField::~CCreatorContactBinaryField()
     { 
     delete iPtr; 
     }
   
 void CCreatorContactBinaryField::ConstructL(const TDesC8& aData)
     {
     iPtr = HBufC8::New(KPhonebookFieldLength);
     iPtr->Des() = aData;
     }
 
 CCreatorContactBinaryField* CCreatorContactBinaryField::NewL(TInt aFieldType, const TDesC8& aData)
      {
      CCreatorContactBinaryField* self = new (ELeave) CCreatorContactBinaryField(aFieldType);
      CleanupStack::PushL(self);
      self->ConstructL(aData);
      CleanupStack::Pop();
      return self;
      }
 
void CCreatorContactBinaryField::AddToContactL(CCreatorEngine* /*aEngine*/, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk)
    {
    LOGSTRING("Creator: CCreatorContactBinaryField::AddToContactL");
    if( aVPbk == 0 )
        User::Leave(KErrArgument);
    if( iPtr )
        {
            aVPbk->AddFieldToContactItemL(aContact, iFieldType, iPtr->Des());
        }
    }
void CCreatorContactBinaryField::ParseL(CCommandParser* parser)
     {
     parser->StrParserL(iPtr, '&');
     }

void CCreatorContactBinaryField::SetDefaultRandomContentL(CCreatorEngine* /*aEngine*/)
    {
    LOGSTRING("Creator: CCreatorContactBinaryField::SetDefaultRandomContentL");
    }

/**
 * CCreatorContactDateTimeField
 */ 
class CCreatorContactDateTimeField : public CCreatorContactFieldImpl
    {
public:
    static CCreatorContactDateTimeField* NewL(TInt aFieldType, const TTime& aData);
    ~CCreatorContactDateTimeField();  
    virtual void AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk);
    virtual void ParseL(CCommandParser* parser);    
private:
CCreatorContactDateTimeField(TInt aFieldType);
    void ConstructL(const TTime& aData);
    TTime iPtr;    
    };
 
CCreatorContactDateTimeField::CCreatorContactDateTimeField(TInt aFieldType) 
: CCreatorContactFieldImpl(aFieldType)
     { }
CCreatorContactDateTimeField::~CCreatorContactDateTimeField()
     {
     }
   
 void CCreatorContactDateTimeField::ConstructL(const TTime& aData)
     {
     iPtr = aData;
     }
 
 CCreatorContactDateTimeField* CCreatorContactDateTimeField::NewL(TInt aFieldType, const TTime& aData)
      {
      CCreatorContactDateTimeField* self = new (ELeave) CCreatorContactDateTimeField(aFieldType);
      CleanupStack::PushL(self);
      self->ConstructL(aData);
      CleanupStack::Pop();
      return self;
      }
 
void CCreatorContactDateTimeField::AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk)
    {
    LOGSTRING("Creator: CCreatorContactDateTimeField::AddToContactL");
    if( aVPbk == 0 )
        User::Leave(KErrArgument);
    
    //TTime undefTime();
    if( iPtr == Time::NullTTime() )
        {
        iPtr = aEngine->RandomDate(CCreatorEngine::EDateFuture);
        }
    
    aVPbk->AddFieldToContactItemL(aContact, iFieldType, iPtr);    
    }
void CCreatorContactDateTimeField::ParseL(CCommandParser* /*parser*/)
     {
     //parser->StrParserL(iPtr, '&');
     }

/**
 * CCreatorContactField Contact field API
 */
CCreatorContactField* CCreatorContactField::NewL(TInt aFieldType, const TDesC& aData)
    {
    CCreatorContactField* self = new (ELeave) CCreatorContactField();
    CleanupStack::PushL(self);
    self->ConstructL(aFieldType, aData);
    CleanupStack::Pop();
    return self;
    }

CCreatorContactField* CCreatorContactField::NewL(TInt aFieldType, const TDesC8& aData)
    {
    CCreatorContactField* self = new (ELeave) CCreatorContactField();
    CleanupStack::PushL(self);
    self->ConstructL(aFieldType, aData);
    CleanupStack::Pop();
    return self;
    }

CCreatorContactField* CCreatorContactField::NewL(TInt aFieldType, const TTime& aData)
    {
    CCreatorContactField* self = new (ELeave) CCreatorContactField();
    CleanupStack::PushL(self);
    self->ConstructL(aFieldType, aData);
    CleanupStack::Pop();
    return self;
    }

void CCreatorContactField::ConstructL(TInt aFieldType, const TDesC& aData)
    {
    pImpl = CCreatorContactTextField::NewL(aFieldType, aData); 
    }

void CCreatorContactField::ConstructL(TInt aFieldType, const TDesC8& aData)
    {
    pImpl = CCreatorContactBinaryField::NewL(aFieldType, aData);    
    }

void CCreatorContactField::ConstructL(TInt aFieldType, const TTime& aData)
    {
    pImpl = CCreatorContactDateTimeField::NewL(aFieldType, aData);    
    }

CCreatorContactField::CCreatorContactField()
    {}
CCreatorContactField::~CCreatorContactField()
    {
    delete pImpl;
    }
 
TInt CCreatorContactField::FieldType()
    {
    return pImpl->FieldType();
    }
void CCreatorContactField::AddToContactL(CCreatorEngine* aEngine, MVPbkStoreContact& aContact, CCreatorVirtualPhonebook* aVPbk)
    {
    pImpl->AddToContactL(aEngine, aContact, aVPbk);
    }
void CCreatorContactField::ParseL(CCommandParser* parser)
    {
    pImpl->ParseL(parser);
    }

void CCreatorContactField::SetRandomParametersL(TRandomLengthType aRandomLenType, TInt aRandomLen )
    {
    pImpl->SetRandomParametersL(aRandomLenType, aRandomLen);
    }
