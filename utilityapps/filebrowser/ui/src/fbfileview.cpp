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

#include "fbfileview.h"
#include "fbsettingsview.h"
#include "fbeditorview.h"
#include "fbsearchview.h"
#include "enginewrapper.h"
#include "notifications.h"
#include "fbfolderselectiondialog.h"
#include "fbfilemodel.h"
#include "filebrowsersortfilterproxymodel.h"
//#include "fbfilelistviewitem.h"
#include "fbpropertiesdialog.h"

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
#include <HbToolBarExtension>
#include <HbSearchPanel>
#include <HbMainWindow>

#include <QString>
#include <QGraphicsLinearLayout>
#include <QItemSelection>

#include <QDebug>
//TODO check if needed to do this way
#include <FB.hrh>

// ---------------------------------------------------------------------------

FbFileView::FbFileView() :
    mEngineWrapper(0),
    mListView(0),
    mToolBar(0),
    mNaviPane(0),
    mSearchPanel(0),
    mMainLayout(0),
    mFbFileModel(0),
    mSortFilterProxyModel(0),
    mOptionMenuActions(),
    mContextMenuActions(),
    mContextMenu(0),
    mToolbarBackAction(0),
    mToolbarFilterAction(0),
    mToolbarSelectionModeAction(0),
    mToolbarPasteAction(0),
    mSelectionModeOnIcon(),
    mSelectionModeOffIcon(),
    mItemHighlighted(false),
    mLocationChanged(false),
    mRemoveFileAfterCopied(false),
//    mClipBoardInUse(false),
    mCurrentIndex(),
    mOldPassword(),
    mPanicCategory(),
    mAbsoluteFilePath(),
    mOverwriteOptions(),
    mTargetDir(),
    mModelIndex(),
    mNewFileName(),
    mProceed(false),
    mEraseMBR(false)
{
    setTitle("File Browser");

    createMenu();
    createContextMenu();
    createSearchPanel();
    createToolBar();
}

// ---------------------------------------------------------------------------	

void FbFileView::init(EngineWrapper *engineWrapper)
{
    mEngineWrapper = engineWrapper;

    mListView = new HbListView(this);
    mListView->listItemPrototype()->setStretchingStyle(HbListViewItem::StretchLandscape);

//    mListView->setItemPrototype(new FbDiskListViewItem(mListView));

    mFbFileModel = new FbFileModel(mEngineWrapper);
    if (!mListView->model()) {
        mEngineWrapper->refreshView();
        mListView->setModel(mFbFileModel);
    }
    mListView->setRootIndex(QModelIndex());

    //setItemVisible(Hb::ToolBarItem, !mEngineWrapper->isDriveListViewActive());
//    mListView->setScrollingStyle(HbScrollArea::PanWithFollowOn);

    connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
    connect(mListView, SIGNAL(longPressed(HbAbstractViewItem*,QPointF)),
            this, SLOT(onLongPressed(HbAbstractViewItem*, QPointF)));
    connect(mEngineWrapper, SIGNAL(fileSystemDataChanged()), this, SLOT(refreshList()));

    mNaviPane = new HbLabel(this);
    mNaviPane->setPlainText(QString(" ")); // TODO get from settings or default
    //mNaviPane->setPlainText(QString(mEngineWrapper->currentPath()));
    HbFontSpec fontSpec(HbFontSpec::PrimarySmall);
    mNaviPane->setFontSpec(fontSpec);

    // Create layout and add list view and toolbar into layout:
    mMainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mMainLayout->addItem(mNaviPane);
    mMainLayout->addItem(mListView);
    setLayout(mMainLayout);
}

// ---------------------------------------------------------------------------

FbFileView::~FbFileView()
{  
    if (mContextMenu) {
        mContextMenu->deleteLater();
    }
    if (mSortFilterProxyModel) {
        delete mSortFilterProxyModel;
    }
    delete mFbFileModel;
    delete mListView;
    delete mToolBar;
}

/**
  Initial setup for options menu.
  Dynamic menu update during the runtime is performed by updateOptionMenu() which
  to menu's aboutToShow() signal.
  */
void FbFileView::createMenu()
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
void FbFileView::createFileMenu()
{
    mOptionMenuActions.mFileMenu = menu()->addMenu("File");

    mOptionMenuActions.mFileBackMoveUp = mOptionMenuActions.mFileMenu->addAction("Back/Move up", this, SLOT(fileBackMoveUp()));
    mOptionMenuActions.mFileSearch = mOptionMenuActions.mFileMenu->addAction("Search...", this, SLOT(fileSearch()));

    mOptionMenuActions.mFileNewMenu = mOptionMenuActions.mFileMenu->addMenu("New");
    mOptionMenuActions.mFileNewFile = mOptionMenuActions.mFileNewMenu->addAction("File", this, SLOT(fileNewFile()));
    mOptionMenuActions.mFileNewDirectory = mOptionMenuActions.mFileNewMenu->addAction("Directory", this, SLOT(fileNewDirectory()));

    mOptionMenuActions.mFileDelete = mOptionMenuActions.mFileMenu->addAction("Delete", this, SLOT(fileDelete()));
    mOptionMenuActions.mFileRename = mOptionMenuActions.mFileMenu->addAction("Rename", this, SLOT(fileRename()));
    mOptionMenuActions.mFileTouch = mOptionMenuActions.mFileMenu->addAction("Touch", this, SLOT(fileTouch()));

    mOptionMenuActions.mFileSetAttributes = mOptionMenuActions.mFileMenu->addAction("Set attributes...", this, SLOT(fileSetAttributes()));
}

/**
  Initial setup for Edit submenu
  */
void FbFileView::createEditMenu()
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
void FbFileView::createViewMenu()
{
    mOptionMenuActions.mViewMenu = menu()->addMenu("View");
    //mOptionMenuActions.mViewMenu->menuAction()->setVisible(false);

    mOptionMenuActions.mViewFilterEntries = mOptionMenuActions.mViewMenu->addAction("Filter entries", this, SLOT(viewFilterEntries()));
    mOptionMenuActions.mViewRefresh = mOptionMenuActions.mViewMenu->addAction("Refresh", this, SLOT(viewRefresh()));
}

/**
  Initial setup for Tools submenu
  */
