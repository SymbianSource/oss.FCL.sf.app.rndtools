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
* Description:  Functional implentation of pointer event service.
*
*/


#ifndef CPOINTEREVENTHANDLER_H
#define CPOINTEREVENTHANDLER_H

// INCLUDES
#include <HTIServicePluginInterface.h>
#include <w32std.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Functional implentation of pointer event service.
*/
class CPointerEventHandler : public CActive
    {
    public:

        enum TError
            {
            EMissingCommand          = 0x01,
            EUnrecognizedCommand     = 0x02,
            ENotReadyCommand         = 0x03,
            EInvalidParameters       = 0x9A
            };

        enum TCommand
            {
            ETapScreen = 0x10,
            ETapAndDrag = 0x11,
            ETapAndDragMultipoint = 0x12,
            EPressPointerDown = 0x13,
            ELiftPointerUp = 0x14,
            EResultOk = 0xFF // only for response message
            };

        enum TPointerState
            {
            EPointerUp,
            EPointerDown,
            EPointerMoving
            };

        /**
        * Two-phased constructor.
        */
        static CPointerEventHandler* NewL();

        /**
        * Destructor.
        */
        virtual ~CPointerEventHandler();

        /**
        * Called by the HtiKeyEventServicePlugin when a pointer event
        * command is received.
        * @param aMessage message body destinated to the servive
        * @param aPriority message priority
        */
        void ProcessMessageL( const TDesC8& aMessage,
            THtiMessagePriority aPriority );

        /**
        * Sets the dispatcher to send outgoing messages to.
        * @param aDispatcher pointer to dispatcher instance
        */
        void SetDispatcher( MHtiDispatcher* aDispatcher );


    protected: // Functions from base classes

        // From CActive
        void RunL();
        TInt RunError(TInt aError);
        void DoCancel();

    private:

        /**
        * C++ default constructor.
        */
        CPointerEventHandler();

        /**
        * 2nd phase constructor.
        */
        void ConstructL();

        // Helper methods
        void HandleTapScreenL( const TDesC8& aData );
        void HandleTapAndDragL( const TDesC8& aData );
        void HandleTapAndDragMultipointL( const TDesC8& aData );
        void HandlePointerDownOrUpL( const TDesC8& aData );

        void SendOkMsgL();
        void SendErrorMessageL( TInt aError, const TDesC8& aDescription );

        void ChangePointerStateL();
        void MoveToNextPointL();
        void PointerDown();
        void PointerUp();
        void PointerMove();
        void SimulatePointerEvent( TRawEvent::TType aType );


    private: // data

        RWsSession iWsSession;
        MHtiDispatcher* iDispatcher; // referenced

        RTimer iTimer;
        TBool  iReady;
        TUint8 iCommand;
        TInt   iX;
        TInt   iY;
        TInt   iTapCount;
        TTimeIntervalMicroSeconds32 iEventDelay;
        TTimeIntervalMicroSeconds32 iActionDelay;
        TPointerState iState;
        RArray<TInt>* iCoords;
    };

#endif // CKEYEVENTHANDLER_H
