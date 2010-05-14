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
    mDirectory(),
    mSelectedFilePath(),
    mFileBrowserModel(0),
    mFileViewMenuActions(),
    mToolbarBackAction(0),
    mSearch(0),
    mSettingsView(0),
    mItemHighlighted(false),
    mLocationChanged(false),
    mRemoveFileAfterCopied(false),
    mClipBoardInUse(false),
    mFolderContentChanged(false),
    mOldPassword(),
    mPanicCategory(),
    mAbsoluteFilePath(),
    mOverwriteOptions(),
    mIsRenameAllowed(true),
    mProceed(false),
    mEraseMBR(false)
{
    setTitle("File Browser");

    createMenu();
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
    //mListView->setHighlightMode(HbItemHighlight::HighlightAlwaysVisible);

    connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));

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
//    if (mSearch !=0) {
//        delete mSearch;
//    }
//    if (mSettingsView != 0) {
//        delete mSettingsView;
//    }
//    if (mEngineWrapper) {
//        delete mEngineWrapper;
//    }
    delete mFileBrowserModel;
    delete mListView;
    delete mToolBar;
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

    if (mFileViewMenuActions.mSelection) {
        mToolBar->addAction(mFileViewMenuActions.mSelection);
    }

    setToolBar(mToolBar);
}

/**
  Initial setup for options menu.
  Dynamic menu update during the runtime is performed by updateMenu() which
  to menu's aboutToShow() signal.
  */
void FileBrowserView::createMenu()
{
    createFileMenu();
    createEditMenu();
    createViewMenu();
    createDiskAdminMenu();
    createToolsMenu();

    createSelectionMenuItem();
    createSettingsMenuItem();
    createAboutMenuItem();
    createExitMenuItem();

    // menu dynamic update
    connect(menu(), SIGNAL(aboutToShow()), this, SLOT(updateMenu()));
}

/**
  Initial setup for File submenu
  */
void FileBrowserView::createFileMenu()
{
    mFileViewMenuActions.mFileMenu = menu()->addMenu("File");

    mFileViewMenuActions.mFileBackMoveUp = mFileViewMenuActions.mFileMenu->addAction("Back/Move up (<-)", this, SLOT(fileBackMoveUp()));
    mFileViewMenuActions.mFileOpenDrive = mFileViewMenuActions.mFileMenu->addAction("Open drive (->)", this, SLOT(fileOpenDrive()));
    mFileViewMenuActions.mFileOpenDirectory = mFileViewMenuActions.mFileMenu->addAction("Open directory (->)", this, SLOT(fileOpenDirectory()));
    mFileViewMenuActions.mFileSearch = mFileViewMenuActions.mFileMenu->addAction("Search", this, SLOT(fileSearch()));
    mFileViewMenuActions.mFileSearch->setVisible(false);

    mFileViewMenuActions.mFileNewMenu = mFileViewMenuActions.mFileMenu->addMenu("New");
    mFileViewMenuActions.mFileNewFile = mFileViewMenuActions.mFileNewMenu->addAction("File", this, SLOT(fileNewFile()));
    mFileViewMenuActions.mFileNewDirectory = mFileViewMenuActions.mFileNewMenu->addAction("Directory", this, SLOT(fileNewDirectory()));

    mFileViewMenuActions.mFileDelete = mFileViewMenuActions.mFileMenu->addAction("Delete", this, SLOT(fileDelete()));
    mFileViewMenuActions.mFileRename = mFileViewMenuActions.mFileMenu->addAction("Rename", this, SLOT(fileRename()));
    mFileViewMenuActions.mFileTouch = mFileViewMenuActions.mFileMenu->addAction("Touch", this, SLOT(fileTouch()));
    mFileViewMenuActions.mFileProperties = mFileViewMenuActions.mFileMenu->addAction("Properties", this, SLOT(fileProperties()));

    mFileViewMenuActions.mFileChecksumsMenu = mFileViewMenuActions.mFileMenu->addMenu("Checksums");
    mFileViewMenuActions.mFileChecksumsMD5 = mFileViewMenuActions.mFileChecksumsMenu->addAction("MD5", this, SLOT(fileChecksumsMD5()));
    mFileViewMenuActions.mFileChecksumsMD2 = mFileViewMenuActions.mFileChecksumsMenu->addAction("MD2", this, SLOT(fileChecksumsMD2()));
    mFileViewMenuActions.mFileChecksumsSHA1 = mFileViewMenuActions.mFileChecksumsMenu->addAction("SHA-1", this, SLOT(fileChecksumsSHA1()));

    mFileViewMenuActions.mFileSetAttributes = mFileViewMenuActions.mFileMenu->addAction("Set attributes...", this, SLOT(fileSetAttributes()));
    mFileViewMenuActions.mFileSetAttributes->setVisible(false);
}

/**
  Initial setup for Edit submenu
  */
