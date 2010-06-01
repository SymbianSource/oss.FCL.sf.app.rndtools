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

#include "filebrowserview.h"
#include "filebrowsermainwindow.h"
#include "settingsview.h"
#include "editorview.h"
#include "searchview.h"
#include "enginewrapper.h"
#include "notifications.h"

#include "filebrowsermodel.h"

#include <HbMainWindow>
#include <HbMenu>
#include <HbPopup>
#include <HbView>
#include <HbMessageBox>
#include <HbAction>
#include <HbLabel>
#include <HbListView>
#include <HbListViewItem>
#include <HbListWidget>
#include <HbLineEdit>
#include <HbAbstractViewItem>
#include <HbSelectionDialog>
#include <HbValidator>
#include <HbInputDialog>
#include <HbToolBar>

#include <QString>
#include <QGraphicsLinearLayout>
#include <QItemSelection>
#include <QDebug>
//TODO check if needed to do this way
#include <FB.hrh>

//const int DRIVEPATHLENGTH = 4;
const QString okActionText("OK");
const QString cancelActionText("Cancel");

// ---------------------------------------------------------------------------

FileBrowserView::FileBrowserView(FileBrowserMainWindow &mainWindow)
    : mMainWindow(mainWindow),
    mEngineWrapper(0),
    mListView(0),
    mToolBar(0),
    mNaviPane(0),
    mMainLayout(0),
    mFileBrowserModel(0),
    mOptionMenuActions(),
    mContextMenuActions(),
    mContextMenu(0),
    mToolbarBackAction(0),
    mItemHighlighted(false),
    mLocationChanged(false),
    mRemoveFileAfterCopied(false),
//    mClipBoardInUse(false),
    mFolderContentChanged(false),
    mCurrentIndex(),
    mOldPassword(),
    mPanicCategory(),
    mAbsoluteFilePath(),
    mOverwriteOptions(),
    mModelIndex(),
    mNewFileName(),
    mProceed(false),
    mEraseMBR(false)
{
    setTitle("File Browser");

    createMenu();
    createContextMenu();
    createToolBar();
}

// ---------------------------------------------------------------------------	

void FileBrowserView::init(EngineWrapper *engineWrapper)
{
    mEngineWrapper = engineWrapper;

    mListView = new HbListView(this);
    mFileBrowserModel = new FileBrowserModel(mEngineWrapper);
    if (!mListView->model()) {
        mListView->setModel(mFileBrowserModel);
        mListView->listItemPrototype()->setStretchingStyle(HbListViewItem::StretchLandscape);
        mEngineWrapper->refreshView();
        mToolbarBackAction->setEnabled(!mEngineWrapper->isDriveListViewActive());
    }

    //mListView->setRootIndex(mFileSystemModel->index(startPath));
    //mListView->setRootIndex(model->index());

    mListView->setScrollingStyle(HbScrollArea::PanWithFollowOn);

    connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
    connect(mListView, SIGNAL(longPressed(HbAbstractViewItem*,QPointF)),
            this, SLOT(onLongPressed(HbAbstractViewItem*, QPointF)));

    mNaviPane = new HbLabel(this);
    mNaviPane->setPlainText(QString(" ")); // TODO get from settings or default
    //mNaviPane->setPlainText(QString(mEngineWrapper->currentPath()));
    HbFontSpec fontSpec(HbFontSpec::PrimarySmall);
    mNaviPane->setFontSpec(fontSpec);

    // Create layout and add list view and toolbar into layout:
    mMainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mMainLayout->addItem(mNaviPane);
    mMainLayout->addItem(mListView);
    //mMainLayout->addItem(mToolBar);
    setLayout(mMainLayout);
}

// ---------------------------------------------------------------------------

FileBrowserView::~FileBrowserView()
{  
//    if (mEngineWrapper) {
//        delete mEngineWrapper;
//    }
    if (mContextMenu) {
        mContextMenu->deleteLater();
    }

    delete mFileBrowserModel;
    delete mListView;
    delete mToolBar;
}

/**
  Initial setup for options menu.
  Dynamic menu update during the runtime is performed by updateOptionMenu() which
  to menu's aboutToShow() signal.
  */
void FileBrowserView::createMenu()
{
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createToolsMenu();

    createSelectionMenuItem();
    createSettingsMenuItem();
    createAboutMenuItem();
    createExitMenuItem();

    // menu dynamic update
    connect(menu(), SIGNAL(aboutToShow()), this, SLOT(updateOptionMenu()));
}

/**
  Initial setup for File submenu
  */
void FileBrowserView::createFileMenu()
{
    mOptionMenuActions.mFileMenu = menu()->addMenu("File");

    mOptionMenuActions.mFileBackMoveUp = mOptionMenuActions.mFileMenu->addAction("Back/Move up (<-)", this, SLOT(fileBackMoveUp()));
    mOptionMenuActions.mFileOpenDrive = mOptionMenuActions.mFileMenu->addAction("Open drive (->)", this, SLOT(fileOpenDrive()));
    mOptionMenuActions.mFileOpenDirectory = mOptionMenuActions.mFileMenu->addAction("Open directory (->)", this, SLOT(fileOpenDirectory()));
    mOptionMenuActions.mFileSearch = mOptionMenuActions.mFileMenu->addAction("Search...", this, SLOT(fileSearch()));
    //mOptionMenuActions.mFileSearch->setVisible(false);

    mOptionMenuActions.mFileNewMenu = mOptionMenuActions.mFileMenu->addMenu("New");
    mOptionMenuActions.mFileNewFile = mOptionMenuActions.mFileNewMenu->addAction("File", this, SLOT(fileNewFile()));
    mOptionMenuActions.mFileNewDirectory = mOptionMenuActions.mFileNewMenu->addAction("Directory", this, SLOT(fileNewDirectory()));

    mOptionMenuActions.mFileDelete = mOptionMenuActions.mFileMenu->addAction("Delete", this, SLOT(fileDelete()));
    mOptionMenuActions.mFileRename = mOptionMenuActions.mFileMenu->addAction("Rename", this, SLOT(fileRename()));
    mOptionMenuActions.mFileTouch = mOptionMenuActions.mFileMenu->addAction("Touch", this, SLOT(fileTouch()));
    mOptionMenuActions.mFileProperties = mOptionMenuActions.mFileMenu->addAction("Properties", this, SLOT(fileProperties()));

//    mOptionMenuActions.mFileChecksumsMenu = mOptionMenuActions.mFileMenu->addMenu("Checksums");
//    mOptionMenuActions.mFileChecksumsMD5 = mOptionMenuActions.mFileChecksumsMenu->addAction("MD5", this, SLOT(fileChecksumsMD5()));
//    mOptionMenuActions.mFileChecksumsMD2 = mOptionMenuActions.mFileChecksumsMenu->addAction("MD2", this, SLOT(fileChecksumsMD2()));
//    mOptionMenuActions.mFileChecksumsSHA1 = mOptionMenuActions.mFileChecksumsMenu->addAction("SHA-1", this, SLOT(fileChecksumsSHA1()));

    mOptionMenuActions.mFileSetAttributes = mOptionMenuActions.mFileMenu->addAction("Set attributes...", this, SLOT(fileSetAttributes()));
    mOptionMenuActions.mFileSetAttributes->setVisible(false);
}

/**
  Initial setup for Edit submenu
  */
void FileBrowserView::createEditMenu()
{
    mOptionMenuActions.mEditMenu = menu()->addMenu("Edit");

    mOptionMenuActions.mEditSnapShotToE = mOptionMenuActions.mEditMenu->addAction("Snap shot to E:", this, SLOT(editSnapShotToE()));
    mOptionMenuActions.mEditSnapShotToE->setVisible(false);
    mOptionMenuActions.mEditCut = mOptionMenuActions.mEditMenu->addAction("Cut", this, SLOT(editCut()));
    mOptionMenuActions.mEditCopy = mOptionMenuActions.mEditMenu->addAction("Copy", this, SLOT(editCopy()));
    mOptionMenuActions.mEditPaste = mOptionMenuActions.mEditMenu->addAction("Paste", this, SLOT(editPaste()));

    mOptionMenuActions.mEditCopyToFolder = mOptionMenuActions.mEditMenu->addAction("Copy to folder...", this, SLOT(editCopyToFolder()));
    mOptionMenuActions.mEditMoveToFolder = mOptionMenuActions.mEditMenu->addAction("Move to folder...", this, SLOT(editMoveToFolder()));

    mOptionMenuActions.mEditSelect = mOptionMenuActions.mEditMenu->addAction("Select", this, SLOT(editSelect()));
    mOptionMenuActions.mEditUnselect = mOptionMenuActions.mEditMenu->addAction("Unselect", this, SLOT(editUnselect()));
    mOptionMenuActions.mEditSelectAll = mOptionMenuActions.mEditMenu->addAction("Select all", this, SLOT(editSelectAll()));
    mOptionMenuActions.mEditUnselectAll = mOptionMenuActions.mEditMenu->addAction("Unselect all", this, SLOT(editUnselectAll()));
}

/**
  Initial setup for View submenu
  */
void FileBrowserView::createViewMenu()
{
    mOptionMenuActions.mViewMenu = menu()->addMenu("View");
    mOptionMenuActions.mViewMenu->menuAction()->setVisible(false);

    mOptionMenuActions.mViewFilterEntries = mOptionMenuActions.mViewMenu->addAction("Filter entries", this, SLOT(viewFilterEntries()));
    mOptionMenuActions.mViewRefresh = mOptionMenuActions.mViewMenu->addAction("Refresh", this, SLOT(viewRefresh()));
}

/**
  Initial setup for Tools submenu
  */
