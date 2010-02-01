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



#ifndef SCREENGRABBERSETTINGLISTVIEW_H
#define SCREENGRABBERSETTINGLISTVIEW_H

// INCLUDES
#include <aknview.h>
#include "SGStd.h"


// CONSTANTS
// UID of view
const TUid KSettingListViewUID = {2};

// FORWARD DECLARATIONS
class CScreenGrabberSettingListContainer;

// CLASS DECLARATION

/**
*  CScreenGrabberSettingListView view class.
* 
*/
class CScreenGrabberSettingListView : public CAknView
    {
    public: // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CScreenGrabberSettingListView();

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
        CScreenGrabberSettingListContainer* iContainer;
    };

#endif

// End of File
