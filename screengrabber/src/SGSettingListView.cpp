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
#include  <screengrabber.rsg>
#include  "SGSettingListView.h"
#include  "SGSettingListContainer.h"
#include  "SGMainView.h"
#include  "SG.hrh"
 

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CScreenGrabberSettingListView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CScreenGrabberSettingListView::ConstructL()
    {
    BaseConstructL( R_SCREENGRABBER_VIEW2 );
    }

// ---------------------------------------------------------
// CScreenGrabberSettingListView::~CScreenGrabberSettingListView()
// ?implementation_description
// ---------------------------------------------------------
//
CScreenGrabberSettingListView::~CScreenGrabberSettingListView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    }

// ---------------------------------------------------------
// TUid CScreenGrabberSettingListView::Id()
// ?implementation_description
// ---------------------------------------------------------
//
TUid CScreenGrabberSettingListView::Id() const
    {
    return KSettingListViewUID;
    }

// ---------------------------------------------------------
// CScreenGrabberSettingListView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CScreenGrabberSettingListView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EScreenGrabberCmdSettingsExit:
            {
            // exit the application
            AppUi()->HandleCommandL( EAknCmdExit );

            break;
            }

        case EScreenGrabberCmdSettingsBack:
            {
            // switch back to the main view
            AppUi()->ActivateLocalViewL( KMainViewUID );

            break;
            }

        case EScreenGrabberCmdSettingsChange:
            {
            // show the setting page
            iContainer->ShowSettingPageL(ETrue);

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
// CScreenGrabberSettingListView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CScreenGrabberSettingListView::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CScreenGrabberSettingListView::DoActivateL(...)
// ?implementation_description
// ---------------------------------------------------------
//
void CScreenGrabberSettingListView::DoActivateL(
   const TVwsViewId& /*aPrevViewId*/,TUid /*aCustomMessageId*/,
   const TDesC8& /*aCustomMessage*/)
    {
    if (!iContainer)
        {
        iContainer = new (ELeave) CScreenGrabberSettingListContainer;
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() );
        AppUi()->AddToStackL( *this, iContainer );
        }
   }

// ---------------------------------------------------------
// CScreenGrabberSettingListView::HandleCommandL(TInt aCommand)
// ?implementation_description
// ---------------------------------------------------------
//
void CScreenGrabberSettingListView::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    
    delete iContainer;
    iContainer = NULL;
    }

// End of File

