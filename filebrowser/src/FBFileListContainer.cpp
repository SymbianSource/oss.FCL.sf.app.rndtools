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
#include "FBFileListContainer.h"
#include "FB.hrh"
#include "FBDocument.h"
#include "FBAppUi.h"
#include "FBModel.h"
#include "FBFileUtils.h"
#include "FBStd.h"
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    #include "FBToolbar.h"
#endif

#include <w32std.h>
#include <AknDef.h>
#include <AknUtils.h> 
#include <eikspane.h>
#include <akntitle.h>
#include <aknnavilabel.h>
#include <aknlists.h>
#include <aknsfld.h>
#include <eikclbd.h>
#include <eiktxlbm.h>
#include <AknsListBoxBackgroundControlContext.h>
#include <AknInfoPopupNoteController.h>
#include <AknIconArray.h>
#include <layoutmetadata.cdl.h>

_LIT(KEmptyDirectory, "No entries");
_LIT(KNoMatches, "No matches");

const TInt KSimpleViewFontHeight = 127;
const TInt KExtendedViewFontHeight1 = 127;
const TInt KExtendedViewFontHeight2 = 105;

const TInt KSimpleViewFontHeightQHD = 120;
const TInt KExtendedViewFontHeight1QHD = 115;
const TInt KExtendedViewFontHeight2QHD = 90;

// ===================================== MEMBER FUNCTIONS =====================================

void CFileBrowserFileListContainer::ConstructL(const TRect& /*aRect*/)
    {
    iModel = static_cast<CFileBrowserDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();
    iModel->SetFileListContainer(this);
    
    iFileViewMode = -1;
    iIsDragging = EFalse;
    
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __SERIES60_32__)
    iToolbar = CFileBrowserToolbar::NewL();
    if ( iModel->Settings().iEnableToolbar && 
         AknLayoutUtils::PenEnabled() )
        {
        iToolbar->ShowToolbarL();        
        }
    else
        {
        iToolbar->HideToolbarL();
        }
#endif
    
    CreateWindowL();
    Window().SetShadowDisabled(EFalse);
    SetBlank();

    SetScreenLayoutL(iModel->Settings().iDisplayMode);
    
    CreateEmptyNaviPaneLabelL();
    
    iModel->FileUtils()->RefreshViewL();

    EnableDragEvents();
    
    ActivateL();
    
    iInfoPopup = CAknInfoPopupNoteController::NewL();
    iInfoPopup->SetTimePopupInView(5000);
    iInfoPopup->SetTimeDelayBeforeShow(500);
    iInfoPopup->SetTextL(_L("TIP: Use left arrow key or touch drag to left to move up in the directory structure."));
    iInfoPopup->ShowInfoPopupNote();
    }

// --------------------------------------------------------------------------------------------

