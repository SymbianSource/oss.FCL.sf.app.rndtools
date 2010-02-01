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



#ifndef SCREENGRABBERDOCUMENT_H
#define SCREENGRABBERDOCUMENT_H

// INCLUDES
#include <AknDoc.h>
   
// CONSTANTS

// FORWARD DECLARATIONS
class CEikAppUi;
class CScreenGrabberModel;


// CLASS DECLARATION

/**
*  CScreenGrabberDocument application class.
*/
class CScreenGrabberDocument : public CAknDocument
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CScreenGrabberDocument* NewL(CEikApplication& aApp);

        /**
        * Destructor.
        */
        virtual ~CScreenGrabberDocument();

    public: // New functions

    public:	// from CEikDocument
	CFileStore* OpenFileL(TBool aDoOpen,const TDesC& aFilename,RFs& aFs);
    protected:  // New functions

    protected:  // Functions from base classes

    private:

        /**
        * EPOC default constructor.
        */
        CScreenGrabberDocument(CEikApplication& aApp);
        void ConstructL();

    private:

        /**
        * From CEikDocument, create CScreenGrabberAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();

    public:
        CScreenGrabberModel* Model();

    private:
        CScreenGrabberModel* iModel;

    };

#endif

// End of File

