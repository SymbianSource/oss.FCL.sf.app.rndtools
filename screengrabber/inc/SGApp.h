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



#ifndef SCREENGRABBERAPP_H
#define SCREENGRABBERAPP_H


// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidScreenGrabber = { 0x101FB751 };

// CLASS DECLARATION

/**
* CScreenGrabberApp application class.
* Provides factory to create concrete document object.
* 
*/
class CScreenGrabberApp : public CAknApplication
    {
    
    public: // Functions from base classes
        /**
        * From CApaApplication, overridden to enable INI file support.
        * @return A pointer to the dictionary store
        */
    CDictionaryStore* OpenIniFileLC(RFs& aFs) const;
    private:

        /**
        * From CApaApplication, creates CScreenGrabberDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID (KUidScreenGrabber).
        * @return The value of KUidScreenGrabber.
        */
        TUid AppDllUid() const;
    };

#endif

// End of File