void FbFileView::createToolsMenu()
{
    mOptionMenuActions.mToolsMenu = menu()->addMenu("Tools");

    mOptionMenuActions.mToolsAllAppsToTextFile = mOptionMenuActions.mToolsMenu->addAction("All apps to a text file", this, SLOT(toolsAllAppsToTextFile()));
    mOptionMenuActions.mToolsAllAppsToTextFile->setVisible(false);
    mOptionMenuActions.mToolsAllFilesToTextFile = mOptionMenuActions.mToolsMenu->addAction("All files to a text file", this, SLOT(toolsAllFilesToTextFile()));
    //mOptionMenuActions.mToolsAllFilesToTextFile->setVisible(false);

//    mOptionMenuActions.mToolsAvkonIconCacheMenu = mOptionMenuActions.mToolsMenu->addMenu("Avkon icon cache");
//    mOptionMenuActions.mToolsAvkonIconCacheMenu->menuAction()->setVisible(false);
//    mOptionMenuActions.mToolsAvkonIconCacheEnable = mOptionMenuActions.mToolsAvkonIconCacheMenu->addAction("Enable", this, SLOT(toolsAvkonIconCacheEnable()));
//    mOptionMenuActions.mToolsAvkonIconCacheDisable = mOptionMenuActions.mToolsAvkonIconCacheMenu->addAction("Clear and disable", this, SLOT(toolsAvkonIconCacheDisable()));

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
void FbFileView::createSelectionMenuItem()
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
void FbFileView::createSettingsMenuItem()
{
    mOptionMenuActions.mSetting = menu()->addAction("Settings...");
    connect(mOptionMenuActions.mSetting, SIGNAL(triggered()), this, SIGNAL(aboutToShowSettingsView()));
}


/**
  Creates About menu item in option menu
  */
void FbFileView::createAboutMenuItem()
{
    // about note
    mOptionMenuActions.mAbout = menu()->addAction("About");
    connect(mOptionMenuActions.mAbout, SIGNAL(triggered()), this, SLOT(about()));
}

/**
  Creates Exit menu item in option menu
  */
void FbFileView::createExitMenuItem()
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
void FbFileView::updateOptionMenu()
{
    bool isFileItemListEmpty = mFbFileModel->rowCount() == 0;
    bool isNormalModeActive = true;       //iModel->FileUtils()->IsNormalModeActive();
    bool isCurrentDriveReadOnly = mEngineWrapper->isCurrentDriveReadOnly();   //iModel->FileUtils()->IsCurrentDriveReadOnly();
    // bool isCurrentItemDirectory = mEngineWrapper->getFileEntry(currentItemIndex()).isDir();
    // bool currentSelected = true;    //iContainer->ListBox()->View()->ItemIsSelected(iContainer->ListBox()->View()->CurrentItemIndex());
    bool isAllSelected = mListView->selectionModel()->selection().count() == mFbFileModel->rowCount();
    //bool isNoneSelected = mListView->selectionModel()->selection().count() != 0;
    bool hasSelectedItems = mListView->selectionModel()->selection().count() != 0;
    bool isSelectionMode = mOptionMenuActions.mSelection && mOptionMenuActions.mSelection->isChecked();
    bool isClipBoardEmpty = !mEngineWrapper->isClipBoardListInUse();
    //bool showSnapShot = false;           //iModel->FileUtils()->DriveSnapShotPossible();

    bool showEditMenu(true);
    if (isFileItemListEmpty && isClipBoardEmpty)
        showEditMenu = false;
    else
        showEditMenu = true;

    mOptionMenuActions.mEditMenu->menuAction()->setVisible(showEditMenu);

    //mOptionMenuActions.mFileBackMoveUp->setVisible( !isDriveListViewActive);

    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileView, isFileItemListEmpty || !hasSelectedItems || isCurrentItemDirectory);
    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileEdit, isFileItemListEmpty || !hasSelectedItems || isCurrentItemDirectory);
    //aMenuPane->SetItemDimmed(EFileBrowserCmdFileSendTo, isFileItemListEmpty || driveListActive || isCurrentItemDirectory);

    mOptionMenuActions.mFileNewMenu->menuAction()->setVisible(!isCurrentDriveReadOnly);
    mOptionMenuActions.mFileDelete->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly && hasSelectedItems/*isSelectionMode*/);
    mOptionMenuActions.mFileRename->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly && hasSelectedItems /*&& !isSelectionMode*/);
    mOptionMenuActions.mFileTouch->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly && hasSelectedItems);

    mOptionMenuActions.mFileSetAttributes->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly && hasSelectedItems);
    // TODO mOptionMenuActions.mFileCompress->setVisible(!(isCurrentDriveReadOnly || isFileItemListEmpty || !hasSelectedItems || isCurrentItemDirectory));
    // TODO mOptionMenuActions.mFileDecompress->setVisible(!(isCurrentDriveReadOnly || isFileItemListEmpty || !hasSelectedItems || isCurrentItemDirectory));

    mOptionMenuActions.mEditMenu->menuAction()->setVisible( (!isSelectionMode && !isClipBoardEmpty && !isCurrentDriveReadOnly)
                                                            || (isSelectionMode));
    mOptionMenuActions.mEditCut->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly && isSelectionMode && hasSelectedItems);
    mOptionMenuActions.mEditCopy->setVisible(!isFileItemListEmpty && isSelectionMode && hasSelectedItems);
    mOptionMenuActions.mEditPaste->setVisible(!isClipBoardEmpty && !isCurrentDriveReadOnly);
    mOptionMenuActions.mEditCopyToFolder->setVisible(!isFileItemListEmpty && isSelectionMode && hasSelectedItems);
    mOptionMenuActions.mEditMoveToFolder->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly && isSelectionMode && hasSelectedItems);

    mOptionMenuActions.mEditSelect->setVisible(false/*!currentSelected && !isFileItemListEmpty*/);
    mOptionMenuActions.mEditUnselect->setVisible(false/*currentSelected && !isFileItemListEmpty*/);
    mOptionMenuActions.mEditSelectAll->setVisible(!isFileItemListEmpty && isSelectionMode && !isAllSelected);
    mOptionMenuActions.mEditUnselectAll->setVisible(!isFileItemListEmpty && hasSelectedItems);

    // TODO mOptionMenuActions.mViewSort->setVisible(!(!isNormalModeActive || isFileItemListEmpty));
    // TODO mOptionMenuActions.mViewOrder->setVisible(!(!isNormalModeActive  || isFileItemListEmpty));
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

void FbFileView::createContextMenu()
{
    mContextMenu = new HbMenu();
    connect(mContextMenu, SIGNAL(aboutToShow()), this, SLOT(updateContextMenu()));

    createFileContextMenu();
    createEditContextMenu();
    createViewContextMenu();
}


