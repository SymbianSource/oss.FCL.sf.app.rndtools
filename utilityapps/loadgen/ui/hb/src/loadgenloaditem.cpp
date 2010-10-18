/*
 * LoadGenLoadItem.cpp
 *
 *  Created on: Aug 5, 2010
 *      Author: sopirbo
 */

#include "LoadGenLoadItem.h"

LoadGenLoadItem::LoadGenLoadItem()
	{
	mExpanded = false;
	mSuspended = false;	
	// TODO Auto-generated constructor stub

	}

bool LoadGenLoadItem::getExpandedState()
	{
	return mExpanded;
	}

void LoadGenLoadItem::setExpanded( bool state )
	{
	mExpanded = state;
	QVariant variant = QVariant( mExpanded );
	setData( variant );
	}

bool LoadGenLoadItem::getSuspendedState()
	{
	return mSuspended;
	}

void LoadGenLoadItem::setSuspended( bool state )
	{
	int role = Qt::UserRole + 2;
	mSuspended = state;
	QVariant variant = QVariant( mSuspended );
	setData( variant, role );
	}

int LoadGenLoadItem::type () const
	{
	return QStandardItem::UserType;
	}

QVariant LoadGenLoadItem::data( int role ) const
	{
	QVariant variant;
	if(role == Qt::UserRole + 1)
		{
		variant = QVariant(mExpanded);
		}
	if(role == Qt::UserRole + 2)
		{
		variant = QVariant(mSuspended);
		}
	else
		{
		return QStandardItem::data( role );
		}
	return variant; 
	}

void LoadGenLoadItem::setData( const QVariant &value, int role )
{
	QVariant variantvalue;
    if (role == Qt::UserRole + 1)
    	{
		variantvalue = QVariant( mExpanded );
        QStandardItem::setData( variantvalue , role );
        return;
    	}
    if (role == Qt::UserRole + 2)
    	{
    	variantvalue = QVariant( mSuspended );
        QStandardItem::setData( variantvalue , role );
        return;
    	}
    QStandardItem::setData( value, role );
}


LoadGenLoadItem::~LoadGenLoadItem()
	{
	// TODO Auto-generated destructor stub
	}
