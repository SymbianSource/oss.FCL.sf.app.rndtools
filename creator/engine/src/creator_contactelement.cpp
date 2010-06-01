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



#include "creator_contactelement.h"
#include "creator_traces.h"
#include "creator_phonebook.h"
#include "creator_factory.h"
#include "creator_contactsetcache.h"
#include <xml/documentparameters.h>
#include <qtcontacts.h>

using namespace Xml;
using namespace creatorcontact;

/**
 * Contact field element mappings:
 */
enum TContactFieldDataType
    {
    EDataTypeText,
    EDataTypeBinary,
    EDataTypeDateTime    
    };
    
class FieldMapping
{
public:
    TPtrC iElementName;
    QString iDetail;
    QString iContext;
    QString  iFieldCode;
    TContactFieldDataType iDataType;
};
//*********************************************************************************
FieldMapping fieldMappingTbl[] = {
       {TPtrC(KFirstname), (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactName::FieldFirstName).operator QString() /*R_VPBK_FIELD_TYPE_FIRSTNAME*/, EDataTypeText},
       {TPtrC(KLastname), (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactName::FieldLastName).operator QString() /*R_VPBK_FIELD_TYPE_LASTNAME*/, EDataTypeText},
       {TPtrC(KPrefix), (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactName::FieldPrefix).operator QString()/*R_VPBK_FIELD_TYPE_PREFIX*/, EDataTypeText},
       {TPtrC(KSuffix), (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactName::FieldSuffix).operator QString()/*R_VPBK_FIELD_TYPE_SUFFIX*/, EDataTypeText},
       {TPtrC(KSecondname), (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactName::FieldMiddleName).operator QString()/*R_VPBK_FIELD_TYPE_SECONDNAME*/, EDataTypeText},
       {TPtrC(KLandphoneHome), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactPhoneNumber::SubTypeLandline).operator QString()/*R_VPBK_FIELD_TYPE_LANDPHONEHOME*/, EDataTypeText},
       
       {TPtrC(KMobilephoneHome), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactPhoneNumber::SubTypeMobile).operator QString()/*R_VPBK_FIELD_TYPE_MOBILEPHONEHOME*/, EDataTypeText},
       {TPtrC(KVideonumberHome), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactPhoneNumber::SubTypeVideo).operator QString()/*R_VPBK_FIELD_TYPE_VIDEONUMBERHOME*/, EDataTypeText},
       {TPtrC(KFaxnumberHome), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactPhoneNumber::SubTypeFacsimile).operator QString()/*R_VPBK_FIELD_TYPE_FAXNUMBERHOME*/, EDataTypeText},
       //{TPtrC(KVoipHome), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactPhoneNumber::SubType).operator QString()/*R_VPBK_FIELD_TYPE_VOIPHOME*/, EDataTypeText},
       {TPtrC(KEmailHome), (QContactEmailAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),""/*R_VPBK_FIELD_TYPE_EMAILHOME*/, EDataTypeText},
       {TPtrC(KUrlHome), (QContactUrl::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactUrl::SubTypeHomePage).operator QString()/*R_VPBK_FIELD_TYPE_URLHOME*/, EDataTypeText},
       //{TPtrC(KAddrlabelHome), R_VPBK_FIELD_TYPE_ADDRLABELHOME, EDataTypeText},
       {TPtrC(KAddrpoHome), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAddress::FieldPostOfficeBox).operator QString()/*R_VPBK_FIELD_TYPE_ADDRPOHOME*/, EDataTypeText},
       
      // {TPtrC(KAddrextHome), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAddress::Field).operator QString()/*R_VPBK_FIELD_TYPE_ADDREXTHOME*/, EDataTypeText},
       {TPtrC(KAddrstreetHome), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAddress::FieldStreet).operator QString()/*R_VPBK_FIELD_TYPE_ADDRSTREETHOME*/, EDataTypeText},
       {TPtrC(KAddrlocalHome), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAddress::FieldLocality).operator QString()/*R_VPBK_FIELD_TYPE_ADDRLOCALHOME*/, EDataTypeText},
       {TPtrC(KAddrregionHome), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAddress::FieldRegion).operator QString()/*R_VPBK_FIELD_TYPE_ADDRREGIONHOME*/, EDataTypeText},
       {TPtrC(KAddrpostcodeHome), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAddress::FieldPostcode).operator QString()/*R_VPBK_FIELD_TYPE_ADDRPOSTCODEHOME*/, EDataTypeText},
       {TPtrC(KAddrcountryHome), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAddress::FieldCountry).operator QString()/*R_VPBK_FIELD_TYPE_ADDRCOUNTRYHOME*/, EDataTypeText},
       {TPtrC(KJobtitle), (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactOrganization::FieldTitle).operator QString()/*R_VPBK_FIELD_TYPE_JOBTITLE*/, EDataTypeText},
       {TPtrC(KCompanyname), (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactOrganization::FieldName).operator QString()/*R_VPBK_FIELD_TYPE_COMPANYNAME*/, EDataTypeText},
       
       {TPtrC(KLandphoneWork), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactPhoneNumber::SubTypeLandline).operator QString()/*R_VPBK_FIELD_TYPE_LANDPHONEWORK*/, EDataTypeText},
       {TPtrC(KMobilephoneWork), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactPhoneNumber::SubTypeMobile).operator QString()/*R_VPBK_FIELD_TYPE_MOBILEPHONEWORK*/, EDataTypeText},
       {TPtrC(KVideonumberWork), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactPhoneNumber::SubTypeVideo).operator QString()/*R_VPBK_FIELD_TYPE_VIDEONUMBERWORK*/, EDataTypeText},
       {TPtrC(KFaxnumberWork), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactPhoneNumber::SubTypeFacsimile).operator QString()/*R_VPBK_FIELD_TYPE_FAXNUMBERWORK*/, EDataTypeText},
       //{TPtrC(KVoipWork), R_VPBK_FIELD_TYPE_VOIPWORK, EDataTypeText},
       {TPtrC(KEmailWork), (QContactEmailAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),""/*R_VPBK_FIELD_TYPE_EMAILWORK*/, EDataTypeText},
       {TPtrC(KUrlWork), (QContactUrl::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactUrl::SubTypeHomePage).operator QString()/*R_VPBK_FIELD_TYPE_URLWORK*/, EDataTypeText},
       //{TPtrC(KAddrlabelWork), R_VPBK_FIELD_TYPE_ADDRLABELWORK, EDataTypeText},
       
       {TPtrC(KAddrpoWork), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAddress::FieldPostOfficeBox).operator QString()/*R_VPBK_FIELD_TYPE_ADDRPOWORK*/, EDataTypeText},
       //{TPtrC(KAddrextWork), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAddress::FieldPostOfficeBox).operator QString()/*R_VPBK_FIELD_TYPE_ADDREXTWORK*/, EDataTypeText},
       {TPtrC(KAddrstreetWork), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAddress::FieldStreet).operator QString()/*R_VPBK_FIELD_TYPE_ADDRSTREETWORK*/, EDataTypeText},
       {TPtrC(KAddrlocalWork), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAddress::FieldLocality).operator QString()/*R_VPBK_FIELD_TYPE_ADDRLOCALWORK*/, EDataTypeText},
       {TPtrC(KAddrregionWork), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAddress::FieldRegion).operator QString()/*R_VPBK_FIELD_TYPE_ADDRREGIONWORK*/, EDataTypeText},
       {TPtrC(KAddrpostcodeWork), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAddress::FieldPostcode).operator QString()/*R_VPBK_FIELD_TYPE_ADDRPOSTCODEWORK*/, EDataTypeText},
       {TPtrC(KAddrcountryWork), (QContactAddress::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAddress::FieldCountry).operator QString()/*R_VPBK_FIELD_TYPE_ADDRCOUNTRYWORK*/, EDataTypeText},
       
       {TPtrC(KLandphoneGen), (QContactPhoneNumber::DefinitionName).operator QString(), "",(QContactPhoneNumber::SubTypeLandline).operator QString(), EDataTypeText},
       {TPtrC(KMobilephoneGen), (QContactPhoneNumber::DefinitionName).operator QString(), "",(QContactPhoneNumber::SubTypeMobile).operator QString(), EDataTypeText},
       {TPtrC(KVideonumberGen), (QContactPhoneNumber::DefinitionName).operator QString(), "",(QContactPhoneNumber::SubTypeVideo).operator QString(), EDataTypeText},
       {TPtrC(KFaxnumberGen), (QContactPhoneNumber::DefinitionName).operator QString(), "",(QContactPhoneNumber::SubTypeFacsimile).operator QString(), EDataTypeText},
     //  {TPtrC(KVoipGen), R_VPBK_FIELD_TYPE_VOIPGEN, EDataTypeText},
     //  {TPtrC(KPoc), R_VPBK_FIELD_TYPE_POC, EDataTypeText},
     //  {TPtrC(KSwis), R_VPBK_FIELD_TYPE_SWIS, EDataTypeText},
     //  {TPtrC(KSip), R_VPBK_FIELD_TYPE_SIP, EDataTypeText},
       {TPtrC(KEmailGen), (QContactEmailAddress::DefinitionName).operator QString(), "","", EDataTypeText},
       
       {TPtrC(KUrlGen), (QContactUrl::DefinitionName).operator QString(), "","", EDataTypeText},
     //  {TPtrC(KAddrlabelGen), R_VPBK_FIELD_TYPE_ADDRLABELGEN, EDataTypeText},
       {TPtrC(KAddrpoGen), (QContactAddress::DefinitionName).operator QString(), "",(QContactAddress::FieldPostOfficeBox).operator QString()/*R_VPBK_FIELD_TYPE_ADDRPOGEN*/, EDataTypeText},
     //  {TPtrC(KAddrextGen), R_VPBK_FIELD_TYPE_ADDREXTGEN, EDataTypeText},
       {TPtrC(KAddrstreetGen), (QContactAddress::DefinitionName).operator QString(), "" , (QContactAddress::FieldStreet).operator QString()/*R_VPBK_FIELD_TYPE_ADDRSTREETGEN*/, EDataTypeText},
       {TPtrC(KAddrlocalGen), (QContactAddress::DefinitionName).operator QString(), "" , (QContactAddress::FieldLocality).operator QString()/*R_VPBK_FIELD_TYPE_ADDRLOCALGEN*/, EDataTypeText},
       {TPtrC(KAddrregionGen), (QContactAddress::DefinitionName).operator QString(), "" , (QContactAddress::FieldRegion).operator QString()/*R_VPBK_FIELD_TYPE_ADDRREGIONGEN*/, EDataTypeText},
       {TPtrC(KAddrpostcodeGen), (QContactAddress::DefinitionName).operator QString(), "" , (QContactAddress::FieldPostcode).operator QString()/*R_VPBK_FIELD_TYPE_ADDRPOSTCODEGEN*/, EDataTypeText},
       {TPtrC(KAddrcountryGen), (QContactAddress::DefinitionName).operator QString(), "" , (QContactAddress::FieldCountry).operator QString()/*R_VPBK_FIELD_TYPE_ADDRCOUNTRYGEN*/, EDataTypeText},
       
       {TPtrC(KPagerNumber),(QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactPhoneNumber::SubTypePager).operator QString()/* R_VPBK_FIELD_TYPE_PAGERNUMBER*/, EDataTypeText},
       {TPtrC(KDtmfString), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactPhoneNumber::SubTypeDtmfMenu).operator QString()/* R_VPBK_FIELD_TYPE_DTMFSTRING*/, EDataTypeText},
     //  {TPtrC(KWvAddress), R_VPBK_FIELD_TYPE_WVADDRESS, EDataTypeText},
       {TPtrC(KDate), (QContactBirthday::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactBirthday::FieldBirthday).operator QString()/* R_VPBK_FIELD_TYPE_DATE*/, EDataTypeDateTime},
       {TPtrC(KNote), (QContactNote::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(), (QContactNote::FieldNote).operator QString()/* R_VPBK_FIELD_TYPE_NOTE*/, EDataTypeText},
       {TPtrC(KThumbnailPath), (QContactAvatar::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactAvatar::SubTypeImage).operator QString()/* R_VPBK_FIELD_TYPE_THUMBNAILPIC*/, EDataTypeText},
      // {TPtrC(KThumbnailId), R_VPBK_FIELD_TYPE_THUMBNAILPIC, EDataTypeText},
      // {TPtrC(KRingTone),  R_VPBK_FIELD_TYPE_RINGTONE, EDataTypeText},
      // {TPtrC(KRingToneId), R_VPBK_FIELD_TYPE_RINGTONE, EDataTypeText},
       
       //{TPtrC(KCallerobjImg), R_VPBK_FIELD_TYPE_CALLEROBJIMG, EDataTypeText},
      // {TPtrC(KCallerobjText), R_VPBK_FIELD_TYPE_CALLEROBJTEXT, EDataTypeText},
       {TPtrC(KMiddlename), (QContactName::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactName::FieldMiddleName).operator QString()/*R_VPBK_FIELD_TYPE_MIDDLENAME*/, EDataTypeText},
       {TPtrC(KDepartment), (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactOrganization::FieldDepartment).operator QString()/*R_VPBK_FIELD_TYPE_DEPARTMENT*/, EDataTypeText},
       {TPtrC(KAsstname), (QContactOrganization::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactOrganization::FieldAssistantName).operator QString()/*R_VPBK_FIELD_TYPE_ASSTNAME*/, EDataTypeText},
       {TPtrC(KSpouse), (QContactFamily::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactFamily::FieldSpouse).operator QString()/* R_VPBK_FIELD_TYPE_SPOUSE*/, EDataTypeText},
       {TPtrC(KChildren), (QContactFamily::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactFamily::FieldChildren).operator QString()/*R_VPBK_FIELD_TYPE_CHILDREN, EDataTypeText},
       {TPtrC(KAsstphone), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactPhoneNumber::SubTypeAssistant).operator QString()/* R_VPBK_FIELD_TYPE_ASSTPHONE*/, EDataTypeText},
       
       {TPtrC(KCarphone), (QContactPhoneNumber::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactPhoneNumber::SubTypeCar).operator QString()/* R_VPBK_FIELD_TYPE_CARPHONE*/, EDataTypeText},
       {TPtrC(KAnniversary), (QContactAnniversary::DefinitionName).operator QString(), (QContactDetail::ContextWork).operator QString(),(QContactAnniversary::SubTypeEmployment).operator QString()/*R_VPBK_FIELD_TYPE_ANNIVERSARY*/, EDataTypeDateTime},
     //  {TPtrC(KSyncclass), R_VPBK_FIELD_TYPE_SYNCCLASS, EDataTypeText},
     //  {TPtrC(KLocPrivacy), R_VPBK_FIELD_TYPE_LOCPRIVACY, EDataTypeText},
       {TPtrC(KGenlabel), (QContactDisplayLabel::DefinitionName).operator QString(), (QContactDetail::ContextHome).operator QString(),(QContactDisplayLabel::FieldLabel).operator QString()/*R_VPBK_FIELD_TYPE_GENLABEL*/, EDataTypeText}
};

