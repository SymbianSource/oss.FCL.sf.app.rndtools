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
#include "SGMainContainer.h"


#include <AknBidiTextUtils.h>
#include <AknUtils.h>
#include <aknnotewrappers.h>
#include <eiksbfrm.h>
#include <AknsDrawUtils.h> 
#include <AknsBasicBackgroundControlContext.h>
#include <akntitle.h>
#include <eikspane.h>  
#include <AknDef.h>

const TUint KLeftMargin = 2;
const TUint KRightMargin = 2;


// ================= MEMBER FUNCTIONS =======================


void CScreenGrabberMainContainer::ConstructL(const TRect& aRect)
    {
    iSkinContext = NULL;

    CreateWindowL();
    Window().SetShadowDisabled(EFalse);
    
    SetBlank();
    
    // set title of the app
    CEikStatusPane* statusPane = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* title = static_cast<CAknTitlePane*>( statusPane->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    title->SetTextL( _L("Screen Grabber") );

    iText = HBufC::NewL(200000);
    iCurrentLine = 0;
    iLineCount = 0;
    iNumberOfLinesFitsScreen = 0;
    iX_factor = 1;
    iY_factor = 1;
    iLeftDrawingPosition = KLeftMargin;

    // get font
    iFont = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont);

    iWrappedArray = new(ELeave) CArrayFixFlat<TPtrC>(500);
    
    SetRect(aRect);
    ActivateL(); 

    PrintText(_L("Please use the settings screen to define the capture mode and any other related settings such capture hotkey and saving format. Using the PNG format for screen shots is recommended, since it is compressed but lossless. The captures can be managed with the Media Gallery application.\n\nYou may start taking screenshots or capturing video now. Please press the Applications key to go back to the Application menu or send this application to the background via the Options menu. To be able to take screenshots or capture video, this applications needs to stay running in the background.\n"));
    }

CScreenGrabberMainContainer::~CScreenGrabberMainContainer()
    {
    if (iSkinContext)
    	delete iSkinContext;
    
	if (iWrappedArray)
		{
    	iWrappedArray->Reset();
    	delete iWrappedArray;
		}

	if (iText)
    	delete iText;

    if (iScrollBarFrame)
        delete iScrollBarFrame;

    if (iWrapperString)
        delete iWrapperString;
    }