void FbFileView::createFileContextMenu()
{
    mContextMenuActions.mFileMenu = mContextMenu->addMenu("File");

    //mContextMenuActions.mFileBackMoveUp = mContextMenuActions.mFileMenu->addAction("Back/Move up (<-)", this, SLOT(fileBackMoveUp()));
//    mContextMenuActions.mFileOpenDirectory = mContextMenuActions.mFileMenu->addAction("Open directory (->)", this, SLOT(fileOpenDirectory()));
    mContextMenuActions.mFileOpenDirectory = mContextMenu->addAction("Open directory (->)", this, SLOT(fileOpenDirectory()));
    mContextMenuActions.mSearch = mContextMenu->addAction("Search...", this, SLOT(fileSearch()));
    mContextMenuActions.mFileSearch = mContextMenuActions.mFileMenu->addAction("Search...", this, SLOT(fileSearch()));

    mContextMenuActions.mFileDelete = mContextMenuActions.mFileMenu->addAction("Delete", this, SLOT(fileDelete()));
    mContextMenuActions.mFileRename = mContextMenuActions.mFileMenu->addAction("Rename", this, SLOT(fileRename()));
    mContextMenuActions.mFileTouch = mContextMenuActions.mFileMenu->addAction("Touch", this, SLOT(fileTouch()));
    mContextMenuActions.mFileProperties = mContextMenuActions.mFileMenu->addAction("Properties", this, SLOT(fileProperties()));

    mContextMenuActions.mFileChecksumsMenu = mContextMenuActions.mFileMenu->addMenu("Checksums");
    mContextMenuActions.mFileChecksumsMD5 = mContextMenuActions.mFileChecksumsMenu->addAction("MD5", this, SLOT(fileChecksumsMD5()));
    mContextMenuActions.mFileChecksumsMD2 = mContextMenuActions.mFileChecksumsMenu->addAction("MD2", this, SLOT(fileChecksumsMD2()));
    mContextMenuActions.mFileChecksumsSHA1 = mContextMenuActions.mFileChecksumsMenu->addAction("SHA-1", this, SLOT(fileChecksumsSHA1()));

    mContextMenuActions.mFileSetAttributes = mContextMenuActions.mFileMenu->addAction("Set attributes...", this, SLOT(fileSetAttributes()));
}

void FbFileView::createEditContextMenu()
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

void FbFileView::createViewContextMenu()
{

}

void FbFileView::updateContextMenu()
{
    bool isFileItemListEmpty = mFbFileModel->rowCount() == 0;
//    bool isNormalModeActive = true;       //iModel->FileUtils()->IsNormalModeActive();
    bool isCurrentDriveReadOnly = mEngineWrapper->isCurrentDriveReadOnly();
    bool isCurrentItemDirectory = mEngineWrapper->getFileEntry(currentItemIndex()).isDir();
    bool hasSelectedItems = mListView->selectionModel()->selection().count() != 0;
    bool isSelectionMode = mOptionMenuActions.mSelection && mOptionMenuActions.mSelection->isChecked();
    bool isClipBoardEmpty = !mEngineWrapper->isClipBoardListInUse();

    mContextMenuActions.mFileOpenDirectory->setVisible(!isFileItemListEmpty && isCurrentItemDirectory && isSelectionMode);
    mContextMenuActions.mSearch->setVisible(!isFileItemListEmpty && isSelectionMode && isCurrentItemDirectory);
    mContextMenuActions.mFileSearch->setVisible(!isFileItemListEmpty && !isSelectionMode && isCurrentItemDirectory);
    // File submenu
    //mContextMenuActions.mFileBackMoveUp->setVisible();
    mContextMenuActions.mFileMenu->menuAction()->setVisible(!isSelectionMode);

    mContextMenuActions.mFileDelete->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly);
    mContextMenuActions.mFileRename->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly && !isSelectionMode);
    mContextMenuActions.mFileTouch->setVisible(!isFileItemListEmpty && !isCurrentDriveReadOnly);
    mContextMenuActions.mFileProperties->setVisible(!isFileItemListEmpty && !isSelectionMode);

    mContextMenuActions.mFileChecksumsMenu->menuAction()->setVisible(!isFileItemListEmpty && !isSelectionMode && !isCurrentItemDirectory);

    // Edit submenu
    mContextMenuActions.mEditMenu->menuAction()->setVisible(!isSelectionMode);
    mContextMenuActions.mEditCut->setVisible(!(isCurrentDriveReadOnly || isFileItemListEmpty));
    mContextMenuActions.mEditCopy->setVisible(!(isFileItemListEmpty));
    mContextMenuActions.mEditPaste->setVisible(!isClipBoardEmpty && !isCurrentDriveReadOnly);
    mContextMenuActions.mEditCopyToFolder->setVisible(!isFileItemListEmpty);
    mContextMenuActions.mEditMoveToFolder->setVisible(!(isCurrentDriveReadOnly || isFileItemListEmpty));
}

// ---------------------------------------------------------------------------

void FbFileView::onLongPressed(HbAbstractViewItem *listViewItem, QPointF coords)
{
    QModelIndex proxyIndex = listViewItem->modelIndex();
    //map to source model if needed
    if (mSortFilterProxyModel) {
        mCurrentIndex = mSortFilterProxyModel->mapToSource(proxyIndex);
    } else {
        mCurrentIndex = proxyIndex;
    }

    mContextMenu->setPreferredPos(coords);
    mContextMenu->show();
}

/**
  Create a file browser search panel
  */
void FbFileView::createSearchPanel()
{
    // Create search panel widget
    mSearchPanel = new HbSearchPanel(this);
    mSearchPanel->setPlaceholderText(QString("Type filter criteria"));
    mSearchPanel->setProgressive(false);
    connect(mSearchPanel, SIGNAL(criteriaChanged(const QString &)), this, SLOT(filterCriteriaChanged(const QString &)));
    connect(mSearchPanel, SIGNAL(exitClicked()), this, SLOT(clearFilterCriteria()));
    mSearchPanel->hide();
}

/**
  Create a file browser tool bar
  */
