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

struct MMGSettings {
  public:
  MMGSettings() = default;
  explicit MMGSettings(const QJsonObject &settings_obj);
  void json(QJsonObject &settings_obj) const;

  bool active() const { return _active; };
  const QString &thruDevice() const { return thru_device; };
  void setActive(bool is_active);
  void setThruDevice(const QString &name) { thru_device = name; };

  private:
  bool _active = true;
  QString thru_device;
};

class MMGConfig : public QObject {
  Q_OBJECT

  public:
  MMGConfig();

  void blog(int log_status, const QString &message) const;

  void load(const QString &path_str = QString());
  void save(const QString &path_str = QString()) const;
  void clear();
  void refresh();
  const QString &activeDeviceName() { return active_device_name; };
  void setActiveDeviceName(const QString &name);
  MMGDevice *find(const QString &name);
  MMGDevice *currentDevice() { return find(active_device_name); };
  const QStringList allDeviceNames() const;
  MMGSettings *preferences() { return &settings; };
  static QString filepath();

  private:
  MMGDevices devices;
  MMGSettings settings;
  QString active_device_name = "";

  void check_device_default_names();
};

#endif // MMG_CONFIG_H
