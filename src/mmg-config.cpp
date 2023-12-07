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

MMGConfig::MMGConfig()
	: _devices(new MMGDeviceManager(this)),
	  _messages(new MMGMessageManager(this)),
	  _actions(new MMGActionManager(this)),
	  _bindings(new MMGBindingManager(this)),
	  _settings(new MMGSettingsManager(this)),

	  _signals(new MMGSignals(this)),
	  _midi(nullptr)
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

	auto default_path = obs_module_config_path(filepath().qtocs());
	QFile file(!path_str.isEmpty() ? qPrintable(path_str) : default_path);
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
	bfree(default_path);

	QJsonParseError parse_err;
	QJsonDocument doc = QJsonDocument::fromJson(config_contents, &parse_err);
	if (parse_err.error != QJsonParseError::NoError) {
		blog(LOG_INFO,
		     "Configuration file data could not be loaded correctly. Reason: " + parse_err.errorString());
	} else {
		blog(LOG_INFO, "Configuration file data loaded. Extracting...");
	}

	if (doc["config"].isArray()) {
		// 2.0.0 - 2.3.1 JSON file
		blog(LOG_INFO, "Old file data extraction detected. Converting...");

		QMap<MMGBinding *, QJsonObject> internal_action_objs;

		for (const QJsonValue &device_obj : doc["config"].toArray()) {
			QString device_name = device_obj["name"].toString();
			QStringList split = device_name.split(" ");

			if (bool ok; split.last().toUInt(&ok) >= 0 && ok) {
				split.removeLast();
				device_name = split.join(" ");
			}

			for (const QJsonValue &binding_obj : device_obj["bindings"].toArray()) {
				MMGBinding *binding = _bindings->add(binding_obj.toObject());
				if (_bindings->find(binding->name()) != binding) _bindings->setUniqueName(binding);

				MMGDevice *device = _devices->find(device_name);
				if (device) binding->setUsedDevices({device});

				MMGMessage *message = _messages->add(binding_obj["message"].toObject());
				message->setName(QString("[%1] %2").arg(device_name).arg(message->name()));
				if (_messages->find(message->name()) != message) _messages->setUniqueName(message);
				binding->setUsedMessages({message});

				QJsonObject action_obj = binding_obj["action"].toObject();
				if (action_obj["category"].toInt() == 16) {
					internal_action_objs[binding] = action_obj;
				} else {
					MMGAction *action = _actions->add(action_obj);
					action->setName(QString("[%1] %2").arg(device_name).arg(action->name()));
					if (_actions->find(action->name()) != action) _actions->setUniqueName(action);
					binding->setUsedActions({action});
				}
			}
		}

		for (MMGBinding *binding : internal_action_objs.keys()) {
			QJsonObject internal_action_obj = internal_action_objs[binding];
			MMGActionList actions;
			MMGAction *action;

			if (internal_action_obj.contains("str1")) {
				for (int i = 0; i < 3; ++i) {
					action = _actions->find(
						internal_action_obj[num_to_str(i + 1, "str")].toString());
					if (action) actions += action;
				}
			} else if (internal_action_obj.contains("action1")) {
				for (int i = 0; i < 3; ++i) {
					action = _actions->find(
						internal_action_obj[num_to_str(i + 1, "action")].toString());
					if (action) actions += action;
				}
			} else if (internal_action_obj.contains("actions")) {
				for (const QJsonValue &value : internal_action_obj["actions"].toArray()) {
					action = _actions->find(value["name"].toString());
					if (action) actions += action;
				}
			}

			binding->setUsedActions(actions);
		}

		blog(LOG_INFO, "Conversion complete.");
	} else if (doc["config"].isObject()) {
		// post 3.0.0 JSON file
		QJsonObject config_obj = doc["config"].toObject();
		_devices->load(config_obj["devices"].toArray());
		_messages->load(config_obj["messages"].toArray());
		_actions->load(config_obj["actions"].toArray());
		// Bindings must load last to get memory from other managers
		_bindings->load(config_obj["bindings"].toArray());
	}

	// Load input device from pre v2.3.0 versions
	auto device = _devices->find(doc["active_device"].toString());
	if (device) device->setActive(TYPE_INPUT, true);

	// Load settings
	if (doc["preferences"].isObject()) {
		QJsonObject preferences = doc["preferences"].toObject();
		if (preferences["thru_device"].isString() && device)
			device->setThru(preferences["thru_device"].toString());
	}
	QJsonArray settings_default_arr;
	for (uint i = 0; i < MMGSettingsManager::visiblePanes(); ++i)
		settings_default_arr += QJsonObject();
	_settings->load(doc["settings"].toArray(settings_default_arr));

	//finish:
	blog(LOG_INFO, "Configuration file data extraction complete.");
	blog(LOG_INFO, "Configuration loading complete.");
}

void MMGConfig::save(const QString &path_str) const
{
	QJsonObject save_obj;

	QJsonObject config_obj;
	_devices->json("devices", config_obj);
	_bindings->json("bindings", config_obj);
	_messages->json("messages", config_obj);
	_actions->json("actions", config_obj);
	save_obj["config"] = config_obj;

	_settings->json("settings", save_obj);

	save_obj["savedate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");

	auto default_path = obs_module_config_path(filepath().qtocs());
	QFile file(!path_str.isEmpty() ? qPrintable(path_str) : default_path);
	file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
	qlonglong result = file.write(json_to_str(save_obj));
	if (result < 0) {
		blog(LOG_INFO, "Configuration unable to be saved. Reason: " + file.errorString());
	} else {
		blog(LOG_INFO, QString("Configuration successfully saved to %1.").arg(file.fileName()));
	}
	file.close();
	bfree(default_path);
}

void MMGConfig::clearAllData()
{
	_messages->clear();
	_actions->clear();
	_bindings->clear();
	_settings->clear();
}

QString MMGConfig::filepath()
{
	return "obs-midi-mg-config.json";
}