void FbFileView::createToolBar()
{
    mToolBar = toolBar();

    connect(mainWindow(), SIGNAL(orientationChanged(Qt::Orientation)),  this, SLOT(setOrientation(Qt::Orientation)));

    // Back button
    mToolbarBackAction = new HbAction(mToolBar);
    mToolbarBackAction->setToolTip("Back");
    HbIcon backIcon(QString(":/qtg_mono_filebrowser_parent_folder.svg"));
    backIcon.setFlags(backIcon.flags() | HbIcon::Colorized);
    mToolbarBackAction->setIcon(backIcon);
    connect(mToolbarBackAction, SIGNAL(triggered()), this, SLOT(fileBackMoveUp()));
    mToolBar->addAction(mToolbarBackAction);

    // Filter button
    mToolbarFilterAction = new HbAction(mToolBar);
    mToolbarFilterAction->setToolTip("Filter");
    HbIcon searchIcon(QString(":/qtg_mono_filebrowser_search.svg"));
    searchIcon.setFlags(searchIcon.flags() | HbIcon::Colorized);
    mToolbarFilterAction->setIcon(searchIcon);
    connect(mToolbarFilterAction, SIGNAL(triggered()), this, SLOT(viewFilterEntries()));
    mToolBar->addAction(mToolbarFilterAction);

    // Selection Mode button
    mToolbarSelectionModeAction = new HbAction(mToolBar);
    mOptionMenuActions.mSelection->setToolTip("Selection mode");
    connect(mToolbarSelectionModeAction, SIGNAL(triggered()), this, SLOT(selectionModeButtonTriggered()));

    mSelectionModeOnIcon.setIconName (QString(":/qtg_mono_filebrowser_selection_mode.svg"));
    mSelectionModeOnIcon.setFlags(mSelectionModeOnIcon.flags() | HbIcon::Colorized);

    mSelectionModeOffIcon.setIconName(QString(":/qtg_mono_filebrowser_selection_mode_off.svg"));
    mSelectionModeOffIcon.setFlags(mSelectionModeOffIcon.flags() | HbIcon::Colorized);

    mToolbarSelectionModeAction->setIcon(mSelectionModeOnIcon);

    mToolBar->addAction(mToolbarSelectionModeAction);

    // Paste button
    mToolbarPasteAction = new HbAction(mToolBar);
    mToolbarPasteAction->setToolTip("Paste");
    HbIcon pasteIcon(QString(":/qtg_mono_filebrowser_paste.svg"));
    pasteIcon.setFlags(pasteIcon.flags() | HbIcon::Colorized);
    mToolbarPasteAction->setIcon(pasteIcon);
    connect(mToolbarPasteAction, SIGNAL(triggered()), this, SLOT(editPaste()));
    mToolBar->addAction(mToolbarPasteAction);
    mToolbarPasteAction->setEnabled(false);
}

/**
  * Change toolbar orientation
  */
void FbFileView::setOrientation(Qt::Orientation orientation)
{
    if (orientation == Qt::Horizontal) {
        mToolBar->setOrientation(Qt::Vertical);
    } else {
        mToolBar->setOrientation(Qt::Horizontal);
    }
}

/**
  Refresh FileBrowser view
  */
void FbFileView::refreshList()
{
    editUnselectAll();
    mEngineWrapper->refreshView();
    if (!mToolbarFilterAction->isEnabled()) {
        clearFilterCriteria();
    }
    mListView->reset();

    if (mListView->model() && mListView->model()->rowCount() > 0) {
        QModelIndex firstIndex = mListView->model()->index(0, 0);
        mListView->scrollTo(firstIndex);
    }

    mToolbarPasteAction->setEnabled(mEngineWrapper->isClipBoardListInUse());

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

// ---------------------------------------------------------------------------	

void FbFileView::fileOpen(HbAction *action)
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
void FbFileView::fileOverwriteDialog(/*QObject *receiver, const char *member*/)
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
void FbFileView::fileOverwrite(HbAction *action)
{
    HbSelectionDialog *dlg = static_cast<HbSelectionDialog*>(sender());
    if(!action && dlg && dlg->selectedItems().count()) {
        mOverwriteOptions.queryIndex = dlg->selectedItems().at(0).toInt();
        if (mOverwriteOptions.queryIndex == EFileActionQueryPostFix) {
            QString heading = QString("Postfix");
            HbInputDialog::queryText(heading,
                                     this, SLOT(fileOverwritePostfix(HbAction *)),
                                     QString(), scene());
        } else if (mOverwriteOptions.queryIndex == EFileActionSkipAllExisting) {
            mOverwriteOptions.overWriteFlags = 0;
            emit overwriteOptionSelected();
        } else {
            emit overwriteOptionSelected();
        }
    } else {
        mOverwriteOptions.doFileOperations = false;
        emit overwriteOptionSelected();
    }
}

/**
  File overwrite postfix query dialog
  */
void FbFileView::fileOverwritePostfix(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        mOverwriteOptions.postFix = dlg->value().toString();
    } else {
        mOverwriteOptions.doFileOperations = false;
    }
    emit overwriteOptionSelected();
}

/**
  Show a list dialog
  \param List \a items of item to select item from.
  \param Title text \a titleText of a dialog heading widget
  \param Receiver \a receiver execute slot on
  \param Slot \a executed when dialog has been closed
  \return None
  */
void FbFileView::openListDialog(const QStringList& items, const QString &titleText,
                                QObject *receiver, const char *member)
{
    // Create a list and some simple content for it
    HbSelectionDialog *dlg = new HbSelectionDialog();
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    // Set items to be popup's content
    dlg->setStringItems(items);
    dlg->setSelectionMode(HbAbstractItemView::SingleSelection);

    HbLabel *title = new HbLabel(dlg);
    title->setPlainText(titleText);
    dlg->setHeadingWidget(title);
    // Launch popup and handle the user response:
    dlg->open(receiver, member);
}

/**
  Stores selection or current index mapped to source model
  */
void FbFileView::storeSelectedItemsOrCurrentItem()
{
    QItemSelectionModel *selectionIndexes = mListView->selectionModel();

    // by default use selected items
    if (selectionIndexes) {
        bool isSelectionMode = mOptionMenuActions.mSelection && mOptionMenuActions.mSelection->isChecked();
        if (isSelectionMode) {
            if (selectionIndexes->hasSelection()) {
                QItemSelection itemSelection = mListView->selectionModel()->selection();
                //map to source model if needed
                if (mSortFilterProxyModel) {
                    mSelectionIndexes = mSortFilterProxyModel->mapSelectionToSource(itemSelection).indexes();
                } else {
                    mSelectionIndexes = itemSelection.indexes();
                }
            } else { // or if none selected, clear selection
                mSelectionIndexes.clear();
            }
        } else { // or if none selected, use the current item index
            mSelectionIndexes.clear();
            QModelIndex currentIndex = currentItemIndex();  //alreade mapped to source model
            mSelectionIndexes.append(currentIndex);
        }
    }
}

// ---------------------------------------------------------------------------

QModelIndex FbFileView::currentItemIndex()
{
    return mCurrentIndex;
}

// ---------------------------------------------------------------------------
// operations in File Menu
// ---------------------------------------------------------------------------

/**
  Move back/up in folder browsing history
  */
