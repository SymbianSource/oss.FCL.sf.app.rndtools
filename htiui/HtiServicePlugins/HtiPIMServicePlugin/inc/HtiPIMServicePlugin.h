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
* Description:  Implementation of ECOM plug-in service interface. Provides
*                PIM service (vCard & vCalendar import).
*
*/


#ifndef CHTIPIMSERVICEPLUGIN_H
#define CHTIPIMSERVICEPLUGIN_H

// INCLUDES
#include <HTIServicePluginInterface.h>
#include <w32std.h>

// CONSTANTS
const TInt KPIMServiceUidValue = 0x10210CCD; // ECOM Implementation UID
const TUid KPIMServiceUid = { KPIMServiceUidValue };

// FORWARD DECLARATIONS
class CPIMHandler;
class CHtiBookmarkHandler;
class CHtiSimDirHandlerVPbk;

// CLASS DECLARATION

/**
* Implementation of ECOM plug-in service interface.
*/
class CHtiPIMServicePlugin : public CHTIServicePluginInterface
    {
    public:

        enum TCommand
            {
            EImportVCard            = 0x01,
            EImportVCalendar        = 0x02,

            EDeleteContact          = 0x03,
            EDeleteCalendar         = 0x04,

            ENotepadAddMemo         = 0x05,
            ENotepadAddMemoFromFile = 0x06,
            ENotepadDeleteAll       = 0x07,

            ESimCardInfo            = 0x10,
            EImportSimContact       = 0x11,
            EDeleteSimContact       = 0x12,

            ECreateBookmark         = 0x1A,
            EDeleteBookmark         = 0x1B,

            EResultOk               = 0xFF // only for response message
            };

    public:

        static CHtiPIMServicePlugin* NewL();

        // Interface implementation

        /**
        * From CHTIServicePluginInterface
        * Called by the HTI Framework when sending message to this service.
        * @param aMessage message body destinated to a servive
        * @param aPriority message priority
        */
        void ProcessMessageL( const TDesC8& aMessage,
            THtiMessagePriority aPriority );

        /**
        * From CHTIServicePluginInterface
        * Indicates to HTI Framework whether the plugin is ready to process
        * a new message or if it's busy processing previous message.
        * @return ETrue if processing, EFalse if ready for new request
        */
        TBool IsBusy();


    protected:

        CHtiPIMServicePlugin();
        void ConstructL();

        virtual ~CHtiPIMServicePlugin();

    private:

        CPIMHandler* iPimHandler;
        CHtiBookmarkHandler* iBookmarkHandler;
        CHtiSimDirHandlerVPbk* iSimDirHandler;
        };

#endif // CHTIPIMSERVICEPLUGIN_H

// End of file