void FileBrowserView::createEditMenu()
{
    mFileViewMenuActions.mEditMenu = menu()->addMenu("Edit");

    mFileViewMenuActions.mEditSnapShotToE = mFileViewMenuActions.mEditMenu->addAction("Snap shot to E:", this, SLOT(editSnapShotToE()));
    mFileViewMenuActions.mEditSnapShotToE->setVisible(false);
    mFileViewMenuActions.mEditCut = mFileViewMenuActions.mEditMenu->addAction("Cut", this, SLOT(editCut()));
    mFileViewMenuActions.mEditCopy = mFileViewMenuActions.mEditMenu->addAction("Copy", this, SLOT(editCopy()));
    mFileViewMenuActions.mEditPaste = mFileViewMenuActions.mEditMenu->addAction("Paste", this, SLOT(editPaste()));

    mFileViewMenuActions.mEditCopyToFolder = mFileViewMenuActions.mEditMenu->addAction("Copy to folder...", this, SLOT(editCopyToFolder()));
    mFileViewMenuActions.mEditMoveToFolder = mFileViewMenuActions.mEditMenu->addAction("Move to folder...", this, SLOT(editMoveToFolder()));

    mFileViewMenuActions.mEditSelect = mFileViewMenuActions.mEditMenu->addAction("Select", this, SLOT(editSelect()));
    mFileViewMenuActions.mEditUnselect = mFileViewMenuActions.mEditMenu->addAction("Unselect", this, SLOT(editUnselect()));
    mFileViewMenuActions.mEditSelectAll = mFileViewMenuActions.mEditMenu->addAction("Select all", this, SLOT(editSelectAll()));
    mFileViewMenuActions.mEditUnselectAll = mFileViewMenuActions.mEditMenu->addAction("Unselect all", this, SLOT(editUnselectAll()));
}

/**
  Initial setup for View submenu
  */
void FileBrowserView::createViewMenu()
{
    mFileViewMenuActions.mViewMenu = menu()->addMenu("View");
    mFileViewMenuActions.mViewMenu->menuAction()->setVisible(false);

    mFileViewMenuActions.mViewFilterEntries = mFileViewMenuActions.mViewMenu->addAction("Filter entries", this, SLOT(viewFilterEntries()));
    mFileViewMenuActions.mViewRefresh = mFileViewMenuActions.mViewMenu->addAction("Refresh", this, SLOT(viewRefresh()));
}

/**
  Initial setup for Disk Admin submenu
  */
void FileBrowserView::createDiskAdminMenu()
{
    mFileViewMenuActions.mDiskAdminMenu = menu()->addMenu("Disk admin");
    mFileViewMenuActions.mDiskAdminMenu->menuAction()->setVisible(false);

    mFileViewMenuActions.mDiskAdminSetDrivePassword = mFileViewMenuActions.mDiskAdminMenu->addAction("Set drive password", this, SLOT(diskAdminSetDrivePassword()));
    mFileViewMenuActions.mDiskAdminUnlockDrive = mFileViewMenuActions.mDiskAdminMenu->addAction("Unlock drive", this, SLOT(diskAdminUnlockDrive()));
    mFileViewMenuActions.mDiskAdminClearDrivePassword = mFileViewMenuActions.mDiskAdminMenu->addAction("Clear drive password", this, SLOT(diskAdminClearDrivePassword()));
    mFileViewMenuActions.mDiskAdminEraseDrivePassword = mFileViewMenuActions.mDiskAdminMenu->addAction("Erase drive password", this, SLOT(diskAdminEraseDrivePassword()));
    mFileViewMenuActions.mDiskAdminFormatDrive = mFileViewMenuActions.mDiskAdminMenu->addAction("Format drive", this, SLOT(diskAdminFormatDrive()));
    mFileViewMenuActions.mDiskAdminQuickFormatDrive = mFileViewMenuActions.mDiskAdminMenu->addAction("Quick format drive", this, SLOT(diskAdminQuickFormatDrive()));
    mFileViewMenuActions.mDiskAdminCheckDisk = mFileViewMenuActions.mDiskAdminMenu->addAction("Check disk", this, SLOT(diskAdminCheckDisk()));
    mFileViewMenuActions.mDiskAdminScanDrive = mFileViewMenuActions.mDiskAdminMenu->addAction("Scan drive", this, SLOT(diskAdminScanDrive()));
    mFileViewMenuActions.mDiskAdminSetDriveName = mFileViewMenuActions.mDiskAdminMenu->addAction("Set drive name", this, SLOT(diskAdminSetDriveName()));
    mFileViewMenuActions.mDiskAdminSetDriveVolumeLabel = mFileViewMenuActions.mDiskAdminMenu->addAction("Set drive volume label", this, SLOT(diskAdminSetDriveVolumeLabel()));
    mFileViewMenuActions.mDiskAdminEjectDrive = mFileViewMenuActions.mDiskAdminMenu->addAction("Eject drive", this, SLOT(diskAdminEjectDrive()));
    mFileViewMenuActions.mDiskAdminDismountDrive = mFileViewMenuActions.mDiskAdminMenu->addAction("Dismount drive", this, SLOT(diskAdminDismountDrive()));
    mFileViewMenuActions.mDiskAdminEraseMBR = mFileViewMenuActions.mDiskAdminMenu->addAction("Erase MBR", this, SLOT(diskAdminEraseMBR()));
    mFileViewMenuActions.mDiskAdminPartitionDrive = mFileViewMenuActions.mDiskAdminMenu->addAction("Partition drive", this, SLOT(diskAdminPartitionDrive()));
}

