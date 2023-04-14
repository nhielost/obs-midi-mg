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

#ifndef MMG_DEVICE_H
#define MMG_DEVICE_H

#include "mmg-binding.h"

class MMGDevice : public QObject {
  Q_OBJECT

  public:
  MMGDevice() = default;
  explicit MMGDevice(const QJsonObject &data);

  void json(QJsonObject &device_obj) const;
  void blog(int log_status, const QString &message) const;

  const QString &name() const { return _name; };
  void setName(const QString &val) { _name = val; };

  void copy(MMGDevice *dest);

  MMGBinding *add(MMGBinding *el = new MMGBinding);
  MMGBinding *copy(MMGBinding *el);
  void move(int from, int to);
  void remove(MMGBinding *el);
  MMGBinding *find(const QString &name);
  void setConnected(bool connected);
  int indexOf(MMGBinding *el);
  qint64 size() const;
  void clear() { _bindings.clear(); };

  const MMGBindingList &bindings() const { return _bindings; };

  static qulonglong get_next_default() { return next_default; };
  static void set_next_default(qulonglong num) { next_default = num; };
  static QString get_next_default_name();

  private:
  QString _name;
  QList<MMGBinding *> _bindings;

  static qulonglong next_default;

  void check_binding_default_names();
};

using MMGDevices = QList<MMGDevice *>;

#endif // MMG_DEVICE_H
