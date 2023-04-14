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

#include "mmg-device.h"
#include "mmg-config.h"

using namespace MMGUtils;

qulonglong MMGDevice::next_default = 0;

MMGDevice::MMGDevice(const QJsonObject &data)
{
  _name = data["name"].toString();
  if (_name.isEmpty()) _name = get_next_default_name();
  if (data.contains("bindings")) {
    for (QJsonValue json_binding : data["bindings"].toArray()) {
      QJsonObject json_obj = json_binding.toObject();
      if (json_obj.isEmpty()) continue;
      add(new MMGBinding(json_binding.toObject()));
    }
  }

  check_binding_default_names();
  blog(LOG_DEBUG, "Device created.");
}

void MMGDevice::json(QJsonObject &device_obj) const
{
  QJsonArray json_bindings;
  device_obj["name"] = _name;
  for (const MMGBinding *const binding : _bindings) {
    QJsonObject json_binding;
    binding->json(json_binding);
    json_bindings += json_binding;
  }
  device_obj["bindings"] = json_bindings;
}

void MMGDevice::blog(int log_status, const QString &message) const
{
  global_blog(log_status, QString::asprintf("[Devices] <%s> ", _name.qtocs()) + message);
}

QString MMGDevice::get_next_default_name()
{
  return QVariant(++MMGDevice::next_default).toString().prepend(mmgtr("Device.Untitled"));
}

void MMGDevice::check_binding_default_names()
{
  for (const MMGBinding *binding : _bindings) {
    if (binding->name().contains(mmgtr("Binding.Untitled"))) {
      QString name = binding->name();
      qulonglong num = QVariant(name.split(mmgtr("Binding.Untitled")).last()).toULongLong();
      if (num > MMGBinding::get_next_default()) MMGBinding::set_next_default(num);
    }
  }
}

MMGBinding *MMGDevice::add(MMGBinding *const el)
{
  _bindings.append(el);
  el->setParent(this);
  return el;
}

MMGBinding *MMGDevice::copy(MMGBinding *const el)
{
  MMGBinding *new_binding = add();
  QString old_name = new_binding->name();
  el->copy(new_binding);
  new_binding->setName(QString::asprintf(mmgtr("Binding.Copy"), el->name().qtocs()) + old_name);
  return new_binding;
}

void MMGDevice::move(int from, int to)
{
  if (from >= size()) return;

  setConnected(false);
  to == size() ? _bindings.append(_bindings.takeAt(from)) : _bindings.move(from, to);
  setConnected(true);
}

void MMGDevice::remove(MMGBinding *const el)
{
  _bindings.removeAt(indexOf(el));
}

int MMGDevice::indexOf(MMGBinding *const el)
{
  int i = -1;
  for (MMGBinding *const binding : _bindings) {
    ++i;
    if (binding == el) return i;
  }
  return -1;
}

qint64 MMGDevice::size() const
{
  return _bindings.size();
}

MMGBinding *MMGDevice::find(const QString &name)
{
  for (MMGBinding *const el : _bindings) {
    if (el->name() == name) return el;
  }
  return nullptr;
}

void MMGDevice::setConnected(bool connected)
{
  for (MMGBinding *binding : _bindings)
    binding->setConnected(connected);
}

void MMGDevice::copy(MMGDevice *dest)
{
  dest->setName(_name);
  for (MMGBinding *binding : _bindings) {
    MMGBinding *new_binding = dest->add();
    binding->copy(new_binding);
  }
}
