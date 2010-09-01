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


#ifndef FILEBROWSERTOOLBAR_H
#define FILEBROWSERTOOLBAR_H


#include <aknappui.h>
#include <akntoolbarobserver.h>
#include <coecobs.h>    // MCoeControlObserver

#ifdef RD_CALEN_ENHANCED_MSK
#include <BADESCA.H> // for CDesCArrayFlat 
#endif

//Forward declarations 
class CAknToolbar;
class CAknButton;
class CCoeControl;
class CAknView;
class CFileBrowserModel;
class CAknInfoPopupNoteController;

class CFileBrowserToolbar : public CBase, 
                            public MAknToolbarObserver,
                            public MCoeControlObserver
    {
    public:
        /**
         * 1st phase constructor
         */
        static CFileBrowserToolbar* NewL();

        /**
         * 1st phase constructor
         */
        static CFileBrowserToolbar* NewLC();

        /**
         * Destructor
         */
        ~CFileBrowserToolbar();

        /**
         * 2nd phase constructor
         */
        void ConstructL();

    public:
        // from MAknToolbarObserver

        /**
         * From MAknToolbarObserver
         */
        void DynInitToolbarL( TInt aResourceId, CAknToolbar* aToolbar );

        /**
         * Handling toolbar key events
         */
        void OfferToolbarEventL( TInt aCommand );

    public:
        // from MCoeControlObserver

        /**
         * Handles toolbar event
         */
        void HandleControlEventL( CCoeControl* aControl, TCoeEvent aEventType );
    
    public:
        // New functions

        /**
         * Updates the toolbar state to default state
         */
        void ResetState();

        /**
         * Enables the display of toolbar
         */
        void ShowToolbarL();

        /**
         * Disables the display of toolbar
         */
        void HideToolbarL();

        /**
         * Returns whether the toolbar is shown or not
         */
        TBool IsShown();
        
        const TRect Rect() const;

    private:
        // New functions
        CFileBrowserToolbar();
        
    // private:
    public:
        CFileBrowserModel* iModel;
        CAknToolbar* iViewerToolbar;
        TBool iShown;
        TInt iFirstSelectionIndex;
        TInt iLastSelectionIndex;
        CAknInfoPopupNoteController* iInfoPopup;
        CAknButton* iSelectbutton;
    };

#endif // FILEBROWSERTOOLBAR_H

// End of File