/**
  Initial setup for Tools submenu
  */
void FileBrowserView::createToolsMenu()
{
    mFileViewMenuActions.mToolsMenu = menu()->addMenu("Tools");

    mFileViewMenuActions.mToolsAllAppsToTextFile = mFileViewMenuActions.mToolsMenu->addAction("All apps to a text file", this, SLOT(toolsAllAppsToTextFile()));
    mFileViewMenuActions.mToolsAllAppsToTextFile->setVisible(false);
    mFileViewMenuActions.mToolsAllFilesToTextFile = mFileViewMenuActions.mToolsMenu->addAction("All files to a text file", this, SLOT(toolsAllFilesToTextFile()));
    mFileViewMenuActions.mToolsAllFilesToTextFile->setVisible(false);

    mFileViewMenuActions.mToolsAvkonIconCacheMenu = mFileViewMenuActions.mToolsMenu->addMenu("Avkon icon cache");
    mFileViewMenuActions.mToolsAvkonIconCacheMenu->menuAction()->setVisible(false);
    mFileViewMenuActions.mToolsAvkonIconCacheEnable = mFileViewMenuActions.mToolsAvkonIconCacheMenu->addAction("Enable", this, SLOT(toolsAvkonIconCacheEnable()));
    mFileViewMenuActions.mToolsAvkonIconCacheDisable = mFileViewMenuActions.mToolsAvkonIconCacheMenu->addAction("Clear and disable", this, SLOT(toolsAvkonIconCacheDisable()));

    mFileViewMenuActions.mToolsDisableExtendedErrors = mFileViewMenuActions.mToolsMenu->addAction("Disable extended errors", this, SLOT(toolsDisableExtendedErrors()));
    mFileViewMenuActions.mToolsDumpMsgStoreWalk = mFileViewMenuActions.mToolsMenu->addAction("Dump msg. store walk", this, SLOT(toolsDumpMsgStoreWalk()));
    mFileViewMenuActions.mToolsDumpMsgStoreWalk->setVisible(false);
    mFileViewMenuActions.mToolsEditDataTypes = mFileViewMenuActions.mToolsMenu->addAction("Edit data types", this, SLOT(toolsEditDataTypes()));
    mFileViewMenuActions.mToolsEditDataTypes->setVisible(false);
    mFileViewMenuActions.mToolsEnableExtendedErrors = mFileViewMenuActions.mToolsMenu->addAction("Enable extended errors", this, SLOT(toolsEnableExtendedErrors()));

    mFileViewMenuActions.mToolsErrorSimulateMenu = mFileViewMenuActions.mToolsMenu->addMenu("Error simulate");
    mFileViewMenuActions.mToolsErrorSimulateLeave = mFileViewMenuActions.mToolsErrorSimulateMenu->addAction("Leave", this, SLOT(toolsErrorSimulateLeave()));
    mFileViewMenuActions.mToolsErrorSimulatePanic = mFileViewMenuActions.mToolsErrorSimulateMenu->addAction("Panic", this, SLOT(toolsErrorSimulatePanic()));
    mFileViewMenuActions.mToolsErrorSimulatePanic->setVisible(false);
    mFileViewMenuActions.mToolsErrorSimulateException = mFileViewMenuActions.mToolsErrorSimulateMenu->addAction("Exception", this, SLOT(toolsErrorSimulateException()));

//    mFileViewMenuActions.mLocalConnectivityMenu = mFileViewMenuActions.mToolsMenu->addMenu("Local connectivity");
//    mFileViewMenuActions.mToolsLocalConnectivityActivateInfrared = mFileViewMenuActions.mLocalConnectivityMenu->addAction("Activate infrared", this, SLOT(toolsLocalConnectivityActivateInfrared()));
//    mFileViewMenuActions.mToolsLocalConnectivityLaunchBTUI = mFileViewMenuActions.mLocalConnectivityMenu->addAction("Launch BT UI", this, SLOT(toolsLocalConnectivityLaunchBTUI()));
//    mFileViewMenuActions.mToolsLocalConnectivityLaunchUSBUI = mFileViewMenuActions.mLocalConnectivityMenu->addAction("Launch USB UI", this, SLOT(toolsLocalConnectivityLaunchUSBUI()));

    mFileViewMenuActions.mToolsMessageAttachmentsMenu = mFileViewMenuActions.mToolsMenu->addMenu("Message attachments");
    mFileViewMenuActions.mToolsMessageAttachmentsMenu->menuAction()->setVisible(false);
    mFileViewMenuActions.mToolsMessageInbox = mFileViewMenuActions.mToolsMessageAttachmentsMenu->addAction("Inbox", this, SLOT(toolsMessageInbox()));
    mFileViewMenuActions.mToolsMessageDrafts = mFileViewMenuActions.mToolsMessageAttachmentsMenu->addAction("Drafts", this, SLOT(toolsMessageDrafts()));
    mFileViewMenuActions.mToolsMessageSentItems = mFileViewMenuActions.mToolsMessageAttachmentsMenu->addAction("Sent items", this, SLOT(toolsMessageSentItems()));
    mFileViewMenuActions.mToolsMessageOutbox = mFileViewMenuActions.mToolsMessageAttachmentsMenu->addAction("Outbox", this, SLOT(toolsMessageOutbox()));

    mFileViewMenuActions.mToolsMemoryInfo = mFileViewMenuActions.mToolsMenu->addAction("Memory info", this, SLOT(toolsMemoryInfo()));
    mFileViewMenuActions.mToolsMemoryInfo->setVisible(false);

    mFileViewMenuActions.mToolsSecureBackupMenu = mFileViewMenuActions.mToolsMenu->addMenu("Secure backup");
    mFileViewMenuActions.mToolsSecureBackupMenu->menuAction()->setVisible(false);
    mFileViewMenuActions.mToolsSecureBackStart = mFileViewMenuActions.mToolsSecureBackupMenu->addAction("Start backup", this, SLOT(toolsSecureBackStart()));
    mFileViewMenuActions.mToolsSecureBackRestore = mFileViewMenuActions.mToolsSecureBackupMenu->addAction("Start restore", this, SLOT(toolsSecureBackRestore()));
    mFileViewMenuActions.mToolsSecureBackStop = mFileViewMenuActions.mToolsSecureBackupMenu->addAction("Stop", this, SLOT(toolsSecureBackStop()));

    mFileViewMenuActions.mToolsSetDebugMask = mFileViewMenuActions.mToolsMenu->addAction("Set debug mask", this, SLOT(toolsSetDebugMaskQuestion()));
    mFileViewMenuActions.mToolsShowOpenFilesHere = mFileViewMenuActions.mToolsMenu->addAction("Show open files here", this, SLOT(toolsShowOpenFilesHere()));
    mFileViewMenuActions.mToolsShowOpenFilesHere->setVisible(false);
}

