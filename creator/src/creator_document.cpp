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



#include "creator_appui.h"
#include "creator_document.h"


// Standard Symbian OS construction sequence
CCreatorDocument* CCreatorDocument::NewL(CEikApplication& aApp)
    {
    CCreatorDocument* self = NewLC(aApp);
    CleanupStack::Pop(self);
    return self;
    }

CCreatorDocument* CCreatorDocument::NewLC(CEikApplication& aApp)
    {
    CCreatorDocument* self = new (ELeave) CCreatorDocument(aApp);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

void CCreatorDocument::ConstructL()
    {
    // Add any construction that can leave here
    }    

CCreatorDocument::CCreatorDocument(CEikApplication& aApp) : CAknDocument(aApp) 
    {
    // Add any construction that can not leave here
    }   

CCreatorDocument::~CCreatorDocument()
    {
    // Any destruction code here
    }

CEikAppUi* CCreatorDocument::CreateAppUiL()
    {
    // Create the application user interface, and return a pointer to it
    CEikAppUi* appUi = new (ELeave) CCreatorAppUi;
    return appUi;
    }

