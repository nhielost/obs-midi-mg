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

MMGDevice::MMGDevice(const QJsonObject &data)
{
	name = data["name"].toString(
		MMGUtils::next_default_name(MMGModes::MMGMODE_DEVICE));
	if (MMGUtils::json_key_exists(data, "bindings", QJsonValue::Array)) {
		QJsonArray arr = data["bindings"].toArray();
		for (QJsonValue json_binding : arr) {
			if (MMGUtils::json_is_valid(json_binding,
						    QJsonValue::Object))
				add(new MMGBinding(json_binding.toObject()));
		}
	}
	if (get_input_port_number(name) != -1) {
		start_reception();
	}
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

void MMGDevice::start_reception()
{
	input_device.set_callback(MMGUtils::call_midi_callback);
	input_device.open_port(get_input_port_number(name));
}

void MMGDevice::stop_reception()
{
	input_device.cancel_callback();
	input_device.close_port();
}

libremidi::midi_in &MMGDevice::get_input_device()
{
	return input_device;
}

libremidi::midi_out &MMGDevice::get_output_device()
{
	return output_device;
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
/*
 * Returns the port number of the specified device.
 * If the device isn't found (possibly due to being disconnected), returns -1
 */
int MMGDevice::get_input_port_number(const QString &deviceName)
{
	return get_input_device_names().indexOf(deviceName);
}
/**
 *
 *
 * @name GetOutPortNumberByDeviceName
 * @Param deviceName
 * @category Device Manager
 * @description Returns the port number of the specified device. \
 *		If the device isn't found (possibly due to being disconnected), returns -1
 * @returns  Device Output Port
 * @rtype int
 */
int MMGDevice::get_output_port_number(const QString &deviceName)
{
	const QStringList portsList = get_output_device_names();
	if (portsList.contains(deviceName))
		return portsList.indexOf(deviceName);
	return -1;
}

void MMGDevice::do_all_actions(const MMGMessage *const message)
{
	for (MMGBinding *const el : bindings) {
		el->do_actions(message);
	}
}

MMGDevice::~MMGDevice()
{
	qDeleteAll(bindings);
	stop_reception();
}
