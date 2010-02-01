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
#include "perfmon_valuescontainer.h"
#include "perfmon.hrh"
#include "perfmon_document.h"
#include "perfmon_appui.h"
#include "perfmon_model.h"

#include <aknutils.h>

_LIT(KPercentageFormat,"%S %d%%");
_LIT(KFreeFormat,"%S free %S%S"); 
_LIT(KSizeFormat,"%S size %S%S"); 

const TInt KLeftMargin = 2;


// ===================================== MEMBER FUNCTIONS =====================================

void CPerfMonValuesContainer::ConstructL(const TRect& aRect)
    {
    iModel = static_cast<CPerfMonDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();
    iFont = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont);
 
    CreateWindowL();
    SetRect(aRect);
    SetBlank();

    ActivateL();
    }

// --------------------------------------------------------------------------------------------

CPerfMonValuesContainer::~CPerfMonValuesContainer()
    {
    }
    
// --------------------------------------------------------------------------------------------

void CPerfMonValuesContainer::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetBrushColor(KRgbWhite);
    gc.Clear(aRect);
    
    // check if sample array has been constructed
    if (iModel->SampleEntryArray())
        {
        // init font
        gc.SetPenColor(KRgbBlack);
        gc.UseFont( iFont );
        TUint separator = iFont->HeightInPixels()-2;

        TInt c(1);

        // draw all entries
        for (TInt i=0; i<iModel->SampleEntryArray()->Count(); i++)
            {
            // check if data available
            if (iModel->SampleEntryArray()->At(i).iSampleDataArray->Count() > 0)
                {
                TSampleData& currentSample = iModel->SampleEntryArray()->At(i).iSampleDataArray->At(0);
                
                if (i == ESourceCPU) // for CPU draw %
                    {
                    TBuf<16> buf;
                    buf.Format(KPercentageFormat, &iModel->SampleEntryArray()->At(i).iDescription, currentSample.iSize > 0 ? TInt( (1 - ((TReal)(currentSample.iFree) / (TReal)currentSample.iSize)) * 100) : 0 );
                    gc.DrawText(buf, TPoint(KLeftMargin,separator*c));
                    c++;
                    }
                
                else if (currentSample.iSize > 0) // ram+drives, ignore absent drives
                    {
                    TBuf<32> amountBuf;
                    amountBuf.AppendNum(currentSample.iFree, TRealFormat(KDefaultRealWidth, 0));
        
                    TBuf<32> buf;
                    buf.Format(KFreeFormat, &iModel->SampleEntryArray()->At(i).iDescription, &amountBuf, &iModel->SampleEntryArray()->At(i).iUnitTypeShort);
                    gc.DrawText(buf, TPoint(KLeftMargin,separator*c));
                    c++;
                    
                    amountBuf.Copy(KNullDesC);
                    amountBuf.AppendNum(currentSample.iSize, TRealFormat(KDefaultRealWidth, 0));
        
                    buf.Format(KSizeFormat, &iModel->SampleEntryArray()->At(i).iDescription, &amountBuf, &iModel->SampleEntryArray()->At(i).iUnitTypeShort);
                    gc.DrawText(buf, TPoint(KLeftMargin,separator*c));
                    c++;            
                    }                    
                }
            }
           
        gc.DiscardFont();        
        }
    }

// --------------------------------------------------------------------------------------------

TKeyResponse CPerfMonValuesContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    return CCoeControl::OfferKeyEventL(aKeyEvent, aType);
    }
        
// --------------------------------------------------------------------------------------------

void CPerfMonValuesContainer::HandleResourceChange(TInt aType)
    {
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        }
    else
        CCoeControl::HandleResourceChange(aType);    
    }

// --------------------------------------------------------------------------------------------

void CPerfMonValuesContainer::DrawUpdate()
    {
    DrawDeferred(); 
    }
    
// --------------------------------------------------------------------------------------------
       
// End of File  
