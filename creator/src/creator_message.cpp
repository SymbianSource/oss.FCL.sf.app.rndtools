/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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



#include "creator_message.h" 
#include "creator_traces.h"
#include "creator_contactsetcache.h"
#include <apgcli.h>
#include <MuiuMsvUiServiceUtilities.h>
#include <utf.h>

#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
#include <MVPbkFieldType.h>
#include <MVPbkContactOperationBase.h>
#include <MVPbkStoreContactFieldCollection.h>
#include <MVPbkContactFieldTextData.h>
#include <CVPbkContactManager.h>
#include <MVPbkStoreContact.h>

#include <CVPbkContactStoreUriArray.h>
#include <TVPbkContactStoreUriPtr.h>
#include <VPbkContactStoreUris.h>
#include <MVPbkContactStoreList.h>
#include <MVPbkContactStore.h>
#include <MVPbkContactStoreProperties.h>
#include <VPbkEng.rsg>
#include <cmsvrecipientlist.h>
#endif
#include <mmf/common/mmfcontrollerpluginresolver.h> // for CleanupResetAndDestroyPushL


//static const TInt KMessagesFieldLength = 1024;

//_LIT(KCreatorMessagesPrefixName, "CR_");
//_LIT(KCreatorMessagesPrefixFolderName, "CR_FLDR_");


const TUid KUidBIOBusinessCardMsg =  {0x10005531}; 
_LIT(KEmpty, "");

//----------------------------------------------------------------------------

CMessagesParameters::CMessagesParameters()
    {
    LOGSTRING("Creator: CMessagesParameters::CMessagesParameters");

    iSenderAddress = HBufC::New(128);
    iRecipientAddress = HBufC::New(128);
    iMessageSubject = HBufC::New(128);
    iMessageBodyText = HBufC::New(10240);
    TRAP_IGNORE(iAttachments = new(ELeave) CArrayFixFlat<TInt>(64));
    iCreateAsUnread = ETrue;
    }

CMessagesParameters::~CMessagesParameters()
    {
    LOGSTRING("Creator: CMessagesParameters::~CMessagesParameters");


    delete iMessageBodyText;
    delete iMessageSubject;
    delete iRecipientAddress;
    delete iSenderAddress;
    delete iAttachments;
    iAttachmentPaths.ResetAndDestroy();
    iAttachmentPaths.Close(); 
    iRecipientAddressArray.ResetAndDestroy();
    iRecipientAddressArray.Close();
    iRecipientLinkIds.Close();
    iSenderLinkIds.Close();
    }

//----------------------------------------------------------------------------

CCreatorMessages* CCreatorMessages::NewL(CCreatorEngine* aEngine)
    {
    CCreatorMessages* self = CCreatorMessages::NewLC(aEngine);
    CleanupStack::Pop(self);
    return self;
    }

CCreatorMessages* CCreatorMessages::NewLC(CCreatorEngine* aEngine)
    {
    CCreatorMessages* self = new (ELeave) CCreatorMessages;
    CleanupStack::PushL(self);
    self->ConstructL(aEngine);
    return self;
    }

CCreatorMessages::CCreatorMessages()
:
iSession(0)
    {
    }

void CCreatorMessages::ConstructL(CCreatorEngine* aEngine)
    {
    LOGSTRING("Creator: CCreatorMessages::ConstructL");
    iEngine = aEngine;
    
    TRAP_IGNORE(iAttachments = new(ELeave) CArrayFixFlat<TInt>(64));
    iWaiter = CAsyncWaiter::NewL();
    }

CCreatorMessages::~CCreatorMessages()
    {
    LOGSTRING("Creator: CCreatorMessages::~CCreatorMessages");

    if ( iEngine && iEntryIds.Count() )
        {
        TRAP_IGNORE( iEngine->WriteEntryIdsToStoreL( iEntryIds, KUidDictionaryUidMessages ) );
        }
    iEntryIds.Reset();
    iEntryIds.Close();
    
    delete iParameters;
    delete iWaiter;
    delete iTmpPhoneNumber;
    delete iTmpEmail;
    delete iAttachments;
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
    delete iExistingContacts;
    if( iContactManager )
    	iContactManager->ContactStoresL().CloseAll(*this);
    delete iContactManager;
    delete iTempContact;
#endif
    iRecipientArray.ResetAndDestroy();
    iRecipientArray.Close();
    iSenderArray.ResetAndDestroy();
    iSenderArray.Close();
    delete iSession;
    }

//----------------------------------------------------------------------------

TBool CCreatorMessages::AskDataFromUserL(TInt aCommand, TInt& aNumberOfEntries)
    {
    LOGSTRING("Creator: CCreatorMessages::AskDataFromUserL");

    if ( aCommand == ECmdDeleteMessages )
        {
        return iEngine->YesNoQueryDialogL( _L("Delete all messages?") );
        }
    else if ( aCommand ==  ECmdDeleteCreatorMessages )
        {
        return iEngine->YesNoQueryDialogL( _L("Delete all messages created with Creator?") );
        }

    if (iEngine->EntriesQueryDialogL(aNumberOfEntries, _L("How many entries to create?")))
        {
        
        // message type query
        CAknListQueryDialog* dlg1 = new(ELeave) CAknListQueryDialog( (TInt*) &iMessageType );
        if (dlg1->ExecuteLD(R_MESSAGE_TYPE_QUERY))
            {
            TInt folderTypeQueryResource(R_FOLDER_TYPE_QUERY);
            
            /*
            if (iMessageType == EEmail)
                folderTypeQueryResource = R_FOLDER_TYPE_WITH_MAILBOX_QUERY;
            */

            // folder query
            CAknListQueryDialog* dlg2 = new(ELeave) CAknListQueryDialog( (TInt*) &iFolderType );
            if (dlg2->ExecuteLD(folderTypeQueryResource))
                {
                /*
                if (iFolderType == EMailbox)
                    {
                    // array to hold mailbox names
                    CDesCArray* names = new (ELeave) CDesCArrayFlat(16);
                    CleanupStack::PushL(names);                    
                    
                    CMsvSession* session = CMsvSession::OpenSyncL(*this);
                    CleanupStack::PushL(session);
    
                    // generate list of mailboxes
                    CMsvEntrySelection* entrySelection = MsvUiServiceUtilities::GetListOfAccountsWithMTML(*session, KUidMsgTypeSMTP, ETrue);
                    CleanupStack::PushL(entrySelection);

                    TBool doReturn(EFalse);
                    
                    if (entrySelection->Count() == 0)
                        {
                        CAknInformationNote* note = new(ELeave) CAknInformationNote;
                        note->ExecuteLD(_L("No mailboxes found"));

                        doReturn = ETrue;
                        }
                    else
                        {
                        // get mailbox names
                        for (TInt i=0; i<entrySelection->Count(); i++)
                            {
                            CMsvEntry* centry = session->GetEntryL(entrySelection->At(i));
                            CleanupStack::PushL(centry);

                            TMsvEntry tentry = centry->Entry();
                            names->AppendL(tentry.iDetails);
                            CleanupStack::PopAndDestroy(); //centry
                            }
                        
                        // show query
                        TInt index(0);
                        CAknListQueryDialog* dlg = new(ELeave) CAknListQueryDialog(&index);
                        dlg->PrepareLC(R_MAILBOX_SELECTION_QUERY);
                        dlg->SetItemTextArray(names);
                        dlg->SetOwnershipType(ELbmDoesNotOwnItemArray);

                        if(dlg->RunLD())
                            {
                            iUserSelectedMailbox = entrySelection->At(index);

                            doReturn = EFalse;
                            }
                        else
                            {
                            doReturn = ETrue;
                            }    
                        
                        }
                    
                    CleanupStack::PopAndDestroy(3); // names, session, entrySelection
                    
                    if (doReturn)
                        return EFalse;                    
                    }
                */
                
                // query create as unread
                CAknListQueryDialog* dlg3 = new(ELeave) CAknListQueryDialog( (TInt*) &iCreateAsUnread );
                if (dlg3->ExecuteLD(R_UNREAD_QUERY))
                    {                

                    // query number of characters in msg body
                    switch (iMessageType)
                        {
                        case ESMS:
                        case EMMS:
                        case EEmail:
                            {
                            iDefinedMessageLength = 100;
                            if (iEngine->EntriesQueryDialogL(iDefinedMessageLength, _L("Amount of characters in message body?"), ETrue))
                                {
                                ;
                                }
                            else
                                return EFalse;

                            break;
                            }
                        case EAMS:
                        	{
                        	iDefinedMessageLength = 0;
                        	break;
                        	}
                        default: break;
                        }

                    // query attachments
                    iAttachments->Reset();
              
                    switch (iMessageType)
                        {
                        case EMMS:
                        case EEmail:
                            {
                            CAknListQueryDialog* dlg4 = new(ELeave) CAknListQueryDialog( iAttachments );
                            if (dlg4->ExecuteLD(R_ATTACHMENT_MULTI_SELECTION_QUERY))
                                {
                                // "none" is selected
                                if (iAttachments->At(0) == 0)
                                    iAttachments->Reset();
                                else  // otherwise fix indexes
                                    {
                                    for (TInt i=0; i<iAttachments->Count(); i++)
                                        iAttachments->At(i)--;  // decrease value by one 

                                    }
                                }
                            else
                                return EFalse;
                            
                            break;
                            }
                        case EAMS:
                        	{
                            iAttachments->AppendL( TInt(0) );

                            CAknListQueryDialog* dlg5 = new(ELeave) CAknListQueryDialog( &iAttachments->At(0) );
                            if (dlg5->ExecuteLD(R_AMS_ATTACHMENT_SINGLE_SELECTION_QUERY))
                                {
                                ;
                                }
                            else
                                return EFalse;                        

                            break;
                        	}
                            
                        case EIrMessage:
                        case EBTMessage:
                            {
                            iAttachments->AppendL( TInt(0) );

                            CAknListQueryDialog* dlg5 = new(ELeave) CAknListQueryDialog( &iAttachments->At(0) );
                            if (dlg5->ExecuteLD(R_ATTACHMENT_SINGLE_SELECTION_QUERY))
                                {
                                ;
                                }
                            else
                                return EFalse;                        

                            break;
                            }
                        default: break;
                        }
                    
                    return ETrue;  // all queries accepted

                    }
                else
                    return EFalse;
                }
            else
                return EFalse;
            }
        else
            return EFalse;
        }
    else
        return EFalse;
    }

