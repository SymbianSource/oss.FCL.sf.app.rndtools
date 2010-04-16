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


#include "creator_factory.h"

#if SYMBIAN_VERSION_SUPPORT >= SYMBIAN_4
    #include "creator_accessPoint.h"
#elif( defined __SERIES60_30__ || defined __SERIES60_31__ )
    #include "creator_phonebook.h"
    #include "creator_accessPoint.h"
#else
    #include "creator_virtualphonebook.h"
    #include "creator_connectionmethod.h"
#endif 

CCreatorPhonebookBase* TCreatorFactory::CreatePhoneBookL(CCreatorEngine* aEngine)
{
#if SYMBIAN_VERSION_SUPPORT >= SYMBIAN_4
	; //TODO return an object
#elif( defined __SERIES60_30__ || defined __SERIES60_31__ )
    return CCreatorPhonebook::NewL(aEngine);
#else
    return CCreatorVirtualPhonebook::NewL(aEngine);
#endif 

}

CCreatorModuleBaseParameters* TCreatorFactory::CreatePhoneBookParametersL()
	{
#if SYMBIAN_VERSION_SUPPORT >= SYMBIAN_4
	; //TODO return an object
#elif( defined __SERIES60_30__ || defined __SERIES60_31__ )
	return new(ELeave) CPhonebookParameters;
#else
	return new(ELeave) CVirtualPhonebookParameters;
#endif 	
	}

CCreatorConnectionSettingsBase* TCreatorFactory::CreateConnectionSettingsL(CCreatorEngine* aEngine)
{
#if( defined __SERIES60_30__ || defined __SERIES60_31__ || SYMBIAN_VERSION_SUPPORT >= SYMBIAN_4 )
    return CCreatorAccessPoints::NewL(aEngine);
#else
    return CCreatorConnectionSettings::NewL(aEngine);    
#endif 

}

CCreatorModuleBaseParameters* TCreatorFactory::CreateConnectionSettingsParametersL()
    {
#if( defined __SERIES60_30__ || defined __SERIES60_31__ || SYMBIAN_VERSION_SUPPORT >= SYMBIAN_4 )
    return new(ELeave) CAccessPointsParameters;
#else
    return new(ELeave) CConnectionSettingsParameters;
#endif 
    }
