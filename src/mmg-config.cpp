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

#include "mmg-config.h"
#include "mmg-utils.h"

#include <obs-module.h>

#include <util/platform.h>

#include <QDateTime>

using namespace MMGUtils;

MMGConfig::MMGConfig()
{
	load();
}

void MMGConfig::load(const QString &path_str)
{
	auto default_path = obs_module_config_path(get_filepath().qtocs());
	OBSDataAutoRelease midi_config;
	if (os_file_exists(default_path) || !path_str.isNull()) {
		midi_config = obs_data_create_from_json_file(
			!path_str.isNull() ? qPrintable(path_str)
					   : default_path);
	} else {
		midi_config = obs_data_create();
		obs_data_save_json_safe(midi_config, default_path, ".tmp",
					".bkp");
	}
	bfree(default_path);

	QJsonDocument doc =
		QJsonDocument::fromJson(obs_data_get_json(midi_config));

	if (!devices.isEmpty())
		clear();

	if (json_key_exists(doc.object(), "config", QJsonValue::Array)) {
		for (QJsonValue obj : doc["config"].toArray()) {
			if (json_is_valid(obj, QJsonValue::Object) &&
			    !obj.toObject().isEmpty()) {
				devices.append(new MMGDevice(obj.toObject()));
			}
		}
	}
	active = doc["active"].toBool(true);
	active_device_name = doc["active_device"].toString("Error");
}

void MMGConfig::save(const QString &path_str) const
{
	QJsonArray config_array;
	for (const MMGDevice *const device : devices) {
		QJsonObject json_device;
		device->json(json_device);
		config_array += json_device;
	}
	QJsonObject config_obj;
	config_obj["config"] = config_array;
	config_obj["savedate"] = QDateTime::currentDateTime().toString(
		"yyyy-MM-dd hh-mm-ss-zzz");
	config_obj["active"] = active;
	config_obj["active_device"] = active_device_name;
	OBSDataAutoRelease save_data =
		obs_data_create_from_json(QJsonDocument(config_obj).toJson());
	auto default_path = obs_module_config_path(get_filepath().qtocs());
	obs_data_save_json_safe(save_data,
				!path_str.isNull() ? qPrintable(path_str)
						   : default_path,
				".tmp", ".bkp");

	bfree(default_path);
}

void MMGConfig::clear()
{
	qDeleteAll(devices);
	devices.clear();
}

MMGDevice *MMGConfig::find_device(const QString &name)
{
	for (MMGDevice *const device : devices) {
		if (device->get_name() == name) {
			active_device_name = name;
			if (MMGDevice::get_input_port_number(name) >= 0 &&
			    !device->get_input_device().is_port_open()) {
				device->get_input_device().set_callback(
					MMGUtils::call_midi_callback);
				device->get_input_device().open_port(
					MMGDevice::get_input_port_number(name));
			}
			return device;
		}
	}
	active_device_name = "Untitled Device 1";
	return nullptr;
}

MMGDevice *MMGConfig::get_active_device() const
{
	for (MMGDevice *const device : devices) {
		if (device->get_name() == active_device_name) {
			return device;
		}
	}
	return nullptr;
}

QString MMGConfig::get_filepath()
{
	return "obs-midi-mg-config.json";
}
