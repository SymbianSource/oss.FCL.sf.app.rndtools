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

#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <hbview.h>

class HbDataForm;
class HbDataFormModelItem;
class FileBrowserMainWindow;
class EngineWrapper;
class FileBrowserSettings;

class SettingsView : public HbView
{
    Q_OBJECT

public:
    SettingsView(FileBrowserMainWindow &mainWindow, EngineWrapper &engineWrapper);
    virtual ~SettingsView();

signals:
    void finished(bool ok);

public slots:
//    void displayModeChanged();
//    void fileViewChanged();
//    void subDirectoryInfoChanged();
//    void associatedIconsChanged();
//    void rememberFolderOnExitChanged();
//    void rememberLastFolderChanged();
//    void showToolbarChanged();

//    void toggleChange(QModelIndex, QModelIndex);
    void accept();
    void reject();

private:
    void initDataForm();
    void createToolbar();
    void constructMenu();
    void loadSettings(const FileBrowserSettings &settings);
    void saveSettings(FileBrowserSettings &settings);

private:
    FileBrowserMainWindow &mMainWindow;
    EngineWrapper &mEngineWrapper;

    HbDataForm *mForm;
    HbDataFormModelItem *mDisplayModeItem;
    HbDataFormModelItem *mFileViewItem;
    HbDataFormModelItem *mShowDirectoryInfoItem;
    HbDataFormModelItem *mShowAssociatedIconsItem;
    HbDataFormModelItem *mRememberFolderOnExitItem;
    HbDataFormModelItem *mRememberLastFolderItem;
    HbDataFormModelItem *mShowToolbarItem;

    HbDataFormModelItem *mSupportNetDrivesItem; //"Support net drives"
    HbDataFormModelItem *mBypassPlatformSecurityItem; //"Bypass plat.security"
    HbDataFormModelItem *mUnlockFilesViaSBItem; //"Unlock files via SB"
    HbDataFormModelItem *mIgnoreProtectionAttributesItem; //"Ign. protection atts"
    HbDataFormModelItem *mNoROAttributeCopyFromZItem; //"No RO-att copy from Z:"

};

#endif // SETTINGSVIEW_H
