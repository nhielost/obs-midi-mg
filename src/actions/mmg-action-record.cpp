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

#include "mmg-action-record.h"

namespace MMGActions {

// MMGActionRecordRunState
const MMGParams<bool> MMGActionRecordRunState::record_params {
	.desc = mmgtr("Plugin.Status"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionRecordRunState::MMGActionRecordRunState(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  record_state(json_obj, "record_state")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionRecordRunState::initOldData(const QJsonObject &json_obj)
{
	int sub = json_obj["sub"].toInt();
	if (json_obj["type"].toInt() == TYPE_OUTPUT) sub /= 2;

	MMGCompatibility::initOldBooleanData(record_state, sub);
}

void MMGActionRecordRunState::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	record_state->json(json_obj, "record_state");
}

void MMGActionRecordRunState::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionRecordRunState *>(dest);
	if (!casted) return;

	record_state.copy(casted->record_state);
}

void MMGActionRecordRunState::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &record_state, &record_params);
}

void MMGActionRecordRunState::execute(const MMGMappingTest &test) const
{
	bool value = obs_frontend_recording_active();
	ACTION_ASSERT(test.applicable(record_state, value),
		      "A status could not be selected. Check the Status Field and try again.");

	if (value && !obs_frontend_recording_active())
		obs_frontend_recording_start();
	else if (!value && obs_frontend_recording_active())
		obs_frontend_recording_stop();

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionRecordRunState::processEvent(obs_frontend_event event) const
{
	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(record_state, event, OBS_FRONTEND_EVENT_RECORDING_STARTED,
				 OBS_FRONTEND_EVENT_RECORDING_STOPPED);
}
// End MMGActionRecordRunState

// MMGActionRecordPauseState
const MMGParams<bool> MMGActionRecordPauseState::pause_params {
	.desc = mmgtr("Plugin.Status"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionRecordPauseState::MMGActionRecordPauseState(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  pause_state(json_obj, "pause_state")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionRecordPauseState::initOldData(const QJsonObject &json_obj)
{
	int sub = json_obj["sub"].toInt() % 3;

	MMGCompatibility::initOldBooleanData(pause_state, sub);
}

void MMGActionRecordPauseState::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	pause_state->json(json_obj, "pause_state");
}

void MMGActionRecordPauseState::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionRecordPauseState *>(dest);
	if (!casted) return;

	pause_state.copy(casted->pause_state);
}

void MMGActionRecordPauseState::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &pause_state, &pause_params);
}

void MMGActionRecordPauseState::execute(const MMGMappingTest &test) const
{
	bool value = obs_frontend_recording_paused();
	ACTION_ASSERT(test.applicable(pause_state, value),
		      "A status could not be selected. Check the Status Field and try again.");

	if (value && !obs_frontend_recording_paused())
		obs_frontend_recording_pause(true);
	else if (!value && obs_frontend_recording_paused())
		obs_frontend_recording_pause(false);

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionRecordPauseState::processEvent(obs_frontend_event event) const
{
	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(pause_state, event, OBS_FRONTEND_EVENT_RECORDING_PAUSED,
				 OBS_FRONTEND_EVENT_RECORDING_UNPAUSED);
}
// End MMGActionRecordPauseState

} // namespace MMGActions
