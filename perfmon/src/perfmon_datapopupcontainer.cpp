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
#include "perfmon_datapopupcontainer.h"
#include "perfmon.hrh"
#include "perfmon_document.h"
#include "perfmon_appui.h"
#include "perfmon_model.h"

#include <AknUtils.h>

_LIT(KPercentageFormat,"%S %d%%");
_LIT(KFreeFormat,"%S free %S%S");

const TInt KLeftMargin = 2;
 

// ===================================== MEMBER FUNCTIONS =====================================

void CPerfMonDataPopupContainer::ConstructL(const TRect& /*aRect*/)
    {
    iModel = static_cast<CPerfMonDocument*>(reinterpret_cast<CEikAppUi*>(iEikonEnv->AppUi())->Document())->Model();
    iFont = LatinPlain12();
    iFontSize = iFont->FontMaxHeight();
 
    // set windowgroup so that it always on top and does not receive focus
    iWindowGroup = RWindowGroup(iCoeEnv->WsSession());
    User::LeaveIfError(iWindowGroup.Construct((TUint32)&iWindowGroup));
    iWindowGroup.SetOrdinalPosition(0, ECoeWinPriorityAlwaysAtFront);
    iWindowGroup.EnableReceiptOfFocus(EFalse);
    
    CreateWindowL(&iWindowGroup);
    //SetRect(aRect);
    SetPositionAndSize();
    SetBlank();

    ActivateL();
    }

// --------------------------------------------------------------------------------------------

CPerfMonDataPopupContainer::~CPerfMonDataPopupContainer()
    {
    iWindowGroup.Close();    
    }
    
// --------------------------------------------------------------------------------------------

void CPerfMonDataPopupContainer::Draw(const TRect& aRect) const
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
        
        // draw a rect around the popup
        gc.DrawRect(aRect);
        
        TInt posCounter(1);
        
        for (TInt i=0; i<iModel->SampleEntryArray()->Count(); i++)
            {
            // check if this setting has been enabled and it has some data
            if (iModel->Settings().iDataPopupSources.iSrcEnabled[i] && iModel->SampleEntryArray()->At(i).iSampleDataArray->Count() > 0)
                {
                TSampleData& currentSample = iModel->SampleEntryArray()->At(i).iSampleDataArray->At(0);    
                TBuf<32> buf;

                // for CPU draw %, other amount of free memory
                if (i == ESourceCPU)
                    {
                    buf.Format(KPercentageFormat, &iModel->SampleEntryArray()->At(i).iDescription, currentSample.iSize > 0 ? TInt( (1 - ((TReal)(currentSample.iFree) / (TReal)currentSample.iSize)) * 100) : 0 );
                    gc.DrawText(buf, TPoint(KLeftMargin, iFontSize*posCounter));
                    }
                else
                    {
                    TBuf<32> freeBuf;
                    freeBuf.AppendNum(currentSample.iFree, TRealFormat(KDefaultRealWidth, 0));

                    TBuf<32> buf;
                    buf.Format(KFreeFormat, &iModel->SampleEntryArray()->At(i).iDescription, &freeBuf, &iModel->SampleEntryArray()->At(i).iUnitTypeShort);
                    gc.DrawText(buf, TPoint(KLeftMargin, iFontSize*posCounter));                    
                    }    
                
                posCounter++;
                }
            }
            
        gc.DiscardFont();        
        }
    }

// --------------------------------------------------------------------------------------------

void CPerfMonDataPopupContainer::SizeChanged()
    {
    DrawNow();
    }   

// --------------------------------------------------------------------------------------------

void CPerfMonDataPopupContainer::SetPositionAndSize()
    {
    CWsScreenDevice* screenDevice = iEikonEnv->ScreenDevice();

    // top right
    if (iModel->Settings().iDataPopupLocation == EDataPopupLocationTopRight)
        {
        // screen orientation is landscape with softkeys on right
        if (AknLayoutUtils::CbaLocation()==AknLayoutUtils::EAknCbaLocationRight)
            {
            SetRect(
                TRect(
                    screenDevice->SizeInPixels().iWidth-102-15,
                    0,
                    screenDevice->SizeInPixels().iWidth-15,
                    iModel->Settings().iDataPopupSources.EnabledSourcesCount()*iFontSize + 3
                    ));
            }

        // any other orientation
        else
            {
            SetRect(
                TRect(
                    screenDevice->SizeInPixels().iWidth-102,
                    0,
                    screenDevice->SizeInPixels().iWidth,
                    iModel->Settings().iDataPopupSources.EnabledSourcesCount()*iFontSize + 3
                    ));
            }        
        }

    // bottom middle
    else if (iModel->Settings().iDataPopupLocation == EDataPopupLocationBottomMiddle)
        {
        SetRect(
            TRect(
                screenDevice->SizeInPixels().iWidth/2-102/2,
                screenDevice->SizeInPixels().iHeight - iModel->Settings().iDataPopupSources.EnabledSourcesCount()*iFontSize - 3,
                screenDevice->SizeInPixels().iWidth/2+102/2,
                screenDevice->SizeInPixels().iHeight
                ));
        }
    }

// --------------------------------------------------------------------------------------------

void CPerfMonDataPopupContainer::UpdateVisibility(TBool aForeground)
    {
    // application has been brought to foregound
    if (aForeground)
        {
        if (iModel->Settings().iDataPopupVisibility==EDataPopupVisbilityAlwaysOn)
            {
            MakeVisible(ETrue);
            }
        else
            {
            MakeVisible(EFalse);
            }    
        }
    
    // application has been sent to background
    else
        {
        if (iModel->Settings().iDataPopupVisibility==EDataPopupVisbilityAlwaysOn
            || iModel->Settings().iDataPopupVisibility==EDataPopupVisbilityBackgroundOnly)
            {
            MakeVisible(ETrue);
            }
        else
            {
            MakeVisible(EFalse);
            }              
        }    
    }
            
// --------------------------------------------------------------------------------------------

void CPerfMonDataPopupContainer::DrawUpdate()
    {
    DrawDeferred(); 
    }
    
// --------------------------------------------------------------------------------------------
       
// End of File  
