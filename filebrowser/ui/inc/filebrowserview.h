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

#ifndef FILEBROWSERVIEWH_H_
#define FILEBROWSERVIEWH_H_

#include "menuaction.h"
#include "enginewrapper.h"

#include <HbView>
#include <HbMainWindow>
#include <HbApplication>

#include <QModelIndexList>

// Forward declarations
class QFileInfo;
class QSignalMapper;
class QItemSelection;
class QString;
class QGraphicsLinearLayout;

class HbListView;
class HbListWidget;
class HbToolBar;
class HbLabel;
class HbDialog;
class HbAbstractViewItem;
class HbMenu;

class FileBrowserMainWindow;
class EditorView;
class SearchView;
class SettingsView;
class EngineWrapper;
class FileBrowserModel;

class FileBrowserView : public HbView
{
    Q_OBJECT

public:
    explicit FileBrowserView(FileBrowserMainWindow &mainWindow);
    virtual ~FileBrowserView();
    void init(EngineWrapper *engineWrapper);

public slots:
    void refreshList();

private:
    void fileOverwriteDialog();
    void openListDialog(const QStringList &items, const QString &titleText, QObject *receiver, const char *member);

    void openPropertyDialog(const QStringList &propertyList, const QString &title);

    QModelIndex currentItemIndex();
    void storeSelectedItemsOrCurrentItem();

    // Menu related methods
    void createMenu();
    void createFileMenu();
    void createEditMenu();
    void createViewMenu();
    void createToolsMenu();

    void createSelectionMenuItem();
    void createSettingsMenuItem();
    void createAboutMenuItem();
    void createExitMenuItem();

    void createContextMenu();
    void createFileContextMenu();
    void createEditContextMenu();
    void createViewContextMenu();
    void createDiskAdminContextMenu();
    void createToolBar();

//    void refreshList();
    void populateFolderContent();

private slots:
    // menu action slots
    // file menu
    void fileBackMoveUp();
    void fileOpenDrive();
    void fileOpenDirectory();
    void fileSearch();

    void fileNewFile();
    void doFileNewFile(HbAction *);

    void fileNewDirectory();
    void doFileNewDirectory(HbAction *);

    void fileDelete();
    void doFileDelete(HbAction *);

    void fileRename();
    void doFileRename(HbAction *);
    void doFileRenameFileExist(HbAction *);

    void fileTouch();
    void doFileTouch(HbAction *);
    void fileProperties();

    void fileChecksumsMD5();
    void fileChecksumsMD2();
    void fileChecksumsSHA1();
    void fileChecksums(TFileBrowserCmdFileChecksums checksumType);

    void fileSetAttributes();

    // edit menu
    void editSnapShotToE();
    void editCut();
    void editCopy();
    void editPaste();

    void editCopyToFolder();
    void doEditCopyToFolder(HbAction *);

    void editMoveToFolder();
    void doEditMoveToFolder(HbAction *);

    void editSelect();
    void editUnselect();
    void editSelectAll();
    void editUnselectAll();

    // view menu
    void viewFilterEntries();
    void viewRefresh();

    // disk admin menu
    void diskAdminSetDrivePassword();
    void diskAdminSetDrivePasswordNew(HbAction *);
    void doDiskAdminSetDrivePassword(HbAction *);

    void diskAdminUnlockDrive();
    void doDiskAdminUnlockDrive(HbAction *);

    void diskAdminClearDrivePassword();
    void doDiskAdminClearDrivePassword(HbAction *);

    void diskAdminEraseDrivePassword();
    void doDiskAdminEraseDrivePassword(HbAction *);

    void diskAdminFormatDrive();
    void doDiskAdminFormatDrive(HbAction *);

    void diskAdminQuickFormatDrive();
    void doDiskAdminQuickFormatDrive(HbAction *);

    void diskAdminCheckDisk();

    void diskAdminScanDrive();
    void doDiskAdminScanDrive(HbAction *);

    void diskAdminSetDriveName();
    void doDiskAdminSetDriveName(HbAction *);

    void diskAdminSetDriveVolumeLabel();
    void doDiskAdminSetDriveVolumeLabel(HbAction *);

