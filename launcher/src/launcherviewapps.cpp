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
#include <aknviewappui.h>
#include <avkon.hrh>
#include <Launcher.rsg>
#include <AknUtils.h>
#include <eikmenup.h>

#include "LauncherViewApps.h"
#include "LauncherContainerApps.h" 

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CLauncherViewApps::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CLauncherViewApps::ConstructL(CLauncherEngine* aEngine)
    {
    BaseConstructL( R_LAUNCHER_VIEW1 );

    iContainer = new (ELeave) CLauncherContainerApps;
    iContainer->SetMopParent(this);
    iContainer->ConstructL( ClientRect(), aEngine );

    AppUi()->AddToStackL( *this, iContainer );

    // add data to the listbox and as default mark all as selected
    iContainer->UpdateFileListL();
    AknSelectionService::HandleMarkableListProcessCommandL( EAknMarkAll, iContainer->ListBox() );

    aEngine->SetContainerApps(iContainer);

    }

// ---------------------------------------------------------
// CLauncherViewApps::~CLauncherViewApps()
// ---------------------------------------------------------
//
CLauncherViewApps::~CLauncherViewApps()
    {
    AppUi()->RemoveFromViewStack( *this, iContainer );
    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CLauncherViewApps::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CLauncherViewApps::Id() const
    {
    return KViewId;
    }

// ------------------------------------------------------------------------------
// CLauncherViewApps::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//

void CLauncherViewApps::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId == R_LAUNCHER_VIEW1_MENU)
        {
        // launch start check
        aMenuPane->SetItemDimmed(ECmdLaunchApps, iContainer->Engine()->LaunchingIsActive());

        // hide launch options when launching is active
        aMenuPane->SetItemDimmed(ECmdLaunchOptions, iContainer->Engine()->LaunchingIsActive());

        // EAknMarkAll
        TBool allSelected = iContainer->ListBox()->SelectionIndexes()->Count() == iContainer->ListBox()->Model()->NumberOfItems();
        aMenuPane->SetItemDimmed(EAknMarkAll, allSelected);

        // EAknUnmarkAll
        TBool noneSelected = iContainer->ListBox()->SelectionIndexes()->Count() == 0;
        aMenuPane->SetItemDimmed(EAknUnmarkAll, noneSelected); 
        }

	else if (aResourceId == R_LAUNCHER_LAUNCHOPTIONS_SUBMENU)
	    {
	    if (iContainer->Engine()->SkipHiddenAndEmbedOnly())
            aMenuPane->SetItemButtonState(ECmdSkipHiddenAndEmbedOnly, EEikMenuItemSymbolOn);
	    else
            aMenuPane->SetItemButtonState(ECmdDoNotSkipAnyFiles, EEikMenuItemSymbolOn);
	    } 

    else
        {
        AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
        }
    }

// ---------------------------------------------------------
// CLauncherViewApps::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
//
void CLauncherViewApps::HandleCommandL(TInt aCommand)
    {  
    switch ( aCommand )
        {
        case ECmdLaunchAppsWithAutoClose:
            {
            iContainer->Engine()->StartAppLaunchingL(iContainer->SelectedApps(), ETrue);
            break;
            }
         
        case ECmdLaunchAppsWithoutAutoClose:
            {
            iContainer->Engine()->StartAppLaunchingL(iContainer->SelectedApps(), EFalse);
            break;
            }

        case ECmdSkipHiddenAndEmbedOnly:
            {
            iContainer->Engine()->SetSkipHiddenAndEmbedOnly(ETrue);
            break;
            }

        case ECmdDoNotSkipAnyFiles:
            {
            iContainer->Engine()->SetSkipHiddenAndEmbedOnly(EFalse);
            break;
            }
                        
        case ECmdRefreshList:
            {
            iContainer->UpdateFileListL();
            AknSelectionService::HandleMarkableListProcessCommandL( EAknMarkAll, iContainer->ListBox() );
            break;
            }

        case EAknCmdMark:
        case EAknCmdUnmark:
        case EAknUnmarkAll:
        case EAknMarkAll:
            {
            AknSelectionService::HandleMarkableListProcessCommandL( aCommand, iContainer->ListBox() );
            break;
            }
        
        default:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CLauncherViewApps::HandleClientRectChange()
// ---------------------------------------------------------
//
void CLauncherViewApps::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CLauncherViewApps::DoActivateL(...)
// ---------------------------------------------------------
//
void CLauncherViewApps::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    //AppUi()->AddToStackL( *this, iContainer );
    iContainer->ActivateL();
    iContainer->MakeVisible(ETrue);
    }

// ---------------------------------------------------------
// CLauncherViewApps::DoDeactivate()
// ---------------------------------------------------------
//
void CLauncherViewApps::DoDeactivate()
    {
    iContainer->MakeVisible(EFalse);
    //AppUi()->RemoveFromViewStack( *this, iContainer );
    }

// End of File

