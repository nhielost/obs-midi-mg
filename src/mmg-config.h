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

class MMGConfig {
public:
	MMGConfig();
	~MMGConfig() { clear(); };

	void load(const QString &path_str = QString());
	void save(const QString &path_str = QString()) const;
	void clear();
	MMGDevice *find_device(const QString &name);
	MMGDevice *get_active_device() const;
	bool is_running() const { return active; };
	void set_running(bool on) { active = on; };

	uint get_next_error_default() { return ++error_device_count; };

	const MMGDevices &get_devices() { return devices; };

	static QString get_filepath();

private:
	MMGDevices devices;
	bool active;
	QString active_device_name;
	uint error_device_count = 0;
};
Q_DECLARE_METATYPE(MMGConfig);
