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

class MMGSettings : public QObject {
  Q_OBJECT

  public:
  MMGSettings() = default;
  explicit MMGSettings(const QJsonObject &json_obj);
  void json(QJsonObject &json_obj) const;

  bool isEditable() const { return editable; };
  bool active() const { return _active; };
  const QString &thruDevice() const { return thru_device; };
  short internalBehavior() const { return internal_behavior_state; };

  signals:
  void activeChanged(bool);
  void thruDeviceChanged(const QString &);
  void internalBehaviorChanged(short);

  public slots:
  void setEditable(bool edit) { editable = edit; };
  void setActive(bool is_active);
  void setThruDevice(const QString &name);
  void setInternalBehavior(short state);

  private:
  bool _active = true;
  QString thru_device;
  short internal_behavior_state = 0;

  bool editable = true;
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
  void setActiveDevice(const QString &name);
  MMGDevice *find(const QString &name);
  MMGDevice *currentDevice() { return active_device; };
  const QStringList allDeviceNames() const;
  MMGSettings *preferences() { return settings; };
  static QString filepath();

  private:
  MMGDevices devices;
  MMGSettings *settings;
  MMGDevice *active_device = nullptr;

  void check_device_default_names();
};

#endif // MMG_CONFIG_H
