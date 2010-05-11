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



#include "creator_phonebook.h" 
#include "creator_traces.h"

#include <CPbkContactIter.h>

//----------------------------------------------------------------------------

CPhonebookParameters::CPhonebookParameters()
    {
    LOGSTRING("Creator: CPhonebookParameters::CPhonebookParameters");

    iContactFirstName = HBufC::New(KPhonebookFieldLength);
    iContactLastName = HBufC::New(KPhonebookFieldLength);
    iContactCompanyName = HBufC::New(KPhonebookFieldLength);
    iContactJobTitle = HBufC::New(KPhonebookFieldLength);
    iContactPostalAddress = HBufC::New(KPhonebookFieldLength);
    iWvUserId = HBufC::New(KPhonebookFieldLength);
    iRingingtone = HBufC::New(KPhonebookFieldLength);
    iThumbnailImage = HBufC::New(KPhonebookFieldLength);
    iContactPhoneNumberGeneral = HBufC::New(KPhonebookFieldLength);
    iContactURL = HBufC::New(KPhonebookFieldLength);
    iContactEmailAddress = HBufC::New(KPhonebookFieldLength);
    iGroupName = HBufC::New(KPhonebookFieldLength);
    }

CPhonebookParameters::~CPhonebookParameters()
    {
    LOGSTRING("Creator: CPhonebookParameters::~CPhonebookParameters");

    delete iGroupName;
    delete iContactEmailAddress;
    delete iContactURL;
    delete iContactPhoneNumberGeneral;
    delete iThumbnailImage;
    delete iRingingtone;
    delete iWvUserId;
    delete iContactPostalAddress;
    delete iContactJobTitle;
    delete iContactCompanyName;
    delete iContactLastName;
    delete iContactFirstName;
    }
void CPhonebookParameters::ParseL (CCommandParser* parser, TParseParams /*aCase = 0*/)
	{
	parser->StrParserL (iContactFirstName, '|');
	parser->StrParserL (iContactLastName, '|');
	parser->StrParserL (iContactCompanyName, '|');
	parser->StrParserL (iContactJobTitle, '|');
	parser->StrParserL (iContactPostalAddress, '|');
	parser->StrParserL (iContactPhoneNumberGeneral, '|');
	parser->StrParserL (iContactURL, '|');
	parser->StrParserL (iContactEmailAddress, '|');
	parser->StrParserL (iWvUserId, '|');
	parser->StrParserL (iRingingtone, '|');
	parser->StrParserL (iThumbnailImage, ';');
	}
//----------------------------------------------------------------------------

CCreatorPhonebook* CCreatorPhonebook::NewL(CCreatorEngine* aEngine)
    {
    CCreatorPhonebook* self = CCreatorPhonebook::NewLC(aEngine);
    CleanupStack::Pop(self);
    return self;
    }

CCreatorPhonebook* CCreatorPhonebook::NewLC(CCreatorEngine* aEngine)
    {
    CCreatorPhonebook* self = new (ELeave) CCreatorPhonebook;
    CleanupStack::PushL(self);
    self->ConstructL(aEngine);
    return self;
    }

CCreatorPhonebook::CCreatorPhonebook()
    {
    }

void CCreatorPhonebook::ConstructL(CCreatorEngine* aEngine)
    {
    LOGSTRING("Creator: CCreatorPhonebook::ConstructL");

    iEngine = aEngine;  
    iContactDBEngine = CPbkContactEngine::NewL();
    iOpCounter = 0;
    SetDefaultParameters();
    }

CCreatorPhonebook::~CCreatorPhonebook()
    {
    LOGSTRING("Creator: CCreatorPhonebook::~CCreatorPhonebook");

    if ( iEngine && iContactIds.Count() )
        {
        TRAP_IGNORE( iEngine->WriteEntryIdsToStoreL( iContactIds, KUidDictionaryUidContacts ) );
        }
    iContactIds.Reset();
    iContactIds.Close();

    if ( iEngine && iGroupIds.Count() )
        {
        TRAP_IGNORE( iEngine->WriteEntryIdsToStoreL( iGroupIds, KUidDictionaryUidContactGroups ) );
        }
    iGroupIds.Reset();
    iGroupIds.Close();
       
    iEntriesToDelete.Reset();
    iEntriesToDelete.Close();
    
    CompactPbkDatabase( ETrue );

    delete iContactDBEngine;

    if (iParameters)
        delete iParameters;
    }

