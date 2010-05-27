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

#ifndef ENGINEWRAPPER_H_
#define ENGINEWRAPPER_H_

#include <QObject>

#include <memspysession.h>

typedef quint64 ProcessId;
typedef quint64 ThreadId;

enum ThreadPriority
{
	ThreadPriorityNull=(-30),
	ThreadPriorityMuchLess=(-20),
	ThreadPriorityLess=(-10),
	ThreadPriorityNormal=0,
	ThreadPriorityMore=10,
	ThreadPriorityMuchMore=20,
	ThreadPriorityRealTime=30,
	ThreadPriorityAbsoluteVeryLow=100,
	ThreadPriorityAbsoluteLowNormal=150,
	ThreadPriorityAbsoluteLow=200,
	ThreadPriorityAbsoluteBackgroundNormal=250,
	ThreadPriorityAbsoluteBackground=300,
	ThreadPriorityAbsoluteForegroundNormal=350,
	ThreadPriorityAbsoluteForeground=400,
	ThreadPriorityAbsoluteHighNormal=450,
	ThreadPriorityAbsoluteHigh=500,
	ThreadPriorityAbsoluteRealTime1=810,
	ThreadPriorityAbsoluteRealTime2=820,
	ThreadPriorityAbsoluteRealTime3=830,
	ThreadPriorityAbsoluteRealTime4=840,
	ThreadPriorityAbsoluteRealTime5=850,
	ThreadPriorityAbsoluteRealTime6=860,
	ThreadPriorityAbsoluteRealTime7=870, 
	ThreadPriorityAbsoluteRealTime8=880
};

enum KernelObjectType
{
	KernelObjectTypeUnknown = -1,
	KernelObjectTypeThread = 0,
	KernelObjectTypeProcess,
	KernelObjectTypeChunk,
	KernelObjectTypeLibrary,
	KernelObjectTypeSemaphore,
	KernelObjectTypeMutex,
	KernelObjectTypeTimer,
	KernelObjectTypeServer,
	KernelObjectTypeSession,
	KernelObjectTypeLogicalDevice,
	KernelObjectTypePhysicalDevice,
	KernelObjectTypeLogicalChannel,
	KernelObjectTypeChangeNotifier,
	KernelObjectTypeUndertaker,
	KernelObjectTypeMsgQueue,
	KernelObjectTypePropertyRef,
	KernelObjectTypeCondVar
};

enum ThreadInfoType
{
	ThreadInfoTypeGeneral = EMemSpyThreadInfoItemTypeGeneralInfo,
	ThreadInfoTypeHeap = EMemSpyThreadInfoItemTypeHeap,
	ThreadInfoTypeStack = EMemSpyThreadInfoItemTypeStack,
	ThreadInfoTypeChunk = EMemSpyThreadInfoItemTypeChunk,
	ThreadInfoTypeCodeSeg = EMemSpyThreadInfoItemTypeCodeSeg,
	ThreadInfoTypeOpenFiles = EMemSpyThreadInfoItemTypeOpenFiles,
	ThreadInfoTypeActiveObjects = EMemSpyThreadInfoItemTypeActiveObject,
	ThreadInfoTypeOwnedThreadHandles = EMemSpyThreadInfoItemTypeOwnedThreadHandles,
	ThreadInfoTypeOwnedProcessHandles = EMemSpyThreadInfoItemTypeOwnedProcessHandles,
	ThreadInfoTypeServer = EMemSpyThreadInfoItemTypeServer,
	ThreadInfoTypeSession = EMemSpyThreadInfoItemTypeSession,
	ThreadInfoTypeSemaphore = EMemSpyThreadInfoItemTypeSemaphore,
	ThreadInfoTypeOtherThreads = EMemSpyThreadInfoItemTypeOtherThreads,
	ThreadInfoTypeOtherProcesses = EMemSpyThreadInfoItemTypeOtherProcesses,
	ThreadInfoTypeMutex = EMemSpyThreadInfoItemTypeMutex,
	ThreadInfoTypeTimer = EMemSpyThreadInfoItemTypeTimer,
	ThreadInfoTypeChannel = EMemSpyThreadInfoItemTypeLogicalChannel,
	ThreadInfoTypeChangeNotifier = EMemSpyThreadInfoItemTypeChangeNotifier,
	ThreadInfoTypeUndertaker = EMemSpyThreadInfoItemTypeUndertaker,
	ThreadInfoTypeMessageQueue = EMemSpyThreadInfoItemTypeMessageQueue,
	ThreadInfoTypeConditionalVariable = EMemSpyThreadInfoItemTypeConditionalVariable,
	ThreadInfoTypeLDD = EMemSpyThreadInfoItemTypeLDD,
	ThreadInfoTypePDD = EMemSpyThreadInfoItemTypePDD,
};

