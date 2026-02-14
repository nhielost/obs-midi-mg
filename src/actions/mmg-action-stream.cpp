/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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

namespace MMGActions {

const MMGParams<bool> MMGActionStream::stream_params {
	.desc = mmgtr("Plugin.Status"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionStream::MMGActionStream(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  stream_state(json_obj, "stream_state")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionStream::initOldData(const QJsonObject &json_obj)
{
	int sub = json_obj["sub"].toInt();
	if (json_obj["type"].toInt() == TYPE_OUTPUT) sub /= 2;

	MMGCompatibility::initOldBooleanData(stream_state, sub);
}

void MMGActionStream::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	stream_state->json(json_obj, "stream_state");
}

void MMGActionStream::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionStream *>(dest);
	if (!casted) return;

	stream_state.copy(casted->stream_state);
}

void MMGActionStream::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &stream_state, &stream_params);
}

void MMGActionStream::execute(const MMGMappingTest &test) const
{
	bool value = obs_frontend_streaming_active();
	ACTION_ASSERT(test.applicable(stream_state, value),
		      "A status could not be selected. Check the Status Field and try again.");

	if (value && !obs_frontend_streaming_active())
		obs_frontend_streaming_start();
	else if (!value && obs_frontend_streaming_active())
		obs_frontend_streaming_stop();

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionStream::processEvent(obs_frontend_event event) const
{
	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(stream_state, event, OBS_FRONTEND_EVENT_STREAMING_STARTED,
				 OBS_FRONTEND_EVENT_STREAMING_STOPPED);
}

} // namespace MMGActions
