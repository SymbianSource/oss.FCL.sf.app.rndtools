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


// INCLUDE FILES
#include "FBFileEditor.h"
#include "FBStd.h"
#include "FB.hrh"
#include <filebrowser.rsg>

#include <utf.h>
#include <eikedwin.h>
#include <eikrted.h>
#include <txtrich.h>
#include <eikspane.h>
#include <avkon.hrh> 
#include <akntitle.h>
#include <AknUtils.h> 
#include <AknBidiTextUtils.h>
#include <aknnotewrappers.h>
#include <eiksbfrm.h>
#include <AknDef.h>
#include <layoutmetadata.cdl.h>

const TUint KLeftMargin = 2;
const TUint KRightMargin = 2;
const TUint KHexSeparatorMargin = 4;

const TInt KEditorFontHeight = 117;
const TInt KViewerFontHeight = 117;
const TInt KEditorFontHeightQHD = 100;
const TInt KViewerFontHeightQHD = 100;



// ================= MEMBER FUNCTIONS =======================

CFileBrowserFileEditorDlg* CFileBrowserFileEditorDlg::NewL(const TDesC& aFileName, TInt aMode)
    {
    CFileBrowserFileEditorDlg* self = new(ELeave) CFileBrowserFileEditorDlg(aFileName, aMode);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;    
    }

// --------------------------------------------------------------------------------------------

CFileBrowserFileEditorDlg::~CFileBrowserFileEditorDlg()
    {
    if (iEditorPF)
        delete iEditorPF;
    }

// --------------------------------------------------------------------------------------------

