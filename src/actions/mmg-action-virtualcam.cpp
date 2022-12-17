/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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

  blog(LOG_DEBUG, "<Virtual Camera> action created.");
}

void MMGActionVirtualCam::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Virtual Camera> Action -> " + message);
}

void MMGActionVirtualCam::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
}

void MMGActionVirtualCam::do_action(const MMGMessage *midi)
{
  Q_UNUSED(midi);
  switch (get_sub()) {
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
  executed = true;
}

void MMGActionVirtualCam::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
}

void MMGActionVirtualCam::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Start Virtual Camera", "Stop Virtual Camera", "Toggle Virtual Camera"};
}
void MMGActionVirtualCam::change_options_str1(MMGActionDisplayParams &val) {}
void MMGActionVirtualCam::change_options_str2(MMGActionDisplayParams &val) {}
void MMGActionVirtualCam::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionVirtualCam::change_options_final(MMGActionDisplayParams &val) {}
