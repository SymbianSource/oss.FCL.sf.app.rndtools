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




#ifndef __CREATORIMPS_H__
#define __CREATORIMPS_H__

#ifdef __PRESENCE

#include "creator_model.h"
#include "creator_modulebase.h"

#include <e32base.h>

#include <cimpssapsettings.h>
#include <cimpssapsettingsstore.h>


class CCreatorEngine;
class CIMPSParameters;


class CCreatorIMPS : public CBase, public MCreatorModuleBase
    {
public: 
    static CCreatorIMPS* NewL(CCreatorEngine* aEngine);
    static CCreatorIMPS* NewLC(CCreatorEngine* aEngine);
    ~CCreatorIMPS();

private:
    CCreatorIMPS();
    void ConstructL(CCreatorEngine* aEngine); // from MCreatorModuleBase

public:
    TBool AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries); // from MCreatorModuleBase
    TInt CreateIMPSServerEntryL(CIMPSParameters *aParameters);
    void DeleteAllL();
    void DeleteAllCreatedByCreatorL();

private:
    CIMPSSAPSettingsStore* iIMPSSAPSettingsStore;

    CIMPSParameters* iParameters;
    RArray<TUint32> iEntryIds; // TUint32 CIMPSSAPSettings::Uid()

public:
    };


class CIMPSParameters : public CCreatorModuleBaseParameters
    {
public: 
    HBufC*              iServerName;
    HBufC*              iServerURL;
    HBufC*              iServerUsername;
    HBufC*              iServerPassword;    
    HBufC*              iServerAccessPointName;
    
    void SetRandomNameL(CCreatorEngine& aEngine);
    void SetRandomUrlL(CCreatorEngine& aEngine);
    void SetRandomUsernameL(CCreatorEngine& aEngine);
    void SetRandomPasswordL(CCreatorEngine& aEngine);
    void SetDefaultAccessPointL();

public:
    CIMPSParameters();
    ~CIMPSParameters();
    };

#endif //__PRESENCE

#endif // __CREATORIMPS_H__
