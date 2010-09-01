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
#include    "SGApp.h"
#include    "SGDocument.h"

#include <eikstart.h>


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CScreenGrabberApp::AppDllUid()
// Returns application UID
// ---------------------------------------------------------
//
TUid CScreenGrabberApp::AppDllUid() const
    {
    return KUidScreenGrabber;
    }

// ---------------------------------------------------------
// CDictionaryStore* CScreenGrabberApp::OpenIniFileLC(RFs& aFs) const
// overrides CAknApplication::OpenIniFileLC to enable INI file support
// ---------------------------------------------------------
//
CDictionaryStore* CScreenGrabberApp::OpenIniFileLC(RFs& aFs) const
{
    return CEikApplication::OpenIniFileLC(aFs);
}
   
// ---------------------------------------------------------
// CScreenGrabberApp::CreateDocumentL()
// Creates CScreenGrabberDocument object
// ---------------------------------------------------------
//
CApaDocument* CScreenGrabberApp::CreateDocumentL()
    {
    return CScreenGrabberDocument::NewL( *this );
    }

// ================= OTHER EXPORTED FUNCTIONS ==============

LOCAL_C CApaApplication* NewApplication()
    {
    return new CScreenGrabberApp;
    }


GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication(NewApplication);
    }
   

// End of File  

