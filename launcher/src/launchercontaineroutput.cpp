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
#include "LauncherContainerOutput.h"
#include "LauncherEngine.h"


#include <AknBidiTextUtils.h>
#include <aknutils.h>
#include <aknnotewrappers.h>
#include <eiksbfrm.h>
#include <akndef.h>

const TUint KLeftMargin = 2;
const TUint KRightMargin = 2;


// ================= MEMBER FUNCTIONS =======================


void CLauncherContainerOutput::ConstructL(const TRect& aRect, CLauncherEngine* aEngine)
    {
    iEngine = aEngine;

    CreateWindowL();
    Window().SetShadowDisabled(EFalse);
    
    SetBlank();

    iText = HBufC::NewL(200000);
    iCurrentLine = 0;
    iLineCount = 0;
    iNumberOfLinesFitsScreen = 0;
    iX_factor = 1;
    iY_factor = 1;
    iLeftDrawingPosition = KLeftMargin;

    iFont = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont);

    iWrappedArray = new(ELeave) CArrayFixFlat<TPtrC>(500);

    
    SetRect(aRect);
    //ActivateL(); 


    PrintTextL(_L("Ready.\n\n"));

    }

CLauncherContainerOutput::~CLauncherContainerOutput()
    {
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
// CLauncherContainerOutput::UpdateVisualContentL()
// ---------------------------------------------------------
//
void CLauncherContainerOutput::UpdateVisualContentL(TBool aScrollToBottom, TBool aScrollToTop)
    {
    TSize rectSize;
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, rectSize);
    TRect rect(rectSize);

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
    if (aScrollToBottom )
        {
        if( iCurrentLine < iLineCount - iNumberOfLinesFitsScreen)
            {
            iCurrentLine = iLineCount-iNumberOfLinesFitsScreen;
            }
        }
    else if( aScrollToTop )
        {
        iCurrentLine = 0;
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
// CLauncherContainerOutput::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CLauncherContainerOutput::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();
    gc.Clear(aRect);
    gc.SetPenColor(KRgbBlack);
    gc.UseFont(iFont);

    // draw the text
    for (TInt i=0; i<iNumberOfLinesFitsScreen; i++)
        {
        // index out of bounds check and then draw text
        if (i+iCurrentLine < iWrappedArray->Count())
            gc.DrawText(iWrappedArray->At(i+iCurrentLine), TPoint(TInt(iLeftDrawingPosition*iX_factor), TInt(iFont->HeightInPixels()*(i+1))));
        }
    }

// ---------------------------------------------------------
// CLauncherContainerOutput::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CLauncherContainerOutput::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }

// ---------------------------------------------------------
// CLauncherContainerOutput::PrintTextL(const TDesC& aDes)
// Print text into output window.
// ---------------------------------------------------------
//
void CLauncherContainerOutput::PrintTextL(const TDesC& aDes)
    {
    iText->Des().Append( aDes );
    UpdateVisualContentL(ETrue);
    }

// ---------------------------------------------------------
// CLauncherContainerOutput::OfferKeyEventL(const TKeyEvent& aKeyEvent,
//                                    TEventCode aType)
// Handle key event. Only up and down key arrow events are
// consumed in order to enable scrolling in output window.
// ---------------------------------------------------------
//
TKeyResponse CLauncherContainerOutput::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
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
// CLauncherContainerOutput::ClearOutputWindowL()
// Clear the output window.
// ---------------------------------------------------------
//
void CLauncherContainerOutput::ClearOutputWindowL()
    {
    iText->Des() = _L("");
    UpdateVisualContentL(EFalse, ETrue);
    }

// ---------------------------------------------------------
// CLauncherContainerOutput::HandleResourceChange(TInt aType)
// Handle layout change of the screen
// ---------------------------------------------------------
//

void CLauncherContainerOutput::HandleResourceChange(TInt aType)
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )
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
        TRAP_IGNORE(UpdateVisualContentL(EFalse));
        }
    else
        {
        CCoeControl::HandleResourceChange(aType);
        }
    }

// ---------------------------------------------------------
// CLauncherContainerOutput::HandleScrollEventL()
// Capture touch events on the scroll bar
// ---------------------------------------------------------
//
void CLauncherContainerOutput::HandleScrollEventL(CEikScrollBar* aScrollBar, TEikScrollEvent aEventType)
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