void FbFileView::fileBackMoveUp()
{
    mLocationChanged = true;
    mFbFileModel->moveUpOneLevel();
    //mListView->setRootIndex(currentItemIndex());
    refreshList();
    if (mEngineWrapper->isDriveListViewActive()) {
        emit aboutToShowDriveView();
    }
}

void FbFileView::fileOpenDirectory()
{
    mLocationChanged = true;
    mFbFileModel->moveDownToDirectory(currentItemIndex());

    refreshList();
}

void FbFileView::fileSearch()
{
    QString searchPath;
    HbAction *contextrMenuAction = static_cast<HbAction *>(sender());
    if (contextrMenuAction
        && (contextrMenuAction == mContextMenuActions.mSearch
            || contextrMenuAction == mContextMenuActions.mFileSearch)
        && mEngineWrapper->getFileEntry(currentItemIndex()).isDir()) {
        searchPath = mEngineWrapper->currentPath()
                     + mEngineWrapper->getFileEntry(currentItemIndex()).name()
                     + QString("\\");
    } else {
        searchPath = mEngineWrapper->currentPath();
    }

    emit aboutToShowSearchView(searchPath);
}

/**
  Open new file dialog
  */
void FbFileView::fileNewFile()
{
    QString heading = QString("Enter filename");
    HbInputDialog::queryText(heading, this, SLOT(doFileNewFile(HbAction*)), QString(), scene());
}

/**
  Create a new file in current directory with a name queried from user
  */
void FbFileView::doFileNewFile(HbAction *action)
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
void FbFileView::fileNewDirectory()
{
    QString heading = QString("Enter directory name");
    HbInputDialog::queryText(heading, this, SLOT(doFileNewDirectory(HbAction*)), QString(), scene());
}

/**
  Create a new directory in current directory with a name queried from user
  */
void FbFileView::doFileNewDirectory(HbAction *action)
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
void FbFileView::fileDelete()
{
    storeSelectedItemsOrCurrentItem();
    const QString messageFormat = "Delete %1 entries?";
    QString message = messageFormat.arg(mSelectionIndexes.count());
    HbMessageBox::question(message, this, SLOT(doFileDelete(int)), HbMessageBox::Yes | HbMessageBox::No);
}

/**
  Delete actually selected files
  */
void FbFileView::doFileDelete(int action)
{
    if (action == HbMessageBox::Yes) {
        mEngineWrapper->deleteItems(mSelectionIndexes);
        mEngineWrapper->startExecutingCommands(QString("Deleting"));
    }
}

/**
  Open rename dialog for actually selected files
  */
void FbFileView::fileRename()
{
    storeSelectedItemsOrCurrentItem();
    mEngineWrapper->setCurrentSelection(mSelectionIndexes);

    for (int i(0), ie(mSelectionIndexes.count()); i < ie; ++i ) {
        mProceed = (i == ie-1); // if the last item
        mModelIndex = mSelectionIndexes.at(i);
        FbFileEntry entry = mEngineWrapper->getFileEntry(mModelIndex);

        QString heading = QString("Enter new name");
        HbInputDialog::queryText(heading, this, SLOT(doFileRename(HbAction*)), entry.name(), scene());
    }
}

/**
  Rename actually selected files
  */
void FbFileView::doFileRename(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        mNewFileName = dlg->value().toString();

        if (mEngineWrapper->targetExists(mModelIndex, mNewFileName)) {
            const QString messageTemplate = QString("%1 already exists, overwrite?");
            QString message = messageTemplate.arg(mNewFileName);
            HbMessageBox::question(message, this, SLOT(doFileRenameFileExist(int)), HbMessageBox::Yes | HbMessageBox::No);
        } else {
            proceedFileRename();
        }
    }
}

/**
  Rename actually selected files
  */
void FbFileView::doFileRenameFileExist(int action)
{
    if (action == HbMessageBox::Yes) {
        proceedFileRename();
    }
}


void FbFileView::proceedFileRename()
{
    mEngineWrapper->rename(mModelIndex, mNewFileName);
    if (mProceed) {
        mEngineWrapper->startExecutingCommands(QString("Renaming"));
        refreshList();
    }
}

/**
  Touch actually selected files
  */
void FbFileView::fileTouch()
{
    storeSelectedItemsOrCurrentItem();
    mEngineWrapper->setCurrentSelection(mSelectionIndexes);

    if (mEngineWrapper->selectionHasDirs()) {
        const QString message = "Recurse touch for all selected dirs?";
        HbMessageBox::question(message, this, SLOT(doFileTouch(int)), HbMessageBox::Yes | HbMessageBox::No);
    } else {
        proceedFileTouch(false);
    }
}

/**
  Touch actually selected files
  */
void FbFileView::doFileTouch(int action)
{
    bool recurse = false;
    if (action == HbMessageBox::Yes) {
        recurse = true;
        }
    proceedFileTouch(recurse);
}

void FbFileView::proceedFileTouch(bool recurse)
{
    mEngineWrapper->touch(recurse);
    mEngineWrapper->startExecutingCommands(QString("Touching"));
    refreshList();
}

void FbFileView::fileChecksumsMD5()
{
    fileChecksums(EFileChecksumsMD5);
}

void FbFileView::fileChecksumsMD2()
{
    fileChecksums(EFileChecksumsMD2);
}

void FbFileView::fileChecksumsSHA1()
{
    fileChecksums(EFileChecksumsSHA1);
}

void FbFileView::fileChecksums(TFileBrowserCmdFileChecksums checksumType)
{
    mEngineWrapper->showFileCheckSums(currentItemIndex(), checksumType);
}

/**
  Show file properties
  */
