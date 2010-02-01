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
#include  <aknviewappui.h>
#include  <avkon.hrh>
#include  <Launcher.rsg>
#include  "LauncherViewOutput.h"
#include  "LauncherContainerOutput.h" 

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CLauncherViewOutput::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CLauncherViewOutput::ConstructL(CLauncherEngine* aEngine)
    {
    BaseConstructL( R_LAUNCHER_VIEW2 );

    iContainer = new (ELeave) CLauncherContainerOutput;
    iContainer->SetMopParent(this);
    iContainer->ConstructL( ClientRect(), aEngine );

    AppUi()->AddToStackL( *this, iContainer );

    aEngine->SetContainerOutput(iContainer);
    }

// ---------------------------------------------------------
// CLauncherViewOutput::~CLauncherViewOutput()
// ---------------------------------------------------------
//
CLauncherViewOutput::~CLauncherViewOutput()
    {
    AppUi()->RemoveFromViewStack( *this, iContainer );
    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CLauncherViewOutput::Id()
// ---------------------------------------------------------
//
TUid CLauncherViewOutput::Id() const
    {
    return KView2Id;
    }

// ------------------------------------------------------------------------------
// CLauncherViewApps::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
//  This function is called by the EIKON framework just before it displays
//  a menu pane. Its default implementation is empty, and by overriding it,
//  the application can set the state of menu items dynamically according
//  to the state of application data.
// ------------------------------------------------------------------------------
//

void CLauncherViewOutput::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId == R_LAUNCHER_VIEW2_MENU)
        {
        // ...
        }
    else
        {
        AppUi()->DynInitMenuPaneL(aResourceId, aMenuPane);
        }
    }

// ---------------------------------------------------------
// CLauncherViewOutput::HandleCommandL(TInt aCommand)
// ---------------------------------------------------------
//
void CLauncherViewOutput::HandleCommandL(TInt aCommand)
    {   

    switch ( aCommand )
        {
        case ECmdClearWindow:
            {
            iContainer->ClearOutputWindowL();
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
// CLauncherViewOutput::HandleClientRectChange()
// ---------------------------------------------------------
//
void CLauncherViewOutput::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CLauncherViewOutput::DoActivateL(...)
// ---------------------------------------------------------
//
void CLauncherViewOutput::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    //AppUi()->AddToStackL( *this, iContainer );
    iContainer->ActivateL();
    iContainer->MakeVisible(ETrue);
    }

// ---------------------------------------------------------
// CLauncherViewOutput::DoDeactivate()
// ---------------------------------------------------------
//
void CLauncherViewOutput::DoDeactivate()
    {
    iContainer->MakeVisible(EFalse);
    //AppUi()->RemoveFromViewStack( *this, iContainer );
    }

// End of File

