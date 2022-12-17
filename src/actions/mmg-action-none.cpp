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

#include "mmg-action-none.h"

using namespace MMGUtils;

void MMGActionNone::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<None> Action -> " + message);
}

void MMGActionNone::json(QJsonObject &json_obj) const
{
  json_obj["category"] = 0;
  json_obj["sub"] = 0;
}

void MMGActionNone::do_action(const MMGMessage *midi)
{
  Q_UNUSED(midi);
  blog(LOG_DEBUG, "Executed successfully.");
  executed = true;
}

void MMGActionNone::deep_copy(MMGAction *dest) const {}

void MMGActionNone::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"None"};
}
void MMGActionNone::change_options_str1(MMGActionDisplayParams &val) {}
void MMGActionNone::change_options_str2(MMGActionDisplayParams &val) {}
void MMGActionNone::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionNone::change_options_final(MMGActionDisplayParams &val) {}
