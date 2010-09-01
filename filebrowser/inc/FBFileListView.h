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


#ifndef FILEBROWSERMAINVIEW_H
#define FILEBROWSERMAINVIEW_H

// INCLUDES
#include <aknview.h>
#include <remconcoreapitargetobserver.h>

#include "FBStd.h"



// CONSTANTS
// UID of view
const TUid KFileListViewUID = {1};


// FORWARD DECLARATIONS
class CFileBrowserFileListContainer;
class CFileBrowserModel;
class CRemConInterfaceSelector;
class CRemConCoreApiTarget;


// CLASS DECLARATION

/**
*  CFileBrowserFileListView view class.
* 
*/
class CFileBrowserFileListView : public CAknView, public MRemConCoreApiTargetObserver
    {
    public: // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CFileBrowserFileListView();

    public: // Functions from base classes
        
        /**
        * From ?base_class ?member_description
        */
        TUid Id() const;

        /**
        * From ?base_class ?member_description
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From ?base_class ?member_description
        */
        void HandleClientRectChange();

    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);

    private:
        // From MRemConCoreApiTargetObserver
        void MrccatoCommand(TRemConCoreApiOperationId aOperationId, TRemConCoreApiButtonAction aButtonAct);
        
    private:

        /**
        * From AknView, ?member_description
        */
        void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
            const TDesC8& aCustomMessage);

        /**
        * From AknView, ?member_description
        */
        void DoDeactivate();

    private: // Data
        CFileBrowserFileListContainer*  iContainer;
        CFileBrowserModel*              iModel;
        CRemConInterfaceSelector*       iRemConSelector;
        CRemConCoreApiTarget*           iRemConTarget;        
    };

#endif

// End of File
