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

#include "mmg-binding.h"
#include "mmg-config.h"
#include "mmg-midi.h"

using namespace MMGUtils;

// MMGBinding
MMGBinding::MMGBinding(MMGBindingManager *parent, const QJsonObject &json_obj)
	: QObject(parent),
	  _messages(new MMGMessageManager(this)),
	  _actions(new MMGActionManager(this)),
	  link(new MMGLink(this))
{
	setObjectName(json_obj["name"].toString(mmgtr("Binding.Untitled")));
	_enabled = json_obj["enabled"].toBool(true);
	_type = (DeviceType)json_obj["type"].toInt();
	reset_mode = json_obj["reset_mode"].toInt();

	_messages->load(json_obj["messages"].toArray());
	_actions->setType(_type);
	_actions->load(json_obj["actions"].toArray());

	setEnabled(_enabled);
}

void MMGBinding::setType(DeviceType type)
{
	if (_type == type) return;

	setConnected(false);
	_type = type;
	_actions->setType(type);

	_messages->clear(false);
	_actions->clear(false);

	setConnected(true);
}

void MMGBinding::blog(int log_status, const QString &message) const
{
	global_blog(log_status, QString("[Bindings] <%1> %2").arg(objectName()).arg(message));
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

void MMGBinding::copy(MMGBinding *dest)
{
	dest->setObjectName(objectName());
	dest->reset_mode = reset_mode;
	dest->_type = _type;
	dest->_actions->setType(_type);

	QJsonObject json_obj;
	dest->_messages->clear();
	dest->_actions->clear();
	_messages->json("messages", json_obj);
	_actions->json("actions", json_obj);
	dest->_messages->load(json_obj["messages"].toArray());
	dest->_actions->load(json_obj["actions"].toArray());

	dest->setEnabled(_enabled);
}

void MMGBinding::toggle()
{
	for (MMGMessage *message : *_messages)
		message->toggle();

	for (MMGAction *action : *_actions)
		action->toggle();
}

void MMGBinding::setEnabled(bool val)
{
	_enabled = val;
	refresh();
}

void MMGBinding::setConnected(bool _connected)
{
	if (_connected && (!_enabled || connected)) return;

	link->establish(_connected);

	connected = _connected;
}

void MMGBinding::refresh()
{
	setConnected(false);
	setConnected(true);
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

void MMGBindingManager::refreshAll()
{
	for (MMGBinding *binding : _list)
		binding->refresh();
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