void FbFileView::fileProperties()
{
    const QString PropertiesEntryFormat("%1\t%2");
    const QString PropertiesEntryTabbedFormat("\t%1");
    const QString PropertiesEntrySizeFormat("%1\t%2 B");
    const QString DateFormat("%D%M%Y%/0%1%/1%2%/2%3%/3");
    const QString TimeFormat("%-B%:0%J%:1%T%:2%S%:3%+B");

    const QString AttributesText("Atts");
    const QString NumberOfEntriesText("Entries");
    const QString NumberOfFilesText("Files");
    const QString SizeText("Size");
    const QString PathText("Path");
    const QString DateText("Date");
    const QString TimeText("Time");
    const QString MimeTypeText("Mime");
    const QString OpensWithText("Opens");

    QModelIndex currentIndex = currentItemIndex();
    QVector<QPair<QString, QString> > properties;
    QString titleText;
    bool showDialog(false);

    if (currentIndex.row() >= 0 && currentIndex.row() < mEngineWrapper->itemCount()) {
        // it is a file or a directory entry
        FbFileEntry fileEntry(mEngineWrapper->getFileEntry(currentIndex));

        // set title
        titleText.append(fileEntry.name());

        // path
        properties.append(qMakePair(PathText, fileEntry.path()));

        // date
        properties.append(qMakePair(DateText, fileEntry.modifiedDateTimeString(DateFormat)));

        // time
        properties.append(qMakePair(TimeText, fileEntry.modifiedDateTimeString(TimeFormat)));

        if (!fileEntry.isDir()) {
            // size
            QLocale loc;
            QString fileSize = loc.toString(fileEntry.size());

            //QString fileSize = QString::number(fileEntry.size());
            properties.append(qMakePair(SizeText, fileSize));
        } else if (fileEntry.isDir()
                   && mEngineWrapper->settings().showSubDirectoryInfo()) {

            QString fullPath;
            fullPath.append(fileEntry.path())
                    .append(fileEntry.name())
                    .append(QString("\\"));

            // number of entries
            int entriesCount = mEngineWrapper->getEntriesCount(fullPath);
            if (entriesCount >= 0) {
                properties.append(qMakePair(NumberOfEntriesText, QString::number(entriesCount)));
            }

            // number of files
            qint64 size(0);
            int allFilesCount = mEngineWrapper->getFilesCountAndSize(fullPath, size);
            properties.append(qMakePair(NumberOfFilesText, QString::number(allFilesCount)));

            // size
            QLocale loc;
            QString folderSize = loc.toString(size);
            properties.append(qMakePair(SizeText, folderSize));
        }

        // attributes
        QStringList attributesValues;
        if (fileEntry.isArchive())
            attributesValues.append(fileEntry.archiveText());
        if (fileEntry.isHidden())
            attributesValues.append(fileEntry.hiddenText());
        if (fileEntry.isReadOnly())
            attributesValues.append(fileEntry.readOnlyText());
        if (fileEntry.isSystem())
            attributesValues.append(fileEntry.systemText());
        if (attributesValues.isEmpty())
            attributesValues.append(fileEntry.noAttributeText());

        for (int i(0), ie(attributesValues.count()); i < ie; ++i) {
            properties.append(qMakePair(AttributesText, attributesValues.at(i)));
        }

        if (!fileEntry.isDir()) {
            // mime type
            QString fullPath = fileEntry.path();
            fullPath.append(fileEntry.name());
            QString mimeType = mEngineWrapper->getMimeType(fullPath);
            if (!mimeType.isEmpty()) {
                properties.append(qMakePair(MimeTypeText, mimeType));
            }

            // opens with
            QString openWith = mEngineWrapper->getOpenWith(fullPath);
            if (!openWith.isEmpty()) {
                properties.append(qMakePair(OpensWithText, openWith));
            }
        }
        showDialog = true;
    }

    if (showDialog) {
        FbPropertiesDialog *dialog = new FbPropertiesDialog();
        dialog->setTitle(titleText);

        // Set listwidget to be popup's content
        dialog->setProperties(properties);
        // Launch popup and handle the user response:
        dialog->open();
    }
}

void FbFileView::fileSetAttributes()
{
    storeSelectedItemsOrCurrentItem();
    mEngineWrapper->setCurrentSelection(mSelectionIndexes);

    QString attributesViewTitle("Multiple entries");

    quint32 setAttributesMask(0);
    quint32 clearAttributesMask(0);
    bool recurse(false);

    // set default masks if only one file selected
    if (mSelectionIndexes.count() == 1)
        {
        mModelIndex = mSelectionIndexes.at(0);
        FbFileEntry fileEntry = mEngineWrapper->getFileEntry(mModelIndex);

        attributesViewTitle = fileEntry.name();

        if (fileEntry.isArchive())
            setAttributesMask |= KEntryAttArchive;
        else
            clearAttributesMask |= KEntryAttArchive;

        if (fileEntry.isHidden())
            setAttributesMask |= KEntryAttHidden;
        else
            clearAttributesMask |= KEntryAttHidden;

        if (fileEntry.isReadOnly())
            setAttributesMask |= KEntryAttReadOnly;
        else
            clearAttributesMask |= KEntryAttReadOnly;

        if (fileEntry.isSystem())
            setAttributesMask |= KEntryAttSystem;
        else
            clearAttributesMask |= KEntryAttSystem;
        }

    emit aboutToShowAttributesView(attributesViewTitle, setAttributesMask, clearAttributesMask, recurse);
}

// edit menu
void FbFileView::editSnapShotToE()
{

}

/**
  Set selected files into clipboard.
  Selected item will be removed after paste operation.
  */
void FbFileView::editCut()
{
    storeSelectedItemsOrCurrentItem();

    // Store indices to clipboard
    mClipboardIndexes.clear();
    for (int i = 0; i < mSelectionIndexes.size(); ++i) {
        mClipboardIndexes.append(mSelectionIndexes.at(i));
    }

    mEngineWrapper->clipboardCut(mClipboardIndexes);
    mEngineWrapper->setCurrentSelection(mClipboardIndexes);

    int operations = mClipboardIndexes.count();
    const QString message = QString ("%1 entries cut to clipboard");
    QString noteMsg = message.arg(operations);

    mToolbarPasteAction->setEnabled(true);
    Notifications::showInformationNote(noteMsg);
}

/**
  Set selected files into clipboard.
  Selected item will not be removed after paste operation.
  */
void FbFileView::editCopy()
{
    storeSelectedItemsOrCurrentItem();

    // Store indices to clipboard
    mClipboardIndexes.clear();
    for (int i = 0; i < mSelectionIndexes.size(); ++i) {
        mClipboardIndexes.append(mSelectionIndexes.at(i));
    }

    mEngineWrapper->clipboardCopy(mClipboardIndexes);
    mEngineWrapper->setCurrentSelection(mClipboardIndexes);

    int operations = mClipboardIndexes.count();

    const QString message = QString ("%1 entries copied to clipboard");
    QString noteMsg = message.arg(operations);

    mToolbarPasteAction->setEnabled(true);
    Notifications::showInformationNote(noteMsg);
}

/**
  Moves or copies file selection stored in clipboard to a actual directory.
  Removing files depend on previous operation, i.e. Cut or Copy
  */
void FbFileView::editPaste()
{
    bool someEntryExists(false);

    someEntryExists = mEngineWrapper->isDestinationEntriesExists(mClipboardIndexes, mEngineWrapper->currentPath());
    if (someEntryExists) {
        connect(this, SIGNAL(overwriteOptionSelected()), this, SLOT(doEditPaste()));
        fileOverwriteDialog();
    } else {
        doEditPaste();
    }
}

