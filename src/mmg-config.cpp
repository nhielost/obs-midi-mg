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

void MMGConfig::check_device_default_names()
{
	for (const MMGDevice *device : devices) {
		if (device->get_name().startsWith("Untitled Device ")) {
			QString name = device->get_name();
			qulonglong num =
				QVariant(name.remove("Untitled Device "))
					.toULongLong();
			if (num > MMGDevice::get_next_default())
				MMGDevice::set_next_default(num);
		}
	}
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
		// JSON file found, introduce devices from the file (even if nonexistent)
		for (QJsonValue obj : doc["config"].toArray()) {
			if (json_is_valid(obj, QJsonValue::Object) &&
			    !obj.toObject().isEmpty()) {
				devices.append(new MMGDevice(obj.toObject()));
			}
		}
		// Load preferences
		if (json_is_valid(doc["preferences"], QJsonValue::Object))
			settings = MMGSettings(doc["preferences"].toObject());
	}
	// Check for devices currently connected that are not included
	// and include them if necessary
	for (const QString &name : MMGDevice::get_input_device_names()) {
		bool device_included = false;
		for (MMGDevice *device : devices) {
			device_included |= (device->get_name() == name);
		}
		if (!device_included) {
			devices.append(new MMGDevice());
			devices.last()->set_name(name);
			devices.last()->open_input_port();
			devices.last()->open_output_port();
		}
	}
	for (const QString &name : MMGDevice::get_output_device_names()) {
		bool device_included = false;
		for (MMGDevice *device : devices) {
			device_included |= (device->get_name() == name);
		}
		if (!device_included) {
			devices.append(new MMGDevice());
			devices.last()->set_name(name);
			devices.last()->open_input_port();
			devices.last()->open_output_port();
		}
	}
	// Check for any unaffiliated devices (i.e. created by obs-midi-mg)
	check_device_default_names();
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

	QJsonObject json_settings;
	settings.json(json_settings);
	config_obj["preferences"] = json_settings;

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
			if (!device->input_port_open()) {
				device->open_input_port();
			}
			if (!device->output_port_open()) {
				device->open_output_port();
			}
			return device;
		}
	}
	return nullptr;
}

const QStringList MMGConfig::get_device_names() const
{
	QStringList names;
	for (MMGDevice *const device : devices) {
		names.append(device->get_name());
	}
	return names;
}

QString MMGConfig::get_filepath()
{
	return "obs-midi-mg-config.json";
}