CFileBrowserFileEditorDlg::CFileBrowserFileEditorDlg(const TDesC& aFileName, TInt aMode) :
    iFileName(aFileName), iActiveMode(aMode)
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::ConstructL()
    {
    // construct a menu bar
    if (iActiveMode == EFileEditorEditAsText || iActiveMode == EFileEditorEditAsHex)
        CAknDialog::ConstructL(R_FILEBROWSER_FILEEDITOR_MENUBAR);
    else
        CAknDialog::ConstructL(R_FILEBROWSER_FILEVIEWER_MENUBAR);
    
    iTextFormat = EFileBrowserCmdFileEditorSaveFormatANSIASCII;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::PreLayoutDynInitL()
    {
    CAknDialog::PreLayoutDynInitL();
    
    // set title text
    CEikStatusPane* sp = iEikonEnv->AppUiFactory()->StatusPane();
    CAknTitlePane* tp = static_cast<CAknTitlePane*>( sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
    tp->SetTextL( _L("File editor") );
    
    
    // init edwin editor component
    if (iActiveMode == EFileEditorEditAsText || iActiveMode == EFileEditorEditAsHex)
        {
        CCoeControl* control = CreateLineByTypeL(KNullDesC, EFileEditorDialogId, EEikCtEdwin, NULL);
        iTextEditor = static_cast<CEikEdwin*>(control);
        iTextEditor->SetMopParent(this);
        iTextEditor->ConstructL(EEikEdwinOwnsWindow|EEikEdwinNoHorizScrolling|EEikEdwinInclusiveSizeFixed|EEikEdwinNoAutoSelection, 0, 0, 0 );
        iTextEditor->CreateTextViewL();
        iTextEditor->SetAknEditorFlags(EAknEditorFlagEnableScrollBars);
        iTextEditor->CreateScrollBarFrameL();
        iTextEditor->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
        iTextEditor->ForceScrollBarUpdateL(); 
        iTextEditor->SetBackgroundColorL(KRgbWhite);
        iTextEditor->SetFocus(ETrue);
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::PostLayoutDynInitL()
    {
    CAknDialog::PostLayoutDynInitL();
    
    // modify layout of the texteditor
    if (iActiveMode == EFileEditorEditAsText || iActiveMode == EFileEditorEditAsHex)
        {
        CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
        const TSize screenSize = dev->SizeInPixels();
        
        // Set the font of the editor
        TFontSpec fontSpec = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont)->FontSpecInTwips();
        fontSpec.iHeight = IsQHD(screenSize) ? KEditorFontHeightQHD : KEditorFontHeight; // set height
        
        TCharFormat cf(fontSpec.iTypeface.iName, fontSpec.iHeight);
        cf.iFontPresentation.iTextColor = KRgbBlack; 

        TCharFormatMask cfm;
        cfm.SetAll();

        CCharFormatLayer *charFL = CCharFormatLayer::NewL(cf, cfm);
        iTextEditor->SetCharFormatLayer(charFL);

        if (iEditorPF)
            {
            delete iEditorPF;
            iEditorPF = NULL;
            }

        iEditorPF = CParaFormat::NewL();
        iEditorPF->iLineSpacingInTwips = IsQHD(screenSize) ? KEditorFontHeightQHD : KEditorFontHeight;
        iEditorPF->iFillColor = KRgbWhite;

        TParaFormatMask pfm;
        pfm.SetAll();

        CParaFormatLayer *paraFL = CParaFormatLayer::NewL(iEditorPF, pfm);
        iTextEditor->SetParaFormatLayer(paraFL);
        

        // set size of the editor
        TRect rect = Rect();
        iTextEditor->SetRect(rect);
        //iTextEditor->Size().iWidth - (CEikScrollBar::DefaultScrollBarBreadth());
        }

    // load the file to editor or viewer
    LoadFileL();
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::OpenFileLC(RFile& aFile, const TDesC& aFileName)
    {
    TInt err = aFile.Open(CEikonEnv::Static()->FsSession(), aFileName, EFileRead|EFileShareReadersOnly);
    
    if (err == KErrInUse)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("The file is in use, cannot open"));
        User::Leave(KErrNone);
        }
    else if (err != KErrNone)
        {
        CAknErrorNote* note = new(ELeave) CAknErrorNote;
        note->ExecuteLD(_L("Cannot open the file"));
        User::Leave(KErrNone);
        }
    else
        {
        // opened succesfully, add to cleanup stack
        CleanupClosePushL(aFile);
        }     
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::GetTextFileMode(RFile& aFile, TInt& aFileSize)
    {
    iTextFormat = EFileBrowserCmdFileEditorSaveFormatANSIASCII;

    // if we are working with text files, check the type first   
    if (iActiveMode == EFileEditorEditAsText || iActiveMode == EFileEditorViewAsText)
        {
        TBuf8<4> BOM;

        // first check for UTF-16
        if (aFileSize >= 2 && aFile.Read(0, BOM, 2) == KErrNone)
            {
            if (BOM.Length()==2 && BOM[0]==0xFF && BOM[1]==0xFE)
                {
                iTextFormat = EFileBrowserCmdFileEditorSaveFormatUTF16LE;
                aFileSize -= 2;
                return;
                }
            else if (BOM.Length()==2 && BOM[0]==0xFE && BOM[1]==0xFF)
                {
                iTextFormat = EFileBrowserCmdFileEditorSaveFormatUTF16BE;
                aFileSize -= 2;
                return;
                }
            }       

        // then check for UTF-8
        if (aFileSize >= 3 && aFile.Read(0, BOM, 3) == KErrNone)
            {
            if (BOM.Length()==3 && BOM[0]==0xEF && BOM[1]==0xBB && BOM[2]==0xBF)
                {
                iTextFormat = EFileBrowserCmdFileEditorSaveFormatUTF8;
                aFileSize -= 3;
                return;
                }
            }
            
        
        // none of those, seek back to beginning
        TInt pos(0);
        aFile.Seek(ESeekStart, pos);
        }    
    }


// --------------------------------------------------------------------------------------------

HBufC16* CFileBrowserFileEditorDlg::ConvertBuf8ToBuf16L(HBufC8* aBuf8)
    {
    __ASSERT_ALWAYS(aBuf8 != NULL, User::Panic(_L("Emp.Buf."),942));

    HBufC16* buf16 = NULL;
    TPtr8 buf8Ptr(aBuf8->Des()); 

    if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatANSIASCII)
        {
        // simple conversion
        buf16 = HBufC16::NewMaxL(aBuf8->Length());
        buf16->Des().Copy(aBuf8->Des());
        }
    
    else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF8)
        {
        // use the conversion service
        buf16 = CnvUtfConverter::ConvertToUnicodeFromUtf8L(aBuf8->Des());
        }            

    else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16LE || iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16BE)
        {
        buf16 = HBufC16::NewL(aBuf8->Length());
        buf16->Des().Copy(KNullDesC);
        
        HBufC16* oneUniCodeChar = HBufC16::NewMaxLC(1);
        TUint16* oneUniCodeCharPtr = &oneUniCodeChar->Des()[0];
       
        for (TInt i=0; i<buf8Ptr.Length(); i+=2)
            {
            // get two 8-bit values and pack them to one 16-bit character depending of endianess

            if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16LE)
                {
                oneUniCodeCharPtr[0] = TUint16((buf8Ptr[i+1] << 8) + buf8Ptr[i]);
                }
            else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16BE)
                {
                oneUniCodeCharPtr[0] = TUint16((buf8Ptr[i] << 8) + buf8Ptr[i+1]);
                }
            
            // append to buffer
            buf16->Des().Append( oneUniCodeChar->Des() );
            }
        
        CleanupStack::PopAndDestroy(); // oneUniCodeChar
        } 
    
    return buf16; //ownership of buf16 will transferred to the caller           
    }

// --------------------------------------------------------------------------------------------

