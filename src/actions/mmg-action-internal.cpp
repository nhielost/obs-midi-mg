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

#include "mmg-action-internal.h"
#include "../mmg-config.h"

using namespace MMGUtils;

MMGActionInternal::MMGActionInternal(const QJsonObject &json_obj)
  : actions{{json_obj, "action1", 1}, {json_obj, "action2", 2}, {json_obj, "action3", 3}}
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Internal> action created.");
}

void MMGActionInternal::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Internal> Action -> " + message);
}

void MMGActionInternal::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  for (int i = 0; i < 3; ++i) {
    actions[i].json(json_obj, num_to_str(i + 1, "action"), false);
  }
}

void MMGActionInternal::do_action(const MMGMessage *midi)
{
  MMGDevice *device = global()->find_current_device();
  if (get_sub() > 2 || get_sub() < 0) return;

  int i = 0;
  while (get_sub() >= i) {
    MMGBinding *binding = device->find_binding(actions[i]);
    if (!binding) return;
    binding->get_action()->do_action(midi);
    ++i;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionInternal::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  actions[0].copy(&dest->str1());
  actions[1].copy(&dest->str2());
  actions[2].copy(&dest->str3());
}

const QStringList MMGActionInternal::enumerate(const QString &current_binding)
{
  QStringList list;
  for (MMGBinding *const binding : global()->find_current_device()->get_bindings()) {
    if (binding->get_name() != current_binding) list.append(binding->get_name());
  }
  return list;
}

void MMGActionInternal::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Do 1 Action", "Do 2 Actions", "Do 3 Actions"};
}
void MMGActionInternal::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.list = enumerate(val.extra_data);
  val.label_text = "Action 1";
}
void MMGActionInternal::change_options_str2(MMGActionDisplayParams &val)
{
  if (subcategory == 0) return;
  val.display = MMGActionDisplayParams::DISPLAY_STR2;
  val.list = enumerate(val.extra_data);
  val.label_text = "Action 2";
}
void MMGActionInternal::change_options_str3(MMGActionDisplayParams &val)
{
  if (subcategory < 2) return;
  val.display = MMGActionDisplayParams::DISPLAY_STR3;
  val.list = enumerate(val.extra_data);
  val.label_text = "Action 3";
}
void MMGActionInternal::change_options_final(MMGActionDisplayParams &val) {}
