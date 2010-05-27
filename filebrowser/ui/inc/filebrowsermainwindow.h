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

#ifndef FILEBROWSERMAINWINDOW_H_
#define FILEBROWSERMAINWINDOW_H_

#include <hbmainwindow.h>

class HbApplication;
class EngineWrapper;
class FileBrowserView;
class SettingsView;
class EditorView;
class SearchView;

class FileBrowserMainWindow : public HbMainWindow
    {
    Q_OBJECT

public:
    explicit FileBrowserMainWindow(QWidget *parent = 0);
    virtual ~FileBrowserMainWindow();
    
    void init();

private slots:
    void openFileBrowserView();
    void openSettingsView();
    void openEditorView(const QString &, bool);
    void openSearchView(const QString &);
    
private:
    EngineWrapper* mEngineWrapper;
    FileBrowserView* mFileBrowserView;
    SettingsView* mSettingsView;
    EditorView* mEditorView;
    SearchView* mSearchView;
    };

#endif /* FILEBROWSERMAINWINDOW_H_ */
