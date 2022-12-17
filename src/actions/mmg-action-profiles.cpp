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

#include "mmg-action-profiles.h"

using namespace MMGUtils;

MMGActionProfiles::MMGActionProfiles(const QJsonObject &json_obj) : profile(json_obj, "profile", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Profiles> action created.");
}

void MMGActionProfiles::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Profiles> Action -> " + message);
}

void MMGActionProfiles::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  profile.json(json_obj, "profile", true);
}

void MMGActionProfiles::do_action(const MMGMessage *midi)
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

  if (get_sub() == 0) {
    if (!(obs_frontend_streaming_active() || obs_frontend_recording_active() ||
	  obs_frontend_virtualcam_active())) {
      set_profile(
	(profile.state() == MMGString::STRINGSTATE_MIDI ? profiles[(int)midi->value()] : profile)
	  .qtocs());
    }
  }

  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionProfiles::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  profile.copy(&dest->str1());
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

void MMGActionProfiles::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Switch Profiles"};
}
void MMGActionProfiles::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Profile";
  val.list = enumerate();
  val.list.append("Use Message Value");
}
void MMGActionProfiles::change_options_str2(MMGActionDisplayParams &val)
{
  profile.set_state(profile == "Use Message Value" ? MMGString::STRINGSTATE_MIDI
						   : MMGString::STRINGSTATE_FIXED);
}
void MMGActionProfiles::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionProfiles::change_options_final(MMGActionDisplayParams &val) {}