void FileBrowserView::createToolsMenu()
{
    mOptionMenuActions.mToolsMenu = menu()->addMenu("Tools");

    mOptionMenuActions.mToolsAllAppsToTextFile = mOptionMenuActions.mToolsMenu->addAction("All apps to a text file", this, SLOT(toolsAllAppsToTextFile()));
    mOptionMenuActions.mToolsAllAppsToTextFile->setVisible(false);
    mOptionMenuActions.mToolsAllFilesToTextFile = mOptionMenuActions.mToolsMenu->addAction("All files to a text file", this, SLOT(toolsAllFilesToTextFile()));
    //mOptionMenuActions.mToolsAllFilesToTextFile->setVisible(false);

    mOptionMenuActions.mToolsAvkonIconCacheMenu = mOptionMenuActions.mToolsMenu->addMenu("Avkon icon cache");
    mOptionMenuActions.mToolsAvkonIconCacheMenu->menuAction()->setVisible(false);
    mOptionMenuActions.mToolsAvkonIconCacheEnable = mOptionMenuActions.mToolsAvkonIconCacheMenu->addAction("Enable", this, SLOT(toolsAvkonIconCacheEnable()));
    mOptionMenuActions.mToolsAvkonIconCacheDisable = mOptionMenuActions.mToolsAvkonIconCacheMenu->addAction("Clear and disable", this, SLOT(toolsAvkonIconCacheDisable()));

    mOptionMenuActions.mToolsDisableExtendedErrors = mOptionMenuActions.mToolsMenu->addAction("Disable extended errors", this, SLOT(toolsDisableExtendedErrors()));
    mOptionMenuActions.mToolsDumpMsgStoreWalk = mOptionMenuActions.mToolsMenu->addAction("Dump msg. store walk", this, SLOT(toolsDumpMsgStoreWalk()));
    mOptionMenuActions.mToolsDumpMsgStoreWalk->setVisible(false);
    mOptionMenuActions.mToolsEditDataTypes = mOptionMenuActions.mToolsMenu->addAction("Edit data types", this, SLOT(toolsEditDataTypes()));
    mOptionMenuActions.mToolsEditDataTypes->setVisible(false);
    mOptionMenuActions.mToolsEnableExtendedErrors = mOptionMenuActions.mToolsMenu->addAction("Enable extended errors", this, SLOT(toolsEnableExtendedErrors()));

    mOptionMenuActions.mToolsErrorSimulateMenu = mOptionMenuActions.mToolsMenu->addMenu("Error simulate");
    mOptionMenuActions.mToolsErrorSimulateLeave = mOptionMenuActions.mToolsErrorSimulateMenu->addAction("Leave", this, SLOT(toolsErrorSimulateLeave()));
    mOptionMenuActions.mToolsErrorSimulatePanic = mOptionMenuActions.mToolsErrorSimulateMenu->addAction("Panic", this, SLOT(toolsErrorSimulatePanic()));
    mOptionMenuActions.mToolsErrorSimulatePanic->setVisible(false);
    mOptionMenuActions.mToolsErrorSimulateException = mOptionMenuActions.mToolsErrorSimulateMenu->addAction("Exception", this, SLOT(toolsErrorSimulateException()));

//    mOptionMenuActions.mLocalConnectivityMenu = mOptionMenuActions.mToolsMenu->addMenu("Local connectivity");
//    mOptionMenuActions.mToolsLocalConnectivityActivateInfrared = mOptionMenuActions.mLocalConnectivityMenu->addAction("Activate infrared", this, SLOT(toolsLocalConnectivityActivateInfrared()));
//    mOptionMenuActions.mToolsLocalConnectivityLaunchBTUI = mOptionMenuActions.mLocalConnectivityMenu->addAction("Launch BT UI", this, SLOT(toolsLocalConnectivityLaunchBTUI()));
//    mOptionMenuActions.mToolsLocalConnectivityLaunchUSBUI = mOptionMenuActions.mLocalConnectivityMenu->addAction("Launch USB UI", this, SLOT(toolsLocalConnectivityLaunchUSBUI()));

    mOptionMenuActions.mToolsMessageAttachmentsMenu = mOptionMenuActions.mToolsMenu->addMenu("Message attachments");
    mOptionMenuActions.mToolsMessageAttachmentsMenu->menuAction()->setVisible(false);
    mOptionMenuActions.mToolsMessageInbox = mOptionMenuActions.mToolsMessageAttachmentsMenu->addAction("Inbox", this, SLOT(toolsMessageInbox()));
    mOptionMenuActions.mToolsMessageDrafts = mOptionMenuActions.mToolsMessageAttachmentsMenu->addAction("Drafts", this, SLOT(toolsMessageDrafts()));
    mOptionMenuActions.mToolsMessageSentItems = mOptionMenuActions.mToolsMessageAttachmentsMenu->addAction("Sent items", this, SLOT(toolsMessageSentItems()));
    mOptionMenuActions.mToolsMessageOutbox = mOptionMenuActions.mToolsMessageAttachmentsMenu->addAction("Outbox", this, SLOT(toolsMessageOutbox()));

    mOptionMenuActions.mToolsMemoryInfo = mOptionMenuActions.mToolsMenu->addAction("Memory info", this, SLOT(toolsMemoryInfo()));
    mOptionMenuActions.mToolsMemoryInfo->setVisible(false);

    mOptionMenuActions.mToolsSecureBackupMenu = mOptionMenuActions.mToolsMenu->addMenu("Secure backup");
    mOptionMenuActions.mToolsSecureBackupMenu->menuAction()->setVisible(false);
    mOptionMenuActions.mToolsSecureBackStart = mOptionMenuActions.mToolsSecureBackupMenu->addAction("Start backup", this, SLOT(toolsSecureBackStart()));
    mOptionMenuActions.mToolsSecureBackRestore = mOptionMenuActions.mToolsSecureBackupMenu->addAction("Start restore", this, SLOT(toolsSecureBackRestore()));
    mOptionMenuActions.mToolsSecureBackStop = mOptionMenuActions.mToolsSecureBackupMenu->addAction("Stop", this, SLOT(toolsSecureBackStop()));

    mOptionMenuActions.mToolsSetDebugMask = mOptionMenuActions.mToolsMenu->addAction("Set debug mask", this, SLOT(toolsSetDebugMaskQuestion()));
    mOptionMenuActions.mToolsShowOpenFilesHere = mOptionMenuActions.mToolsMenu->addAction("Show open files here", this, SLOT(toolsShowOpenFilesHere()));
    mOptionMenuActions.mToolsShowOpenFilesHere->setVisible(false);
}

/**
  Creates Selection mode menu item in option menu
  */
void FileBrowserView::createSelectionMenuItem()
{
    if (!mOptionMenuActions.mSelection) {
        mOptionMenuActions.mSelection = menu()->addAction("Selection mode");
        mOptionMenuActions.mSelection->setToolTip("Selection mode");
        mOptionMenuActions.mSelection->setCheckable(true);
        connect(mOptionMenuActions.mSelection, SIGNAL(triggered()), this, SLOT(selectionModeChanged()));
    }
}

/**
  Creates Setting menu item in option menu
  */
void FileBrowserView::createSettingsMenuItem()
{
    mOptionMenuActions.mSetting = menu()->addAction("Settings...");
    connect(mOptionMenuActions.mSetting, SIGNAL(triggered()), this, SIGNAL(aboutToShowSettingsView()));
}


/**
  Creates About menu item in option menu
  */
void FileBrowserView::createAboutMenuItem()
{
    // about note
    mOptionMenuActions.mAbout = menu()->addAction("About");
    connect(mOptionMenuActions.mAbout, SIGNAL(triggered()), this, SLOT(about()));
}

/**
  Creates Exit menu item in option menu
  */
