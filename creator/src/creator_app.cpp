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


#include "creator_document.h"
#include "creator_app.h"

CApaDocument* CCreatorApplication::CreateDocumentL()
    {  
    // Create an Creator document, and return a pointer to it
    CApaDocument* document = CCreatorDocument::NewL(*this);
    return document;
    }

TUid CCreatorApplication::AppDllUid() const
    {
    // Return the UID for the Creator application
    return KUidCreatorApp;
    }

