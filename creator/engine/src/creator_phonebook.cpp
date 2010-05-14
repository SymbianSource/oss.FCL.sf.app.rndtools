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
#include "creator_contactsetcache.h"

#include "creator_phonebook.h" 
#include "creator_traces.h"

#include <bautils.h> 
#include <qpixmap>

//#include <QDebug>
_LIT(KTempPath, "C:\\Data\\Creator\\");


typedef struct {
QString iDetail;
QString iFieldContext;
QString iFieldString;
TInt iRandomType;
} DetailFieldInfo;
static const TInt RND_TYPE_UNDEF = -99;
DetailFieldInfo CreatorPbkMiscTextFields[] = {        
        { (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactName::FieldFirstName).operator QString(), (TInt) CCreatorEngine::EFirstName},
        { (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactName::FieldLastName).operator QString(), (TInt) CCreatorEngine::ESurname},
        { (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactOrganization::FieldName).operator QString(), (TInt) CCreatorEngine::ECompany},
        { (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactOrganization::FieldTitle).operator QString(), (TInt) CCreatorEngine::EJobTitle},
        { (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactName::FieldPrefix).operator QString(), (TInt) CCreatorEngine::EPrefix},
        { (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactName::FieldSuffix).operator QString(), (TInt) CCreatorEngine::ESuffix},
        { (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactName::FieldMiddleName).operator QString(), (TInt) CCreatorEngine::EFirstName},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactAddress::FieldStreet).operator QString(), (TInt) CCreatorEngine::EAddress},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactAddress::FieldLocality).operator QString(), (TInt) CCreatorEngine::ECity},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactAddress::FieldRegion).operator QString(), (TInt) CCreatorEngine::EState},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactAddress::FieldPostcode).operator QString(), (TInt) CCreatorEngine::EPostcode},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactAddress::FieldCountry).operator QString(), (TInt) CCreatorEngine::ECountry},
        { (QContactAddress::DefinitionName).operator QString(), "", (QContactAddress::FieldStreet).operator QString(), (TInt) CCreatorEngine::EAddress},
        { (QContactAddress::DefinitionName).operator QString(), "", (QContactAddress::FieldLocality).operator QString(), (TInt) CCreatorEngine::ECity},
        { (QContactAddress::DefinitionName).operator QString(), "", (QContactAddress::FieldRegion).operator QString(), (TInt) CCreatorEngine::EState},
        { (QContactAddress::DefinitionName).operator QString(), "", (QContactAddress::FieldPostcode).operator QString(), (TInt) CCreatorEngine::EPostcode},
        { (QContactAddress::DefinitionName).operator QString(), "", (QContactAddress::FieldCountry).operator QString(), (TInt) CCreatorEngine::ECountry},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactAddress::FieldStreet).operator QString(), (TInt) CCreatorEngine::EAddress},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactAddress::FieldLocality).operator QString(), (TInt) CCreatorEngine::ECity},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactAddress::FieldRegion).operator QString(), (TInt) CCreatorEngine::EState},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactAddress::FieldPostcode).operator QString(), (TInt) CCreatorEngine::EPostcode},
        { (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactAddress::FieldCountry).operator QString(), (TInt) CCreatorEngine::ECountry},
        { (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactPhoneNumber::SubTypeMobile).operator QString(), (TInt) CCreatorEngine::EPhoneNumber},
        { (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactPhoneNumber::SubTypeLandline).operator QString(), (TInt) CCreatorEngine::EPhoneNumber},
        { (QContactNote::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactNote::FieldNote).operator QString(), (TInt)  CCreatorEngine::EMemoText},
        { (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactOrganization::FieldDepartment).operator QString(), (TInt) CCreatorEngine::ECompany},
        { (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(), (QContactOrganization::FieldAssistantName).operator QString(), (TInt) CCreatorEngine::ECompany},
        { (QContactFamily::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactFamily::FieldSpouse).operator QString(), (TInt) CCreatorEngine::EFirstName},
        { (QContactFamily::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactFamily::FieldChildren).operator QString(), (TInt) CCreatorEngine::EFirstName},
        { (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),  (QContactName::FieldCustomLabel).operator QString(), (TInt) CCreatorEngine::EFirstName}
        };
                  

typedef struct{
QString iFieldContext;
QString iFieldString;
}PhoneNumInfo;
PhoneNumInfo CreatorPhoneNumberFields[] =
    {
    { "", "Landline"},
    { "Home", "Landline"},
    { "Work", "Landline"},                        
    { "","Mobile"},
    { "Home","Mobile"},
    { "Work", "Mobile"},
    { "Home", "Facsimile"},
    { "Work", "Facsimile"},
    { "Work", "Pager"},
    { "Home", "Video"},
    { "Work", "Video"},   //{ "Home", "Voice" },//{ "Work", "Voice" },
    { "Work", "Assistant" },
    { "Home",  "Car" }
    };


QString CreatorPbkEmailFields[] =
    {
    //R_VPBK_FIELD_TYPE_EMAILGEN,
    (QContactDetail::ContextHome).operator QString(),//"Home",//R_VPBK_FIELD_TYPE_EMAILHOME,
    (QContactDetail::ContextWork).operator QString()//R_VPBK_FIELD_TYPE_EMAILWORK
    };


CPhonebookParameters::CPhonebookParameters()
    {
    LOGSTRING("Creator: CVirtualPhonebookParameters::CVirtualPhonebookParameters");
    //iGroupName = HBufC::New(KPhonebookFieldLength);  
    }

CPhonebookParameters::~CPhonebookParameters()
    {
    LOGSTRING("Creator: CVirtualPhonebookParameters::~CVirtualPhonebookParameters");

    //delete iGroupName;   
    //iContactFields.ResetAndDestroy();
    //iContactFields.Close();
    
    //iLinkIds.clear();
    iLinkIds.Reset();
    iLinkIds.Close();
    }

