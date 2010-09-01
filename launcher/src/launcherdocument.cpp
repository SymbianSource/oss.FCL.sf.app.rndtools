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



#include "launcherappui.h"
#include "launcherdocument.h"


// Standard Symbian OS construction sequence
CLauncherDocument* CLauncherDocument::NewL(CEikApplication& aApp)
    {
    CLauncherDocument* self = NewLC(aApp);
    CleanupStack::Pop(self);
    return self;
    }

CLauncherDocument* CLauncherDocument::NewLC(CEikApplication& aApp)
    {
    CLauncherDocument* self = new (ELeave) CLauncherDocument(aApp);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

void CLauncherDocument::ConstructL()
    {
    // Add any construction that can leave here
    }    

CLauncherDocument::CLauncherDocument(CEikApplication& aApp) : CAknDocument(aApp) 
    {
    // Add any construction that can not leave here
    }   

CLauncherDocument::~CLauncherDocument()
    {
    // Any destruction code here
    }

CEikAppUi* CLauncherDocument::CreateAppUiL()
    {
    // Create the application user interface, and return a pointer to it
    CEikAppUi* appUi = new (ELeave) CLauncherAppUi;
    return appUi;
    }