// ---------------------------------------------------------
// CScreenGrabberMainContainer::UpdateVisualContentL()
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::UpdateVisualContentL(TBool aScrollToBottom)
    {
    TSize rectSize;
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, rectSize);
    TRect rect(rectSize);

    // set the correct drawing area for the skin background
    if(iSkinContext)
        {
        delete iSkinContext;
        iSkinContext = NULL;
        }  
    iSkinContext = CAknsBasicBackgroundControlContext::NewL(KAknsIIDQsnBgAreaMain, rectSize, EFalse);

    // init scroll bar if not yet done
    if (!iScrollBarFrame)
        {
        iScrollBarFrame = new(ELeave) CEikScrollBarFrame(this, this, ETrue);

        CAknAppUi* appUi = iAvkonAppUi;
        
	    if (AknLayoutUtils::DefaultScrollBarType(appUi) == CEikScrollBarFrame::EDoubleSpan)
	        {
		    iScrollBarFrame->CreateDoubleSpanScrollBarsL(ETrue, EFalse);            
		    iScrollBarFrame->SetTypeOfVScrollBar(CEikScrollBarFrame::EDoubleSpan);
	        }
	    else
	        {
		    iScrollBarFrame->SetTypeOfVScrollBar(CEikScrollBarFrame::EArrowHead);
	        }  
	    iScrollBarFrame->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
        }

    // drawing positions, needed for scalable ui
    iX_factor = TReal(rect.Width()) / 176;
    iY_factor = TReal(rect.Height()) / 144;

    iWrappedArray->Reset();

    if (iWrapperString)
        {
        delete iWrapperString;
        iWrapperString = NULL;
        }

    // define drawing width, some extra space needed if using double span scrolling bar
    TUint drawingWidth(0);
    if (iScrollBarFrame->TypeOfVScrollBar() == CEikScrollBarFrame::EDoubleSpan)
        {
        if (AknLayoutUtils::LayoutMirrored())  // scroll bar on 'left'
            {
            iLeftDrawingPosition = KLeftMargin + 12;
            drawingWidth = TInt( rect.Width() - iLeftDrawingPosition*iX_factor - (KRightMargin*iX_factor)); 
            }
        else // scroll bar on 'right'
            {
            iLeftDrawingPosition = KLeftMargin;  
            drawingWidth = TInt( rect.Width() - iLeftDrawingPosition*iX_factor - (KRightMargin*iX_factor + 7*iX_factor)); 
            }        
        }
    else
        drawingWidth = TInt( rect.Width() - KLeftMargin*iX_factor - KRightMargin*iX_factor);

    // wrap the text
    iWrapperString = AknBidiTextUtils::ConvertToVisualAndWrapToArrayL(
        iText->Des(),
        drawingWidth,
        *iFont,
        *iWrappedArray
        );
    
    iLineCount = iWrappedArray->Count();

    // count amount of lines fits to screen
    iNumberOfLinesFitsScreen = TInt(rect.Height() / (iFont->HeightInPixels()));

    // check if needed to scroll to the bottom
    if (aScrollToBottom && iCurrentLine < iLineCount - iNumberOfLinesFitsScreen)
        {
        iCurrentLine = iLineCount-iNumberOfLinesFitsScreen;
        }

    // update the scroll bars
	TEikScrollBarModel horizontalBar;
    TEikScrollBarModel verticalBar;
    verticalBar.iThumbPosition = iCurrentLine;
    verticalBar.iScrollSpan = iLineCount - iNumberOfLinesFitsScreen + 1;
    verticalBar.iThumbSpan = 1;

    TEikScrollBarFrameLayout layout;
	layout.iTilingMode = TEikScrollBarFrameLayout::EInclusiveRectConstant;

    if (iScrollBarFrame->TypeOfVScrollBar() == CEikScrollBarFrame::EDoubleSpan)
        {    
        // do not let scrollbar values overflow
        if (verticalBar.iThumbPosition + verticalBar.iThumbSpan > verticalBar.iScrollSpan)
            verticalBar.iThumbPosition = verticalBar.iScrollSpan - verticalBar.iThumbSpan;
		
        TAknDoubleSpanScrollBarModel horizontalDSBar(horizontalBar);
        TAknDoubleSpanScrollBarModel verticalDSBar(verticalBar);
       
        iScrollBarFrame->TileL(&horizontalDSBar, &verticalDSBar, rect, rect, layout);        
        iScrollBarFrame->SetVFocusPosToThumbPos(verticalDSBar.FocusPosition());
	    }  
	else
	    {
		iScrollBarFrame->TileL( &horizontalBar, &verticalBar, rect, rect, layout );
		iScrollBarFrame->SetVFocusPosToThumbPos( verticalBar.iThumbPosition );
	    }


    // update the screen
    DrawNow();
    }

// ---------------------------------------------------------
// CScreenGrabberMainContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();

    MAknsSkinInstance* skin = AknsUtils::SkinInstance();
    if (iSkinContext)
	    {
        // draws the skin background
	    AknsDrawUtils::Background(skin, iSkinContext, this, gc, aRect);
	    }
    else
        {
        gc.Clear(aRect);
        }


    // get the text color from the skin   
    TRgb skinPenColor;
    if (AknsUtils::GetCachedColor( skin, skinPenColor, KAknsIIDQsnTextColors, EAknsCIQsnTextColorsCG6 ) == KErrNone)
        {
		gc.SetPenColor(skinPenColor);
		}	
    else
        {
        gc.SetPenColor(KRgbBlack);
        }

    gc.UseFont(iFont);

    // draw the text
    for (TInt i=0; i<iNumberOfLinesFitsScreen; i++)
        {
        // index out of bounds check and then draw text
        if (i+iCurrentLine < iWrappedArray->Count())
            gc.DrawText(iWrappedArray->At(i+iCurrentLine), TPoint(TInt(iLeftDrawingPosition*iX_factor), TInt(iFont->HeightInPixels()*(i+1))));
        }

    gc.DiscardFont();
    }

