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
#include "LauncherContainerApps.h"
#include "LauncherEngine.h"


#include <eikclb.h>
#include <eikclbd.h>
#include <aknconsts.h>
#include <aknutils.h>
#include <aknnotewrappers.h>
#include <akniconarray.h> 
#include <f32file.h>
#include <avkon.mbg>
#include <AknIconUtils.h>
#include <akndef.h>


_LIT(KTab,"\t");


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CLauncherContainerApps::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CLauncherContainerApps::ConstructL(const TRect& aRect, CLauncherEngine* aEngine)
    {
    iEngine = aEngine;

    CreateWindowL();
    Window().SetShadowDisabled(EFalse);
    SetRect(aRect);

    iListBox = new(ELeave) CAknSingleStyleListBox;
    iListBox->SetContainerWindowL(*this);
    iListBox->ConstructL(this, EAknListBoxMarkableList);
    iListBox->View()->SetListEmptyTextL(_L("No apps found"));

    iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);

    CAknIconArray* iconArray = new(ELeave) CAknIconArray(1);
    CleanupStack::PushL(iconArray);
    CFbsBitmap* markBitmap = NULL;
    CFbsBitmap* markBitmapMask = NULL;
    
    TRgb defaultColor;
    defaultColor = CEikonEnv::Static()->Color(EColorControlText);

    AknsUtils::CreateColorIconL(AknsUtils::SkinInstance(),
            KAknsIIDQgnIndiMarkedAdd,
            KAknsIIDQsnIconColors,
            EAknsCIQsnIconColorsCG13,
            markBitmap,
            markBitmapMask,
            AknIconUtils::AvkonIconFileName(),
            EMbmAvkonQgn_indi_marked_add,
            EMbmAvkonQgn_indi_marked_add_mask,
            defaultColor
            );
     
    CGulIcon* markIcon = CGulIcon::NewL(markBitmap,markBitmapMask);
    iconArray->AppendL(markIcon);                       
    CleanupStack::Pop(); // iconArray

    iListBox->ItemDrawer()->ColumnData()->SetIconArray(iconArray);

    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
    iListBox->SetListBoxObserver(this);
    
    // set size of the listbox
    TSize outputRectSize;
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, outputRectSize);
    TRect outputRect(outputRectSize);
    iListBox->SetRect(outputRect);


    iListBox->ActivateL();
   
    //ActivateL();  
    }

// Destructor
CLauncherContainerApps::~CLauncherContainerApps()
    {
    delete iListBox;
    iListBox = NULL;
    }

// ---------------------------------------------------------

const CArrayFix<TInt>* CLauncherContainerApps::SelectedApps()
    {
    const CListBoxView::CSelectionIndexArray* indices = iListBox->SelectionIndexes();
    
    return static_cast<const CArrayFix<TInt>*>(indices);
    }

// ---------------------------------------------------------

void CLauncherContainerApps::UpdateFileListL()
    {
    // get list of all apps from the engine
	CDesCArray* appFullPathsArray = iEngine->ListOfAllAppsL();
		
    // get an instance of the listbox's item array
	MDesCArray* tempArray = iListBox->Model()->ItemTextArray();
	CDesCArray* listBoxArray = (CDesCArray*)tempArray;
	listBoxArray->Reset();

	for ( TInt i=0; i<appFullPathsArray->Count(); i++)
	    {
        // parse the filename
        TParse nameParser;
        TInt err = nameParser.SetNoWild((*appFullPathsArray)[i], NULL, NULL);
        
        if (!err)
		    {
            TFileName name;
            name.Copy(KTab);
            name.Append(nameParser.Drive());
            name.Append(nameParser.NameAndExt());
            name.Append(KTab);
            name.Append(KTab);

		    listBoxArray->AppendL( name );
            }
        else
            {
		    listBoxArray->AppendL( _L("\t*invalid file*\t\t") );
            }
        }

    // updates the listbox
    iListBox->HandleItemAdditionL();
    }

// ---------------------------------------------------------
// CLauncherContainerApps::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CLauncherContainerApps::SizeChanged()
{
    TSize outputRectSize;
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, outputRectSize);
    TRect outputRect(outputRectSize);
    
    if (iListBox)
        iListBox->SetRect(outputRect);
}


// ---------------------------------------------------------
// CLauncherContainerApps::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CLauncherContainerApps::CountComponentControls() const
    {
    return 1; // return nbr of controls inside this container
    }

// ---------------------------------------------------------
// CLauncherContainerApps::ComponentControl(TInt aIndex) const
// ---------------------------------------------------------
//
CCoeControl* CLauncherContainerApps::ComponentControl(TInt /*aIndex*/) const
    {
    return iListBox;
    }

// ---------------------------------------------------------
// CBCAppMainContainer::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CLauncherContainerApps::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();
    gc.Clear(aRect);
    }

// ---------------------------------------------------------
// CLauncherContainerApps::HandleControlEventL(
//     CCoeControl* aControl,TCoeEvent aEventType)
// ---------------------------------------------------------
//
void CLauncherContainerApps::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    }


// ---------------------------------------------------------
// CLauncherContainerApps::OfferKeyEventL(const TKeyEvent& aKeyEvent,
//                                    TEventCode aType)
// Handle key event. Only up and down key arrow events are
// consumed in order to enable scrolling in output window.
// ---------------------------------------------------------
//
TKeyResponse CLauncherContainerApps::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
    {
    if(aType != EEventKey)
        return EKeyWasNotConsumed;

    // handle OK key
    if (aKeyEvent.iCode == EKeyOK)
        {
        if (iListBox->View()->ItemIsSelected(iListBox->CurrentItemIndex()))
            {
            AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdUnmark, iListBox );
            }
        else
            {
            AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdMark, iListBox );
            }

        return EKeyWasNotConsumed;
        }
    
    // left and right keys are needed by other class
    else if (aKeyEvent.iCode == EKeyLeftArrow || aKeyEvent.iCode == EKeyRightArrow)
        return EKeyWasNotConsumed;

    else
        return iListBox->OfferKeyEventL(aKeyEvent, aType);
    }

// ---------------------------------------------------------

void CLauncherContainerApps::HandleListBoxEventL(CEikListBox* /*aListBox*/, TListBoxEvent aEventType)
    {
    switch (aEventType)
        {
        case EEventEnterKeyPressed:
        case EEventItemDoubleClicked:
            {
            if (iListBox->View()->ItemIsSelected(iListBox->CurrentItemIndex()))
                {
                AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdUnmark, iListBox );
                }
            else
                {
                AknSelectionService::HandleMarkableListProcessCommandL( EAknCmdMark, iListBox );
                }            
            }
            break;
        default:
            break;
        }
    }
    
// ---------------------------------------------------------
//

void CLauncherContainerApps::HandleResourceChange(TInt aType)
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);

        TSize outputRectSize;
        AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, outputRectSize);
        TRect outputRect(outputRectSize);
        iListBox->SetRect(outputRect);
        }
    else
        {
        CCoeControl::HandleResourceChange(aType);
        }
    }


// End of File  