void FileBrowserView::createExitMenuItem()
{
    // application exit
    mOptionMenuActions.mExit = menu()->addAction("Exit");
    connect(mOptionMenuActions.mExit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

/**
  update menu: disk admin available only in device root view. edit available only in folder view
  when file or folder content exist in current folder, or clipboard has copied item.
  file and view menus updated every time regarding the folder content.
  tools, settings, about, exit always available.
  If there's remove and add operations at same time, always remove first
  to keep to the correct menu items order.
  */
void FileBrowserView::updateOptionMenu()
{
    bool isFileItemListEmpty = mFileBrowserModel->rowCount() == 0;
    bool isDriveListActive = mEngineWrapper->isDriveListViewActive();
    bool isNormalModeActive = true;       //iModel->FileUtils()->IsNormalModeActive();
    bool currentDriveReadOnly = mEngineWrapper->isCurrentDriveReadOnly();   //iModel->FileUtils()->IsCurrentDriveReadOnly();
    bool currentItemDirectory = mEngineWrapper->getFileEntry(currentItemIndex()).isDir();
    bool listBoxSelections = mListView->selectionModel()->selection().count() == 0;
    bool isSelectionMode = mOptionMenuActions.mSelection && mOptionMenuActions.mSelection->isChecked();
    bool emptyClipBoard = !mEngineWrapper->isClipBoardListInUse();
    bool showSnapShot = false;           //iModel->FileUtils()->DriveSnapShotPossible();

    bool showEditMenu(true);
    if (isDriveListActive) {
        if (!showSnapShot || isFileItemListEmpty && emptyClipBoard)
            showEditMenu = false;
        else
            showEditMenu = true;
    } else {
        if (isFileItemListEmpty && emptyClipBoard)
            showEditMenu = false;
        else
            showEditMenu = true;
    }

    mOptionMenuActions.mEditMenu->menuAction()->setVisible(showEditMenu);
    // TODO mContextMenuActions.mDiskAdminMenu->menuAction()->setVisible(isDriveListActive);

    mOptionMenuActions.mFileBackMoveUp->setVisible( !isDriveListActive);

    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileOpen, isFileItemListEmpty || isDriveListActive || currentItemDirectory);
    mOptionMenuActions.mFileOpenDrive->setVisible( !(isFileItemListEmpty || !isDriveListActive));
    mOptionMenuActions.mFileOpenDirectory->setVisible( !(isFileItemListEmpty || isDriveListActive || !currentItemDirectory));

    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileView, isFileItemListEmpty || listBoxSelections || currentItemDirectory || isDriveListActive);
    //aMenuPane->SetItemDimmed(EFileBrowserCmd FileEdit, isFileItemListEmpty || listBoxSelections || currentItemDirectory || isDriveListActive);
    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileSendTo, isFileItemListEmpty || driveListActive || currentItemDirectory);

    mOptionMenuActions.mFileNewMenu->menuAction()->setVisible(!(isDriveListActive || currentDriveReadOnly));
    mOptionMenuActions.mFileDelete->setVisible(!isFileItemListEmpty && !isDriveListActive && !currentDriveReadOnly && isSelectionMode);
    mOptionMenuActions.mFileRename->setVisible(!isFileItemListEmpty && !isDriveListActive && !currentDriveReadOnly && !listBoxSelections && isSelectionMode);
    mOptionMenuActions.mFileTouch->setVisible(!(isFileItemListEmpty || isDriveListActive || currentDriveReadOnly));
    mOptionMenuActions.mFileProperties->setVisible(!(isFileItemListEmpty || listBoxSelections));
    // TODO mOptionMenuActions.mFileChecksumsMenu->setVisible(!(isFileItemListEmpty || listBoxSelections || currentItemDirectory || isDriveListActive));
    // TODO mOptionMenuActions.mFileSetAttributes->setVisible(!(isFileItemListEmpty || isDriveListActive || currentDriveReadOnly));
    // TODO mOptionMenuActions.mFileCompress->setVisible(!(currentDriveReadOnly || isFileItemListEmpty || listBoxSelections || currentItemDirectory || isDriveListActive));
    // TODO mOptionMenuActions.mFileDecompress->setVisible(!(currentDriveReadOnly || isFileItemListEmpty || listBoxSelections || currentItemDirectory || isDriveListActive));

//    bool currentSelected = true;    //iContainer->ListBox()->View()->ItemIsSelected(iContainer->ListBox()->View()->CurrentItemIndex());
    bool allSelected = mListView->selectionModel()->selection().count() == mFileBrowserModel->rowCount();
    bool noneSelected = mListView->selectionModel()->selection().count() != 0;

    //mOptionMenuActions.mEditSnapShotToE->setVisible(isDriveListActive); // TODO
    mOptionMenuActions.mEditCut->setVisible(!isDriveListActive && !currentDriveReadOnly && !isFileItemListEmpty && !isSelectionMode);
    mOptionMenuActions.mEditCopy->setVisible(!isDriveListActive && !isFileItemListEmpty);
    mOptionMenuActions.mEditPaste->setVisible(!(isDriveListActive || emptyClipBoard || currentDriveReadOnly));
    mOptionMenuActions.mEditCopyToFolder->setVisible(!(isDriveListActive || isFileItemListEmpty));
    mOptionMenuActions.mEditMoveToFolder->setVisible(!(isDriveListActive || currentDriveReadOnly || isFileItemListEmpty));

    mOptionMenuActions.mEditSelect->setVisible(false/*!isDriveListActive && !currentSelected && !isFileItemListEmpty*/);
    mOptionMenuActions.mEditUnselect->setVisible(false/*!isDriveListActive && currentSelected && !isFileItemListEmpty*/);
    mOptionMenuActions.mEditSelectAll->setVisible(!isDriveListActive && !allSelected && !isFileItemListEmpty);
    mOptionMenuActions.mEditUnselectAll->setVisible(!isDriveListActive && !noneSelected && !isFileItemListEmpty);

    // TODO mOptionMenuActions.mViewSort->setVisible(!(!isNormalModeActive || isDriveListActive || isFileItemListEmpty));
    // TODO mOptionMenuActions.mViewOrder->setVisible(!(!isNormalModeActive || isDriveListActive || isFileItemListEmpty));
    mOptionMenuActions.mViewRefresh->setVisible(isNormalModeActive);
    mOptionMenuActions.mViewFilterEntries->setVisible(!isFileItemListEmpty);

    // TODO R_FILEBROWSER_VIEW_SORT_SUBMENU
    // aMenuPane->SetItemButtonState(iModel->FileUtils()->SortMode(), EEikMenuItemSymbolOn);

    // TODO R_FILEBROWSER_VIEW_ORDER_SUBMENU
    // aMenuPane->SetItemButtonState(iModel->FileUtils()->OrderMode(), EEikMenuItemSymbolOn);

    // aResourceId == R_FILEBROWSER_TOOLS_SUBMENU
    bool noExtendedErrorsAllowed = mEngineWrapper->ErrRdFileExists();
    mOptionMenuActions.mToolsDisableExtendedErrors->setVisible(noExtendedErrorsAllowed);
    mOptionMenuActions.mToolsEnableExtendedErrors->setVisible(!noExtendedErrorsAllowed);

//    bool infraRedAllowed = mEngineWrapper->FileExists(KIRAppPath);
//    bool bluetoothAllowed = mEngineWrapper->FileExists(KBTAppPath);
//    bool usbAllowed = mEngineWrapper->FileExists(KUSBAppPath);
//
//    bool noLocalCon = !infraRedAllowed && !bluetoothAllowed && !usbAllowed;
//    mOptionMenuActions.mToolsLocalConnectivityMenu->menuAction()->setVisible(!noLocalCon);
//
//    mOptionMenuActions.mToolsLocalConnectivityActivateInfrared->setVisible(infraRedAllowed);
//    mOptionMenuActions.mToolsLocalConnectivityLaunchBTUI->setVisible(bluetoothAllowed);
//    mOptionMenuActions.mToolsLocalConnectivityLaunchUSBUI->setVisible(usbAllowed);
}

void FileBrowserView::createContextMenu()
{
    mContextMenu = new HbMenu();
    connect(mContextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));

    createFileContextMenu();
    createEditContextMenu();
    createViewContextMenu();
    createDiskAdminContextMenu();
}


void FileBrowserView::createFileContextMenu()
{
    mContextMenuActions.mFileMenu = mContextMenu->addMenu("File");

    mContextMenuActions.mFileBackMoveUp = mContextMenuActions.mFileMenu->addAction("Back/Move up (<-)", this, SLOT(fileBackMoveUp()));
    mContextMenuActions.mFileOpenDrive = mContextMenuActions.mFileMenu->addAction("Open drive (->)", this, SLOT(fileOpenDrive()));
    mContextMenuActions.mFileOpenDirectory = mContextMenuActions.mFileMenu->addAction("Open directory (->)", this, SLOT(fileOpenDirectory()));
//    mContextMenuActions.mFileSearch = mContextMenuActions.mFileMenu->addAction("Search...", this, SLOT(fileSearch()));
    //mContextMenuActions.mFileSearch->setVisible(false);

//    mContextMenuActions.mFileNewMenu = mContextMenuActions.mFileMenu->addMenu("New");
//    mContextMenuActions.mFileNewFile = mContextMenuActions.mFileNewMenu->addAction("File", this, SLOT(fileNewFile()));
//    mContextMenuActions.mFileNewDirectory = mContextMenuActions.mFileNewMenu->addAction("Directory", this, SLOT(fileNewDirectory()));

    mContextMenuActions.mFileDelete = mContextMenuActions.mFileMenu->addAction("Delete", this, SLOT(fileDelete()));
    mContextMenuActions.mFileRename = mContextMenuActions.mFileMenu->addAction("Rename", this, SLOT(fileRename()));
    mContextMenuActions.mFileTouch = mContextMenuActions.mFileMenu->addAction("Touch", this, SLOT(fileTouch()));
    mContextMenuActions.mFileProperties = mContextMenuActions.mFileMenu->addAction("Properties", this, SLOT(fileProperties()));

    mContextMenuActions.mFileChecksumsMenu = mContextMenuActions.mFileMenu->addMenu("Checksums");
    mContextMenuActions.mFileChecksumsMD5 = mContextMenuActions.mFileChecksumsMenu->addAction("MD5", this, SLOT(fileChecksumsMD5()));
    mContextMenuActions.mFileChecksumsMD2 = mContextMenuActions.mFileChecksumsMenu->addAction("MD2", this, SLOT(fileChecksumsMD2()));
    mContextMenuActions.mFileChecksumsSHA1 = mContextMenuActions.mFileChecksumsMenu->addAction("SHA-1", this, SLOT(fileChecksumsSHA1()));

//    mContextMenuActions.mFileSetAttributes = mContextMenuActions.mFileMenu->addAction("Set attributes...", this, SLOT(fileSetAttributes()));
//    mContextMenuActions.mFileSetAttributes->setVisible(false);
}

void FileBrowserView::createEditContextMenu()
{
    mContextMenuActions.mEditMenu = mContextMenu->addMenu("Edit");

    //mContextMenuActions.mEditSnapShotToE = mContextMenuActions.mEditMenu->addAction("Snap shot to E:", this, SLOT(editSnapShotToE()));
//    mContextMenuActions.mEditSnapShotToE->setVisible(false);
    mContextMenuActions.mEditCut = mContextMenuActions.mEditMenu->addAction("Cut", this, SLOT(editCut()));
    mContextMenuActions.mEditCopy = mContextMenuActions.mEditMenu->addAction("Copy", this, SLOT(editCopy()));
    mContextMenuActions.mEditPaste = mContextMenuActions.mEditMenu->addAction("Paste", this, SLOT(editPaste()));

    mContextMenuActions.mEditCopyToFolder = mContextMenuActions.mEditMenu->addAction("Copy to folder...", this, SLOT(editCopyToFolder()));
    mContextMenuActions.mEditMoveToFolder = mContextMenuActions.mEditMenu->addAction("Move to folder...", this, SLOT(editMoveToFolder()));
}

