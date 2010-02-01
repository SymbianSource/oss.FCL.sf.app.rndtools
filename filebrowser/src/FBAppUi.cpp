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
#include "FBAppUi.h"
#include "FBFileListView.h"
#include "FB.hrh"
#include "FBModel.h"
#include "FBDocument.h"
#include <filebrowser.rsg>

#include <avkon.hrh>
#include <AknQueryDialog.h>

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CFileBrowserAppUi::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CFileBrowserAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin);

    iModel = static_cast<CFileBrowserDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();

    CFileBrowserFileListView* fileListView = new (ELeave) CFileBrowserFileListView;
    CleanupStack::PushL( fileListView );
    fileListView->ConstructL();
    AddViewL( fileListView );           // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();                // fileListView

    SetDefaultViewL(*fileListView);

    // notify the model that everything has been constructed
    iModel->ActivateModelL();
    }

// ----------------------------------------------------
// CFileBrowserAppUi::~CFileBrowserAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CFileBrowserAppUi::~CFileBrowserAppUi()
    {
    // notify the model that app ui will be deconstructed
    if (iModel)
        TRAP_IGNORE(iModel->DeActivateModelL());
    }

// ------------------------------------------------------------------------------
// CFileBrowserAppUi::::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CFileBrowserAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/)
    {
    }

// ----------------------------------------------------
// CFileBrowserAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// ?implementation_description
// ----------------------------------------------------
//
TKeyResponse CFileBrowserAppUi::HandleKeyEventL(
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
    {
    return EKeyWasNotConsumed;
    }

// ----------------------------------------------------
// CFileBrowserAppUi::HandleCommandL(TInt aCommand)
// ?implementation_description
// ----------------------------------------------------
//
void CFileBrowserAppUi::HandleCommandL(TInt aCommand)
    {
    switch ( aCommand )
        {
        // a normal way to close an application
        case EAknCmdExit:
        case EEikCmdExit: 
			{
            Exit();
			}
            break;

        default:
            break;      
        }
    }

// ----------------------------------------------------
// CFileBrowserAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
// ?implementation_description
// ----------------------------------------------------
//
void CFileBrowserAppUi::HandleWsEventL(const TWsEvent& aEvent, CCoeControl* aDestination)
    {
    //if (iModel->ScreenCapture()->HandleKeyCaptureEventsL(aEvent))
      	CAknViewAppUi::HandleWsEventL(aEvent, aDestination);  //continue the event loop if needed
    }

// End of File  