//----------------------------------------------------------------------------

TInt CCreatorMessages::CreateRandomMessageEntryL(TInt aCommand)
    {
    LOGSTRING2("Creator: CCreatorMessages::CreateRandomMessageEntryL - Command: %d", aCommand);

    TInt err = KErrNone;
    
    iDefinedMessageLength = iEngine->RandomNumber(100, 300);


    switch (aCommand)
        {
	    case ECmdCreateRandomEntrySMSInbox:     { iMessageType=ESMS; iFolderType=EInbox; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntrySMSDrafts:    { iMessageType=ESMS; iFolderType=EDrafts; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntrySMSOutbox:    { iMessageType=ESMS; iFolderType=EOutbox; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntrySMSSent:      { iMessageType=ESMS; iFolderType=ESent; err=CreateMessageEntryL(NULL); break;}
	    
        case ECmdCreateRandomEntryMMSInbox:     { iMessageType=EMMS; iFolderType=EInbox; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryMMSDrafts:    { iMessageType=EMMS; iFolderType=EDrafts; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryMMSOutbox:    { iMessageType=EMMS; iFolderType=EOutbox; err=CreateMessageEntryL(NULL); break;}    
	    case ECmdCreateRandomEntryMMSSent:      { iMessageType=EMMS; iFolderType=ESent; err=CreateMessageEntryL(NULL); break;}
	    
        case ECmdCreateRandomEntryEmailInbox:   { iMessageType=EEmail; iFolderType=EInbox; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryEmailDrafts:  { iMessageType=EEmail; iFolderType=EDrafts; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryEmailOutbox:  { iMessageType=EEmail; iFolderType=EOutbox; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryEmailSent:    { iMessageType=EEmail; iFolderType=ESent; err=CreateMessageEntryL(NULL); break;}
	    
        case ECmdCreateRandomEntryBIOInbox:     { iMessageType=ESmartMessage; iFolderType=EInbox; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryBIODrafts:    { iMessageType=ESmartMessage; iFolderType=EDrafts; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryBIOOutbox:    { iMessageType=ESmartMessage; iFolderType=EOutbox; err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryBIOSent:      { iMessageType=ESmartMessage; iFolderType=ESent; err=CreateMessageEntryL(NULL); break;}
	    
        case ECmdCreateRandomEntryIRInbox:      { iMessageType=EIrMessage; iFolderType=EInbox; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryIRDrafts:     { iMessageType=EIrMessage; iFolderType=EDrafts; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryIROutbox:     { iMessageType=EIrMessage; iFolderType=EOutbox; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryIRSent:       { iMessageType=EIrMessage; iFolderType=ESent; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
	    
        case ECmdCreateRandomEntryBTInbox:      { iMessageType=EBTMessage; iFolderType=EInbox; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryBTDrafts:     { iMessageType=EBTMessage; iFolderType=EDrafts; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryBTOutbox:     { iMessageType=EBTMessage; iFolderType=EOutbox; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryBTSent:       { iMessageType=EBTMessage; iFolderType=ESent; iAttachments->Reset(); iAttachments->AppendL( iEngine->RandomNumber(CCreatorEngine::EJPEG_25kB, CCreatorEngine::LAST_FILE_ID-1) ); err=CreateMessageEntryL(NULL); break;}
        
	    case ECmdCreateRandomEntryAMSInbox:     { iMessageType=EAMS; iFolderType=EInbox; iAttachments->Reset(); iAttachments->AppendL( CCreatorEngine::EMP3_250kB); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryAMSDrafts:     { iMessageType=EAMS; iFolderType=EDrafts; iAttachments->Reset(); iAttachments->AppendL( CCreatorEngine::EMP3_250kB); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryAMSOutbox:     { iMessageType=EAMS; iFolderType=EOutbox; iAttachments->Reset(); iAttachments->AppendL( CCreatorEngine::EMP3_250kB); err=CreateMessageEntryL(NULL); break;}
	    case ECmdCreateRandomEntryAMSSent:     { iMessageType=EAMS; iFolderType=ESent; iAttachments->Reset(); iAttachments->AppendL( CCreatorEngine::EMP3_250kB); err=CreateMessageEntryL(NULL); break;}

	    default: { User::Panic(_L("MessageCommand"), 901);  break;}
        }

    return err;
    }

//----------------------------------------------------------------------------

TInt CCreatorMessages::CreateMessageEntryL(CMessagesParameters *aParameters, TBool /*aTakeUserGivenParameters*/)
    {
    LOGSTRING("Creator: CCreatorMessages::CreateMessageEntryL");
    
    delete iParameters;
    iParameters = 0;   
    
    CMessagesParameters* parameters = aParameters;
           
    // random data needed if no predefined data available
    if (!parameters)
        {
        iParameters = new(ELeave) CMessagesParameters;
        parameters = iParameters;
        // check if values from ui needed
        //if (aTakeUserGivenParameters)
        //    {            
            parameters->iMessageType = iMessageType;
            parameters->iFolderType = iFolderType;
            parameters->iDefinedMessageLength = iDefinedMessageLength;
            parameters->iCreateAsUnread = iCreateAsUnread;

            for (TInt i=0; i<iAttachments->Count(); i++)
                parameters->iAttachments->AppendL(iAttachments->At(i)); 
        //    }
        //else
        //    {
            // message length
        //    parameters->iDefinedMessageLength = iEngine->RandomNumber(100, 300);
        //    }    

        if (parameters->iMessageType == EEmail)
            {
            parameters->iSenderAddress->Des().Copy( iEngine->RandomString(CCreatorEngine::EFirstName) );
            parameters->iSenderAddress->Des().Append( _L("@") );
            parameters->iSenderAddress->Des().Append( iEngine->RandomString(CCreatorEngine::ECompany) );
            parameters->iSenderAddress->Des().Append( _L(".com") );

            parameters->iRecipientAddress->Des().Copy( iEngine->RandomString(CCreatorEngine::EFirstName) );
            parameters->iRecipientAddress->Des().Append( _L("@") );
            parameters->iRecipientAddress->Des().Append( iEngine->RandomString(CCreatorEngine::ECompany) );
            parameters->iRecipientAddress->Des().Append( _L(".net") );
            }
        else if (parameters->iMessageType == EIrMessage || parameters->iMessageType == EBTMessage)
            {
            parameters->iSenderAddress->Des().Copy( iEngine->RandomString(CCreatorEngine::EFirstName) );
            parameters->iRecipientAddress->Des().Copy( iEngine->RandomString(CCreatorEngine::EFirstName) );
            }
        else
            {
            parameters->iSenderAddress->Des().Copy( iEngine->RandomString(CCreatorEngine::EPhoneNumber) );
            parameters->iRecipientAddress->Des().Copy( iEngine->RandomString(CCreatorEngine::EPhoneNumber) );
            }

        // subject
        parameters->iMessageSubject->Des() = iEngine->RandomString(CCreatorEngine::EMessageSubject);

        // message body
        parameters->iMessageBodyText->Des().Copy(_L(""));
        TPtrC randText = iEngine->RandomString(CCreatorEngine::EMessageText);
        for (TInt i=0; i<parameters->iDefinedMessageLength / randText.Length() +1; i++)
            {
            parameters->iMessageBodyText->Des().Append( randText );
            parameters->iMessageBodyText->Des().Append( _L(" ") );
            }
        parameters->iMessageBodyText->Des() = parameters->iMessageBodyText->Des().Left(parameters->iDefinedMessageLength);

        // BIO message type (from bitsids.h)  // TO-DO: add support for all BIO message types...
        parameters->iBIOMessageType = KUidBIOBusinessCardMsg;
        }
    
    
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
    
    // Get number of existing senders and recipients:
    for( TInt i = 0; i < parameters->iRecipientLinkIds.Count(); ++i )
        {       
        
        TLinkIdParam recipientId = parameters->iRecipientLinkIds[i];        
        // Is maximum number limited:
        if(recipientId.iLinkAmount == KUndef || 
           recipientId.iLinkAmount >= ContactLinkCache::Instance()->ContactSet(recipientId.iLinkId).NumberOfExistingContacts() )
            {
            parameters->iNumberOfExistingRecipients += ContactLinkCache::Instance()->ContactSet(recipientId.iLinkId).NumberOfExistingContacts();
            }
        else
            {
            parameters->iNumberOfExistingRecipients += recipientId.iLinkAmount;
            }
        }
    for( TInt i = 0; i < parameters->iSenderLinkIds.Count(); ++i )
        {
        TLinkIdParam senderId = parameters->iSenderLinkIds[i];
        // Is maximum number limited:
        if( senderId.iLinkAmount == KUndef ||
            senderId.iLinkAmount >= ContactLinkCache::Instance()->ContactSet(senderId.iLinkId).NumberOfExistingContacts())
            {
            parameters->iNumberOfExistingSenders += ContactLinkCache::Instance()->ContactSet(senderId.iLinkId).NumberOfExistingContacts();
            }
        else
            {
            parameters->iNumberOfExistingSenders += senderId.iLinkAmount;
            }            
        }
    
#endif

    TInt err = KErrNone;

    switch (parameters->iMessageType)
        {
        case ESMS:          { err = CreateSMSEntryL(*parameters);  break; }
        case EMMS:          { err = CreateMMSEntryL(*parameters);  break; }
        case EAMS:          { err = CreateAMSEntryL(*parameters);  break; }
        case EEmail:        { err = CreateEmailEntryL(*parameters);  break; }
        case ESmartMessage: { err = CreateSmartMessageEntryL(*parameters);  break; }
        case EIrMessage:    { err = CreateObexEntryL( TUid::Uid(KUidMsgTypeIrTInt32), *parameters );  break; }    
        case EBTMessage:    { err = CreateObexEntryL( TUid::Uid(KUidMsgTypeBtTInt32), *parameters );  break; }
        default:            { User::Panic(_L("MsgType"), 851);  break;}

        }

    iSenderArray.ResetAndDestroy();
    iRecipientArray.ResetAndDestroy();
    return err;
    }

//----------------------------------------------------------------------------

TInt CCreatorMessages::CreateSMSEntryL(const CMessagesParameters& parameters)
    {
    LOGSTRING("Creator: CCreatorMessages::CreateSMSEntryL");

    TInt err = KErrNone;
               
    // init    
    if( iSession == 0 )
        {
        iSession = CMsvSession::OpenSyncL(*this);
        }

    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*iSession);
    CleanupStack::PushL(registry);

    // get the client mtm and return if it isn't supported in the system        
    CSmsClientMtm* clientMtm = NULL;
    TRAP(err, clientMtm = static_cast<CSmsClientMtm*>(registry->NewMtmL(KUidMsgTypeSMS)));
    if (err || !clientMtm)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("SMS: Message type module not found"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        }  
    CleanupStack::PushL(clientMtm);   

    // create a new object to access an existing entry
    CMsvEntry* msvEntry = CMsvEntry::NewL(*iSession, KMsvGlobalInBoxIndexEntryId, TMsvSelectionOrdering());
    CleanupStack::PushL(msvEntry);
  
    // get default service
    TMsvId defaultServiceId = 0;
    TRAP(err, defaultServiceId = clientMtm->DefaultServiceL());
    if (err)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("SMS: Define a SMS centre first"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        } 
    
    // set folder type
    switch (parameters.iFolderType)
        {
        case EInbox:    { msvEntry->SetEntryL(KMsvGlobalInBoxIndexEntryId);   break; }
        case EDrafts:   { msvEntry->SetEntryL(KMsvDraftEntryId);              break; }
        case EOutbox:   { msvEntry->SetEntryL(KMsvGlobalOutBoxIndexEntryId);  break; }
        case ESent:     { msvEntry->SetEntryL(KMsvSentEntryId);               break; }
        default:        { User::Panic(_L("Folder Type"), 871);                break; } 
        }
        
    // mtm takes ownership of entry context	
    CleanupStack::Pop(msvEntry);
    clientMtm->SetCurrentEntryL(msvEntry);    
    
    // create a new message
    clientMtm->CreateMessageL(defaultServiceId);

    // set the from field to sms header
    if (parameters.iFolderType == EInbox)
        {
        CSmsHeader& smsHeader = clientMtm->SmsHeader();

        if( parameters.iSenderAddress )
            {
            smsHeader.SetFromAddressL( parameters.iSenderAddress->Des() );
            }
        else
            {
            smsHeader.SetFromAddressL(KEmpty);
            }
        }       

    // set body
    clientMtm->Body().Reset();
    clientMtm->Body().InsertL(0, parameters.iMessageBodyText->Des());        

    // get the entry of the message
    TMsvEntry messageEntry = clientMtm->Entry().Entry();

    // set the details field
    if (parameters.iFolderType == EInbox)
        {
        SetSenderToEntryDetails(messageEntry, parameters, EFalse);        
        }
    else
        {
        SetRecipientToEntryDetails(messageEntry, parameters, EFalse);
        // Add all recipients to clientMtm
        // iRecipientArray is up-to-date so don't call AddRecipientsL here 
        for( TInt i = 0; i < iRecipientArray.Count(); i++ )
            {
            clientMtm->AddAddresseeL( iRecipientArray[i]->Des() );
            }
        }

    // set the sending state for outbox message
    // to avoid SMSMtmUI:15 panic in messages application
    if (parameters.iFolderType == EOutbox )
        {
        messageEntry.SetSendingState( KMsvSendStateWaiting );
        }

    // set the description field same as the first part of the message body
    messageEntry.iDescription.Set( parameters.iMessageBodyText->Des().Left(KSmsDescriptionLength) );

    // save the changes done above
    clientMtm->Entry().ChangeL(messageEntry);

    // save the message     
    clientMtm->SaveMessageL();
    	
	// final fine tuning
    messageEntry.SetAttachment(EFalse);
    messageEntry.iDate.HomeTime();
    messageEntry.SetVisible(ETrue);
    messageEntry.SetInPreparation(EFalse);
   
    if (parameters.iCreateAsUnread)
        {
        messageEntry.SetUnread(ETrue);
        messageEntry.SetNew(ETrue);
        }
    else
        {
        messageEntry.SetUnread(EFalse);
        messageEntry.SetNew(EFalse);
        }
    
    messageEntry.SetComplete(ETrue);
    messageEntry.iServiceId = defaultServiceId;
    messageEntry.iRelatedId = 0;
    clientMtm->Entry().ChangeL(messageEntry);

    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( messageEntry.Id() );
    
    CleanupStack::PopAndDestroy(2); //registry, clientMtm
        
    return err;

    }

//----------------------------------------------------------------------------

TInt CCreatorMessages::CreateMMSEntryL(const CMessagesParameters& parameters)
    {
    LOGSTRING("Creator: CCreatorMessages::CreateMMSEntryL");

    TInt err = KErrNone;

        
    // init    
    if( iSession == 0 )
        {
        iSession = CMsvSession::OpenSyncL(*this);
        }
    
    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*iSession);
    CleanupStack::PushL(registry);

    // get the client mtm and return if it isn't supported in the system        
    CMmsClientMtm* clientMtm = NULL;
    TRAP(err, clientMtm = static_cast<CMmsClientMtm*>(registry->NewMtmL(KUidMsgTypeMultimedia)));
    if (err || !clientMtm)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("MMS: Message type module not found"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        }  
    CleanupStack::PushL(clientMtm);   

    // create a new object to access an existing entry
    CMsvEntry* msvEntry = CMsvEntry::NewL(*iSession, KMsvGlobalInBoxIndexEntryId, TMsvSelectionOrdering());
    CleanupStack::PushL(msvEntry);
  
    // get default service
    TMsvId defaultServiceId = 0;
    TRAP(err, defaultServiceId = clientMtm->DefaultServiceL());
    if (err)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("MMS: Define MMS settings first"));
        CleanupStack::PopAndDestroy(4);
        User::Leave(KErrNotFound);
        } 
    
    // set folder type
    switch (parameters.iFolderType)
        {
        case EInbox:    { msvEntry->SetEntryL(KMsvGlobalInBoxIndexEntryId);   break; }
        case EDrafts:   { msvEntry->SetEntryL(KMsvDraftEntryId);              break; }
        case EOutbox:   { msvEntry->SetEntryL(KMsvGlobalOutBoxIndexEntryId);  break; }
        case ESent:     { msvEntry->SetEntryL(KMsvSentEntryId);               break; }
        default:        { User::Panic(_L("Folder Type"), 871);                break; } 
        }
        
    // mtm takes ownership of entry context	
    CleanupStack::Pop(msvEntry);
    clientMtm->SetCurrentEntryL(msvEntry);    
    
    // create a new message
    clientMtm->CreateMessageL(defaultServiceId);

    // set subject
    clientMtm->SetSubjectL( parameters.iMessageSubject->Des() );
            
    // set some mms defaults
    clientMtm->SetMessageClass(EMmsClassPersonal);
    clientMtm->SetExpiryInterval(86400);
    clientMtm->SetDeliveryTimeInterval(3600);
    clientMtm->SetMessagePriority(EMmsPriorityLow);
    clientMtm->SetSenderVisibility(EMmsMaximumSenderVisibility);
    clientMtm->SetDeliveryReport(EMmsDeliveryReportYes);
    clientMtm->SetReadReply(EMmsReadReplyYes);        
    
    // set the sender / addressee field
    if (parameters.iFolderType == EInbox)
        {        
        AddMtmSenderL(*clientMtm, parameters, EFalse);                
        }
    
    // Add all recipients to clientMtm
    AddRecipientsL( *clientMtm, parameters, EFalse );  
        
    // get an access to the message store
    CMsvStore* store = msvEntry->EditStoreL();
    CleanupStack::PushL(store);                  
   

    // set body as an attachment
    CMsvMimeHeaders* mimeHeaders = CMsvMimeHeaders::NewL();
    CleanupStack::PushL(mimeHeaders);
    mimeHeaders->SetContentTypeL( _L8("text") );
    mimeHeaders->SetContentSubTypeL( _L8("plain") );
    mimeHeaders->SetMimeCharset( KMmsUtf8 );
    mimeHeaders->SetSuggestedFilenameL( _L("body.txt") );
    
    MMsvAttachmentManager& attachMan = store->AttachmentManagerL();
    CMsvAttachment* attachmentInfo = CMsvAttachment::NewL(CMsvAttachment::EMsvFile); // ownership of attachmentInfo will be transferred
    CleanupStack::PushL(attachmentInfo);
    attachmentInfo->SetAttachmentNameL( _L("body.txt") );
    attachmentInfo->SetMimeTypeL( _L8("text/plain") );
    mimeHeaders->StoreL( *attachmentInfo );
    
    RFile textFile;

    CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
    attachMan.CreateAttachmentL(_L("body.txt"), textFile, attachmentInfo, waiter->iStatus );
    CleanupStack::Pop(); // waiter
    CleanupStack::Pop(); // attachmentInfo
    CleanupStack::PushL(waiter);
    waiter->StartAndWait();
    CleanupStack::PopAndDestroy(waiter);        
   
    HBufC8* tempBuf = CnvUtfConverter::ConvertFromUnicodeToUtf8L( parameters.iMessageBodyText->Des() );
    CleanupStack::PushL(tempBuf);
    textFile.Write( tempBuf->Des() );        
    textFile.Close();
    CleanupStack::PopAndDestroy(); //tempBuf
    
    CleanupStack::PopAndDestroy(); //mimeHeaders

    
    // get the entry of the message
    TMsvEntry messageEntry = clientMtm->Entry().Entry();        
    
    // set the details field
    if (parameters.iFolderType == EInbox)
        {
        SetSenderToEntryDetails(messageEntry, parameters, EFalse);        
        }
    else
        {
        SetRecipientToEntryDetails(messageEntry, parameters, EFalse);
        }    

    // set the description field same as the message subject
    messageEntry.iDescription.Set( parameters.iMessageSubject->Des().Left(KSmsDescriptionLength) );

    // save the changes done above
    clientMtm->Entry().ChangeL(messageEntry);        
    

      // save the attachments
    HandleAttachementsL(parameters, store, err );
//    for (TInt i=0; i<parameters.iAttachments->Count(); i++)
//        {
//        TFileName sourceFileName = iEngine->TestDataPathL( (CCreatorEngine::TTestDataPath) parameters.iAttachments->At(i) );
//
//        TParse parser;
//        parser.Set(sourceFileName, NULL, NULL);
//        TFileName shortFileName = parser.NameAndExt();
//
//        // get the mime type
//        RApaLsSession ls;
//        User::LeaveIfError(ls.Connect());
//        CleanupClosePushL<RApaLsSession>(ls);
//        TUid appUid;
//        TDataType dataType;
//        ls.AppForDocument(sourceFileName, appUid, dataType);
//        CleanupStack::PopAndDestroy(); //ls
//        TPtrC8 mimeType = dataType.Des8();
//        
//        // attachment settings
//        MMsvAttachmentManager& attachMan = store->AttachmentManagerL();
//        CMsvAttachment* attachmentInfo = CMsvAttachment::NewL(CMsvAttachment::EMsvFile); // ownership of attachmentInfo will be transferred
//        attachmentInfo->SetAttachmentNameL( shortFileName );        
//        attachmentInfo->SetMimeTypeL( mimeType );        
//
//        // save
//        CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
//        
//        TRAP(err, attachMan.AddAttachmentL(sourceFileName, attachmentInfo, waiter->iStatus ));
//        if (err == KErrNone)
//            waiter->StartAndWait();
//        else
//            LOGSTRING2("Creator: CCreatorMessages::CreateMMSEntryL failed to attach %S", &sourceFileName);
//
//        CleanupStack::PopAndDestroy(waiter);  
//        }

    // save the changes made to the message store
    store->CommitL();
    CleanupStack::PopAndDestroy(store);   
           
    
    // save the message      
    clientMtm->SaveMessageL();
    	
	// final fine tuning
	messageEntry.SetAttachment(ETrue);
    messageEntry.iDate.HomeTime();
    messageEntry.SetVisible(ETrue);
    messageEntry.SetInPreparation(EFalse);
    if (parameters.iCreateAsUnread)
        {
        messageEntry.SetUnread(ETrue);
        messageEntry.SetNew(ETrue);
        }
    else
        {
        messageEntry.SetUnread(EFalse);
        messageEntry.SetNew(EFalse);
        }
    messageEntry.SetComplete(ETrue);
    messageEntry.iServiceId = defaultServiceId;
    messageEntry.iRelatedId = 0;
    
    if (parameters.iFolderType == EDrafts) 
        messageEntry.SetReadOnly(EFalse);
    else
        messageEntry.SetReadOnly(ETrue);

	messageEntry.iMtmData1 = KMmsMessageMRetrieveConf | KMmsMessageMobileTerminated;
	
    clientMtm->Entry().ChangeL(messageEntry);
    
    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( messageEntry.Id() );
    
    CleanupStack::PopAndDestroy(2); // registry, clientMtm        
        

    return err;
    }

//----------------------------------------------------------------------------
void CCreatorMessages::HandleAttachementsL(const CMessagesParameters& parameters, CMsvStore* store, TInt err )
    {
    // save the attachments (added with id's)
  for (TInt i=0; i<parameters.iAttachments->Count(); i++)
      {
      TFileName sourceFileName = iEngine->TestDataPathL( (CCreatorEngine::TTestDataPath) parameters.iAttachments->At(i) );
      HandleAttachementL(store, err, sourceFileName );

      }    
  //Adding attachmentpaths
  for (TInt i=0; i<parameters.iAttachmentPaths.Count(); i++)
      {
      HBufC* point = parameters.iAttachmentPaths[i];
      TFileName sourceFileName(point->Des());      
      HandleAttachementL(store, err, sourceFileName );
      }
    }
//----------------------------------------------------------------------------

void CCreatorMessages::HandleAttachementL(CMsvStore* store, TInt err, TFileName& sourceFileName )
    {
    TParse parser;
    parser.Set(sourceFileName, NULL, NULL);
    TFileName shortFileName = parser.NameAndExt();

    // get the mime type
    RApaLsSession ls;
    User::LeaveIfError(ls.Connect());
    CleanupClosePushL<RApaLsSession>(ls);
    TUid appUid;
    TDataType dataType;
    ls.AppForDocument(sourceFileName, appUid, dataType);
    CleanupStack::PopAndDestroy(); //ls
    TPtrC8 mimeType = dataType.Des8();
    
    // attachment settings
    MMsvAttachmentManager& attachMan = store->AttachmentManagerL();
    CMsvAttachment* attachmentInfo = CMsvAttachment::NewL(CMsvAttachment::EMsvFile); // ownership of attachmentInfo will be transferred
    CleanupStack::PushL(attachmentInfo);
    attachmentInfo->SetAttachmentNameL( shortFileName );        
    attachmentInfo->SetMimeTypeL( mimeType );
    
    if ( mimeType == _L8("text/plain") )
        {
        // CMsvMimeHeaders needs to be stored in case of text file 
        CMsvMimeHeaders* mimeHeaders = CMsvMimeHeaders::NewL();
        CleanupStack::PushL(mimeHeaders);
        mimeHeaders->SetContentTypeL( _L8("text") );
        mimeHeaders->SetContentSubTypeL( _L8("plain") );
        mimeHeaders->SetMimeCharset( KMmsUtf8 );
        mimeHeaders->StoreL( *attachmentInfo );
        CleanupStack::PopAndDestroy( mimeHeaders );
        }

    // save
    
    CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
    TRAP(err, attachMan.AddAttachmentL(sourceFileName, attachmentInfo, waiter->iStatus ));
    if (err == KErrNone)
    	{
        waiter->StartAndWait();
        CleanupStack::PopAndDestroy(waiter);
        CleanupStack::Pop(); // attachmentInfo, the ownership is transferred
    	}
    else
    	{
    	CleanupStack::PopAndDestroy(waiter);
    	CleanupStack::PopAndDestroy(); // attachmentInfo, ownership is not transferred (leave occurs)
        LOGSTRING2("Creator: CCreatorMessages::HandleAttachementsL failed to attach %S", &sourceFileName);
    	}
    }
      //----------------------------------------------------------------------------

TInt CCreatorMessages::CreateAMSEntryL(const CMessagesParameters& parameters)
    {
    LOGSTRING("Creator: CCreatorMessages::CreateAMSEntryL");

    TInt err = KErrNone;

        
    // init    
    if( iSession == 0 )
        {
        iSession = CMsvSession::OpenSyncL(*this);
        }

    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*iSession);
    CleanupStack::PushL(registry);

    // get the client mtm and return if it isn't supported in the system        
    CMmsClientMtm* clientMtm = NULL;
    TRAP(err, clientMtm = static_cast<CMmsClientMtm*>(registry->NewMtmL(KUidMsgTypeMultimedia)));
    if (err || !clientMtm)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("MMS: Message type module not found"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        }  
    CleanupStack::PushL(clientMtm);   

    // create a new object to access an existing entry
    CMsvEntry* msvEntry = CMsvEntry::NewL(*iSession, KMsvGlobalInBoxIndexEntryId, TMsvSelectionOrdering());
    CleanupStack::PushL(msvEntry);
  
    // get default service
    TMsvId defaultServiceId = 0;
    TRAP(err, defaultServiceId = clientMtm->DefaultServiceL());
    if (err)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("MMS: Define MMS settings first"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        } 
    
    // set folder type
    switch (parameters.iFolderType)
        {
        case EInbox:    { msvEntry->SetEntryL(KMsvGlobalInBoxIndexEntryId);   break; }
        case EDrafts:   { msvEntry->SetEntryL(KMsvDraftEntryId);              break; }
        case EOutbox:   { msvEntry->SetEntryL(KMsvGlobalOutBoxIndexEntryId);  break; }
        case ESent:     { msvEntry->SetEntryL(KMsvSentEntryId);               break; }
        default:        { User::Panic(_L("Folder Type"), 871);                break; } 
        }
        
    // mtm takes ownership of entry context	
    CleanupStack::Pop(msvEntry);
    clientMtm->SetCurrentEntryL(msvEntry);    
    
    // create a new message
    clientMtm->CreateMessageL(defaultServiceId);

    // set subject
    clientMtm->SetSubjectL(parameters.iMessageSubject->Des() );
            
    // set some mms defaults
    clientMtm->SetMessageClass(EMmsClassPersonal);
    clientMtm->SetExpiryInterval(86400);
    clientMtm->SetDeliveryTimeInterval(3600);
    clientMtm->SetMessagePriority(EMmsPriorityLow);
    clientMtm->SetSenderVisibility(EMmsMaximumSenderVisibility);
    clientMtm->SetDeliveryReport(EMmsDeliveryReportYes);
    clientMtm->SetReadReply(EMmsReadReplyYes);        
    
    // set the sender / addressee field
    if (parameters.iFolderType == EInbox)
        {
        AddMtmSenderL(*clientMtm, parameters, EFalse);                
        }

    // Add all recipients to clientMtm
    AddRecipientsL( *clientMtm, parameters, EFalse );
    
        
    // get an access to the message store
    CMsvStore* store = msvEntry->EditStoreL();
    CleanupStack::PushL(store);                  
   
    // get the entry of the message
    TMsvEntry messageEntry = clientMtm->Entry().Entry();        
    
    // set the details field
    if (parameters.iFolderType == EInbox)
        {
        SetSenderToEntryDetails(messageEntry, parameters, EFalse);        
        }  
    else
        {
        SetRecipientToEntryDetails(messageEntry, parameters, EFalse);
        }

    // set the description field same as the message subject
    messageEntry.iDescription.Set( parameters.iMessageSubject->Des().Left(KSmsDescriptionLength) );
    messageEntry.iBioType = KUidMsgSubTypeMmsAudioMsg.iUid;
    
    // save the changes done above
    clientMtm->Entry().ChangeL(messageEntry);        
    

    TFileName sourceFileName = iEngine->TestDataPathL( CCreatorEngine::EAMR_20kB );
    TParse parser;
    parser.Set(sourceFileName, NULL, NULL);
    TFileName shortFileName = parser.NameAndExt();

    // get the mime type
    RApaLsSession ls;
    User::LeaveIfError(ls.Connect());
    CleanupClosePushL<RApaLsSession>(ls);
    TUid appUid;
    TDataType dataType;
    ls.AppForDocument(sourceFileName, appUid, dataType);
    CleanupStack::PopAndDestroy(); //ls
    TPtrC8 mimeType = dataType.Des8();
        
    // attachment settings
    MMsvAttachmentManager& attachMan = store->AttachmentManagerL();
    CMsvAttachment* attachmentInfo = CMsvAttachment::NewL(CMsvAttachment::EMsvFile); // ownership of attachmentInfo will be transferred
    attachmentInfo->SetAttachmentNameL( shortFileName );        
    attachmentInfo->SetMimeTypeL( mimeType );        

    // save
    CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
      
    TRAP(err, attachMan.AddAttachmentL(sourceFileName, attachmentInfo, waiter->iStatus ));
    if (err == KErrNone)
        waiter->StartAndWait();
    else
        LOGSTRING2("Creator: CCreatorMessages::CreateAMSEntryL failed to attach %S", &sourceFileName);

    CleanupStack::PopAndDestroy(waiter);  

    // save the changes made to the message store
    store->CommitL();
    CleanupStack::PopAndDestroy(store);   
           
    
    // save the message      
    clientMtm->SaveMessageL();
    	
	// final fine tuning
	messageEntry.SetAttachment(ETrue);
    messageEntry.iDate.HomeTime();
    messageEntry.SetVisible(ETrue);
    messageEntry.SetInPreparation(EFalse);
    if (parameters.iCreateAsUnread)
        {
        messageEntry.SetUnread(ETrue);
        messageEntry.SetNew(ETrue);
        }
    else
        {
        messageEntry.SetUnread(EFalse);
        messageEntry.SetNew(EFalse);
        }
    messageEntry.SetComplete(ETrue);
    messageEntry.iServiceId = defaultServiceId;
    messageEntry.iRelatedId = 0;
    
    if (parameters.iFolderType == EDrafts) 
        messageEntry.SetReadOnly(EFalse);
    else
        messageEntry.SetReadOnly(ETrue);

	messageEntry.iMtmData1 = KMmsMessageMRetrieveConf | KMmsMessageMobileTerminated;
	
    clientMtm->Entry().ChangeL(messageEntry);
    
    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( messageEntry.Id() );
    
    CleanupStack::PopAndDestroy(2); // registry, clientMtm        
        

    return err;
    }

//----------------------------------------------------------------------------

TInt CCreatorMessages::CreateEmailEntryL(const CMessagesParameters& parameters)
    {
    LOGSTRING("Creator: CCreatorMessages::CreateEmailEntryL");

    TInt err = KErrNone;

    // init    
    if( iSession == 0 )
        {
        iSession = CMsvSession::OpenSyncL(*this);
        }
    
    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*iSession);
    CleanupStack::PushL(registry);
    
    // get the client mtm and return if it isn't supported in the system        
    CSmtpClientMtm* clientMtm = NULL;
    TRAP(err, clientMtm = static_cast<CSmtpClientMtm*>(registry->NewMtmL(KUidMsgTypeSMTP)));
    
    if (err || !clientMtm)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("Email: Message type module not found"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        }  
    CleanupStack::PushL(clientMtm);   

    // create a new object to access an existing entry
    CMsvEntry* msvEntry = CMsvEntry::NewL(*iSession, KMsvGlobalInBoxIndexEntryId, TMsvSelectionOrdering());
    CleanupStack::PushL(msvEntry);
  
    // get default service
    TMsvId defaultServiceId(0);
    TRAP(err, defaultServiceId = clientMtm->DefaultServiceL());
    if (err)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("Email: Define a mailbox first"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        } 
                
    // set folder type
    switch (parameters.iFolderType)
        {
        case EInbox:    { msvEntry->SetEntryL(KMsvGlobalInBoxIndexEntryId);   break; }
        case EDrafts:   { msvEntry->SetEntryL(KMsvDraftEntryId);              break; }
        case EOutbox:   { msvEntry->SetEntryL(KMsvGlobalOutBoxIndexEntryId);  break; }
        case ESent:     { msvEntry->SetEntryL(KMsvSentEntryId);               break; }
        case EMailbox:  { msvEntry->SetEntryL(iUserSelectedMailbox);          break; }
        default:        { User::Panic(_L("Folder Type"), 871);                break; } 
        }
        
    // mtm takes ownership of entry context	
    CleanupStack::Pop(msvEntry);
    clientMtm->SetCurrentEntryL(msvEntry); 

    // create a new message
    clientMtm->CreateMessageL(defaultServiceId);

    // set subject
    clientMtm->SetSubjectL( parameters.iMessageSubject->Des() );
    
    // set body
    clientMtm->Body().Reset();
    clientMtm->Body().InsertL(0, parameters.iMessageBodyText->Des()); 

    // get the entry of the message
    TMsvEntry messageEntry = clientMtm->Entry().Entry();
    
    // set the address fields
    if (parameters.iFolderType == EInbox)
        {
        AddSenderToMtmAddresseeL(*clientMtm, parameters, ETrue );
        SetSenderToEntryDetails(messageEntry, parameters, ETrue);
        messageEntry.iMtm = KUidMsgTypeIMAP4;  // or any other than KUidMsgTypeSMTP to display 'from' field instead of 'to' field 
        }
    else
        {
        // Add all recipients to clientMtm
        AddRecipientsL( *clientMtm, parameters, ETrue );
        SetRecipientToEntryDetails(messageEntry, parameters, EFalse);        
        }

    // set the description field same as the message subject
    messageEntry.iDescription.Set( parameters.iMessageSubject->Des().Left(KSmsDescriptionLength) );

    // save the changes done above
    clientMtm->Entry().ChangeL(messageEntry);
    
    // get an access to the message store
    CMsvStore* store = msvEntry->EditStoreL();
    CleanupStack::PushL(store); 
    
    // save the attachments
    HandleAttachementsL(parameters, store, err);
//    for (TInt i=0; i<parameters.iAttachments->Count(); i++)
//        {
//        TFileName sourceFileName = iEngine->TestDataPathL( (CCreatorEngine::TTestDataPath) parameters.iAttachments->At(i) );
//
//        TParse parser;
//        parser.Set(sourceFileName, NULL, NULL);
//        TFileName shortFileName = parser.NameAndExt();
//
//        // get the mime type
//        RApaLsSession ls;
//        User::LeaveIfError(ls.Connect());
//        CleanupClosePushL<RApaLsSession>(ls);
//        TUid appUid;
//        TDataType dataType;
//        ls.AppForDocument(sourceFileName, appUid, dataType);
//        CleanupStack::PopAndDestroy(); //ls
//        TPtrC8 mimeType = dataType.Des8();
//        
//        // attachment settings
//        MMsvAttachmentManager& attachMan = store->AttachmentManagerL();
//        CMsvAttachment* attachmentInfo = CMsvAttachment::NewL(CMsvAttachment::EMsvFile); // ownership of attachmentInfo will be transferred
//        attachmentInfo->SetAttachmentNameL( shortFileName );        
//        attachmentInfo->SetMimeTypeL( mimeType );        
//
//        // save
//        CAsyncWaiter* waiter = CAsyncWaiter::NewLC();
//
//        TRAP(err, attachMan.AddAttachmentL(sourceFileName, attachmentInfo, waiter->iStatus ));
//        if (err == KErrNone)
//            waiter->StartAndWait();
//        else
//            LOGSTRING2("Creator: CCreatorMessages::CreateEmailEntryL failed to attach %S", &sourceFileName);
//
//        CleanupStack::PopAndDestroy(waiter);  
//        }

    // save the changes made to the message store
    store->CommitL();
    CleanupStack::PopAndDestroy(store); 
            
    // save the message      
    clientMtm->SaveMessageL();
    	
	// final fine tuning
	TMsvEmailEntry temailEntry = static_cast<TMsvEmailEntry>(messageEntry);
    temailEntry.SetMessageFolderType(EFolderTypeUnknown);
    temailEntry.SetDisconnectedOperation(ENoDisconnectedOperations);
    temailEntry.SetEncrypted(EFalse);
    temailEntry.SetSigned(EFalse);
    temailEntry.SetVCard(EFalse);
    temailEntry.SetVCalendar(EFalse);
    temailEntry.SetReceipt(EFalse);
    temailEntry.SetMHTMLEmail(EFalse);
    temailEntry.SetBodyTextComplete(ETrue);
    
	if (parameters.iAttachments->Count() > 0)
        temailEntry.SetAttachment(ETrue);
    else
        temailEntry.SetAttachment(EFalse);
	
    temailEntry.iDate.HomeTime();
    temailEntry.SetVisible(ETrue);
    temailEntry.SetInPreparation(EFalse);
    if (parameters.iCreateAsUnread)
        {
        temailEntry.SetUnread(ETrue);
        temailEntry.SetNew(ETrue);
        }
    else
        {
        temailEntry.SetUnread(EFalse);
        temailEntry.SetNew(EFalse);
        }
    temailEntry.SetComplete(ETrue);
    temailEntry.iServiceId = defaultServiceId;
    temailEntry.iRelatedId = 0;
    
    clientMtm->Entry().ChangeL(temailEntry);

    // reset email headers
    CImHeader* header = CImHeader::NewLC();
    CMsvStore* msvStore = msvEntry->EditStoreL();
    CleanupStack::PushL(msvStore);
    header->RestoreL(*msvStore);
    header->SetSubjectL( parameters.iMessageSubject->Des() );
    if( iSenderArray.Count() == 0 )
        {
        GetSendersL(iSenderArray, parameters, ETrue, 1 );
        }
    
    if( iSenderArray.Count() > 0 )
        {
        header->SetFromL(iSenderArray[0]->Des());
        }
    else
        {
        header->SetFromL( parameters.iSenderAddress->Des() );
        }
    if( parameters.iRecipientAddress->Length() > 0 )
        {
        header->SetReceiptAddressL( parameters.iRecipientAddress->Des() );
        }
    else if( parameters.iRecipientAddressArray.Count() > 0 )
        {
        header->SetReceiptAddressL( parameters.iRecipientAddressArray[0]->Des());
        }
    
    header->StoreL( *msvStore );
    msvStore->CommitL();
    CleanupStack::PopAndDestroy(2); // msvStore, header
    
    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( messageEntry.Id() );
    
    CleanupStack::PopAndDestroy(2); // registry, clientMtm 

    return err;
    }

//----------------------------------------------------------------------------

TInt CCreatorMessages::CreateSmartMessageEntryL(const CMessagesParameters& parameters)
    {
    LOGSTRING("Creator: CCreatorMessages::CreateSmartMessageEntryL");

    TInt err = KErrNone;

    // init    
    if( iSession == 0 )
        {
        iSession = CMsvSession::OpenSyncL(*this);
        }
    
    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*iSession);
    CleanupStack::PushL(registry);

    // get the client mtm and return if it isn't supported in the system        
    CSmsClientMtm* clientMtm = NULL;
    TRAP(err, clientMtm = static_cast<CSmsClientMtm*>(registry->NewMtmL(KUidMsgTypeSMS)));
    if (err || !clientMtm)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("BIO: Message type module not found"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        }  
    CleanupStack::PushL(clientMtm);   

    // create a new object to access an existing entry
    CMsvEntry* msvEntry = CMsvEntry::NewL(*iSession, KMsvGlobalInBoxIndexEntryId, TMsvSelectionOrdering());
    CleanupStack::PushL(msvEntry);
  
    // get default service
    TMsvId defaultServiceId = 0;
    TRAP(err, defaultServiceId = clientMtm->DefaultServiceL());
    if (err)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("SMS: Define a SMS centre first"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        } 
    
    // set folder type
    switch (parameters.iFolderType)
        {
        case EInbox:    { msvEntry->SetEntryL(KMsvGlobalInBoxIndexEntryId);   break; }
        case EDrafts:   { msvEntry->SetEntryL(KMsvDraftEntryId);              break; }
        case EOutbox:   { msvEntry->SetEntryL(KMsvGlobalOutBoxIndexEntryId);  break; }
        case ESent:     { msvEntry->SetEntryL(KMsvSentEntryId);               break; }
        default:        { User::Panic(_L("Folder Type"), 871);                break; } 
        }
        
    // mtm takes ownership of entry context	
    CleanupStack::Pop(msvEntry);
    clientMtm->SetCurrentEntryL(msvEntry);    
    
    // create a new message
    clientMtm->CreateMessageL(defaultServiceId);

     // set the from field to sms header
    if (parameters.iFolderType == EInbox)
        {
        CSmsHeader& smsHeader = clientMtm->SmsHeader();
        
        if( parameters.iSenderAddress )
            {
            smsHeader.SetFromAddressL( parameters.iSenderAddress->Des() );
            }
        else
            {
            smsHeader.SetFromAddressL(KEmpty);
            }
        }       

    // set body, the BIO message itself
    clientMtm->Body().Reset();
    clientMtm->Body().InsertL(0, _L("Business Card\nTester Mike\nThe Company Ltd.\nSoftware Engineer\ntel +358 66 1234567\n")); 

    // get the entry of the message
    TMsvEntry messageEntry = clientMtm->Entry().Entry();

    // TO-DO: Add support for all BIO messages, currently just insert a business card message ... :
    // set the message type
    // parameters.iBIOMessageType ...
    clientMtm->BioTypeChangedL(KUidBIOBusinessCardMsg);
    messageEntry.iBioType = KUidBIOBusinessCardMsg.iUid;

    // set the details field
    if (parameters.iFolderType == EInbox)
        {
        SetSenderToEntryDetails(messageEntry, parameters, EFalse);        
        }        
    else
        {
        SetRecipientToEntryDetails(messageEntry, parameters, EFalse);
        }

    // set the subject line
    messageEntry.iDescription.Set( parameters.iMessageSubject->Des().Left(KSmsDescriptionLength) );

    // set correct MTM type
    messageEntry.iMtm= KUidBIOMessageTypeMtm;

    // save the changes done above
    clientMtm->Entry().ChangeL(messageEntry);

    // save the message     
    clientMtm->SaveMessageL();
    	
	// final fine tuning
    messageEntry.SetAttachment(EFalse);
    messageEntry.iDate.HomeTime();
    messageEntry.SetVisible(ETrue);
    messageEntry.SetInPreparation(EFalse);
    if (parameters.iCreateAsUnread)
        {
        messageEntry.SetUnread(ETrue);
        messageEntry.SetNew(ETrue);
        }
    else
        {
        messageEntry.SetUnread(EFalse);
        messageEntry.SetNew(EFalse);
        }
    messageEntry.SetComplete(ETrue);
    messageEntry.iServiceId = defaultServiceId;
    messageEntry.iRelatedId = 0;
    clientMtm->Entry().ChangeL(messageEntry);
    
    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( messageEntry.Id() );
    
    CleanupStack::PopAndDestroy(2); // registry, clientMtm
        
    
    return err;
    }

//----------------------------------------------------------------------------

TInt CCreatorMessages::CreateObexEntryL(TUid aMtm, const CMessagesParameters& parameters)
    {
    LOGSTRING("Creator: CCreatorMessages::CreateObexEntryL");

    TInt err = KErrNone;

    // init    
    if( iSession == 0 )
        {
        iSession = CMsvSession::OpenSyncL(*this);
        }    
    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*iSession);
    CleanupStack::PushL(registry);

    // get the client mtm and return if it isn't supported in the system        
    CObexClientMtm* clientMtm = NULL;
    TRAP(err, clientMtm = static_cast<CObexClientMtm*>(registry->NewMtmL(aMtm)));
    if (err || !clientMtm)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("Obex: Message type module not found"));
        CleanupStack::PopAndDestroy(2);
        User::Leave(KErrNotFound);
        }  
    CleanupStack::PushL(clientMtm);   

    // create a new object to access an existing entry
    CMsvEntry* msvEntry = CMsvEntry::NewL(*iSession, KMsvGlobalInBoxIndexEntryId, TMsvSelectionOrdering());
    CleanupStack::PushL(msvEntry);
  
    // define default service
    TMsvId defaultServiceId = 0;
    
    // set folder type
    switch (parameters.iFolderType)
        {
        case EInbox:    { msvEntry->SetEntryL(KMsvGlobalInBoxIndexEntryId);   break; }
        case EDrafts:   { msvEntry->SetEntryL(KMsvDraftEntryId);              break; }
        case EOutbox:   { msvEntry->SetEntryL(KMsvGlobalOutBoxIndexEntryId);  break; }
        case ESent:     { msvEntry->SetEntryL(KMsvSentEntryId);               break; }
        default:        { User::Panic(_L("Folder Type"), 871);                break; } 
        }
        
    // mtm takes ownership of entry context	
    CleanupStack::Pop(msvEntry);
    clientMtm->SetCurrentEntryL(msvEntry);    
    
    // create a new message
    clientMtm->CreateMessageL(defaultServiceId);

    // get the entry of the message
    TMsvEntry messageEntry = clientMtm->Entry().Entry();

    // set subject
    clientMtm->SetSubjectL( parameters.iMessageSubject->Des() );
    messageEntry.iDescription.Set( parameters.iMessageSubject->Des().Left(KSmsDescriptionLength) );

    // set body, must be empty for obex messages
    clientMtm->Body().Reset();

    // set the details field and
    if (parameters.iFolderType == EInbox)
        {
        SetSenderToEntryDetails(messageEntry, parameters, EFalse);
        }        
    else
        {
        SetRecipientToEntryDetails(messageEntry, parameters, EFalse);
        }
    
    // set mtm
    messageEntry.iMtm = aMtm;
    messageEntry.iType = KUidMsvMessageEntry;
    messageEntry.iServiceId = KMsvUnknownServiceIndexEntryId;

    // save the changes done above
    clientMtm->Entry().ChangeL(messageEntry);

    // save the message      
    clientMtm->SaveMessageL();
    	
	// final fine tuning
    messageEntry.iDate.HomeTime();
    messageEntry.SetVisible(ETrue);
    messageEntry.SetInPreparation(EFalse);
    if (parameters.iCreateAsUnread)
        {
        messageEntry.SetUnread(ETrue);
        messageEntry.SetNew(ETrue);
        }
    else
        {
        messageEntry.SetUnread(EFalse);
        messageEntry.SetNew(EFalse);
        }
    messageEntry.SetComplete(ETrue);
    clientMtm->Entry().ChangeL(messageEntry);
    
    // save the attachment
    if (parameters.iAttachments->Count() >= 1)
        {
        // create a new entry for the attachment
        TMsvEntry attachTEntry;
        attachTEntry.iType      = KUidMsvAttachmentEntry;
        attachTEntry.iServiceId = KMsvUnknownServiceIndexEntryId;   
        attachTEntry.iMtm       = KUidMsgTypeBt; //save as bt message
        
        msvEntry->CreateL(attachTEntry);
        
        CMsvEntry* attachEntry = iSession->GetEntryL(attachTEntry.Id());
        clientMtm->SetCurrentEntryL(attachEntry);
                
        // get source file
        TFileName sourceFileName = iEngine->TestDataPathL( (CCreatorEngine::TTestDataPath) parameters.iAttachments->At(0) );

        // get the mime type
        RApaLsSession ls;
        User::LeaveIfError(ls.Connect());
        CleanupClosePushL<RApaLsSession>(ls);
        TUid appUid;
        TDataType mimeType;
        ls.AppForDocument(sourceFileName, appUid, mimeType);
        CleanupStack::PopAndDestroy(); //ls
        
        CAsyncWaiter* waiter = CAsyncWaiter::NewLC();

        // add an attachment to the current message entry
        TRAP(err, clientMtm->AddAttachmentL( sourceFileName, mimeType.Des8(), 0, waiter->iStatus ));
        if (err == KErrNone)
            waiter->StartAndWait();
        else
            LOGSTRING2("Creator: CCreatorMessages::CreateObexEntryL failed to attach %S", &sourceFileName);
        
        CleanupStack::PopAndDestroy(waiter);
        }
    
    // id has been generated, store it for being able to delete
    // only entries created with Creator
    iEntryIds.Append( messageEntry.Id() );
    
    CleanupStack::PopAndDestroy(2); // registry, clientMtm 


    return err;
    }

//----------------------------------------------------------------------------

void CCreatorMessages::HandleSessionEventL(TMsvSessionEvent /*aEvent*/, TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/) // from MMsvSessionObserver
    {
    }

//----------------------------------------------------------------------------
void CCreatorMessages::SetSenderToEntryDetails(TMsvEntry& aMsgEntry, const CMessagesParameters& aParameters, TBool aUseEmailAddress)
    {        
    // Only one sender allowed:
    if( iSenderArray.Count() == 0 )
        {
        GetSendersL(iSenderArray, aParameters, aUseEmailAddress, 1 );
        }
    if( iSenderArray.Count() > 0 )
        {        
        aMsgEntry.iDetails.Set( iSenderArray[0]->Des() );        
        }
    else
        {
        aMsgEntry.iDetails.Set(KEmpty);
        }        
    }

//----------------------------------------------------------------------------
void CCreatorMessages::SetRecipientToEntryDetails(TMsvEntry& aMsgEntry, const CMessagesParameters& aParameters, TBool aUseEmailAddress)
    {        
    // Only one sender allowed:
    GetAllRecipientsL(iRecipientArray, aParameters, aUseEmailAddress);
    if( iRecipientArray.Count() > 0 )
        {
        const TDesC& temp = iRecipientArray[0]->Des();
        aMsgEntry.iDetails.Set( temp );
        }
    else
        {
        aMsgEntry.iDetails.Set(KEmpty);
        }        
    }

//----------------------------------------------------------------------------
void CCreatorMessages::AddSenderToMtmAddresseeL(CBaseMtm& aMtm, const CMessagesParameters& aParameters, TBool aUseEmailAddress )
    {    
    // Only one sender allowed:
    if( iSenderArray.Count() == 0 )
        {
        GetSendersL(iSenderArray, aParameters, aUseEmailAddress, 1 );
        }
    
    for( TInt i = 0; i < iSenderArray.Count(); ++i )
        {
        aMtm.AddAddresseeL( iSenderArray[i]->Des() );        
        }    
    }

//----------------------------------------------------------------------------
void CCreatorMessages::AddMtmSenderL(CMmsClientMtm& aMtm, const CMessagesParameters& aParameters, TBool aUseEmailAddress )
    {    
    // Only one sender allowed:
    if( iSenderArray.Count() == 0 )
        {
        GetSendersL(iSenderArray, aParameters, aUseEmailAddress, 1 );
        }
    
    if( iSenderArray.Count() > 0 )
        {
        aMtm.SetSenderL( iSenderArray[0]->Des() );        
        }    
    }

//----------------------------------------------------------------------------
void CCreatorMessages::GetSendersL(RPointerArray<HBufC>& aSenderArray, const CMessagesParameters& aParameters, TBool aUseEmailAddress, TInt aMaxNum )
    {
    TInt counter = 0;
    
    if( counter < aMaxNum || aMaxNum == KUndef )
        {
        if( aParameters.iSenderAddress && aParameters.iSenderAddress->Length() > 0 )
            {
            HBufC* temp = HBufC::NewL(aParameters.iSenderAddress->Length());
            CleanupStack::PushL(temp);
            temp->Des().Copy(aParameters.iSenderAddress->Des());
            aSenderArray.AppendL( temp );
            CleanupStack::Pop(); // temp
            ++counter;
            }
        }
            
    if( counter < aMaxNum || aMaxNum == KUndef )
        {
        RPointerArray<HBufC> tempArray;
        CleanupResetAndDestroyPushL( tempArray );
        GetLinkedAddressesL(tempArray, aParameters.iSenderLinkIds, aUseEmailAddress, aParameters.iNumberOfExistingSenders);
        for( TInt i = 0; i < tempArray.Count() && (counter < aMaxNum || aMaxNum == KUndef); ++i )
            {
            HBufC* temp = tempArray[i]->AllocLC();
            aSenderArray.AppendL(temp);
            CleanupStack::Pop(); // temp
            ++counter;
            }
        CleanupStack::PopAndDestroy(); // tempArray
        }    
    }

//----------------------------------------------------------------------------
void CCreatorMessages::AddRecipientsL(CBaseMtm& aMtm, const CMessagesParameters& aParameters, TBool aUseEmailAddress )
    {
    GetAllRecipientsL(iRecipientArray, aParameters, aUseEmailAddress);
    for( TInt i = 0; i < iRecipientArray.Count(); ++i )
        {
        aMtm.AddAddresseeL( iRecipientArray[i]->Des() );        
        }    
    }

//----------------------------------------------------------------------------
void CCreatorMessages::GetAllRecipientsL(RPointerArray<HBufC>& aRecipientArray, const CMessagesParameters& aParameters, TBool aUseEmailAddress )
    {
    
    // If iRecipientAddress is given, add it to the array:
    if( aParameters.iRecipientAddress && aParameters.iRecipientAddress->Length() > 0 )
        {
        HBufC* temp = HBufC::NewL(aParameters.iRecipientAddress->Length());
        CleanupStack::PushL(temp);
        temp->Des().Copy(aParameters.iRecipientAddress->Des());
        aRecipientArray.AppendL( temp );
        CleanupStack::Pop(); // temp
        }
    
    // Add all recipients listed in iRecipientAddressArray:
    for( TInt i = 0; i < aParameters.iRecipientAddressArray.Count(); ++i )
        {
        HBufC* temp = HBufC::NewL(aParameters.iRecipientAddressArray[i]->Length());
        CleanupStack::PushL(temp);
        temp->Des().Copy(aParameters.iRecipientAddressArray[i]->Des());
        aRecipientArray.AppendL( temp );
        CleanupStack::Pop(); // temp        
        }
    
    // Get all linked addresses. 
    GetLinkedAddressesL(aRecipientArray, aParameters.iRecipientLinkIds, aUseEmailAddress, aParameters.iNumberOfExistingRecipients);
    }

/*
 * Get all linked addresses. This method gets addresses that are linked with contact-set id and 
 * existing addresses from the contact database, if required.
 */
void CCreatorMessages::GetLinkedAddressesL(
        RPointerArray<HBufC>& aAddressArray, 
        const RArray<TLinkIdParam>& aLinkIds, 
        TBool aUseEmailAddress,
        TInt aNumOfExistingAddresses )
    {
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
    
    if( aNumOfExistingAddresses > 0 || aLinkIds.Count() > 0 )
        {
        // First create contact manager:
        if( iContactManager == 0)
        	{
        	CVPbkContactStoreUriArray* uriArray = CVPbkContactStoreUriArray::NewLC();
        	uriArray->AppendL( TVPbkContactStoreUriPtr(VPbkContactStoreUris::DefaultCntDbUri()));
        	iContactManager = CVPbkContactManager::NewL( *uriArray, &CCoeEnv::Static()->FsSession());
        	CleanupStack::PopAndDestroy(uriArray);

        
            // Then open the stores:
                
            MVPbkContactStoreList& storeList = iContactManager->ContactStoresL();              
            storeList.OpenAllL(*this);
            iWaiter->StartAndWait(); 
            _LIT(dbUri, "cntdb://c:contacts.cdb");
            TVPbkContactStoreUriPtr uri(dbUri);                   
            iStore = storeList.Find(uri);
        	}
        // Existing attendees:
        if( aNumOfExistingAddresses > 0 )
        	{        	
        	if( iStore )
        		{
        		const MVPbkFieldTypeList& fieldList = iStore->StoreProperties().SupportedFields();

        		// get contacts, results will be set to iExistingContacts
        		MVPbkContactOperationBase* operation = iContactManager->FindL(_L(""), fieldList , *this);
        		if (operation)
        			{
        			CleanupDeletePushL( operation );
        			iWaiter->StartAndWait();//Making asynchronous FindL to synchronous
        			CleanupStack::Pop(operation);
        			delete operation;
        			}
        		if( iExistingContacts )
        			{
        			// Loop the links pointing to the existing contacts:
        			TInt addedContacts = 0;
        			for( TInt i = 0; i < iExistingContacts->Count() && addedContacts < aNumOfExistingAddresses; ++i )
        				{
        				// Get the address of the actual contact based on the contact link. Results are stored in
        				// iTmpEmail and iTmpPhoneNumber
        				MVPbkContactOperationBase* operation = iContactManager->RetrieveContactL( iExistingContacts->At(i), *this );
        				if (operation)
        					{
        					CleanupDeletePushL( operation );
        					iWaiter->StartAndWait(); // Making asynchronous RetrieveContactL to synchronous
        					CleanupStack::Pop(); // operation
        					delete operation;
        					operation = 0;
        					HandleSingleContactResultL();
        					if( aUseEmailAddress && iTmpEmail && iTmpEmail->Length() > 0 )
        						{
        						HBufC* temp = HBufC::NewL(iTmpEmail->Length());
        						CleanupStack::PushL(temp);
        						temp->Des().Copy(iTmpEmail->Des());
        						aAddressArray.AppendL(temp);
        						CleanupStack::Pop(); // temp
        						delete iTmpEmail;
        						iTmpEmail = 0; // The pointer is owned by the aAddressArray now.    
        						++addedContacts;
        						}
        					else if( !aUseEmailAddress && iTmpPhoneNumber && iTmpPhoneNumber->Length() > 0 )
        						{
        						HBufC* temp = HBufC::NewL(iTmpPhoneNumber->Length());
        						CleanupStack::PushL(temp);
        						temp->Des().Copy(iTmpPhoneNumber->Des());
        						aAddressArray.AppendL(temp);
        						CleanupStack::Pop(); // temp
        						delete iTmpPhoneNumber;
        						iTmpPhoneNumber = 0; // The pointer is owned by the aAddressArray now.
        						++addedContacts;
        						}
        					}                            
        				}
        			}
        		} 
        	}

        // Loop linked contact-set ids:
        for( TInt i = 0; i < aLinkIds.Count(); ++i )
        	{
        	TLinkIdParam id = aLinkIds[i];
        	// Get all contact links in the contact-set...
        	RPointerArray<MVPbkContactLink>& links = ContactLinkCache::Instance()->ContactLinks(id.iLinkId);
        	// ... and loop the links:
        	TInt addedContacts = 0;
        	for( TInt j = 0; j < links.Count() && (id.iLinkAmount < 0 || addedContacts < id.iLinkAmount); ++j )
        		{
        		// Again retrieve contact based on the contact link. Results are stored in
        		// iTmpEmail and iTmpPhoneNumber:
        		MVPbkContactOperationBase* operation = iContactManager->RetrieveContactL( *(links[j]), *this );
        		if (operation)
        			{
        			CleanupDeletePushL( operation );
        			iWaiter->StartAndWait(); // Making asynchronous RetrieveContactL to synchronous
        			CleanupStack::Pop(operation);
        			delete operation;
        			operation = 0;
        			HandleSingleContactResultL();
        			if( aUseEmailAddress && iTmpEmail && iTmpEmail->Length() > 0 )
        				{                                    
        				aAddressArray.AppendL(iTmpEmail);
        				iTmpEmail = 0; // The pointer is owned by the aAddressArray now.
        				++addedContacts;
        				}
        			else if( !aUseEmailAddress && iTmpPhoneNumber && iTmpPhoneNumber->Length() > 0 )
        				{
        				aAddressArray.AppendL(iTmpPhoneNumber);
        				iTmpPhoneNumber = 0; // The pointer is owned by the aAddressArray now.
        				++addedContacts;
        				}
        			}                 
        		}
        	}

        //if( iContactManager )
        //	iContactManager->ContactStoresL().CloseAll(*this);
        }
#endif
}

//----------------------------------------------------------------------------
#if(!defined __SERIES60_30__ && !defined __SERIES60_31__)
void CCreatorMessages::VPbkSingleContactOperationComplete(
            MVPbkContactOperationBase& /*aOperation*/,
            MVPbkStoreContact* aContact )
    {
    LOGSTRING("Creator: CCreatorMessages::VPbkSingleContactOperationComplete" );
    iWaiter->Cancel();
    delete iTempContact;    
    iTempContact = aContact;
    }

void CCreatorMessages::HandleSingleContactResultL()
	{
    if( iTempContact )
        {        
        TPtrC phoneNumber;        
        TPtrC email;
       
        // Put contact's name to temporary member:
        delete iTmpPhoneNumber;
        iTmpPhoneNumber = 0;
        delete iTmpEmail;
        iTmpEmail = 0;
       
        TBool phFound = EFalse;
        TBool emailFound = EFalse;
        
        MVPbkStoreContactFieldCollection& fields = iTempContact->Fields();
        TInt fieldCount = fields.FieldCount();
        for (TInt i = 0; i < fieldCount; ++i)
            {
            MVPbkStoreContactField& field = fields.FieldAt(i);
            MVPbkContactFieldData& fieldData = field.FieldData();
            const MVPbkFieldType* fieldType = field.BestMatchingFieldType();
            
            if( fieldType ) 
                {
                // Find phone number and email:
                TInt fieldId( fieldType->FieldTypeResId() );
            
                TVPbkFieldStorageType dataType = fieldData.DataType();
                if( dataType == EVPbkFieldStorageTypeText)
                    {
                    MVPbkContactFieldTextData& textData = MVPbkContactFieldTextData::Cast(fieldData);                
                    if( !phFound &&
                        (fieldId == R_VPBK_FIELD_TYPE_LANDPHONEGEN ||
                        fieldId == R_VPBK_FIELD_TYPE_LANDPHONEHOME ||
                        fieldId == R_VPBK_FIELD_TYPE_LANDPHONEWORK ||                      
                        fieldId == R_VPBK_FIELD_TYPE_MOBILEPHONEGEN ||
                        fieldId == R_VPBK_FIELD_TYPE_MOBILEPHONEHOME ||
                        fieldId == R_VPBK_FIELD_TYPE_MOBILEPHONEWORK ))
                        {                    
                        phoneNumber.Set(textData.Text());
                        phFound = ETrue;
                        }
                    else if( !emailFound &&
                            (fieldId == R_VPBK_FIELD_TYPE_EMAILGEN ||
                            fieldId == R_VPBK_FIELD_TYPE_EMAILHOME ||
                            fieldId == R_VPBK_FIELD_TYPE_EMAILWORK ))
                        {
                        email.Set(textData.Text());
                        emailFound = ETrue;
                        }
                    
                    if( phFound && emailFound )
                        break; // No need to loop more fields.
                    }            
                }
            }
        if( phoneNumber.Length() > 0 )
            {
            delete iTmpPhoneNumber;
            iTmpPhoneNumber = 0;
            iTmpPhoneNumber = HBufC::NewL(phoneNumber.Length());
            iTmpPhoneNumber->Des().Copy(phoneNumber); 
            }                
        if( email.Length() > 0 )
            {
            delete iTmpEmail;
            iTmpEmail = 0;
            iTmpEmail = HBufC::NewL(email.Length());            	
            iTmpEmail->Des().Copy(email);
            }
        delete iTempContact;
        iTempContact = 0;
        }
    }
    
void CCreatorMessages::VPbkSingleContactOperationFailed(
            MVPbkContactOperationBase& /*aOperation*/, 
            TInt aError )
    {
    LOGSTRING2("Creator: CCreatorMessages::VPbkSingleContactOperationFailed - Error: %d", aError );
    iWaiter->Cancel();
    }

void CCreatorMessages::OpenComplete()
    {    
    LOGSTRING("Creator: CCreatorMessages::OpenComplete()");
    iWaiter->Cancel();
    }

void CCreatorMessages::StoreReady(MVPbkContactStore& /*aContactStore*/)
    {
    LOGSTRING("Creator: CCreatorMessages::StoreReady()");
    iWaiter->Cancel();
    }

void CCreatorMessages::StoreUnavailable(MVPbkContactStore& /*aContactStore*/, 
        TInt /*aReason*/)
    {
    LOGSTRING("Creator: CCreatorMessages::StoreUnavailable()");
    iWaiter->Cancel();
    }


void CCreatorMessages::HandleStoreEventL(
        MVPbkContactStore& /*aContactStore*/, 
        TVPbkContactStoreEvent /*aStoreEvent*/)
    {
    LOGSTRING("Creator: CCreatorMessages::HandleStoreEventL()");
    iWaiter->Cancel();
    }

void CCreatorMessages::FindCompleteL( MVPbkContactLinkArray* aResults )
    {
    LOGSTRING("Creator: CCreatorMessages::FindCompleteL()");
    iWaiter->Cancel();
    delete iExistingContacts;
    iExistingContacts = aResults;
    }

void CCreatorMessages::FindFailed( TInt aError )
    {
    LOGSTRING2("Creator: CCreatorMessages:FindFailed() - Error: %d", aError);    
    iWaiter->Cancel();
    delete iExistingContacts;
    iExistingContacts = 0;
    }
#endif


//----------------------------------------------------------------------------
void CCreatorMessages::DeleteAllL()
    {
    LOGSTRING("Creator: CCreatorMessages::DeleteAllL");
    DeleteAllMessagesL( EFalse );
    }

//----------------------------------------------------------------------------
void CCreatorMessages::DeleteAllMessagesL( TBool aOnlyCreatedWithCreator )
    {
    LOGSTRING("Creator: CCreatorMessages::DeleteAllMessagesL");
    TInt err( KErrNone );
    
    // init    
    if( iSession == 0 )
        {
        iSession = CMsvSession::OpenSyncL(*this);
        }
    
    CClientMtmRegistry* registry = CClientMtmRegistry::NewL(*iSession);
    CleanupStack::PushL( registry );

    // get the client mtm and return if it isn't supported in the system        
    CSmsClientMtm* clientMtm( NULL );
    TRAP( err, clientMtm = static_cast<CSmsClientMtm*>( registry->NewMtmL( KUidMsgTypeSMS ) ) );
    if ( err || !clientMtm )
        {
        CAknErrorNote* note = new (ELeave) CAknErrorNote();
        note->ExecuteLD( _L( "SMS: Message type module not found" ) );
        CleanupStack::PopAndDestroy( 2 );
        User::Leave( KErrNotFound );
        }
    CleanupStack::PushL( clientMtm );
    
    TRAP_IGNORE( DeleteAllFromFolderL( KMsvGlobalInBoxIndexEntryId, iSession, clientMtm, aOnlyCreatedWithCreator ) );
    TRAP_IGNORE( DeleteAllFromFolderL( KMsvDraftEntryId, iSession, clientMtm, aOnlyCreatedWithCreator ) );
    TRAP_IGNORE( DeleteAllFromFolderL( KMsvGlobalOutBoxIndexEntryId, iSession, clientMtm, aOnlyCreatedWithCreator ) );
    TRAP_IGNORE( DeleteAllFromFolderL( KMsvSentEntryId, iSession, clientMtm, aOnlyCreatedWithCreator ) );
    
    // reset must be done here, because iEntryIds is stored in destructor
    iEntryIds.Reset();
    
    // all entries deleted, remove the Messages related registry
    iEngine->RemoveStoreL( KUidDictionaryUidMessages );

    CleanupStack::PopAndDestroy( clientMtm );
    CleanupStack::PopAndDestroy( registry );    
    }

//----------------------------------------------------------------------------
void CCreatorMessages::DeleteAllFromFolderL( const TMsvId aContext, 
                                             CMsvSession* aSession,
                                             CSmsClientMtm* aClientMtm,
                                             TBool aOnlyCreatedWithCreator )
    {
    LOGSTRING("Creator: CCreatorMessages::DeleteAllFromFolderL");
    
    TMsvSelectionOrdering sort;
    sort.SetShowInvisibleEntries( ETrue );

    CMsvEntry* inboxContext = CMsvEntry::NewL( *aSession, aContext, sort );
    CleanupStack::PushL( inboxContext );

    CMsvEntrySelection* entries = inboxContext->ChildrenL();
    CleanupStack::PushL( entries );

    TInt msgCount = entries->Count();
    for ( TInt i = 0; i < entries->Count(); i++)
        {
        TMsvId entryID = entries->At(i);
        aClientMtm->SwitchCurrentEntryL( entryID );

        if ( !aOnlyCreatedWithCreator || iEntryIds.Find( entryID ) != KErrNotFound )
            {
            CMsvEntry* entry = aSession->GetEntryL( (*entries)[i] );
            CleanupStack::PushL( entry );

            entry->DeleteL( entries->At(i) );
            CleanupStack::PopAndDestroy( entry );            
            }
        }
    
    CleanupStack::PopAndDestroy( entries );
    CleanupStack::PopAndDestroy( inboxContext );
    }

//----------------------------------------------------------------------------
void CCreatorMessages::DeleteAllCreatedByCreatorL()
    {
    LOGSTRING("Creator: CCreatorMessages::DeleteAllCreatedByCreatorL");
    
    iEntryIds.Reset();
    
    // fetch ids of entries created by Creator
    iEngine->ReadEntryIdsFromStoreL( iEntryIds, KUidDictionaryUidMessages );
    
    // delete them
    DeleteAllMessagesL( ETrue );
    }

CRecipientInfo::CRecipientInfo() 
    {
    
    }

CRecipientInfo::~CRecipientInfo()
    {
    delete iPhoneNumber;
    delete iEmailAddress;
    }

void CRecipientInfo::SetPhoneNumber(HBufC* aPhone)
    {
    delete iPhoneNumber;    
    iPhoneNumber = aPhone;    
    }

void CRecipientInfo::SetEmailAddress(HBufC* aEmail)
    {
    delete iEmailAddress;    
    iEmailAddress = aEmail;    
    }

const HBufC* CRecipientInfo::PhoneNumber() const
    {
    return iPhoneNumber;
    }

const HBufC* CRecipientInfo::EmailAddress() const
    {
    return iEmailAddress;
    }
