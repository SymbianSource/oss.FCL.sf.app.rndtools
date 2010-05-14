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

#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <hbview.h>
#include <hbmainwindow.h>
//#include <QtGui>

class HbTextEdit;
class HbAction;

class EditorView : public HbView
{
    Q_OBJECT

public:
    EditorView(HbMainWindow &mainWindow);
    ~EditorView();
    void open(const QString& fileName, bool flagReadOnly);

signals:
    void finished(bool ok);

private slots:
    void backButtonClicked();
    void displayInText();
    void displayInHex();
    void updateMenu();

    /*
    void newFile();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();
    */
 

private:
    void loadFile(const QString &fileName);
    void createMenu();

    /* Main Window of folderbrowser */
    HbMainWindow &mMainWindow;
    // editor
    HbTextEdit *mTextEdit;
    // if flag true, file is currently in hex format
    bool mFileHex;
    bool mFlagReadOnly;
    QString mFileName;
    QString mFileContent;

    HbAction* mToTextAction;
    HbAction* mToHexAction;
    HbAction* mExitAction;
	
};

#endif // EDITORVIEW_H
