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

#include "mmg-device.h"

bool json_old_bindings(const QJsonObject &json, MMGDevice *parent)
{
	if (json.isEmpty())
		return false;

	QList<MMGMessage *> message_list;
	QList<MMGAction *> action_list;

	if (MMGUtils::json_key_exists(json, "messages", QJsonValue::Array)) {
		for (QJsonValue val : json["messages"].toArray()) {
			message_list.append(new MMGMessage(val.toObject()));
		}
	}
	if (MMGUtils::json_key_exists(json, "actions", QJsonValue::Array)) {
		for (QJsonValue val : json["actions"].toArray()) {
			action_list.append(new MMGAction(val.toObject()));
		}
	}

	if (action_list.isEmpty())
		return false;

	for (qlonglong i = 0; i < action_list.size(); ++i) {
		MMGBinding *binding = parent->add();
		if (parent->find_binding(json["name"].toString())) {
			binding->set_name(binding->get_next_default_name());
		} else {
			binding->set_name(json["name"].toString());
		}
		if (message_list.value(i)) {
			*(binding->get_message()) = *(message_list[i]);
		} else {
			*(binding->get_message()) = *(message_list.last());
		}
		*(binding->get_action()) = *(action_list[i]);
	}

	return true;
}

qulonglong MMGDevice::next_default = 0;

MMGDevice::MMGDevice(const QJsonObject &data)
{
	name = data["name"].toString();
	if (name.isEmpty())
		name = get_next_default_name();
	if (MMGUtils::json_key_exists(data, "bindings", QJsonValue::Array)) {
		QJsonArray arr = data["bindings"].toArray();
		for (QJsonValue json_binding : arr) {
			if (json_old_bindings(json_binding.toObject(), this))
				continue;
			if (MMGUtils::json_is_valid(json_binding,
						    QJsonValue::Object)) {
				add(new MMGBinding(json_binding.toObject()));
			}
		}
	}
	check_binding_default_names();
	blog(LOG_DEBUG, "Device created.");
}

void MMGDevice::json(QJsonObject &device_obj) const
{
	QJsonArray json_bindings;
	device_obj["name"] = name;
	for (const MMGBinding *const binding : bindings) {
		QJsonObject json_binding;
		binding->json(json_binding);
		json_bindings += json_binding;
	}
	device_obj["bindings"] = json_bindings;
}

void MMGDevice::blog(int log_status, const QString &message) const
{
	global_blog(log_status, "Device {" + name + "} -> " + message);
}

QString MMGDevice::get_next_default_name()
{
	return QVariant(++MMGDevice::next_default)
		.toString()
		.prepend("Untitled Device ");
}

void MMGDevice::check_binding_default_names()
{
	for (const MMGBinding *binding : bindings) {
		if (binding->get_name().startsWith("Untitled Binding ")) {
			QString name = binding->get_name();
			qulonglong num =
				QVariant(name.remove("Untitled Binding "))
					.toULongLong();
			if (num > MMGBinding::get_next_default())
				MMGBinding::set_next_default(num);
		}
	}
}

MMGBinding *MMGDevice::add(MMGBinding *const el)
{
	bindings.append(el);
	return el;
}

MMGBinding *MMGDevice::copy(MMGBinding *const el)
{
	MMGBinding *new_binding = add();
	el->deep_copy(new_binding);
	new_binding->set_name("(Copy of " + el->get_name() + ") " +
			      MMGBinding::get_next_default_name());
	return new_binding;
}

void MMGDevice::remove(MMGBinding *const el)
{
	bindings.removeAt(index_of(el));
}

int MMGDevice::index_of(MMGBinding *const el)
{
	int i = -1;
	for (MMGBinding *const binding : bindings) {
		++i;
		if (binding == el)
			return i;
	}
	return -1;
}

int MMGDevice::size() const
{
	return bindings.size();
}

MMGBinding *MMGDevice::find_binding(const QString &name)
{
	for (MMGBinding *const el : bindings) {
		if (el->get_name() == name)
			return el;
	}
	return nullptr;
}

