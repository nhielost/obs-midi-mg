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
public:
	MMGSettings() = default;
	explicit MMGSettings(const QJsonObject &settings_obj);
	void json(QJsonObject &settings_obj) const;

	bool get_active() const { return active; };
	bool get_tooltips() const { return tooltips; };
	void set_active(bool is_active);
	void set_tooltips(bool is_tooltips) { tooltips = is_tooltips; };

private:
	bool active = true;
	bool tooltips = true;
};

class MMGConfig {
public:
	MMGConfig() { load(); };
	~MMGConfig() { clear(); };

	void blog(int log_status, const QString &message) const;

	void load(const QString &path_str = QString());
	void save(const QString &path_str = QString()) const;
	void clear();
	void load_new_devices();
	QString get_active_device_name() { return active_device_name; };
	void set_active_device_name(const QString &name);
	MMGDevice *find_device(const QString &name);
	MMGDevice *find_current_device()
	{
		return find_device(active_device_name);
	};
	const QStringList get_device_names() const;
	MMGSettings &preferences() { return settings; };

	static QString get_filepath();
	static void
	set_listening_callback(std::function<void(MMGMessage *)> callback)
	{
		cb = callback;
	};
	static void set_listening(bool value) { listening = value; };
	static bool is_listening(MMGMessage *incoming);

private:
	MMGDevices devices;
	MMGSettings settings;
	QString active_device_name = "";

	void check_device_default_names();

	static bool listening;
	static std::function<void(MMGMessage *)> cb;
};
Q_DECLARE_METATYPE(MMGConfig);
