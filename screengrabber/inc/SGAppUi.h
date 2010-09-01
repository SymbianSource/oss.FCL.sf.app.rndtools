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



#ifndef SCREENGRABBERAPPUI_H
#define SCREENGRABBERAPPUI_H

// INCLUDES
#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknViewAppUi.h>
#include <akntabgrp.h>
#include <aknnavide.h>
#include "SGStd.h"

// FORWARD DECLARATIONS
class CScreenGrabberMainContainer;
class CScreenGrabberModel;

// CONSTANTS
//const ?type ?constant_var = ?constant;


// CLASS DECLARATION


class CScreenGrabberAppUi : public CAknViewAppUi
    {
    public: // // Constructors and destructor

        /**
        * EPOC default constructor.
        */      
        void ConstructL();

        /**
        * Destructor.
        */      
        ~CScreenGrabberAppUi();
        
    public: // New functions

    public: // Functions from base classes
        void HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination);

    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);

    private:
        /**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event. 
        * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed). 
        */
        virtual TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType);

    private: //Data
        CScreenGrabberModel*    iModel;

    };

#endif

// End of File
