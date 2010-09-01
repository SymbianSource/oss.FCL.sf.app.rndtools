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
#include "SGDocument.h"
#include "SGAppUi.h"
#include "SGModel.h"

// ================= MEMBER FUNCTIONS =======================

// constructor
CScreenGrabberDocument::CScreenGrabberDocument(CEikApplication& aApp)
: CAknDocument(aApp)    
    {
    }

// destructor
CScreenGrabberDocument::~CScreenGrabberDocument()
    {
    delete iModel;
    }

// EPOC default constructor can leave.
void CScreenGrabberDocument::ConstructL()
    {
    iModel = CScreenGrabberModel::NewL();
    }

// Two-phased constructor.
CScreenGrabberDocument* CScreenGrabberDocument::NewL(
        CEikApplication& aApp)     // CScreenGrabberApp reference
    {
    CScreenGrabberDocument* self = new (ELeave) CScreenGrabberDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }
    
// ----------------------------------------------------
// CScreenGrabberDocument::CreateAppUiL()
// constructs CScreenGrabberAppUi
// ----------------------------------------------------
//
CEikAppUi* CScreenGrabberDocument::CreateAppUiL()
    {
    return new (ELeave) CScreenGrabberAppUi;
    }

// ----------------------------------------------------
// CScreenGrabberDocument::OpenFileL
// Overrides CAknDocument::OpenFileL to support document file
// ----------------------------------------------------
//
CFileStore* CScreenGrabberDocument::OpenFileL(TBool aDoOpen,const TDesC& aFilename,RFs& aFs)
    {
    return CEikDocument::OpenFileL(aDoOpen, aFilename, aFs);
    }


// ----------------------------------------------------

CScreenGrabberModel* CScreenGrabberDocument::Model()
    {
    return iModel;
    }

// ----------------------------------------------------

// End of File  