void FileBrowserView::createViewContextMenu()
{

}

/**
  Initial setup for Disk Admin submenu
  */
void FileBrowserView::createDiskAdminContextMenu()
{
    mContextMenuActions.mDiskAdminMenu = mContextMenu->addMenu("Disk admin");
    //mContextMenuActions.mDiskAdminMenu->menuAction()->setVisible(false);

    mContextMenuActions.mDiskAdminSetDrivePassword = mContextMenuActions.mDiskAdminMenu->addAction("Set drive password", this, SLOT(diskAdminSetDrivePassword()));
    mContextMenuActions.mDiskAdminUnlockDrive = mContextMenuActions.mDiskAdminMenu->addAction("Unlock drive", this, SLOT(diskAdminUnlockDrive()));
    mContextMenuActions.mDiskAdminClearDrivePassword = mContextMenuActions.mDiskAdminMenu->addAction("Clear drive password", this, SLOT(diskAdminClearDrivePassword()));
    mContextMenuActions.mDiskAdminEraseDrivePassword = mContextMenuActions.mDiskAdminMenu->addAction("Erase drive password", this, SLOT(diskAdminEraseDrivePassword()));

    mContextMenuActions.mDiskAdminFormatDrive = mContextMenuActions.mDiskAdminMenu->addAction("Format drive", this, SLOT(diskAdminFormatDrive()));
    mContextMenuActions.mDiskAdminFormatDrive->setVisible(false);
    mContextMenuActions.mDiskAdminQuickFormatDrive = mContextMenuActions.mDiskAdminMenu->addAction("Quick format drive", this, SLOT(diskAdminQuickFormatDrive()));
    mContextMenuActions.mDiskAdminQuickFormatDrive->setVisible(false);

    mContextMenuActions.mDiskAdminCheckDisk = mContextMenuActions.mDiskAdminMenu->addAction("Check disk", this, SLOT(diskAdminCheckDisk()));
    mContextMenuActions.mDiskAdminScanDrive = mContextMenuActions.mDiskAdminMenu->addAction("Scan drive", this, SLOT(diskAdminScanDrive()));
    mContextMenuActions.mDiskAdminSetDriveName = mContextMenuActions.mDiskAdminMenu->addAction("Set drive name", this, SLOT(diskAdminSetDriveName()));
    mContextMenuActions.mDiskAdminSetDriveVolumeLabel = mContextMenuActions.mDiskAdminMenu->addAction("Set drive volume label", this, SLOT(diskAdminSetDriveVolumeLabel()));
    mContextMenuActions.mDiskAdminEjectDrive = mContextMenuActions.mDiskAdminMenu->addAction("Eject drive", this, SLOT(diskAdminEjectDrive()));
    mContextMenuActions.mDiskAdminDismountDrive = mContextMenuActions.mDiskAdminMenu->addAction("Dismount drive", this, SLOT(diskAdminDismountDrive()));
    mContextMenuActions.mDiskAdminEraseMBR = mContextMenuActions.mDiskAdminMenu->addAction("Erase MBR", this, SLOT(diskAdminEraseMBR()));
    mContextMenuActions.mDiskAdminPartitionDrive = mContextMenuActions.mDiskAdminMenu->addAction("Partition drive", this, SLOT(diskAdminPartitionDrive()));
}

void FileBrowserView::updateContextMenu()
{
    bool isFileItemListEmpty = mFileBrowserModel->rowCount() == 0;
    bool isDriveListActive = mEngineWrapper->isDriveListViewActive();
//    bool isNormalModeActive = true;       //iModel->FileUtils()->IsNormalModeActive();
    bool currentDriveReadOnly = mEngineWrapper->isCurrentDriveReadOnly();
    bool currentItemDirectory = mEngineWrapper->getFileEntry(mCurrentIndex /*currentItemIndex()*/).isDir();
    bool listBoxSelections = mListView->selectionModel()->selection().count() == 0;
    bool isSelectionMode = mOptionMenuActions.mSelection && mOptionMenuActions.mSelection->isChecked();
    bool emptyClipBoard = !mEngineWrapper->isClipBoardListInUse();
//    bool showSnapShot = false;           //iModel->FileUtils()->DriveSnapShotPossible();

//    bool showEditMenu(true);
//    if (isDriveListActive) {
//        if (!showSnapShot || isFileItemListEmpty && emptyClipBoard)
//            showEditMenu = false;
//        else
//            showEditMenu = true;
//    } else {
//        if (isFileItemListEmpty && emptyClipBoard)
//            showEditMenu = false;
//        else
//            showEditMenu = true;
//    }

    // File submenu
    mContextMenuActions.mFileBackMoveUp->setVisible( !isDriveListActive);
    mContextMenuActions.mFileOpenDrive->setVisible( !isFileItemListEmpty && isDriveListActive);
    mContextMenuActions.mFileOpenDirectory->setVisible( !isFileItemListEmpty && !isDriveListActive && currentItemDirectory);

//    mContextMenuActions.mFileNewMenu->menuAction()->setVisible(!(isDriveListActive || currentDriveReadOnly));
    mContextMenuActions.mFileDelete->setVisible(!isFileItemListEmpty && !isDriveListActive && !currentDriveReadOnly);
    mContextMenuActions.mFileRename->setVisible(!isFileItemListEmpty && !isDriveListActive && !currentDriveReadOnly && !listBoxSelections);
    mContextMenuActions.mFileTouch->setVisible(!isFileItemListEmpty && !isDriveListActive && !currentDriveReadOnly);
    mContextMenuActions.mFileProperties->setVisible(!isFileItemListEmpty && !listBoxSelections && !isSelectionMode);

    mContextMenuActions.mFileChecksumsMenu->menuAction()->setVisible(!(isFileItemListEmpty || isSelectionMode /*|| listBoxSelections*/ || currentItemDirectory || isDriveListActive));
    // Edit submenu
    mContextMenuActions.mEditMenu->menuAction()->setVisible(!isDriveListActive);
    mContextMenuActions.mEditCut->setVisible(!(isDriveListActive || currentDriveReadOnly || isFileItemListEmpty));
    mContextMenuActions.mEditCopy->setVisible(!(isDriveListActive || isFileItemListEmpty));
    mContextMenuActions.mEditPaste->setVisible(!isDriveListActive && !emptyClipBoard && !currentDriveReadOnly);
    mContextMenuActions.mEditCopyToFolder->setVisible(!(isDriveListActive || isFileItemListEmpty));
    mContextMenuActions.mEditMoveToFolder->setVisible(!(isDriveListActive || currentDriveReadOnly || isFileItemListEmpty));
    //DiskAdmin submenu
    mContextMenuActions.mDiskAdminMenu->menuAction()->setVisible(isDriveListActive);
}

// ---------------------------------------------------------------------------

void FileBrowserView::onLongPressed(HbAbstractViewItem *listViewItem, QPointF coords)
{
    //Q_UNUSED(listViewItem);

//    QItemSelectionModel *selectionIndexes = mListView->selectionModel();

    // by default use selected items
//    if (selectionIndexes && selectionIndexes->hasSelection()) {
//        mSelectionIndexes = mListView->selectionModel()->selectedIndexes();
//    } else {
        mCurrentIndex = listViewItem->modelIndex();
//        mSelectionIndexes.clear();
//        mSelectionIndexes.append(mModelIndex);
//    }
    mContextMenu->setPreferredPos(coords);
    mContextMenu->show();
}


/**
  Create a file browser tool bar
  */
void FileBrowserView::createToolBar()
{
    mToolBar = new HbToolBar(this);

    mToolbarBackAction = new HbAction(/*"Back"*/);
    mToolbarBackAction->setToolTip("Back");
    mToolbarBackAction->setIcon(HbIcon(QString(":/qgn_indi_tb_filebrowser_folder_parent.svg")));
    connect(mToolbarBackAction, SIGNAL(triggered()), this, SLOT(fileBackMoveUp()));
    mToolBar->addAction(mToolbarBackAction);

    if (mOptionMenuActions.mSelection) {
        mToolBar->addAction(mOptionMenuActions.mSelection);
    }

    setToolBar(mToolBar);
}

/**
  Refresh FileBrowser view
  */
void FileBrowserView::refreshList()
{
    mEngineWrapper->refreshView();
//    mNaviPane->setPlainText(QString(mEngineWrapper->currentPath()));
    mListView->reset();
    mListView->setModel(mFileBrowserModel);
    mToolbarBackAction->setEnabled(!mEngineWrapper->isDriveListViewActive());

    TListingMode listingMode = mEngineWrapper->listingMode();
    if (listingMode == ENormalEntries)
        mNaviPane->setPlainText(QString(mEngineWrapper->currentPath()));
    else if (listingMode == ESearchResults)
        mNaviPane->setPlainText(QString(tr("Search results")));
    else if (listingMode == EOpenFiles)
        mNaviPane->setPlainText(QString(tr("Open files")));
    else if (listingMode == EMsgAttachmentsInbox)
        mNaviPane->setPlainText(QString(tr("Attachments in Inbox")));
    else if (listingMode == EMsgAttachmentsDrafts)
        mNaviPane->setPlainText(QString(tr("Attachments in Drafts")));
    else if (listingMode == EMsgAttachmentsSentItems)
        mNaviPane->setPlainText(QString(tr("Attachments in Sent Items")));
    else if (listingMode == EMsgAttachmentsOutbox)
        mNaviPane->setPlainText(QString(tr("Attachments in Outbox")));
}

