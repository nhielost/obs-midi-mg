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

#include <obs-module.h>

#include <util/platform.h>

#include <QDateTime>
#include <QFile>
#include <QThread>

using namespace MMGUtils;

#define MMG_ENABLED_CHECK \
  if (!editable) return

// MMGSettings
MMGSettings::MMGSettings(const QJsonObject &settings_obj)
{
  setActive(settings_obj["active"].toBool(true));
  setThruDevice(settings_obj["thru_device"].toString());
  setInternalBehavior(settings_obj["internal_behavior"].toInt());
}

void MMGSettings::json(QJsonObject &settings_obj) const
{
  settings_obj["active"] = _active;
  settings_obj["thru_device"] = thru_device;
  settings_obj["internal_behavior"] = internal_behavior_state;
}

void MMGSettings::setActive(bool is_active)
{
  MMG_ENABLED_CHECK;
  if (is_active == _active) return;

  _active = is_active;
  emit activeChanged(is_active);

  if (!global()) return;
  if (!_active) {
    input_device()->closeInputPort();
  } else {
    if (global()->currentDevice()) input_device()->openInputPort(global()->currentDevice());
  }
}

void MMGSettings::setThruDevice(const QString &name)
{
  MMG_ENABLED_CHECK;
  if (name == thru_device && name != "") return;

  if (name == "")
    thru_device = mmgtr("Plugin.Disabled");
  else
    thru_device = name;
  emit thruDeviceChanged(name);
}

void MMGSettings::setInternalBehavior(short state)
{
  MMG_ENABLED_CHECK;
  if (state == internal_behavior_state) return;

  internal_behavior_state = state;
  emit internalBehaviorChanged(state);
}

#undef MMG_ENABLED_CHECK
// End MMGSettings

// MMGConfig
MMGConfig::MMGConfig()
{
  load();
};

void MMGConfig::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "[Configuration] " + message);
}

void MMGConfig::check_device_default_names()
{
  for (const MMGDevice *device : devices) {
    if (device->name().startsWith(mmgtr("Device.Untitled"))) {
      QString name = device->name();
      qulonglong num = QVariant(name.remove(mmgtr("Device.Untitled"))).toULongLong();
      if (num > MMGDevice::get_next_default()) MMGDevice::set_next_default(num);
    }
  }
}

void MMGConfig::load(const QString &path_str)
{
  blog(LOG_INFO, "Loading configuration...");

  auto default_path = obs_module_config_path(filepath().qtocs());
  QFile file(!path_str.isEmpty() ? qPrintable(path_str) : default_path);
  file.open(QFile::ReadOnly | QFile::Text);
  QByteArray config_contents;
  if (file.exists()) {
    blog(LOG_INFO,
	 QString::asprintf("Loading configuration file data from %s...", file.fileName().qtocs()));
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
	 "Configuration file data could not be loaded. Reason: " + parse_err.errorString());
  } else {
    blog(LOG_INFO, "Configuration file data loaded. Extracting...");
  }

  if (!devices.isEmpty()) clear();

  if (doc.object().contains("config")) {
    // JSON file found, introduce devices from the file (even if nonexistent)
    for (QJsonValue device : doc["config"].toArray()) {
      // If file is old, kill the import process as it is unsupported after v2.3.0
      if (device["bindings"].isArray()) {
	if (device["bindings"][0].toObject().contains("actions")) {
	  blog(LOG_INFO, "Configuration file data is outdated and cannot be accepted.");
	  blog(LOG_INFO, "Reverting to default configuration...");
	  active_device = nullptr;
	  goto finish;
	}
      }

      QJsonObject device_obj = device.toObject();
      if (device_obj.isEmpty()) continue;
      devices.append(new MMGDevice(device_obj));
      devices.last()->setParent(this);
    }

    // Load active device and disconnect all bindings not a part of the active device
    active_device = find(doc["active_device"].toString());
    for (MMGDevice *device : devices)
      if (device != active_device) device->setConnected(false);

    // Load preferences
    if (doc["preferences"].isObject()) {
      settings = new MMGSettings(doc["preferences"].toObject());
      settings->setParent(this);
    }
  }

finish:
  blog(LOG_INFO, "Configuration file data extraction complete.");

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

  config_obj["savedate"] = QDateTime::currentDateTime().toString("yyyy-MM-dd hh-mm-ss-zzz");

  QJsonObject json_settings;
  settings->json(json_settings);
  config_obj["preferences"] = json_settings;

  config_obj["active_device"] = active_device ? active_device->name() : "";

  auto default_path = obs_module_config_path(filepath().qtocs());
  QFile file(!path_str.isEmpty() ? qPrintable(path_str) : default_path);
  file.open(QFile::WriteOnly | QFile::Text | QFile::Truncate);
  qlonglong result = file.write(json_to_str(config_obj));
  if (result < 0) {
    blog(LOG_INFO, "Configuration unable to be saved. Reason: " + file.errorString());
  } else {
    blog(LOG_INFO,
	 QString::asprintf("Configuration successfully saved to %s.", file.fileName().qtocs()));
  }
  file.close();
  bfree(default_path);
}

void MMGConfig::clear()
{
  qDeleteAll(devices);
  devices.clear();
}

void MMGConfig::refresh()
{
  // Check for devices currently connected that are not included
  // and include them if necessary
  for (const QString &name : input_device()->inputDeviceNames()) {
    bool device_included = false;
    for (MMGDevice *device : devices) {
      device_included |= (device->name() == name);
    }
    if (!device_included) {
      devices.append(new MMGDevice());
      devices.last()->setName(name);
      blog(LOG_INFO, QString::asprintf("New device <%s> detected.", name.qtocs()));
    }
  }
  for (const QString &name : output_device()->outputDeviceNames()) {
    bool device_included = false;
    for (MMGDevice *device : devices) {
      device_included |= (device->name() == name);
    }
    if (!device_included) {
      devices.append(new MMGDevice());
      devices.last()->setName(name);
      blog(LOG_INFO, QString::asprintf("New device <%s> detected.", name.qtocs()));
    }
  }

  // Check if any devices are disconnected AND empty and remove them
  for (MMGDevice *device : devices) {
    if (input_device()->inputPort(device) == (uint)-1 &&
	output_device()->outputPort(device) == (uint)-1 && device->size() == 0) {
      blog(LOG_INFO, QString::asprintf("Empty device <%s> removed.", device->name().qtocs()));

      if (active_device == device) active_device = nullptr;
      devices.removeOne(device);
      delete device;
    }
  }

  // Open the current device
  if (active_device && settings->active()) input_device()->openInputPort(active_device);
}

void MMGConfig::setActiveDevice(const QString &name)
{
  if (active_device) {
    if (active_device->name() == name) return;
    active_device->setConnected(false);
  }

  active_device = find(name);
  if (!active_device) return;

  active_device->setConnected(true);
  if (settings->active()) input_device()->openInputPort(active_device);
}

MMGDevice *MMGConfig::find(const QString &name)
{
  for (MMGDevice *device : devices) {
    if (device->name() == name) return device;
  }
  return nullptr;
}

const QStringList MMGConfig::allDeviceNames() const
{
  QStringList names;
  for (MMGDevice *const device : devices) {
    names.append(device->name());
  }
  return names;
}

QString MMGConfig::filepath()
{
  return "obs-midi-mg-config.json";
}
// End MMGConfig