/**
  Creates Selection mode menu item in option menu
  */
void FileBrowserView::createSelectionMenuItem()
{
    if (!mFileViewMenuActions.mSelection) {
        mFileViewMenuActions.mSelection = menu()->addAction("Selection mode");
        mFileViewMenuActions.mSelection->setToolTip("Selection mode");
        mFileViewMenuActions.mSelection->setCheckable(true);
        connect(mFileViewMenuActions.mSelection, SIGNAL(triggered()), this, SLOT(selectionModeChanged()));
    }
}

/**
  Creates Setting menu item in option menu
  */
void FileBrowserView::createSettingsMenuItem()
{
    mFileViewMenuActions.mSetting = menu()->addAction("Settings");
    connect(mFileViewMenuActions.mSetting, SIGNAL(triggered()), this, SIGNAL(aboutToShowSettingsView()));
}


/**
  Creates About menu item in option menu
  */
void FileBrowserView::createAboutMenuItem()
{
    // about note
    mFileViewMenuActions.mAbout = menu()->addAction("About");
    connect(mFileViewMenuActions.mAbout, SIGNAL(triggered()), this, SLOT(about()));
}

/**
  Creates Exit menu item in option menu
  */
void FileBrowserView::createExitMenuItem()
{
    // application exit
    mFileViewMenuActions.mExit = menu()->addAction("Exit");
    connect(mFileViewMenuActions.mExit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

/**
  update menu: disk admin available only in device root view. edit available only in folder view
  when file or folder content exist in current folder, or clipboard has copied item.
  file and view menus updated every time regarding the folder content.
  tools, settings, about, exit always available.
  If there's remove and add operations at same time, always remove first
  to keep to the correct menu items order.
  */
void FileBrowserView::updateMenu()
{
    bool emptyListBox = mFileBrowserModel->rowCount() == 0;           //iContainer->ListBoxNumberOfVisibleItems() == 0;
    bool driveListActive = mEngineWrapper->isDriveListViewActive(); //iModel->FileUtils()->IsDriveListViewActive();
    bool normalModeActive = true;       //iModel->FileUtils()->IsNormalModeActive();
    bool currentDriveReadOnly = mEngineWrapper->isCurrentDriveReadOnly();   //iModel->FileUtils()->IsCurrentDriveReadOnly();
    bool currentItemDirectory = mEngineWrapper->getFileEntry(currentItemIndex()).isDir();   //iModel->FileUtils()->IsCurrentItemDirectory();
    bool listBoxSelections = mListView->selectionModel()->selection().count() == 0;      //iContainer->ListBoxSelectionIndexesCount() == 0;
    bool emptyClipBoard = !mEngineWrapper->isClipBoardListInUse();
    bool showSnapShot = false;           //iModel->FileUtils()->DriveSnapShotPossible();

    bool showEditMenu(true);
    if (driveListActive) {
        if (!showSnapShot || emptyListBox && emptyClipBoard)
            showEditMenu = false;
        else
            showEditMenu = true;
    } else {
        if (emptyListBox && emptyClipBoard)
            showEditMenu = false;
        else
            showEditMenu = true;
    }

    mFileViewMenuActions.mEditMenu->menuAction()->setVisible(showEditMenu);
    // TODO mFileViewMenuActions.mDiskAdminMenu->menuAction()->setVisible(driveListActive);

    mFileViewMenuActions.mFileBackMoveUp->setVisible( !driveListActive);

    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileOpen, emptyListBox || driveListActive || currentItemDirectory);
    mFileViewMenuActions.mFileOpenDrive->setVisible( !(emptyListBox || !driveListActive));
    mFileViewMenuActions.mFileOpenDirectory->setVisible( !(emptyListBox || driveListActive || !currentItemDirectory));

    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileView, emptyListBox || listBoxSelections || currentItemDirectory || driveListActive);
    //aMenuPane->SetItemDimmed(EFileBrowserCmd FileEdit, emptyListBox || listBoxSelections || currentItemDirectory || driveListActive);
    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileSendTo, emptyListBox || driveListActive || currentItemDirectory);

    mFileViewMenuActions.mFileNewMenu->menuAction()->setVisible(!(driveListActive || currentDriveReadOnly));
    mFileViewMenuActions.mFileDelete->setVisible(!(emptyListBox || driveListActive || currentDriveReadOnly));
    mFileViewMenuActions.mFileRename->setVisible(!(emptyListBox || driveListActive || currentDriveReadOnly || listBoxSelections));
    mFileViewMenuActions.mFileTouch->setVisible(!(emptyListBox || driveListActive || currentDriveReadOnly));
    mFileViewMenuActions.mFileProperties->setVisible(!(emptyListBox || listBoxSelections));
    // TODO mFileViewMenuActions.mFileChecksums->setVisible(!(emptyListBox || listBoxSelections || currentItemDirectory || driveListActive));
    // TODO mFileViewMenuActions.mFileSetAttributes->setVisible(!(emptyListBox || driveListActive || currentDriveReadOnly));
    // TODO mFileViewMenuActions.mFileCompress->setVisible(!(currentDriveReadOnly || emptyListBox || listBoxSelections || currentItemDirectory || driveListActive));
    // TODO mFileViewMenuActions.mFileDecompress->setVisible(!(currentDriveReadOnly || emptyListBox || listBoxSelections || currentItemDirectory || driveListActive));

    bool currentSelected = true;    //iContainer->ListBox()->View()->ItemIsSelected(iContainer->ListBox()->View()->CurrentItemIndex());
    bool allSelected = mListView->selectionModel()->selection().count() == mFileBrowserModel->rowCount();        //iContainer->ListBox()->SelectionIndexes()->Count() == iContainer->ListBox()->Model()->NumberOfItems();
    bool noneSelected = mListView->selectionModel()->selection().count() != 0;       //iContainer->ListBox()->SelectionIndexes()->Count() == 0;

    //mFileViewMenuActions.mEditSnapShotToE->setVisible(driveListActive); // TODO
    mFileViewMenuActions.mEditCut->setVisible(!(driveListActive || currentDriveReadOnly || emptyListBox));
    mFileViewMenuActions.mEditCopy->setVisible(!(driveListActive || emptyListBox));
    mFileViewMenuActions.mEditPaste->setVisible(!(driveListActive || emptyClipBoard || currentDriveReadOnly));
    mFileViewMenuActions.mEditCopyToFolder->setVisible(!(driveListActive || emptyListBox));
    mFileViewMenuActions.mEditMoveToFolder->setVisible(!(driveListActive || currentDriveReadOnly || emptyListBox));
    mFileViewMenuActions.mEditSelect->setVisible(!(driveListActive || currentSelected || emptyListBox));
    mFileViewMenuActions.mEditUnselect->setVisible(!(driveListActive || !currentSelected || emptyListBox));
    mFileViewMenuActions.mEditSelectAll->setVisible(!(driveListActive || allSelected || emptyListBox));
    mFileViewMenuActions.mEditUnselectAll->setVisible(!(driveListActive || noneSelected || emptyListBox));

    // TODO mFileViewMenuActions.mViewSort->setVisible(!(!normalModeActive || driveListActive || emptyListBox));
    // TODO mFileViewMenuActions.mViewOrder->setVisible(!(!normalModeActive || driveListActive || emptyListBox));
    mFileViewMenuActions.mViewRefresh->setVisible(normalModeActive);
    mFileViewMenuActions.mViewFilterEntries->setVisible(!emptyListBox);

    // TODO R_FILEBROWSER_VIEW_SORT_SUBMENU
    // aMenuPane->SetItemButtonState(iModel->FileUtils()->SortMode(), EEikMenuItemSymbolOn);

    // TODO R_FILEBROWSER_VIEW_ORDER_SUBMENU
    // aMenuPane->SetItemButtonState(iModel->FileUtils()->OrderMode(), EEikMenuItemSymbolOn);

    // aResourceId == R_FILEBROWSER_TOOLS_SUBMENU
    bool noExtendedErrorsAllowed = mEngineWrapper->ErrRdFileExists();
    mFileViewMenuActions.mToolsDisableExtendedErrors->setVisible(noExtendedErrorsAllowed);
    mFileViewMenuActions.mToolsEnableExtendedErrors->setVisible(!noExtendedErrorsAllowed);