/**
  Populate changed folder content, i.e. in practice navigation list items
  */
void FileBrowserView::populateFolderContent()
{
    // update the file browser by setting up the model with current directory as root path
    if(mListView->model() == 0)  {
        mFileBrowserModel = new FileBrowserModel(mEngineWrapper);
        mListView->setModel(mFileBrowserModel);
    }

    refreshList();
    //mFileSystemModel->setFilter(mFileSystemModel->filter() | QDir::System | QDir::Hidden);
    //mFileSystemModel->setRootPath(directory);
    //mListView->setRootIndex(mFileSystemModel->index(directory));
}

// ---------------------------------------------------------------------------	

void FileBrowserView::fileOpen(HbAction *action)
{
//    Q_UNUSED(action);
    HbSelectionDialog *dlg = static_cast<HbSelectionDialog*>(sender());
    if(!action && dlg && dlg->selectedItems().count()){
        int selectionIndex = dlg->selectedItems().at(0).toInt();

        if (selectionIndex == 0) {
            // open editor view
            emit aboutToShowEditorView(mAbsoluteFilePath, true);
        } else if (selectionIndex == 1) {
            // AppArc
            mEngineWrapper->openAppArc(mAbsoluteFilePath);
        } else {
            // DocHandler
            mEngineWrapper->openDocHandler(mAbsoluteFilePath, true);
        }
    }
}

/**
  Open overwrite dialog
  */
void FileBrowserView::fileOverwriteDialog()
{
    mOverwriteOptions = OverwriteOptions();
    // open user-dialog to select: view as text/hex,  open w/AppArc or open w/DocH. embed
    QStringList list;
    list << QString("Overwrite all")
            << QString("Skip all existing")
            << QString("Gen. unique filenames")
            << QString("Query postfix");
    openListDialog(list, QString("Overwrite?"), this, SLOT(fileOverwrite(HbAction *)));
}

/**
  File overwrite
  */
void FileBrowserView::fileOverwrite(HbAction *action)
{
//    Q_UNUSED(action);
    HbSelectionDialog *dlg = static_cast<HbSelectionDialog*>(sender());
    if(!action && dlg && dlg->selectedItems().count()) {
        mOverwriteOptions.queryIndex = dlg->selectedItems().at(0).toInt();
        if (mOverwriteOptions.queryIndex == EFileActionQueryPostFix) {
            QString heading = QString("Postfix");
            HbInputDialog::getText(heading, this, SLOT(fileOverwritePostfix(HbAction *)), QString(), scene());
        } else if (mOverwriteOptions.queryIndex == EFileActionSkipAllExisting) {
            mOverwriteOptions.overWriteFlags = 0;
        }
    } else {
        mOverwriteOptions.doFileOperations = false;
    }
}

/**
  File overwrite postfix query dialog
  */
void FileBrowserView::fileOverwritePostfix(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        mOverwriteOptions.postFix = dlg->value().toString();
    } else {
        mOverwriteOptions.doFileOperations = false;
    }
}

// ---------------------------------------------------------------------------
/**
  Show a list dialog
  \param List aList of item to select item from.
  \param Title text titleText of a dialog heading widget
  \return None
  */
void FileBrowserView::openListDialog(const QStringList& items, const QString &titleText, QObject* receiver, const char* member)
{
//    Q_UNUSED(items);
//    Q_UNUSED(titleText);
//    Q_UNUSED(receiver);
//    Q_UNUSED(member);
    // Create a list and some simple content for it
    HbSelectionDialog *dlg = new HbSelectionDialog();
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    // Set items to be popup's content
    dlg->setStringItems(items);
    dlg->setSelectionMode(HbAbstractItemView::SingleSelection);
    //dlg->setDismissPolicy(HbPopup::TapOutside);

    HbLabel *title = new HbLabel(dlg);
    title->setPlainText(titleText);
    dlg->setHeadingWidget(title);

    // Launch popup and handle the user response:
    dlg->open(receiver, member);
}

// ---------------------------------------------------------------------------

void FileBrowserView::openPropertyDialog(const QStringList& propertyList, const QString& title)
{
    HbDialog *dialog = new HbDialog();
    dialog->setDismissPolicy(HbPopup::TapOutside);
    dialog->setTimeout(HbPopup::NoTimeout);

    HbLabel *titleWidget = new HbLabel();
    titleWidget->setPlainText(title);
    dialog->setHeadingWidget(titleWidget);

    // Create a list and some simple content for it
    HbListWidget *list = new HbListWidget();
    QString str;
    foreach (str, propertyList) {
        list->addItem(str);
    }

    // Connect list item activation signal to close the popup
    connect(list, SIGNAL(activated(HbListWidgetItem*)), dialog, SLOT(close()));

    HbAction *cancelAction = new HbAction("Close");
    dialog->setPrimaryAction(cancelAction);

    // Set listwidget to be popup's content
    dialog->setContentWidget(list);
    // Launch popup and handle the user response:
    dialog->open();
}

void FileBrowserView::storeSelectedItemsOrCurrentItem()
{
    QItemSelectionModel *selectionIndexes = mListView->selectionModel();

    // by default use selected items
    if (selectionIndexes) {
        if (selectionIndexes->hasSelection()) {
            mSelectionIndexes = mListView->selectionModel()->selectedIndexes();
        } else { // or if none selected, use the current item index
            mSelectionIndexes.clear();
            mSelectionIndexes.append(mCurrentIndex);
//            QModelIndex currentIndex = currentItemIndex();
//            if (mFileBrowserModel->rowCount(currentItemIndex) > currentItemIndex && currentItemIndex >= 0)
//            {
//                modelIndexList.append(currentIndex);
//            }
        }
    }
//    mClipBoardInUse = true;
}

// ---------------------------------------------------------------------------

QModelIndex FileBrowserView::currentItemIndex()
{
    return mCurrentIndex;//mListView->selectionModel()->currentIndex();
}

// ---------------------------------------------------------------------------
// operations in File Menu
// ---------------------------------------------------------------------------

/**
  Move back/up in folder browsing history
  */
void FileBrowserView::fileBackMoveUp()
{
    mLocationChanged = true;
//    if(mDirectory.length() < DRIVEPATHLENGTH) {
//        // location in the root of any drive -> move back/up to root of device
//        QModelIndex index = currentItemIndex();
//        const QString filePath = mFileSystemModel->filePath(index);
//        qDebug() << "handleBackButton filePath" << filePath;
//        mDirectory = mInitDirPath.path();
//        populateFolderContent(mDirectory);
//    }
//    else if(mDirectory != mInitDirPath.path()) {
//        // location in any folder in any drive -> move back/up
//        QDir dir(mDirectory);
//        dir.cdUp();
//        const QString currentPath = dir.absolutePath();
//        mDirectory = currentPath;
//        populateFolderContent(currentPath);
//        mSelectedFilePath = "";
//    } else {
//        // location already in the device root, no way up.
//        // do nothing.
//    }
    mEngineWrapper->moveUpOneLevel();
    populateFolderContent();
}

void FileBrowserView::fileOpenDrive()
{
    // TODO make a separate function to be called from here and fileOpenDirectory()
    mLocationChanged = true;
    // get selected drive or directory from list view model and open it:
    //if (mListView->selectionModel()->hasSelection()) {
//    if (mListView->selectionModel()->selection().count() != 0) {
//        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->moveDownToDirectory(mCurrentIndex);
        populateFolderContent();
//    } else {
//        Notifications::showErrorNote("not selected item!");
//    }
}

void FileBrowserView::fileOpenDirectory()
{
    mLocationChanged = true;
    // get selected drive or directory from list view model and open it:
    //if (mListView->selectionModel()->hasSelection()) {
//    if (mListView->selectionModel()->selection().count() != 0) {
//        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->moveDownToDirectory(mCurrentIndex);
        populateFolderContent();
//    } else {
//        Notifications::showErrorNote("not selected item!");
//    }
}

void FileBrowserView::fileSearch()
{
    QString searchPath;
//    if (mEngineWrapper->currentPath() != mInitDirPath.path()) {
//        searchPath = mDirectory;
//        searchPath.replace("/", "\\");
//        searchPath+="\\";
//    }
    searchPath = mEngineWrapper->currentPath();
//    mSearch->open(searchPath);
    emit aboutToShowSearchView(searchPath);
}

/**
  Open new file dialog
  */
void FileBrowserView::fileNewFile()
{
    QString heading = QString("Enter filename");
    HbInputDialog::getText(heading, this, SLOT(doFileNewFile(HbAction*)), QString(), scene());
}

/**
  Create a new file in current directory with a name queried from user
  */
void FileBrowserView::doFileNewFile(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString newFileName = dlg->value().toString();
        mEngineWrapper->createNewFile(newFileName);
        refreshList();
    }
}

/**
  Open new directory dialog
  */
void FileBrowserView::fileNewDirectory()
{
    QString heading = QString("Enter directory name");
    HbInputDialog::getText(heading, this, SLOT(doFileNewDirectory(HbAction*)), QString(), scene());
}

/**
  Create a new directory in current directory with a name queried from user
  */
void FileBrowserView::doFileNewDirectory(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString newDirectoryName = dlg->value().toString();
        mEngineWrapper->createNewDirectory(newDirectoryName);
        refreshList();
    }
}

/**
  Question for Delete actually selected files
  */