class MemSpyProcess
{
public:
	MemSpyProcess(CMemSpyApiProcess* process)
		: mProcess(process)
	{}
	
	virtual  ~MemSpyProcess() { delete mProcess;	}
	
	ProcessId id() const { return mProcess->Id(); }
	
	QString name() const { return QString((QChar*) mProcess->Name().Ptr(), mProcess->Name().Length()); }
	
	
private:
	CMemSpyApiProcess *mProcess;
};

class MemSpyThread
{
public:
	MemSpyThread(CMemSpyApiThread* thread)
		: mThread(thread)
	{}
	
	virtual ~MemSpyThread() { delete mThread;	}
	
	ThreadId id() const { return mThread->Id(); }
	
	QString name() const { return QString((QChar*) mThread->Name().Ptr(), mThread->Name().Length()); }
	
	int priority() const { return mThread->ThreadPriority(); }
	
	
private:
	CMemSpyApiThread *mThread;
};

class MemSpyKernelObjectType
{
public:
	MemSpyKernelObjectType(CMemSpyApiKernelObject* type)
		: mType(type)
	{}
	
	virtual ~MemSpyKernelObjectType() { delete mType; }
	
	int id() const { return mType->Type(); }
	
	QString name() const { return QString((QChar*) mType->Name().Ptr(), mType->Name().Length()); }
	
	int count() const { return mType->Count(); }
	
	qint64 size() const { return mType->Size(); }
	
private:
	CMemSpyApiKernelObject *mType;
};

class MemSpyKernelObject
{
public:
	MemSpyKernelObject(CMemSpyApiKernelObjectItem* object)
		: mObject(object)
	{}
	
	virtual ~MemSpyKernelObject() { delete mObject; }
	
	int type() const { return mObject->Type(); }
	
	QString name() const { return QString::fromLatin1((char*)mObject->Name().Ptr(), mObject->Name().Length()); }
	
	QString nameDetail() const { return QString::fromLatin1((char*)mObject->NameDetail().Ptr(), mObject->NameDetail().Length()); }
	
	int accessCount() const { return mObject->AccessCount(); }
	
	int uniqueId() const { return mObject->UniqueID(); }
	
	unsigned int protection() const { return mObject->Protection(); }
	
	unsigned int addressOfKernelOwner() const { return reinterpret_cast<unsigned int>(mObject->AddressOfKernelOwner()); }
	
	unsigned int kernelAddress() const { return reinterpret_cast<unsigned int>(mObject->Handle()); }
	
	unsigned int addressOfOwningProcess() const { return reinterpret_cast<unsigned int>(mObject->AddressOfOwningProcess()); }
	
	int id() const { return mObject->Id(); }
	
	int priority() const { return mObject->Priority(); }
	
	QString nameOfOwner() const { return QString::fromLatin1((char*)mObject->NameOfOwner().Ptr(), mObject->NameOfOwner().Length()); }
	
	unsigned int creatorId() const { return mObject->CreatorId(); }
	
