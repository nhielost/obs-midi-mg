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
	void blog(int log_status, const QString &message) const;

	const QString get_name() const { return name; };
	void set_name(const QString &val) { name = val; };

	const MMGBindingList &get_bindings() const { return bindings; };

	MMGBinding *add(MMGBinding *const el = new MMGBinding);
	void remove(MMGBinding *const el);
	MMGBinding *find_binding(const QString &name);
	int index_of(MMGBinding *const el);
	int size() const;

	void do_all_actions(const MMGSharedMessage &message);

	static void open_input_port(MMGDevice *device);
	static void open_output_port(MMGDevice *device);
	static void close_input_port();
	static void close_output_port();
	static bool input_port_open();
	static bool output_port_open();
	static void output_send(const libremidi::message &message);
	const QString input_device_status() const;
	const QString output_device_status() const;

	static QStringList get_input_device_names();
	static uint get_input_port_number(const QString &deviceName);
	static QStringList get_output_device_names();
	static uint get_output_port_number(const QString &deviceName);

	static qulonglong get_next_default() { return next_default; };
	static void set_next_default(qulonglong num) { next_default = num; };
	static QString get_next_default_name();

private:
	QString name;
	MMGBindingList bindings;

	static qulonglong next_default;

	void check_binding_default_names();
};

using MMGDevices = QList<MMGDevice *>;
