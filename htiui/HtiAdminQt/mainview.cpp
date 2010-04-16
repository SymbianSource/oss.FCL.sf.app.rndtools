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
* Description:  Implementation of HtiAdmin main.
*
*/


#include <hbmainwindow.h>
#include <hbapplication.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hblabel.h>
#include <HbInputDialog.h>
#include <qgraphicslinearlayout.h>
#include <HbListDialog.h>
#include <hbmessagebox.h>



#include "htienginewrapper.h"
#include "mainview.h"
#include "hbtoolbar.h"

// ---------------------------------------------------------------------------

MainView::MainView(HbMainWindow &mainWindow, HtiEngineWrapper& engineWrapper):
    mMainWindow(mainWindow),
    mEngineWrapper(engineWrapper)
{
}

// ---------------------------------------------------------------------------

MainView::~MainView()
{

}

// ---------------------------------------------------------------------------

void MainView::init(HbApplication &app)
{
    this->setTitle("Hti Admin");
    createToolbar();
    createMenu(app);
    createTexts();
    
    mEngineWrapper.updateStatuses();
    
    QString version;
    mEngineWrapper.getVersionIfo(version);
    mEngineWrapper.listCommPlugins();
    
    mHtiVersionTxt->setPlainText("Hti Version: " + version);
}

// ---------------------------------------------------------------------------
void MainView::createMenu(HbApplication& app)
{
    
    HbMenu* menu = new HbMenu();
    
    if (menu != NULL) {
        
        mActionExit = menu->addAction("Exit");
        connect(mActionExit, SIGNAL(triggered()), &app, SLOT( quit() ) );

        connect(mActionStartHti, SIGNAL(triggered()), this, SLOT( startHti() ) );
        connect(mActionStopHti, SIGNAL(triggered()), this, SLOT( stopHti() ) );
        
        mActionSelectComm = menu->addAction("Select Communication");
        connect(mActionSelectComm, SIGNAL(triggered()), this, SLOT( enableComm() ) );
        
        mActionSetPriority = menu->addAction("Set Priority");
        connect(mActionSetPriority, SIGNAL(triggered()), this, SLOT( setPriority() ) );
        
        mActionEnableAuto = menu->addAction("Enable Auto Start");
        connect(mActionEnableAuto, SIGNAL(triggered()), this, SLOT( enableAutoStart() ) );
        
        mActionDisableAuto = menu->addAction("Disable Auto Start");
        connect(mActionDisableAuto, SIGNAL(triggered()), this, SLOT( disableAutoStart() ) );
        
        mActionEnableWdog = menu->addAction("Enable Watchdog");
        connect(mActionEnableWdog, SIGNAL(triggered()), this, SLOT( enableWatchdog() ) );
                
        mActionDisableWdog = menu->addAction("Disable Watchdog");
        connect(mActionDisableWdog, SIGNAL(triggered()), this, SLOT( disableWatchdog() ) );
        
        mActionEnableConsole = menu->addAction("Enable Console");
        connect(mActionEnableConsole, SIGNAL(triggered()), this, SLOT( enableConsole() ) );
                        
		mActionDisableConsole = menu->addAction("Disable Console");
		connect(mActionDisableConsole, SIGNAL(triggered()), this, SLOT( disableConsole() ) );
		
		mActionSetParameter = menu->addAction("Set Parameter");
		connect(mActionSetParameter, SIGNAL(triggered()), this, SLOT( showParamList() ) );
        
        this->setMenu(menu);
    }
    
    
    connect(&mEngineWrapper, SIGNAL(statusChanged(HtiEngineWrapper::HtiStatus)), this, SLOT(htiStatusChanged(HtiEngineWrapper::HtiStatus)));
    connect(&mEngineWrapper, SIGNAL(commSet(QString&)), this, SLOT(commStatusChanged(QString&)));
    connect(&mEngineWrapper, SIGNAL(commDetails(QString&)), this, SLOT(commDetailsChanged(QString&)));
    connect(&mEngineWrapper, SIGNAL(autostartSet(HtiEngineWrapper::AutoStartStatus)), this, SLOT(autostartStatusChanged(HtiEngineWrapper::AutoStartStatus)));
    connect(&mEngineWrapper, SIGNAL(consoleSet(bool)), this, SLOT(consoleStatusChanged(bool)));
    connect(&mEngineWrapper, SIGNAL(watchDogSet(bool)), this, SLOT(watchDogStatusChanged(bool)));
    connect(&mEngineWrapper, SIGNAL(commPluginsRetrieved(QStringList&)), this, SLOT(updatePluginInfo(QStringList&)));
    
    
}

