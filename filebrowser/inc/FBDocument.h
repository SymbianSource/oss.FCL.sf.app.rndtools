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


#ifndef FILEBROWSERDOCUMENT_H
#define FILEBROWSERDOCUMENT_H

// INCLUDES
#include <AknDoc.h>
   
// CONSTANTS

// FORWARD DECLARATIONS
class CEikAppUi;
class CFileBrowserModel;


// CLASS DECLARATION

/**
*  CFileBrowserDocument application class.
*/
class CFileBrowserDocument : public CAknDocument
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CFileBrowserDocument* NewL(CEikApplication& aApp);

        /**
        * Destructor.
        */
        virtual ~CFileBrowserDocument();

    public: // New functions

    public:	// from CEikDocument
	CFileStore* OpenFileL(TBool aDoOpen,const TDesC& aFilename,RFs& aFs);
    protected:  // New functions

    protected:  // Functions from base classes

    private:

        /**
        * EPOC default constructor.
        */
        CFileBrowserDocument(CEikApplication& aApp);
        void ConstructL();

    private:

        /**
        * From CEikDocument, create CFileBrowserAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();

    public:
        CFileBrowserModel* Model();

    private:
        CFileBrowserModel* iModel;

    };

#endif

// End of File

