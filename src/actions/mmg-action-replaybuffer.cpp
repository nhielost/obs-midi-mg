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

#include "mmg-action-replaybuffer.h"

#include <util/config-file.h>

namespace MMGActions {

static bool replayBufferEnabled()
{
	config_t *obs_config = obs_frontend_get_profile_config();

	return (MMGString(config_get_string(obs_config, "Output", "Mode")) == "Simple" &&
		config_get_bool(obs_config, "SimpleOutput", "RecRB")) ||
	       (MMGString(config_get_string(obs_config, "Output", "Mode")) == "Advanced" &&
		config_get_bool(obs_config, "AdvOut", "RecRB"));
};

// MMGActionReplayBufferRunState
const MMGParams<bool> MMGActionReplayBufferRunState::repbuf_params {
	.desc = mmgtr("Plugin.Status"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionReplayBufferRunState::MMGActionReplayBufferRunState(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  repbuf_state(json_obj, "repbuf_state")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionReplayBufferRunState::initOldData(const QJsonObject &json_obj)
{
	int sub = json_obj["sub"].toInt();
	if (json_obj["type"].toInt() == TYPE_OUTPUT) sub /= 2;

	MMGCompatibility::initOldBooleanData(repbuf_state, sub);
}

void MMGActionReplayBufferRunState::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	repbuf_state->json(json_obj, "repbuf_state");
}

void MMGActionReplayBufferRunState::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionReplayBufferRunState *>(dest);
	if (!casted) return;

	repbuf_state.copy(casted->repbuf_state);
}

void MMGActionReplayBufferRunState::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &repbuf_state, &repbuf_params);
}

void MMGActionReplayBufferRunState::execute(const MMGMappingTest &test) const
{
	ACTION_ASSERT(replayBufferEnabled(), "Replay buffers are not enabled.");

	bool value = obs_frontend_replay_buffer_active();
	ACTION_ASSERT(test.applicable(repbuf_state, value),
		      "A status could not be selected. Check the Status Field and try again.");

	if (value && !obs_frontend_replay_buffer_active())
		obs_frontend_replay_buffer_start();
	else if (!value && obs_frontend_replay_buffer_active())
		obs_frontend_replay_buffer_stop();

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionReplayBufferRunState::processEvent(obs_frontend_event event) const
{
	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(repbuf_state, event, OBS_FRONTEND_EVENT_REPLAY_BUFFER_STARTED,
				 OBS_FRONTEND_EVENT_REPLAY_BUFFER_STOPPED);
}
// End MMGActionReplayBufferRunState

// MMGActionReplayBufferSave
MMGActionReplayBufferSave::MMGActionReplayBufferSave(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj)
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionReplayBufferSave::execute(const MMGMappingTest &) const
{
	ACTION_ASSERT(replayBufferEnabled(), "Replay buffers are not enabled.");

	if (obs_frontend_replay_buffer_active()) obs_frontend_replay_buffer_save();

	blog(LOG_DEBUG, "Successfully executed.");
}
// End MMGActionReplayBufferSave

} // namespace MMGActions
