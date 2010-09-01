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

#ifndef MEMSPYDEVICEWIDEOPERATIONDIALOG_H
#define MEMSPYDEVICEWIDEOPERATIONDIALOG_H

// System includes
#include <AknProgressDialog.h>
#include <AknWaitDialog.h>

// Engine includes
#include <memspy/engine/memspydevicewideoperations.h>

// Classes referenced
class CMemSpyEngine;

class MMemSpyDeviceWideOperationDialogObserver
    {
public:
    virtual void DWOperationStarted() = 0;
    virtual void DWOperationCancelled() = 0;
    virtual void DWOperationCompleted() = 0;
    };


class CMemSpyDeviceWideOperationDialog : public CBase, public MProgressDialogCallback, public MMemSpyDeviceWideOperationsObserver
    {
public:
    static void ExecuteLD( CMemSpyEngine& aEngine, MMemSpyDeviceWideOperationDialogObserver& aObserver, CMemSpyDeviceWideOperations::TOperation aOperation );
    ~CMemSpyDeviceWideOperationDialog();

private:
    CMemSpyDeviceWideOperationDialog( CMemSpyEngine& aEngine, MMemSpyDeviceWideOperationDialogObserver& aObserver );
    void ExecuteL( CMemSpyDeviceWideOperations::TOperation aOperation );

public: // API
    void Cancel();

private: // From MProgressDialogCallback
    void DialogDismissedL( TInt aButtonId );

private: // From MMemSpyDeviceWideOperationsObserver
    void HandleDeviceWideOperationEvent( TEvent aEvent, TInt aParam1, const TDesC& aParam2 );

private: // Internal methods
    void SetDialogCaptionL( const TDesC& aText );

private: // Member data
    CMemSpyEngine& iEngine;
    MMemSpyDeviceWideOperationDialogObserver& iObserver;
    TBool iForcedCancel;
    CEikProgressInfo* iProgressInfo;
    CAknProgressDialog* iProgressDialog;
    CMemSpyDeviceWideOperations* iOperation;
    };


#endif