void FileBrowserView::fileDelete()
{
    storeSelectedItemsOrCurrentItem();
    const QString messageFormat = "Delete %1 entries?";
    QString message = messageFormat.arg(mSelectionIndexes.count());
    HbMessageBox::question(message, this, SLOT(doFileDelete(HbAction*)));
}

/**
  Delete actually selected files
  */
void FileBrowserView::doFileDelete(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        //storeSelectedItemsOrCurrentItem();
        mEngineWrapper->deleteItems(mSelectionIndexes);
        refreshList();
    }
}

/**
  Open rename dialog for actually selected files
  */
void FileBrowserView::fileRename()
{
    storeSelectedItemsOrCurrentItem();
    mEngineWrapper->setCurrentSelection(mSelectionIndexes);

    for (int i(0), ie(mSelectionIndexes.count()); i < ie; ++i ) {
        mProceed = (i == ie-1); // if the last item
        mModelIndex = mSelectionIndexes.at(i);
        FileEntry entry = mEngineWrapper->getFileEntry(mModelIndex);

        QString heading = QString("Enter new name");
        HbInputDialog::getText(heading, this, SLOT(doFileRename(HbAction*)), entry.name(), scene());
    }
}

/**
  Rename actually selected files
  */
void FileBrowserView::doFileRename(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        mNewFileName = dlg->value().toString();

        if (mEngineWrapper->targetExists(mModelIndex, mNewFileName)) {
            const QString messageTemplate = QString("%1 already exists, overwrite?");
            QString message = messageTemplate.arg(mNewFileName);
            HbMessageBox::question(message, this, SLOT(doFileRenameFileExist(HbAction *)));
        } else {
            mEngineWrapper->rename(mModelIndex, mNewFileName);
            if (mProceed) {
                mEngineWrapper->startExecutingCommands(QString("Renaming"));
                refreshList();
            }
        }
    }
}

/**
  Rename actually selected files
  */
void FileBrowserView::doFileRenameFileExist(HbAction *action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        mEngineWrapper->rename(mModelIndex, mNewFileName);
        if (mProceed) {
            mEngineWrapper->startExecutingCommands(QString("Renaming"));
            refreshList();
        }
    }
}

/**
  Touch actually selected files
  */
void FileBrowserView::fileTouch()
{
    storeSelectedItemsOrCurrentItem();
    mEngineWrapper->setCurrentSelection(mSelectionIndexes);

    if (mEngineWrapper->selectionHasDirs()) {
        const QString message = "Recurse touch for all selected dirs?";
        HbMessageBox::question(message, this, SLOT(doFileTouch(HbAction*)));
    }
    else{
        mEngineWrapper->touch(false);
        refreshList();
    }
}

/**
  Touch actually selected files
  */
void FileBrowserView::doFileTouch(HbAction* action)
{
    bool recurse = false;
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        recurse = true;
        }
    mEngineWrapper->touch(recurse);
    refreshList();
}

void FileBrowserView::fileChecksumsMD5()
{
    fileChecksums(EFileChecksumsMD5);
}

void FileBrowserView::fileChecksumsMD2()
{
    fileChecksums(EFileChecksumsMD2);
}

void FileBrowserView::fileChecksumsSHA1()
{
    fileChecksums(EFileChecksumsSHA1);
}

void FileBrowserView::fileChecksums(TFileBrowserCmdFileChecksums checksumType)
{
//    QModelIndex currentIndex = currentItemIndex();
    mEngineWrapper->showFileCheckSums(mCurrentIndex, checksumType);
}

/**
  Show file properties
  */
void FileBrowserView::fileProperties()
{
    QModelIndex currentIndex = currentItemIndex();
    QStringList propertyList;
    QString titleText;
    mEngineWrapper->properties(currentIndex, propertyList, titleText);
    openPropertyDialog(propertyList, titleText);
}

void FileBrowserView::fileSetAttributes()
{

}

// edit menu
void FileBrowserView::editSnapShotToE()
{

}

/**
  Set selected files into clipboard.
  Selected item will be removed after paste operation.
  */
void FileBrowserView::editCut()
{
    storeSelectedItemsOrCurrentItem();
    mClipboardIndexes = mSelectionIndexes;

    mEngineWrapper->clipboardCut(mClipboardIndexes);

    int operations = mClipboardIndexes.count();
    const QString message = QString ("%1 entries cut to clipboard");
    QString noteMsg = message.arg(operations);

    Notifications::showInformationNote(noteMsg);
}

/**
  Set selected files into clipboard.
  Selected item will not be removed after paste operation.
  */
void FileBrowserView::editCopy()
{
    storeSelectedItemsOrCurrentItem();
    mClipboardIndexes = mSelectionIndexes;

    mEngineWrapper->clipboardCopy(mClipboardIndexes);

    int operations = mClipboardIndexes.count();

    const QString message = QString ("%1 entries copied to clipboard");
    QString noteMsg = message.arg(operations);

    Notifications::showInformationNote(noteMsg);
}

/**
  Moves or copies file selection stored in clipboard to a actual directory.
  Removing files depend on previous operation, i.e. Cut or Copy
  */
void FileBrowserView::editPaste()
{
    bool someEntryExists(false);

    // TODO Set entry items here

    someEntryExists = mEngineWrapper->isDestinationEntriesExists(mClipboardIndexes, mEngineWrapper->currentPath());
    if (someEntryExists) {
        fileOverwriteDialog();
    }
    mEngineWrapper->clipboardPaste(mOverwriteOptions);
}

/**
  Open copy to folder new filename dialog
  */
void FileBrowserView::editCopyToFolder()
{
    QString heading = QString("Enter new name");
    HbInputDialog::getText(heading, this, SLOT(doEditCopyToFolder(HbAction*)), mEngineWrapper->currentPath(), scene());
}

/**
  Copies current file selection to a queried directory.
  */
void FileBrowserView::doEditCopyToFolder(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString targetDir = dlg->value().toString();
        bool someEntryExists(false);

        // TODO Set entry items here
        storeSelectedItemsOrCurrentItem();
        mEngineWrapper->setCurrentSelection(mSelectionIndexes);

        someEntryExists = mEngineWrapper->isDestinationEntriesExists(mSelectionIndexes, targetDir);
        if (someEntryExists) {
            fileOverwriteDialog();
        }
        mEngineWrapper->copyToFolder(targetDir, mOverwriteOptions, false);
        refreshList();
    }
}

/**
  Open move to folder new filename dialog.
  */
void FileBrowserView::editMoveToFolder()
{
    QString heading = QString("Enter new name");
    HbInputDialog::getText(heading, this, SLOT(doEditCopyToFolder(HbAction*)), mEngineWrapper->currentPath(), scene());
}

/**
  Moves current file selection to a queried directory.
  */
void FileBrowserView::doEditMoveToFolder(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString targetDir = dlg->value().toString();
        bool someEntryExists(false);

        // TODO Set entry items here
        storeSelectedItemsOrCurrentItem();
        mEngineWrapper->setCurrentSelection(mSelectionIndexes);

        someEntryExists = mEngineWrapper->isDestinationEntriesExists(mSelectionIndexes, targetDir);
        if (someEntryExists) {
            fileOverwriteDialog();
        }
        mEngineWrapper->copyToFolder(targetDir, mOverwriteOptions, true);
        refreshList();
    }
}

/**
  Select current file
  */
void FileBrowserView::editSelect()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {
        selectionModel->select(selectionModel->currentIndex(), QItemSelectionModel::SelectCurrent);
        selectionModel->select(selectionModel->currentIndex(), QItemSelectionModel::Select);
        itemHighlighted(selectionModel->currentIndex());
        refreshList();
    }
}

/**
  Unselect current file
  */
void FileBrowserView::editUnselect()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {
        selectionModel->select(selectionModel->currentIndex(), QItemSelectionModel::Deselect);
        itemHighlighted(selectionModel->currentIndex());
    }
}

/**
  Select all files
  */
void FileBrowserView::editSelectAll()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {

        if (mFileBrowserModel->rowCount() > 0) {
            QModelIndex firstIndex = mFileBrowserModel->index(0, 0);
            QModelIndex lastIndex = mFileBrowserModel->index( (mFileBrowserModel->rowCount() - 1), 0);

            QItemSelection itemSelection(firstIndex, lastIndex);
            //selectionModel->select(itemSelection, QItemSelectionModel::SelectCurrent);
            selectionModel->select(itemSelection, QItemSelectionModel::Select);
        }
    }
}

/**
  Unselect all files
  */
void FileBrowserView::editUnselectAll()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {
        selectionModel->clearSelection();
    }
}

// ---------------------------------------------------------------------------
// view menu
// ---------------------------------------------------------------------------
void FileBrowserView::viewFilterEntries()
{

}
void FileBrowserView::viewRefresh()
{
    refreshList();
}

// ---------------------------------------------------------------------------
// disk admin menu
// ---------------------------------------------------------------------------

/**
  Open old password for the selected drive dialog.
  */
void FileBrowserView::diskAdminSetDrivePassword()
{
    QModelIndex currentIndex = currentItemIndex();
    // check if the drive has a password
    if (mEngineWrapper->hasDrivePassword(currentIndex)) {
        QString heading = QString("Existing password");
        HbInputDialog::getText(heading, this, SLOT(diskAdminSetDrivePasswordNew(HbAction*)), QString(), scene());
    }
}

/**
   Open new password for the selected drive dialog.
  */
void FileBrowserView::diskAdminSetDrivePasswordNew(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        mOldPassword = dlg->value().toString();
    }

    QString heading = QString("New password");
    HbInputDialog::getText(heading, this, SLOT(doDiskAdminSetDrivePassword(HbAction*)), mOldPassword, scene());
}

/**
   Set password for the selected drive.
  */
