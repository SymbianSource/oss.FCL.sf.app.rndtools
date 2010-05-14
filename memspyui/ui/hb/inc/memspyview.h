/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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

#ifndef MEMSPYVIEW_H_
#define MEMSPYVIEW_H_

#include <HbView>
#include <HbListView>
#include <QVariantMap>

class EngineWrapper;
class ViewManager;

class MemSpyView : public HbView
{
	Q_OBJECT
	
public:
	MemSpyView(EngineWrapper &engine, ViewManager &viewManager);
	
	virtual ~MemSpyView();
	
protected:
	
	virtual QList<QAction*> createOutputActions();
	
	virtual HbMenu* createToolMenu();
	
	virtual HbToolBar* createToolBar();
	
	virtual bool isRefreshable() const;
	
public slots:

	virtual void initialize(const QVariantMap& params);

	virtual void refresh();
	
	void showAbout();
	
protected:
	
	HbListView mListView;
	
	EngineWrapper &mEngine;
	
	ViewManager &mViewManager;
	
private:
	HbMenu* mOutputMenu;
	HbMenu* mOutputGenInfoMenu;
	HbMenu* mOutputHeapInfoMenu;
	HbMenu* mOutputStackInfoMenu;
	HbToolBar* mToolBar;
	
};

#endif /* MEMSPYVIEW_H_ */