HBufC8* CFileBrowserFileEditorDlg::ConvertBuf16ToBuf8L(HBufC16* aBuf16)
    {
    __ASSERT_ALWAYS(aBuf16 != NULL, User::Panic(_L("Emp.Buf."),943));

    HBufC8* buf8 = NULL;
    TPtr16 buf16Ptr(aBuf16->Des()); 

    if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatANSIASCII)
        {
        // simple conversion
        buf8 = HBufC8::NewMaxL(aBuf16->Length());
        buf8->Des().Copy(aBuf16->Des());
        }
    
    else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF8)
        {
        // use the conversion service
        buf8 = CnvUtfConverter::ConvertFromUnicodeToUtf8L(aBuf16->Des());
        }            

    else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16LE || iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16BE)
        {
        buf8 = HBufC8::NewL(aBuf16->Length()*2);
        buf8->Des().Copy(KNullDesC);

        TUint16* charPtr = NULL;
        TBuf8<2> twoChars;
        twoChars.Copy(_L8("XX"));        
       
        for (TInt i=0; i<aBuf16->Length(); i++)
            {
            charPtr = &aBuf16->Des()[i];
            
            // get 16-bit character to two variables
            if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16LE)
                {
                twoChars[0] = TUint8(charPtr[0]);
                twoChars[1] = TUint8(charPtr[0] << 8);
                }
            else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16BE)
                {
                twoChars[0] = TUint8(charPtr[0] << 8);
                twoChars[1] = TUint8(charPtr[0]);
                }

            // append to the buffer
            buf8->Des().Append(twoChars);

            } // for
            
        } // else if
    
    return buf8; //ownership of buf8 will transferred to the caller           
    }                
// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::LoadFileL()
    {
    // open the file for reading
    RFile fileP;
    OpenFileLC(fileP, iFileName);
    
    // get file size
    TInt fileSize(0);
    User::LeaveIfError(fileP.Size(fileSize));

    // read BOM from file
    GetTextFileMode(fileP, fileSize);

    
    // editor related file loading
    if (iActiveMode == EFileEditorEditAsText || iActiveMode == EFileEditorEditAsHex)
        {
        // make sure that there is currently no text in the editor 
        iTextEditor->Text()->Reset();

        // set character handling related cases for the editor
        if (iActiveMode == EFileEditorEditAsText)
            {
            iTextEditor->SetOnlyASCIIChars(EFalse);
            iTextEditor->SetAknEditorCase(EAknEditorTextCase);
            }
        else if (iActiveMode == EFileEditorEditAsHex)
            {
            iTextEditor->SetOnlyASCIIChars(ETrue);
            iTextEditor->SetAknEditorCase(EAknEditorUpperCase);
            }  
            
        const TInt KBufSize(1024);
        TInt fileOffset(0);
        TInt printOffset(0);

        HBufC8* buf = HBufC8::NewLC(KBufSize);
        TPtr8 bufPtr(buf->Des());
        TChar ch;
        _LIT(KHex, "%02x");
        TBuf<3> hexBuf;

        // read to buffer and supply the editor
        while (fileOffset < fileSize)
            {
            // read to buffer
            User::LeaveIfError( fileP.Read(bufPtr, KBufSize) );
            
            // handle in text format
            if (iActiveMode == EFileEditorEditAsText)
                {
                HBufC16* buf16 = ConvertBuf8ToBuf16L(buf);
                CleanupStack::PushL(buf16);
                TPtr16 buf16Ptr(buf16->Des());

                // replace any line breaks and append to the editor
                for (TInt i=0; i<buf16Ptr.Length(); i++)
                    {
                    ch = buf16Ptr[i] != '\n' ? (TChar)buf16Ptr[i] : (TChar)CEditableText::ELineBreak;
                    iTextEditor->Text()->InsertL(printOffset, ch);
                    printOffset++;
                    }
                
                CleanupStack::PopAndDestroy(); // buf16
                }
            
            // or handle in hex format
            else if (iActiveMode == EFileEditorEditAsHex)
                {
                for (TInt i=0; i<bufPtr.Length(); i++)
                    {
                    hexBuf.Format(KHex, bufPtr[i]);
                    hexBuf.UpperCase();
                    hexBuf.Append(_L(" "));                    
                    iTextEditor->Text()->InsertL(printOffset, hexBuf);
                    printOffset += 3;
                    }
                }
            
            fileOffset += bufPtr.Length();
            }
                
        CleanupStack::PopAndDestroy(); // buf

        // make sure that the cursor position is in the beginning 
        iTextEditor->SetCursorPosL(0, EFalse);
        iTextEditor->NotifyNewDocumentL();
        iTextEditor->UpdateScrollBarsL();
        }


    // viewer related file loading
    else if (iActiveMode == EFileEditorViewAsText || iActiveMode == EFileEditorViewAsHex)
        {
        // load the entire file to buffer
        HBufC8* buf = HBufC8::NewLC(fileSize);
        TPtr8 bufPtr(buf->Des());
        
        User::LeaveIfError( fileP.Read(bufPtr, fileSize) );
        
        // send the buffer to viewer
        if (iActiveMode == EFileEditorViewAsText)
            {
            HBufC16* buf16 = ConvertBuf8ToBuf16L(buf);
            CleanupStack::PopAndDestroy(); // buf
            iViewer->FormatAsTextL(buf16);
            }

        else if (iActiveMode == EFileEditorViewAsHex)
            {
            iViewer->FormatAsHexL(buf);
            CleanupStack::Pop(); // buf
            }
        }

    CleanupStack::PopAndDestroy(); // fileP
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::SaveEditorDataL(TBool aQueryNewName)
    {
    TBool doSave(ETrue);
    CPlainText* docText = iTextEditor->Text();
    TInt docLength = docText->DocumentLength();
    
    TFileName fileName;
    fileName.Copy(iFileName);
    
    // check that format of the file is valid
    if (iActiveMode == EFileEditorEditAsHex)
        {
        // check every 3rd char is a space
        for (TInt i=2; i<docLength; i+=3)
            {
            TPtrC docPtr = docText->Read(i, 1);
            if (docPtr.Length() < 1 || docPtr[0] != ' ')
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Separate hex blocks with space!"));
                return;                
                }
            }
        
        // check for valid chars
        for (TInt i=0; i<docLength; i+=3)
            {
            TPtrC docPtr = docText->Read(i, 2);
            if (docPtr.Length() < 2 || !IsHexChar(docPtr[0]) || !IsHexChar(docPtr[1]))
                {
                CAknErrorNote* note = new(ELeave) CAknErrorNote;
                note->ExecuteLD(_L("Use only hex characters!"));
                return;                
                }
            }
        }
    
    
    // query new file name
    if (aQueryNewName)
        {
        CAknTextQueryDialog* textQuery = CAknTextQueryDialog::NewL(fileName);
        textQuery->SetPromptL(_L("Full path:"));
        
        if (textQuery->ExecuteLD(R_GENERAL_TEXT_QUERY))
            {
            doSave = ETrue;
            }
        else
            {
            doSave = EFalse;
            }
        }


    // continue with writing
    if (doSave)        
        {
        RFile fileP;
        TInt err = fileP.Replace(CEikonEnv::Static()->FsSession(), fileName, EFileWrite);
            
        if (err == KErrNone)    
            {
            CleanupClosePushL(fileP);
            
            const TInt KBufSize(1002);
            TInt fileOffset(0);        
            TInt docOffset(0);
            TLex converter;
            TUint hexValue(0);
            
            // write BOM for text files
            if (iActiveMode == EFileEditorEditAsText)
                {
                if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16LE)
                    {
                    TBuf8<2> BOM;
                    BOM.Append(0xFF);
                    BOM.Append(0xFE);
                    
                    if (fileP.Write(0, BOM, 2) == KErrNone)
                        fileOffset += 2;
                    }
                
                else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16BE)
                    {
                    TBuf8<2> BOM;
                    BOM.Append(0xFE);
                    BOM.Append(0xFF);
                    
                    if (fileP.Write(0, BOM, 2) == KErrNone)
                        fileOffset += 2;
                    }
                                  
                else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF8)
                    {
                    TBuf8<3> BOM;
                    BOM.Append(0xEF);
                    BOM.Append(0xBB);
                    BOM.Append(0xBF);
                    
                    if (fileP.Write(0, BOM, 3) == KErrNone)
                        fileOffset += 3;
                    }                
                }
            
            
            HBufC8* buf = HBufC8::NewLC(KBufSize * 2);
            TPtr8 bufPtr(buf->Des());      
          
            while (docOffset < docLength)
                {
                // get text from storage
                TPtrC docPtr = docText->Read(docOffset, KBufSize);
                bufPtr.Copy(KNullDesC);

                if (iActiveMode == EFileEditorEditAsText)
                    {
                    HBufC16* convBuf = HBufC16::NewLC(docPtr.Length());
                    convBuf->Des().Copy(KNullDesC);
                    
                     // convert to line breaks first
                    for (TInt i=0; i<docPtr.Length(); i++)
                        {
                        if (docPtr[i] == CEditableText::EParagraphDelimiter) // skip extra char from the editor
                            continue;    
                        else if (docPtr[i] == CEditableText::ELineBreak)
                            convBuf->Des().Append('\n');
                        else
                            convBuf->Des().Append(docPtr[i]);
                        }
 
                    HBufC8* buf8 = ConvertBuf16ToBuf8L(convBuf);
                    
                    // make sure the sizes are correct
                    if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatANSIASCII || iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF8)
                        {
                        bufPtr.Copy(buf8->Des().Left(convBuf->Length())); 
                        }
                    else if (iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16LE || iTextFormat == EFileBrowserCmdFileEditorSaveFormatUTF16BE)
                        {
                        bufPtr.Copy(buf8->Des().Left(convBuf->Length()*2)); 
                        }
                    
                    delete buf8;
                    buf8 = NULL;

                    CleanupStack::PopAndDestroy(); // convBuf
                    }
                    
                else if (iActiveMode == EFileEditorEditAsHex)
                    {
                    for (TInt i=0; i<docPtr.Length(); i+=3)
                        {
                        if (i+1 >= docPtr.Length() - 1)
                            break;
                        
                        TBuf<3> buf2;
                        buf2.Append(docPtr[i]);
                        buf2.Append(docPtr[i+1]);

                        // convert from hex to decimal
                        converter.Assign(buf2);
                        if (converter.Val(hexValue, EHex) == KErrNone)
                            {
                            bufPtr.Append(hexValue);
                            }
                        }                    
                    }
                
                // write to a file
                err = fileP.Write(fileOffset, bufPtr, bufPtr.Length());
                if (err != KErrNone)
                    break;
                
                docOffset += docPtr.Length();
                fileOffset += bufPtr.Length();
                }
            
           CleanupStack::PopAndDestroy(2); // buf, fileP
            }
        
        if (err == KErrNone)
            {
            // remember the new file name if save was succesfull
            iFileName.Copy(fileName);
            
            CAknConfirmationNote* note = new(ELeave) CAknConfirmationNote;
            note->ExecuteLD(_L("Saved succesfully"));
            } 
        else if (err == KErrInUse)
            {
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD(_L("The file is in use, cannot write"));
            User::Leave(KErrNone);
            }
        else
            {
            CAknErrorNote* note = new(ELeave) CAknErrorNote;
            note->ExecuteLD(_L("Cannot write to the file"));
            User::Leave(KErrNone);
            }
        }
    }
    
