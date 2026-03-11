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

#include "mmg-binding.h"
#include "mmg-config.h"

#include <QThreadPool>

// MMGBinding
MMGBinding::MMGBinding(MMGBindingManager *parent, const QJsonObject &json_obj)
	: QObject(parent),
	  QRunnable(),
	  _messages(new MMGMessageManager(this, "messages")),
	  _actions(new MMGActionManager(this, "actions"))
{
	setObjectName(json_obj["name"].toString(mmgtr("Binding.Untitled")));
	setAutoDelete(false);

	_enabled = json_obj["enabled"].toBool(true);
	_type = (DeviceType)json_obj["type"].toInt();
	reset_mode = json_obj["reset_mode"].toInt();

	MMGStates::MMGReferenceIndexHandler::setOldReferenceIndex(MMGStates::ReferenceIndex(0));
	_messages->load(json_obj);
	_actions->load(json_obj);
}

void MMGBinding::setType(DeviceType type)
{
	if (_type == type) return;

	setConnected(false);
	_type = type;

	_messages->clear(false);
	_actions->clear(false);

	if (_type == TYPE_OUTPUT) {
		_actions->at(0)->setObjectName(mmgtr("Actions.Condition").translate());
	} else {
		_messages->at(0)->setObjectName(mmgtr("Message.Condition").translate());
	}

	setConnected(true);
}

void MMGBinding::blog(int log_status, const QString &message) const
{
	mmgblog(log_status, QString("[Bindings] <%1> %2").arg(objectName()).arg(message));
}

void MMGBinding::json(QJsonObject &binding_obj) const
{
	binding_obj["name"] = objectName();
	binding_obj["enabled"] = _enabled;
	binding_obj["type"] = _type;
	binding_obj["reset_mode"] = reset_mode;

	_messages->json(binding_obj);
	_actions->json(binding_obj);
}

void MMGBinding::copy(MMGBinding *dest)
{
	dest->setObjectName(objectName());
	dest->reset_mode = reset_mode;
	dest->_type = _type;

	QJsonObject json_obj;
	dest->_messages->clear();
	dest->_actions->clear();
	_messages->json(json_obj);
	_actions->json(json_obj);
	dest->_messages->load(json_obj);
	dest->_actions->load(json_obj);

	dest->setEnabled(_enabled);
}

void MMGBinding::setEnabled(bool val)
{
	_enabled = val;
	refresh();
}

void MMGBinding::setConnected(bool _connected)
{
	if (_connected && (!_enabled || connected)) return;

	switch (_type) {
		case TYPE_INPUT:
		default:
			if (_connected) {
				connect(messages(0), &MMGMessage::refreshRequested, this, &MMGBinding::refresh,
					Qt::UniqueConnection);
				connect(messages(0), &MMGMessage::fulfilled, this, &MMGBinding::execute,
					Qt::UniqueConnection);
			} else {
				disconnect(messages(0), &MMGMessage::refreshRequested, this, &MMGBinding::refresh);
				disconnect(messages(0), &MMGMessage::fulfilled, this, &MMGBinding::execute);
			}

			messages(0)->connectDevice(_connected);
			break;

		case TYPE_OUTPUT:
			if (_connected) {
				connect(actions(0), &MMGAction::refreshRequested, this, &MMGBinding::refresh,
					Qt::UniqueConnection);
				connect(actions(0), &MMGAction::fulfilled, this, &MMGBinding::execute,
					Qt::UniqueConnection);
			} else {
				disconnect(actions(0), &MMGAction::refreshRequested, this, &MMGBinding::refresh);
				disconnect(actions(0), &MMGAction::fulfilled, this, &MMGBinding::execute);
			}

			actions(0)->connectSignal(_connected);
			break;
	}

	connected = _connected;
}

void MMGBinding::refresh()
{
	setConnected(false);
	setConnected(true);
}

void MMGBinding::execute(const MMGMappingTest &test)
{
	if (_type == TYPE_OUTPUT) {
		if (_messages->size() < 1) {
			blog(LOG_INFO, "EXECUTION FAILED: No messages to send!");
			return;
		}
	} else {
		if (_actions->size() < 1) {
			blog(LOG_INFO, "EXECUTION FAILED: No actions to execute!");
			return;
		}
	}

	_test = test;
	stop_request = reset_mode != MMGBinding::BINDING_CONTINUOUS;

	QThreadPool::globalInstance()->start(this);
}

void MMGBinding::run()
{
	stop_request = false;
	MMGMappingTest thread_test = _test;

	if (_type == TYPE_OUTPUT) {
		for (MMGMessage *message : *_messages) {
			if (stop_request) break;
			message->send(thread_test);
		}
	} else {
		for (MMGAction *action : *_actions) {
			if (stop_request) break;
			action->execute(thread_test);
		}
	}

	stop_request = false;
}
// End MMGBinding

template <> MMGBindingManager *MMGBindingManager::generate(MMGCollections *parent, const QJsonObject &json_obj)
{
	auto *manager = new MMGBindingManager(parent, "bindings");
	manager->setObjectName(json_obj["name"].toString(mmgtr("Collection.Untitled")));
	manager->load(json_obj);
	return manager;
}
