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

#include "mmg-config.h"

#include <QFile>

namespace MMGCompatibility {

static QMultiMap<QString, QString> old_internal_bindings;
static void oldConfigPreLoad(QJsonObject &);
static void oldConfigPostLoad();

} // namespace MMGCompatibility

// MMGConfig
MMGConfig::MMGConfig()
	: _collections(new MMGCollections(this, "collections")),
	  _devices(new MMGDeviceManager(this, "devices")),
	  _preferences(new MMGPreferenceManager(this, "preferences"))
{
	MMGSignal::initSignals();

	QJsonObject json_obj;
	for (auto &[id, name] : MMGPreferences::availablePreferences()) {
		MMGJson::setValue(json_obj, "id", id);
		auto *preference = _preferences->add(json_obj);
		preference->setObjectName(name.translate());
	}
}

void MMGConfig::blog(int log_status, const QString &message) const
{
	mmgblog(log_status, "[Configuration] " + message);
}

void MMGConfig::findFileVersion()
{
	if (doc.contains("file_version")) { // v3.1.0+
		file_version = FileVersion(doc["file_version"].toInt());
		if (file_version < VERSION_3_1 || file_version > currentFileVersion())
			file_version = currentFileVersion();
	} else if (doc.contains("collections")) { // v3.0.0 - v3.0.3
		file_version = VERSION_3_0;
	} else if (doc.contains("config") && doc["config"].isArray()) { // v2.0.0 - v2.3.0
		file_version = VERSION_2_0;
	} else { // For new configurations or file versions before v2.0.0
		doc = {};
		file_version = currentFileVersion();
	}
}

void MMGConfig::load(const QString &path_str)
{
	clearAllData();

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
	doc = QJsonDocument::fromJson(config_contents, &parse_err).object();
	if (parse_err.error != QJsonParseError::NoError) {
		blog(LOG_INFO,
		     "Configuration file data could not be loaded correctly. Reason: " + parse_err.errorString());
	} else {
		blog(LOG_INFO, "Configuration file data loaded. Extraction will begin when "
			       "OBS has finished loading.");
	}

	findFileVersion();
}

void MMGConfig::finishLoad()
{
	blog(LOG_INFO, "Extracting configuration file data...");

	FileVersion loading_file_version = file_version;
	if (loading_file_version == VERSION_2_0) MMGCompatibility::oldConfigPreLoad(doc);

	QJsonObject preferences_obj = doc["preferences"].toObject();
	for (auto preference : *_preferences)
		preference->load(preferences_obj);

	_devices->load(doc);
	_collections->load(doc);

	doc = {};
	file_version = currentFileVersion();

	// Importing files from v2.x has special logic for Internal actions
	// that must occur after fully loading everything else
	if (loading_file_version == VERSION_2_0) MMGCompatibility::oldConfigPostLoad();

	for (MMGBindingManager *manager : *_collections)
		for (MMGBinding *binding : *manager)
			binding->refresh();

	blog(LOG_INFO, "Configuration loading complete.");
}

void MMGConfig::save(const QString &path_str) const
{
	QJsonObject preferences_obj;
	for (auto preference : *_preferences)
		preference->json(preferences_obj);
	doc["preferences"] = preferences_obj;

	_devices->json(doc);
	_collections->json(doc);

	doc["savedate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");
	doc["plugin_version"] = OBS_MIDIMG_VERSION_DISPLAY;
	doc["file_version"] = currentFileVersion();

	QFile file(filepath(path_str));
	file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
	qlonglong result = file.write(MMGJson::toString(doc));
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
	auto default_path = obs_module_config_path(filename());
	QString full_path = !path_str.isEmpty() ? path_str : QString(default_path);
	bfree(default_path);
	return full_path;
}
// End MMGConfig

namespace MMGCompatibility {

static void cleanDeviceName(QString &device_name)
{
	QStringList split = device_name.split(" ");

	bool ok;
	split.last().toUInt(&ok);
	if (!ok) return;

	split.removeLast();
	device_name = split.join(" ");
}

static void oldConfigPreLoad(QJsonObject &doc)
{
	// File version 3 = 2.0.0 - 2.3.1 JSON file (before 2.0.0 is not supported)
	blog(LOG_INFO, "Old file data detected. Converting...");

	QJsonArray device_array;
	QJsonArray collection_array;

	for (QJsonValue device_val : doc["config"].toArray()) {
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

		for (QJsonValue binding_val : device_val["bindings"].toArray()) {
			QJsonObject binding_obj = binding_val.toObject();

			QJsonArray message_array;
			binding_obj["message"].toObject()["device"] = device_name;
			message_array += binding_obj["message"];
			binding_obj["messages"] = message_array;

			QJsonArray action_array;
			QJsonObject action_obj = binding_obj["action"].toObject();
			if (action_obj["category"].toInt() == 16) {
				if (action_obj.contains("actions")) {
					for (QJsonValue action_val : action_obj["actions"].toArray()) {
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

static void oldConfigPostLoad()
{
	for (const QString &collection_name : old_internal_bindings.keys()) {
		MMGBindingManager *collection = manager(collection)->find(collection_name);
		if (!collection) continue;

		for (const QString &binding_name : old_internal_bindings.values(collection_name)) {
			MMGBinding *binding = collection->find(binding_name);
			if (!binding) continue;

			MMGTranslationMap<MMGAction *> action_map = binding->actions()->names();
			binding->actions()->clear();
			for (const MMGText &name : action_map.values()) {
				MMGBinding *internal_action_binding = collection->find(name);
				if (!internal_action_binding) continue;

				binding->actions()
					->copy(internal_action_binding->actions(0))
					->setObjectName(name.translate());
			}
		}
	}

	old_internal_bindings.clear();
}

} // namespace MMGCompatibility
