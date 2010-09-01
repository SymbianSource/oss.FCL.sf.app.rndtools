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


// INCLUDE FILES
#include "FBDocument.h"
#include "FBAppUi.h"
#include "FBModel.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
CFileBrowserDocument::CFileBrowserDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
    {
    }

// destructor
CFileBrowserDocument::~CFileBrowserDocument()
    {
    delete iModel;
    }

// EPOC default constructor can leave.
void CFileBrowserDocument::ConstructL()
    {
    iModel = CFileBrowserModel::NewL();
    }

// Two-phased constructor.
CFileBrowserDocument* CFileBrowserDocument::NewL(
        CEikApplication& aApp)     // CFileBrowserApp reference
    {
    CFileBrowserDocument* self = new (ELeave) CFileBrowserDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }
    
// ----------------------------------------------------
// CFileBrowserDocument::CreateAppUiL()
// constructs CFileBrowserAppUi
// ----------------------------------------------------
//
CEikAppUi* CFileBrowserDocument::CreateAppUiL()
    {
    return new (ELeave) CFileBrowserAppUi;
    }

// ----------------------------------------------------
// CFileBrowserDocument::OpenFileL
// Overrides CAknDocument::OpenFileL to support document file
// ----------------------------------------------------
//
CFileStore* CFileBrowserDocument::OpenFileL(TBool aDoOpen,const TDesC& aFilename,RFs& aFs)
    {
    return CEikDocument::OpenFileL(aDoOpen, aFilename, aFs);
    }


// ----------------------------------------------------

CFileBrowserModel* CFileBrowserDocument::Model()
    {
    return iModel;
    }

// ----------------------------------------------------

// End of File  