// ---------------------------------------------------------------------------
void MainView::createTexts()
{
	QGraphicsLinearLayout* layout = new QGraphicsLinearLayout(Qt::Vertical, this);
	
	mCommDetailsTxt = new HbLabel(this);
	mHtiStatusTxt = new HbLabel(this);
	mHtiVersionTxt = new HbLabel(this);
	
	mHtiAutoStartStatusTxt = new HbLabel(this);
	mCommunicationTxt = new HbLabel(this);
	
	mHtiStatusTxt->setPlainText("Hti Status:");
	mHtiVersionTxt->setPlainText("Hti Version:");
	mHtiAutoStartStatusTxt->setPlainText("Auto Start:");
	mCommunicationTxt->setPlainText("Communication ");
		
	layout->addItem(mHtiStatusTxt);
	layout->addItem(mHtiVersionTxt);
	layout->addItem(mHtiAutoStartStatusTxt);
	layout->addItem(mCommunicationTxt);
	layout->addItem(mCommDetailsTxt);

	layout->setMaximumHeight(300);
	this->setLayout(layout);
}

// ---------------------------------------------------------------------------
void MainView::htiStatusChanged(HtiEngineWrapper::HtiStatus newStatus)
{
	switch(newStatus)
			{
			case HtiEngineWrapper::Running:
				mHtiStatusTxt->setPlainText("Hti Status: Running" );
				break;
			case HtiEngineWrapper::Stopped:
				mHtiStatusTxt->setPlainText("Hti Status: Stopped" );
				break;
			case HtiEngineWrapper::Panic:
				mHtiStatusTxt->setPlainText("Hti Status: Panic" );
				break;
			default:
				mHtiStatusTxt->setPlainText("Hti Status: Error" );
			}
	
	// Update menu items status
	bool isRunning = (newStatus == HtiEngineWrapper::Running);
    mActionStartHti->setEnabled(!isRunning);
    mActionStopHti->setEnabled(isRunning);
    mActionSelectComm->setEnabled(!isRunning);
    mActionSetPriority->setEnabled(!isRunning);
    mActionEnableAuto->setEnabled(!isRunning);
    mActionDisableAuto->setEnabled(!isRunning);
    mActionEnableWdog->setEnabled(!isRunning);
    mActionDisableWdog->setEnabled(!isRunning);
    mActionEnableConsole->setEnabled(!isRunning);
    mActionDisableConsole->setEnabled(!isRunning);
    mActionSetParameter->setEnabled(!isRunning);
}

// ---------------------------------------------------------------------------
void MainView::commStatusChanged(QString& newStatus)
{
	if (mCommunicationTxt) {
		mCommunicationTxt->setPlainText("Communication: " + newStatus);
	}
}

// ---------------------------------------------------------------------------
void MainView::commDetailsChanged(QString& newStatus)
{
	if (mCommDetailsTxt) {
		mCommDetailsTxt->setPlainText(newStatus);
	}
}

// ---------------------------------------------------------------------------
void MainView::updatePluginInfo(QStringList& pluginList)
{
	mPluginList = pluginList;
}

// ---------------------------------------------------------------------------
void MainView::autostartStatusChanged(HtiEngineWrapper::AutoStartStatus newStatus)
{
	switch(newStatus)
		{
		case HtiEngineWrapper::AutoStartDisabled:
			mHtiAutoStartStatusTxt->setPlainText("Auto Start: Disabled" );
			break;
		case HtiEngineWrapper::AutoStartEnabled:
			mHtiAutoStartStatusTxt->setPlainText("Auto Start: Enabled" );
			break;
		default:
			mHtiAutoStartStatusTxt->setPlainText("Auto Start: Unknown" );
		}
	
	bool isAutoStartEnabled = (newStatus == HtiEngineWrapper::AutoStartEnabled);
    mActionEnableAuto->setVisible(!isAutoStartEnabled);
    mActionDisableAuto->setVisible(isAutoStartEnabled);
}

// ---------------------------------------------------------------------------
void MainView::consoleStatusChanged(bool enabled)
{
    mActionEnableConsole->setVisible(!enabled);
    mActionDisableConsole->setVisible(enabled);
}

