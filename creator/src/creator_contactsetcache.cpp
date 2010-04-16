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


#include "creator_contactsetcache.h"


CContactLinkCacheImp* ContactLinkCache::iImp = 0;

CCreatorContactSet* CCreatorContactSet::NewL(TInt aLinkId, TInt aNumOfExistingContacts)
    {
    return new (ELeave) CCreatorContactSet(aLinkId, aNumOfExistingContacts);    
    }

TInt CCreatorContactSet::LinkId() const
    {
    return iLinkId;
    }

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && SYMBIAN_VERSION_SUPPORT < SYMBIAN_4)
void CCreatorContactSet::AppendL(MVPbkContactLink* aContactLink)
    {
    iContactLinks.AppendL(aContactLink);
    }

RPointerArray<MVPbkContactLink>& CCreatorContactSet::ContactLinks()
    {
    return iContactLinks;
    }

const RPointerArray<MVPbkContactLink>& CCreatorContactSet::ContactLinks() const
    {
    return iContactLinks;
    }


TInt CCreatorContactSet::NumberOfExistingContacts() const
    {
    return iNumOfExistingContacts;
    }

CCreatorContactSet::~CCreatorContactSet()
    {
    iContactLinks.Reset();
    iContactLinks.Close();
    }

#else
CCreatorContactSet::~CCreatorContactSet()
    {
    }
#endif


CCreatorContactSet::CCreatorContactSet(TInt aLinkId, TInt aNumOfExistingContacts)
    {
    iLinkId = aLinkId;
    iNumOfExistingContacts = aNumOfExistingContacts;
    }

class CContactLinkCacheImp : public CBase, public MContactLinkCache
{
public:
    static CContactLinkCacheImp* NewL();
    virtual ~CContactLinkCacheImp();
    virtual void AppendL(CCreatorContactSet* aContactSet);
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && SYMBIAN_VERSION_SUPPORT < SYMBIAN_4)
    virtual RPointerArray<MVPbkContactLink>& ContactLinks(TInt aLinkId);
    virtual const RPointerArray<MVPbkContactLink>& ContactLinks(TInt aLinkId) const;
#endif        
    virtual RPointerArray<CCreatorContactSet>& ContactSets();
    virtual const RPointerArray<CCreatorContactSet>& ContactSets() const;
    virtual const CCreatorContactSet& ContactSet(TInt aLinkId) const;
    virtual CCreatorContactSet& ContactSet(TInt aLinkId);
    
private:
    void ConstructL();
    CContactLinkCacheImp();
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && SYMBIAN_VERSION_SUPPORT < SYMBIAN_4)    
    RPointerArray<MVPbkContactLink> iEmptyLinks;
#endif    
    RPointerArray<CCreatorContactSet> iContactSets;
    CCreatorContactSet* iDummyContactSet;
};

CContactLinkCacheImp* CContactLinkCacheImp::NewL()
    {
    CContactLinkCacheImp* self = new (ELeave) CContactLinkCacheImp();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

void CContactLinkCacheImp::ConstructL()
    {
    if( iDummyContactSet == 0 )
        iDummyContactSet = CCreatorContactSet::NewL(-1, 0);    
    }

CContactLinkCacheImp::CContactLinkCacheImp()
: iDummyContactSet(0)
    {}

CContactLinkCacheImp::~CContactLinkCacheImp()
    {
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && SYMBIAN_VERSION_SUPPORT < SYMBIAN_4)    
    iEmptyLinks.ResetAndDestroy(); // just in case...
    iEmptyLinks.Close();
#endif
    iContactSets.ResetAndDestroy();
    iContactSets.Close();
    delete iDummyContactSet;
    }
void CContactLinkCacheImp::AppendL(CCreatorContactSet* aContactSet)
    {
    iContactSets.AppendL(aContactSet);
    }

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && SYMBIAN_VERSION_SUPPORT < SYMBIAN_4)

RPointerArray<MVPbkContactLink>& CContactLinkCacheImp::ContactLinks(TInt aLinkId)
    {
    for( TInt i = 0; i < iContactSets.Count(); ++i )
        {
        if( iContactSets[i]->LinkId() == aLinkId )
            {
            return iContactSets[i]->ContactLinks();
            }
        }
    return iEmptyLinks;
    }

const RPointerArray<MVPbkContactLink>& CContactLinkCacheImp::ContactLinks(TInt aLinkId) const
    {
    for( TInt i = 0; i < iContactSets.Count(); ++i )
        {
        if( iContactSets[i]->LinkId() == aLinkId )
            {
            return iContactSets[i]->ContactLinks();
            }
        }
    return iEmptyLinks;
    }

#endif

const CCreatorContactSet& CContactLinkCacheImp::ContactSet(TInt aLinkId) const
    {
    for( TInt i = 0; i < iContactSets.Count(); ++i )
        {
        if( iContactSets[i]->LinkId() == aLinkId )
            {
            return *iContactSets[i];
            }
        }
        
    return *iDummyContactSet;
    }

CCreatorContactSet& CContactLinkCacheImp::ContactSet(TInt aLinkId)
    {
    for( TInt i = 0; i < iContactSets.Count(); ++i )
        {
        if( iContactSets[i]->LinkId() == aLinkId )
            {
            return *iContactSets[i];
            }
        }
        
    return *iDummyContactSet;
    }

RPointerArray<CCreatorContactSet>& CContactLinkCacheImp::ContactSets()
    {
    return iContactSets;
    }

const RPointerArray<CCreatorContactSet>& CContactLinkCacheImp::ContactSets() const
    {
    return iContactSets;
    }


void ContactLinkCache::InitializeL()
    {
    if( iImp == 0 )
        {
        iImp = CContactLinkCacheImp::NewL();
        }
    }

void ContactLinkCache::DestroyL()
    {
    delete iImp;
    iImp = 0;
    }

MContactLinkCache* ContactLinkCache::Instance()
    {
    return iImp;
    }


    /*{
public:
    static void ;
    static void DestructLD();
    
    static MContactLinkCache* Instance();
    
private:
    CContactLinkImp* iImp;
};*/