CFileBrowserFileListContainer::~CFileBrowserFileListContainer()
    {
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    delete iToolbar;
#endif
    delete iInfoPopup;
    
    DeleteNaviPane();

    if (iSearchField)
        delete iSearchField;
    
    if (iListBox)
        delete iListBox;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::CreateEmptyNaviPaneLabelL()
    {
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();

    iNaviContainer = static_cast<CAknNavigationControlContainer*>(sp->ControlL(TUid::Uid(EEikStatusPaneUidNavi)));
    iNaviDecorator = iNaviContainer->CreateNavigationLabelL();
    iNaviContainer->PushL(*iNaviDecorator);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::DeleteNaviPane()
    {
    if (iNaviContainer)
        {
        iNaviContainer->Pop(iNaviDecorator);
        }
    
    if (iNaviDecorator)
        {
        delete iNaviDecorator;
        iNaviDecorator = NULL;
        }
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();
    gc.Clear(aRect);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// --------------------------------------------------------------------------------------------

TKeyResponse CFileBrowserFileListContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    // open search field with alpha digit numbers
    if (aType == EEventKeyDown && !iSearchField && aKeyEvent.iScanCode < EStdKeyF1 && TChar(aKeyEvent.iScanCode).IsAlphaDigit())
        {
        EnableSearchFieldL();
        
        if (iSearchField)
            {
            //static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model())->Filter()->DeferredSendKeyEventToFepL(aKeyEvent.iCode);
            return EKeyWasConsumed;
            }
        }
    
    // close search field with clear-button if it's empty
    if (iSearchField && aKeyEvent.iCode == EKeyBackspace)
        {
           TFileName searchText;
           iSearchField->GetSearchText(searchText);
           
           if (searchText == KNullDesC)
               {
               DisableSearchFieldL();
               return EKeyWasConsumed; 
               }
        }

    // check file/directory handling related cases
    if (iModel->FileUtils()->HandleOfferKeyEventL(aKeyEvent, aType) == EKeyWasConsumed)
        return EKeyWasConsumed;
    
    TKeyResponse result = EKeyWasNotConsumed;

    if (iSearchField && iListBox)
        {
        // find items with all event codes (that's the reason why there is EEventKey instead of aType)
        TBool needsRefresh(EFalse);
        result = AknFind::HandleFindOfferKeyEventL(aKeyEvent, EEventKey, this, iListBox, iSearchField, EFalse, needsRefresh);
        
        if (needsRefresh)
            DrawNow();
        }

    // scroll one page up/down with volume keys
    if (result == EKeyWasNotConsumed && iListBox && iListBox->Model()->NumberOfItems() > 0 && (aKeyEvent.iCode == EKeyIncVolume || (aKeyEvent.iCode=='#' && !iModel->IsHashKeySelectionInUse())))
        {
        CListBoxView::TSelectionMode selectionMode = CListBoxView::ENoSelection;
        CListBoxView::TCursorMovement moveto = CListBoxView::ECursorPrevScreen;
        
        iListBox->View()->MoveCursorL(moveto, selectionMode);
      
        iListBox->SetCurrentItemIndex(iListBox->CurrentItemIndex()); 
        iListBox->DrawDeferred();

        return EKeyWasConsumed;
        
        }
    else if (result == EKeyWasNotConsumed && iListBox && iListBox->Model()->NumberOfItems() > 0 && (aKeyEvent.iCode == EKeyDecVolume || aKeyEvent.iCode=='*'))
        {
        CListBoxView::TSelectionMode selectionMode = CListBoxView::ENoSelection;
        CListBoxView::TCursorMovement moveto = CListBoxView::ECursorNextScreen;
        
        if (iListBox->CurrentItemIndex() >= iListBox->Model()->NumberOfItems() - iListBox->View()->NumberOfItemsThatFitInRect(iListBox->View()->ViewRect()))
            moveto = CListBoxView::ECursorLastItem;
        
        iListBox->View()->MoveCursorL(moveto, selectionMode);

        iListBox->SetCurrentItemIndex(iListBox->CurrentItemIndex()); 
        iListBox->DrawDeferred();

        return EKeyWasConsumed;
        }

    // offer key event to the listbox if not already consumed
    else if (result == EKeyWasNotConsumed && iListBox)
        return iListBox->OfferKeyEventL(aKeyEvent, aType);
    else
        return result;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
    {
    switch (aEventType)
        {
        case EEventEnterKeyPressed:
        case EEventItemDoubleClicked:
            {
            // "emulate" enter key press and send it to file utils for prosessing
            
            TKeyEvent keyEvent;
            keyEvent.iCode = EKeyEnter;
            keyEvent.iModifiers = 0;
            
            TEventCode type = EEventKey;
    
            iModel->FileUtils()->HandleOfferKeyEventL(keyEvent, type);          
            }
            break;
        default:
            break;
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::HandlePointerEventL(const TPointerEvent &aPointerEvent)
    {
    // detect direction of dragging by comparing the start and finish points
    
    if (aPointerEvent.iType == TPointerEvent::EButton1Down)
        {
        iDragStartPoint = aPointerEvent.iPosition;
        iIsDragging = EFalse;
        }
    else if (aPointerEvent.iType == TPointerEvent::EDrag)
        {
        iIsDragging = ETrue;
        
        return;
        }
    else if (aPointerEvent.iType == TPointerEvent::EButton1Up)
        {
        if (iIsDragging)
            {
            const TInt KDelta = iDragStartPoint.iX - aPointerEvent.iPosition.iX;
            const TInt KThreshold = 30;
            
            if (KDelta < -KThreshold) // dragging to right
                {
                // "emulate" right key press and send it to file utils for prosessing
                
                TKeyEvent keyEvent;
                keyEvent.iCode = EKeyRightArrow;
                keyEvent.iModifiers = 0;
                
                TEventCode type = EEventKey;
        
                iModel->FileUtils()->HandleOfferKeyEventL(keyEvent, type);
                
                return;
                }
            else if (KDelta > KThreshold) // dragging to left
                {
                // "emulate" left key press and send it to file utils for prosessing
                
                TKeyEvent keyEvent;
                keyEvent.iCode = EKeyLeftArrow;
                keyEvent.iModifiers = 0;
                
                TEventCode type = EEventKey;
        
                iModel->FileUtils()->HandleOfferKeyEventL(keyEvent, type);
                
                return;
                }            
            }
        iIsDragging = EFalse;
        }
    else
        {
        iIsDragging = EFalse;        
        }

    CCoeControl::HandlePointerEventL(aPointerEvent);
    }
            
// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::CreateListBoxL(TInt aFileViewMode)
    {
    iFileViewMode = aFileViewMode;
    
    if (iSearchField)
        DisableSearchFieldL();
    
    if (iListBox)
        {
        delete iListBox;
        iListBox = NULL;    
        }
    
    if (iFileViewMode == EFileViewModeSimple)
        {
        iListBox = new(ELeave) CSimpleFileViewListBox;
        }
    else     
        {
        iListBox = new(ELeave) CExtendedFileViewListBox;
        }
        
                
    iListBox->SetContainerWindowL(*this);
    iListBox->ConstructL(this, EAknListBoxMarkableList);
    
    iListBox->View()->SetListEmptyTextL(KEmptyDirectory);
        
    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
    iListBox->SetListBoxObserver(this);

    // make sure listbox has correct size
    SizeChanged();

    iListBox->ActivateL();
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileListContainer::CurrentListBoxItemIndex()
    {
    if (iListBox && iSearchField)
        {
        TInt currentItemIndex = iListBox->CurrentItemIndex();
        
        if (currentItemIndex >= 0)
            return static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model())->Filter()->FilteredItemIndex(currentItemIndex);
        else
            return KErrNotFound;
        }
    else if (iListBox)
        {
        return iListBox->CurrentItemIndex();
        }
    else
        return KErrNotFound;    
    }

// --------------------------------------------------------------------------------------------

const CArrayFix<TInt>* CFileBrowserFileListContainer::ListBoxSelectionIndexes()
    {
    if (iListBox && iSearchField)
        {
        TRAP_IGNORE( static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model())->Filter()->UpdateSelectionIndexesL() ); // !!!
        return static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model())->Filter()->SelectionIndexes();
        }
    else if (iListBox)
        {
        const CListBoxView::CSelectionIndexArray* indices = iListBox->SelectionIndexes();
        return static_cast<const CArrayFix<TInt>*>(indices);
        }
    else
        return NULL;
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileListContainer::ListBoxSelectionIndexesCount()
    {
    if (iListBox && iSearchField)
        {
        TRAP_IGNORE( static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model())->Filter()->UpdateSelectionIndexesL() ); // !!!
        return static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model())->Filter()->SelectionIndexes()->Count();
        }
    else if (iListBox)
        {
        return iListBox->SelectionIndexes()->Count();
        }
    else
        return KErrNotFound;
    }
    
// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileListContainer::ListBoxNumberOfVisibleItems()
    {
    if (iListBox && iSearchField)
        {
        return static_cast<CAknFilteredTextListBoxModel*>(iListBox->Model())->Filter()->FilteredNumberOfItems();
        }
    else if (iListBox)
        {
        return iListBox->Model()->NumberOfItems();
        }
    else
        return KErrNotFound;
    }
        
// --------------------------------------------------------------------------------------------

CAknIconArray* CFileBrowserFileListContainer::ListBoxIconArrayL()
    {
    if (iListBox)
        { 
        if (iFileViewMode == EFileViewModeSimple)
            {
            return static_cast<CAknIconArray*>(static_cast<CEikColumnListBox*>(iListBox)->ItemDrawer()->ColumnData()->IconArray());
            }
        else if (iFileViewMode == EFileViewModeExtended)
            {
            return static_cast<CAknIconArray*>(static_cast<CEikFormattedCellListBox*>(iListBox)->ItemDrawer()->ColumnData()->IconArray());
            }
        }

    return NULL;
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::SetListBoxIconArrayL(CAknIconArray* aIconArray)
    {
    if (iListBox)
        { 
        if (iFileViewMode == EFileViewModeSimple)
            {
            //CArrayPtr<CGulIcon>* currentIconArray = static_cast<CEikColumnListBox*>(iListBox)->ItemDrawer()->ColumnData()->IconArray();
            //delete currentIconArray;
            //currentIconArray = NULL;       
            static_cast<CEikColumnListBox*>(iListBox)->ItemDrawer()->ColumnData()->SetIconArray(aIconArray);
            }
        else if (iFileViewMode == EFileViewModeExtended)
            {
            //CArrayPtr<CGulIcon>* currentIconArray = static_cast<CEikFormattedCellListBox*>(iListBox)->ItemDrawer()->ColumnData()->IconArray();
            //delete currentIconArray;
            //currentIconArray = NULL;       
            static_cast<CEikFormattedCellListBox*>(iListBox)->ItemDrawer()->ColumnData()->SetIconArray(aIconArray);
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::SetListBoxTextArrayL(CDesCArray* aTextArray)
    {
    if (iListBox)
        {    
        iListBox->Model()->SetItemTextArray(aTextArray);
        iListBox->Model()->SetOwnershipType(ELbmOwnsItemArray);
        iListBox->HandleItemAdditionL();
        iListBox->UpdateScrollBarsL();
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::EnableSearchFieldL()
    {
    if (iListBox && !iSearchField && iListBox->Model()->NumberOfItems() > 0)
        {
        iSearchField = CAknSearchField::NewL(*this, CAknSearchField::ESearch, NULL, KMaxFileName);
        
        iSearchField->SetObserver(this);
        iSearchField->SetFocus(ETrue);
        iSearchField->MakeVisible(ETrue);
        iSearchField->ResetL();
        
        iListBox->View()->SetListEmptyTextL(KNoMatches);
        
        // enable filtering
        ((CAknFilteredTextListBoxModel*)iListBox->Model())->CreateFilterL(iListBox, iSearchField);
        
        SizeChanged();   

        // update cba
        CEikButtonGroupContainer* cbaGroup = CEikButtonGroupContainer::Current();
        cbaGroup->SetCommandSetL(R_AVKON_SOFTKEYS_OPTIONS_CANCEL);
        cbaGroup->DrawNow();
        }
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserFileListContainer::IsSearchFieldEnabled()
    {
    return iSearchField != NULL;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::DisableSearchFieldL()
    {
    if (iSearchField)
        {
        iSearchField->MakeVisible( EFalse );
        iSearchField->SetFocusing( EFalse );
        
        delete iSearchField;
        iSearchField = NULL;  

        iListBox->View()->SetListEmptyTextL(KEmptyDirectory);

        // disable filtering
        ((CAknFilteredTextListBoxModel*)iListBox->Model())->RemoveFilter();

        SizeChanged();
        DrawNow();

        // update cba
        CEikButtonGroupContainer* cbaGroup = CEikButtonGroupContainer::Current();
        cbaGroup->SetCommandSetL(R_AVKON_SOFTKEYS_OPTIONS_EXIT);
        cbaGroup->DrawNow();
        
        // Clear selections because using filter causes selections to go out of sync
        UpdateToolbar();
        AknSelectionService::HandleMarkableListProcessCommandL( EAknUnmarkAll, iListBox );
        }
    }

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileListContainer::CountComponentControls() const
    {
    TInt controls(0);

    if (iListBox)
        controls++;
    
    if (iSearchField)
        controls++;
    
    return controls;
    }

// --------------------------------------------------------------------------------------------

CCoeControl* CFileBrowserFileListContainer::ComponentControl(TInt aIndex) const
    {
    switch (aIndex)
        {
        case 0:
            {
            return iListBox;
            }
        case 1:
            {
            return iSearchField;
            }
        default:
            {
            return NULL;
            }
        }
    }    

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::SetScreenLayoutL(TInt aLayoutMode)
    {
    // make sure that the search field has been disabled
    DisableSearchFieldL();

    // make sure that the title of the application is correct
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* tp = static_cast<CAknTitlePane*>( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    tp->SetTextL( _L("FileBrowser") );
    
    // change the layout mode
    if (aLayoutMode == EDisplayModeFullScreen)
        sp->SwitchLayoutL(R_AVKON_STATUS_PANE_LAYOUT_SMALL);
    else
        sp->SwitchLayoutL(R_AVKON_STATUS_PANE_LAYOUT_USUAL);
    
    // update layout
    HandleResourceChange(KEikDynamicLayoutVariantSwitch);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::SetNaviPaneTextL(const TDesC& aText)
    {
    if (iNaviDecorator)
        {
        delete iNaviDecorator;
        iNaviDecorator = NULL;
        }
    
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    iNaviDecorator = iNaviContainer->CreateMessageLabelL(aText);
    iNaviContainer->PushL(*iNaviDecorator);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::SizeChanged()
    {
    if (iListBox && iSearchField)
        {
        TSize findPaneSize;
        AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EFindPane, findPaneSize);

        TRect mainPaneRect;
        TRect naviPaneRect;

        if (Layout_Meta_Data::IsLandscapeOrientation() ||
            iEikonEnv->AppUiFactory()->StatusPane()->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_SMALL)
            {
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
            if ( iToolbar && iModel->Settings().iEnableToolbar )
                {
                iToolbar->ShowToolbarL();
                // Get location of softkeys 
                AknLayoutUtils::TAknCbaLocation cbaLocation = AknLayoutUtils::CbaLocation(); 
                if ( cbaLocation == AknLayoutUtils::EAknCbaLocationBottom ) 
                    { 
                    // here there is the BSK mode
                    mainPaneRect.iBr.iX -= iToolbar->Rect().Width();
                    findPaneSize.iWidth -= iToolbar->Rect().Width();
                    }
                }
#endif
            }
        else
            {
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ENaviPane, naviPaneRect);
            mainPaneRect.iTl.iY = naviPaneRect.Height();
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
            if ( iToolbar && iModel->Settings().iEnableToolbar )
                {
                iToolbar->ShowToolbarL();
                mainPaneRect.iBr.iY -= iToolbar->Rect().Height();
                }
#endif
            }
        
        mainPaneRect.iBr.iY -= findPaneSize.iHeight;  

        TRect listBoxRect(mainPaneRect.Size());
        iListBox->SetRect(listBoxRect);

        if (iFileViewMode == EFileViewModeSimple)
            {        
            CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikColumnListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
            listBoxContext->SetRect(listBoxRect);
            }
        else if (iFileViewMode == EFileViewModeExtended)
            {        
            CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikFormattedCellListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
            listBoxContext->SetRect(listBoxRect);
            }        

        iSearchField->SetRect(TRect(TPoint(0,mainPaneRect.Height()), findPaneSize));
        
        if (iSearchField->IsVisible() && iListBox->IsVisible())
            iSearchField->SetLinePos(2);

        }
        
    else if (iListBox)
        {
        TRect mainPaneRect;
        TRect naviPaneRect;

        if (Layout_Meta_Data::IsLandscapeOrientation() ||
            iEikonEnv->AppUiFactory()->StatusPane()->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_SMALL)
            {
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
            if ( iToolbar && iModel->Settings().iEnableToolbar )
                {
                iToolbar->ShowToolbarL();
                // Get location of softkeys 
                AknLayoutUtils::TAknCbaLocation cbaLocation = AknLayoutUtils::CbaLocation(); 
                if ( cbaLocation == AknLayoutUtils::EAknCbaLocationBottom ) 
                    { 
                    // here there is the BSK mode
                    mainPaneRect.iBr.iX -= iToolbar->Rect().Width();
                    }
                }
#endif
            }
        else
            {
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ENaviPane, naviPaneRect);
            mainPaneRect.iTl.iY = naviPaneRect.Height();
            
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
            if ( iToolbar && iModel->Settings().iEnableToolbar )
                {
                iToolbar->ShowToolbarL();
                mainPaneRect.iBr.iY -= iToolbar->Rect().Height();
                }
#endif
            }
        
        TRect listBoxRect(mainPaneRect.Size());
        iListBox->SetRect(listBoxRect);

        if (iFileViewMode == EFileViewModeSimple)
            {        
            CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikColumnListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
            listBoxContext->SetRect(listBoxRect);
            }
        else if (iFileViewMode == EFileViewModeExtended)
            {        
            CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikFormattedCellListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
            listBoxContext->SetRect(listBoxRect);
            }
        }
    }
        
// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);

    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect mainPaneRect;
        TRect naviPaneRect;
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __SERIES60_32__)
        TBool needToCutSearchFieldLength( EFalse );
#endif
        
        if (Layout_Meta_Data::IsLandscapeOrientation() ||
            iEikonEnv->AppUiFactory()->StatusPane()->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_SMALL)
            {
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __SERIES60_32__)
            if ( iToolbar && iModel->Settings().iEnableToolbar )
                {
                iToolbar->ShowToolbarL();
                // Get location of softkeys 
                AknLayoutUtils::TAknCbaLocation cbaLocation = AknLayoutUtils::CbaLocation(); 
                if ( cbaLocation == AknLayoutUtils::EAknCbaLocationBottom ) 
                    { 
                    // here there is the BSK mode
                    mainPaneRect.iBr.iX -= iToolbar->Rect().Width();
                    needToCutSearchFieldLength = ETrue;
                    }
                }
#endif
            }
        else
            {
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
            AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ENaviPane, naviPaneRect);
            mainPaneRect.iTl.iY = naviPaneRect.Height();

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __SERIES60_32__)
            if ( iToolbar && iModel->Settings().iEnableToolbar )
                {
                iToolbar->ShowToolbarL();
                mainPaneRect.iBr.iY -= iToolbar->Rect().Height();
                }
#endif
            }

        SetRect(mainPaneRect);

        if (iListBox && iSearchField)
            {
            TSize findPaneSize;
            AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EFindPane, findPaneSize);
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __SERIES60_32__)
            if ( needToCutSearchFieldLength )
                {
                findPaneSize.iWidth -= iToolbar->Rect().Width();
                }
#endif
            mainPaneRect.iBr.iY -= findPaneSize.iHeight;
            
            TRect listBoxRect(mainPaneRect.Size());
            iListBox->SetRect(listBoxRect);
            
            if (iFileViewMode == EFileViewModeSimple)
                {        
                CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikColumnListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
                listBoxContext->SetRect(listBoxRect);
                }
            else if (iFileViewMode == EFileViewModeExtended)
                {        
                CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikFormattedCellListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
                listBoxContext->SetRect(listBoxRect);
                }    
            
            iSearchField->SetRect(TRect(TPoint(0,mainPaneRect.Height()), findPaneSize));
            
            if (iSearchField->IsVisible() && iListBox->IsVisible())
                iSearchField->SetLinePos(2);
        
            TRAP_IGNORE(iListBox->UpdateScrollBarsL());
            }        
        else if (iListBox)
            {
            TRect listBoxRect(mainPaneRect.Size());
            iListBox->SetRect(listBoxRect);
            
            if (iFileViewMode == EFileViewModeSimple)
                {        
                CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikColumnListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
                listBoxContext->SetRect(listBoxRect);
                }
            else if (iFileViewMode == EFileViewModeExtended)
                {        
                CAknsListBoxBackgroundControlContext* listBoxContext = static_cast<CAknsListBoxBackgroundControlContext*>(static_cast<CEikFormattedCellListBox*>(iListBox)->ItemDrawer()->ColumnData()->SkinBackgroundContext());
                listBoxContext->SetRect(listBoxRect);
                }    
            
            TRAP_IGNORE(iListBox->UpdateScrollBarsL());
            }
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::HandleSettingsChangeL()
    {
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    if ( iModel->Settings().iEnableToolbar )
        {
        if ( AknLayoutUtils::PenEnabled() )
            {
            if ( !iToolbar )
                {
                iToolbar = CFileBrowserToolbar::NewL();
                }
            iToolbar->ShowToolbarL();
            }
        }
    else
        {
        HideToolbar();
        }
    SizeChanged();
#endif
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::HideToolbar()
    {
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    if ( iToolbar && iToolbar->IsShown() )
        {
        iToolbar->HideToolbarL();
        }
#endif
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileListContainer::UpdateToolbar()
    {
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__ && !defined __S60_32__)
    if ( iToolbar && iToolbar->IsShown() )
        {
        iToolbar->ResetState();
        }
#endif    
    }

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

CSimpleFileViewListBox::~CSimpleFileViewListBox()
    {
    ReleaseFonts();
    }

// --------------------------------------------------------------------------------------------

void CSimpleFileViewListBox::ReleaseFonts()
    {
    if (iFont)
        {
        CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
        dev->ReleaseFont(iFont);
        iFont = NULL;   
        }
    }

// --------------------------------------------------------------------------------------------
    
void CSimpleFileViewListBox::SizeChanged()
    {
    // call the base class function first
    CAknSingleGraphicStyleListBox::SizeChanged();

    // customize the list box
    TRAP_IGNORE(SizeChangedL());
    }

// --------------------------------------------------------------------------------------------

void CSimpleFileViewListBox::SizeChangedL()
    {
    // disable separator line
    ItemDrawer()->ColumnData()->SetSeparatorLinePosition(ENoLine);

    // enable marquee
    ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);

    // get font
    ReleaseFonts();
    CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
    const TSize screenSize = dev->SizeInPixels();

    TFontSpec fontSpec = AknLayoutUtils::FontFromId(EAknLogicalFontPrimarySmallFont)->FontSpecInTwips();
    fontSpec.iHeight = IsQHD(screenSize) ? KSimpleViewFontHeightQHD : KSimpleViewFontHeight;
    dev->GetNearestFontInTwips(iFont, fontSpec);
    
    // set fonts
    ItemDrawer()->ColumnData()->SetColumnFontL(0, iFont);
    ItemDrawer()->ColumnData()->SetColumnFontL(1, iFont);
    ItemDrawer()->ColumnData()->SetColumnFontL(2, iFont);
    ItemDrawer()->ColumnData()->SetColumnFontL(3, iFont);

    // set row height
    TInt rowHeight = iFont->HeightInPixels()+4;    
    TSize itemCellSize = View()->ItemDrawer()->ItemCellSize();
    itemCellSize.iHeight = rowHeight; 
    SetItemHeightL(rowHeight);
    View()->ItemDrawer()->SetItemCellSize(itemCellSize);
    
    // set icon sizes
    TInt normalIconSize = rowHeight * 0.9;
    TInt markingIconSize = rowHeight * 0.8;
    ItemDrawer()->ColumnData()->SetSubCellIconSize(0, TSize(normalIconSize,normalIconSize));
    ItemDrawer()->ColumnData()->SetSubCellIconSize(2, TSize(markingIconSize,markingIconSize));
    ItemDrawer()->ColumnData()->SetSubCellIconSize(3, TSize(markingIconSize,markingIconSize));

    // set column widths
    ItemDrawer()->ColumnData()->SetColumnWidthPixelL(0, normalIconSize);
    ItemDrawer()->ColumnData()->SetColumnWidthPixelL(1, itemCellSize.iWidth-normalIconSize-markingIconSize);
    ItemDrawer()->ColumnData()->SetColumnWidthPixelL(2, markingIconSize);
    ItemDrawer()->ColumnData()->SetColumnWidthPixelL(3, markingIconSize);

    // set baseline for the text
    ItemDrawer()->ColumnData()->SetColumnBaselinePosL(1, iFont->HeightInPixels()-iFont->BaselineOffsetInPixels()-1);

    // set margins
    TMargins iconMargins;
    iconMargins.iBottom = 0;
    iconMargins.iLeft = 0;
    iconMargins.iRight = 0;
    iconMargins.iTop = 0;

    TMargins textMargins;
    textMargins.iBottom = 0;
    textMargins.iLeft = markingIconSize * 0.15;
    textMargins.iRight = markingIconSize * 0.15;
    textMargins.iTop = 0;

    ItemDrawer()->ColumnData()->SetColumnMarginsL(0, iconMargins);
    ItemDrawer()->ColumnData()->SetColumnMarginsL(1, textMargins);
    ItemDrawer()->ColumnData()->SetColumnMarginsL(2, iconMargins);
    ItemDrawer()->ColumnData()->SetColumnMarginsL(3, iconMargins);
    }

// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

CExtendedFileViewListBox::~CExtendedFileViewListBox()
    {
    ReleaseFonts();
    }

// --------------------------------------------------------------------------------------------

void CExtendedFileViewListBox::ReleaseFonts()
    {
    if (iFont1)
        {
        CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
        dev->ReleaseFont(iFont1);
        iFont1 = NULL;     
        }

    if (iFont2)
        {
        CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
        dev->ReleaseFont(iFont2);
        iFont2 = NULL;      
        }        
    }
    
// --------------------------------------------------------------------------------------------

void CExtendedFileViewListBox::SizeChanged()
    {
    // call the base class function first
    CAknDoubleGraphicStyleListBox::SizeChanged();

    // customize the list box
    TRAP_IGNORE(SizeChangedL());
    }

// --------------------------------------------------------------------------------------------

void CExtendedFileViewListBox::SizeChangedL()
    {
    // disable separator line
    ItemDrawer()->ColumnData()->SetSeparatorLinePosition(ENoLine);

    // enable marquee
    ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
    
    // get fonts
    ReleaseFonts();
    CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
    const TSize screenSize = dev->SizeInPixels();
    
    TFontSpec fontSpec1 = AknLayoutUtils::FontFromId(EAknLogicalFontPrimarySmallFont)->FontSpecInTwips();
    fontSpec1.iHeight = IsQHD(screenSize) ? KExtendedViewFontHeight1QHD : KExtendedViewFontHeight1;
    dev->GetNearestFontInTwips(iFont1, fontSpec1);

    TFontSpec fontSpec2 = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont)->FontSpecInTwips();
    fontSpec2.iHeight = IsQHD(screenSize) ? KExtendedViewFontHeight2QHD : KExtendedViewFontHeight2;
    dev->GetNearestFontInTwips(iFont2, fontSpec2);    

        
    // set fonts
    ItemDrawer()->ColumnData()->SetSubCellFontL(0, iFont1);
    ItemDrawer()->ColumnData()->SetSubCellFontL(1, iFont1);
    ItemDrawer()->ColumnData()->SetSubCellFontL(2, iFont2);
    ItemDrawer()->ColumnData()->SetSubCellFontL(3, iFont1);
    ItemDrawer()->ColumnData()->SetSubCellFontL(4, iFont1);

    // set row height
    TInt rowHeight1 = iFont1->HeightInPixels()+3;    
    TInt rowHeight2 = iFont2->HeightInPixels()+3;    
    TSize itemCellSize = View()->ItemDrawer()->ItemCellSize();
    itemCellSize.iHeight = rowHeight1+rowHeight2; 
    SetItemHeightL(rowHeight1+rowHeight2);
    View()->ItemDrawer()->SetItemCellSize(itemCellSize);

    
    // set icon sizes
    TInt normalIconSize = rowHeight1 * 0.9;
    TInt markingIconSize = rowHeight1 * 0.8;
    ItemDrawer()->ColumnData()->SetSubCellIconSize(0, TSize(normalIconSize,normalIconSize));
    ItemDrawer()->ColumnData()->SetSubCellIconSize(3, TSize(markingIconSize,markingIconSize));
    ItemDrawer()->ColumnData()->SetSubCellIconSize(4, TSize(markingIconSize,markingIconSize));

    // set sub cell sizes
    ItemDrawer()->ColumnData()->SetSubCellSizeL(0, TSize(normalIconSize,rowHeight1+rowHeight2));
    ItemDrawer()->ColumnData()->SetSubCellSizeL(1, TSize(itemCellSize.iWidth-normalIconSize-markingIconSize,rowHeight1));
    ItemDrawer()->ColumnData()->SetSubCellSizeL(2, TSize(itemCellSize.iWidth-normalIconSize-markingIconSize,rowHeight2));
    ItemDrawer()->ColumnData()->SetSubCellSizeL(3, TSize(markingIconSize,rowHeight1+rowHeight2));
    ItemDrawer()->ColumnData()->SetSubCellSizeL(4, TSize(markingIconSize,rowHeight1+rowHeight2));

    // set sub cell positions
    ItemDrawer()->ColumnData()->SetSubCellPositionL(0, TPoint(0,0));
    ItemDrawer()->ColumnData()->SetSubCellPositionL(1, TPoint(normalIconSize,0));
    ItemDrawer()->ColumnData()->SetSubCellPositionL(2, TPoint(normalIconSize,rowHeight1));
    ItemDrawer()->ColumnData()->SetSubCellPositionL(3, TPoint(itemCellSize.iWidth-normalIconSize-markingIconSize,0));
    ItemDrawer()->ColumnData()->SetSubCellPositionL(4, TPoint(itemCellSize.iWidth-normalIconSize-markingIconSize,0));
    
    // set baselines for the texts
    ItemDrawer()->ColumnData()->SetSubCellBaselinePosL(1, iFont1->HeightInPixels()-iFont1->BaselineOffsetInPixels()-1);
    ItemDrawer()->ColumnData()->SetSubCellBaselinePosL(2, rowHeight1+iFont2->HeightInPixels()-iFont2->BaselineOffsetInPixels()-1);
    
    // set margins
    TMargins iconMargins;
    iconMargins.iBottom = rowHeight2;
    iconMargins.iLeft = 0;
    iconMargins.iRight = 0;
    iconMargins.iTop = 0;

    TMargins textMargins;
    textMargins.iBottom = 0;
    textMargins.iLeft = markingIconSize * 0.15;
    textMargins.iRight = markingIconSize * 0.15;
    textMargins.iTop = 0;

    ItemDrawer()->ColumnData()->SetSubCellMarginsL(0, iconMargins);
    ItemDrawer()->ColumnData()->SetSubCellMarginsL(1, textMargins);
    ItemDrawer()->ColumnData()->SetSubCellMarginsL(2, textMargins);
    ItemDrawer()->ColumnData()->SetSubCellMarginsL(3, iconMargins);
    ItemDrawer()->ColumnData()->SetSubCellMarginsL(4, iconMargins);
    }
    
// --------------------------------------------------------------------------------------------
        
// End of File  