// --------------------------------------------------------------------------------------------

TBool CFileBrowserFileEditorDlg::IsHexChar(TInt aCh)
    {
    if (aCh >= '0' && aCh <= '9')
        return ETrue;
    else if (aCh >= 'a' && aCh <= 'f')
        return ETrue;
    else if (aCh >= 'A' && aCh <= 'F')
        return ETrue;
    else
        return EFalse;
    }
      
// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::ProcessCommandL(TInt aCommandId)
    {
    CAknDialog::ProcessCommandL(aCommandId);

    switch (aCommandId)
        {
        case EFileBrowserCmdFileEditorViewAsText:
            {
            iActiveMode = EFileEditorViewAsText;
            LoadFileL();
            }
            break;

        case EFileBrowserCmdFileEditorViewAsHex:
            {
            iActiveMode = EFileEditorViewAsHex;
            LoadFileL();
            }
            break;

//
        case EFileBrowserCmdFileEditorSave:
            {
            SaveEditorDataL();
            }
            break;
            
        case EFileBrowserCmdFileEditorSaveAs:
            {
            SaveEditorDataL(ETrue);
            }
            break;

        case EFileBrowserCmdFileEditorSaveFormatANSIASCII:
        case EFileBrowserCmdFileEditorSaveFormatUTF8:
        case EFileBrowserCmdFileEditorSaveFormatUTF16LE:
        case EFileBrowserCmdFileEditorSaveFormatUTF16BE:
            {
            iTextFormat = aCommandId;
            }
            break;
            
        case EFileBrowserCmdFileEditorEditAsText:
            {
            iActiveMode = EFileEditorEditAsText;
            LoadFileL();
            }
            break;

        case EFileBrowserCmdFileEditorEditAsHex:
            {
            iActiveMode = EFileEditorEditAsHex;
            LoadFileL();
            }
            break;

//                        
        case EFileBrowserCmdFileEditorExit:
            TryExitL(EAknCmdExit);
            break;

        default:
            break;
        }
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    if (aResourceId == R_FILEBROWSER_FILEVIEWER_MENUPANE)
        {
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileEditorViewAsText, iActiveMode == EFileEditorViewAsText);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileEditorViewAsHex, iActiveMode == EFileEditorViewAsHex);
        }
    else if (aResourceId == R_FILEBROWSER_FILEEDITOR_MENUPANE)
        {
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileEditorEditAsText, iActiveMode == EFileEditorEditAsText);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileEditorEditAsHex, iActiveMode == EFileEditorEditAsHex);
        aMenuPane->SetItemDimmed(EFileBrowserCmdFileEditorSaveFormat, iActiveMode == EFileEditorEditAsHex);
        }
	else if (aResourceId == R_FILEBROWSER_FILEEDITOR_SAVEFORMAT_SUBMENU)
	    {
        aMenuPane->SetItemButtonState(iTextFormat, EEikMenuItemSymbolOn);
	    }
    }

// --------------------------------------------------------------------------------------------

SEikControlInfo CFileBrowserFileEditorDlg::CreateCustomControlL(TInt aControlType)
    {
    SEikControlInfo controlInfo;
    controlInfo.iControl = NULL;
    controlInfo.iTrailerTextId = 0;
    controlInfo.iFlags = 0;

    if (aControlType == EFileViewerDialogControl)
        {
        // create a control
        iViewer = new(ELeave) CFileBrowserFileEditorViewControl();
        iViewer->SetMopParent(this);
        iViewer->ConstructL();

        controlInfo.iControl = iViewer;
        }
        
    return controlInfo;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorDlg::SetSizeAndPosition(const TSize& /*aSize*/)
    {
    // fill whole window
    TRect mainPaneRect;
    TRect naviPaneRect;

    if (Layout_Meta_Data::IsLandscapeOrientation() ||
        iEikonEnv->AppUiFactory()->StatusPane()->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_SMALL)
        {
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        }
    else
        {
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ENaviPane, naviPaneRect);
        mainPaneRect.iTl.iY = naviPaneRect.Height();   
        }
            
    SetRect(mainPaneRect);
    }

// --------------------------------------------------------------------------------------------

