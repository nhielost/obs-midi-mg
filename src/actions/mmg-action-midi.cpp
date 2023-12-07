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
MMGActionMIDI::MMGActionMIDI(MMGActionManager *parent, const QJsonObject &json_obj) : MMGAction(parent, json_obj)
{
	midi_binding = new MMGBinding(nullptr, json_obj);
	midi_binding->setParent(this);

	MMGDeviceList devices = midi_binding->usedDevices();
	MMGMessageList messages = midi_binding->usedMessages();
	midi_binding->setType(TYPE_OUTPUT);
	type_check = type();
	midi_binding->setUsedDevices(devices);
	midi_binding->setUsedMessages(messages);

	_queue = new MMGConnectionQueue(this);

	if (json_obj.contains("device")) {
		QJsonObject message_obj = json_obj;
		MMGDevice *device = manager(device)->find(json_obj["device"].toString());
		if (device) midi_binding->setUsedDevices({device});
	}

	if (json_obj.contains("channel")) {
		QJsonObject message_obj = json_obj;
		message_obj["name"] = mmgtr("Actions.Composite.MIDIMessageName");
		midi_binding->setUsedMessages({manager(message)->add(message_obj)});
	}

	blog(LOG_DEBUG, "Action created.");
}

void MMGActionMIDI::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	QJsonArray devices_array;
	for (MMGDevice *device : midi_binding->usedDevices())
		devices_array += device->name();

	json_obj["devices"] = devices_array;

	QJsonArray messages_array;
	for (MMGMessage *message : midi_binding->usedMessages())
		messages_array += message->name();

	json_obj["messages"] = messages_array;
}

void MMGActionMIDI::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionMIDI *>(dest);
	if (!casted) return;

	midi_binding->copy(casted->midi_binding);
}

void MMGActionMIDI::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	binding_display = new MMGBindingDisplay(display(), false);
	binding_display->layout()->setContentsMargins(10, 10, 10, 10);
	binding_display->setStorage(midi_binding);
	connect(binding_display, &MMGBindingDisplay::edited, this, &MMGActionMIDI::editClicked);
	display()->setFields(binding_display);
}

void MMGActionMIDI::editClicked(int page)
{
	emit display()->editRequest(midi_binding, page);
}

void MMGActionMIDI::setComboOptions(QComboBox *sub)
{
	sub->addItems(subModuleTextList({"Message"}));
}

void MMGActionMIDI::setActionParams()
{
	if (type() != type_check) { // To clear on type change
		midi_binding->setType(TYPE_INPUT);
		midi_binding->setType(TYPE_OUTPUT);
	}
	type_check = type();
	binding_display->display();
}

void MMGActionMIDI::execute(const MMGMessage *midi) const
{
	QScopedPointer<MMGMessage> sent_message(new MMGMessage);

	for (MMGDevice *device : midi_binding->usedDevices()) {
		if (!device->isActive(TYPE_OUTPUT)) {
			blog(LOG_INFO, QString("Output device <%1> is not connected. (Is the output device enabled?)")
					       .arg(device->name()));
			continue;
		}

		for (MMGMessage *message : midi_binding->usedMessages()) {
			message->copy(sent_message.data());

			if (sent_message->type().state() == STATE_MIDI) sent_message->type() = midi->type();
			sent_message->channel().chooseOther(midi->channel());
			sent_message->note().chooseOther(midi->note());
			sent_message->value().chooseOther(midi->value());

			device->sendMessage(sent_message.data());
		}
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionMIDI::connectOBSSignals()
{
	disconnectOBSSignals();
	if (type() == TYPE_OUTPUT) _queue->connectQueue();
}

void MMGActionMIDI::disconnectOBSSignals()
{
	_queue->disconnectQueue();
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

	for (MMGDevice *device : action->midi_binding->usedDevices())
		connect(device, &MMGMIDIPort::messageReceived, this, &MMGConnectionQueue::messageFound);
}

void MMGConnectionQueue::disconnectQueue()
{
	for (MMGDevice *device : action->midi_binding->usedDevices())
		disconnect(device, &MMGMIDIPort::messageReceived, this, nullptr);
}

void MMGConnectionQueue::resetQueue()
{
	queue.clear();

	for (MMGMessage *message : action->midi_binding->usedMessages())
		queue.enqueue(message);
}

void MMGConnectionQueue::resetConnections()
{
	disconnectQueue();
	if (action->type() == TYPE_OUTPUT) connectQueue();
}

void MMGConnectionQueue::messageFound(const MMGSharedMessage &incoming)
{
	MMGMessage *message = manager(message)->find(queue.head()->name());
	if (!message) {
		queue.dequeue();
		messageFound(incoming); // Try the next message
		return;
	}

	if (!message->acceptable(incoming.get())) return;

	queue.dequeue();
	message->toggle();

	if (queue.isEmpty()) {
		emit action->eventTriggered();
		resetQueue();
	}
}
// End MMGConnectionQueue
