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
#include <QFile>

using namespace MMGUtils;

void MMGConfig::blog(int log_status, const QString &message) const
{
	QString temp_msg = "Config -> ";
	temp_msg.append(message);
	global_blog(log_status, temp_msg);
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
	blog(LOG_INFO, "Loading configuration...");

	auto default_path = obs_module_config_path(get_filepath().qtocs());
	QFile file(!path_str.isEmpty() ? qPrintable(path_str) : default_path);
	file.open(QFile::ReadOnly | QFile::Text);
	QByteArray config_contents;
	if (file.exists()) {
		QString log_str = "Loading configuration file data from ";
		log_str.append(file.fileName());
		log_str.append("...");
		blog(LOG_INFO, log_str);

		config_contents = file.readAll();

		/*midi_config = obs_data_create_from_json_file(
			!path_str.isEmpty() ? qPrintable(path_str)
					   : default_path);*/
	} else {
		blog(LOG_INFO,
		     "Configuration file not found. Loading new data...");
		config_contents = "{}";
	}
	file.close();
	bfree(default_path);

	QJsonParseError parse_err;
	QJsonDocument doc =
		QJsonDocument::fromJson(config_contents, &parse_err);
	if (parse_err.error != QJsonParseError::NoError) {
		blog(LOG_INFO,
		     "Configuration file data could not be loaded. Reason: ");
		blog(LOG_INFO, parse_err.errorString());
	} else {
		blog(LOG_INFO, "Configuration file data loaded. Extracting...");
	}

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
	blog(LOG_INFO, "Configuration file data extraction complete.");
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
	blog(LOG_INFO, "New devices loaded.");
	// Check for any unaffiliated devices (i.e. created by obs-midi-mg)
	check_device_default_names();
	blog(LOG_INFO, "Configuration loading complete.");
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

	auto default_path = obs_module_config_path(get_filepath().qtocs());
	QFile file(!path_str.isEmpty() ? qPrintable(path_str) : default_path);
	file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
	qlonglong result = file.write(
		QJsonDocument(config_obj).toJson(QJsonDocument::Compact));
	if (result < 0) {
		blog(LOG_INFO, "Configuration unable to be saved. Reason: ");
		blog(LOG_INFO, file.errorString());
	} else {
		QString log_str = "Configuration successfully saved to ";
		log_str.append(file.fileName());
		log_str.append(".");
		blog(LOG_INFO, log_str);
	}
	file.close();
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