//*********************************************************************************

CCreatorContactElementBase* CCreatorContactElementBase::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactElementBase::NewL");
    CCreatorContactElementBase* self = new (ELeave) CCreatorContactElementBase(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactElementBase::CCreatorContactElementBase(CCreatorEngine* aEngine) : CCreatorScriptElement(aEngine)
    { }

CCreatorContactElement* CCreatorContactElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactElement::NewL");
    CCreatorContactElement* self = new (ELeave) CCreatorContactElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactElement::CCreatorContactElement(CCreatorEngine* aEngine) 
: 
CCreatorContactElementBase(aEngine)
    { 
    iIsCommandElement = ETrue;
    }


void CCreatorContactElement::ExecuteCommandL()
    {    
    LOGSTRING("Creator: CCreatorContactElement::ExecuteCommandL");
    // Find out how many contacts should we create:
    const CCreatorScriptAttribute* contactAmountAttr = this->FindAttributeByName(KAmount);    
    TInt contactAmount = 1;    
    if( contactAmountAttr )
        {
        contactAmount = ConvertStrToIntL(contactAmountAttr->Value());
        }
    CCreatorScriptElement* fieldsElement = FindSubElement(KFields);
    TInt fieldMappingTblSize = sizeof(fieldMappingTbl)/sizeof(FieldMapping);
    for( TInt cI = 0; cI < contactAmount; ++cI )
        {
        CPhonebookParameters* param = (CPhonebookParameters*) TCreatorFactory::CreatePhoneBookParametersL();
        CleanupStack::PushL(param);
        // Loop all the field elements
        if( fieldsElement && fieldsElement->SubElements().Count() > 0 )
            {
            
            const RPointerArray<CCreatorScriptElement>& fieldList = fieldsElement->SubElements();
            
            if( fieldList.Count() == 0 )
                {
                
                }
            for( TInt i = 0; i < fieldList.Count(); ++i )
                {
                TPtrC fieldName = fieldList[i]->Name();
                CCreatorContactField* field = 0;
				field = CCreatorContactField::NewL();

                const CCreatorScriptAttribute* amountAttr = fieldList[i]->FindAttributeByName(KAmount);
                const CCreatorScriptAttribute* rndLenAttr = fieldList[i]->FindAttributeByName(KRandomLength);
                const CCreatorScriptAttribute* increaseAttr = fieldList[i]->FindAttributeByName(KIncrease);
                TBool increase( EFalse );
                if ( increaseAttr )
                    {
                    increase = ConvertStrToBooleanL( increaseAttr->Value() );
                    }
                for( TInt j = 0; j < fieldMappingTblSize; ++j )
                    {
                    const FieldMapping& mapping = fieldMappingTbl[j];
                    if( fieldName == mapping.iElementName )
                        {
                        TInt rndLen = 0;
                        TPtrC content = fieldList[i]->Content();
                                                                        
//                        MCreatorRandomDataField::TRandomLengthType randomLenType = MCreatorRandomDataField::ERandomLengthUndefined;
                    
                        if( content == TPtrC(KEmpty) || content == TPtrC(KNullDesC) )
                            {
                        // random
							CleanupStack::PushL( field );
							QContactDetail cntDetail = field->CreateContactDetailL(iEngine,param,mapping.iDetail,mapping.iContext,mapping.iFieldCode, KErrNotFound );
                            if(!cntDetail.isEmpty())
                            	{
								field->AddFieldToParam( param, cntDetail ); //it will do "param->iContactFields.AppendL(field);"
                            	}
                            CleanupStack::Pop( field );
                            }
                        else
                            {
							CleanupStack::PushL( field );
                        	QContactDetail cntDetail = field->CreateContactDetailL(iEngine,param,mapping.iDetail,mapping.iContext,mapping.iFieldCode, content );
                            if(!cntDetail.isEmpty())
                            	{
                        		field->AddFieldToParam( param, cntDetail ); //it will do "param->iContactFields.AppendL(field);"
                                }
                            CleanupStack::Pop( field );
                            }

                        break;
                        
                        }
                    }

                }
            } 
        else
            {
            // No fields specified, so add all fields with random content:
            for( TInt i = 0; i < fieldMappingTblSize; ++i )
                {
                if( fieldMappingTbl[i].iElementName == KThumbnailId || 
                    fieldMappingTbl[i].iElementName == KRingToneId )
                    {
                    // Skip thumbnail and ringtone IDs, since they are duplicates with thumbnailpath and ringtonepath
                    continue;
                    }
                
                CCreatorContactField* field = CCreatorContactField::NewL();
                CleanupStack::PushL( field );
                QContactDetail cntDetail = field->CreateContactDetailL(iEngine,param,fieldMappingTbl[i].iDetail,fieldMappingTbl[i].iContext,fieldMappingTbl[i].iFieldCode, KErrNotFound );
                if(!cntDetail.isEmpty())
                	{
					field->AddFieldToParam( param, cntDetail ); //it will do "param->iContactFields.AppendL(field);"
                	}
                CleanupStack::Pop( field );
                }
            }
        iEngine->AppendToCommandArrayL(ECmdCreatePhoneBookEntryContacts, param);
        iParameters.AppendL(param); // Save the pointer to the parametes. 
        CleanupStack::Pop(param);
        }
    }


/*
void CCreatorContactElement::ExecuteCommandL()
    {}
*/
CCreatorContactSetElement* CCreatorContactSetElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactSetElement::NewL");
    CCreatorContactSetElement* self = new (ELeave) CCreatorContactSetElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactSetElement::CCreatorContactSetElement(CCreatorEngine* aEngine) 
: 
CCreatorContactElementBase(aEngine)
    { }

void CCreatorContactSetElement::ExecuteCommandL()
    { }

void CCreatorContactSetElement::AddToCacheL()
    {    
    LOGSTRING("Creator: CCreatorContactSetElement::AddToCacheL");
    const CCreatorScriptAttribute* linkIdAttr = this->FindAttributeByName(KId);
    const CCreatorScriptAttribute* existingElemsAttr = this->FindAttributeByName(KExistingContacts);
    TInt numOfExistingContacts = 0;
    if( existingElemsAttr )
        {
        numOfExistingContacts = ConvertStrToIntL(existingElemsAttr->Value());
        }
    if( linkIdAttr )
        {
        TInt linkId = ConvertStrToIntL(linkIdAttr->Value());
        if( linkId > 0 )
            {    
            CCreatorContactSet* newSet = CCreatorContactSet::NewL(linkId, numOfExistingContacts);
            CleanupStack::PushL(newSet);
            ContactLinkCache::Instance()->AppendL(newSet);
            CleanupStack::Pop(newSet);
                 
            // Mark sub-elements (i.e. contacts) to this contact-set:
            for( TInt i = 0; i < iSubElements.Count(); ++i )
                {
                for( TInt j = 0; j < iSubElements[i]->CommandParameters().Count(); ++j)
                    {
                    CCreatorModuleBaseParameters* params = iSubElements[i]->CommandParameters()[j];
                    if( params )
                        {
                        params->SetScriptLinkId(linkId);
                        }
                    }
                }
            }
        }
    }

CCreatorContactGroupElement* CCreatorContactGroupElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactGroupElement::NewL");
    CCreatorContactGroupElement* self = new (ELeave) CCreatorContactGroupElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }
