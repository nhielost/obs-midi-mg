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

// MMGBinding
MMGBinding::MMGBinding(MMGBindingManager *parent, const QJsonObject &json_obj)
	: QObject(parent), _messages(new MMGMessageManager(this)), _actions(new MMGActionManager(this))
{
	thread = new MMGBindingThread(this);

	setObjectName(json_obj["name"].toString(mmgtr("Binding.Untitled")));
	_enabled = json_obj["enabled"].toBool(true);
	_type = (DeviceType)json_obj["type"].toInt();
	reset_mode = json_obj["reset_mode"].toInt();

	_messages->load(json_obj["messages"].toArray());
	_actions->load(json_obj["actions"].toArray());

	setEnabled(_enabled);
}

void MMGBinding::setType(DeviceType type)
{
	if (_type == type) return;

	setConnected(false);
	_type = type;

	_messages->clear(false);
	_actions->clear(false);
	_actions->at(0)->setType(type);

	setConnected(true);
}

void MMGBinding::json(QJsonObject &binding_obj) const
{
	binding_obj["name"] = objectName();
	binding_obj["enabled"] = _enabled;
	binding_obj["type"] = _type;
	binding_obj["reset_mode"] = reset_mode;

	_messages->json("messages", binding_obj);
	_actions->json("actions", binding_obj);
}

void MMGBinding::blog(int log_status, const QString &message) const
{
	global_blog(log_status, QString("[Bindings] <%1> %2").arg(objectName()).arg(message));
}

void MMGBinding::copy(MMGBinding *dest)
{
	dest->setObjectName(objectName());
	dest->reset_mode = reset_mode;
	dest->_type = _type;

	QJsonObject json_obj;
	dest->_messages->clear();
	dest->_actions->clear();
	_messages->json("messages", json_obj);
	_actions->json("actions", json_obj);
	dest->_messages->load(json_obj["messages"].toArray());
	dest->_actions->load(json_obj["actions"].toArray());

	dest->setEnabled(_enabled);
}

void MMGBinding::setEnabled(bool val)
{
	setConnected(false);
	_enabled = val;
	setConnected(true);
}

void MMGBinding::setConnected(bool _connected)
{
	if (_connected && (!_enabled || connected)) return;

	MMGMIDIPort *device = messages(0)->device();

	switch (_type) {
		case TYPE_INPUT:
		default:
			if (!device) break;
			if (_connected) {
				connect(device, &MMGMIDIPort::messageReceived, this, &MMGBinding::executeInput,
					Qt::UniqueConnection);
			} else {
				disconnect(device, &MMGMIDIPort::messageReceived, this, nullptr);
			}
			device->incConnection(_connected);
			break;

		case TYPE_OUTPUT:
			if (_connected) {
				connect(actions(0), &MMGAction::eventTriggered, this, &MMGBinding::executeOutput);
				actions(0)->connectOBSSignals();
			} else {
				disconnect(actions(0), &MMGAction::eventTriggered, this, nullptr);
				actions(0)->disconnectOBSSignals();
			}
			break;
	}

	connected = _connected;
}

void MMGBinding::executeInput(const MMGSharedMessage &incoming) const
{
	if (!messages(0)->acceptable(incoming.get())) {
		blog(LOG_DEBUG, "Message received is not acceptable.");
		return;
	}

	MMGBindingThread *exec_thread = reset_mode ? thread->createNew() : thread;
	exec_thread->restart(incoming.get());
}

void MMGBinding::executeOutput(const QList<MMGNumber> &values) const
{
	MMGBindingThread *exec_thread = reset_mode ? thread->createNew() : thread;
	exec_thread->restart(values);
}

QDataStream &operator<<(QDataStream &out, const MMGBinding *&obj)
{
	return out << (const QObject *&)obj;
}

QDataStream &operator>>(QDataStream &in, MMGBinding *&obj)
{
	return in >> (QObject *&)obj;
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
	if (binding->_messages->size() < 1) {
		blog(LOG_INFO, "FAILED: No messages to send!");
		return;
	}
	if (incoming_values.isEmpty()) {
		blog(LOG_INFO, "FAILED: Values not found. Report this as a bug.");
		return;
	}

	int i;
	MMGNumber used_value;

	for (i = 0; i < binding->_messages->size(); ++i) {
		if (mutex.try_lock()) return;

		binding->messages(i)->copy(applied_message);

		used_value = incoming_values.size() < i ? incoming_values[i] : incoming_values.last();
		applied_message->applyValues(used_value);

		applied_message->send();
	}

	for (MMGMessage *message : *binding->_messages)
		message->toggle();
	binding->actions(0)->toggle();
}

void MMGBindingThread::doActions()
{
	incoming_message->copy(applied_message);
	applied_message->type() = incoming_message->type().value();
	applied_message->channel() = incoming_message->channel().value();
	applied_message->note() = incoming_message->note().value();
	applied_message->value() = incoming_message->value().value();

	for (MMGAction *action : *binding->_actions) {
		if (mutex.try_lock()) return;
		action->execute(applied_message);
	}

	binding->messages(0)->toggle();
	for (MMGAction *action : *binding->_actions)
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
MMGBindingManager::MMGBindingManager(QObject *parent, const QJsonObject &json_obj) : MMGManager(parent)
{
	setObjectName(json_obj["name"].toString(mmgtr("Collection.Untitled")));

	for (const QJsonValue &val : json_obj["bindings"].toArray())
		add(val.toObject());

	if (size() < 1) add();
}

MMGBinding *MMGBindingManager::add(const QJsonObject &json_obj)
{
	return MMGManager::add(new MMGBinding(this, json_obj));
}

void MMGBindingManager::copy(MMGBindingManager *dest)
{
	dest->setObjectName(objectName());

	dest->clear();
	for (MMGBinding *binding : _list) {
		dest->copy(binding);
	}
}

void MMGBindingManager::json(QJsonObject &json_obj) const
{
	json_obj["name"] = objectName();
	MMGManager::json("bindings", json_obj);
}

void MMGBindingManager::resetConnections()
{
	for (MMGBinding *binding : _list) {
		binding->setConnected(false);
		binding->setConnected(true);
	}
}

QDataStream &operator<<(QDataStream &out, const MMGBindingManager *&obj)
{
	return out << (const QObject *&)obj;
}

QDataStream &operator>>(QDataStream &in, MMGBindingManager *&obj)
{
	return in >> (QObject *&)obj;
}
// End MMGBindingManager

// MMGCollections
MMGBindingManager *MMGCollections::add(const QJsonObject &json_obj)
{
	return MMGManager::add(new MMGBindingManager(this, json_obj));
}
// End MMGCollections
