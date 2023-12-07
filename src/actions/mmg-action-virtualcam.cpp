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

#include "mmg-action-virtualcam.h"

using namespace MMGUtils;

MMGActionVirtualCam::MMGActionVirtualCam(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj)
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionVirtualCam::setComboOptions(QComboBox *sub)
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
			opts << obstr_all("Basic.Main", {"StartVirtualCam", "StopVirtualCam"})
			     << subModuleText("Toggle");
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"Started", "Stopped", "Toggle"});
			break;

		default:
			break;
	}

	sub->addItems(opts);
}

void MMGActionVirtualCam::execute(const MMGMessage *) const
{
	switch (sub()) {
		case VIRCAM_ON:
			if (!obs_frontend_virtualcam_active()) obs_frontend_start_virtualcam();
			break;

		case VIRCAM_OFF:
			if (obs_frontend_virtualcam_active()) obs_frontend_stop_virtualcam();
			break;

		case VIRCAM_TOGGLE_ONOFF:
			if (obs_frontend_virtualcam_active()) {
				obs_frontend_stop_virtualcam();
			} else {
				obs_frontend_start_virtualcam();
			}
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionVirtualCam::connectOBSSignals()
{
	disconnectOBSSignals();
	connect(mmgsignals(), &MMGSignals::frontendEvent, this, &MMGActionVirtualCam::frontendCallback);
}

void MMGActionVirtualCam::disconnectOBSSignals()
{
	disconnect(mmgsignals(), &MMGSignals::frontendEvent, this, nullptr);
}

void MMGActionVirtualCam::frontendCallback(obs_frontend_event event) const
{
	switch (sub()) {
		case VIRCAM_STARTED:
			if (event != OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED) return;
			break;

		case VIRCAM_STOPPED:
			if (event != OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED) return;
			break;

		case VIRCAM_TOGGLE_STARTED:
			if (event != OBS_FRONTEND_EVENT_VIRTUALCAM_STARTED &&
			    event != OBS_FRONTEND_EVENT_VIRTUALCAM_STOPPED)
				return;
			break;

		default:
			return;
	}

	emit eventTriggered();
}
