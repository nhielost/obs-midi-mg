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

#pragma once

#include "mmg-binding.h"

class MMGDevice {
public:
	MMGDevice() = default;
	explicit MMGDevice(const QJsonObject &data);
	~MMGDevice();

	void json(QJsonObject &device_obj) const;

	const QString get_name() const { return name; };
	void set_name(const QString &val) { name = val; };

	const MMGBindingList &get_bindings() const { return bindings; };
	uint get_next_binding_default() { return ++next_default_names[0]; }
	uint get_next_message_default() { return ++next_default_names[1]; }
	uint get_next_action_default() { return ++next_default_names[2]; }

	MMGBinding *add(MMGBinding *const el = new MMGBinding);
	void remove(MMGBinding *const el);
	MMGBinding *find_binding(const QString &name);
	int index_of(MMGBinding *const el);
	int size() const;

	void start_reception();
	void stop_reception();
	void do_all_actions(const MMGMessage *const message);

	libremidi::midi_in &get_input_device();
	libremidi::midi_out &get_output_device();

	static QStringList get_input_device_names();
	static int get_input_port_number(const QString &deviceName);
	static QStringList get_output_device_names();
	static int get_output_port_number(const QString &deviceName);

private:
	QString name;
	MMGBindingList bindings;
	libremidi::midi_in input_device;
	libremidi::midi_out output_device;
	uint next_default_names[3]{0, 0, 0};
};

using MMGDevices = QList<MMGDevice *>;