    void diskAdminEjectDrive();
    void diskAdminDismountDrive();
    void doDiskAdminDismountDrive(HbAction *);

    void diskAdminEraseMBR();
    void doDiskAdminEraseMBR(HbAction *);
    void doDiskAdminReallyEraseMBR(HbAction *);
    void doDiskAdminNotRemovableReallyEraseMBR(HbAction *);
    
    void diskAdminPartitionDrive();
    void diskAdminPartitionDriveProceed(HbAction *);
    void diskAdminPartitionDriveReallyProceed(HbAction *);
    void diskAdminPartitionDriveIsNotRemovable(HbAction *);
    void diskAdminPartitionDriveEraseMbr(HbAction *);
    void diskAdminPartitionDriveGetCount(HbAction*);

    // tools menu
    void toolsAllAppsToTextFile();
    void toolsAllFilesToTextFile();
    void toolsAvkonIconCacheEnable();
    void toolsAvkonIconCacheDisable();

    void toolsDisableExtendedErrors();
    void toolsDumpMsgStoreWalk();
    void toolsEditDataTypes();
    void toolsEnableExtendedErrors();

    void toolsErrorSimulateLeave();
    void doToolsErrorSimulateLeave(HbAction *);

    void toolsErrorSimulatePanic();
    void doToolsErrorSimulatePanicCode(HbAction *);
    void doToolsErrorSimulatePanic(HbAction *);

    void toolsErrorSimulateException();
    void doToolsErrorSimulateException(HbAction *);

//    void toolsLocalConnectivityActivateInfrared();
//    void toolsLocalConnectivityLaunchBTUI();
//    void toolsLocalConnectivityLaunchUSBUI();
    void toolsMessageInbox();
    void toolsMessageDrafts();
    void toolsMessageSentItems();
    void toolsMessageOutbox();
    void toolsMemoryInfo();
    void toolsSecureBackStart();
    void toolsSecureBackRestore();
    void toolsSecureBackStop();
    void toolsSetDebugMaskQuestion();
    void toolsSetDebugMask(HbAction *);
    void toolsShowOpenFilesHere();

    // main menu items
    void selectionModeChanged();
    void about();
    
signals:
    void aboutToShowSettingsView();
    void aboutToShowEditorView(const QString &, bool);
    void aboutToShowSearchView(const QString &);
    void aboutToSimulateLeave(int);

private slots:
    void itemHighlighted(const QModelIndex &index);
    //void itemSelected(const QModelIndex &index);
    void updateOptionMenu();
    void updateContextMenu();
    void selectionChanged(const QItemSelection &, const QItemSelection &);
    void activated(const QModelIndex& index);
    void activateSelectionMode();
    void deActivateSelectionMode();
    void onLongPressed(HbAbstractViewItem *, QPointF);

    void fileOpen(HbAction *);
    void fileOverwrite(HbAction *);
    void fileOverwritePostfix(HbAction *);

private:
    FileBrowserMainWindow &mMainWindow;
    EngineWrapper *mEngineWrapper;

    HbListView *mListView;
    HbToolBar *mToolBar;
    HbLabel *mNaviPane;
    QGraphicsLinearLayout *mMainLayout;

    // file info contains all needed information of selected file from file model
    QModelIndexList mClipboardIndexes;
    QModelIndexList mSelectionIndexes;

    FileBrowserModel *mFileBrowserModel;
    OptionMenuActions mOptionMenuActions;
    ContextMenuActions mContextMenuActions;
    HbMenu *mContextMenu;
    HbAction *mToolbarBackAction;

    // flags
    bool mItemHighlighted;
    bool mLocationChanged;
    // flag for removing source file after copied to target file
    bool mRemoveFileAfterCopied;
    bool mClipBoardInUse;
    bool mFolderContentChanged;
    QModelIndex mCurrentIndex;

    // temporarily storage
    QString mOldPassword;
    QString mPanicCategory;
    QString mAbsoluteFilePath;
    OverwriteOptions mOverwriteOptions;
    QModelIndex mModelIndex;
    QString mNewFileName;
    bool mProceed;
    bool mEraseMBR;
};



#endif /* FILEBROWSERVIEWH_H_ */
