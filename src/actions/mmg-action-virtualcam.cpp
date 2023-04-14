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

MMGActionVirtualCam::MMGActionVirtualCam(const QJsonObject &json_obj)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionVirtualCam::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Virtual Camera] " + message);
}

void MMGActionVirtualCam::execute(const MMGMessage *midi) const
{
  Q_UNUSED(midi);
  switch (sub()) {
    case MMGActionVirtualCam::VIRCAM_ON:
      if (!obs_frontend_virtualcam_active()) obs_frontend_start_virtualcam();
      break;
    case MMGActionVirtualCam::VIRCAM_OFF:
      if (obs_frontend_virtualcam_active()) obs_frontend_stop_virtualcam();
      break;
    case MMGActionVirtualCam::VIRCAM_TOGGLE_ONOFF:
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

void MMGActionVirtualCam::setSubOptions(QComboBox *sub)
{
  QStringList opts = obstr_all("Basic.Main", {"StartVirtualCam", "StopVirtualCam"});
  opts.append(mmgtr("Actions.VirtualCamera.Sub.ToggleVirtualCamera"));
  sub->addItems(opts);
}