TBool CFileBrowserFileEditorDlg::RunDlgLD()
    {
    if (iActiveMode == EFileEditorViewAsText || iActiveMode == EFileEditorViewAsHex)
        return ExecuteLD(R_FILEBROWSER_FILEVIEWER_DIALOG);
    else
        return ExecuteLD(R_FILEBROWSER_FILEEDITOR_DIALOG);
    }
       
// --------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------

CFileBrowserFileEditorViewControl::~CFileBrowserFileEditorViewControl()
    {
    ResetVariables();
    }

// --------------------------------------------------------------------------------------------

CFileBrowserFileEditorViewControl::CFileBrowserFileEditorViewControl()
    {
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::ConstructL()
    {
    CreateWindowL();
    SetBlank();
    SetRect(Rect());
    ActivateL();
    
    iTotalNumberOfLines = 0;
    }

// --------------------------------------------------------------------------------------------

TSize CFileBrowserFileEditorViewControl::MinimumSize()
    {
    TRect mainPaneRect;
    TRect naviPaneRect;
    
    if (Layout_Meta_Data::IsLandscapeOrientation() ||
        iEikonEnv->AppUiFactory()->StatusPane()->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_SMALL)
        {
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        }
    else
        {
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ENaviPane, naviPaneRect);
        mainPaneRect.iTl.iY = naviPaneRect.Height();   
        }

    return mainPaneRect.Size();
    }
        
// --------------------------------------------------------------------------------------------

