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

#include "mmg-binding.h"
#include "mmg-config.h"
#include "mmg-midi.h"

using namespace MMGUtils;

#define SHORT_ENUMERATE(kind, statement) \
	for (auto kind : _##kind##s)     \
	statement

// MMGBinding
MMGBinding::MMGBinding(MMGBindingManager *parent, const QJsonObject &json_obj) : QObject(parent)
{
	thread = new MMGBindingThread(this);

	_name = json_obj["name"].toString(mmgtr("Binding.Untitled"));
	_enabled = json_obj["enabled"].toBool(true);
	_type = (DeviceType)json_obj["type"].toInt();

	for (const QJsonValue &device_name : json_obj["devices"].toArray())
		_devices += manager(device)->find(device_name.toString());
	SHORT_ENUMERATE(device, device->addConnection(this));

	for (const QJsonValue &message_name : json_obj["messages"].toArray())
		_messages += manager(message)->find(message_name.toString());
	SHORT_ENUMERATE(message, message->addConnection(this));

	for (const QJsonValue &action_name : json_obj["actions"].toArray())
		_actions += manager(action)->find(action_name.toString());
	SHORT_ENUMERATE(action, action->addConnection(this));

	if (json_obj.contains("settings")) {
		_settings = new MMGBindingSettings(nullptr, json_obj["settings"].toObject());
	} else {
		_settings = new MMGBindingSettings(nullptr);
		auto casted_settings = dynamic_cast<MMGBindingSettings *>(manager(setting)->at(1));
		if (casted_settings) casted_settings->copy(_settings);
	}
	_settings->setParent(this);
}

void MMGBinding::setType(DeviceType type)
{
	if (_type == type) return;

	_type = type;
	setUsedDevices({});
	setUsedMessages({});
	setUsedActions({});
}

void MMGBinding::json(QJsonObject &binding_obj) const
{
	binding_obj["name"] = _name;
	binding_obj["enabled"] = _enabled;
	binding_obj["type"] = type();

	QJsonArray device_arr;
	SHORT_ENUMERATE(device, device_arr += device->name());
	binding_obj["devices"] = device_arr;

	QJsonArray message_arr;
	SHORT_ENUMERATE(message, message_arr += message->name());
	binding_obj["messages"] = message_arr;

	QJsonArray action_arr;
	SHORT_ENUMERATE(action, action_arr += action->name());
	binding_obj["actions"] = action_arr;

	QJsonObject settings_obj;
	_settings->json(settings_obj);
	binding_obj["settings"] = settings_obj;
}

void MMGBinding::blog(int log_status, const QString &message) const
{
	global_blog(log_status, QString("[Bindings] <%1> %2").arg(_name).arg(message));
}

void MMGBinding::copy(MMGBinding *dest)
{
	dest->setName(_name);
	dest->setUsedDevices(_devices);
	dest->setUsedMessages(_messages);
	dest->setUsedActions(_actions);
	dest->setEnabled(_enabled);
	dest->_settings->copy(_settings);
}

void MMGBinding::setEnabled(bool val)
{
	setConnected(false);
	_enabled = val;
	setConnected(true);
}

void MMGBinding::setUsedDevices(const MMGDeviceList &devices)
{
	setConnected(false);
	SHORT_ENUMERATE(device, device->removeConnection(this));

	_devices = devices;

	setConnected(true);
	SHORT_ENUMERATE(device, device->addConnection(this));
}

void MMGBinding::setUsedMessages(const MMGMessageList &messages)
{
	SHORT_ENUMERATE(message, message->removeConnection(this));

	_messages = messages;

	SHORT_ENUMERATE(message, message->addConnection(this));
}

void MMGBinding::setUsedActions(const MMGActionList &actions)
{
	setConnected(false);
	SHORT_ENUMERATE(action, action->removeConnection(this));

	_actions = actions;

	setConnected(true);
	SHORT_ENUMERATE(action, action->addConnection(this));
}

void MMGBinding::replaceAction(MMGAction *action)
{
	auto sender_action = dynamic_cast<MMGAction *>(sender());
	if (!sender_action) return;

	setConnected(false);

	if (action->type() != type()) {
		_actions.removeOne(sender_action);
		sender_action->removeConnection(this);
	} else {
		_actions[_actions.indexOf(sender_action)] = action;
		action->addConnection(this);
	}

	setConnected(true);
}

void MMGBinding::setConnected(bool _connected)
{
	if (_connected && (!_enabled || connected)) return;

	switch (_type) {
		case TYPE_INPUT:
		default:
			for (MMGDevice *device : _devices) {
				if (_connected) {
					connect(device, &MMGMIDIPort::messageReceived, this, &MMGBinding::executeInput,
						Qt::UniqueConnection);
				} else {
					disconnect(device, &MMGMIDIPort::messageReceived, this, nullptr);
				}
			}
			break;

		case TYPE_OUTPUT:
			for (MMGAction *action : _actions) {
				if (_connected) {
					connect(action, &MMGAction::eventTriggered, this, &MMGBinding::executeOutput);
					action->connectOBSSignals();
				} else {
					disconnect(action, &MMGAction::eventTriggered, this, nullptr);
					action->disconnectOBSSignals();
				}
			}
			break;
	}

	connected = _connected;
}

void MMGBinding::executeInput(const MMGSharedMessage &incoming) const
{
	MMGBindingThread *exec_thread = _settings->resetBehavior() ? thread->createNew() : thread;
	exec_thread->restart(incoming.get());
}

void MMGBinding::executeOutput(const QList<MMGNumber> &values) const
{
	MMGBindingThread *exec_thread = _settings->resetBehavior() ? thread->createNew() : thread;
	exec_thread->restart(values);
}
// End MMGBinding

// MMGBindingThread
short MMGBindingThread::thread_count = 0;

MMGBindingThread::MMGBindingThread(MMGBinding *parent) : QThread(parent), binding(parent)
{
	incoming_message = new MMGMessage(this);
	applied_message = new MMGMessage(this);
}

void MMGBindingThread::blog(int log_status, const QString &message) const
{
	binding->blog(log_status, message);
}

void MMGBindingThread::run()
{
	thread_count++;
	locked = true;
	mutex.lock();

	binding->type() == TYPE_OUTPUT ? sendMessages() : doActions();

	mutex.unlock();
	locked = false;
	thread_count--;
}

void MMGBindingThread::sendMessages()
{
	if (binding->_messages.isEmpty()) {
		blog(LOG_INFO, "FAILED: No messages to send!");
		return;
	}
	if (incoming_values.isEmpty()) {
		blog(LOG_INFO, "FAILED: Values not found. Report this as a bug.");
		return;
	}

	int i;
	MMGNumber used_value;

	for (MMGDevice *device : binding->_devices) {
		for (i = 0; i < binding->_messages.size(); ++i) {
			// if (mutex.try_lock_for(binding->getChronoTime())) return;
			if (mutex.try_lock()) return;

			binding->_messages[i]->copy(applied_message);

			used_value = incoming_values.size() < i ? incoming_values[i] : incoming_values.last();
			applied_message->applyValues(used_value);

			device->sendMessage(applied_message);
		}
	}

	for (MMGMessage *message : binding->_messages)
		message->toggle();
	binding->_actions[0]->toggle();
}

void MMGBindingThread::doActions()
{
	if (binding->_messages.isEmpty()) {
		blog(LOG_INFO, "FAILED: No message to check for!");
		return;
	}

	MMGMessage *checked_message = binding->_messages[0];
	if (!checked_message->acceptable(incoming_message)) {
		blog(LOG_DEBUG, "Message received is not acceptable.");
		return;
	}

	checked_message->copy(applied_message);
	applied_message->type() = incoming_message->type().value();
	applied_message->channel() = incoming_message->channel().value();
	applied_message->note() = incoming_message->note().value();
	applied_message->value() = incoming_message->value().value();

	for (MMGAction *action : binding->_actions) {
		// if (mutex.try_lock_for(binding->getChronoTime())) return;
		if (mutex.try_lock()) return;
		action->execute(applied_message);
	}

	checked_message->toggle();
	for (MMGAction *action : binding->_actions)
		action->toggle();
}

void MMGBindingThread::restart(const MMGMessage *msg)
{
	if (locked) {
		mutex.unlock();
		wait();
	}
	msg->copy(incoming_message);
	start();
}

void MMGBindingThread::restart(const QList<MMGNumber> &values)
{
	if (locked) {
		mutex.unlock();
		wait();
	}
	incoming_values = values;
	start();
}

MMGBindingThread *MMGBindingThread::createNew() const
{
	if (thread_count > 0xff) {
		global_blog(LOG_INFO, "Thread count exceeded - the provided function will not execute.");
		return nullptr;
	}

	auto custom_thread = new MMGBindingThread(binding);
	connect(custom_thread, &QThread::finished, &QObject::deleteLater);
	return custom_thread;
}
// End MMGBindingThread

// MMGBindingManager
MMGBinding *MMGBindingManager::add(const QJsonObject &json_obj)
{
	return MMGManager::add(new MMGBinding(this, json_obj));
}

bool MMGBindingManager::filter(DeviceType type, MMGBinding *check) const
{
	return type == TYPE_NONE || check->type() == type;
}

void MMGBindingManager::resetConnections()
{
	for (MMGBinding *binding : _list) {
		binding->setConnected(false);
		binding->setConnected(true);
	}
}
// End MMGBindingManager
