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



#ifndef SCREENGRABBERSETTINGLISTCONTAINER_H
#define SCREENGRABBERSETTINGLISTCONTAINER_H


// INCLUDES
#include <aknsettingitemlist.h>
#include "SGModel.h"
 
// FORWARD DECLARATIONS
class CScreenGrabberModel;

// CLASS DECLARATION

/**
*  CScreenGrabberSettingListContainer  container control class.
*  
*/
class CScreenGrabberSettingListContainer : public CAknSettingItemList
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        ~CScreenGrabberSettingListContainer();

    public: // New functions
        void ShowSettingPageL(TInt aCalledFromMenu);

    public: // Functions from base classes
        void HandleResourceChange(TInt aType);

    private: // Functions from base classes

        CAknSettingItem* CreateSettingItemL( TInt aIdentifier );
        void HandleListBoxEventL(CEikListBox *aListBox, TListBoxEvent aEventType);

    private:
        void UpdateSettingsL();
        void SetVisibilitiesOfSettingItems(); 

    private: //data
        CScreenGrabberModel*    iModel;
        TGrabSettings           iGrabSettings;


    };

#endif

// End of File