CCreatorContactGroupElement::CCreatorContactGroupElement(CCreatorEngine* aEngine)
: 
CCreatorContactElementBase(aEngine)
    { 
    iIsCommandElement = ETrue;
    }


void CCreatorContactGroupElement::ExecuteCommandL()
    {
    LOGSTRING("Creator: CCreatorContactGroupElement::ExecuteCommandL");
    // Get attributes (amount and name)
    const CCreatorScriptAttribute* groupAmountAttr = this->FindAttributeByName(KAmount);        
    const CCreatorScriptAttribute* groupNameAttr = this->FindAttributeByName(KName);
    // How many groups to create:
    TInt groupAmount = 1;        
    if( groupAmountAttr )
        {
        groupAmount = ConvertStrToIntL(groupAmountAttr->Value());
        }
    
    for( TInt i = 0; i < groupAmount; ++i )
        {        
        CCreatorScriptElement* membersElement = FindSubElement(KMembers);
        
        if( membersElement )
            {    
            CPhonebookParameters* param = (CPhonebookParameters*) TCreatorFactory::CreatePhoneBookParametersL();            
            CleanupStack::PushL(param);
            
            // Links to contact-sets:
            const RPointerArray<CCreatorScriptElement>& linkList = membersElement->SubElements();
            if( linkList.Count() > 0 )
                {
                for( TInt i = 0; i < linkList.Count(); ++i )
                    {
                    AppendContactSetReferenceL(*linkList[i], param->iLinkIds);
                    }                
                }
            
            if( groupNameAttr )
                {
                param ->iGroupName = QString::fromUtf16(groupNameAttr->Value().Ptr(), groupNameAttr->Value().Length()); 	//param->iGroupName->Des().Copy( groupNameAttr->Value() );
                }
            else
                {
                param->iGroupName =   QString::fromUtf16( iEngine->RandomString(CCreatorEngine::EGroupName).Ptr(), iEngine->RandomString(CCreatorEngine::EGroupName).Length()); 	//->Des().Copy( iEngine->RandomString(CCreatorEngine::EGroupName) );
                }                
            iEngine->AppendToCommandArrayL(ECmdCreatePhoneBookEntryGroups, param);
            CleanupStack::Pop(param);
            }
        }
    }

/*
void CCreatorContactGroupElement::ExecuteCommandL()
    {}
*/
CCreatorContactFieldElement* CCreatorContactFieldElement::NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext)
    {
    LOGSTRING("Creator: CCreatorContactFieldElement::NewL");
    CCreatorContactFieldElement* self = new (ELeave) CCreatorContactFieldElement(aEngine);
    CleanupStack::PushL(self);
    self->ConstructL(aName, aContext);
    CleanupStack::Pop();
    return self;
    }

CCreatorContactFieldElement::CCreatorContactFieldElement(CCreatorEngine* aEngine) 
: 
CCreatorScriptElement(aEngine)
    { }
