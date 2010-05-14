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

#include "enginewrapper.h"
#include <QMessageBox>

EngineWrapper::~EngineWrapper()
{
	mSession.Close();
}

bool EngineWrapper::initialize()
{
	return mSession.Connect() == KErrNone;
}

QList<MemSpyProcess*> EngineWrapper::getProcesses()
{
	QList<MemSpyProcess*> result;
	
	RArray<CMemSpyApiProcess*> proc;
	TRAPD(error, mSession.GetProcessesL(proc));
	if (error == KErrNone)
		for(TInt i=0; i<proc.Count(); i++)
			result.append(new MemSpyProcess(proc[i]));
	
	return result;
}

QList<MemSpyThread*> EngineWrapper::getThreads(ProcessId processId)
{
	QList<MemSpyThread*> result;
	
	RArray<CMemSpyApiThread*> proc;
	TRAPD(error, mSession.GetThreadsL(processId, proc));
	if (error == KErrNone)
		for(TInt i=0; i<proc.Count(); i++)
			result.append(new MemSpyThread(proc[i]));
	
	return result;
}

void EngineWrapper::setThreadPriority(ThreadId threadId, ThreadPriority priority)
{
	TRAPD(error, mSession.SetThreadPriorityL(threadId, priority));
}

QList<MemSpyKernelObjectType*> EngineWrapper::getKernelObjectTypes()
{
	QList<MemSpyKernelObjectType*> result;
		
	RArray<CMemSpyApiKernelObject*> types;
	TInt error = mSession.GetKernelObjects(types);
	if (error == KErrNone)
		for(TInt i=0; i<types.Count(); i++)
			result.append(new MemSpyKernelObjectType(types[i]));
	
	return result;
}

QList<MemSpyKernelObject*> EngineWrapper::getKernelObjects(int type)
{
	QList<MemSpyKernelObject*> result;
		
	RArray<CMemSpyApiKernelObjectItem*> objects;
	TInt error = mSession.GetKernelObjectItems(objects, 
			static_cast<TMemSpyDriverContainerType>(type));
	if (error == KErrNone)
		for(TInt i=0; i<objects.Count(); i++)
			result.append(new MemSpyKernelObject(objects[i]));
	
	return result;
}
