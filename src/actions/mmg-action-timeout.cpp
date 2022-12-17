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

#include "mmg-action-timeout.h"

#include <thread>

using namespace MMGUtils;

MMGActionTimeout::MMGActionTimeout(const QJsonObject &json_obj) : time(json_obj, "time", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Timeout> action created.");
}

void MMGActionTimeout::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Timeout> Action -> " + message);
}

void MMGActionTimeout::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  time.json(json_obj, "time", true);
}

void MMGActionTimeout::do_action(const MMGMessage *midi)
{
  switch (get_sub()) {
    case MMGActionTimeout::TIMEOUT_MS:
      std::this_thread::sleep_for(std::chrono::milliseconds((int)time.choose(midi)));
      break;
    case MMGActionTimeout::TIMEOUT_S:
      std::this_thread::sleep_for(std::chrono::seconds((int)time.choose(midi)));
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionTimeout::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  time.copy(&dest->num1());
}

void MMGActionTimeout::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Wait in Milliseconds", "Wait in Seconds"};
}
void MMGActionTimeout::change_options_str1(MMGActionDisplayParams &val)
{
  val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

  val.label_lcds[0] = "Time";
  val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI;
  val.lcds[0]->set_range(1.0, 1000.0);
  val.lcds[0]->set_step(1.0, 10.0);
  val.lcds[0]->set_default_value(1.0);
}
void MMGActionTimeout::change_options_str2(MMGActionDisplayParams &val) {}
void MMGActionTimeout::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionTimeout::change_options_final(MMGActionDisplayParams &val) {}
