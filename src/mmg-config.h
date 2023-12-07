/*
obs-midi-mg
Copyright (C) 2022-2023 nhielost <nhielost@gmail.com>

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

#ifndef MMG_CONFIG_H
#define MMG_CONFIG_H

#include "mmg-binding.h"
#include "mmg-settings.h"

class MMGConfig : public QObject {
	Q_OBJECT

public:
	MMGConfig();

	void blog(int log_status, const QString &message) const;

	void load(const QString &path_str = QString());
	void save(const QString &path_str = QString()) const;
	void clearAllData();

	MMGDeviceManager *devices() const { return _devices; };
	MMGMessageManager *messages() const { return _messages; };
	MMGActionManager *actions() const { return _actions; };
	MMGBindingManager *bindings() const { return _bindings; };
	MMGSettingsManager *settings() const { return _settings; };

	MMGSignals *mmgsignals() const { return _signals; };
	MMGMIDI *midi() const { return _midi; };

	static QString filepath();

private:
	MMGDeviceManager *_devices;
	MMGMessageManager *_messages;
	MMGActionManager *_actions;
	MMGBindingManager *_bindings;
	MMGSettingsManager *_settings;

	MMGSignals *_signals;
	MMGMIDI *_midi;
};

#define manager(which) config()->which##s()

#endif // MMG_CONFIG_H
