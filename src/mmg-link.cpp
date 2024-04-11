/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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
#include "mmg-midi.h"

using namespace MMGUtils;

short MMGLink::thread_count = 0;

MMGLink::MMGLink(MMGBinding *parent) : QThread(parent), binding(parent), incoming_message(new MMGMessage(this))
{
	setObjectName(QString("<%1>").arg(binding->objectName()));
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
			if (!binding->messages(0)->device()) break;

			if (_connect) {
				connect(binding->messages(0)->device(), &MMGMIDIPort::messageReceived, this,
					&MMGLink::messageReceived, Qt::UniqueConnection);
			} else {
				disconnect(binding->messages(0)->device(), &MMGMIDIPort::messageReceived, this,
					   &MMGLink::messageReceived);
			}
			break;

		case TYPE_OUTPUT:
			if (_connect) {
				connect(binding->actions(0), &MMGAction::fulfilled, this, &MMGLink::actionFulfilled,
					Qt::UniqueConnection);
			} else {
				disconnect(binding->actions(0), &MMGAction::fulfilled, this, &MMGLink::actionFulfilled);
			}
			binding->actions(0)->connectSignals(_connect);
			break;
	}
}

void MMGLink::execute()
{
	if (binding->resetMode()) {
		if (thread_count > 0xff) {
			blog(LOG_INFO, "Thread count exceeded - the provided function will not execute.");
			return;
		}

		auto link = new MMGLink(binding);
		link->setObjectName(QString("<%1> (thread %2)").arg(binding->objectName()).arg(thread_count + 1));
		connect(link, &QThread::finished, &QObject::deleteLater);
		link->start();
	} else {
		if (locked) {
			mutex.unlock();
			wait();
		}

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
		action->execute(incoming_message);
	}

	binding->toggle();
}

void MMGLink::executeOutput()
{
	MMGMessage *applied_message = new MMGMessage;

	for (int i = 0; i < binding->messages()->size(); ++i) {
		if (mutex.try_lock()) return;

		binding->messages(i)->copy(applied_message);
		applied_message->applyValues(incoming_nums.size() < i ? incoming_nums[i] : incoming_nums.last());
		applied_message->send();
	}

	binding->toggle();
	delete applied_message;
}

void MMGLink::messageReceived(const MMGSharedMessage &incoming)
{
	MMGMessage *checked_message = binding->messages(0);
	if (!checked_message->acceptable(incoming.get())) return;

	if (binding->actions()->size() < 1) {
		blog(LOG_INFO, "FAILED: No actions to execute!");
		return;
	}

	checked_message->copy(incoming_message);
	incoming_message->type().setValue(incoming->type().value());
	incoming_message->channel().setValue(incoming->channel().value());
	incoming_message->note().setValue(incoming->note().value());
	incoming_message->value().setValue(incoming->value().value());

	execute();
}

void MMGLink::actionFulfilled(const MMGNumberList &values)
{
	if (binding->messages()->size() < 1) {
		blog(LOG_INFO, "FAILED: No messages to send!");
		return;
	}
	if (values.isEmpty()) {
		blog(LOG_INFO, "FAILED: Values not found. Report this as a bug.");
		return;
	}

	incoming_nums = values;
	execute();
}
