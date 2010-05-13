/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef NOTIFICATIONS_H_
#define NOTIFICATIONS_H_

#include <hbmessagebox.h>
#include <hbselectiondialog.h>

class HbProgressDialog;
class HbPopup;

class Notifications : public HbPopup
    {

	Q_OBJECT 	
	
public:
    
    /**
     * shows about box
     */
    static void about();
    
    /** 
     * shows error message 
     */
    static void error(const QString& errorMessage);
	
	/**
     * to shows progressbar
     */
	static HbProgressDialog* showProgressBar(const QString& text, int max);
	
	/**
	* Entries query dialog
	*/
	static bool entriesQueryDialog(int& numberOfEntries, const QString& text, bool acceptsZero);
    
	/**
	* Time query dialog
	*/
    static bool timeQueryDialog(QDate& date, const QString& text);

	/**
	* Yes or No query dialog
	*/    
    static bool yesNoQueryDialog(const QString& text);
	
	/**
	* popup list dialog for selecting one item from list
	*/
	static void popupListDialog(const QString& text, QStringList& items, HbAbstractItemView::SelectionMode mode, QObject* receiver = 0, const char* member = 0);
	
	/**
	* directory query dialog
	*/
	static bool directoryQueryDialog(const QString& text, QString& directory);

    /**
     * shows global HbGlobalCommonNote type note
     */
	static void showGlobalNote(const QString& text, HbMessageBox::MessageBoxType type, HbPopup::DefaultTimeout timeout = HbPopup::ConfirmationNoteTimeout);
    };



#endif // NOTIFICATIONS_H_