void FileBrowserView::doDiskAdminSetDrivePassword(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString newPassword = dlg->value().toString();
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminSetDrivePassword(currentIndex, mOldPassword, newPassword);
        refreshList();
    }
}

/**
  Open Unlock the selected drive dialog.
  */
void FileBrowserView::diskAdminUnlockDrive()
{
    QModelIndex currentIndex = currentItemIndex();
    // check if the drive is locked
    if (mEngineWrapper->isDriveLocked(currentIndex)) {
        QString heading = QString("Existing password");
        HbInputDialog::getText(heading, this, SLOT(doDiskAdminUnlockDrive(HbAction*)), QString(), scene());
    } else {
        Notifications::showInformationNote(QString("This drive is not locked"));
    }
}

/**
  Unlock the selected drive.
  */
void FileBrowserView::doDiskAdminUnlockDrive(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString oldPassword = dlg->value().toString();
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminUnlockDrive(currentIndex, oldPassword);
        refreshList();
    }
}

/**
  Open clear password of the selected drive dialog.
  */
void FileBrowserView::diskAdminClearDrivePassword()
{
    QModelIndex currentIndex = currentItemIndex();
    // check if the drive has a password
    if (mEngineWrapper->hasDrivePassword(currentIndex)) {
        QString heading = QString("Existing password");
        HbInputDialog::getText(heading, this, SLOT(doDiskAdminClearDrivePassword(HbAction*)), QString(), scene());
    } else {
        Notifications::showInformationNote(QString("This drive has no password"));
    }
}

/**
  Clear password of the selected drive.
  */
void FileBrowserView::doDiskAdminClearDrivePassword(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString oldPassword = dlg->value().toString();
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminClearDrivePassword(currentIndex, oldPassword);
        refreshList();
    }
}


/**
  Question for erase password of the selected drive
  */
void FileBrowserView::diskAdminEraseDrivePassword()
{
    // check if the drive has a password
    QModelIndex currentIndex = currentItemIndex();
    if (mEngineWrapper->hasDrivePassword(currentIndex)) {
        HbMessageBox::question(QString("Are you sure? All data can be lost!"), this, SLOT(doDiskAdminEraseDrivePassword(HbAction*)));
    } else {
        Notifications::showInformationNote(QString("This drive has no password"));
    }
}

/**
  Erase password of the selected drive
  */
void FileBrowserView::doDiskAdminEraseDrivePassword(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminEraseDrivePassword(currentIndex);
        refreshList();
    }
}

/**
  Performs format on the selected drive
  */
void FileBrowserView::diskAdminFormatDrive()
{
    HbMessageBox::question(QString("Are you sure? All data will be lost!"), this, SLOT(doDiskAdminFormatDrive(HbAction*)));
}

/**
  Performs format on the selected drive
  */
void FileBrowserView::doDiskAdminFormatDrive(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminFormatDrive(currentIndex, false);
    }
}

/**
  Performs quick format on the selected drive
  */
void FileBrowserView::diskAdminQuickFormatDrive()
{
    HbMessageBox::question(QString("Are you sure? All data will be lost!"), this, SLOT(doDiskAdminQuickFormatDrive(HbAction*)));
}

/**
  Performs quick format on the selected drive
  */
void FileBrowserView::doDiskAdminQuickFormatDrive(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminFormatDrive(currentIndex, true);
    }
}

/**
    Checks the disk integrity on the selected drive
  */
void FileBrowserView::diskAdminCheckDisk()
{
    QModelIndex currentIndex = currentItemIndex();
    mEngineWrapper->DiskAdminCheckDisk(currentIndex);
}

/**
  Checks the selected drive for errors and corrects them
  */
void FileBrowserView::diskAdminScanDrive()
{
    HbMessageBox::question(QString("This finds errors on disk and corrects them. Proceed?"), this, SLOT(doDiskAdminScanDrive(HbAction*)));
}

/**
  Checks the selected drive for errors and corrects them
  */
void FileBrowserView::doDiskAdminScanDrive(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminScanDrive(currentIndex);
        refreshList();
    }
}

/**
  Open drive name dialog
  */
void FileBrowserView::diskAdminSetDriveName()
{
    QString driveName;

    // get existing drive name
    QModelIndex currentIndex = currentItemIndex();
    mEngineWrapper->GetDriveName(currentIndex, driveName);

    QString heading = QString("New name");
    HbInputDialog::getText(heading, this, SLOT(doDiskAdminSetDriveName(HbAction*)), driveName, scene());
}

/**
  Set drive name.
  */
void FileBrowserView::doDiskAdminSetDriveName(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString driveName = dlg->value().toString();

        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminSetDriveName(currentIndex, driveName);

        refreshList();
    }
}

/**
  Open drive volume label dialog
  */
void FileBrowserView::diskAdminSetDriveVolumeLabel()
{
    QString volumeLabel;

    // get existing drive name
    QModelIndex currentIndex = currentItemIndex();
    mEngineWrapper->GetDriveVolumeLabel(currentIndex, volumeLabel);

    QString heading = QString("New volume label");
    HbInputDialog::getText(heading, this, SLOT(doDiskAdminSetDriveVolumeLabel(HbAction*)), volumeLabel, scene());
}

/**
  Set drive volume label.
  */
void FileBrowserView::doDiskAdminSetDriveVolumeLabel(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString volumeLabel = dlg->value().toString();

        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminSetDriveVolumeLabel(currentIndex, volumeLabel);

        refreshList();
    }
}

/**
  Eject the selected drive
  */
void FileBrowserView::diskAdminEjectDrive()
{
    QModelIndex currentIndex = currentItemIndex();
    mEngineWrapper->DiskAdminEjectDrive(currentIndex);
    refreshList();
}

/**
  Dismount the selected drive
  */
void FileBrowserView::diskAdminDismountDrive()
{
    HbMessageBox::question(QString("Are you sure you know what are you doing?"), this, SLOT(doDiskAdminDismountDrive(HbAction*)));
}

void FileBrowserView::doDiskAdminDismountDrive(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminDismountDrive(currentIndex);
        refreshList();
    }
}

/**
  Erase Master Boot Record of the selected drive
  */
void FileBrowserView::diskAdminEraseMBR()
{
    // TODO What to do with FB LITE macros?
    HbMessageBox::question(QString("Are you sure? Your media driver must support this!"), this, SLOT(doDiskAdminEraseMBR(HbAction*)));
}

void FileBrowserView::doDiskAdminEraseMBR(HbAction* action)
{
    // TODO What to do with FB LITE macros?
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        HbMessageBox::question(QString("Are you really sure you know what are you doing ?!?"), this, SLOT(doDiskAdminReallyEraseMBR(HbAction*)));
    }
}

void FileBrowserView::doDiskAdminReallyEraseMBR(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        // warn if the selected drive is not detected as removable
        if (mEngineWrapper->isDriveRemovable(currentIndex)) {
            mEngineWrapper->DiskAdminEraseMBR(currentIndex);
            refreshList();
        } else {
            HbMessageBox::question(QString("Selected drive is not removable, really continue?"), this, SLOT(doDiskAdminNotRemovableReallyEraseMBR(HbAction*)));
        }
    }
}

void FileBrowserView::doDiskAdminNotRemovableReallyEraseMBR(HbAction* action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->DiskAdminEraseMBR(currentIndex);
        refreshList();
    }

}

/**
  Partition the selected drive
  */
void FileBrowserView::diskAdminPartitionDrive()
{
    const QString message("Are you sure? Your media driver must support this!");
    HbMessageBox::question(message, this, SLOT(diskAdminPartitionDriveProceed(HbAction *)));
}

/**
  Partition the selected drive if user is sure
  */
void FileBrowserView::diskAdminPartitionDriveProceed(HbAction *action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        const QString message("Are you really sure you know what are you doing ?!?");
        HbMessageBox::question(message, this, SLOT(diskAdminPartitionDriveReallyProceed(HbAction *)));
    }
}

/**
  Partition the selected drive if user is really sure
  */
void FileBrowserView::diskAdminPartitionDriveReallyProceed(HbAction *action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEraseMBR = false;
        // warn if the selected drive is not detected as removable
        mProceed = false;
        if (mEngineWrapper->isDriveRemovable(currentIndex)) {
            mProceed = true;
        } else {
            const QString message("Selected drive is not removable, really continue?");
            HbMessageBox::question(message, this, SLOT(diskAdminPartitionDriveIsNotRemovable(HbAction *)));
        }

        if (mProceed) {
            // query if erase mbr
            mEraseMBR = false;

            QString message("Erase MBR first (normally needed)?");
            HbMessageBox::question(message, this, SLOT(diskAdminPartitionDriveEraseMbr(HbAction *)));

            // TODO use HbListDialog
            QStringList list;
            list << "1" << "2" << "3" << "4";
            openListDialog(list, QString("Partitions?"), this, SLOT(diskAdminPartitionDriveGetCount(HbAction*)));
        }
    }
}

/**
  Store result of user query about proceeding when drive is not removable.
  */
void FileBrowserView::diskAdminPartitionDriveIsNotRemovable(HbAction *action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        mProceed = true;
    } else {
        mProceed = false;
    }
}

/**
  Store result of user query about erase MBR
  */
void FileBrowserView::diskAdminPartitionDriveEraseMbr(HbAction *action)
{
    if (action && action->text().compare(QString("Yes"), Qt::CaseInsensitive) == 0) {
        mEraseMBR = true;
    }
}

/**
  Partition the selected drive
  */
