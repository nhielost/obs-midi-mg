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

#include "mmg-action-replaybuffer.h"
#include <util/config-file.h>

using namespace MMGUtils;

MMGActionReplayBuffer::MMGActionReplayBuffer(const QJsonObject &json_obj)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionReplayBuffer::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Replay Buffer] " + message);
}

void MMGActionReplayBuffer::execute(const MMGMessage *midi) const
{
  config_t *obs_config = obs_frontend_get_profile_config();
  if ((QString(config_get_string(obs_config, "Output", "Mode")) == "Simple" &&
       !config_get_bool(obs_config, "SimpleOutput", "RecRB")) ||
      (QString(config_get_string(obs_config, "Output", "Mode")) == "Advanced" &&
       !config_get_bool(obs_config, "AdvOut", "RecRB"))) {
    blog(LOG_INFO, "FAILED: Replay buffers are not enabled.");
    return;
  }

  Q_UNUSED(midi);
  switch (sub()) {
    case MMGActionReplayBuffer::REPBUF_ON:
      if (!obs_frontend_replay_buffer_active()) obs_frontend_replay_buffer_start();
      break;
    case MMGActionReplayBuffer::REPBUF_OFF:
      if (obs_frontend_replay_buffer_active()) obs_frontend_replay_buffer_stop();
      break;
    case MMGActionReplayBuffer::REPBUF_TOGGLE_ONOFF:
      if (obs_frontend_replay_buffer_active()) {
	obs_frontend_replay_buffer_stop();
      } else {
	obs_frontend_replay_buffer_start();
      }
      break;
    case MMGActionReplayBuffer::REPBUF_SAVE:
      if (obs_frontend_replay_buffer_active()) obs_frontend_replay_buffer_save();
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionReplayBuffer::setSubOptions(QComboBox *sub)
{
  QStringList opts = obstr_all("Basic.Main", {"StartReplayBuffer", "StopReplayBuffer"});
  opts.append(mmgtr("Actions.ReplayBuffer.Sub.ToggleReplayBuffer"));
  opts.append(mmgtr("Actions.ReplayBuffer.Sub.SaveReplayBuffer"));
  sub->addItems(opts);
}
