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
* Description:  Class that handles SIM card contacts creation and deletion
*                using the new Virtual Phonebook API
*
*/


#ifndef HTISIMDIRHANDLERVPBK_H
#define HTISIMDIRHANDLERVPBK_H


// INCLUDES
#include <MVPbkContactObserver.h>
#include <MVPbkContactStoreObserver.h>
#include <MVPbkContactViewObserver.h>
#include <MVPbkBatchOperationObserver.h>
//#include <MVPbkContactStoreListObserver.h>
#include <HTIServicePluginInterface.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CVPbkContactManager;
class MVPbkContactStore;
class MVPbkContactView;

// CLASS DECLARATION

/**
*  Class that handles SIM card contacts creation and deletion using the
*  new Virtual Phonebook API.
*/
class CHtiSimDirHandlerVPbk : public CBase,
//                              public MVPbkContactStoreListObserver,
                              public MVPbkContactStoreObserver,
                              public MVPbkContactObserver,
                              public MVPbkContactViewObserver,
                              public MVPbkBatchOperationObserver
    {
    public:

    enum TContactFieldType
        {
        ENameField        = 0x01,
        ESecondNameField  = 0x02,
        EPhoneNumberField = 0x03,
        EEMailField       = 0x04,
        EAdditNumberField = 0x05
        };

    public:

        /**
        * Two-phased constructor.
        */
        static CHtiSimDirHandlerVPbk* NewL();

        /**
        * Called when there is a message to be processed by this service.
        * @param aMessage message body destinated to the servive
        * @param aPriority message priority
        */
        void ProcessMessageL( const TDesC8& aMessage,
            THtiMessagePriority aPriority );

        /**
        * Indicates whether this handler is ready to receive
        * a new message or if it's busy processing previous message.
        * @return ETrue if processing, EFalse if ready for new request
        */
        TBool IsBusy();

        /**
        * Destructor.
        */
        virtual ~CHtiSimDirHandlerVPbk();

        /**
        * Sets the dispatcher to send outgoing messages to.
        * @param aDispatcher pointer to dispatcher instance
        */
        void SetDispatcher( MHtiDispatcher* aDispatcher );

    public: // Functions from base classes

        // From MVPbkContactStoreListObserver

        /**
         * Called when the opening process is complete, ie. all stores
         * have been reported either failed or successful open.
         */
        //void OpenComplete();

        // From MVPbkContactStoreObserver

        /**
         * Called when a contact store is ready to use.
         */
        void StoreReady( MVPbkContactStore& aContactStore );

        /**
         * Called when a contact store becomes unavailable.
         * @param aContactStore The store that became unavailable.
         * @param aReason The reason why the store is unavailable.
         *                This is one of the system wide error codes.
         */
        void StoreUnavailable( MVPbkContactStore& aContactStore, TInt aReason );

        /**
         * Called when changes occur in the contact store.
         * @param aContactStore The store the event occurred in.
         * @param aStoreEvent   Event that has occured.
         */
        void HandleStoreEventL( MVPbkContactStore& aContactStore,
                                TVPbkContactStoreEvent aStoreEvent);

        // From MVPbkContactObserver

        /**
         * Called when a contact operation has succesfully completed.
         */
        void ContactOperationCompleted( TContactOpResult aResult );

        /**
         * Called when a contact operation has failed.
         */
        void ContactOperationFailed( TContactOp aOpCode, TInt aErrorCode,
                TBool aErrorNotified );

        // From MVPbkContactViewObserver

        /**
         * Called when a view is ready for use.
         */
        void ContactViewReady( MVPbkContactViewBase& aView );

        /**
         * Called when a view is unavailable for a while.
         */
        void ContactViewUnavailable( MVPbkContactViewBase& aView );

        /**
         * Called when a contact has been added to the view.
         */
        void ContactAddedToView( MVPbkContactViewBase& aView,
                                 TInt aIndex,
                                 const MVPbkContactLink& aContactLink );

        /**
         * Called when a contact has been removed from a view.
         */
        void ContactRemovedFromView( MVPbkContactViewBase& aView,
                                     TInt aIndex,
                                     const MVPbkContactLink& aContactLink );

        /**
         * Called when an error occurs in the view.
         */
        void ContactViewError( MVPbkContactViewBase& aView,
                               TInt aError,
                               TBool aErrorNotified );

        // From MVPbkBatchOperationObserver

        /**
         * Called when one step of the operation is complete
         */
        void StepComplete( MVPbkContactOperationBase& aOperation,
                           TInt aStepSize );

        /**
         * Called when one step of the operation fails
         */
        TBool StepFailed( MVPbkContactOperationBase& aOperation,
                          TInt aStepSize, TInt aError );

        /**
         * Called when operation is completed
         */
        void OperationComplete( MVPbkContactOperationBase& aOperation );


    private: // constructors

        /**
        * C++ default constructor.
        */
        CHtiSimDirHandlerVPbk();

        /**
        * 2nd phase constructor.
        */
        void ConstructL();

    private: // helpers

        void HandleSimCardInfoL();
        void HandleSimContactImportL();
        void HandleSimContactDeleteL();

        void DeleteContactsInViewL();
        TBool CheckImportMsg();

        void SendOkMsgL( const TDesC8& aData );
        void SendErrorMessageL( TInt aError, const TDesC8& aDescription );

    private: // data

        // Pointer to the dispatcher (referenced)
        MHtiDispatcher* iDispatcher;

        // The contact manager
        CVPbkContactManager* iContactManager;

        // The SIM contact store (referenced)
        MVPbkContactStore* iSimStore;

        // The contact view
        MVPbkContactViewBase* iContactView;

        // The current/latest contact operation
        MVPbkContactOperationBase* iCurrentOperation;

        // The latest received command message without the command code
        HBufC8* iMessage;

        // The command code of the latest/current command
        TUint8 iCommand;

        // Flag indicating if service is busy processing a request
        TBool iIsBusy;

        TBool iIsStoreOpen;

    };

#endif // HTISIMDIRHANDLERVPBK_H
