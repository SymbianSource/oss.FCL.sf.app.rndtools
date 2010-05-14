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

#include "searchview.h"
#include "enginewrapper.h"

#include <HbView>
#include <HbMainWindow>

#include <HbDataForm>
#include <HbDataFormModel>
#include <HbDataFormModelItem>
#include <HbAction>
#include <HbPushButton>
#include <HbListWidget>
#include <HbListWidgetItem>
#include <HbDialog>
#include <HbMenu>

#include <QtGui>
#include <QDir>
#include <QFileInfo>
#include <QProgressBar>


const QStringList RECURSEMODES = (QStringList() << "Non-recursive" << "Recursive");

SearchView::SearchView(HbView &mainView, HbMainWindow &mainWindow, EngineWrapper &engineWrapper)
: mMainWindow(mainWindow),
  mMainView(mainView),
  mEngineWrapper(engineWrapper),
  mAttributes(),
  mResults(),
  mForm(0),
  mSearchFileNameItem(0),
  mWildCardItem(0),
  mHasString(0),
  mRecurse(0),
  mMinSize(0),
  mMaxSize(0),
  mMinDate(0),
  mMaxDate(0),
  mPath()
{
}

SearchView::~SearchView() 
{
}

void SearchView::open(const QString &path)
{   
    // Remove main view from main window
    mMainWindow.removeView(&mMainView);
    mMainWindow.addView(this);
    // Set title and add this view to main window
    setTitle("Search");
	
    mForm = new HbDataForm(/*HbFormView::ModeEdit,*/ this);
    HbDataFormModel *model = new HbDataFormModel();
    mPath = path;
    mSearchFileNameItem = new HbDataFormModelItem(HbDataFormModelItem::TextItem);
    mSearchFileNameItem->setLabel("Search dir: ");

    mWildCardItem = new HbDataFormModelItem(HbDataFormModelItem::TextItem);
    mWildCardItem->setLabel("Wildcards: ");

    mHasString = new HbDataFormModelItem(HbDataFormModelItem::TextItem);
    mHasString->setLabel("Has string: ");
    
    mRecurse = new HbDataFormModelItem(HbDataFormModelItem::ComboBoxItem);
    mRecurse->setLabel("Recurse: ");
    mRecurse->setContentWidgetData("items", RECURSEMODES);
    
    mMinSize = new HbDataFormModelItem(HbDataFormModelItem::TextItem);
    mMinSize->setLabel("Min size: ");
    
    mMaxSize = new HbDataFormModelItem(HbDataFormModelItem::TextItem);
    mMaxSize->setLabel("Max size: ");
    
    mMinDate = new HbDataFormModelItem(HbDataFormModelItem::TextItem);
    mMinDate->setLabel("Min date: ");
    mMinDate->setContentWidgetData("text", "01/01/1980");
    
    mMaxDate = new HbDataFormModelItem(HbDataFormModelItem::TextItem);
    mMaxDate->setLabel("Max date: ");
    mMaxDate->setContentWidgetData("text", "31/12/2060");
    
    // load file search attribute values from FB engine:
    loadAttributes();
    
    model->appendDataFormItem(mSearchFileNameItem);
    model->appendDataFormItem(mWildCardItem);
    model->appendDataFormItem(mHasString);
    model->appendDataFormItem(mRecurse);
    model->appendDataFormItem(mMinSize);
    model->appendDataFormItem(mMaxSize);
    model->appendDataFormItem(mMinDate);
    model->appendDataFormItem(mMaxDate);
    mForm->setModel(model);

    QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical,this);

    HbPushButton* buttonSearch = new HbPushButton("Search");
    buttonSearch->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    connect(buttonSearch, SIGNAL(clicked()), this, SLOT(startFileSearch()));

	HbPushButton* buttonCancel = new HbPushButton("Cancel");
    buttonCancel->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(backButtonClicked()));

    // Create widget and layout for buttons
    QGraphicsWidget *buttonsWidget = new QGraphicsWidget(this);
    QGraphicsGridLayout *gridLayout = new QGraphicsGridLayout(buttonsWidget);
    
    // add buttons to layout
    gridLayout->addItem(buttonSearch, 0, 0);
	gridLayout->addItem(buttonCancel, 0, 1);
    buttonsWidget->setLayout(gridLayout);
	// add form and buttons widget to list layout
	layout->addItem(mForm);
	layout->addItem(buttonsWidget);
    setLayout(layout);
    // about to close connect to go back to file browser view
    connect(this, SIGNAL(aboutToClose()), this, SLOT(backButtonClicked())); 
}