void FbFileView::doEditPaste()
{
    mEngineWrapper->clipboardPaste(mOverwriteOptions);
    mEngineWrapper->startExecutingCommands(mEngineWrapper->getClipBoardMode() == EClipBoardModeCut ?
                                           QString("Moving") : QString("Copying") );
    disconnect(this, SIGNAL(overwriteOptionSelected()), this, SLOT(doEditPaste()));
}

/**
  Open copy to folder new filename dialog
  */
void FbFileView::editCopyToFolder()
{
    QString heading = QString("Enter new name");
    FbCopyToFolderSelectionDialog *folderSelectionDialog = new FbCopyToFolderSelectionDialog();
    folderSelectionDialog->open(this, SLOT(doEditCopyToFolder(int)));
}

/**
  Copies current file selection to a queried directory.
  */
void FbFileView::doEditCopyToFolder(int action)
{
    FbCopyToFolderSelectionDialog *dlg = qobject_cast<FbCopyToFolderSelectionDialog*>(sender());
    if (dlg && action == HbDialog::Accepted) {
        mTargetDir = dlg->selectedFolder();

        bool someEntryExists(false);

        // TODO Set entry items here
        storeSelectedItemsOrCurrentItem();
        mEngineWrapper->setCurrentSelection(mSelectionIndexes);

        someEntryExists = mEngineWrapper->isDestinationEntriesExists(mSelectionIndexes, mTargetDir);
        if (someEntryExists) {
            connect(this, SIGNAL(overwriteOptionSelected()), this, SLOT(doEditCopy()));
            fileOverwriteDialog();
        } else {
            doEditCopy();
        }
    }
}

void FbFileView::doEditCopy()
{
    mEngineWrapper->copyToFolder(mTargetDir, mOverwriteOptions, false);
    mEngineWrapper->startExecutingCommands(QString("Copying"));
    disconnect(this, SIGNAL(overwriteOptionSelected()), this, SLOT(doEditCopy()));
}

/**
  Open move to folder new filename dialog.
  */
void FbFileView::editMoveToFolder()
{
    QString heading = QString("Enter new name");
    FbMoveToFolderSelectionDialog *folderSelectionDialog = new FbMoveToFolderSelectionDialog();
    folderSelectionDialog->open(this, SLOT(doEditMoveToFolder(int)));
}

/**
  Moves current file selection to a queried directory.
  */
void FbFileView::doEditMoveToFolder(int action)
{
    FbMoveToFolderSelectionDialog *dlg = qobject_cast<FbMoveToFolderSelectionDialog*>(sender());
    if (dlg && action == HbDialog::Accepted) {
        mTargetDir = dlg->selectedFolder();

        bool someEntryExists(false);

        // TODO Set entry items here
        storeSelectedItemsOrCurrentItem();
        mEngineWrapper->setCurrentSelection(mSelectionIndexes);

        someEntryExists = mEngineWrapper->isDestinationEntriesExists(mSelectionIndexes, mTargetDir);
        if (someEntryExists) {
            connect(this, SIGNAL(overwriteOptionSelected()), this, SLOT(doEditMove()));
            fileOverwriteDialog();
        } else {
            doEditMove();
        }
    }
}

void FbFileView::doEditMove()
{
    mEngineWrapper->copyToFolder(mTargetDir, mOverwriteOptions, true);
    mEngineWrapper->startExecutingCommands(QString("Moving"));
    disconnect(this, SIGNAL(overwriteOptionSelected()), this, SLOT(doEditMove()));
}

/**
  Select current file
  */
void FbFileView::editSelect()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {
        selectionModel->select(selectionModel->currentIndex(), QItemSelectionModel::SelectCurrent);
        selectionModel->select(selectionModel->currentIndex(), QItemSelectionModel::Select);
        refreshList();
    }
}

/**
  Unselect current file
  */
void FbFileView::editUnselect()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {
        selectionModel->select(selectionModel->currentIndex(), QItemSelectionModel::Deselect);
//        itemHighlighted(selectionModel->currentIndex());
    }
}

/**
  Select all files
  */
void FbFileView::editSelectAll()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {

        //if (mFileBrowserModel->rowCount() > 0) {
        if (mListView->model() && mListView->model()->rowCount() > 0) {
            QModelIndex firstIndex = mListView->model()->index(0, 0);
            QModelIndex lastIndex = mListView->model()->index( (mListView->model()->rowCount() - 1), 0);

            QItemSelection itemSelection(firstIndex, lastIndex);
            selectionModel->select(itemSelection, QItemSelectionModel::SelectCurrent);
        }
    }
}

/**
  Unselect all files
  */
void FbFileView::editUnselectAll()
{
    QItemSelectionModel *selectionModel = mListView->selectionModel();
    if (selectionModel) {
        selectionModel->clearSelection();
    }
}

// ---------------------------------------------------------------------------
// view menu
// ---------------------------------------------------------------------------
/**
  * Open the search panel
  */
void FbFileView::viewFilterEntries()
{
    if (mToolbarFilterAction) {
        mToolbarFilterAction->setEnabled(false);
    }

    if (mMainLayout && mSearchPanel) {
        mMainLayout->addItem(mSearchPanel);
        mSearchPanel->show();
    }
}

/**
  Set filter criteria to proxy model
  */
void FbFileView::filterCriteriaChanged(const QString &criteria)
{
    if (!mSortFilterProxyModel) {
        mSortFilterProxyModel = new FileBrowserSortFilterProxyModel(this);
        mSortFilterProxyModel->setSourceModel(mFbFileModel);
        mSortFilterProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
        mListView->setModel(mSortFilterProxyModel);

    }
    mSortFilterProxyModel->setFilterCriteria(criteria);

    setTitle(criteria);
    if (criteria.isEmpty()) {
        setTitle("File Browser");
    }
}

/**
  Set filter criteria to proxy model
  */
void FbFileView::clearFilterCriteria()
{
    if (mToolbarFilterAction) {
        mToolbarFilterAction->setEnabled(true);
    }

    if (mSortFilterProxyModel) {
        mListView->setModel(mFbFileModel);
        mSortFilterProxyModel->deleteLater();
        mSortFilterProxyModel = 0;
    }

    if (mMainLayout && mSearchPanel) {
        mSearchPanel->setCriteria(QString(""));
        mMainLayout->removeItem(mSearchPanel);
        mSearchPanel->hide();
        refreshList();
    }
    setTitle("File Browser");
}

/**
  Refresh view
  */
void FbFileView::viewRefresh()
{
    refreshList();
}

// ---------------------------------------------------------------------------
// tools menu
// ---------------------------------------------------------------------------
void FbFileView::toolsAllAppsToTextFile()
{

}

/**
  Write all files to text file
  */
