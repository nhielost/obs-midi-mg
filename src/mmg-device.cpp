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

qulonglong MMGDevice::next_default = 0;

MMGDevice::MMGDevice(const QJsonObject &data)
{
	name = data["name"].toString();
	if (name.isEmpty())
		name = get_next_default_name();
	if (MMGUtils::json_key_exists(data, "bindings", QJsonValue::Array)) {
		QJsonArray arr = data["bindings"].toArray();
		for (QJsonValue json_binding : arr) {
			if (MMGUtils::json_is_valid(json_binding,
						    QJsonValue::Object))
				add(new MMGBinding(json_binding.toObject()));
		}
	}
	check_binding_default_names();
	open_input_port();
	open_output_port();
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
	QString temp_msg = "Device {";
	temp_msg.append(get_name());
	temp_msg.append("} -> ");
	temp_msg.append(message);
	global_blog(log_status, temp_msg);
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
		if (name == el->get_name())
			return el;
	}
	return nullptr;
}

void MMGDevice::open_input_port()
{
	if (get_input_port_number(name) >= 0) {
		input_device.set_callback(MMGUtils::call_midi_callback);
		input_device.open_port(get_input_port_number(name));
		blog(LOG_INFO, "Input port successfully opened.");
	}
}

void MMGDevice::open_output_port()
{
	if (get_output_port_number(name) >= 0) {
		output_device.open_port(get_output_port_number(name));
		blog(LOG_INFO, "Output port successfully opened.");
	}
}

void MMGDevice::close_input_port()
{
	input_device.cancel_callback();
	input_device.close_port();
	blog(LOG_INFO, "Input port closed.");
}

void MMGDevice::close_output_port()
{
	output_device.close_port();
	blog(LOG_INFO, "Output port closed.");
}

bool MMGDevice::input_port_open() const
{
	return input_device.is_port_open();
}

bool MMGDevice::output_port_open() const
{
	return output_device.is_port_open();
}

void MMGDevice::output_send(const libremidi::message &message)
{
	output_device.send_message(message);
}

const QString MMGDevice::input_device_status() const
{
	if (input_port_open()) {
		return "Active";
	} else if (get_output_port_number(name) >= 0) {
		return "Unavailable";
	} else {
		return "Not Connected";
	}
}

const QString MMGDevice::output_device_status() const
{
	if (output_port_open()) {
		return "Active";
	} else if (get_input_port_number(name) >= 0) {
		return "Unavailable";
	} else {
		return "Not Connected";
	}
}

QStringList MMGDevice::get_input_device_names()
{
	QStringList inputs;
	for (uint i = 0; i < libremidi::midi_in().get_port_count(); ++i) {
		inputs.append(QString::fromStdString(
			libremidi::midi_in().get_port_name(i)));
	}
	return inputs;
}
/*
 * Returns QStringList of Output Port Names
 */
QStringList MMGDevice::get_output_device_names()
{
	QStringList outputs;
	for (uint i = 0; i < libremidi::midi_out().get_port_count(); ++i) {
		outputs.append(QString::fromStdString(
			libremidi::midi_out().get_port_name(i)));
	}
	return outputs;
}

int MMGDevice::get_input_port_number(const QString &device_name)
{
	return get_input_device_names().indexOf(device_name);
}

int MMGDevice::get_output_port_number(const QString &device_name)
{
	return get_output_device_names().indexOf(device_name);
}

void MMGDevice::do_all_actions(const MMGSharedMessage &message)
{
	for (MMGBinding *const el : bindings) {
		el->do_actions(message);
	}
}

MMGDevice::~MMGDevice()
{
	qDeleteAll(bindings);
	bindings.clear();
	close_input_port();
	close_output_port();
}