//----------------------------------------------------------------------------
TInt CCreatorPhonebook::CreateContactEntryL(CCreatorModuleBaseParameters *aParameters)
    {
    LOGSTRING("Creator: CCreatorPhonebook::CreateContactEntryL");
    
    delete iParameters;
    iParameters = 0;
    
    CPhonebookParameters* parameters = (CPhonebookParameters*) aParameters;
    
    // random data needed if no predefined data available
    if (!parameters)
        {
        iParameters = new(ELeave) CPhonebookParameters;
        
        iParameters->iNumberOfPhoneNumberFields = iNumberOfPhoneNumberFields;
        iParameters->iNumberOfURLFields = iNumberOfURLFields;
        iParameters->iNumberOfEmailAddressFields = iNumberOfEmailAddressFields;        
        
        iParameters->iNumberOfPhoneNumberFields = iNumberOfPhoneNumberFields;
        iParameters->iNumberOfURLFields = iNumberOfURLFields;
        iParameters->iNumberOfEmailAddressFields = iNumberOfEmailAddressFields;
        
        iParameters->iContactFirstName->Des() = iEngine->RandomString(CCreatorEngine::EFirstName);
        iParameters->iContactLastName->Des() = iEngine->RandomString(CCreatorEngine::ESurname);
        iParameters->iContactCompanyName->Des() = iEngine->RandomString(CCreatorEngine::ECompany);
        iParameters->iContactJobTitle->Des() = iEngine->RandomString(CCreatorEngine::EJobTitle);
        iParameters->iContactPostalAddress->Des() = iEngine->RandomString(CCreatorEngine::EAddress);
        
        iParameters->iWvUserId->Des().Copy(_L("wv:"));
        iParameters->iWvUserId->Des().Append(iParameters->iContactFirstName->Des());

        // TO-DO.....
        //iParameters->iRingingtone->Des() = KNullDesC;
        //iParameters->iThumbnailImage->Des() = KNullDesC;

        iParameters->iContactPhoneNumberGeneral->Des() = iEngine->RandomString(CCreatorEngine::EPhoneNumber);
  
        iParameters->iContactURL->Des().Copy( _L("http://www.") );
        iParameters->iContactURL->Des().Append( iParameters->iContactCompanyName->Des() );
        iParameters->iContactURL->Des().Append( _L(".com/") );

        iParameters->iContactEmailAddress->Des().Copy( iParameters->iContactFirstName->Des() );
        iParameters->iContactEmailAddress->Des().Append( _L("@") );
        iParameters->iContactEmailAddress->Des().Append( iParameters->iContactCompanyName->Des() );
        iParameters->iContactEmailAddress->Des().Append( _L(".com") );

        }
    else
        {
        iParameters = parameters;
        }

    TInt err = KErrNone;

    // create a new contact item based on the default template
	CPbkContactItem* newContact = iContactDBEngine->CreateEmptyContactL();
	CleanupStack::PushL(newContact);

    AddFieldToContactItemL (*newContact, EPbkFieldIdFirstName, iParameters->iContactFirstName->Des());
    AddFieldToContactItemL (*newContact, EPbkFieldIdLastName, iParameters->iContactLastName->Des());
    AddFieldToContactItemL (*newContact, EPbkFieldIdCompanyName, iParameters->iContactCompanyName->Des());
    AddFieldToContactItemL (*newContact, EPbkFieldIdJobTitle, iParameters->iContactJobTitle->Des());
    AddFieldToContactItemL (*newContact, EPbkFieldIdPostalAddress, iParameters->iContactPostalAddress->Des());
    
    AddFieldToContactItemL (*newContact, EPbkFieldIdWVID, iParameters->iWvUserId->Des());
    AddFieldToContactItemL (*newContact, EPbkFieldIdPersonalRingingToneIndication, iParameters->iRingingtone->Des());
    AddFieldToContactItemL (*newContact, EPbkFieldIdThumbnailImage, iParameters->iThumbnailImage->Des());

    for (TInt i=0; i<iParameters->iNumberOfPhoneNumberFields; i++)
        {
        if (i%6 == 0)
            AddFieldToContactItemL (*newContact, EPbkFieldIdPhoneNumberGeneral, iParameters->iContactPhoneNumberGeneral->Des());
        else if (i%6 == 1)
            AddFieldToContactItemL (*newContact, EPbkFieldIdPhoneNumberMobile, iParameters->iContactPhoneNumberGeneral->Des());
        else if (i%6 == 2)
            AddFieldToContactItemL (*newContact, EPbkFieldIdFaxNumber, iParameters->iContactPhoneNumberGeneral->Des());
        else if (i%6 == 3)
            AddFieldToContactItemL (*newContact, EPbkFieldIdPagerNumber, iParameters->iContactPhoneNumberGeneral->Des());
        else if (i%6 == 4)
            AddFieldToContactItemL (*newContact, EPbkFieldIdPhoneNumberHome, iParameters->iContactPhoneNumberGeneral->Des());
        else 
            AddFieldToContactItemL (*newContact, EPbkFieldIdPhoneNumberWork, iParameters->iContactPhoneNumberGeneral->Des());
        }

    for (TInt j=0; j<iParameters->iNumberOfURLFields; j++)
        AddFieldToContactItemL (*newContact, EPbkFieldIdURL, iParameters->iContactURL->Des());

    for (TInt k=0; k<iParameters->iNumberOfEmailAddressFields; k++)
        AddFieldToContactItemL (*newContact, EPbkFieldIdEmailAddress, iParameters->iContactEmailAddress->Des());


    // add the contact item to the database
	iContactDBEngine->AddNewContactL(*newContact);

    // id has been generated, store it for being able to delete
    // entries created only with Creator
	iContactIds.Append( newContact->Id() );

	CleanupStack::PopAndDestroy(); // newContact

    CompactPbkDatabase();

    return err;
    }

