/*
 * Copyright (C) 2004-2012 Geometer Plus <contact@geometerplus.com>
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

#include "ZLQtTime.h"

#include <QtCore/QMetaType>

void ZLQtTimeManager::createInstance(){
  ourInstance = new ZLQtTimeManager();
  qRegisterMetaType< shared_ptr<ZLRunnable> >("shared_ptr<ZLRunnable>");
}

void ZLQtTimeManager::addTask(shared_ptr<ZLRunnable> task, int interval) {
	QMetaObject::invokeMethod(this, "addTaskSlot", Qt::AutoConnection, Q_ARG(shared_ptr<ZLRunnable>, task), Q_ARG(int, interval));
}

void ZLQtTimeManager::removeTaskInternal(shared_ptr<ZLRunnable> task) {
	QMetaObject::invokeMethod(this, "removeTaskInternalSlot", Qt::AutoConnection, Q_ARG(shared_ptr<ZLRunnable>, task));
}

void ZLQtTimeManager::addTaskSlot(shared_ptr<ZLRunnable> task, int interval) {
	removeTask(task);
	if ((interval > 0) && !task.isNull()) {
		int id = startTimer(interval);
		myTimers[task] = id;
		myTasks[id] = task;
	}
}

void ZLQtTimeManager::removeTaskInternalSlot(shared_ptr<ZLRunnable> task) {
	std::map<shared_ptr<ZLRunnable>,int>::iterator it = myTimers.find(task);
	if (it != myTimers.end()) {
		killTimer(it->second);
		myTasks.erase(myTasks.find(it->second));
		myTimers.erase(it);
	}
}

void ZLQtTimeManager::timerEvent(QTimerEvent *event) {
	myTasks[event->timerId()]->run();
}
