/*
 * Copyright (C) 2009-2011 Geometer Plus <contact@geometerplus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "ZLTreeNode.h"
#include "ZLTreeListener.h"

const ZLTypeId ZLTreeNode::TYPE_ID(ZLObjectWithRTTI::TYPE_ID);

ZLTreeNode::ZLTreeNode() : myParent(0), myChildIndex(-1) {
}

ZLTreeNode::~ZLTreeNode() {
	for (List::iterator it = myChildren.begin(); it != myChildren.end(); ++it) {
		delete *it;
	}
}

const ZLTypeId &ZLTreeNode::typeId() const {
	return TYPE_ID;
}

void ZLTreeNode::clear() {
	ZLTreeListener * const handler = listener();
	List::iterator it;
	for (int i = myChildren.size() - 1; i >= 0; --i) {
		it = myChildren.begin() + i;
		delete *it;
		myChildren.erase(it);
		if (handler)
			handler->onChildRemoved(this, i);
	}
}

size_t ZLTreeNode::level() const {
	size_t l = 0;
	for (ZLTreeNode *parent = myParent; parent; parent = parent->myParent) {
		++l;
	}
	return l;
}

ZLTreeListener *ZLTreeNode::listener() const {
	if (const ZLTreeListener::RootNode *node = zlobject_cast<const ZLTreeListener::RootNode*>(this))
		return &node->myListener;
	else if (myParent)
		return myParent->listener();
	else
		return 0;
}

ZLTreeNode *ZLTreeNode::parent() const {
	return myParent;
}

ZLTreeNode *ZLTreeNode::previous() const {
	if (myChildIndex == 0)
		return 0;
	return myParent->children().at(myChildIndex - 1);
}

ZLTreeNode *ZLTreeNode::next() const {
	if (myChildIndex + 1 >= myParent->children().size())
		return 0;
	return myParent->children().at(myChildIndex + 1);
}

const ZLTreeNode::List &ZLTreeNode::children() const {
	return myChildren;
}

void ZLTreeNode::requestChildren() {
}

void ZLTreeNode::insert(ZLTreeNode *node, size_t index) {
	index = std::min(index, myChildren.size());
	node->myChildIndex = index;
	List::iterator it = myChildren.insert(myChildren.begin() + index, node);
	for (; it != myChildren.end(); ++it) {
		++(*it)->myChildIndex;
	}
	if (ZLTreeListener *handler = listener())
		handler->onChildAdded(this, index);
}

void ZLTreeNode::remove(size_t index) {
	if (index >= myChildren.size())
		return;
	List::iterator it = myChildren.erase(myChildren.begin() + index);
	for (; it != myChildren.end(); ++it) {
		--(*it)->myChildIndex;
	}
	if (ZLTreeListener *handler = listener())
		handler->onChildRemoved(this, index);
}

void ZLTreeNode::updated() {
	if (ZLTreeListener *handler = listener())
		handler->onChildUpdated(this);
}