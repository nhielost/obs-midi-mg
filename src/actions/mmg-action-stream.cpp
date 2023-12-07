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

MMGActionStream::MMGActionStream(MMGActionManager *parent, const QJsonObject &json_obj) : MMGAction(parent, json_obj)
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionStream::setComboOptions(QComboBox *sub)
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
			opts << obstr_all("Basic.Main", {"StartStreaming", "StopStreaming"}) << subModuleText("Toggle");
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList(
				{"Starting", "Started", "Stopping", "Stopped", "ToggleStarting", "ToggleStarted"});
			break;

		default:
			break;
	}

	sub->addItems(opts);
}

void MMGActionStream::execute(const MMGMessage *) const
{
	switch (sub()) {
		case STREAM_ON:
			if (!obs_frontend_streaming_active()) obs_frontend_streaming_start();
			break;

		case STREAM_OFF:
			if (obs_frontend_streaming_active()) obs_frontend_streaming_stop();
			break;

		case STREAM_TOGGLE_ONOFF:
			if (obs_frontend_streaming_active()) {
				obs_frontend_streaming_stop();
			} else {
				obs_frontend_streaming_start();
			}
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionStream::connectOBSSignals()
{
	disconnectOBSSignals();
	connect(mmgsignals(), &MMGSignals::frontendEvent, this, &MMGActionStream::frontendCallback);
}

void MMGActionStream::disconnectOBSSignals()
{
	disconnect(mmgsignals(), &MMGSignals::frontendEvent, this, nullptr);
}

void MMGActionStream::frontendCallback(obs_frontend_event event) const
{
	switch (sub()) {
		case STREAM_STARTING:
			if (event != OBS_FRONTEND_EVENT_STREAMING_STARTING) return;
			break;

		case STREAM_STARTED:
			if (event != OBS_FRONTEND_EVENT_STREAMING_STARTED) return;
			break;
		case STREAM_STOPPING:
			if (event != OBS_FRONTEND_EVENT_STREAMING_STOPPING) return;
			break;

		case STREAM_STOPPED:
			if (event != OBS_FRONTEND_EVENT_STREAMING_STOPPED) return;
			break;

		case STREAM_TOGGLE_STARTING:
			if (event != OBS_FRONTEND_EVENT_STREAMING_STARTING &&
			    event != OBS_FRONTEND_EVENT_STREAMING_STOPPING)
				return;
			break;

		case STREAM_TOGGLE_STARTED:
			if (event != OBS_FRONTEND_EVENT_STREAMING_STARTED &&
			    event != OBS_FRONTEND_EVENT_STREAMING_STOPPED)
				return;
			break;

		default:
			return;
	}

	emit eventTriggered();
}