// ---------------------------------------------------------------------------
void MainView::watchDogStatusChanged(bool enabled)
{
    mActionEnableWdog->setVisible(!enabled);
    mActionDisableWdog->setVisible(enabled);
}


// ---------------------------------------------------------------------------

void MainView::createToolbar()
{
	mActionStartHti = toolBar()->addAction("Start Hti");
	mActionStopHti = toolBar()->addAction("Stop Hti");
}

// ---------------------------------------------------------------------------

void MainView::startHti()
{
	mEngineWrapper.startHti();
}

// ---------------------------------------------------------------------------

void MainView::stopHti()
{
	mEngineWrapper.stopHti();
}

// ---------------------------------------------------------------------------
void MainView::enableComm()
{
    //Get current selection
	QString currentComm;
	mEngineWrapper.getSelectedComm(currentComm);
	int curSelection = mPluginList.indexOf(currentComm, 0);
	
	QStringList selection;
	bool ok = false;
	selection = HbListDialog::getStringItems("Select Comm", mPluginList, curSelection, &ok, HbAbstractItemView::SingleSelection);
		
	if(ok){
			if(selection[0] == QString("Bt serial comm")){
				enableBTComm();
			}
			else if(selection[0] == QString("IP comm")){
				enableIPComm();
			}
			else if(selection[0] == QString("SERIAL comm")){
				enableSerialComm();
			}
			else{
				//All other comm plugins
				mEngineWrapper.enableOtherComm(selection[0]);
			}	
		}
}

// ---------------------------------------------------------------------------
void MainView::enableSerialComm()
{
	bool ok = false;
    QString str = HbInputDialog::getText(
					"Set Comm Port number ",
					"",
					&ok);
    
    if(ok){
    	mEngineWrapper.enableSerial(str);
    }
}


// ---------------------------------------------------------------------------
void MainView::enableIPComm()
{
    // Get IAPs list
    QStringList iapsList;
    mEngineWrapper.listIAPs(iapsList); 
    if(iapsList.count() == 0)
    {
        HbMessageBox msg("No IAPs for selection!", HbMessageBox::MessageTypeWarning);
        msg.exec();
        return;
    }
    
    // Get current selection
    QString curIapName;
    QString param = "IAPName";
    mEngineWrapper.getIPCfgParam(param, curIapName);
    int curSelction = iapsList.indexOf(curIapName, 0);
    
    QString iap;
	QStringList selection;
	bool ok = false;
	selection = HbListDialog::getStringItems("Select IAP:", iapsList, curSelction, &ok, HbAbstractItemView::SingleSelection);
	
	if(ok)
	{
        iap = selection[0];
	}
	else
	{
        return;
	}
	
	QStringList srcList;
	srcList <<"Listen" <<"Connect";
	
	selection = HbListDialog::getStringItems("Select IP Comm", srcList, 0, &ok, HbAbstractItemView::SingleSelection);
	
	if(ok){
		if(selection[0] == srcList[0]){
			QString port = HbInputDialog::getText(
								"Local port",
								"",
								&ok);	
		
			if(ok){
				mEngineWrapper.ipListen(port, iap);
			}
			
		}
		else{
			QString host = HbInputDialog::getText(
								"Remote Host",
								"",
								&ok);	
					
			if(ok){
				QString port = HbInputDialog::getText(
												"Remote port",
												"",
												&ok);	
			
				if(ok){
					mEngineWrapper.ipConnect(host, port, iap);
				}
			}
		}
			
	}
}

// ---------------------------------------------------------------------------
void MainView::enableBTComm()
{
	QStringList srcList;
	QStringList selection;
	 
	srcList <<"BT address" <<"BT name" <<"Search when starting" ;
	
	bool ok = false;
	selection = HbListDialog::getStringItems("", srcList, 0, &ok, HbAbstractItemView::SingleSelection);
	
	if(ok){
		if(selection[0] == srcList[0]){
			QString address = HbInputDialog::getText(
						"BT address",
						"",
						&ok);
		
			if(ok){
				mEngineWrapper.enableBtByAddress(address);	
			}
		}
		else if(selection[0] == srcList[1]){
			QString name = HbInputDialog::getText(
								"BT name",
								"",
								&ok);
				
					if(ok){
						mEngineWrapper.enableBtByName(name);	
					}
		}
		else if(selection[0] == srcList[2]){
			mEngineWrapper.btSearch();
		}
	}
}


