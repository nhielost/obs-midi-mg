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

MMGActionRecord::MMGActionRecord(MMGActionManager *parent, const QJsonObject &json_obj) : MMGAction(parent, json_obj)
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionRecord::subNames() const
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
		default:
			opts << obstr_all("Basic.Main", {"StartRecording", "StopRecording"}) << subModuleText("Toggle")
			     << obstr_all("Basic.Main", {"PauseRecording", "UnpauseRecording"})
			     << subModuleText("TogglePause");
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"Starting", "Started", "Stopping", "Stopped", "ToggleStarting",
						   "ToggleStarted", "Paused", "Resumed", "TogglePaused"});
			break;
	}

	return opts;
}

void MMGActionRecord::execute(const MMGMessage *) const
{
	switch (sub()) {
		case RECORD_ON:
			if (!obs_frontend_recording_active()) obs_frontend_recording_start();
			break;

		case RECORD_OFF:
			if (obs_frontend_recording_active()) obs_frontend_recording_stop();
			break;

		case RECORD_TOGGLE_ONOFF:
			if (obs_frontend_recording_active()) {
				obs_frontend_recording_stop();
			} else {
				obs_frontend_recording_start();
			}
			break;

		case RECORD_PAUSE:
			if (!obs_frontend_recording_paused()) obs_frontend_recording_pause(true);
			break;

		case RECORD_RESUME:
			if (obs_frontend_recording_paused()) obs_frontend_recording_pause(false);
			break;

		case RECORD_TOGGLE_PAUSE:
			obs_frontend_recording_pause(!obs_frontend_recording_paused());
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionRecord::frontendEventReceived(obs_frontend_event event)
{
	switch (sub()) {
		case RECORD_STARTING:
			if (event != OBS_FRONTEND_EVENT_RECORDING_STARTING) return;
			break;

		case RECORD_STARTED:
			if (event != OBS_FRONTEND_EVENT_RECORDING_STARTED) return;
			break;

		case RECORD_STOPPING:
			if (event != OBS_FRONTEND_EVENT_RECORDING_STOPPING) return;
			break;

		case RECORD_STOPPED:
			if (event != OBS_FRONTEND_EVENT_RECORDING_STOPPED) return;
			break;

		case RECORD_TOGGLE_STARTING:
			if (event != OBS_FRONTEND_EVENT_RECORDING_STARTING &&
			    event != OBS_FRONTEND_EVENT_RECORDING_STOPPING)
				return;
			break;

		case RECORD_TOGGLE_STARTED:
			if (event != OBS_FRONTEND_EVENT_RECORDING_STARTED &&
			    event != OBS_FRONTEND_EVENT_RECORDING_STOPPED)
				return;
			break;

		case RECORD_PAUSED:
			if (event != OBS_FRONTEND_EVENT_RECORDING_PAUSED) return;
			break;
		case RECORD_RESUMED:
			if (event != OBS_FRONTEND_EVENT_RECORDING_UNPAUSED) return;
			break;

		case RECORD_TOGGLE_PAUSED:
			if (event != OBS_FRONTEND_EVENT_RECORDING_PAUSED &&
			    event != OBS_FRONTEND_EVENT_RECORDING_UNPAUSED)
				return;
			break;

		default:
			return;
	}

	triggerEvent();
}
