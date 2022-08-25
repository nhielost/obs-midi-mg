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

struct MMGSettings {
	bool active = true;
	bool tooltips = true;

	MMGSettings() = default;
	explicit MMGSettings(const QJsonObject &settings_obj)
	{
		active = settings_obj["active"].toBool(true);
		tooltips = settings_obj["tooltips"].toBool(true);
	}

	void json(QJsonObject &settings_obj) const
	{
		settings_obj["active"] = active;
		settings_obj["tooltips"] = tooltips;
	}
};

class MMGConfig {
public:
	MMGConfig() { load(); };
	~MMGConfig() { clear(); };

	void blog(int log_status, const QString &message) const;

	void load(const QString &path_str = QString());
	void save(const QString &path_str = QString()) const;
	void clear();
	MMGDevice *find_device(const QString &name);
	const QStringList get_device_names() const;
	MMGSettings &preferences() { return settings; };

	static QString get_filepath();

private:
	MMGDevices devices;
	MMGSettings settings;

	void check_device_default_names();
};
Q_DECLARE_METATYPE(MMGConfig);
