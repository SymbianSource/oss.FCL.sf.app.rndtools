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
#include "LauncherAppUi.h"
#include "LauncherViewApps.h"
#include "LauncherViewOutput.h"
#include <Launcher.rsg>
#include "launcher.hrh"

#include <avkon.hrh>
#include <aknnotewrappers.h>
#include <eikmenup.h>
#include <aknmessagequerydialog.h>


// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CLauncherAppUi::ConstructL()
// ?implementation_description
// ----------------------------------------------------------
//
void CLauncherAppUi::ConstructL()
    {
    BaseConstructL(EAknEnableSkin);

    iSendUi = CSendUi::NewL();

    // Show tabs for main views from resources
    CEikStatusPane* sp = StatusPane();

    // Fetch pointer to the default navi pane control
    iNaviPane = (CAknNavigationControlContainer*)sp->ControlL( 
        TUid::Uid(EEikStatusPaneUidNavi));

    // Tabgroup has been read from resource and it were pushed to the navi pane. 
    // Get pointer to the navigation decorator with the ResourceDecorator() function. 
    // Application owns the decorator and it has responsibility to delete the object.
    iDecoratedTabGroup = iNaviPane->ResourceDecorator();
    if (iDecoratedTabGroup)
        {
        iTabGroup = (CAknTabGroup*) iDecoratedTabGroup->DecoratedControl();
        }

    // init the engine class
    iEngine = CLauncherEngine::NewL(this);

    // init the apps view
    CLauncherViewApps* view1 = new (ELeave) CLauncherViewApps;
    CleanupStack::PushL( view1 );
    view1->ConstructL(iEngine);
    AddViewL( view1 );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // view1

    // init the output view
    CLauncherViewOutput* view2 = new (ELeave) CLauncherViewOutput;
    CleanupStack::PushL( view2 );
    view2->ConstructL(iEngine);
    AddViewL( view2 );      // transfer ownership to CAknViewAppUi
    CleanupStack::Pop();    // view2

    // set apps view as default
    SetDefaultViewL(*view1);

    }

// ----------------------------------------------------
// CLauncherAppUi::~CLauncherAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CLauncherAppUi::~CLauncherAppUi()
    {
    delete iEngine;

    delete iDecoratedTabGroup;

    delete iSendUi;
    }

// ------------------------------------------------------------------------------
// CLauncherAppUi::::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//
void CLauncherAppUi::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId == R_LAUNCHER_APP_MENU)
        {
        // launch stop check
        aMenuPane->SetItemDimmed(ECmdStopLaunch, !iEngine->LaunchingIsActive());

        // deleting log is possible only when the log exists
        aMenuPane->SetItemDimmed(ECmdDeleteLog, !(iEngine->LogFileExists() || iEngine->BCLogFileExists()));

        // SendUI for the log file
        if (iEngine->LogFileExists() || iEngine->BCLogFileExists())
            {
            TInt index(0);
            aMenuPane->ItemAndPos( ECmdDeleteLog, index );
            iSendUi->AddSendMenuItemL(*aMenuPane, index-1, ECmdSendUILogFile, TSendingCapabilities(0, 0, TSendingCapabilities::ESupportsAttachments));
            aMenuPane->SetItemTextL(ECmdSendUILogFile, _L("Send log(s)"));
            }
        }

    else if (aResourceId == R_LAUNCHER_DLLINFO_SUBMENU)
        {
        //SendUI for the system dll file
        TInt index(0);
        aMenuPane->ItemAndPos( ECmdCompareDLLs, index );

        iSendUi->AddSendMenuItemL(*aMenuPane, index, ECmdSendUISystemDllFile, TSendingCapabilities(0, 0, TSendingCapabilities::ESupportsAttachments));
        aMenuPane->SetItemTextL(ECmdSendUISystemDllFile, _L("Send list of DLLs"));
        aMenuPane->SetItemButtonState(ECmdSendUISystemDllFile, EEikMenuItemSymbolIndeterminate); 
        }
    }

// ----------------------------------------------------
// CLauncherAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// ----------------------------------------------------
//
TKeyResponse CLauncherAppUi::HandleKeyEventL(
    const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
    {
    if ( iTabGroup == NULL )
        {
        return EKeyWasNotConsumed;
        }

    TInt active = iTabGroup->ActiveTabIndex();
    TInt count = iTabGroup->TabCount();

    switch ( aKeyEvent.iCode )
        {
        case EKeyLeftArrow:
            if ( active > 0 )
                {
                active--;
                iTabGroup->SetActiveTabByIndex( active );
                ActivateLocalViewL(TUid::Uid(iTabGroup->TabIdFromIndex(active)));
                }
            break;
        case EKeyRightArrow:
            if( (active + 1) < count )
                {
                active++;
                iTabGroup->SetActiveTabByIndex( active );
                ActivateLocalViewL(TUid::Uid(iTabGroup->TabIdFromIndex(active)));
                }
            break;
        default:
            return EKeyWasNotConsumed;
        }

    return EKeyWasConsumed;
    }

// ----------------------------------------------------
// CLauncherAppUi::HandleCommandL(TInt aCommand)
// ----------------------------------------------------
//
void CLauncherAppUi::HandleCommandL(TInt aCommand)
    {
    switch ( aCommand )
        {
        case EEikCmdExit:
        case EAknSoftkeyExit:
            {
            Exit();
            break;
            }
        case ECmdDeleteLog:
            {
            TInt ret = KErrNone;
            
            if( iEngine->LogFileExists())
                {
                ret = iEngine->DeleteLogFile();
                }
            if (ret == KErrNone)                
                {
                if( iEngine->BCLogFileExists())
                    {
                    ret = iEngine->DeleteBCLogFile();
                    }
                }            
            if( ret == KErrNone )
                {
                _LIT(KMessage, "Log file(s) deleted");
                CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
                note->ExecuteLD(KMessage);
                }
            else
                {
                _LIT(KMessage, "Unable to delete");
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(KMessage);
                }
            break;
            }        
        case ECmdCompareDLLs:
            {
            iEngine->AnalyseDLLsL();
            break;
            }

        case ECmdAbout:
            {
	        CAknMessageQueryDialog* dialog = new(ELeave) CAknMessageQueryDialog;
            dialog->ExecuteLD(R_LAUNCHER_ABOUT_DIALOG);

            break;
            }

        case ECmdStopLaunch:
            {
            iEngine->StopLaunchingL();
            break;
            }

        case ECmdSendUILogFile:
            {
            iEngine->SendLogViaSendUiL(iSendUi);
            break;
            }

        case ECmdSendUISystemDllFile:
            {
            iEngine->SendListOfSystemDllsViaSendUiL(iSendUi);
            break;
            }
        case EAknSoftkeyCancel:
            {
            iEngine->Cancel();
            break;
            }
        default:
            {
            }
            break;      
        }
    }

// End of File  