TKeyResponse CFileBrowserFileEditorViewControl::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
    {
    if(aType != EEventKey)
        return EKeyWasNotConsumed;
    
    // normally scroll only one line
    TInt scrollLines(1); 
    
    // with special keys scroll iNumberOfLinesFitsScreen-1 lines instead
    if (aKeyEvent.iCode == EKeyIncVolume || aKeyEvent.iCode=='#' || aKeyEvent.iCode == EKeyDecVolume || aKeyEvent.iCode=='*')
        scrollLines = iNumberOfLinesFitsScreen-1;
    
    
    if (aKeyEvent.iCode == EKeyUpArrow || aKeyEvent.iCode == EKeyIncVolume || aKeyEvent.iCode=='#')
        {
        if (iCurrentLine > 0)
            {
            iCurrentLine -= scrollLines;
            
            if (iCurrentLine < 0)
                iCurrentLine = 0;
            
            UpdateScrollBarL();
            DrawNow();
            }

        return EKeyWasConsumed;
        }
    
    else if (aKeyEvent.iCode == EKeyDownArrow || aKeyEvent.iCode == EKeyDecVolume || aKeyEvent.iCode=='*')
        {
        if (iActiveMode == EFileEditorViewAsText)
            {
            // no need to scroll text if it fits to the screen
            if (iTotalNumberOfLines <= iNumberOfLinesFitsScreen)
                return EKeyWasConsumed;

            iCurrentLine += scrollLines;

            // adjust the last line to end of the screen            
            if (iCurrentLine > iTotalNumberOfLines - iNumberOfLinesFitsScreen)
                iCurrentLine = iTotalNumberOfLines - iNumberOfLinesFitsScreen;            
            }
        else if (iActiveMode == EFileEditorViewAsHex)
            {
            iCurrentLine += scrollLines;

            // last line can be first line on the screen            
            if (iCurrentLine > iTotalNumberOfLines - 1)
                iCurrentLine = iTotalNumberOfLines - 1;              
            }
            
        UpdateScrollBarL();
        DrawNow();
        
        return EKeyWasConsumed;
        }
    
    return EKeyWasNotConsumed;
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::FormatAsTextL(HBufC16* aBuf)
    {
    if (aBuf == NULL)    
        ResetVariables(EFalse);
    else
        ResetVariables();
    
    InitVariables();
    iActiveMode = EFileEditorViewAsText;
    
    // store the bugger
    if (aBuf != NULL)
        iTextBuf = aBuf;
    
    iWrappedTextArray = new(ELeave) CArrayFixFlat<TPtrC>(32);
    
    // wrap to array
    AknTextUtils::WrapToArrayL(iTextBuf->Des(), iDrawingWidth, *iFont, *iWrappedTextArray);
    iTotalNumberOfLines = iWrappedTextArray->Count();

    // count amount of lines fits to screen
    iNumberOfLinesFitsScreen = TInt(iViewerRect.Height() / (iFont->HeightInPixels()));

    // update scroll bars
    UpdateScrollBarL();

    // update the screen
    DrawNow();    
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::FormatAsHexL(HBufC8* aBuf)
    {
    if (aBuf == NULL)    
        ResetVariables(EFalse);
    else
        ResetVariables();
    
    InitVariables();
    iActiveMode = EFileEditorViewAsHex;
    
    // store the buffer
    if (aBuf != NULL)
        iHexesBuffer = aBuf;
    
    // calculate amount of hex values fits to the screen
    TUint charWidth = iFont->CharWidthInPixels('X');
    iHexGrougWidth = charWidth*2 + TInt (KHexSeparatorMargin*iX_factor);
    iAmountOfHexesFitsHorizontally = TUint( iDrawingWidth / iHexGrougWidth );

    // calculate total number of files
    iTotalNumberOfLines = iHexesBuffer->Length() / iAmountOfHexesFitsHorizontally + 1;

    // count amount of lines fits to screen
    iNumberOfLinesFitsScreen = TInt(iViewerRect.Height() / iFont->HeightInPixels()) - 1;

    // update scroll bars
    UpdateScrollBarL();

    // update the screen
    DrawNow();      
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::InitVariables()
    {
    iCurrentLine = 0;

    // calculate layout data
    TRect mainPaneRect;
    TRect naviPaneRect;
    
    if (Layout_Meta_Data::IsLandscapeOrientation() ||
        iEikonEnv->AppUiFactory()->StatusPane()->CurrentLayoutResId() != R_AVKON_STATUS_PANE_LAYOUT_SMALL)
        {
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        }
    else
        {
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::ENaviPane, naviPaneRect);
        mainPaneRect.iTl.iY = naviPaneRect.Height();   
        }

    iViewerRect = TRect(mainPaneRect.Size());

    iX_factor = TReal(iViewerRect.Width()) / 176;
    iY_factor = TReal(iViewerRect.Height()) / 144;


    // define drawing width, some extra space needed for double span scrolling bar
    if (AknLayoutUtils::LayoutMirrored())  // scroll bar on 'left'
        {
        iLeftDrawingPosition = KLeftMargin + 12;
        iDrawingWidth = TUint( iViewerRect.Width() - iLeftDrawingPosition*iX_factor - (KRightMargin*iX_factor)); 
        }
    else // scroll bar on 'right'
        {
        iLeftDrawingPosition = KLeftMargin;  
        iDrawingWidth = TUint( iViewerRect.Width() - iLeftDrawingPosition*iX_factor - (KRightMargin*iX_factor + 7*iX_factor)); 
        } 


    // set font
    CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
    const TSize screenSize = dev->SizeInPixels();
    
    TFontSpec fontSpec = AknLayoutUtils::FontFromId(EAknLogicalFontSecondaryFont)->FontSpecInTwips();
    fontSpec.iHeight = IsQHD(screenSize) ? KViewerFontHeightQHD : KViewerFontHeight; // set height
    dev->GetNearestFontInTwips(iFont, fontSpec);
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::ResetVariables(TBool aResetData)
    {
    if (iScrollBarFrame)
        {
        delete iScrollBarFrame;
        iScrollBarFrame = NULL;
        }
        
    if (iHexesBuffer && aResetData)
        {
        delete iHexesBuffer;
        iHexesBuffer = NULL;
        }    

    if (iWrappedTextArray)
        {
        delete iWrappedTextArray;
        iWrappedTextArray = NULL;
        }

    if (iTextBuf && aResetData)
        {
        delete iTextBuf;
        iTextBuf = NULL;
        }

    if (iFont)
        {
        CGraphicsDevice* dev = iCoeEnv->ScreenDevice();
        dev->ReleaseFont(iFont);
        iFont = NULL;
        }
    }
    
// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::UpdateScrollBarL()
    {
    TRect rect(iViewerRect);
    
    // init the scroll bar    
    if (!iScrollBarFrame)
        {
        iScrollBarFrame = new(ELeave) CEikScrollBarFrame(this, this, ETrue);
        iScrollBarFrame->CreateDoubleSpanScrollBarsL(ETrue, EFalse);            
        iScrollBarFrame->SetTypeOfVScrollBar(CEikScrollBarFrame::EDoubleSpan);
        iScrollBarFrame->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto);
        }       


    // update values
	TEikScrollBarModel horizontalBar;
    TEikScrollBarModel verticalBar;

    if (iActiveMode == EFileEditorViewAsText)
        {    
        verticalBar.iThumbPosition = iCurrentLine;
        verticalBar.iScrollSpan = iTotalNumberOfLines - iNumberOfLinesFitsScreen + 1;
        verticalBar.iThumbSpan = 1;
        }
    
    else if (iActiveMode == EFileEditorViewAsHex)
        {    
        verticalBar.iThumbPosition = iCurrentLine;
        verticalBar.iScrollSpan = iTotalNumberOfLines;
        verticalBar.iThumbSpan = 1;
        }
        
    TEikScrollBarFrameLayout layout;
	layout.iTilingMode = TEikScrollBarFrameLayout::EInclusiveRectConstant;

    // do not let scrollbar values overflow
    if (verticalBar.iThumbPosition + verticalBar.iThumbSpan > verticalBar.iScrollSpan)
        verticalBar.iThumbPosition = verticalBar.iScrollSpan - verticalBar.iThumbSpan;
	
    TAknDoubleSpanScrollBarModel horizontalDSBar(horizontalBar);
    TAknDoubleSpanScrollBarModel verticalDSBar(verticalBar);
   
    iScrollBarFrame->TileL(&horizontalDSBar, &verticalDSBar, rect, rect, layout);        
    iScrollBarFrame->SetVFocusPosToThumbPos(verticalDSBar.FocusPosition());
    }

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();
    gc.Clear(aRect);
    gc.SetPenColor(KRgbBlack);
    gc.UseFont(iFont);
    
    TInt fontHeight = iFont->HeightInPixels();

    if (iActiveMode == EFileEditorViewAsText)
        {
        // draw the text
        for (TInt i=0; i<iNumberOfLinesFitsScreen; i++)
            {
            // check for bounds and draw
            if (iCurrentLine >= 0 && iTotalNumberOfLines > i+iCurrentLine)
                gc.DrawText(iWrappedTextArray->At(i+iCurrentLine), TPoint(TInt(iLeftDrawingPosition*iX_factor), TInt(fontHeight*(i+1))));
            }        
        }

    else if (iActiveMode == EFileEditorViewAsHex)
        {
        TInt hexesOffSet = iCurrentLine * iAmountOfHexesFitsHorizontally; 
        TInt topLineHeight = 2*iY_factor + fontHeight;
        
        // draw the top line
        gc.SetPenColor(KRgbWhite);
        
        gc.SetBrushColor(KRgbBlue);
        gc.SetBrushStyle(CGraphicsContext::ESolidBrush);
        if (AknLayoutUtils::LayoutMirrored())  // scroll bar on 'left'
            {
            gc.DrawRect(TRect((KLeftMargin + 12)*iX_factor, 0, aRect.iBr.iX, topLineHeight));
            }
        else // scroll bar on 'right'
            {
            gc.DrawRect(TRect(0, 0, aRect.iBr.iX-(KRightMargin*iX_factor + 7*iX_factor), topLineHeight));
            } 

        TBuf<128> topLineBuf;
        topLineBuf.AppendNum(hexesOffSet);
        topLineBuf.Append(_L(": "));
        
        for (TInt i=0; i<iAmountOfHexesFitsHorizontally; i++)
            {
            TInt pos = hexesOffSet + i;
            
            if (pos < iHexesBuffer->Des().Length())
                {
                topLineBuf.Append(iHexesBuffer->Des()[pos]);
                }
            }
 
        gc.DrawText(topLineBuf, TPoint(TInt(iLeftDrawingPosition*iX_factor), TInt(fontHeight)));

 
        // draw hex values
        gc.SetPenColor(KRgbBlack);
        for (TInt i=0; i<iNumberOfLinesFitsScreen; i++)
            {
            for (TInt j=0; j<iAmountOfHexesFitsHorizontally; j++)
                {
                TInt pos = hexesOffSet + i*iAmountOfHexesFitsHorizontally + j;
                
                if (pos >= iHexesBuffer->Des().Length())
                    return;
                
                TUint8* ptr = &iHexesBuffer->Des()[pos];
                
                _LIT(KHex, "%02x");
                TBuf<5> hexBuf;
                hexBuf.Format(KHex, ptr[0]);
                hexBuf.UpperCase();

                gc.DrawText(hexBuf, TPoint(TInt(iLeftDrawingPosition*iX_factor + j*iHexGrougWidth), TInt(topLineHeight + fontHeight*(i+1))));
                }
            }         
        }

    gc.DiscardFont();
    }
  
// --------------------------------------------------------------------------------------------

CCoeControl* CFileBrowserFileEditorViewControl::ComponentControl(TInt aIndex) const
	{
	if (aIndex == 0 && iScrollBarFrame)
	    return iScrollBarFrame->VerticalScrollBar();
	else
	    return NULL;
	}

// --------------------------------------------------------------------------------------------

TInt CFileBrowserFileEditorViewControl::CountComponentControls() const
	{
	if (iScrollBarFrame)
	    return 1;
	else
	    return 0;
	}

// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::HandleScrollEventL(CEikScrollBar* aScrollBar, TEikScrollEvent aEventType)
    {
    if ((aEventType == EEikScrollPageDown) || (aEventType == EEikScrollPageUp) || 
       (aEventType == EEikScrollThumbDragVert) || (aEventType == EEikScrollUp) ||
       (aEventType == EEikScrollDown))
        {
        iCurrentLine = aScrollBar->ThumbPosition();
        UpdateScrollBarL();
        DrawNow();
        }
    }

// --------------------------------------------------------------------------------------------
 
TTypeUid::Ptr CFileBrowserFileEditorViewControl::MopSupplyObject(TTypeUid aId)
    {
    return CCoeControl::MopSupplyObject(aId);
    }
	   
// --------------------------------------------------------------------------------------------

void CFileBrowserFileEditorViewControl::HandleResourceChange(TInt aType)
    {
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        SetRect(Rect());
        ActivateL();
        
        // reformat the text
        if (iActiveMode == EFileEditorViewAsText)
            {
            FormatAsTextL();
            }
        else if (iActiveMode == EFileEditorViewAsHex)
            {
            FormatAsHexL();
            }
        }
    else
        {
        CCoeControl::HandleResourceChange(aType);
        }
    }

// --------------------------------------------------------------------------------------------

// End of File