//----------------------------------------------------------------------------

TInt CCreatorPhonebook::CreateGroupEntryL(CCreatorModuleBaseParameters *aParameters) 
    { 
    LOGSTRING("Creator: CCreatorPhonebook::CreateGroupEntryL"); 

    // clear any existing parameter definations 
    if (iParameters) 
        { 
        delete iParameters; 
        iParameters = NULL; 
        } 
        
    // random data needed if no predefined data available 
    if (!aParameters) 
        { 
        iParameters = new(ELeave) CPhonebookParameters; 
        
        iParameters->iContactsInGroup = iContactsInGroup; 

        iParameters->iGroupName->Des().Copy( iEngine->RandomString(CCreatorEngine::EGroupName) ); 
        iParameters->iGroupName->Des().Append( _L(" #") ); 
        iParameters->iGroupName->Des().AppendNum( iEngine->RandomNumber(1000, 9999) ); 
        } 
    else 
        { 
        // otherwise replace the parameters object 
        iParameters = (CPhonebookParameters*) aParameters; 
        } 

    TInt err(KErrNone); 

    // create contact iterator 
    CPbkContactIter* contactIter = iContactDBEngine->CreateContactIteratorLC(); 
    
    // create contact id array 
    CContactIdArray* contactIdArray = CContactIdArray::NewLC();  

    // get all contacts    
    TBool getFirstContact(ETrue); 
    TContactItemId contactId; 
    
    for (;;) 
        { 
        if (getFirstContact) 
            { 
            // get first contact 
            contactId = contactIter->FirstL(); 
            getFirstContact = EFalse; 
            } 
        else 
            { 
            // get next contact 
            contactId = contactIter->NextL(); 
            } 
        
        // check if valid contact id received 
        if (contactId == KNullContactId) 
            { 
            // no more contacts, so break 
            break; 
            } 
        else 
            { 
            // append to array 
            contactIdArray->AddL(contactId); 
            }    
        } 

    // create a new contact group 
    CContactGroup* newGroup = iContactDBEngine->CreateContactGroupL( iParameters->iGroupName->Des() ); 
    CleanupStack::PushL(newGroup); 

    // define amounts of contacts to be added to the group 
    TInt amountOfContactsToBeAdded; 
    if (iParameters->iContactsInGroup == KCreateRandomAmountOfGroups) 
        amountOfContactsToBeAdded = iEngine->RandomNumber(30); 
    else     
        amountOfContactsToBeAdded = iParameters->iContactsInGroup; 
    
    
    // a simple way to randomise order of contacts in the array 
    if (iEngine->RandomNumber(1, 2) == 1) 
        contactIdArray->ReverseOrder(); 
    
    // assign contacts from the array to the group if any contacts available        
    if (contactIdArray->Count() > 0) 
        { 
        // get random start number which defines from which position to start adding contacts 
        TInt arrayPosCounter = iEngine->RandomNumber(contactIdArray->Count()-1); 
        TInt addedContacsCounter(0); 
    
        for (TInt y=0; y<amountOfContactsToBeAdded; y++) 
            { 
            // add contact to group 
            iContactDBEngine->AddContactToGroupL((*contactIdArray)[arrayPosCounter], newGroup->Id()); 
            addedContacsCounter++; 
            arrayPosCounter++; 
            
            // no need to add more contacts than actually available 
            if (addedContacsCounter >= contactIdArray->Count()) 
                break; 

            // if limit reached, start from the beginning 
            if (arrayPosCounter >= contactIdArray->Count()) 
                arrayPosCounter = 0; 
            } 
        }
    
    // id has been generated, store it for being able to delete
    // entries created only with Creator
    iGroupIds.Append( newGroup->Id() );
    
    CleanupStack::PopAndDestroy(3); // newGroup, contactIdArray, contactIter 

    CompactPbkDatabase(); 

    return err; 
    } 


//----------------------------------------------------------------------------

