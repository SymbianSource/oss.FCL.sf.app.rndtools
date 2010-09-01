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



#ifndef SCREENGRABBERMAINCONTAINER_H
#define SCREENGRABBERMAINCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eiksbobs.h>

// FORWARD DECLARATIONS
class CEikScrollBarFrame;
class CAknsBasicBackgroundControlContext;


// CLASS DECLARATION

/**
*  CScreenGrabberMainContainer  container control class.
*  
*/
class CScreenGrabberMainContainer : public CCoeControl, MCoeControlObserver, MEikScrollBarObserver
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        ~CScreenGrabberMainContainer();

    public: // New functions

        void PrintText(const TDesC& aDes);
        void ClearOutputWindow();


    public: // Functions from base classes

    private: // Functions from base classes

        void Draw(const TRect& aRect) const;
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        TTypeUid::Ptr MopSupplyObject(TTypeUid aId);

    public:
        void HandleResourceChange(TInt aType);
        void HandleScrollEventL(CEikScrollBar* aScrollBar,TEikScrollEvent aEventType); // From MEikScrollBarObserver
        
	private:
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode /*aType*/);
    
    private:
        void UpdateVisualContentL(TBool aScrollToBottom);

    private: //data
        CEikScrollBarFrame*     iScrollBarFrame;
        HBufC*                  iText;
        const CFont*            iFont;
        HBufC*                  iWrapperString;
        CArrayFix<TPtrC>*       iWrappedArray;
        TInt                    iCurrentLine;
        TInt                    iLineCount;
        TInt                    iNumberOfLinesFitsScreen;
        TInt                    iLeftDrawingPosition;
        TReal                   iX_factor;
        TReal                   iY_factor;

        CAknsBasicBackgroundControlContext* iSkinContext;


    };

#endif

// End of File