void SearchView::backButtonClicked()
{
    mMainWindow.removeView(this);
	mForm->deleteLater();
	mForm = 0;
    mMainWindow.addView(&mMainView);
}

void SearchView::startFileSearch()
{
    // save form data values and set them as search attributes into FB engine. 
    readFormItems();
    mEngineWrapper.setFileSearchAttributes(mAttributes);
    
    // Create busy indicator widget with  label **Searching**. Launch it to screen when
    // FB Utils engine started for file search.  Hide busy indicator when search completed.
    QProgressBar *progressBar = new QProgressBar();
	progressBar->setMaximumHeight(16);
	progressBar->setMaximumWidth(200);    
    progressBar->setTextVisible(false);
    progressBar->setMaximum(0);
    progressBar->setMinimum(0);
    progressBar->setValue(1);

    QWidget *busyIndWidget = new QWidget();
    busyIndWidget->setGeometry(12, 50, 120, 60);
    QVBoxLayout *vbl = new QVBoxLayout(busyIndWidget);
    QLabel *label = new QLabel("**Searching**");
    vbl->addWidget(label);
    vbl->addWidget(progressBar);
    QHBoxLayout *hbl1 = new QHBoxLayout();
    vbl->addLayout(hbl1);   
    busyIndWidget->show();        
    qApp->processEvents();
    bool err = mEngineWrapper.searchFiles();
    busyIndWidget->hide();

    // show results of file search:
    mResults = mEngineWrapper.getSearchResults();
    fileSearchResults();
}


void SearchView::loadAttributes()
{
    // get settings from engine;
    mAttributes = mEngineWrapper.getFileSearchAttributes();
    
    // set data:
    mSearchFileNameItem->setContentWidgetData("text", mPath);
    mSearchFileNameItem->setContentWidgetData("text", mAttributes.mSearchDir);
    mWildCardItem->setContentWidgetData("text",mAttributes.mWildCards);
    mHasString->setContentWidgetData("text", mAttributes.mTextInFile);

    mRecurse->setContentWidgetData("selected",mAttributes.mRecurse);

    mMinSize->setContentWidgetData("text",mAttributes.mMinSize);
    mMaxSize->setContentWidgetData("text", mAttributes.mMaxSize);

    mMinDate->setContentWidgetData("text", mAttributes.mMinDate);
    mMaxDate->setContentWidgetData("text", mAttributes.mMaxDate);
}


void SearchView::fileSearchResults()
{
    HbDialog *dialog = new HbDialog();
    dialog->setDismissPolicy(HbPopup::TapOutside);
    dialog->setTimeout(HbPopup::NoTimeout);

    // Create a list and some simple content for it
    HbListWidget *resultsList = new HbListWidget();
    resultsList->addItem(QString("%1 entries found").arg(mResults.mNumberOfFoundFiles));
    for (int i = 0; i < mResults.mFoundFilesList->size(); i++) {
        resultsList->addItem(QString("%1").arg(mResults.mFoundFilesList->at(i)));
        }
    dialog->setContentWidget(resultsList);    
    dialog->open();
}

/**
 * Reads form items and saves values of them into member variable mAttributes
 */
void SearchView::readFormItems()
{
    mAttributes.mSearchDir  = mSearchFileNameItem->contentWidgetData("text").toString();
    mAttributes.mWildCards  = mWildCardItem->contentWidgetData("text").toString();
    mAttributes.mTextInFile = mHasString->contentWidgetData("text").toString();
    mAttributes.mMinSize    = mMinSize->contentWidgetData("text").toInt();
    mAttributes.mMaxSize    = mMaxSize->contentWidgetData("text").toInt();
    mAttributes.mMinDate    = mMinDate->contentWidgetData("text").toDate();
    mAttributes.mMaxDate    = mMaxDate->contentWidgetData("text").toDate();
    mAttributes.mRecurse    = mRecurse->contentWidgetData("selected").toBool();
}
