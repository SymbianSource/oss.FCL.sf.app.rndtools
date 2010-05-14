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

#ifndef MEMSPYTHREADDETAILINDEXVIEW_H_
#define MEMSPYTHREADDETAILINDEXVIEW_H_

#include "memspyview.h"
#include "enginewrapper.h"

class HbMenu;

class MemSpyThreadDetailIndexView : public MemSpyView
{
	Q_OBJECT
	
public:
	MemSpyThreadDetailIndexView(EngineWrapper &engine, ViewManager &viewManager) 
		: MemSpyView(engine, viewManager) {}
protected:
	virtual void initialize(const QVariantMap& params);
	virtual HbMenu* createToolMenu();

private slots:
	void changePriority();
	void itemClicked(const QModelIndex& index);
	
private:
	ThreadId mThreadId;
	HbMenu *mPriorityMenu;
};

#endif /* MEMSPYTHREADDETAILINDEXVIEW_H_ */
