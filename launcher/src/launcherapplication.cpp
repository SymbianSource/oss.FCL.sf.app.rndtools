/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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


#include "LauncherDocument.h"
#include "LauncherApplication.h"

// local constants
static const TUid KUidLauncherApp = {0x101FB74F}; 

CApaDocument* CLauncherApplication::CreateDocumentL()
    {  
    // Create an Launcher document, and return a pointer to it
    CApaDocument* document = CLauncherDocument::NewL(*this);
    return document;
    }

TUid CLauncherApplication::AppDllUid() const
    {
    // Return the UID for the Launcher application
    return KUidLauncherApp;
    }

