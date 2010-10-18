/*
 * LoadGenListItem.cpp
 *
 *  Created on: Jul 29, 2010
 *      Author: sopirbo
 */

#include "loadgenlistitem.h"


#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbabstractitemview.h>

#include <QGraphicsLinearLayout>
#include <QRectF>
#include <QPainter>


LoadGenListItem::LoadGenListItem( EngineWrapper& engWrapp, QGraphicsItem *parent ) : HbListViewItem( parent ),
		mEngineWrapper(engWrapp),
		hLayout(0),
		vLayout(0),
		hButtonLayout(0),
	    mItemText(0),
		mEditButton(0),
		mSuspendButton(0),
		mStopButton(0),
		mIcon(0),
		mExpanded(false)
	{

		
	// TODO Auto-generated constructor stub
	
	}


LoadGenListItem::~LoadGenListItem()
	{
	// TODO Auto-generated destructor stub
	}

HbAbstractViewItem *LoadGenListItem::createItem()
{
    return new LoadGenListItem(*this);
}

QHash<QString, QVariant> LoadGenListItem::transientState() const
		{

		QHash<QString,QVariant> state;
	
/*		if ( mExpanded )
			{
			state.insert( "myexpanded", mExpanded );
			}
		else
			{
			state.insert( "myexpanded", false );
			}*/
		return state;
		
		}

void LoadGenListItem::setTransientState(const QHash<QString, QVariant> &state)
	{
/*
	mExpanded = state.value("myexpanded").toBool();*/
	HbAbstractViewItem::setTransientState( state ); 
	}


void LoadGenListItem::EditButtonClicked()
	{
	mEngineWrapper.loadEdit(modelIndex().row());
	
	}

void LoadGenListItem::StopButtonClicked()
	{


	mEngineWrapper.asyncStopLoad();
	}

void LoadGenListItem::SuspendButtonClicked()
	{
	mEngineWrapper.asyncSuspendOrResumeSelectedOrHighlightedItems();



	}

void LoadGenListItem::ChangeExpandedState()
	{
	/*
	if( mExpanded ) 
		{
		mExpanded = false;
		}
	else
		{
		mExpanded=true;
		}*/
	}



void LoadGenListItem::updateChildItems( )
{
	if(!hLayout)
		{
		init();
		}

	int userRole = Qt::UserRole + 1;
	int userRole2 = Qt::UserRole + 2;
    mExpanded = qvariant_cast<bool>( modelIndex().data( userRole ) );
    
    QVariant variant = modelIndex().data(Qt::DisplayRole);
    
    QString itemText = qvariant_cast<QString>( variant );
    if(variant.isValid())
    	{
		itemText = qvariant_cast<QString>( variant );
    	}
    else
    	{
		itemText = "Text tTest";
    	}
    
    QVariant decorationRole = modelIndex().data(Qt::DecorationRole);
	if (decorationRole.isValid()) 
	   	{
	    QIcon icon = qvariant_cast<QIcon>(decorationRole);
	    mIcon->setIcon( HbIcon(icon) );
	   	}

	bool suspended = qvariant_cast<bool>( modelIndex().data( userRole2 ) );
    if( suspended )
    	{
		mSuspendButton->setText("Resume");
    	}
    else
    	{
		mSuspendButton->setText("Suspend");
    	}
    
    if( !mExpanded )
    	{
		mItemText->setTextWrapping(Hb::TextNoWrap);
		mItemText->setPlainText(itemText);
		vLayout->removeItem(hButtonLayout);
    	}
    else
    	{
		mItemText->setTextWrapping(Hb::TextWordWrap);
		mItemText->setPlainText(itemText);
		//mItemText->setHtml( itemText );
		vLayout->addItem(hButtonLayout);
    	}
    
    
}

void LoadGenListItem::initButtonLayout()
	{
	    
	    hButtonLayout = new QGraphicsLinearLayout();
	    hButtonLayout->setOrientation( Qt::Horizontal );

	    //edit button 
	    mEditButton = new HbPushButton();
	    mEditButton->setText("Edit");
	    hButtonLayout->addItem( mEditButton );
	    hButtonLayout->setAlignment( mEditButton, Qt::AlignLeft );

	    //pause button 
	    mSuspendButton = new HbPushButton();
	    mSuspendButton->setText("SuspRes");
	    hButtonLayout->addItem( mSuspendButton );
	    hButtonLayout->setAlignment( mSuspendButton , Qt::AlignLeft );
	    
	    //pause button 
	    mStopButton = new HbPushButton();
	    mStopButton->setText("Stop");
	    hButtonLayout->addItem( mStopButton );
	    hButtonLayout->setAlignment( mStopButton , Qt::AlignLeft );
	    
	 
	    connect( mEditButton, SIGNAL( clicked() ), this , SLOT( EditButtonClicked() ) );
	    connect( mStopButton, SIGNAL( clicked() ), this , SLOT( StopButtonClicked() ) );
	    connect( mSuspendButton, SIGNAL( clicked() ), this , SLOT( SuspendButtonClicked() ) );
	}


void LoadGenListItem::init()
{
	hLayout = new QGraphicsLinearLayout();
	hLayout->setContentsMargins(0, 0, 0, 0);

	hLayout->setOrientation( Qt::Horizontal );
	//hLayout->addItem( layout() );

	vLayout = new QGraphicsLinearLayout();
	vLayout->setOrientation( Qt::Vertical );

	//icon
	mIcon = new HbLabel();
	hLayout->addItem( mIcon );
	hLayout->setAlignment( mIcon, Qt::AlignLeft );
	//hLayout->setStretchFactor( mIcon, 1 );
	
    //text
    mItemText = new HbLabel();
    HbFontSpec fontSpecPrimary( HbFontSpec::Primary );
    mItemText->setFontSpec( fontSpecPrimary );
    vLayout->addItem( mItemText );
    vLayout->setAlignment( mItemText, Qt::AlignLeft );

    
    initButtonLayout();
    

	hLayout->addItem( vLayout );

	hLayout->setStretchFactor( vLayout, 2 );
	
	setLayout(hLayout);
	
}
