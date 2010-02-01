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


#ifndef LAUNCHERCONTAINERAPPS_H
#define LAUNCHERCONTAINERAPPS_H

// INCLUDES
#include <coecntrl.h>
#include <aknlists.h>
#include <eiklbo.h> 
   
// FORWARD DECLARATIONS
class CLauncherEngine;

// CLASS DECLARATION

/**
*  CLauncherContainerApps  container control class.
*  
*/
class CLauncherContainerApps : public CCoeControl, MCoeControlObserver, MEikListBoxObserver
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect, CLauncherEngine* aEngine);

        /**
        * Destructor.
        */
        ~CLauncherContainerApps();

    public: // New functions

    public: // Functions from base classes

    public: // From MEikListBoxObserver
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
    
    private: // Functions from base classes

        /**
        * From CoeControl.
        */
        void SizeChanged();  
    
        /**
        * From CoeControl.
        */
        TInt CountComponentControls() const;
    
        /**
        * From CCoeControl.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;
    
        /**
        * Event handling section, e.g Listbox events.
        */
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);

        /**
        * From CCoeControl,Draw.
        */
        void Draw(const TRect& aRect) const;
    
        /**
        * Key event handler. Handles up and down arrow keys, so that
        * output window can be scrolled.
        */
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

    public:
        void HandleResourceChange(TInt aType);

    public:    
        const CArrayFix<TInt>* SelectedApps();
        void UpdateFileListL();
        void MarkCurrentItemL();
        void UnmarkCurrentItem();
        TInt NumberOfSelectedItems() const;
        void MarkAllItemsL();
        void UnmarkAllItems();
        
        CEikListBox* ListBox() { return iListBox; }
        CLauncherEngine* Engine() { return iEngine; }


            
    private: //data
        CDesCArray* iSelectedItems;
        CAknSingleStyleListBox* iListBox;

        CLauncherEngine* iEngine;
    
    };

#endif

// End of File
