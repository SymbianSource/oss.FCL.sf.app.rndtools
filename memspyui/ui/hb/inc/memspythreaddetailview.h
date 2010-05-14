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

#ifndef MEMSPYTHREADDETAILVIEW_H_
#define MEMSPYTHREADDETAILVIEW_H_

#include "memspyview.h"
#include "enginewrapper.h"

class HbMenu;

class MemSpyThreadDetailView : public MemSpyView
{
	Q_OBJECT
	
public:
	MemSpyThreadDetailView(EngineWrapper &engine, ViewManager &viewManager) 
		: MemSpyView(engine, viewManager) {}
protected:
	virtual void initialize(const QVariantMap& params);
	virtual HbMenu* createToolMenu();

private slots:
	void changePriority();
	
private:
	ThreadId mThreadId;
	HbMenu *mPriorityMenu;
};

#endif /* MEMSPYTHREADDETAILVIEW_H_ */