void CPhonebookParameters::ParseL(CCommandParser* /*parser*/, TParseParams /*aCase = 0*/)
	{
	}

TInt CPhonebookParameters::ScriptLinkId() const
    {
    return iLinkId;
    }

void CPhonebookParameters::SetScriptLinkId(TInt aLinkId)
    {
    iLinkId = aLinkId;
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
    CCreatorPhonebook* self = new (ELeave) CCreatorPhonebook();
    CleanupStack::PushL(self);
    self->ConstructL(aEngine);
    return self;
    }


CCreatorPhonebook::CCreatorPhonebook() 
    {
    iAddAllFields = EFalse;
    }

void CCreatorPhonebook::ConstructL(CCreatorEngine* aEngine)
    {
    LOGSTRING("Creator: CCreatorPhonebook::ConstructL");
	
	iEngine = aEngine;
	
	iContactMngr = new QContactManager("symbian");

	SetDefaultParameters();
    }
	

CCreatorPhonebook::~CCreatorPhonebook()
    {
    LOGSTRING("Creator: CCreatorPhonebook::~CCreatorPhonebook");
 
    if ( iContactsToDelete.Count() )
    	{
        TRAP_IGNORE( StoreLinksForDeleteL( iContactsToDelete, KUidDictionaryUidContacts ) );
        }
    if ( iContactGroupsToDelete.Count() )
    	{
        TRAP_IGNORE( StoreLinksForDeleteL( iContactGroupsToDelete, KUidDictionaryUidContactGroups ) );
        }
    
	if( iContactMngr )
		{
		delete iContactMngr;
		}
    
    if (iParameters)
    	{
        delete iParameters;
    	}
    }

//----------------------------------------------------------------------------

