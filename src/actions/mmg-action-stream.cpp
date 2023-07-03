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

void MMGActionStream::execute(const MMGMessage *midi) const
{
  Q_UNUSED(midi);
  switch (sub()) {
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
}

void MMGActionStream::setSubOptions(QComboBox *sub)
{
  sub->addItems({"Start Streaming", "Stop Streaming", "Toggle Streaming"});
}
