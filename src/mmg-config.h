/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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
#include "mmg-device.h"
#include "mmg-preference.h"

#include <QDateTime>

class MMGConfig : public QObject {
	Q_OBJECT

public:
	MMGConfig();

	enum FileVersion : uint8_t {
		VERSION_0_0,
		VERSION_1_0,
		VERSION_2_0,
		VERSION_3_0,
		VERSION_3_1,
	};

	void blog(int log_status, const QString &message) const;

	void load(const QString &path_str = QString());
	void save(const QString &path_str = QString()) const;
	void finishLoad();
	void clearAllData();

	FileVersion fileVersion() const { return file_version; };

	MMGCollections *collections() const { return _collections; };
	MMGDeviceManager *devices() const { return _devices; };
	MMGPreferenceManager *preferences() const { return _preferences; };

	static const char *filename() { return "obs-midi-mg-config.json"; };
	static QString filepath(const QString &path_str);
	static FileVersion currentFileVersion() { return VERSION_3_1; };

signals:
	void refreshRequested();
	void midiStateChanged();

private:
	void findFileVersion();

private:
	MMGCollections *_collections;
	MMGDeviceManager *_devices;
	MMGPreferenceManager *_preferences;

	mutable QJsonObject doc;
	FileVersion file_version = currentFileVersion();
};

#define manager(which) config()->which##s()

#endif // MMG_CONFIG_H
