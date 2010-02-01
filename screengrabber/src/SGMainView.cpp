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
#include  <aknViewAppUi.h>
#include  <avkon.hrh>
#include  <apgtask.h>
#include  <aknmessagequerydialog.h> 

#include  <screengrabber.rsg>
#include  "SG.hrh"
#include  "SGMainView.h"
#include  "SGMainContainer.h"
#include  "SGSettingListView.h"
#include  "SGDocument.h" 
#include  "SGModel.h"

#ifdef SCREENGRABBER_SCREEN_RESOLUTION_CHANGE_SUPPORT
 #include <AknSgcc.h>
 #include <AknLayoutConfig.h>
 #include <e32property.h>
 #include <UikonInternalPSKeys.h>
#endif

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CScreenGrabberMainView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CScreenGrabberMainView::ConstructL()
    {
    BaseConstructL( R_SCREENGRABBER_VIEW1 );
    }

// ---------------------------------------------------------
// CScreenGrabberMainView::~CScreenGrabberMainView()
// ---------------------------------------------------------
//
CScreenGrabberMainView::~CScreenGrabberMainView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CScreenGrabberMainView::Id()
// ---------------------------------------------------------
//
TUid CScreenGrabberMainView::Id() const
    {
    return KMainViewUID;
    }



#ifdef SCREENGRABBER_SCREEN_RESOLUTION_CHANGE_SUPPORT

// ---------------------------------------------------------
// TUid CScreenGrabberMainView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
// ---------------------------------------------------------
//

void CScreenGrabberMainView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId == R_SCREENGRABBER_RESOLUTION_SUBMENU)
        {
        // generate list of supported screen resolutions
        TInt state = 0;
        TInt width = 0;
        TInt height = 0;
        TInt screenMode = 0;
        
        const CAknLayoutConfig& config = CAknSgcClient::LayoutConfig();
        const CAknLayoutConfig::THardwareStateArray& hwStates = config.HardwareStates();
        const CAknLayoutConfig::TScreenModeArray& screenModes = config.ScreenModes();

        CWsScreenDevice* screenDevice = CEikonEnv::Static()->ScreenDevice();
        TInt currentHwState = screenDevice->CurrentScreenMode();
            
        for (TInt i=0; i<hwStates.Count(); i++)
            {
            // get an entry entry
            const CAknLayoutConfig::THardwareState& hwState = hwStates.At(i);
            screenMode = hwState.ScreenMode();
            const CAknLayoutConfig::TScreenMode& mode = screenModes.Find(screenMode);
            
            // get more info
            state = hwState.StateNumber();
            width = mode.PixelsTwipsAndRotation().iPixelSize.iWidth;
            height = mode.PixelsTwipsAndRotation().iPixelSize.iHeight;
            
            // generate new menu text
            TBuf<64> menuText;
            menuText.Format(_L("%dx%d  (%d)"), width, height, state);
            
            // create new menu item
            CEikMenuPaneItem::SData menuItem;
            menuItem.iFlags = 0;
            menuItem.iText = menuText;
            menuItem.iCommandId = EScreenGrabberCmdScreenResolutionItem1 + state;
            menuItem.iCascadeId = 0;
            aMenuPane->AddMenuItemL(menuItem);
            }
        }
    }

#endif
    
// ---------------------------------------------------------
// CScreenGrabberMainView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
//
void CScreenGrabberMainView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EScreenGrabberCmdSendToBackground:
            {
            TApaTask selfTask(iEikonEnv->WsSession());
            selfTask.SetWgId(iEikonEnv->RootWin().Identifier());
            selfTask.SendToBackground();
            break;
            }

#ifdef SCREENGRABBER_SCREEN_RESOLUTION_CHANGE_SUPPORT
        case EScreenGrabberCmdScreenResolutionItem1:
        case EScreenGrabberCmdScreenResolutionItem2:
        case EScreenGrabberCmdScreenResolutionItem3:
        case EScreenGrabberCmdScreenResolutionItem4:
        case EScreenGrabberCmdScreenResolutionItem5:
        case EScreenGrabberCmdScreenResolutionItem6:
        case EScreenGrabberCmdScreenResolutionItem7:
        case EScreenGrabberCmdScreenResolutionItem8:
        case EScreenGrabberCmdScreenResolutionItem9:
        case EScreenGrabberCmdScreenResolutionItem10:
        case EScreenGrabberCmdScreenResolutionItem11:
        case EScreenGrabberCmdScreenResolutionItem12:
            {
            // change resolution
            const CAknLayoutConfig& config = CAknSgcClient::LayoutConfig();
            const CAknLayoutConfig::THardwareStateArray& hwStates = config.HardwareStates();
            const CAknLayoutConfig::TScreenModeArray& screenModes = config.ScreenModes();
        
            const CAknLayoutConfig::THardwareState& hwState = hwStates.At(aCommand-EScreenGrabberCmdScreenResolutionItem1);
            TInt screenMode = hwState.ScreenMode();
            TInt state = hwState.StateNumber();
            
            CWsScreenDevice* screenDevice = CEikonEnv::Static()->ScreenDevice();
            TInt err = RProperty::Define(KPSUidUikon, KUikLayoutState, RProperty::EInt);
            err = RProperty::Set(KPSUidUikon, KUikLayoutState, state);

            screenDevice->SetScreenMode(screenMode);

            break;
            }
#endif
            
        case EScreenGrabberCmdSettings:
            {
            AppUi()->ActivateLocalViewL( KSettingListViewUID );
            break;
            }

        case EScreenGrabberCmdAbout:
            {
	        CAknMessageQueryDialog* dialog = new (ELeave) CAknMessageQueryDialog;
            dialog->ExecuteLD(R_SCREENGRABBER_ABOUT_DIALOG);
            }
            break;
        
        case EAknSoftkeyExit:
            {
            AppUi()->HandleCommandL( EEikCmdExit );
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
// CScreenGrabberMainView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CScreenGrabberMainView::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CScreenGrabberMainView::DoActivateL(...)
// ---------------------------------------------------------
//
void CScreenGrabberMainView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    if (!iContainer)
        {
        iContainer = new (ELeave) CScreenGrabberMainContainer;
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() );
        AppUi()->AddToStackL( *this, iContainer );
        } 
   }

// ---------------------------------------------------------
// CScreenGrabberMainView::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
//
void CScreenGrabberMainView::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    
    delete iContainer;
    iContainer = NULL;
    }

// End of File