void MMGDevice::open_input_port(MMGDevice *device)
{
	if (!device)
		return;

	if (input_port_open())
		close_input_port();

	device->blog(LOG_INFO, "Opening input port...");

	if (get_input_port_number(device->get_name()) == (uint)-1) {
		device->blog(
			LOG_INFO,
			"Port opening failed: Device is disconnected or does not exist.");
		return;
	}

	try {
		input_device()->set_callback(MMGUtils::call_midi_callback);
		input_device()->open_port(
			get_input_port_number(device->get_name()));
		device->blog(LOG_INFO, "Input port successfully opened.");
	} catch (const libremidi::driver_error &err) {
		device->blog(LOG_INFO, err.what());
	} catch (const libremidi::invalid_parameter_error &err) {
		device->blog(LOG_INFO, err.what());
	} catch (const libremidi::system_error &err) {
		device->blog(LOG_INFO, err.what());
	}
}

void MMGDevice::open_output_port(MMGDevice *device)
{
	if (!device)
		return;

	if (output_port_open())
		close_output_port();

	device->blog(LOG_INFO, "Opening output port...");

	if (get_output_port_number(device->get_name()) == (uint)-1) {
		device->blog(
			LOG_INFO,
			"Port opening failed: Device is disconnected or does not exist.");
		return;
	}

	try {
		output_device()->open_port(
			get_output_port_number(device->get_name()));
		device->blog(LOG_INFO, "Output port successfully opened.");
	} catch (const libremidi::driver_error &err) {
		device->blog(LOG_INFO, err.what());
	} catch (const libremidi::invalid_parameter_error &err) {
		device->blog(LOG_INFO, err.what());
	} catch (const libremidi::system_error &err) {
		device->blog(LOG_INFO, err.what());
	}
}

void MMGDevice::close_input_port()
{
	input_device()->cancel_callback();
	input_device()->close_port();
	global_blog(LOG_INFO, "Main -> Input port closed.");
}

void MMGDevice::close_output_port()
{
	output_device()->close_port();
	global_blog(LOG_INFO, "Main -> Output port closed.");
}

bool MMGDevice::input_port_open()
{
	return input_device()->is_port_open();
}

bool MMGDevice::output_port_open()
{
	return output_device()->is_port_open();
}

void MMGDevice::output_send(const libremidi::message &message)
{
	output_device()->send_message(message);
}

const QString MMGDevice::input_device_status() const
{
	if (get_input_port_number(name) != (uint)-1) {
		return "Ready";
	} else if (get_output_port_number(name) != (uint)-1) {
		return "Unavailable";
	} else {
		return "Not Connected";
	}
}

const QString MMGDevice::output_device_status() const
{
	if (get_output_port_number(name) != (uint)-1) {
		return "Ready";
	} else if (get_input_port_number(name) != (uint)-1) {
		return "Unavailable";
	} else {
		return "Not Connected";
	}
}

QStringList MMGDevice::get_input_device_names()
{
	QStringList inputs;
	for (uint i = 0; i < input_device()->get_port_count(); ++i) {
		inputs.append(QString::fromStdString(
			input_device()->get_port_name(i)));
	}
	return inputs;
}
/*
 * Returns QStringList of Output Port Names
 */
QStringList MMGDevice::get_output_device_names()
{
	QStringList outputs;
	for (uint i = 0; i < output_device()->get_port_count(); ++i) {
		outputs.append(QString::fromStdString(
			output_device()->get_port_name(i)));
	}
	return outputs;
}

uint MMGDevice::get_input_port_number(const QString &device_name)
{
	return get_input_device_names().indexOf(device_name);
}

uint MMGDevice::get_output_port_number(const QString &device_name)
{
	return get_output_device_names().indexOf(device_name);
}

void MMGDevice::do_all_actions(const MMGSharedMessage &message)
{
	for (MMGBinding *const el : bindings) {
		el->do_action(message);
	}
	for (MMGBinding *const el : bindings) {
		el->reset_execution();
	}
}

void MMGDevice::deep_copy(MMGDevice *dest)
{
	dest->set_name(name);
	for (MMGBinding *binding : bindings) {
		MMGBinding *new_binding = dest->add();
		binding->deep_copy(new_binding);
	}
}

MMGDevice::~MMGDevice()
{
	qDeleteAll(bindings);
	clear();
}
