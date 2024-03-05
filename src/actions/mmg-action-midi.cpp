/*
obs-midi-mg
Copyright (C) 2022-2023 nhielost <nhielost@gmail.com>

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

#include "mmg-action-midi.h"
#include "../mmg-config.h"

using namespace MMGUtils;

// MMGActionMIDI
MMGActionMIDI::MMGActionMIDI(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj), messages(new MMGMessageManager(this)), _queue(new MMGConnectionQueue(this))
{
	if (json_obj.contains("device")) {
		messages->add(json_obj);
	} else {
		messages->load(json_obj["messages"].toArray());
	}

	blog(LOG_DEBUG, "Action created.");
}

void MMGActionMIDI::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	messages->json("messages", json_obj);
}

void MMGActionMIDI::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionMIDI *>(dest);
	if (!casted) return;

	casted->messages->clear();
	for (MMGMessage *message : *messages)
		message->copy(casted->messages->add());
}

void MMGActionMIDI::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	message_display = new MMGMessageDisplay(display());
	display()->setFields(message_display);
}

void MMGActionMIDI::setActionParams()
{
	message_display->setStorage(messages->at(0));
	//message_display->display();
}

void MMGActionMIDI::execute(const MMGMessage *midi) const
{
	QScopedPointer<MMGMessage> sent_message(new MMGMessage);

	for (MMGMessage *message : *messages) {
		message->copy(sent_message.data());

		if (sent_message->type().state() == STATE_MIDI) sent_message->type() = midi->type();
		sent_message->channel().chooseOther(midi->channel());
		sent_message->note().chooseOther(midi->note());
		sent_message->value().chooseOther(midi->value());

		sent_message->send();
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionMIDI::connectSignals(bool _connect)
{
	_queue->disconnectQueue();
	if (!_connect) return;

	if (type() == TYPE_OUTPUT) _queue->connectQueue();
}
// End MMGActionMIDI

// MMGConnectionQueue
MMGConnectionQueue::MMGConnectionQueue(MMGActionMIDI *parent) : QObject(parent)
{
	action = parent;
}

void MMGConnectionQueue::blog(int log_status, const QString &message) const
{
	action->blog(log_status, message);
}

void MMGConnectionQueue::connectQueue()
{
	resetQueue();
	if (queue.isEmpty()) return;

	connect(action->messages->at(0)->device(), &MMGMIDIPort::messageReceived, this,
		&MMGConnectionQueue::messageFound);
}

void MMGConnectionQueue::disconnectQueue()
{
	if (queue.isEmpty()) return;
	disconnect(queue.head()->device(), &MMGMIDIPort::messageReceived, this, nullptr);
}

void MMGConnectionQueue::resetQueue()
{
	queue.clear();

	for (MMGMessage *message : *action->messages)
		queue.enqueue(message);
}

void MMGConnectionQueue::resetConnections()
{
	disconnectQueue();
	if (action->type() == TYPE_OUTPUT) connectQueue();
}

void MMGConnectionQueue::messageFound(const MMGSharedMessage &incoming)
{
	MMGMessage *message = queue.head();
	if (!message) {
		disconnectQueue();
		queue.dequeue();
		messageFound(incoming); // Try the next message
		return;
	}

	if (!message->acceptable(incoming.get())) return;

	message->toggle();
	disconnectQueue();

	message = queue.dequeue();
	connect(message->device(), &MMGMIDIPort::messageReceived, this, &MMGConnectionQueue::messageFound);

	if (queue.isEmpty()) {
		emit action->triggerEvent();
		resetQueue();
	}
}
// End MMGConnectionQueue
