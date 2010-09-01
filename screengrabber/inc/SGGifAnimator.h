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



#ifndef __GIFANIMATOR_H__
#define __GIFANIMATOR_H__

#include <e32std.h>
#include <e32base.h>
#include "SGModel.h"
#include "SGStd.h"


class CImageEncoder;
class CSavingProgressDialog;
class CAknGlobalProgressDialog;

class MDialogCallback
    {
public:
    virtual void DialogDismissedL( TInt aButtonId ) = 0;
    };


class CGifAnimator : public CActive, public MDialogCallback
	{
public:
	static int CreateGifAnimation(const TDesC& aFileName, TSize aDimensions, CVideoFrameArray* aVideoFrameArray);
	~CGifAnimator();

private:
    CGifAnimator();
    void StartL(const TDesC& aFileName, const TSize& aDimensions, CVideoFrameArray* aVideoFrameArray);
    void WriteHeaderL(const TSize& aDimensions);
    void WriteGraphicControlL(const TVideoFrame& aFrame);
    void WriteImageDescriptorL(const TVideoFrame& aFrame);
    CFbsBitmap* GetBitmapLC(TVideoFrame& aFrame, const TSize& aDimensions);
    void WriteRasterDataL(CFbsBitmap* aBitmap);
    void WriteFooterL();
    void FinishL();
    void WriteInt8L(TInt aValue);
    void WriteInt16L(TInt aValue);
    void RunL();
	void DoCancel();
	void DialogDismissedL(TInt aButtonId); // from MDialogCallback
    
private:
    RFs                         iFs;
    RFile                       iOutFile;
	CImageEncoder*              iImageEncoder;
	HBufC8*                     iGIFImageData;
	CActiveSchedulerWait        iWait;
    CSavingProgressDialog*      iSavingProgressDialog;
    };


class CSavingProgressDialog : public CActive
    {
public:
    static CSavingProgressDialog* NewL(MDialogCallback* aDialogCallback);
    ~CSavingProgressDialog();
    void StartDisplayingL(const TDesC &aText, TInt aFinalValue);
    void IncreaseProgressValueWithOne();
    void ProcessFinished();

private:
    void DoCancel();
    void RunL();

private: 
    CSavingProgressDialog();
    void ConstructL(MDialogCallback* aDialogCallback);

private:
    MDialogCallback*            iDialogCallback;
    CAknGlobalProgressDialog*   iGlobalProgressDialog;
    TBool                       iVisible;
    TInt                        iCurrentValue;
    TInt                        iFinalValue;
    };
		
	
#endif // __GIFANIMATOR_H__