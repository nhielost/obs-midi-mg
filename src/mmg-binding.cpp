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

MMGBinding::MMGBinding(const QJsonObject &obj)
{
	name = obj["name"].toString(
		MMGUtils::next_default_name(MMGModes::MMGMODE_BINDING));
	mode = obj["mode"].toInt(1);
	if (MMGUtils::json_key_exists(obj, "messages", QJsonValue::Array)) {
		QJsonArray msg_arr = obj["messages"].toArray();
		for (QJsonValue child : msg_arr) {
			if (!MMGUtils::json_is_valid(child,
						     QJsonValue::Object) ||
			    child.toObject().isEmpty())
				break;
			add_message(new MMGMessage(child.toObject()));
		}
	}
	if (MMGUtils::json_key_exists(obj, "actions", QJsonValue::Array)) {
		QJsonArray act_arr = obj["actions"].toArray();
		for (QJsonValue child : act_arr) {
			if (!MMGUtils::json_is_valid(child,
						     QJsonValue::Object) ||
			    child.toObject().isEmpty())
				break;
			add_action(new MMGAction(child.toObject()));
		}
	}
}

void MMGBinding::json(QJsonObject &binding_obj) const
{
	binding_obj["name"] = name;
	binding_obj["mode"] = (int)mode;
	QJsonArray msg_arr;
	for (MMGMessage *const chi : messages) {
		QJsonObject json_chi;
		chi->json(json_chi);
		msg_arr += json_chi;
	}
	binding_obj["messages"] = msg_arr;
	QJsonArray act_arr;
	for (MMGAction *const chi : actions) {
		QJsonObject json_chi;
		chi->json(json_chi);
		act_arr += json_chi;
	}
	binding_obj["actions"] = act_arr;
}

MMGMessage *MMGBinding::find_message(const QString &name)
{
	for (MMGMessage *const chi : get_messages()) {
		if (chi->get_name() == name)
			return chi;
	}
	return nullptr;
}

MMGAction *MMGBinding::find_action(const QString &name)
{
	for (MMGAction *const chi : get_actions()) {
		if (chi->get_name() == name)
			return chi;
	}
	return nullptr;
}

MMGMessage *MMGBinding::add_message(MMGMessage *const el)
{
	messages.append(el);
	return el;
}

void MMGBinding::insert_message(int index, MMGMessage *const el)
{
	messages.insert(index, el);
}

void MMGBinding::remove(MMGMessage *const el)
{
	messages.removeAt(index_of(el));
}

MMGAction *MMGBinding::add_action(MMGAction *const el)
{
	actions.append(el);
	return el;
}

void MMGBinding::insert_action(int index, MMGAction *const el)
{
	actions.insert(index, el);
}

void MMGBinding::remove(MMGAction *const el)
{
	actions.removeAt(index_of(el));
}

size_t MMGBinding::index_of(MMGMessage *const el) const
{
	return messages.indexOf(el);
}

size_t MMGBinding::index_of(MMGAction *const el) const
{
	return actions.indexOf(el);
}

size_t MMGBinding::message_size() const
{
	return messages.size();
}

size_t MMGBinding::action_size() const
{
	return actions.size();
}

void MMGBinding::move_elements(MMGModes mode, size_t from, size_t to)
{
	switch (mode) {
	case MMGModes::MMGMODE_MESSAGE:
		if (from >= message_size())
			break;
		to == message_size() ? messages.append(messages.takeAt(from))
				     : messages.move(from, to);
		break;
	case MMGModes::MMGMODE_ACTION:
		if (from >= action_size())
			break;
		to == action_size() ? actions.append(actions.takeAt(from))
				    : actions.move(from, to);
		break;
	default:
		break;
	}
}

bool MMGBinding::is_valid()
{
	return message_size() > 0;
}

void MMGBinding::do_actions(const MMGSharedMessage &incoming)
{
	// Check if binding contains a message
	if (!is_valid())
		return;
	// Variable initialization to appease compilers
	size_t saved_message_index = 0;
	size_t action_index = 0;
	// Check the incoming message with the next message
	if (!get_messages()[current_message_index]->is_acceptable(
		    incoming.get())) {
		goto reset_binding;
	}
	// Append the incoming message to a message list so it can be used (for Correspondence and Multiply Mode)
	saved_messages.append(incoming);
	// Checks if the message is the last message
	if (current_message_index + 1 < message_size()) {
		++current_message_index;
		return;
	}

	// Execute!
	switch (get_mode()) {

	case Mode::MMGBINDING_CONSECUTIVE:
		while (action_index < action_size()) {
			actions[action_index]->do_action(incoming.get());
			++action_index;
		}
		break;

	case Mode::MMGBINDING_CORRESPONDENCE:
		while (action_index < action_size()) {
			actions[action_index]->do_action(
				saved_messages[fmod(saved_message_index,
						    saved_messages.size())]
					.get());
			++action_index;
			++saved_message_index;
		}
		break;

	case Mode::MMGBINDING_MULTIPLY:
		while (action_index < action_size()) {
			for (const MMGSharedMessage &msg : saved_messages) {
				actions[action_index]->do_action(msg.get());
			}
			++action_index;
		}
		break;

	default:
		break;
	}
reset_binding:
	// Reset everything (on failure or on completion)
	current_message_index = 0;
	saved_messages.clear();
}
