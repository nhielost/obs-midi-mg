/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include "mmg-link.h"
#include "mmg-binding.h"

uint32_t MMGLink::thread_count = 0u;

MMGLink::MMGLink(MMGBinding *parent) : QThread(parent), binding(parent)
{
	changeName(binding->objectName());
	connect(binding, &QObject::objectNameChanged, this, &MMGLink::changeName);
}

void MMGLink::changeName(const QString &name)
{
	setObjectName(QString("<%1>").arg(name));
}

void MMGLink::blog(int log_status, const QString &message) const
{
	binding->blog(log_status, "{LINK} " + message);
}

void MMGLink::establish(bool _connect)
{
	switch (binding->type()) {
		case TYPE_INPUT:
		default:
			if (_connect) {
				connect(binding->messages(0), &MMGMessage::refreshRequested, binding,
					&MMGBinding::refresh, Qt::UniqueConnection);
				connect(binding->messages(0), &MMGMessage::fulfilled, this, &MMGLink::execute,
					Qt::UniqueConnection);
			} else {
				disconnect(binding->messages(0), &MMGMessage::refreshRequested, binding,
					   &MMGBinding::refresh);
				disconnect(binding->messages(0), &MMGMessage::fulfilled, this, &MMGLink::execute);
			}

			binding->messages(0)->connectDevice(_connect);
			break;

		case TYPE_OUTPUT:
			if (_connect) {
				connect(binding->actions(0), &MMGAction::refreshRequested, binding,
					&MMGBinding::refresh, Qt::UniqueConnection);
				connect(binding->actions(0), &MMGAction::fulfilled, this, &MMGLink::execute,
					Qt::UniqueConnection);
			} else {
				disconnect(binding->actions(0), &MMGAction::refreshRequested, binding,
					   &MMGBinding::refresh);
				disconnect(binding->actions(0), &MMGAction::fulfilled, this, &MMGLink::execute);
			}

			binding->actions(0)->connectSignal(_connect);
			break;
	}
}

void MMGLink::execute(const MMGMappingTest &test)
{
	if (thread_count > 0x7fffffff) {
		blog(LOG_INFO, "FAILED: Thread count exceeded - the provided function will "
			       "not execute.");
		return;
	}

	if (binding->type() == TYPE_OUTPUT) {
		if (binding->messages()->size() < 1) {
			blog(LOG_INFO, "FAILED: No messages to send!");
			return;
		}
	} else {
		if (binding->actions()->size() < 1) {
			blog(LOG_INFO, "FAILED: No actions to execute!");
			return;
		}
	}

	if (binding->resetMode()) {
		auto link = new MMGLink(binding);
		link->changeName(QString("Thread #%1 <%2>").arg(thread_count + 1).arg(binding->objectName()));
		connect(link, &QThread::finished, &QObject::deleteLater);
		link->_test = test;
		link->start();
	} else {
		if (locked) {
			mutex.unlock();
			wait();
		}

		_test = test;
		start();
	}
}

void MMGLink::run()
{
	thread_count++;
	locked = true;
	mutex.lock();

	binding->type() == TYPE_OUTPUT ? executeOutput() : executeInput();

	mutex.unlock();
	locked = false;
	thread_count--;
}

void MMGLink::executeInput()
{
	for (MMGAction *action : *binding->actions()) {
		if (mutex.try_lock()) return;
		action->execute(_test);
	}
}

void MMGLink::executeOutput()
{
	for (MMGMessage *message : *binding->messages()) {
		if (mutex.try_lock()) return;
		message->send(_test);
	}
}