//    bool infraRedAllowed = mEngineWrapper->FileExists(KIRAppPath);
//    bool bluetoothAllowed = mEngineWrapper->FileExists(KBTAppPath);
//    bool usbAllowed = mEngineWrapper->FileExists(KUSBAppPath);
//
//    bool noLocalCon = !infraRedAllowed && !bluetoothAllowed && !usbAllowed;
//    mFileViewMenuActions.mToolsLocalConnectivityMenu->menuAction()->setVisible(!noLocalCon);
//
//    mFileViewMenuActions.mToolsLocalConnectivityActivateInfrared->setVisible(infraRedAllowed);
//    mFileViewMenuActions.mToolsLocalConnectivityLaunchBTUI->setVisible(bluetoothAllowed);
//    mFileViewMenuActions.mToolsLocalConnectivityLaunchUSBUI->setVisible(usbAllowed);
}

/**
  Refresh FileBrowser view
  */
void FileBrowserView::refreshList()
{
    mEngineWrapper->refreshView();
    mNaviPane->setPlainText(QString(mEngineWrapper->currentPath()));
    mListView->reset();
    mListView->setModel(mFileBrowserModel);
    mToolbarBackAction->setEnabled(!mEngineWrapper->isDriveListViewActive());
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
    HbSelectionDialog *dlg = static_cast<HbSelectionDialog*>(sender());
    if(!action && dlg && dlg->selectedModelIndexes().count()){
        int selectionIndex = dlg->selectedModelIndexes().at(0).row();

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
    HbSelectionDialog *dlg = static_cast<HbSelectionDialog*>(sender());
    if(!action && dlg && dlg->selectedModelIndexes().count()) {
        mOverwriteOptions.queryIndex = dlg->selectedModelIndexes().at(0).row();
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
    if (action == dlg->primaryAction()) {
        mOverwriteOptions.postFix = dlg->value().toString();
    } else {
        mOverwriteOptions.doFileOperations = false;
    }
}

// ---------------------------------------------------------------------------
/**
  Show a list dialog
  \param List aList of item to select item from.
  \param Title text aTitleText of a dialog heading widget
  \return None
  */
void FileBrowserView::openListDialog(const QStringList& items, const QString &aTitleText, QObject* receiver, const char* member)
{
    // Create a list and some simple content for it
    HbSelectionDialog *dlg = new HbSelectionDialog();
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    // Set items to be popup's content
    dlg->setStringItems(items);
    dlg->setSelectionMode(HbAbstractItemView::SingleSelection);
    //dlg->setDismissPolicy(HbPopup::TapOutside);
    
    HbLabel *title = new HbLabel(dlg);
    title->setPlainText(aTitleText);
    dlg->setHeadingWidget(title);

    // Launch popup and handle the user response:
    dlg->open(receiver, member);
}

// ---------------------------------------------------------------------------

HbDialog *FileBrowserView::filePathQuery(const QString &headingText,
                                         const QString &text,
                                         const QString &primaryActionText,
                                         const QString &secondaryActionText)
{
    HbDialog *dialog = new HbDialog();
    dialog->setDismissPolicy(HbPopup::TapOutside);
    dialog->setTimeout(HbPopup::NoTimeout);
    HbLineEdit *edit = new HbLineEdit();
    HbAction *primaryAction = new HbAction(primaryActionText);
    HbAction *secondaryAction = new HbAction(secondaryActionText);
    // connect signal to close pop-up if cancel selected:
    connect(secondaryAction, SIGNAL(triggered()), dialog, SLOT(close()));
    edit->setText(text);
    dialog->setHeadingWidget(new HbLabel(headingText));
    //popup->setHeadingWidget(dlgTitle);
    dialog->setContentWidget(edit);
    dialog->setPrimaryAction(primaryAction);
    dialog->setSecondaryAction(secondaryAction);

    return dialog;
}

// ---------------------------------------------------------------------------

//HbDialog *FileBrowserView::openTextQuery(const QString &headingText,
//                                         const QString &text,
//                                         const QString &primaryActionText,
//                                         const QString &secondaryActionText)
//{
//    HbDialog *dialog = new HbDialog();
//    dialog->setDismissPolicy(HbPopup::TapOutside);
//    dialog->setTimeout(HbPopup::NoTimeout);
//    HbLineEdit *edit = new HbLineEdit();
//    HbAction *primaryAction = new HbAction(primaryActionText);
//    HbAction *secondaryAction = new HbAction(secondaryActionText);
//    // connect signal to close pop-up if cancel selected:
//    connect(secondaryAction, SIGNAL(triggered()), dialog, SLOT(close()));
//    edit->setText(text);
//    dialog->setHeadingWidget(new HbLabel(headingText));
//    //popup->setHeadingWidget(dlgTitle);
//    dialog->setContentWidget(edit);
//    dialog->setPrimaryAction(primaryAction);
//    dialog->setSecondaryAction(secondaryAction);
//
//    return dialog;
//}

// ---------------------------------------------------------------------------

HbDialog *FileBrowserView::openNumberQuery(const QString &headingText,
                                           const QString &text,
                                           const QString &primaryActionText,
                                           const QString &secondaryActionText,
                                           int aMin/* = -99999*/,
                                           int aMax/* = 99999*/)
{
    HbDialog *dialog = new HbDialog();
    dialog->setDismissPolicy(HbPopup::TapOutside);
    dialog->setTimeout(HbPopup::NoTimeout);
    HbAction *primaryAction = new HbAction(primaryActionText);
    HbAction *secondaryAction = new HbAction(secondaryActionText);
    // connect signal to close pop-up if cancel selected:
    connect(secondaryAction, SIGNAL(triggered()), dialog, SLOT(close()));

    HbLineEdit *edit = new HbLineEdit();
    HbValidator *validator = new HbValidator();
    validator->addField(new QIntValidator(aMin, aMax, 0), text);
    edit->setValidator(validator);
    edit->setInputMethodHints(Qt::ImhDigitsOnly);

    //edit->setText(text);
    dialog->setHeadingWidget(new HbLabel(headingText));
    //popup->setHeadingWidget(dlgTitle);
    dialog->setContentWidget(edit);
    dialog->setPrimaryAction(primaryAction);
    dialog->setSecondaryAction(secondaryAction);

    return dialog;
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

QModelIndexList FileBrowserView::getSelectedItemsOrCurrentItem()
{
    QModelIndexList modelIndexList;
    QItemSelectionModel *selectionIndexes = mListView->selectionModel();

    // by default use selected items
    if (selectionIndexes) {
        if (selectionIndexes->hasSelection()) {
            modelIndexList = mListView->selectionModel()->selectedIndexes();
        } else { // or if none selected, use the current item index
            QModelIndex currentIndex = currentItemIndex();
//            if (mFileBrowserModel->rowCount(currentItemIndex) > currentItemIndex && currentItemIndex >= 0)
//            {
                modelIndexList.append(currentIndex);
//            }
        }
    }
    mClipBoardInUse = true;
    return modelIndexList;
}

// ---------------------------------------------------------------------------

QModelIndex FileBrowserView::currentItemIndex()
{
    return mListView->selectionModel()->currentIndex();
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
    if (mListView->selectionModel()->selection().count() != 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->moveDownToDirectory(currentIndex);
        populateFolderContent();
    } else {
        Notifications::showErrorNote("not selected item!");
    }
}

void FileBrowserView::fileOpenDirectory()
{
    mLocationChanged = true;
    // get selected drive or directory from list view model and open it:
    //if (mListView->selectionModel()->hasSelection()) {
    if (mListView->selectionModel()->selection().count() != 0) {
        QModelIndex currentIndex = currentItemIndex();
        mEngineWrapper->moveDownToDirectory(currentIndex);
        populateFolderContent();
    } else {
        Notifications::showErrorNote("not selected item!");
    }
}

void FileBrowserView::fileSearch()
{
    // create and launch search:
//    if(mSearch != 0){
//        delete mSearch;
//        mSearch = 0;
//    }
//    mSearch = new SearchView(*this, mMainWindow, *mEngineWrapper);
//    QString searchPath;
//    if (mDirectory != mInitDirPath.path()) {
//        searchPath = mDirectory;
//        searchPath.replace("/", "\\");
//        searchPath+="\\";
//    }
//    mSearch->open(searchPath);
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
    if (action == dlg->primaryAction())
    {
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
    if (action == dlg->primaryAction())
    {
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
    QModelIndexList currentSelection = getSelectedItemsOrCurrentItem();
    const QString messageFormat = "Delete %1 entries?";
    QString message = messageFormat.arg(currentSelection.count());
    HbMessageBox::question(message, this, SLOT(doFileDelete(HbAction*)));
}

/**
  Delete actually selected files
  */
void FileBrowserView::doFileDelete(HbAction* action)
{
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
        QModelIndexList currentSelection = getSelectedItemsOrCurrentItem();
        mEngineWrapper->deleteItems(currentSelection);
        refreshList();
    }
}

/**
  Open rename dialog for actually selected files
  */
void FileBrowserView::fileRename()
{
    QModelIndexList currentSelection = getSelectedItemsOrCurrentItem();
    mEngineWrapper->setCurrentSelection(currentSelection);

    for (int i(0), ie(currentSelection.count()); i < ie; ++i ) {
        mModelIndex = currentSelection.at(i);
        FileEntry entry = mEngineWrapper->getFileEntry(mModelIndex);

        QString heading = QString("Enter new name");
        HbInputDialog::getText(heading, this, SLOT(doFileRename(HbAction*)), entry.name(), scene());

    }
    mEngineWrapper->startExecutingCommands(QString("Renaming"));
    refreshList();
}

/**
  Rename actually selected files
  */
void FileBrowserView::doFileRename(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (action == dlg->primaryAction())
    {
        QString newName = dlg->value().toString();

        if (mEngineWrapper->targetExists(mModelIndex, newName)) {

            const QString messageTemplate = QString("%1 already exists, overwrite?");
            QString message = messageTemplate.arg(newName);
            HbMessageBox::question(message, this, SLOT(doFileRenameFileExist(HbAction *)));
        }
        if (mIsRenameAllowed) {
            mEngineWrapper->rename(mModelIndex, newName);
        }
    }
}

/**
  Rename actually selected files
  */
void FileBrowserView::doFileRenameFileExist(HbAction *action)
{
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->secondaryAction()) {
        mIsRenameAllowed = false;
    }
}

/**
  Touch actually selected files
  */
void FileBrowserView::fileTouch()
{
    QModelIndexList currentSelection = getSelectedItemsOrCurrentItem();
    mEngineWrapper->setCurrentSelection(currentSelection);

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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    QModelIndex currentIndex = currentItemIndex();
    mEngineWrapper->showFileCheckSums(currentIndex, checksumType);
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
    mClipboardIndices = getSelectedItemsOrCurrentItem();

    mEngineWrapper->clipboardCut(mClipboardIndices);

    int operations = mClipboardIndices.count();
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
    mClipboardIndices = getSelectedItemsOrCurrentItem();

    mEngineWrapper->clipboardCopy(mClipboardIndices);

    int operations = mClipboardIndices.count();

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

    someEntryExists = mEngineWrapper->isDestinationEntriesExists(mClipboardIndices, mEngineWrapper->currentPath());
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
    if (action == dlg->primaryAction())
    {
        QString targetDir = dlg->value().toString();
        bool someEntryExists(false);

        // TODO Set entry items here
        QModelIndexList currentSelection = getSelectedItemsOrCurrentItem();
        mEngineWrapper->setCurrentSelection(currentSelection);

        someEntryExists = mEngineWrapper->isDestinationEntriesExists(currentSelection, targetDir);
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
    if (action == dlg->primaryAction())
    {
        QString targetDir = dlg->value().toString();
        bool someEntryExists(false);

        // TODO Set entry items here
        QModelIndexList currentSelection = getSelectedItemsOrCurrentItem();
        mEngineWrapper->setCurrentSelection(currentSelection);

        someEntryExists = mEngineWrapper->isDestinationEntriesExists(currentSelection, targetDir);
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
    if (action == dlg->primaryAction()) {
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
    if (action == dlg->primaryAction()) {
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
    if (action == dlg->primaryAction())
    {
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
    if (action == dlg->primaryAction())
    {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    if (action == dlg->primaryAction()) {
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
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()){
        HbMessageBox::question(QString("Are you really sure you know what are you doing ?!?"), this, SLOT(doDiskAdminReallyEraseMBR(HbAction*)));
    }
}

void FileBrowserView::doDiskAdminReallyEraseMBR(HbAction* action)
{
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
        const QString message("Are you really sure you know what are you doing ?!?");
        HbMessageBox::question(message, this, SLOT(diskAdminPartitionDriveReallyProceed(HbAction *)));
    }
}

/**
  Partition the selected drive if user is really sure
  */
void FileBrowserView::diskAdminPartitionDriveReallyProceed(HbAction *action)
{
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
        QModelIndex currentIndex = currentItemIndex();
        mEraseMBR = false;
        // warn if the selected drive is not detected as removable
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
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
    HbMessageBox *dlg = qobject_cast<HbMessageBox*>(sender());
    if (action == dlg->primaryAction()) {
        mEraseMBR = true;
    }
}

/**
  Partition the selected drive
  */
void FileBrowserView::diskAdminPartitionDriveGetCount(HbAction* action)
{
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
void FileBrowserView::toolsAllFilesToTextFile()
{

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
    if (action == dlg->primaryAction()) {
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
    if (action == dlg->primaryAction()) {
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
    if (action == dlg->primaryAction()) {
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
    if (action == dlg->primaryAction()) {
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
    if(action == dlg->primaryAction())
    {
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
    if (mFileViewMenuActions.mSelection->isChecked()) {
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
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    itemHighlighted(selectionModel->currentIndex());
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