void FbFileView::toolsAllFilesToTextFile()
{
    mEngineWrapper->toolsWriteAllFiles();
}

//void FbFileView::toolsAvkonIconCacheEnable()
//{
//
//}
//void FbFileView::toolsAvkonIconCacheDisable()
//{
//
//}

/**
  Disable extended errors
  */
void FbFileView::toolsDisableExtendedErrors()
{
    mEngineWrapper->ToolsSetErrRd(false);
}

void FbFileView::toolsDumpMsgStoreWalk()
{

}
void FbFileView::toolsEditDataTypes()
{

}

/**
  Enable extended errors
  */
void FbFileView::toolsEnableExtendedErrors()
{
    mEngineWrapper->ToolsSetErrRd(true);
}

/**
  Open simulate leave dialog
  */
void FbFileView::toolsErrorSimulateLeave()
{
    int leaveCode = -6;
    QString heading = QString("Leave code");
    //HbInputDialog::queryInt(heading, this, SLOT(doToolsErrorSimulateLeave(HbAction*)), leaveCode, scene());
    HbInputDialog::queryText(heading, this, SLOT(doToolsErrorSimulateLeave(HbAction*)), QString::number(leaveCode), scene());
}


/**
  Simulate leave.
  */
void FbFileView::doToolsErrorSimulateLeave(HbAction *action)
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
void FbFileView::toolsErrorSimulatePanic()
{
    mPanicCategory = QString ("Test Category");
    QString heading = QString("Panic category");
    HbInputDialog::queryText(heading, this, SLOT(doToolsErrorSimulatePanicCode(HbAction*)), mPanicCategory, scene());
}

/**
  Simulate panic.
  */
void FbFileView::doToolsErrorSimulatePanicCode(HbAction *action)
{
    HbInputDialog *dlg = static_cast<HbInputDialog*>(sender());
    if (dlg && action && action->text().compare(QString("Ok"), Qt::CaseInsensitive) == 0) {
        mPanicCategory = dlg->value().toString();
        int panicCode(555);
        QString heading = QString("Panic code");
        HbInputDialog::queryInt(heading, this, SLOT(doToolsErrorSimulatePanic(HbAction*)), panicCode, scene());
    }
}

/**
  Simulate panic.
  */
void FbFileView::doToolsErrorSimulatePanic(HbAction *action)
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
void FbFileView::toolsErrorSimulateException()
{
    int exceptionCode = 0;
    QString heading = QString("Exception code");
    HbInputDialog::queryInt(heading, this, SLOT(doToolsErrorSimulateException(HbAction*)), exceptionCode, scene());
}

/**
  Simulate exception.
  */
void FbFileView::doToolsErrorSimulateException(HbAction *action)
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

//    void FbFileView::toolsLocalConnectivityActivateInfrared()
//{
//
//}
//    void FbFileView::toolsLocalConnectivityLaunchBTUI()
//{
//
//}
//    void FbFileView::toolsLocalConnectivityLaunchUSBUI()
//{
//
//}
void FbFileView::toolsMessageInbox()
{

}
void FbFileView::toolsMessageDrafts()
{

}
void FbFileView::toolsMessageSentItems()
{

}
void FbFileView::toolsMessageOutbox()
{

}
void FbFileView::toolsMemoryInfo()
{

}
void FbFileView::toolsSecureBackStart()
{

}
void FbFileView::toolsSecureBackRestore()
{

}
void FbFileView::toolsSecureBackStop()
{

}

/**
  Open debug mask dialog
  */
void FbFileView::toolsSetDebugMaskQuestion()
{
    quint32 dbgMask = mEngineWrapper->getDebugMask();
    QString dbgMaskText = QString("0x").append(QString::number(dbgMask, 16));
    QString heading = QString("Kernel debug mask in hex format");
    HbInputDialog::queryText(heading, this, SLOT(toolsSetDebugMask(HbAction*)), dbgMaskText, scene());
}

/**
  Set debug mask
  */
void FbFileView::toolsSetDebugMask(HbAction *action)
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

void FbFileView::toolsShowOpenFilesHere()
{

}

// ---------------------------------------------------------------------------
// main menu items
// ---------------------------------------------------------------------------
void FbFileView::selectionModeChanged()
{
    if (mOptionMenuActions.mSelection->isChecked()) {
        activateSelectionMode();
        mToolbarSelectionModeAction->setIcon(mSelectionModeOffIcon);
    } else {
        deActivateSelectionMode();
        mToolbarSelectionModeAction->setIcon(mSelectionModeOnIcon);
    }
}

void FbFileView::selectionModeButtonTriggered()
{
    if (mOptionMenuActions.mSelection->isChecked()) {
        mOptionMenuActions.mSelection->setChecked(false);
    } else {
        mOptionMenuActions.mSelection->setChecked(true);
    }
    selectionModeChanged();
}

/**
  Show about note
  */
void FbFileView::about()
{
    Notifications::showAboutNote();
}

// ---------------------------------------------------------------------------
// End of operations
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------

/**
  An item is clicked from navigation item list. Navigation item list contains
  drive-, folder- or file items. Opens selected drive, folder or file popup menu
  */
void FbFileView::activated(const QModelIndex& index)
{
    if (mFbFileModel) {
        //map to source model
        QModelIndex activatedIndex = index;
        if (mSortFilterProxyModel) {
            activatedIndex = mSortFilterProxyModel->mapToSource(index);
        }

        if (mEngineWrapper->getFileEntry(activatedIndex).isDir()) {
            // populate new content of changed navigation view.
            mFbFileModel->moveDownToDirectory(activatedIndex);
            refreshList();
        } else {  // file item
            // mSelectedFilePath = filePath;
            FbFileEntry fileEntry = mEngineWrapper->getFileEntry(activatedIndex);
            mAbsoluteFilePath = fileEntry.path() + fileEntry.name();

            // open user-dialog to select: view as text/hex,  open w/AppArc or open w/DocH. embed
            QStringList list;
            list << QString("View as text/hex")
                 << QString("Open w/ AppArc")
                 << QString("Open w/ DocH. embed");
            openListDialog(list, QString("Open file"), this, SLOT(fileOpen(HbAction *)));
        }
    }
}

// ---------------------------------------------------------------------------

void FbFileView::activateSelectionMode()
{
    QString path;
    disconnect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
    mListView->setSelectionMode(HbListView::MultiSelection);
}

// ---------------------------------------------------------------------------

void FbFileView::deActivateSelectionMode()
{
    mListView->setSelectionMode(HbListView::NoSelection);
    connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(activated(QModelIndex)));
    editUnselectAll();
}

// ---------------------------------------------------------------------------
