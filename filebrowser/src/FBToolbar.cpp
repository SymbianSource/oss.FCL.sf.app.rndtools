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
#include "FBToolbar.h"

#include <aknappui.h>
#include <akntoolbar.h>
#include <AknsUtils.h>
#include <aknbutton.h>
#include <akntouchpane.h>
#include <gulicon.h>
#include <eikapp.h>
#include <StringLoader.h>
#include <data_caging_path_literals.hrh>
#include <layoutmetadata.cdl.h>
#include <aknlayoutscalable_avkon.cdl.h>
#include <aknlayoutscalable_apps.cdl.h>
#include <filebrowser.rsg>
#include <AknInfoPopupNoteController.h>

#include "FB.hrh"
#include "FBDocument.h"
#include "FBModel.h"
#include "FBFileUtils.h"
#include "FBFileListContainer.h"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CFileBrowserToolbar::NewL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
CFileBrowserToolbar* CFileBrowserToolbar::NewL(/* CAknView& aView*/ )
    {
    CFileBrowserToolbar* self = CFileBrowserToolbar::NewLC();
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CFileBrowserToolbar::NewLC()
// Creates and leaves on failure. 
// -----------------------------------------------------------------------------
//
CFileBrowserToolbar* CFileBrowserToolbar::NewLC()
    {
    CFileBrowserToolbar* self = new ( ELeave )CFileBrowserToolbar();
    CleanupStack::PushL( self );
    self->ConstructL();               
    return self;
    }

// -----------------------------------------------------------------------------
// CFileBrowserToolbar::~CFileBrowserToolbar()
// removes the toolbar items when it leaves Editor/viewer plugin.
// -----------------------------------------------------------------------------
//
CFileBrowserToolbar::~CFileBrowserToolbar()
    {
    delete iInfoPopup;
    if ( AknLayoutUtils::PenEnabled() )
        {
        if ( iViewerToolbar )
            {
            iViewerToolbar->SetToolbarVisibility( EFalse );
            TRAP_IGNORE( iViewerToolbar->DisableToolbarL(ETrue) );
            }
        }

    if ( iViewerToolbar )
        {
        iViewerToolbar->HideItem( EFileBrowserCmdFileBack, ETrue, EFalse );
        iViewerToolbar->HideItem( EFileBrowserCmdFileSearch, ETrue, EFalse );
        iViewerToolbar->HideItem( EFileBrowserCmdEditSelect, ETrue, EFalse );
        }
    }
// -----------------------------------------------------------------------------
// CFileBrowserToolbar::CFileBrowserToolbar()
// C++ construct function add items the toolbar.
// -----------------------------------------------------------------------------
//
CFileBrowserToolbar::CFileBrowserToolbar() :
    iFirstSelectionIndex( KErrNotFound ),
    iLastSelectionIndex( KErrNotFound )
    {
    }

// -----------------------------------------------------------------------------
// CFileBrowserToolbar::ConstructL()
// C++ construct function add items the toolbar.
// -----------------------------------------------------------------------------
//
void CFileBrowserToolbar::ConstructL()
    {
    CAknAppUi* appUi = (CAknAppUi*)CEikonEnv::Static()->EikAppUi();
    iModel = static_cast<CFileBrowserDocument*>(appUi->Document())->Model();

    iViewerToolbar = appUi->CurrentFixedToolbar();
    iSelectbutton = static_cast<CAknButton*>( iViewerToolbar->ControlOrNull( EFileBrowserCmdEditSelect ) );

    if ( iViewerToolbar )
        {
        iShown = ETrue;
        iViewerToolbar->SetObserver( this );
        iViewerToolbar->SetToolbarObserver( this );
        iViewerToolbar->SetWithSliding( ETrue );
        iViewerToolbar->SetCloseOnAllCommands( EFalse );
        iViewerToolbar->SetFocusing( EFalse );
        }
    iInfoPopup = CAknInfoPopupNoteController::NewL();
    iInfoPopup->SetTimePopupInView(5000);
    iInfoPopup->SetTimeDelayBeforeShow(500);
    }

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::ResetState
// Updates the toolbar 
// ---------------------------------------------------------------------------
//
void CFileBrowserToolbar::ResetState()
    {
    if ( AknLayoutUtils::PenEnabled() )
        {
        iSelectbutton->SetCurrentState( 0, ETrue );
        iFirstSelectionIndex = KErrNotFound;
        iLastSelectionIndex = KErrNotFound;
        }
    if ( iInfoPopup )
        {
        iInfoPopup->HideInfoPopupNote();
        }
    }

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::DynInitToolbarL
// Initialise before the display of the toolbar
// ---------------------------------------------------------------------------
//
void CFileBrowserToolbar::DynInitToolbarL( TInt /*aResourceId*/, CAknToolbar* /*aToolbar*/ )
    {
    ResetState();
    iViewerToolbar->SetFocusing(EFalse);
    }

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::ShowToolbar
// Shows the toolbar
// ---------------------------------------------------------------------------
//
void CFileBrowserToolbar::ShowToolbarL()
    {
    if(AknLayoutUtils::PenEnabled() )
        {
        TRAP_IGNORE( iViewerToolbar->DisableToolbarL(EFalse) );
        iViewerToolbar->SetToolbarVisibility(ETrue);
        }
    }

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::ShowToolbar
// Hides the Toolbar
// ---------------------------------------------------------------------------
//
void  CFileBrowserToolbar::HideToolbarL()
    {
    // Undim the toolbar if dimmed
    if(AknLayoutUtils::PenEnabled() )
        {
        TRAP_IGNORE( iViewerToolbar->DisableToolbarL(ETrue) );
        iViewerToolbar->SetToolbarVisibility(EFalse);
        }
    }

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::OfferToolbarEventL
// Handling toolbar key events
// ---------------------------------------------------------------------------
//
void CFileBrowserToolbar::OfferToolbarEventL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EFileBrowserCmdFileBack:
            {
            iModel->FileUtils()->MoveUpOneLevelL();
            break;
            }
        case EFileBrowserCmdFileSearch:
            {
            TBool emptyListBox = iModel->FileListContainer()->ListBoxNumberOfVisibleItems() == 0;
            TBool searchFieldEnabled = iModel->FileListContainer()->IsSearchFieldEnabled();
            if ( !emptyListBox && !searchFieldEnabled )
                {
                iModel->FileListContainer()->EnableSearchFieldL();
                }
            else if ( searchFieldEnabled )
                {
                iModel->FileListContainer()->DisableSearchFieldL();
                }
            break;
            }
        case EFileBrowserCmdEditSelect:
            {
            TBool emptyListBox = iModel->FileListContainer()->ListBoxNumberOfVisibleItems() == 0;
            TBool driveListActive = iModel->FileUtils()->IsDriveListViewActive();
            TBool allSelected = iModel->FileListContainer()->ListBox()->SelectionIndexes()->Count() == iModel->FileListContainer()->ListBox()->Model()->NumberOfItems();
            TBool noneSelected = iModel->FileListContainer()->ListBox()->SelectionIndexes()->Count() == 0;
            
            if ( iViewerToolbar->EventModifiers() == CAknToolbar::ELongPressEnded )
                {                
                // handle long tap event 
                iFirstSelectionIndex = KErrNotFound;
                iLastSelectionIndex = KErrNotFound;
                iSelectbutton->SetCurrentState( 0, ETrue );

                if ( !( driveListActive || allSelected || emptyListBox ) )
                    {
                    AknSelectionService::HandleMarkableListProcessCommandL( EAknMarkAll, iModel->FileListContainer()->ListBox() );
                    }
                else if ( !( driveListActive || noneSelected || emptyListBox ) )
                    {
                    AknSelectionService::HandleMarkableListProcessCommandL( EAknUnmarkAll, iModel->FileListContainer()->ListBox() );
                    }
                }
            else if ( iViewerToolbar->EventModifiers() != CAknToolbar::ELongPress )
                {
                // handle single tap event
                TBool currentSelected = iModel->FileListContainer()->ListBox()->View()->ItemIsSelected(iModel->FileListContainer()->ListBox()->View()->CurrentItemIndex());                
                
                if ( !( driveListActive || emptyListBox ) )
                    {
                    if ( noneSelected && iFirstSelectionIndex == KErrNotFound )
                        {
                        iLastSelectionIndex = KErrNotFound;
                        }
                    
                    if ( iFirstSelectionIndex == KErrNotFound )
                        {
                        // first item selected
                        iFirstSelectionIndex = iModel->FileListContainer()->ListBox()->CurrentItemIndex();
                        if ( currentSelected )
                            {
                            AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdUnmark, iModel->FileListContainer()->ListBox() );
                            }
                        else
                            {
                            AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdMark, iModel->FileListContainer()->ListBox() );
                            }
                        // Toolbar button updates its state (icon)
                        iInfoPopup->SetTextL( _L("Select the last item.") );
                        iInfoPopup->ShowInfoPopupNote();
                        }
                    else
                        {
                        iInfoPopup->HideInfoPopupNote();
                        iLastSelectionIndex = iModel->FileListContainer()->ListBox()->CurrentItemIndex();
                        if ( iLastSelectionIndex == iFirstSelectionIndex )
                            {
                            // the same item selected second time
                            iFirstSelectionIndex = KErrNotFound;
                            iLastSelectionIndex = KErrNotFound;
                            if ( currentSelected )
                                {
                                AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdUnmark, iModel->FileListContainer()->ListBox() );
                                }
                            else
                                {
                                AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdMark, iModel->FileListContainer()->ListBox() );
                                }
                            // Toolbar button updates its state (icon)
                            }
                        else
                            {
                            // the last item selected, toggle the item selection between first and last 
                            TInt skipFirstToggle(1);
                            TInt skipLastToggle(0);
                            TInt finalFocus = iLastSelectionIndex;
                            if ( iFirstSelectionIndex > iLastSelectionIndex )
                                {
                                TInt tmp = iFirstSelectionIndex;
                                iFirstSelectionIndex = iLastSelectionIndex;
                                iLastSelectionIndex = tmp;
                                skipFirstToggle = 0;
                                skipLastToggle = 1;
                                }
                            for ( TInt i = iFirstSelectionIndex+skipFirstToggle; i <= iLastSelectionIndex-skipLastToggle ; i++ )
                                {
                                iModel->FileListContainer()->ListBox()->SetCurrentItemIndexAndDraw( i );
                                TBool itemSelected = iModel->FileListContainer()->ListBox()->View()->ItemIsSelected(iModel->FileListContainer()->ListBox()->View()->CurrentItemIndex());
                                if ( itemSelected )
                                    {
                                    TRAP_IGNORE( AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdUnmark, iModel->FileListContainer()->ListBox() ) );
                                    }
                                else
                                    {
                                    TRAP_IGNORE( AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdMark, iModel->FileListContainer()->ListBox() ) );
                                    }
                                }
                            iFirstSelectionIndex = KErrNotFound;
                            iLastSelectionIndex = KErrNotFound;
                            iModel->FileListContainer()->ListBox()->SetCurrentItemIndexAndDraw( finalFocus );
                            // Toolbar button updates its state (icon)
                            }
                        }
                    }
                else
                    {
                    iSelectbutton->SetCurrentState( 0, ETrue );
                    }
                }
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::HandleControlEventL
// Handles toolbar event cannot contain any code
// ---------------------------------------------------------------------------
//
void CFileBrowserToolbar::HandleControlEventL( CCoeControl* /*aControl*/,
                                               TCoeEvent /*aEventType*/ )
    {
    }

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::IsShown
// Returns whether the toolbar is shown or not
// ---------------------------------------------------------------------------
//
TBool CFileBrowserToolbar::IsShown()
    {
    return iShown;
    }   

// ---------------------------------------------------------------------------
// CFileBrowserToolbar::Rect
// 
// ---------------------------------------------------------------------------
//
const TRect CFileBrowserToolbar::Rect() const
    {
    return iViewerToolbar->Rect();
    }

// End of File

