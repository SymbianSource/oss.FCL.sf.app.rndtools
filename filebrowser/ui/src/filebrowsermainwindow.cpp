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


#include <HbApplication>
#include <HbMainWindow>

#include "filebrowsermainwindow.h"
#include "enginewrapper.h"
#include "filebrowserview.h"
#include "settingsview.h"
#include "editorview.h"

FileBrowserMainWindow::FileBrowserMainWindow(QWidget *parent) :
        HbMainWindow( parent )
        ,mEngineWrapper(0)
        ,mFileBrowserView(0)
        ,mSettingsView(0)
        ,mEditorView(0)
{
}

FileBrowserMainWindow::~FileBrowserMainWindow ()
{
    if (mEngineWrapper) {
        delete mEngineWrapper;
    }
}

void FileBrowserMainWindow::init()
{
    // Create Engine Wrapper and initialize it
    mEngineWrapper = new EngineWrapper();
    int error = mEngineWrapper->init();
    Q_ASSERT_X(error == 1, "FileBrowser", "Engine initialization failed");

    // Create file browser view
    mFileBrowserView = new FileBrowserView(*this);
    connect(mFileBrowserView, SIGNAL(aboutToShowSettingsView()), this, SLOT(openSettingsView()));
    mFileBrowserView->init(mEngineWrapper);
    addView(mFileBrowserView);

    // Create settings view
    mSettingsView = new SettingsView(*this, *mEngineWrapper);
    connect(mSettingsView, SIGNAL(finished(bool)), this, SLOT(openFileBrowserView()));
    addView(mSettingsView);

    // Create settings view
    mEditorView = new EditorView(*this);
    connect(mFileBrowserView, SIGNAL(aboutToShowEditorView(const QString &, bool)), this, SLOT(openEditorView(const QString &, bool)));
    connect(mEditorView, SIGNAL(finished(bool)), this, SLOT(openFileBrowserView()));
    addView(mEditorView);

    // Show ApplicationView at startup
    setCurrentView(mFileBrowserView);
    // Show HbMainWindow
    show();
}

void FileBrowserMainWindow::openFileBrowserView()
{
    setCurrentView(mFileBrowserView);
}

void FileBrowserMainWindow::openSettingsView()
{
    setCurrentView(mSettingsView);
}

void FileBrowserMainWindow::openEditorView(const QString &fileName, bool flagReadOnly)
{
    mEditorView->open(fileName, flagReadOnly);
    setCurrentView(mEditorView);
}
