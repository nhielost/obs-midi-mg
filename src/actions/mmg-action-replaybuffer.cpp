/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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

MMGActionReplayBuffer::MMGActionReplayBuffer(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj)
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionReplayBuffer::subNames() const
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
		default:
			opts << obstr_all("Basic.Main", {"StartReplayBuffer", "StopReplayBuffer"})
			     << subModuleText("Toggle") << subModuleText("Save");
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"Starting", "Started", "Stopping", "Stopped", "ToggleStarting",
						   "ToggleStarted", "Save"});
			break;
	}

	return opts;
}

void MMGActionReplayBuffer::execute(const MMGMessage *) const
{
	config_t *obs_config = obs_frontend_get_profile_config();
	ACTION_ASSERT((QString(config_get_string(obs_config, "Output", "Mode")) == "Simple" &&
		       config_get_bool(obs_config, "SimpleOutput", "RecRB")) ||
			      (QString(config_get_string(obs_config, "Output", "Mode")) == "Advanced" &&
			       config_get_bool(obs_config, "AdvOut", "RecRB")),
		      "Replay buffers are not enabled.");

	switch (sub()) {
		case REPBUF_ON:
			if (!obs_frontend_replay_buffer_active()) obs_frontend_replay_buffer_start();
			break;

		case REPBUF_OFF:
			if (obs_frontend_replay_buffer_active()) obs_frontend_replay_buffer_stop();
			break;

		case REPBUF_TOGGLE_ONOFF:
			if (obs_frontend_replay_buffer_active()) {
				obs_frontend_replay_buffer_stop();
			} else {
				obs_frontend_replay_buffer_start();
			}
			break;

		case REPBUF_SAVE:
			if (obs_frontend_replay_buffer_active()) obs_frontend_replay_buffer_save();
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionReplayBuffer::frontendEventReceived(obs_frontend_event event)
{
	switch (sub()) {
		case REPBUF_STARTING:
			if (event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING) return;
			break;

		case REPBUF_STARTED:
			if (event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED) return;
			break;

		case REPBUF_STOPPING:
			if (event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING) return;
			break;

		case REPBUF_STOPPED:
			if (event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED) return;
			break;

		case REPBUF_TOGGLE_STARTING:
			if (event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTING &&
			    event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPING)
				return;
			break;

		case REPBUF_TOGGLE_STARTED:
			if (event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED &&
			    event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED)
				return;
			break;

		case REPBUF_SAVED:
			if (event != OBS_FRONTEND_EVENT_REPLAY_BUFFER_SAVED) return;
			break;

		default:
			return;
	}

	triggerEvent();
}