void FileBrowserView::diskAdminPartitionDriveGetCount(HbAction* action)
{
//    Q_UNUSED(action);
    HbSelectionDialog *dlg = static_cast<HbSelectionDialog*>(sender());
    if(!action && dlg && dlg->selectedItems().count()){
        int selectionIndex = dlg->selectedItems().at(0).toInt();
        QModelIndex currentIndex = currentItemIndex();
        int amountOfPartitions = selectionIndex + 1;
        mEngineWrapper->DiskAdminPartitionDrive(currentIndex, mEraseMBR, amountOfPartitions);
        refreshList();
    }
}

// ---------------------------------------------------------------------------
// tools menu
// ---------------------------------------------------------------------------
void FileBrowserView::toolsAllAppsToTextFile()
{

}

/**
  Write all files to text file
  */
void FileBrowserView::toolsAllFilesToTextFile()
{
    mEngineWrapper->toolsWriteAllFiles();
}

void FileBrowserView::toolsAvkonIconCacheEnable()
{

}
void FileBrowserView::toolsAvkonIconCacheDisable()
{

}

/**
  Disable extended errors
  */
void FileBrowserView::toolsDisableExtendedErrors()
{
    mEngineWrapper->ToolsSetErrRd(false);
}

void FileBrowserView::toolsDumpMsgStoreWalk()
{

}
void FileBrowserView::toolsEditDataTypes()
{

}

/**
  Enable extended errors
  */
void FileBrowserView::toolsEnableExtendedErrors()
{
    mEngineWrapper->ToolsSetErrRd(true);
}

/**
  Open simulate leave dialog
  */
void FileBrowserView::toolsErrorSimulateLeave()
{
    int leaveCode = -6;
    QString heading = QString("Leave code");
    //HbInputDialog::getInteger(heading, this, SLOT(doToolsErrorSimulateLeave(HbAction*)), leaveCode, scene());
    HbInputDialog::getText(heading, this, SLOT(doToolsErrorSimulateLeave(HbAction*)), QString::number(leaveCode), scene());
}


/**
  Simulate leave.
  */
void FileBrowserView::doToolsErrorSimulateLeave(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        bool ok;
        int leaveCode = dlg->value().toString().toInt(&ok);
        if (leaveCode != 0 || ok) {
            mEngineWrapper->ToolsErrorSimulateLeave(leaveCode);
        }
    }
}

/**
  Open simulate panic dialog.
  */
void FileBrowserView::toolsErrorSimulatePanic()
{
    mPanicCategory = QString ("Test Category");
    QString heading = QString("Panic category");
    HbInputDialog::getText(heading, this, SLOT(doToolsErrorSimulatePanicCode(HbAction*)), mPanicCategory, scene());
}

/**
  Simulate panic.
  */
void FileBrowserView::doToolsErrorSimulatePanicCode(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        mPanicCategory = dlg->value().toString();
        int panicCode(555);
        QString heading = QString("Panic code");
        HbInputDialog::getInteger(heading, this, SLOT(doToolsErrorSimulatePanic(HbAction*)), panicCode, scene());
    }
}

/**
  Simulate panic.
  */
void FileBrowserView::doToolsErrorSimulatePanic(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        bool ok;
        int panicCode = dlg->value().toInt(&ok);
        if (panicCode != 0 || ok) {
            mEngineWrapper->ToolsErrorSimulatePanic(mPanicCategory, panicCode);
        }
    }
}

/**
  Open simulate exception dialog.
  */
void FileBrowserView::toolsErrorSimulateException()
{
    int exceptionCode = 0;
    QString heading = QString("Exception code");
    HbInputDialog::getInteger(heading, this, SLOT(doToolsErrorSimulateException(HbAction*)), exceptionCode, scene());
}

/**
  Simulate exception.
  */
void FileBrowserView::doToolsErrorSimulateException(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        bool ok;
        int exceptionCode = dlg->value().toInt(&ok);
        if (exceptionCode != 0 || ok) {
            mEngineWrapper->ToolsErrorSimulateException(exceptionCode);
        }
    }
}

//    void FileBrowserView::toolsLocalConnectivityActivateInfrared()
//{
//
//}
//    void FileBrowserView::toolsLocalConnectivityLaunchBTUI()
//{
//
//}
//    void FileBrowserView::toolsLocalConnectivityLaunchUSBUI()
//{
//
//}
void FileBrowserView::toolsMessageInbox()
{

}
void FileBrowserView::toolsMessageDrafts()
{

}
void FileBrowserView::toolsMessageSentItems()
{

}
void FileBrowserView::toolsMessageOutbox()
{

}
void FileBrowserView::toolsMemoryInfo()
{

}
void FileBrowserView::toolsSecureBackStart()
{

}
void FileBrowserView::toolsSecureBackRestore()
{

}
void FileBrowserView::toolsSecureBackStop()
{

}

/**
  Open debug mask dialog
  */
void FileBrowserView::toolsSetDebugMaskQuestion()
{
    quint32 dbgMask = mEngineWrapper->getDebugMask();
    QString dbgMaskText = QString("0x").append(QString::number(dbgMask, 16));
    QString heading = QString("Kernel debug mask in hex format");
    HbInputDialog::getText(heading, this, SLOT(toolsSetDebugMask(HbAction*)), dbgMaskText, scene());
}

/**
  Set debug mask
  */
void FileBrowserView::toolsSetDebugMask(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        QString dbgMaskText = dlg->value().toString();
        if (dbgMaskText.length() > 2 && dbgMaskText[0]=='0' && dbgMaskText[1]=='x') {
            bool ok;
            quint32 dbgMask = dbgMaskText.toUInt(&ok, 16);
            if (dbgMask != 0 || ok) {
                mEngineWrapper->toolsSetDebugMask(dbgMask);
                Notifications::showConfirmationNote(QString("Changed"));
            } else {
                Notifications::showErrorNote(QString("Cannot convert value"));
            }
        } else {
            Notifications::showErrorNote(QString("Not in hex format"));
        }
    }
}

void FileBrowserView::toolsShowOpenFilesHere()
{

}

// ---------------------------------------------------------------------------
// main menu items
// ---------------------------------------------------------------------------
void FileBrowserView::selectionModeChanged()
{
    if (mOptionMenuActions.mSelection->isChecked()) {
         activateSelectionMode();
     } else {
         deActivateSelectionMode();
     }
}

/**
  Show about note
  */
void FileBrowserView::about()
{
    Notifications::showAboutNote();
}

// ---------------------------------------------------------------------------
// End of operations
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/**
  Item is selected from list when selection mode is activated from menu
  */
void FileBrowserView::selectionChanged(const QItemSelection &/*selected*/, const QItemSelection &/*deselected*/)
{
    //QItemSelectionModel *selectionModel = mListView->selectionModel();
    //itemHighlighted(selectionModel->currentIndex());
}

/**
  An item is highlighted = single-clicked from file/folder list.
  */
void FileBrowserView::itemHighlighted(const QModelIndex& index)
{
    Q_UNUSED(index)
//    mItemHighlighted = true;
//    mFileInfo = mFileSystemModel->fileInfo(index);
}


/**
  An item is clicked from navigation item list. Navigation item list contains
  drive-, folder- or file items. Opens selected drive, folder or file popup menu
  */
void FileBrowserView::activated(const QModelIndex& index)
{
    if (mFileBrowserModel) {
        if (mEngineWrapper->isDriveListViewActive()) {
            mEngineWrapper->moveDownToDirectory(index);
            populateFolderContent();
        } else if (mEngineWrapper->getFileEntry(index).isDir()) {
            // populate new content of changed navigation view.
            // mLocationChanged = true;
            // mDirectory = filePath;
            mEngineWrapper->moveDownToDirectory(index);
            populateFolderContent();
        } else {  // file item
            // mSelectedFilePath = filePath;
            FileEntry fileEntry = mEngineWrapper->getFileEntry(index);
            mAbsoluteFilePath = fileEntry.path() + fileEntry.name();

            // open user-dialog to select: view as text/hex,  open w/AppArc or open w/DocH. embed
            QStringList list;
            list << QString("View as text/hex") << QString("Open w/ AppArc") << QString("Open w/ DocH. embed");
            openListDialog(list, QString("Open file"), this, SLOT(fileOpen(HbAction *)));
        }
    }
}

// ---------------------------------------------------------------------------

void FileBrowserView::activateSelectionMode()
{
    QString path;
    disconnect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
    mListView->setSelectionMode(HbListView::MultiSelection);

//    if (mDirectory != mInitDirPath.path()) {
//        QDir dir(mDirectory);
//        path = mDirectory;
//        QStringList dirs = dir.entryList(QDir::AllDirs | QDir::System | QDir::Hidden);
//        if (dirs.count() > 0) {
//            path.append("\\" + dirs.at(0) + "\\");
//        }
//    } else {
//        path = ("C:\\");
//    }
//    QModelIndex index = mFileSystemModel->index(path,0);
//    mListView->selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
//    mListView->selectionModel()->select(index, QItemSelectionModel::Select);
//    //mListView->setHighlightMode(HbItemHighlight::HighlightAlwaysVisible);
//    mListView->setFocus();    // TODO use focus in
    if (mListView->selectionModel()) {
        connect(mListView->selectionModel(),
                SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
                this,
                SLOT(selectionChanged(QItemSelection, QItemSelection)));
//        // flag to indicate that selection mode changed, "edit" sub-menu update needed
//        mFolderContentChanged = true;
    }
}

// ---------------------------------------------------------------------------

void FileBrowserView::deActivateSelectionMode()
{
    disconnect(mListView->selectionModel(),
               SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
               this,
               SLOT(selectionChanged(QItemSelection, QItemSelection)));
    mListView->setSelectionMode(HbListView::NoSelection);
    connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
    editUnselectAll();
    // flag to indicate that selection mode changed, "edit" sub-menu update needed
    mFolderContentChanged = true;
}

// ---------------------------------------------------------------------------
