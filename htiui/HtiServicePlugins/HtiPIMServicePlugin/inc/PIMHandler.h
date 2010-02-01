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
* Description:  Functional implentation of PIM service.
*
*/


#ifndef CPIMHANDLER_H
#define CPIMHANDLER_H

// INCLUDES
#include <e32base.h>
#include <s32mem.h>
#include <HTIServicePluginInterface.h>
#include <MVPbkBatchOperationObserver.h>
#include <MVPbkContactCopyObserver.h>
#include <MVPbkContactStoreObserver.h>
#include <MVPbkContactViewObserver.h>
#include <calprogresscallback.h>

// CONSTANTS

// FORWARD DECLARATIONS
class CVPbkContactManager;
class CVPbkVCardEng;
class CCalSession;
class CCalEntryView;

// CLASS DECLARATION

/**
*  Functional implentation of PIM service.
*/
class CPIMHandler : public CBase,
                    public MVPbkContactStoreObserver,
                    public MVPbkContactCopyObserver,
                    public MVPbkContactViewObserver,
                    public MVPbkBatchOperationObserver,
                    public MCalProgressCallBack
    {
    public:

        static CPIMHandler* NewL();

        // Interface implementation
        void ProcessMessageL( const TDesC8& aMessage,
            THtiMessagePriority aPriority );

        TBool IsBusy();

        virtual ~CPIMHandler();

        void SetDispatcher( MHtiDispatcher* aDispatcher );


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

        // From MVPbkContactCopyObserver

        /**
         * Called when the contact has been successfully commited or
         * copied. Caller takes the ownership of results.
         * @param aOperation    The operation that this observer monitors.
         * @param aResults  An array of links to copied contacts.
         *                  Caller takes the ownership of the
         *                  object immediately.
         */
        void ContactsSaved( MVPbkContactOperationBase& aOperation,
            MVPbkContactLinkArray* aResults );

        /**
         * Called when there was en error while saving contact(s).
         * @param aOperation    The operation that this observer monitors.
         * @param aError an error that occured.
         */
        void ContactsSavingFailed(
                MVPbkContactOperationBase& aOperation, TInt aError );

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


        // From MCalProgressCallBack

        void Progress( TInt aPercentageCompleted );
        void Completed( TInt aError );
        TBool NotifyProgress();

    private:

        CPIMHandler();
        void ConstructL();

    private: // helpers

        void HandleVCardImportFuncL( const TDesC8& aData );
        void HandleVCalendarImportFuncL( const TDesC8& aData );
        void HandleContactDeleteFuncL( const TDesC8& aData );
        void HandleCalendarDeleteFuncL( const TDesC8& aData );
        void HandleNotepadAddMemoFuncL( const TDesC8& aData );
        void HandleNotepadAddMemoFromFileFuncL( const TDesC8& aData );
        void HandleNotepadDeleteAllFuncL();
        void CreateContactDeleteViewL();
        void DeleteContactsInViewL();

        void SendOkMsgL( const TDesC8& aData );
        void SendNotepadOkMsgL( CHtiPIMServicePlugin::TCommand aCommand );
        void SendErrorMessageL( TInt aError, const TDesC8& aDescription );

    private: // data
        TUint8                iCommand;
        TBool                 iIsBusy;
        CBufFlat*             iBuffer;
        RBufReadStream        iReadStream;
        CCalSession*          iCalSession;
        CCalEntryView*        iEntryView;
        CVPbkContactManager*  iContactManager;
        CVPbkVCardEng*        iVCardEngine;
        MVPbkContactStore*    iContactStore;
        MVPbkContactOperationBase* iOp;
        MVPbkContactViewBase* iContactView;
        MHtiDispatcher*       iDispatcher; // referenced
        CActiveSchedulerWait* iWaiter;
        TInt                  iEntryViewErr;
    };

#endif // CPIMHANDLER_H
