/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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

qulonglong MMGBinding::next_default = 1;

MMGBinding::MMGBinding()
{
	name = get_next_default_name();
	toggling = false;
	enabled = true;
	message = new MMGMessage;
	action = new MMGAction;
	blog(LOG_DEBUG, "Empty binding created.");
}

MMGBinding::MMGBinding(const QJsonObject &obj)
{
	name = obj["name"].toString();
	if (name.isEmpty())
		name = get_next_default_name();
	toggling = obj["toggling"].toBool();
	enabled = obj["enabled"].toBool(true);
	message = new MMGMessage(obj["message"].toObject());
	action = new MMGAction(obj["action"].toObject());
	blog(LOG_DEBUG, "Binding created.");
}

void MMGBinding::json(QJsonObject &binding_obj) const
{
	binding_obj["name"] = name;
	binding_obj["toggling"] = toggling;
	binding_obj["enabled"] = enabled;
	QJsonObject msg;
	message->json(msg);
	binding_obj["message"] = msg;
	QJsonObject act;
	action->json(act);
	binding_obj["action"] = act;
}

void MMGBinding::blog(int log_status, const QString &message) const
{
	global_blog(log_status, "Binding {" + name + "} -> " + message);
}

QString MMGBinding::get_next_default_name()
{
	return QVariant(++MMGBinding::next_default)
		.toString()
		.prepend("Untitled Binding ");
}

void MMGBinding::do_action(const MMGSharedMessage &incoming)
{
	if (!enabled)
		return;

	if (!message->is_acceptable(incoming.get())) {
		blog(LOG_DEBUG, "Message received is not acceptable.");
		return;
	}

	action->do_action(incoming);

	if (toggling)
		message->toggle();
}

void MMGBinding::deep_copy(MMGBinding *dest)
{
	if (!name.contains("Untitled Binding"))
		dest->set_name(name);
	dest->set_toggling(get_toggling());
	message->deep_copy(dest->get_message());
	action->deep_copy(dest->get_action());
}
