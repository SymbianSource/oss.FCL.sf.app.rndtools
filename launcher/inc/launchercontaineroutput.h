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


#ifndef LAUNCHERCONTAINEROUTPUT_H
#define LAUNCHERCONTAINEROUTPUT_H

// INCLUDES
#include <coecntrl.h>
#include <eiksbobs.h>

class CEikRichTextEditor;
class CEikScrollBarFrame;
class CLauncherEngine; 


// FORWARD DECLARATIONS


// CLASS DECLARATION

/**
*  CLauncherContainerOutput  container control class.
*  
*/
class CLauncherContainerOutput : public CCoeControl, MCoeControlObserver, MEikScrollBarObserver
    {
    public: // Constructors and destructor
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect, CLauncherEngine* aEngine);

        /**
        * Destructor.
        */
        ~CLauncherContainerOutput();

    public: // New functions

        void PrintTextL(const TDesC& aDes);
        void ClearOutputWindowL();


    public: // Functions from base classes

    private: // Functions from base classes

        void Draw(const TRect& aRect) const;
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        void HandleScrollEventL(CEikScrollBar* aScrollBar,TEikScrollEvent aEventType); // From MEikScrollBarObserver

    public:
        void HandleResourceChange(TInt aType);

	private:
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode /*aType*/);
    
    private:
        /**
         * Updates the containers content in screen.
         * @param aScrollToBottom If true, screen is scrolled to bottom.
         * @param aScrollToTop If true, screen is scrolled to top. Ignored if
         * aScrollToBottom is set true.
         */
        void UpdateVisualContentL(TBool aScrollToBottom, TBool aScrollToTop = EFalse);

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

        CLauncherEngine* iEngine;

    };

#endif

// End of File
