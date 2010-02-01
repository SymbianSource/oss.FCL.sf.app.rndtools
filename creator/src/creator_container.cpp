/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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


#include <coemain.h>
#include <aknnotewrappers.h>
#include <creator.rsg>
#include <AknDef.h>
#include "creator_container.h"



// Standard construction sequence
CCreatorAppView* CCreatorAppView::NewL(const TRect& aRect)
    {
    CCreatorAppView* self = CCreatorAppView::NewLC(aRect);
    CleanupStack::Pop(self);
    return self;
    }

CCreatorAppView* CCreatorAppView::NewLC(const TRect& aRect)
    {
    CCreatorAppView* self = new (ELeave) CCreatorAppView;
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    return self;
    }

void CCreatorAppView::ConstructL(const TRect& aRect)
    {
    // Create a window for this application view
    CreateWindowL();

    // Set the windows size
    SetRect(aRect);

    // Activate the window, which makes it ready to be drawn
    ActivateL();

    }

CCreatorAppView::CCreatorAppView()
    {
    // Add any construction code that can not leave here
    }


CCreatorAppView::~CCreatorAppView()
    {
    // Add any destruction code here
    }


// Draw this application's view to the screen
void CCreatorAppView::Draw(const TRect& /*aRect*/) const
    {

    // Get the standard graphics context 
    CWindowGc& gc = SystemGc();

    // Gets the control's extent
    TRect rect = Rect();

    // Clears the screen
    gc.Clear(rect);

    // variables for memory and disk handling
    TDriveNumber cDrive=EDriveC;
    TDriveNumber dDrive=EDriveD;
    TDriveNumber eDrive=EDriveE;
    TVolumeInfo vinfo;
    TBuf<16> cFree;
    TBuf<16> dFree;
    TBuf<16> eFree;
    TBuf<16> cSize;
    TBuf<16> dSize;
    TBuf<16> eSize;
    TBuf<64> cText;
    TBuf<64> dText;
    TBuf<64> eText;
    TBuf<64> hText;

    // get an access to file server
    RFs& fsSession = ControlEnv()->FsSession();

    // check the C-drive
    fsSession.Volume(vinfo, cDrive);
    cFree.Num(TInt64(vinfo.iFree/1024));
    cSize.Num(TInt64(vinfo.iSize/1024));

    // format the information about the C-drive
    cText.Copy(_L("C: "));
    cText.Append(cFree);
    cText.Append(_L("kB of "));
    cText.Append(cSize);
    cText.Append(_L("kB"));


    // the same thing for D-drive
    fsSession.Volume(vinfo, dDrive);
    dFree.Num(TInt64(vinfo.iFree/1024));
    dSize.Num(TInt64(vinfo.iSize/1024));

    dText.Copy(_L("D: "));
    dText.Append(dFree);
    dText.Append(_L("kB of "));
    dText.Append(dSize);
    dText.Append(_L("kB"));


    // the same thing for E-drive (MMC), if it exists 
    if (MMC_OK())
        {
        fsSession.Volume(vinfo, eDrive);
        eFree.Num(TInt64(vinfo.iFree/1024));
        eSize.Num(TInt64(vinfo.iSize/1024));

        eText.Copy(_L("E: "));
        eText.Append(eFree);
        eText.Append(_L("kB of "));
        eText.Append(eSize);
        eText.Append(_L("kB"));
        }	


    // available work memory
    TMemoryInfoV1Buf memory;
    UserHal::MemoryInfo(memory);
    TInt64 freeMemBytes=(TInt64)(memory().iFreeRamInBytes);
    TInt64 sizeMemBytes=(TInt64)(memory().iTotalRamInBytes);

    /// ...
    TInt64 sizeWorkMemBytes = sizeMemBytes; 


    TBuf<16> freeMem;
    TBuf<16> sizeWorkMem;

    freeMem.Num(TInt64(freeMemBytes/1024));
    sizeWorkMem.Num(TInt64(sizeWorkMemBytes/1024));

    hText.Copy(freeMem);
    hText.Append(_L("kB of "));
    hText.Append(sizeWorkMem);
    hText.Append(_L("kB"));


    // draw all the texts to the screen
    gc.UseFont(AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont));
   
    TSize mainPaneSize;
    AknLayoutUtils::LayoutMetricsSize(AknLayoutUtils::EMainPane, mainPaneSize);
    
    TReal X_factor = TReal(mainPaneSize.iWidth) / 176;
    TReal Y_factor = TReal(mainPaneSize.iHeight) / 144;

    gc.SetPenColor(KRgbDarkBlue);
    gc.DrawText(_L("Free RAM memory:"), TPoint(TInt(4*X_factor), TInt(15*Y_factor)));
    gc.SetPenColor(KRgbBlack);
    gc.DrawText(hText, TPoint(TInt(4*X_factor), TInt(30*Y_factor)));

    gc.SetPenColor(KRgbDarkBlue);
    gc.DrawText(_L("Free disk memory:"), TPoint(TInt(4*X_factor), TInt(55*Y_factor)));
    gc.SetPenColor(KRgbBlack);
    gc.DrawText(cText, TPoint(TInt(4*X_factor), TInt(70*Y_factor)));
    gc.DrawText(dText, TPoint(TInt(4*X_factor), TInt(85*Y_factor)));

    if (MMC_OK())
        {
        gc.DrawText(eText, TPoint(TInt(4*X_factor), TInt(100*Y_factor)));
        }

    }

TKeyResponse CCreatorAppView::OfferKeyEventL(
		const TKeyEvent& aKeyEvent,TEventCode aType)
	{
	// return the default functionality
	return CCoeControl::OfferKeyEventL(aKeyEvent, aType);
	}


TCoeInputCapabilities CCreatorAppView::InputCapabilities() const
	{
	// this class does not implement any 'unusual' input capabilities
	return TCoeInputCapabilities::ENone;
	}


void CCreatorAppView::UserDraw() const
	{
	ActivateGc();
	Draw(Rect());
	DeactivateGc();
	}



TBool CCreatorAppView::MMC_OK() const
	{
	TBool MMCstatus = EFalse;

	TDriveNumber eDrive=EDriveE;
    TVolumeInfo vinfo;

	// check if we can access the E-drive:
	if ( iEikonEnv->FsSession().Volume(vinfo, eDrive) == KErrNone)
		{
		MMCstatus = ETrue;
		}
	else
		{
		MMCstatus = EFalse;
		}

	// returns ETrue if MMC working properly
	return MMCstatus;
	}


void CCreatorAppView::HandleResourceChange(TInt aType)
    {
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        }
    }

