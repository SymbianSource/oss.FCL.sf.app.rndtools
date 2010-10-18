/*
 * LoadGenListItem.h
 *
 *  Created on: Jul 29, 2010
 *      Author: sopirbo
 */

#ifndef LOADGENLISTITEM_H_
#define LOADGENLISTITEM_H_

#include "enginewrapper.h"

#include <hblistview.h>
#include <HbListViewItem>

#include <QGraphicsWidget>
#include <QtGlobal>

class HbAbstractViewItem;
class HbLabel;
class HbPushButton;
class HbWidget;

class QGraphicsLinearLayout;
class QModelIndex;
class QGraphicsItem;
class EngineWrapper;

class LoadGenListItem : public HbListViewItem
	{
	Q_OBJECT
public:
	LoadGenListItem( EngineWrapper& engWrapp , QGraphicsItem *parent = 0 );
	
	//from HbListViewItem
	virtual HbAbstractViewItem* createItem();
	virtual void updateChildItems();
	virtual ~LoadGenListItem();
	
	void LoadGenListItem::setTransientState(const QHash<QString, QVariant> &state);
	QHash<QString, QVariant> LoadGenListItem::transientState() const;

	void ChangeExpandedState();
	
signals:
	
	
public slots:
	void EditButtonClicked();
	void StopButtonClicked();
	void SuspendButtonClicked();
private:
	void init();
	void initButtonLayout();
	void initLayoutShort();

private:
	QGraphicsLinearLayout *hLayout;
	QGraphicsLinearLayout *hButtonLayout;
	QGraphicsLinearLayout *vLayout;
	HbLabel *mItemText;
	HbLabel *mIcon;
	HbPushButton *mEditButton;
	HbPushButton *mSuspendButton;
	HbPushButton *mStopButton;
	EngineWrapper& mEngineWrapper;
	bool mExpanded;
	};

#endif /* LOADGENLISTITEM_H_ */
