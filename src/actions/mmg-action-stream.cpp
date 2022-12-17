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

#include "mmg-action-stream.h"

using namespace MMGUtils;

MMGActionStream::MMGActionStream(const QJsonObject &json_obj)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Stream> action created.");
}

void MMGActionStream::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Stream> Action -> " + message);
}

void MMGActionStream::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
}

void MMGActionStream::do_action(const MMGMessage *midi)
{
  Q_UNUSED(midi);
  switch (get_sub()) {
    case MMGActionStream::STREAM_ON:
      if (!obs_frontend_streaming_active()) obs_frontend_streaming_start();
      break;
    case MMGActionStream::STREAM_OFF:
      if (obs_frontend_streaming_active()) obs_frontend_streaming_stop();
      break;
    case MMGActionStream::STREAM_TOGGLE_ONOFF:
      if (obs_frontend_streaming_active()) {
	obs_frontend_streaming_stop();
      } else {
	obs_frontend_streaming_start();
      }
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Executed successfully.");
  executed = true;
}

void MMGActionStream::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
}

void MMGActionStream::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Start Streaming", "Stop Streaming", "Toggle Streaming"};
}
void MMGActionStream::change_options_str1(MMGActionDisplayParams &val) {}
void MMGActionStream::change_options_str2(MMGActionDisplayParams &val) {}
void MMGActionStream::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionStream::change_options_final(MMGActionDisplayParams &val) {}
