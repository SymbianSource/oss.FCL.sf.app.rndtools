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

#include "MemSpyUiUtils.h"

// System includes
#include <coemain.h>
#include <memspyui.rsg>




void MemSpyUiUtils::Format( TDes& aBuf, TInt aResourceId, ...)
    {
	VA_LIST list;
    VA_START(list,aResourceId);
	TBuf<128> format;
	CCoeEnv::Static()->ReadResource( format, aResourceId );
	aBuf.FormatList( format, list );
    }


void MemSpyUiUtils::GetErrorText( TDes& aBuf, TInt aError )
    {
    if  ( aError == KErrNotSupported )
        {
        _LIT( KMemSpyErrorText, "Not Supported" );
        aBuf = KMemSpyErrorText;
        }
    else if ( aError == KErrNotReady )
        {
        _LIT( KMemSpyErrorText, "Not Ready" );
        aBuf = KMemSpyErrorText;
        }
    else if ( aError == KErrNotFound )
        {
        _LIT( KMemSpyErrorText, "Missing" );
        aBuf = KMemSpyErrorText;
        }
    else if ( aError == KErrGeneral )
        {
        _LIT( KMemSpyErrorText, "General Error" );
        aBuf = KMemSpyErrorText;
        }
    else
        {
        _LIT( KMemSpyItemValueError, "Error: %d" );
        aBuf.Format( KMemSpyItemValueError, aError );
        }
    }

