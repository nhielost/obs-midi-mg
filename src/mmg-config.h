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

#include "mmg-device.h"
#include "mmg-binding.h"
#include "mmg-settings.h"

class MMGOldConfig;

class MMGConfig : public QObject {
	Q_OBJECT

public:
	MMGConfig();

	void blog(int log_status, const QString &message) const;

	void load(const QString &path_str = QString());
	void save(const QString &path_str = QString()) const;
	void clearAllData();

	MMGCollections *collections() const { return _collections; };
	MMGDeviceManager *devices() const { return _devices; };
	MMGSettings *settings() const { return _settings; };

	MMGSignals *mmgsignals() const { return _signals; };
	MMGMIDI *midi() const { return _midi; };

	static QString filename() { return "obs-midi-mg-config.json"; };
	static QString filepath(const QString &path_str);

private:
	MMGCollections *_collections;
	MMGDeviceManager *_devices;
	MMGSettings *_settings;

	MMGSignals *_signals;
	MMGMIDI *_midi;

	MMGOldConfig *old_config;
};

#define enum_manager(which) for (auto val : *manager(which))
#define manager(which) config()->which##s()
#define midi() config()->midi()

class MMGOldConfig : public QObject {
	Q_OBJECT

public:
	MMGOldConfig(MMGConfig *parent) : QObject(parent), new_config(parent) {}

	void blog(int log_status, const QString &message) { new_config->blog(log_status, message); };
	void load(QJsonObject &doc);
	void postLoad();

private:
	void cleanDeviceName(QString &) const;

	MMGConfig *new_config;
	QMultiMap<QString, QString> old_internal_bindings;
};

#endif // MMG_CONFIG_H