TInt CCreatorPhonebook::CreateContactEntryL(CCreatorModuleBaseParameters *aParameters)
    {
	TInt err = KErrNone;
	delete iParameters;
	    iParameters = 0;
	CPhonebookParameters* parameters = 0;
	//InitializeContactParamsL();
	
	if( aParameters == 0 )
	    {
	    InitializeContactParamsL();
	    parameters = iParameters;
	    }
	else
	    {
	    parameters = (CPhonebookParameters*) aParameters;
	    }
	    
	bool success = false;
	// create a new contact item
	QContact iStore;
	
	int numberOfFields = parameters->iContactFields.count();
	QString phone;
	for(int i=0; i< numberOfFields; i++ )
		{
		QContactDetail* cntdet = new QContactDetail(parameters->iContactFields.at(i));
		success = iStore.saveDetail( cntdet );
		delete cntdet;		
		}
	
	iContactMngr->saveContact( &iStore );
	
	iContactsToDelete.Append( (TUint32)iStore.localId() );
	
	// If this contact has a link id in script (i.e. belongs to a contact-set), we must cache the contact id:
	    if( parameters->ScriptLinkId() > 0 )
	        {                
	        RPointerArray<CCreatorContactSet>& contactsets = ContactLinkCache::Instance()->ContactSets();
	        TBool setFound(EFalse);
	        for(TInt i = 0; i < contactsets.Count(); ++i )
	            {
	            if( contactsets[i]->LinkId() == parameters->ScriptLinkId() )
	                {
	                if( iStore.localId() )
	                    {
	                    contactsets[i]->AppendL( iStore.localId() );
	                    iContactLinkArray.AppendL( (TUint32)iStore.localId() );
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
	    
    return err;
    }


//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

void CCreatorPhonebook::DeleteAllL()
    {
	QList<QContactLocalId> contacts = iContactMngr->contactIds();
	DeleteContactsL( contacts );
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteAllCreatedByCreatorL");
    DeleteItemsCreatedWithCreatorL( KUidDictionaryUidContacts );
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllGroupsL()
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteAllGroupsL");
    
    QList<QContactLocalId> contacts = iContactMngr->contactIds();
    QList<QContactLocalId> groups;
    for(int i = 0; i < contacts.count(); i++)
    	{
		QContact group = iContactMngr->contact( contacts.at(i) );
		if( group.type() == QContactType::TypeGroup )
			{
			groups.append( group.localId() );
			}
    	}
    DeleteContactsL( groups );

    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteAllGroupsCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorPhonebook::DeleteAllGroupsCreatedByCreatorL");
    DeleteItemsCreatedWithCreatorL( KUidDictionaryUidContactGroups );
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteContactsL( QList<QContactLocalId>& aContacts /*MVPbkContactLinkArray* aContacts, TBool aGroup*/ )
    {
	//QList<QContactLocalId> contacts = iContactMngr->contactIds();
    QMap<int, QContactManager::Error> errorMap;
	iContactMngr->removeContacts( aContacts, &errorMap );
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DeleteItemsCreatedWithCreatorL( TUid aStoreUid )
    {
	CDictionaryFileStore* store = iEngine->FileStoreLC();
	       User::LeaveIfNull( store );
	       
	       QList<QContactLocalId> contacts;
	       // backup previous contact links from store
	       // otherwise they would be overwritten when calling out.WriteL
	       TUint32 creatorLink;
	       if ( store->IsPresentL( aStoreUid ) )
	           {
	           RDictionaryReadStream in;
	           in.OpenLC( *store, aStoreUid );
	           TRAP_IGNORE( 
	               do{ 
	                   creatorLink = in.ReadUint32L();
	                   QContact contact = iContactMngr->contact( creatorLink );
	                   if( (contact.type() == QContactType::TypeGroup && aStoreUid == KUidDictionaryUidContactGroups ) ||  (contact.type() != QContactType::TypeGroup && aStoreUid != KUidDictionaryUidContactGroups) )
	                	   {
						   contacts.append( creatorLink );
	                	   }
	               }while( creatorLink );)
	           
	           CleanupStack::PopAndDestroy(); // in
	           }
	       
	       
	       DeleteContactsL( contacts );
	       
	       store->Remove( aStoreUid );
	       store->CommitL();
	       
	       CleanupStack::PopAndDestroy( store );
    }

//----------------------------------------------------------------------------
void CCreatorPhonebook::DoDeleteItemsCreatedWithCreatorL( TUid /*aStoreUid*/, CDictionaryFileStore* /*aStore*/ )
    {
    
    }

//----------------------------------------------------------------------------
TBool CCreatorPhonebook::HasOtherThanGroupsL( /*MVPbkContactLinkArray* aContacts */)
    {
    LOGSTRING("Creator: CCreatorPhonebook::HasOtherThanGroupsL");
    TBool result( EFalse );
    return result;
    }

QContactDetail CCreatorPhonebook::CreateContactDetail(QString aDetail, QString aFieldContext, QString aFieldString, TInt aRand )
	{
	QContactDetail contactDetail;
	TPtrC contentData;
	if( aDetail == QContactPhoneNumber::DefinitionName)
		{
		QContactPhoneNumber phoneNumber;// = contactDetail;
		if(!aFieldContext.isEmpty())
		   	{
			phoneNumber.setContexts(aFieldContext);
			}
		phoneNumber.setSubTypes(aFieldString);
		contentData.Set(iEngine->RandomString((CCreatorEngine::TRandomStringType) aRand));
		QString number = QString::fromUtf16(contentData.Ptr(),contentData.Length());
		phoneNumber.setNumber(number);
		return phoneNumber;
		}
	else if( aDetail == QContactName::DefinitionName )			//--Contact NAME-----------------------------
		{
		QContactName contactName;
		for(int i = 0 ; i < iParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
			{
			if(iParameters->iContactFields.at(i).definitionName() == QContactName::DefinitionName )
				{
				contactName = iParameters->iContactFields.at(i);
				}
			}
		contentData.Set(iEngine->RandomString((CCreatorEngine::TRandomStringType) aRand));
		QString name = QString::fromUtf16(contentData.Ptr(),contentData.Length());
		if(aFieldString == QContactName::FieldFirstName)
			{
			if(contactName.firstName().isEmpty())
				{
				contactName.setFirstName( name );
				}
			}
		else if(aFieldString == QContactName::FieldLastName)
			{
			if(contactName.lastName().isEmpty())
				{
				contactName.setLastName( name );
				}
			}
		else if(aFieldString == QContactName::FieldMiddleName)
			{
			if(contactName.middleName().isEmpty())
				{
				contactName.setMiddleName( name );
				}
			}
		else if(aFieldString == QContactName::FieldPrefix)
			{
			if(contactName.prefix().isEmpty())
				{
				contactName.setPrefix( name );
				}
			}
		else if(aFieldString == QContactName::FieldSuffix)
			{
			if(contactName.suffix().isEmpty())
				{
				contactName.setSuffix( name );
				}
			}
		else		//QContactName::FieldCustomLabel:
			{
			if(contactName.customLabel().isEmpty())
				{
				contactName.setCustomLabel( name );
				}
			}
		return contactName;
		}
	else if( aDetail == QContactOrganization::DefinitionName )			//--Contact Company-----------------------------
		{
		QContactOrganization contactCompany;
		
		for(int i = 0 ; i < iParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
			{
			if(iParameters->iContactFields.at(i).definitionName() == QContactOrganization::DefinitionName )
				{
				contactCompany = iParameters->iContactFields.at(i);
				}
			}
		contentData.Set(iEngine->RandomString((CCreatorEngine::TRandomStringType) aRand));
		QString company = QString::fromUtf16(contentData.Ptr(),contentData.Length());
		if(aFieldString == QContactOrganization::FieldName)
			{
			if(contactCompany.name().isEmpty())
				{
				contactCompany.setName( company );
				}
			}
		if(aFieldString == QContactOrganization::FieldTitle)
			{
			if(contactCompany.title().isEmpty())
				{
				contactCompany.setTitle( company );
				}
			}
		if(aFieldString == QContactOrganization::FieldDepartment)
			{
				QStringList depList = contactCompany.department();
				depList.append(company);
				contactCompany.setDepartment(depList);
			}
		if(aFieldString == QContactOrganization::FieldAssistantName)
			{
			if(contactCompany.assistantName().isEmpty())
				{
				contactCompany.setAssistantName( company );
				}
			}
		return contactCompany;
		}
	else if( aDetail == QContactAddress::DefinitionName )			//--Contact Address-----------------------------
			{
			QContactAddress contactAddress;
			
			for(int i = 0 ; i < iParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
				{
				if(iParameters->iContactFields.at(i).definitionName() == QContactAddress::DefinitionName && iParameters->iContactFields.at(i).value(QContactDetail::FieldContext) == aFieldContext )
					{
					contactAddress = iParameters->iContactFields.at(i);
					}
				}
			
			contactAddress.setContexts( aFieldContext );
			
			contentData.Set(iEngine->RandomString((CCreatorEngine::TRandomStringType) aRand));
			QString address = QString::fromUtf16(contentData.Ptr(),contentData.Length());
			if(aFieldString == QContactAddress::FieldStreet )
				{
				if( contactAddress.street().isEmpty() )
					{
					contactAddress.setStreet( address );
					}
				}
			else if(aFieldString == QContactAddress::FieldLocality )
				{
				if( contactAddress.locality().isEmpty() )
					{
					contactAddress.setLocality( address );
					}
				}
			else if(aFieldString == QContactAddress::FieldRegion )
				{
				if( contactAddress.region().isEmpty() )
					{
					contactAddress.setRegion( address );
					}
				}
			else if(aFieldString == QContactAddress::FieldPostcode )
				{
				if( contactAddress.postcode().isEmpty() )
					{
					contactAddress.setPostcode( address );
					}
				}
			else if(aFieldString == QContactAddress::FieldCountry )
				{
				if( contactAddress.country().isEmpty() )
					{
					contactAddress.setCountry( address );
					}
				}
			else 
				{
				return contactDetail;
				}
			return contactAddress;
			}
	else if( aDetail == QContactNote::DefinitionName )			//--Contact Note-----------------------------
				{
				QContactNote contactNote;
				contentData.Set(iEngine->RandomString((CCreatorEngine::TRandomStringType) aRand));
				QString note = QString::fromUtf16(contentData.Ptr(),contentData.Length());
				contactNote.setNote(note);
				return contactNote;
				}
	else if( aDetail == QContactFamily::DefinitionName )			//--Contact Family-----------------------------
				{
				QContactFamily contactFamily;
				
				for(int i = 0 ; i < iParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
					{
					if(iParameters->iContactFields.at(i).definitionName() == QContactFamily::DefinitionName && iParameters->iContactFields.at(i).value(QContactDetail::FieldContext) == aFieldContext )
						{
						contactFamily = iParameters->iContactFields.at(i);
						}
					}
				contentData.Set(iEngine->RandomString((CCreatorEngine::TRandomStringType) aRand));
				QString familyData = QString::fromUtf16(contentData.Ptr(),contentData.Length());
				if(aFieldString == QContactFamily::FieldSpouse )
					{
					if( contactFamily.spouse().isEmpty() )
						{
						contactFamily.setSpouse( familyData );
						}
					}
				if(aFieldString == QContactFamily::FieldChildren )
					{
						QStringList children = contactFamily.children();
						children.append( familyData );
						contactFamily.setChildren( children );
					}
				
				return contactFamily;
				}
	
	if( aDetail == QContactAvatar::DefinitionName)						//--Contact Picture-----------------------------
			{
			RFs& fs = CCoeEnv::Static()->FsSession();
			QContactAvatar contactAvatar;
			TBuf<KMaxFileName> srcPath;
			iEngine->RandomPictureFileL(srcPath);
			TBuf<KMaxFileName> destPath(KTempPath);
						
			if(!BaflUtils::FolderExists( fs, destPath ))
				{
				BaflUtils::EnsurePathExistsL( fs, destPath );
				}
			
			TInt err=BaflUtils::CopyFile( fs, srcPath, destPath );

			TParse temp;
			temp.Set( srcPath,NULL,NULL );
			destPath.Append(temp.NameAndExt());
			
			QString avatarFile = QString::fromUtf16( destPath.Ptr(),destPath.Length() );
			
			QPixmap avatarPix(avatarFile);

			contactAvatar.setAvatar(avatarFile);
            contactAvatar.setPixmap(avatarPix);

			return contactAvatar;
			}
	if( aDetail == QContactAnniversary::DefinitionName)
			{
			QContactAnniversary contactAnniversary;
			QDate date;
			TTime datetime = iEngine->RandomDate( CCreatorEngine::EDateFuture );
			date.setDate( datetime.DateTime().Year(),(int) (datetime.DateTime().Month()+1), datetime.DateTime().Day() );
			contactAnniversary.setOriginalDate( date );
			return contactAnniversary;
			}
			


	
	
	return contactDetail;
	}
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
void CCreatorPhonebook::StoreLinksForDeleteL( RArray<TUint32>& aLinks, TUid aStoreUid )
    {
    LOGSTRING("Creator: CCreatorPhonebook::StoreLinksForDeleteL");
    CDictionaryFileStore* store = iEngine->FileStoreLC();
       User::LeaveIfNull( store );
       
       // backup previous contact links from store
       // otherwise they would be overwritten when calling out.WriteL
       TUint32 previousLink;
       if ( store->IsPresentL( aStoreUid ) )
           {
           RDictionaryReadStream in;
           in.OpenLC( *store, aStoreUid );
           bool r = false;
           TRAP_IGNORE( 
               do{ //iContactMngr->CreateLinksLC( in )) != NULL ) // will leave with KErrEof
                   previousLink = in.ReadUint32L();
                   iPreviousDeleteLinks.Append( previousLink );
                   if(previousLink)
                	   {
					   r=true;
                	   }
                   else
                	   {
					   r=false;
                	   }
               }while(r==true); );
           
           CleanupStack::PopAndDestroy(); // in
           }

       RDictionaryWriteStream out;       
       out.AssignLC( *store, aStoreUid );
       
       // restore previous links
       for ( TInt i = 0; i < iPreviousDeleteLinks.Count(); i++ )
           {
           out.WriteUint32L( iPreviousDeleteLinks[i] );
           //CleanupStack::PopAndDestroy(); // PackLC            
           }
       
       // write new links
       for(int i=0; i < aLinks.Count(); i++)
    	   {
		   //TUint32 localId = (TUint32) aLinks[i];
		   //out.WriteUint32L( aLinks[i] );
		   out.WriteUint32L( aLinks[i] );
    	   }
       out.CommitL();
       //CleanupStack::PopAndDestroy(); // aLinks.PackLC
       CleanupStack::PopAndDestroy(); // out
       
       store->CommitL();

       CleanupStack::PopAndDestroy( store );
    }

//----------------------------------------------------------------------------
       
//----------------------------------------------------------------------------

void CCreatorPhonebook::InitializeContactParamsL(/*CCreatorModuleBaseParameters* aParameters*/)
    {
    LOGSTRING("Creator: CCreatorPhonebook::InitializeContactParamsL");        
 
    iParameters = new (ELeave) CPhonebookParameters;
           
    iParameters->iNumberOfPhoneNumberFields = iNumberOfPhoneNumberFields;
    iParameters->iNumberOfURLFields = iNumberOfURLFields;
    iParameters->iNumberOfEmailAddressFields = iNumberOfEmailAddressFields;       
    
        
    QContactName name;
    
    TPtrC fname = iEngine->RandomString(CCreatorEngine::EFirstName);
    TPtrC lname = iEngine->RandomString(CCreatorEngine::ESurname);
    QString firstname = QString::fromUtf16( fname.Ptr(), fname.Length() ); 
    QString lastname  = QString::fromUtf16( lname.Ptr(), lname.Length() );               
    
    name.setFirstName(firstname);
    name.setLastName(lastname);
    
    if( iAddAllFields )
        {
		TInt textFieldCount = sizeof(CreatorPbkMiscTextFields) / sizeof(DetailFieldInfo);
		for( TInt tfIndex = 0; tfIndex < textFieldCount; ++tfIndex )
                    { 
//    				QContactDetail field = CreateContactDetail( CreatorPbkMiscTextFields[tfIndex].iDetail, CreatorPbkMiscTextFields[tfIndex].iFieldContext, CreatorPbkMiscTextFields[tfIndex].iFieldString, CreatorPbkMiscTextFields[tfIndex].iRandomType );
    				
    				CCreatorContactField* field = CCreatorContactField::NewL();
	                CleanupStack::PushL( field );
	                QContactDetail cntDetail = field->CreateContactDetail(iEngine,iParameters,CreatorPbkMiscTextFields[tfIndex].iDetail, CreatorPbkMiscTextFields[tfIndex].iFieldContext, CreatorPbkMiscTextFields[tfIndex].iFieldString, CreatorPbkMiscTextFields[tfIndex].iRandomType );
  	                if(!cntDetail.isEmpty())
  	                	{
  						field->AddFieldToParam( iParameters, cntDetail ); //it will do "param->iContactFields.AppendL(field);"
  	                	}
  	                CleanupStack::Pop( field );
                    }
		// Add binary fields:
		TPtrC emptyData;
		CCreatorContactField* fieldPicture = CCreatorContactField::NewL();
		CleanupStack::PushL( fieldPicture );
		QContactDetail cntDetail = fieldPicture->CreateContactDetail(iEngine,iParameters,QContactAvatar::DefinitionName,"","",emptyData );
		if(!cntDetail.isEmpty())
		  	{
		fieldPicture->AddFieldToParam( iParameters, cntDetail ); //it will do "param->iContactFields.AppendL(field);"
		  	}
		CleanupStack::Pop( fieldPicture );
		
		// Add date-time fields:
		CCreatorContactField* fieldAnniv = CCreatorContactField::NewL();
		CleanupStack::PushL( fieldAnniv );
		QContactDetail cntDetAnniv = fieldAnniv->CreateContactDetail(iEngine,iParameters,QContactAnniversary::DefinitionName,"","",emptyData );
		if(!cntDetAnniv .isEmpty())
		  	{
			fieldAnniv->AddFieldToParam( iParameters, cntDetAnniv ); //it will do "param->iContactFields.AppendL(field);"
			}
		CleanupStack::Pop( fieldAnniv );

    //***************************************************************************
    
        }
    else
        {    
		iParameters->iContactFields.append( name );
        }
                        
      // Phone numbers:
    TInt phoneFieldCount = sizeof(CreatorPhoneNumberFields) / sizeof(PhoneNumInfo);
    
    TInt inc=0;
    for( int i=0; i<iNumberOfPhoneNumberFields; i++ )
    	{
    
		if(inc >= phoneFieldCount )
        	{
    		inc = 0;
        	}
    
		QContactPhoneNumber phoneNum;
		TPtrC phoneNumber = iEngine->RandomString(CCreatorEngine::EPhoneNumber);
		QString phone = QString::fromUtf16( phoneNumber.Ptr(), phoneNumber.Length() );
		if(!CreatorPhoneNumberFields[inc].iFieldContext.isEmpty())
			{
			phoneNum.setContexts(CreatorPhoneNumberFields[inc].iFieldContext);
			}
		phoneNum.setSubTypes(CreatorPhoneNumberFields[inc].iFieldString);
		

		phoneNum.setNumber( phone );

		iParameters->iContactFields.append( phoneNum );
		inc++;
    	}
    
    
    
    // URLs:
    for( int i=0; i<iNumberOfURLFields; i++ )
        {
    	CCreatorContactField* fieldUrl = CCreatorContactField::NewL();
    	CleanupStack::PushL( fieldUrl );
    	QContactDetail cntDetUrl = fieldUrl->CreateContactDetail(iEngine,iParameters,QContactUrl::DefinitionName,"","",KErrNotFound );
    	if(!cntDetUrl.isEmpty())
    	  	{
			fieldUrl->AddFieldToParam( iParameters, cntDetUrl ); //it will do "param->iContactFields.AppendL(field);"
    	  	}
    	CleanupStack::Pop( fieldUrl );

        }

    // EMail addresses:
    for( int i=0; i<iNumberOfEmailAddressFields; i++ )
        	{
			CCreatorContactField* fieldEmail = CCreatorContactField::NewL();
        	CleanupStack::PushL( fieldEmail );
        	QContactDetail cntDetEmail = fieldEmail->CreateContactDetail(iEngine,iParameters,QContactEmailAddress::DefinitionName,"","",KErrNotFound );
        	if(!cntDetEmail.isEmpty())
        	  	{
				fieldEmail->AddFieldToParam( iParameters, cntDetEmail ); //it will do "param->iContactFields.AppendL(field);"
        	  	}
        	CleanupStack::Pop( fieldEmail );
        	}
    
    }



// Checks if the link is a group or not
TBool CCreatorPhonebook::IsContactGroupL( QContactLocalId& aLink )
	{
    QContact group = iContactMngr->contact( aLink );
    if( group.type() == QContactType::TypeGroup )
        {
        return ETrue;
        }
	return EFalse;
	}

TInt CCreatorPhonebook::CreateGroupEntryL(CCreatorModuleBaseParameters *aParameters)
    {
	 LOGSTRING("Creator: CCreatorVirtualPhonebook::CreateGroupEntryL");
	    delete iParameters;
	    iParameters = 0;
	    
	    CPhonebookParameters* parameters = (CPhonebookParameters*) aParameters;
	    
	    if( !parameters )
	        {
	        iParameters = new (ELeave) CPhonebookParameters;
	        iParameters->iContactsInGroup = iContactsInGroup;
	        TPtrC gname = iEngine->RandomString(CCreatorEngine::EGroupName);
	        iParameters->iGroupName = QString::fromUtf16( gname.Ptr(),gname.Length() );
	        iParameters->iGroupName += " #";
	        int rnd = iEngine->RandomNumber(1000, 9999);
	        iParameters->iGroupName += QString::number( rnd, 10 );
	        parameters = iParameters;
	        }

	    TInt err = KErrNone;
	 
	    // create a new contact group
	    QContact newGroup;
	    newGroup.setType(QContactType::TypeGroup);
        QContactName newGroupName;
        newGroupName.setCustomLabel( parameters->iGroupName );
        newGroup.saveDetail(&newGroupName);
        iContactMngr->saveContact(&newGroup);
        QContactLocalId newGroupId = newGroup.localId();

	        
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
	          
	            const RArray<QContactLocalId> links = set.ContactLinks();//ContactLinkCache::Instance()->ContactSets();//set.ContactLinks();
	            TInt numberOfExplicitLinks = links.Count(); // Number of defined contacts in contact-set
	            TInt numberOfExistingContacts = set.NumberOfExistingContacts(); // Number of existing contacts in contact-set
	            TInt maxAmount = numberOfExplicitLinks + numberOfExistingContacts;
	            
	            if( parameters->iLinkIds[i].iLinkAmount > 0 )
	                maxAmount = parameters->iLinkIds[i].iLinkAmount; // Max amount is limited
	            
	            TInt addedMembers = 0;
	            
	            for( TInt j = 0; j < links.Count() && addedMembers < maxAmount; ++j )
	                {
	                QContactLocalId link = links[j];
	                QContact contactLink = iContactMngr->contact( link );
	                if( link && IsContactGroupL( link ) == EFalse )
	                    {
                        //QList<QContactRelationship> relationships = contactLink.relationships(QContactRelationship::HasMember);
	                    if(/*!relationships.count() && */contactLink.type() == QContactType::TypeContact ) //just for contacts that are not in relationship - not in group yet
	                        {
	                        QContactRelationship* contactRel = new QContactRelationship();
	                        contactRel->setRelationshipType(QContactRelationship::HasMember);
	                        contactRel->setFirst(newGroup.id());
	                        contactRel->setSecond( contactLink.id() );
	                        iContactMngr->saveRelationship( contactRel );
	                        delete contactRel;
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
			QList<QContactLocalId> contacts = iContactMngr->contactIds();
			QContactRelationshipFilter rFilter;
			rFilter.setRelationshipType(QContactRelationship::HasMember);
			rFilter.setRelatedContactRole(QContactRelationshipFilter::First);
			rFilter.setRelatedContactId( newGroup.id() );
			
			int cnt = 0;
			for(int i=0; cnt < amountOfContactsToBeAdded && i < contacts.count() ; i++ )
				{
				QContact contact = iContactMngr->contact( iContactMngr->contactIds().at(i) );
				if( iContactMngr->error() == QContactManager::DoesNotExistError )
					{
				
					}
				else
					{
					QList<QContactRelationship> relationships = contact.relationships(QContactRelationship::HasMember);
					if(!relationships.count() && contact.type() == QContactType::TypeContact ) //just for contacts that are not in relationship - not in group yet
						{
						QContactRelationship* contactRel = new QContactRelationship();
						contactRel->setRelationshipType(QContactRelationship::HasMember);
						contactRel->setFirst(newGroup.id());
						contactRel->setSecond(contact.id());
						iContactMngr->saveRelationship( contactRel );
						delete contactRel;
						cnt++;
						}
					}
				}
			
	       // group members and their count
			QList<QContactLocalId> groupMemberIds = iContactMngr->contactIds( rFilter );
			int testcnt = groupMemberIds.count(); 
	    
	   
	        }
	    
	    // store the link to contact, so that Creator is able to delete
	    // it when user requests deletion of contacts that were created with Creator 
	    iContactGroupsToDelete.Append( (TUint32)newGroupId );
	    
	    return err;
    }

//----------------------------------------------------------------------------

TInt CCreatorPhonebook::CreateSubscribedContactEntryL(CCreatorModuleBaseParameters* /*aParameters*/)
    {
    LOGSTRING("Creator: CCreatorPhonebook::CreateSubscribedContactEntryL");

    return KErrNotSupported;
    }

//----------------------------------------------------------------------------

CCreatorContactField::CCreatorContactField()
    {
    }
CCreatorContactField::~CCreatorContactField()
    {
    }

CCreatorContactField* CCreatorContactField::NewL()
    {
    CCreatorContactField* self = new (ELeave) CCreatorContactField();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
void CCreatorContactField::ConstructL()
    {
    //pImpl = CCreatorContactTextField::NewL(aFieldType, aData); 
    }
QContactDetail CCreatorContactField::CreateContactDetail(CCreatorEngine* aEngine,CPhonebookParameters* aParameters,QString aDetail, QString aFieldContext, QString aFieldString, TInt aRand )
    {
	QContactDetail emptyDet;
    TPtrC contentData;
    HBufC16* tempData = 0;
    if(aRand == KErrNotFound)
    	{
		TInt textFieldCount = sizeof(CreatorPbkMiscTextFields) / sizeof(DetailFieldInfo);
		for( TInt tfIndex = 0; tfIndex < textFieldCount; ++tfIndex )
			{
			if( CreatorPbkMiscTextFields[tfIndex].iDetail == aDetail && CreatorPbkMiscTextFields[tfIndex].iFieldContext == aFieldContext && CreatorPbkMiscTextFields[tfIndex].iFieldString == aFieldString)
				{
				aRand = CreatorPbkMiscTextFields[tfIndex].iRandomType;
				}
			}
    	}
    
    if( aRand != KErrNotFound ) 
    	{
		contentData.Set(aEngine->RandomString((CCreatorEngine::TRandomStringType) aRand));
    	}
    else
    	{
		if( aDetail == QContactPhoneNumber::DefinitionName )
			{
			contentData.Set(aEngine->RandomString(CCreatorEngine::EPhoneNumber));
			}
		else if( aDetail == QContactEmailAddress::DefinitionName )
			{
			tempData = aEngine->CreateEmailAddressLC();
			contentData.Set( tempData->Des() );
			}
		else if( aDetail == QContactUrl::DefinitionName )
			{
			tempData = aEngine->CreateHTTPUrlLC(); 
			contentData.Set( tempData->Des() );
			}
		else
			{
			return emptyDet;
			}
    	}
    emptyDet = CreateContactDetail( aEngine, aParameters, aDetail, aFieldContext, aFieldString, contentData );
    if( tempData )
    	{
		CleanupStack::PopAndDestroy( tempData );
    	}
    return emptyDet;
    }

QContactDetail CCreatorContactField::CreateContactDetail(CCreatorEngine* aEngine,CPhonebookParameters* aParameters,QString aDetail, QString aFieldContext, QString aFieldString, TPtrC aData )
    {
    QContactDetail contactDetail;
        
        if( aDetail == QContactPhoneNumber::DefinitionName)
            {
            QContactPhoneNumber phoneNumber;// = contactDetail;
            if(!aFieldContext.isEmpty())
            	{
				phoneNumber.setContexts(aFieldContext);
            	}
            phoneNumber.setSubTypes(aFieldString);
            QString number = QString::fromUtf16(aData.Ptr(),aData.Length());
            phoneNumber.setNumber(number);
            return phoneNumber;
            }
        else if( aDetail == QContactName::DefinitionName )          //--Contact NAME-----------------------------
            {
            QContactName contactName;
            for(int i = 0 ; i < aParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
                {
                if(aParameters->iContactFields.at(i).definitionName() == QContactName::DefinitionName )
                    {
                    contactName = aParameters->iContactFields.at(i);
                    }
                }
            QString name = QString::fromUtf16(aData.Ptr(),aData.Length());
            if(aFieldString == QContactName::FieldFirstName)
                {
                if(contactName.firstName().isEmpty())
                    {
                    contactName.setFirstName( name );
                    }
                }
            else if(aFieldString == QContactName::FieldLastName)
                {
                if(contactName.lastName().isEmpty())
                    {
                    contactName.setLastName( name );
                    }
                }
            else if(aFieldString == QContactName::FieldMiddleName)
                {
                if(contactName.middleName().isEmpty())
                    {
                    contactName.setMiddleName( name );
                    }
                }
            else if(aFieldString == QContactName::FieldPrefix)
                {
                if(contactName.prefix().isEmpty())
                    {
                    contactName.setPrefix( name );
                    }
                }
            else if(aFieldString == QContactName::FieldSuffix)
                {
                if(contactName.suffix().isEmpty())
                    {
                    contactName.setSuffix( name );
                    }
                }
            else        //QContactName::FieldCustomLabel:
                {
                if(contactName.customLabel().isEmpty())
                    {
                    contactName.setCustomLabel( name );
                    }
                }
            return contactName;
            }
        else if( aDetail == QContactOrganization::DefinitionName )          //--Contact Company-----------------------------
            {
            QContactOrganization contactCompany;
            
            for(int i = 0 ; i < aParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
                {
                if(aParameters->iContactFields.at(i).definitionName() == QContactOrganization::DefinitionName )
                    {
                    contactCompany = aParameters->iContactFields.at(i);
                    }
                }
            
            QString company = QString::fromUtf16(aData.Ptr(),aData.Length());
            if(aFieldString == QContactOrganization::FieldName)
                {
                if(contactCompany.name().isEmpty())
                    {
                    contactCompany.setName( company );
                    }
                }
            if(aFieldString == QContactOrganization::FieldTitle)
                {
                if(contactCompany.title().isEmpty())
                    {
                    contactCompany.setTitle( company );
                    }
                }
            if(aFieldString == QContactOrganization::FieldDepartment)
                {
                QStringList depList = contactCompany.department();
                depList.append(company);
                contactCompany.setDepartment(depList);
                }
            if(aFieldString == QContactOrganization::FieldAssistantName)
                {
                if(contactCompany.assistantName().isEmpty())
                    {
                    contactCompany.setAssistantName( company );
                    }
                }
            return contactCompany;
            }
        else if( aDetail == QContactAddress::DefinitionName )           //--Contact Address-----------------------------
                {
                QContactAddress contactAddress;
                
                for(int i = 0 ; i < aParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
                    {
                    if(aParameters->iContactFields.at(i).definitionName() == QContactAddress::DefinitionName && aParameters->iContactFields.at(i).value(QContactDetail::FieldContext) == aFieldContext )
                        {
                        contactAddress = aParameters->iContactFields.at(i);
                        }
                    }
                if( !aFieldContext.isEmpty() )
                    {
					contactAddress.setContexts( aFieldContext );
                    }
                QString address = QString::fromUtf16(aData.Ptr(),aData.Length());
                if(aFieldString == QContactAddress::FieldStreet )
                    {
                    if( contactAddress.street().isEmpty() )
                        {
                        contactAddress.setStreet( address );
                        }
                    }
                else if(aFieldString == QContactAddress::FieldLocality )
                    {
                    if( contactAddress.locality().isEmpty() )
                        {
                        contactAddress.setLocality( address );
                        }
                    }
                else if(aFieldString == QContactAddress::FieldRegion )
                    {
                    if( contactAddress.region().isEmpty() )
                        {
                        contactAddress.setRegion( address );
                        }
                    }
                else if(aFieldString == QContactAddress::FieldPostcode )
                    {
                    if( contactAddress.postcode().isEmpty() )
                        {
                        contactAddress.setPostcode( address );
                        }
                    }
                else if(aFieldString == QContactAddress::FieldCountry )
                    {
                    if( contactAddress.country().isEmpty() )
                        {
                        contactAddress.setCountry( address );
                        }
                    }
                else 
                    {
                    return contactDetail;
                    }
                return contactAddress;
                }
        else if( aDetail == QContactNote::DefinitionName )          //--Contact Note-----------------------------
                    {
                    QContactNote contactNote;
                    QString note = QString::fromUtf16(aData.Ptr(),aData.Length());
                    contactNote.setNote(note);
                    return contactNote;
                    }
        else if( aDetail == QContactFamily::DefinitionName )            //--Contact Family-----------------------------
                    {
                    QContactFamily contactFamily;
                    
                    for(int i = 0 ; i < aParameters->iContactFields.count() ; i++ ) //go through all contact details to check if there is already Contact Name to set other details
                        {
                        if(aParameters->iContactFields.at(i).definitionName() == QContactFamily::DefinitionName && aParameters->iContactFields.at(i).value(QContactDetail::FieldContext) == aFieldContext )
                            {
                            contactFamily = aParameters->iContactFields.at(i);
                            }
                        }
                   
                    QString familyData = QString::fromUtf16(aData.Ptr(),aData.Length());
                    if(aFieldString == QContactFamily::FieldSpouse )
                        {
                        if( contactFamily.spouse().isEmpty() )
                            {
                            contactFamily.setSpouse( familyData );
                            }
                        }
                    if(aFieldString == QContactFamily::FieldChildren )
                        {
                            QStringList children = contactFamily.children();
                            children.append( familyData );
                            contactFamily.setChildren( children );
                        }
                    
                    return contactFamily;
                    }
        
        if( aDetail == QContactAvatar::DefinitionName)                      //--Contact Picture-----------------------------
                {
                RFs& fs = CCoeEnv::Static()->FsSession();
                QContactAvatar contactAvatar;
                TBuf<KMaxFileName> srcPath;
                aEngine->RandomPictureFileL(srcPath);
                TBuf<KMaxFileName> destPath(KTempPath);
                            
                if(!BaflUtils::FolderExists( fs, destPath ))
                    {
                    BaflUtils::EnsurePathExistsL( fs, destPath );
                    }
                
                TInt err=BaflUtils::CopyFile( fs, srcPath, destPath );

                TParse temp;
                temp.Set( srcPath,NULL,NULL );
                destPath.Append(temp.NameAndExt());
                
                QString avatarFile = QString::fromUtf16( destPath.Ptr(),destPath.Length() );
                
                QPixmap avatarPix(avatarFile);

                contactAvatar.setAvatar(avatarFile);
                contactAvatar.setPixmap(avatarPix);

                return contactAvatar;
                }
        if( aDetail == QContactAnniversary::DefinitionName) //--Anniversary------------------------------
                {
                QContactAnniversary contactAnniversary;
                QDate date;
                TTime datetime = aEngine->RandomDate( CCreatorEngine::EDateFuture );
                date.setDate( datetime.DateTime().Year(),(int) (datetime.DateTime().Month()+1), datetime.DateTime().Day() );
                contactAnniversary.setOriginalDate( date );
                return contactAnniversary;
                }
        if( aDetail == QContactEmailAddress::DefinitionName) //--Email------------------------------------
            {
            QContactEmailAddress email;
            QString emailAddr = QString::fromUtf16(aData.Ptr(),aData.Length());
            if( !aFieldContext.isEmpty() )
            	{
				email.setContexts( aFieldContext );
            	}
            email.setEmailAddress( emailAddr );
            return email;
            }
        if( aDetail == QContactUrl::DefinitionName )    //--Url-------------------------------------------
            {
            QContactUrl url;
            QString urlStr = QString::fromUtf16(aData.Ptr(),aData.Length());
            if( !aFieldContext.isEmpty() )
                {
				url.setContexts( aFieldContext );
                }
            url.setUrl(urlStr);
            return url;
            }
        if( aDetail == QContactBirthday::DefinitionName )   //--Birthday-----------------------------------
            {
            QContactBirthday birthday;
            QDate date;
            TTime datetime = aEngine->RandomDate( CCreatorEngine::EDatePast );
            date.setDate( datetime.DateTime().Year(),(int) (datetime.DateTime().Month()+1), datetime.DateTime().Day() );
            birthday.setDate( date );
            return birthday;
            }
                
        return contactDetail;
    }

void CCreatorContactField::AddFieldToParam( CPhonebookParameters* aParam, QContactDetail aDetail)
    {
     bool replace = false;
     for(int i = 0 ; i< aParam->iContactFields.count() ; i++) //go through
          {
           if( !aDetail.isEmpty() && aDetail.definitionName() == aParam->iContactFields.at(i).definitionName() 
        		   && aDetail.definitionName() != QContactPhoneNumber::DefinitionName 
        		   && aDetail.definitionName() != QContactEmailAddress::DefinitionName
        		   && aDetail.definitionName() != QContactUrl::DefinitionName )
               {
               QString context = aDetail.value(QContactDetail::FieldContext);
               bool isContextEmpty = context.isEmpty();
               if( isContextEmpty ||  ( aDetail.value(QContactDetail::FieldContext) == aParam->iContactFields.at(i).value(QContactDetail::FieldContext)) )
                   {
                   //replace
                   aParam->iContactFields.replace(i,aDetail);
                   replace = true;
                   }
               }
          }
          if(!replace)
             {
             if(!aDetail.isEmpty())
                {
                aParam->iContactFields.append(aDetail);
                }
             }
      
    }
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

