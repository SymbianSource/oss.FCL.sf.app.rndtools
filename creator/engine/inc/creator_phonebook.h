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





#ifndef __CREATORPHONEBOOK_H__
#define __CREATORPHONEBOOK_H__

#include "creator_phonebookbase.h"
#include "creator_randomdatafield.h"


//#include <qmobilityglobal.h> //defines Q_SFW_EXPORT
//#include <xqservicerequest.h>

#include <QString>
#include <qglobal.h>
#include <QTime>
#include <qtcontacts.h>
//#include <qcontactmanager.h>
//#include <qcontactname.h>
//#include <qcontactid.h>
//#include <qcontactphonenumber.h>
//#include <qcontacturl.h>
//#include <qcontactemailaddress.h>
//#include <qcontact.h>

//#include <e32base.h>
//#include <cntdef.h>
//#include <cntdb.h>
//#include <cntitem.h>
//#include <cntfldst.h>
//#include <cntview.h>
//#include <cvpbkcontactmanager.h>
//#include <pbkfields.hrh>
//#include <cpbk2imagemanager.h>
//#include <mvpbkcontactstorelistobserver.h>
//#include <mvpbkbatchoperationobserver.h>
//#include <mvpbkcontactfindobserver.h>
//#include <mvpbkcontactobserver.h>
//#include <mpbk2imageoperationobservers.h>
//#include <vpbkeng.rsg>
//#include <cvpbkcontactlinkarray.h>
//#include <MVPbkContactViewObserver.h>
QTM_USE_NAMESPACE

class CCreatorEngine;
class CCreatorModuleBaseParameters;

//class QContactManager;
//class QContact;
//class QContactId;
//class QContactData;
//class QContactName;

//class MVPbkStoreContact;  //to change
//class MVPbkContactStore;	//to change
//class CAsyncWaiter;			//to change - remove
class CContactDatabase;	
//class MVPbkContactLinkArray;	//to change
class CPhonebookParameters;	


class CCreatorPhonebook : public CCreatorPhonebookBase 
    {
public: 
    static CCreatorPhonebook* NewL(CCreatorEngine* aEngine);
    static CCreatorPhonebook* NewLC(CCreatorEngine* aEngine);
    ~CCreatorPhonebook();

private:
    CCreatorPhonebook();
    void ConstructL(CCreatorEngine* aEngine); // from MCreatorModuleBase

public:  
    TInt CreateContactEntryL(CCreatorModuleBaseParameters *aParameters);    
    TInt CreateGroupEntryL(CCreatorModuleBaseParameters *aParameters);
    TInt CreateSubscribedContactEntryL(CCreatorModuleBaseParameters *aParameters); 

    
    void DeleteAllL();
    void DeleteAllCreatedByCreatorL();
    void DeleteAllGroupsL();
    void DeleteAllGroupsCreatedByCreatorL();
    
private:
    
    void InitializeContactParamsL(/*CCreatorModuleBaseParameters* aParameters*/);
    TBool IsContactGroupL( QContactLocalId& aLink );
    void StoreLinksForDeleteL( RArray<TUint32>& aLinks, TUid aStoreUid );														//modify
    void DeleteContactsL( QList<QContactLocalId>& contacts /*MVPbkContactLinkArray* aContacts, TBool aGroup*/ );																//modify
    void DeleteItemsCreatedWithCreatorL( TUid aStoreUid );
    void DoDeleteItemsCreatedWithCreatorL( TUid aStoreUid, CDictionaryFileStore* aStore );
    TBool HasOtherThanGroupsL( /*MVPbkContactLinkArray* aContacts */);																			//modify
    

private:
	
    QContactManager* iContactMngr;//CVPbkContactManager* iContactManager;
    TInt iOpCounter;
    
    CPhonebookParameters* iParameters;
    
    static QString iPhoneNumberFields[];
    static TInt iUrlFields[];
    static TInt iEmailFields[];
    //QList<QContactLocalId> 
    RArray<TUint32> iContactLinkArray;//CVPbkContactLinkArray* iContactLinkArray;	//modify
    RArray<TUint32>  iContactsToDelete;	//CVPbkContactLinkArray* iContactsToDelete;	//modify
    RArray<TUint32>  iContactGroupsToDelete;	//CVPbkContactLinkArray* iContactGroupsToDelete;	//modify
    
    RArray<TUint32> iPreviousDeleteLinks;
    //RPointerArray<MVPbkContactLinkArray> iPreviousDeleteLinks;  //modify
    
private:
	//new variables
	/// Ref: the target of the copy
	QContact* iStore;     //MVPbkContactStore* iStore;  
    
    //CAsyncWaiter* iWaiter;	//remove
    
    /// Own: Contact database for this store
    CContactDatabase* iContactDb;    
    
    //Contacts found in contacts db.
    QList<QContactId>* iContactResults;//MVPbkContactLinkArray* iContactResults;
    // Contact groups that are found in the store. These are used in filtering
    // the groups from the find results.
    QList<QContactId>* iContactGroupsInStore;//MVPbkContactLinkArray* iContactGroupsInStore;
    
    };

/**
 * phonebook parameters
 */
    

class CPhonebookParameters : public CCreatorModuleBaseParameters
    {
public:
	void ParseL(CCommandParser* parser, TParseParams /*aCase = 0*/);
	
	TInt CPhonebookParameters::ScriptLinkId() const;
	void CPhonebookParameters::SetScriptLinkId(TInt aLinkId);

	QList<QContactDetail> iContactFields;//	RPointerArray<CCreatorContactField> iContactFields;

    QString iGroupName;//HBufC*  iGroupName;
    TInt iContactsInGroup;
    TInt iNumberOfPhoneNumberFields;
    TInt iNumberOfURLFields;
    TInt iNumberOfEmailAddressFields;
    TInt iContactSetPtr;
    RArray<TLinkIdParam> iLinkIds; //QList<QContactId> iLinkIds;// For contactgroup. Stores the linked contact ids.
    
public:
    CPhonebookParameters();
    ~CPhonebookParameters();
    
private:
    TInt iLinkId; // For contact. Stores the contact id
    };


class CCreatorContactField : public CBase//, public MCreatorRandomDataField
    {
public:    
    static CCreatorContactField* NewL();
    QContactDetail CreateContactDetailL(CCreatorEngine* aEngine,CPhonebookParameters* aParameters,QString aDetail, QString aFieldContext, QString aFieldString, TInt aRand = KErrNotFound );
    QContactDetail CreateContactDetailL(CCreatorEngine* aEngine,CPhonebookParameters* aParameters,QString aDetail, QString aFieldContext, QString aFieldString, TPtrC aData );
    void AddFieldToParam( CPhonebookParameters* aParam, QContactDetail aDetail);
    ~CCreatorContactField();
private:
    CCreatorContactField();
    void ConstructL();
    };

#endif // __CREATORPHONEBOOK_H__