// ---------------------------------------------------------
// CScreenGrabberMainContainer::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CScreenGrabberMainContainer::PrintText(const TDesC& aDes)
// Print text into output window.
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::PrintText(const TDesC& aDes)
    {
    iText->Des().Append( aDes );
    UpdateVisualContentL(EFalse);  // do not automatically scroll to the bottom
    }

// ---------------------------------------------------------
// CScreenGrabberMainContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,
//                                    TEventCode aType)
// Handle key event. Only up and down key arrow events are
// consumed in order to enable scrolling in output window.
// ---------------------------------------------------------
//
TKeyResponse CScreenGrabberMainContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
    {
    if(aType != EEventKey)
        return EKeyWasNotConsumed;
    
    if(aKeyEvent.iCode == EKeyUpArrow)
        {
        if (iCurrentLine > 0)
            {
            iCurrentLine--;
            UpdateVisualContentL(EFalse);
            }

        return EKeyWasConsumed;
        }
    
    else if(aKeyEvent.iCode == EKeyDownArrow)
        {
        if (iCurrentLine < iLineCount - iNumberOfLinesFitsScreen)
            {
            iCurrentLine++;
            UpdateVisualContentL(EFalse);
            }

        return EKeyWasConsumed;
        }
    
    return EKeyWasNotConsumed;
    }


// ---------------------------------------------------------
// CScreenGrabberMainContainer::ClearOutputWindow()
// Clear the output window.
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::ClearOutputWindow()
    {
    iText->Des() = _L("");
    UpdateVisualContentL(ETrue);
    }


// ---------------------------------------------------------
// CScreenGrabberMainContainer::HandleResourceChange(TInt aType)
// Handle layout change of the screen
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::HandleResourceChange(TInt aType)
    {
    if (aType == KEikDynamicLayoutVariantSwitch || aType == KAknsMessageSkinChange)
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);

        iCurrentLine = 0; // scroll to top

        // update font
        iFont = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont);

        // delete the scroll and update screen
        if (iScrollBarFrame)
            {
            delete iScrollBarFrame;
            iScrollBarFrame = NULL;
            }
        UpdateVisualContentL(EFalse);
        }
    else
        {
        CCoeControl::HandleResourceChange(aType);
        }
    }

// ---------------------------------------------------------
// CScreenGrabberMainContainer::MopSupplyObject(TTypeUid aId)
// Pass skin information if needed
// ---------------------------------------------------------
//
TTypeUid::Ptr CScreenGrabberMainContainer::MopSupplyObject(TTypeUid aId)
    {
    if (aId.iUid == MAknsControlContext::ETypeId && iSkinContext)
        {
        return MAknsControlContext::SupplyMopObject(aId, iSkinContext);
        }

    return CCoeControl::MopSupplyObject(aId);
    }


// ---------------------------------------------------------
// CScreenGrabberMainContainer::HandleScrollEventL()
// Capture touch events on the scroll bar
// ---------------------------------------------------------
//
void CScreenGrabberMainContainer::HandleScrollEventL(CEikScrollBar* aScrollBar, TEikScrollEvent aEventType)
    {
    // only on page up/down,scroll up/down and drag events
    if ((aEventType == EEikScrollPageDown) || (aEventType == EEikScrollPageUp) || 
       (aEventType == EEikScrollThumbDragVert) || (aEventType == EEikScrollUp) ||
       (aEventType == EEikScrollDown))
        {
        // get the current position from the scroll bar
        iCurrentLine = aScrollBar->ThumbPosition();
        
        // refresh now
        UpdateVisualContentL(EFalse);
        }
    }

    
// End of File  
