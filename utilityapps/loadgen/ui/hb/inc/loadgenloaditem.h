/*
 * LoadGenLoadItem.h
 *
 *  Created on: Aug 5, 2010
 *      Author: sopirbo
 */

#ifndef LOADGENLOADITEM_H_
#define LOADGENLOADITEM_H_

#include <qstandarditemmodel.h>

class LoadGenLoadItem : public QStandardItem
	{
public:
	LoadGenLoadItem();
	virtual ~LoadGenLoadItem();
	
	int type () const;
	QVariant data(int role = Qt::UserRole + 1) const;
	void setData(const QVariant &value, int role = Qt::UserRole + 1);
	
	bool getExpandedState();
	void setExpanded( bool state );
	
	bool getSuspendedState();
	void setSuspended( bool state );
	
private:
	bool mExpanded;
	bool mSuspended;
	};

#endif /* LOADGENLOADITEM_H_ */