	int attributes() const { return mObject->Attributes(); }
	
	unsigned int addressOfDataBssStackChunk() const { return reinterpret_cast<unsigned int>(mObject->AddressOfDataBssStackChunk()); }
	
	unsigned int securityZone() const { return mObject->SecurityZone(); }
	
	unsigned int size() const { return mObject->Size(); }
	
	unsigned int maxSize() const { return mObject->MaxSize(); }
	
	unsigned int bottom() const { return mObject->Bottom(); }
	
	unsigned int top() const { return mObject->Top(); }
	
	unsigned int startPos() const { return mObject->StartPos(); }
	
	unsigned int controllingOwner() const { return mObject->ControllingOwner(); }
	
	unsigned int restrictions() const { return mObject->Restrictions(); }
	
	unsigned int mapAttr() const { return mObject->Restrictions(); }
	
	unsigned int chunkType() const { return mObject->ChunkType(); }
	
	int mapCount() const { return mObject->MapCount(); }
	
	unsigned int state() const { return mObject->State(); }
	
	unsigned int addressOfCodeSeg() const { return reinterpret_cast<unsigned int>(mObject->AddressOfCodeSeg()); }
	
	unsigned int resetting() const { return mObject->Resetting(); }
	
	unsigned int order() const { return mObject->Order(); }
	
	QString version() const { return QString((QChar*) mObject->Version().Name().Ptr(), mObject->Version().Name().Length()); }
	
	unsigned int parseMask() const { return mObject->ParseMask(); }
	
	unsigned int unitsMask() const { return mObject->UnitsMask(); }
	
	unsigned int changes() const { return mObject->Changes(); }
	
	int count() const { return mObject->Count(); }
	
	int waitCount() const { return mObject->WaitCount(); }

	int sessionType() const { return mObject->SessionType(); }

	int timerType() const { return mObject->TimerType(); }
	
	int timerState() const { return mObject->TimerState(); }
	
	unsigned int addressOfOwningThread() const { return reinterpret_cast<unsigned int>(mObject->AddressOfOwningThread()); }
	
	unsigned int addressOfServer() const { return reinterpret_cast<unsigned int>(mObject->AddressOfServer()); }
	
	unsigned int svrSessionType() const { return mObject->SvrSessionType(); }
	
	int msgCount() const { return mObject->MsgCount(); }
	
	int msgLimit() const { return mObject->MsgLimit(); }
	
	unsigned int totalAccessCount() const { return mObject->TotalAccessCount(); }
	
	int openChannels() const { return mObject->OpenChannels(); }
	
private:
	CMemSpyApiKernelObjectItem *mObject;
};

class MemSpyThreadInfoItem 
{
public:
	MemSpyThreadInfoItem(CMemSpyApiThreadInfoItem *item)
		: mItem(item)
	{}
	
	virtual  ~MemSpyThreadInfoItem() { delete mItem;	}
	
	QString caption() const { return QString((QChar*) mItem->Caption().Ptr(), mItem->Caption().Length()); }
	
	QString value() const { return QString((QChar*) mItem->Value().Ptr(), mItem->Value().Length()); }
	
private:
	CMemSpyApiThreadInfoItem* mItem;
};

class EngineWrapper : public QObject
{
public:
	virtual ~EngineWrapper();
	bool initialize();
	
	QList<MemSpyProcess*> getProcesses();
	
	QList<MemSpyThread*> getThreads(ProcessId processId);
	
	QList<MemSpyThreadInfoItem*> getThreadInfo(ThreadId threadId, ThreadInfoType type);
	
	void setThreadPriority(ThreadId threadId, ThreadPriority priority);
	
	QList<MemSpyKernelObjectType*> getKernelObjectTypes();
	
	QList<MemSpyKernelObject*> getKernelObjects(int type);
	
private:
	RMemSpySession mSession;
	
};

#endif /* ENGINEWRAPPER_H_ */