TInt CCreatorPhonebook::CreateSubscribedContactEntryL(CCreatorModuleBaseParameters* /*aParameters*/)
    {
    LOGSTRING("Creator: CCreatorPhonebook::CreateSubscribedContactEntryL");

    return KErrNotSupported;
    }

//----------------------------------------------------------------------------

void CCreatorPhonebook::AddFieldToContactItemL(CPbkContactItem& aItem, TPbkFieldId aFieldId, const TDesC& aFieldText)
	{
    //LOGSTRING("Creator: CCreatorPhonebook::AddFieldToContactItemL");

    CPbkFieldInfo* fieldInfo = iContactDBEngine->FieldsInfo().Find( aFieldId );
    if( fieldInfo ) // field was found
        {
        // add the field and value to contact
        TPbkContactItemField* itemField = aItem.AddOrReturnUnusedFieldL( *fieldInfo );

        if( itemField && aFieldText.Length() > 0 ) // add text also
            {
            if( itemField->ItemField().StorageType() == KStorageTypeText )
                {
		        CContactTextField* textField = itemField->ItemField().TextStorage();
                
                // clip the text if it doesn't fit to the field
	            textField->SetTextL( aFieldText.Left( fieldInfo->MaxLength() ) );
	    	    }
    		}
        }
    }

//----------------------------------------------------------------------------

void CCreatorPhonebook::CompactPbkDatabase(TBool aCompressImmediately)
    {
    LOGSTRING("Creator: CCreatorPhonebook::CompactPbkDatabase");

    iOpCounter++;

    // compress the database immediately or after each 100th operation
    if (aCompressImmediately || iOpCounter%100 == 0)
        {
        CContactDatabase& db = iContactDBEngine->Database();

        if (db.CompressRequired())
            {
            LOGSTRING("Creator: CCreatorPhonebook::CompactPbkDatabase - Compacting...");
            TRAP_IGNORE( db.CompactL() );
            }
        }
    }
//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllL()
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteAllL");
    DeleteAllItemsL( KUidDictionaryUidContacts );
    iContactIds.Reset();
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteAllCreatedByCreatorL");
    DeleteItemsCreatedWithCreatorL( KUidDictionaryUidContacts );
    iContactIds.Reset();
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllGroupsL()
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteAllGroupsL");
    DeleteAllItemsL( KUidDictionaryUidContactGroups );
    iGroupIds.Reset();
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllGroupsCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteAllGroupsCreatedByCreatorL");
    DeleteItemsCreatedWithCreatorL( KUidDictionaryUidContactGroups );
    iGroupIds.Reset();
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllItemsL( TUid aStoreUid )
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteItemsL");
    __ASSERT_ALWAYS( aStoreUid == KUidDictionaryUidContacts || 
                     aStoreUid == KUidDictionaryUidContactGroups,
                     User::Panic( _L("CCreatorPb"), KErrArgument ) );
    
    CContactIdArray* allContacts = iContactDBEngine->FindLC( _L("") );
    
    CPbkContactItem* item;
    TInt i(0);
    // remove that type of item ids from the array
    // that we don't want to delete
    while( allContacts && i < allContacts->Count() )
        {
        item = iContactDBEngine->ReadContactL( (*allContacts)[i] );
        if ( ( aStoreUid == KUidDictionaryUidContactGroups ) == 
             ( item->Type() != KUidContactGroup ) )
            {
            allContacts->Remove( i );
            }
        else
            {
            i++;
            }
        delete item;
        item = NULL;
        }
    iContactDBEngine->DeleteContactsL( *allContacts );
    CleanupStack::PopAndDestroy( allContacts );

    // contacts deleted, remove the store
    CDictionaryFileStore* store = iEngine->FileStoreLC();
    if ( store )
        {
        store->Remove( aStoreUid );
        store->CommitL();        
        }
    CleanupStack::PopAndDestroy( store );
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteItemsCreatedWithCreatorL( TUid aStoreUid )
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteItemsCreatedWithCreatorL");
    __ASSERT_ALWAYS( aStoreUid == KUidDictionaryUidContacts || 
                     aStoreUid == KUidDictionaryUidContactGroups,
                     User::Panic( _L("CCreatorPb"), KErrArgument ) );
    
    iEntriesToDelete.Reset();
    
    // fetch ids of entries created by Creator
    iEngine->ReadEntryIdsFromStoreL( iEntriesToDelete, aStoreUid );
    
    // delete entries
    for ( TInt i = 0; i < iEntriesToDelete.Count(); i++ )
        {
        TRAP_IGNORE( iContactDBEngine->DeleteContactL( iEntriesToDelete[i] ) );
        }
    
    iEntriesToDelete.Reset();
    
    // all entries deleted, remove the Logs related registry
    iEngine->RemoveStoreL( aStoreUid );
    }