// ---------------------------------------------------------------------------
void MainView::setPriority()
{
    // Get current priority
    bool ok = false;
    QString curPriority;
    QString param = "Priority";
    mEngineWrapper.getHtiCfgParam(param, curPriority);
    int curSelection = curPriority.toInt(&ok);
    if(ok){
        curSelection--;
    }
    else{
    curSelection = 2;
    }
    
	QStringList srcList;
	QStringList selection;
	 
	srcList <<"Backgroung" <<"Foregound" <<"High" << "Absolute High";
	
	
	selection = HbListDialog::getStringItems("Select Hti Priority", srcList, curSelection, &ok, HbAbstractItemView::SingleSelection);
	
	if(ok){
		if(selection[0] == srcList[0]){
			mEngineWrapper.setPriorityBackground();
		}
		else if(selection[0] == srcList[1]){
					mEngineWrapper.setPriorityForeground();
		}
		else if(selection[0] == srcList[2]){
			mEngineWrapper.setPriorityHigh();
		}
		else{
			mEngineWrapper.setPriorityAbsoluteHigh();
		}	
	}
}

// ---------------------------------------------------------------------------
void MainView::enableAutoStart()
{
	mEngineWrapper.autoStartEnable(true);
}

// ---------------------------------------------------------------------------
void MainView::disableAutoStart()
{
	mEngineWrapper.autoStartEnable(false);
}

// ---------------------------------------------------------------------------
void MainView::enableWatchdog()
{
	mEngineWrapper.watchDogEnable(true);
}

// ---------------------------------------------------------------------------
void MainView::disableWatchdog()
{
	mEngineWrapper.watchDogEnable(false);
}

// ---------------------------------------------------------------------------
void MainView::enableConsole()
{
	mEngineWrapper.consoleEnable(true);
}

// ---------------------------------------------------------------------------
void MainView::disableConsole()
{
	mEngineWrapper.consoleEnable(false);
}

// ---------------------------------------------------------------------------
void MainView::showParamList()
{
	QStringList srcList;
	QStringList selection;
	QString value;
	QString name;
	QString cfgSelection;
	QString paramSelection;
	QString cfgHti = "Hti.cfg";
	QString cfgBtComm ="HtiBtComm.cfg";
	QString cfgSerialComm = "HtiSerialComm.cfg";
	QString cfgIPComm = "HtiIPComm.cfg";
	 
	srcList <<cfgHti <<cfgBtComm <<cfgSerialComm << cfgIPComm;
	
	bool ok = false;
	selection = HbListDialog::getStringItems("Select cfg file to modify", srcList, 0, &ok, HbAbstractItemView::SingleSelection);
	
	if(ok){
        cfgSelection = selection[0];
        srcList.clear();
	    if(cfgSelection == cfgHti){
	    srcList <<"CommPlugin" <<"MaxMsgSize" <<"MaxQueueSize" <<"MaxHeapSize"<<"Priority"
	            <<"ShowConsole"<<"MaxWaitTime"<<"StartUpDelay"<<"EnableHtiWatchDog"
	            <<"EnableHtiAutoStart"<<"ShowErrorDialogs"<<"ReconnectDelay";
	    }
	    else if(cfgSelection == cfgBtComm){
	        srcList <<"BtDeviceName" <<"BtDeviceName";
	    }
	    else if(cfgSelection == cfgSerialComm){
            srcList <<"CommPort" <<"DataRate"<<"Parity"<<"DataBits"<<"StopBits"<<"SendDelay"<<"Handshake";
	    }
	    else{
	    srcList <<"IAPName"<<"LocalPort"<<"RemoteHost"<<"RemotePort"<<"ConnectTimeout";
	    }
	    
	    selection = HbListDialog::getStringItems("Select a parameter name in" + cfgSelection, srcList, 0, &ok, HbAbstractItemView::SingleSelection);
	}
    
    if(ok){
        name = selection[0];
        value = HbInputDialog::getText("Value for paramater " + name, "", &ok);
    }
    
    if(ok){
       if(cfgSelection == cfgHti){
            mEngineWrapper.setHtiCfgParam(name, value);
        }
        else if(cfgSelection == cfgBtComm){
            mEngineWrapper.setBtCfgParam(name, value);
        }
        else if(cfgSelection == cfgSerialComm){
            mEngineWrapper.setSerialCfgParam(name, value);
        }
        else{
            mEngineWrapper.setIPCfgParam(name, value);
        }   
    }
}


