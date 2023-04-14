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

#include "mmg-action-profiles.h"

using namespace MMGUtils;

MMGActionProfiles::MMGActionProfiles(const QJsonObject &json_obj) : profile(json_obj, "profile", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionProfiles::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Profiles] " + message);
}

void MMGActionProfiles::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  profile.json(json_obj, "profile");
}

void MMGActionProfiles::execute(const MMGMessage *midi) const
{
  const QStringList profiles = MMGActionProfiles::enumerate();

  if (MIDI_STRING_IS_NOT_IN_RANGE(profile, profiles.size())) {
    blog(LOG_INFO, "FAILED: MIDI value exceeds profile count.");
    return;
  }

  // For new Profile change (pre 28.0.0 method encounters threading errors)
  auto set_profile = [](const char *name) {
    char *current_profile = obs_frontend_get_current_profile();
    if (name == current_profile) {
      bfree(current_profile);
      return;
    }
    bfree(current_profile);
    obs_queue_task(
      OBS_TASK_UI,
      [](void *param) {
	auto profile_name = (char **)param;
	obs_frontend_set_current_profile(*profile_name);
      },
      &name, true);
  };

  if (sub() == 0) {
    if (!(obs_frontend_streaming_active() || obs_frontend_recording_active() ||
	  obs_frontend_virtualcam_active())) {
      set_profile(
	(profile.state() == MMGString::STRINGSTATE_MIDI ? profiles[(int)midi->value()] : profile)
	  .qtocs());
    }
  }

  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionProfiles::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionProfiles *>(dest);
  if (!casted) return;

  casted->profile = profile.copy();
}

void MMGActionProfiles::setEditable(bool edit)
{
  profile.set_edit(edit);
}

const QStringList MMGActionProfiles::enumerate()
{
  QStringList list;

  char **profile_names = obs_frontend_get_profiles();
  for (int i = 0; profile_names[i] != 0; ++i) {
    list.append(profile_names[i]);
  }
  bfree(profile_names);

  return list;
}

void MMGActionProfiles::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&profile);
}

void MMGActionProfiles::setSubOptions(QComboBox *sub)
{
  sub->addItem(mmgtr("Actions.Profiles.Sub.Switch"));
}

void MMGActionProfiles::setSubConfig()
{
  _display->setStr1Visible(true);
  _display->setStr1Description(obstr("TitleBar.Profile"));
  QStringList options = enumerate();
  options.append(mmgtr("Fields.UseMessageValue"));
  _display->setStr1Options(options);
}
