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



#ifndef CREATORIMPSSERVERELEMENT_H_
#define CREATORIMPSSERVERELEMENT_H_

#ifdef __PRESENCE

#include "creator_scriptelement.h"

namespace creatorimps
{
    _LIT(KImpsServer, "impsserver");
    _LIT(KName, "name");
    _LIT(KUrl, "url");
    _LIT(KUsername, "username");
    _LIT(KPassword, "password");
    _LIT(KConnectionMethodName, "connectionmethodname");
    _LIT(KDefault, "default");
}

class CCreatorImpsServerElement : public CCreatorScriptElement
{
public:
    static CCreatorImpsServerElement* NewL(CCreatorEngine* aEngine, const TDesC& aName, const TDesC& aContext = KNullDesC);
    void ExecuteCommandL();
    
protected:
    CCreatorImpsServerElement(CCreatorEngine* aEngine);
};

#endif // __PRESENCE

#endif /*CREATORIMPSSERVERELEMENT_H_*/
