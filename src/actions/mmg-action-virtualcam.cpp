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

#include "mmg-action-virtualcam.h"

namespace MMGActions {

const MMGParams<bool> MMGActionVirtualCam::vircam_params {
	.desc = mmgtr("Plugin.Status"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionVirtualCam::MMGActionVirtualCam(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  vircam_state(json_obj, "vircam_state")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionVirtualCam::initOldData(const QJsonObject &json_obj)
{
	int sub = json_obj["sub"].toInt();

	MMGCompatibility::initOldBooleanData(vircam_state, sub);
}

void MMGActionVirtualCam::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	vircam_state->json(json_obj, "vircam_state");
}

void MMGActionVirtualCam::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionVirtualCam *>(dest);
	if (!casted) return;

	vircam_state.copy(casted->vircam_state);
}

void MMGActionVirtualCam::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &vircam_state, &vircam_params);
}

void MMGActionVirtualCam::execute(const MMGMappingTest &test) const
{
	bool value = obs_frontend_virtualcam_active();
	ACTION_ASSERT(test.applicable(vircam_state, value),
		      "A status could not be selected. Check the Status Field and try again.");

	if (value && !obs_frontend_virtualcam_active())
		obs_frontend_start_virtualcam();
	else if (!value && obs_frontend_virtualcam_active())
		obs_frontend_stop_virtualcam();

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionVirtualCam::processEvent(obs_frontend_event event) const
{
	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(vircam_state, event, OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED,
				 OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED);
}

} // namespace MMGActions
