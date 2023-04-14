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

#include "mmg-action-record.h"

using namespace MMGUtils;

MMGActionRecord::MMGActionRecord(const QJsonObject &json_obj)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionRecord::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Recording] " + message);
}

void MMGActionRecord::execute(const MMGMessage *midi) const
{
  Q_UNUSED(midi);
  switch (sub()) {
    case MMGActionRecord::RECORD_ON:
      if (!obs_frontend_recording_active()) obs_frontend_recording_start();
      break;
    case MMGActionRecord::RECORD_OFF:
      if (obs_frontend_recording_active()) obs_frontend_recording_stop();
      break;
    case MMGActionRecord::RECORD_TOGGLE_ONOFF:
      if (obs_frontend_recording_active()) {
	obs_frontend_recording_stop();
      } else {
	obs_frontend_recording_start();
      }
      break;
    case MMGActionRecord::RECORD_PAUSE:
      if (!obs_frontend_recording_paused()) obs_frontend_recording_pause(true);
      break;
    case MMGActionRecord::RECORD_RESUME:
      if (obs_frontend_recording_paused()) obs_frontend_recording_pause(false);
      break;
    case MMGActionRecord::RECORD_TOGGLE_PAUSE:
      obs_frontend_recording_pause(!obs_frontend_recording_paused());
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionRecord::setSubOptions(QComboBox *sub)
{
  QStringList opts = obstr_all("Basic.Main", {"StartRecording", "StopRecording", "PauseRecording",
					      "UnpauseRecording"});
  opts.insert(2, mmgtr("Actions.Recording.Sub.ToggleRecording"));
  opts.append(mmgtr("Actions.Recording.Sub.TogglePauseRecording"));
  sub->addItems(opts);
}
