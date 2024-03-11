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

#include "mmg-config.h"
#include "mmg-utils.h"

#include <QDateTime>
#include <QFile>

using namespace MMGUtils;

// MMGConfig
MMGConfig::MMGConfig()
	: _collections(new MMGCollections(this)),
	  _devices(new MMGDeviceManager(this)),
	  _settings(new MMGSettings(this)),

	  _signals(new MMGSignals(this)),
	  _midi(nullptr),

	  old_config(new MMGOldConfig(this))
{
}

void MMGConfig::blog(int log_status, const QString &message) const
{
	global_blog(log_status, "[Configuration] " + message);
}

void MMGConfig::load(const QString &path_str)
{
	// Init MIDI on first load, and clear old data on imports
	if (!_midi) {
		blog(LOG_INFO, "Initializing MIDI...");
		_midi = new MMGMIDI(this);
		blog(LOG_INFO, "MIDI initialized.");
	} else {
		clearAllData();
	}

	blog(LOG_INFO, "Loading configuration...");

	QFile file(filepath(path_str));
	file.open(QFile::ReadOnly | QFile::Text);
	QByteArray config_contents;
	if (file.exists()) {
		blog(LOG_INFO, QString("Loading configuration file data from %1...").arg(file.fileName()));
		config_contents = file.readAll();
	} else {
		blog(LOG_INFO, "Configuration file not found. Loading new configuration data...");
		config_contents = "{}";
	}
	file.close();

	QJsonParseError parse_err;
	QJsonObject doc = QJsonDocument::fromJson(config_contents, &parse_err).object();
	if (parse_err.error != QJsonParseError::NoError) {
		blog(LOG_INFO,
		     "Configuration file data could not be loaded correctly. Reason: " + parse_err.errorString());
	} else {
		blog(LOG_INFO, "Configuration file data loaded. Extracting...");
	}

	old_config->load(doc);

	_devices->load(doc["devices"].toArray());
	_collections->load(doc["collections"].toArray());
	_settings->load(doc["preferences"].toObject());

	old_config->postLoad();

	blog(LOG_INFO, "Configuration loading complete.");
}

void MMGConfig::save(const QString &path_str) const
{
	QJsonObject save_obj;

	_devices->json("devices", save_obj);
	_collections->json("collections", save_obj);
	_settings->json("preferences", save_obj);

	save_obj["savedate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");
	save_obj["version"] = OBS_MIDIMG_VERSION;

	QFile file(filepath(path_str));
	file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
	qlonglong result = file.write(json_to_str(save_obj));
	if (result < 0) {
		blog(LOG_INFO, "Configuration unable to be saved. Reason: " + file.errorString());
	} else {
		blog(LOG_INFO, QString("Configuration successfully saved to %1.").arg(file.fileName()));
	}
	file.close();
}

void MMGConfig::clearAllData()
{
	_collections->clear();
}

QString MMGConfig::filepath(const QString &path_str)
{
	auto default_path = obs_module_config_path("obs-midi-mg-config.json");
	QString full_path = !path_str.isEmpty() ? path_str : QString(default_path);
	bfree(default_path);
	return full_path;
}
// End MMGConfig

// MMGOldConfig
void MMGOldConfig::load(QJsonObject &doc)
{
	if (!doc["config"].isArray()) return;

	// 2.0.0 - 2.3.1 JSON file (before 2.0.0 is not supported)
	blog(LOG_INFO, "Old file data detected. Converting...");

	QJsonArray device_array;
	QJsonArray collection_array;

	for (const QJsonValue &device_val : doc["config"].toArray()) {
		QJsonObject collection_obj;

		// Get Proper Device Name and Setup
		QString device_name = device_val["name"].toString();
		cleanDeviceName(device_name);

		QJsonObject device_obj;
		device_obj["name"] = device_name;
		if (doc["active_device"].toString().contains(device_name)) {
			device_obj["active"] = 1;
			device_obj["thru"] = doc["preferences"][QString("thru_device")].toString();
		}
		device_array += device_obj;
		collection_obj["name"] = device_name;

		// Get Bindings
		QJsonArray binding_array;

		for (const QJsonValue &binding_val : device_val["bindings"].toArray()) {
			QJsonObject binding_obj = binding_val.toObject();

			QJsonArray message_array;
			binding_obj["message"].toObject()["device"] = device_name;
			message_array += binding_obj["message"];
			binding_obj["messages"] = message_array;

			QJsonArray action_array;
			QJsonObject action_obj = binding_obj["action"].toObject();
			if (action_obj["category"].toInt() == 16) {
				if (action_obj.contains("actions")) {
					for (const QJsonValue &action_val : action_obj["actions"].toArray()) {
						QJsonObject internal_obj;
						internal_obj["name"] = action_val["action"];
						action_array += internal_obj;
					}
				} else {
					for (int i = 0; i < 3; ++i) {
						QJsonObject internal_obj;
						internal_obj["name"] =
							action_obj[(action_obj.contains("str1") ? "str" : "action") +
								   QString::number(i + 1)];
						action_array += internal_obj;
					}
				}
				old_internal_bindings.insert(device_name, binding_obj["name"].toString());
			} else {
				action_array += action_obj;
			}
			binding_obj["actions"] = action_array;

			binding_array += binding_obj;
		}
		collection_obj["bindings"] = binding_array;

		collection_array += collection_obj;
	}

	doc["devices"] = device_array;
	doc["collections"] = collection_array;

	blog(LOG_INFO, "Conversion completed successfully.");
}

void MMGOldConfig::postLoad()
{
	for (const QString &collection_name : old_internal_bindings.keys()) {
		MMGBindingManager *collection = manager(collection)->find(collection_name);
		if (!collection) continue;

		for (const QString &binding_name : old_internal_bindings.values(collection_name)) {
			MMGBinding *binding = collection->find(binding_name);
			if (!binding) continue;

			QStringList action_list = binding->actions()->names();
			binding->actions()->clear();
			for (const QString &action : action_list) {
				MMGBinding *internal_action_binding = collection->find(action);
				if (!internal_action_binding) continue;

				binding->actions()->copy(internal_action_binding->actions(0))->setObjectName(action);
			}
		}
	}

	old_internal_bindings.clear();
}

void MMGOldConfig::cleanDeviceName(QString &device_name) const
{
	QStringList split = device_name.split(" ");

	if (bool ok; split.last().toUInt(&ok) >= 0 && ok) {
		split.removeLast();
		device_name = split.join(" ");
	}
}
// End MMGOldConfig
