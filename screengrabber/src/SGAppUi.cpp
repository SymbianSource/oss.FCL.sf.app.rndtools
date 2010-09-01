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
#include "SGAppUi.h"
#include "SGMainView.h"
#include "SGSettingListView.h"
#include <screengrabber.rsg>
#include "SG.hrh"
#include "SGModel.h"
#include "SGDocument.h"

#include <avkon.hrh>
#include <AknQueryDialog.h>

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CScreenGrabberAppUi::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CScreenGrabberAppUi::ConstructL()
    {
    // disable window server priority control for this application
    iEikonEnv->WsSession().ComputeMode( RWsSession::EPriorityControlDisabled );

    BaseConstructL(EAknEnableSkin);

    iModel = static_cast<CScreenGrabberDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();

    CScreenGrabberMainView* view1 = new (ELeave) CScreenGrabberMainView;

    CleanupStack::PushL( view1 );
    view1->ConstructL();
    AddViewL( view1 );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // view1

    CScreenGrabberSettingListView* view2 = new (ELeave) CScreenGrabberSettingListView;

    CleanupStack::PushL( view2 );
    view2->ConstructL();
    AddViewL( view2 );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // view2

    SetDefaultViewL(*view1);

    // notify the model that everything has been constructed
    iModel->ActivateModelL();
    }

// ----------------------------------------------------
// CScreenGrabberAppUi::~CScreenGrabberAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CScreenGrabberAppUi::~CScreenGrabberAppUi()
    {
    // notify the model that app ui will be deconstructed
    if (iModel)
        TRAP_IGNORE(iModel->DeActivateModelL());
    }

// ------------------------------------------------------------------------------
// CScreenGrabberAppUi::::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CScreenGrabberAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
    {
    }

// ----------------------------------------------------
// CScreenGrabberAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// ?implementation_description
// ----------------------------------------------------
//
TKeyResponse CScreenGrabberAppUi::HandleKeyEventL(
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
    {
    return EKeyWasNotConsumed;
    }

// ----------------------------------------------------
// CScreenGrabberAppUi::HandleCommandL(TInt aCommand)
// ?implementation_description
// ----------------------------------------------------
//
void CScreenGrabberAppUi::HandleCommandL(TInt aCommand)
    {
    switch ( aCommand )
        {
        // a normal way to close an application
        case EAknCmdExit:
        case EEikCmdExit: 
			{
            CAknQueryDialog* exitQuery = CAknQueryDialog::NewL();
            exitQuery->SetPromptL(_L("Stop taking screen shots and exit?"));
            if (exitQuery->ExecuteLD(R_MY_GENERAL_CONFIRMATION_QUERY))
                {
                // pressed yes, exit
		        Exit();
                }
            else
                {
                // pressed no, do nothing
                }
			}
            break;

        default:
            break;      
        }
    }

// ----------------------------------------------------
// CScreenGrabberAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
// ?implementation_description
// ----------------------------------------------------
//
void CScreenGrabberAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
    {
    if (iModel->HandleCaptureCommandsL(aEvent))
      	CAknAppUi::HandleWsEventL(aEvent, aDestination);  //continue the event loop if needed
    }

// End of File  
