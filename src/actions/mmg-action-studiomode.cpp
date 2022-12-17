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

#include "mmg-action-studiomode.h"
#include "mmg-action-scenes.h"

using namespace MMGUtils;

MMGActionStudioMode::MMGActionStudioMode(const QJsonObject &json_obj) : scene(json_obj, "scene", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Studio Mode> action created.");
}

void MMGActionStudioMode::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Studio Mode> Action -> " + message);
}

void MMGActionStudioMode::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  scene.json(json_obj, "scene", true);
}

void MMGActionStudioMode::do_action(const MMGMessage *midi)
{
  const QStringList scenes = MMGActionScenes::enumerate();

  if (MIDI_STRING_IS_NOT_IN_RANGE(scene, scenes.size())) {
    blog(LOG_INFO, "FAILED: MIDI value exceeded scene count.");
    return;
  }

  // For new Studio Mode activation (pre 28.0.0 method encounters threading errors)
  auto set_studio_mode = [](bool on) {
    if (obs_frontend_preview_program_mode_active() == on) return;
    obs_queue_task(
      OBS_TASK_UI,
      [](void *param) {
	auto enabled = (bool *)param;
	obs_frontend_set_preview_program_mode(*enabled);
      },
      &on, true);
  };

  OBSSourceAutoRelease source_obs_scene = obs_get_source_by_name(
    (scene.state() == MMGString::STRINGSTATE_MIDI ? scenes[(int)midi->value()] : scene).qtocs());
  OBSSourceAutoRelease obs_preview_scene = obs_frontend_get_current_preview_scene();

  switch (get_sub()) {
    case MMGActionStudioMode::STUDIOMODE_ON:
      set_studio_mode(true);
      break;
    case MMGActionStudioMode::STUDIOMODE_OFF:
      set_studio_mode(false);
      break;
    case MMGActionStudioMode::STUDIOMODE_TOGGLE_ONOFF:
      set_studio_mode(!obs_frontend_preview_program_mode_active());
      break;
    case MMGActionStudioMode::STUDIOMODE_CHANGEPREVIEW:
      if (!source_obs_scene) {
	blog(LOG_INFO, "FAILED: Scene does not exist.");
	return;
      }
      obs_frontend_set_current_preview_scene(source_obs_scene);
      break;
    case MMGActionStudioMode::STUDIOMODE_TRANSITION:
      obs_frontend_set_current_scene(obs_preview_scene);
      break;
    default:
      break;
  }

  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionStudioMode::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  scene.copy(&dest->str1());
}

void MMGActionStudioMode::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Turn On Studio Mode", "Turn Off Studio Mode", "Toggle Studio Mode",
	      "Change Preview Scene", "Preview to Program"};
}
void MMGActionStudioMode::change_options_str1(MMGActionDisplayParams &val)
{
  if (subcategory == 3) {
    val.display = MMGActionDisplayParams::DISPLAY_STR1;
    val.label_text = "Preview Scene";
    val.list = MMGActionScenes::enumerate();
    val.list.append("Use Message Value");
  }
}
void MMGActionStudioMode::change_options_str2(MMGActionDisplayParams &val)
{
  if (subcategory == 3) {
    scene.set_state(scene == "Use Message Value" ? MMGString::STRINGSTATE_MIDI
						 : MMGString::STRINGSTATE_FIXED);
  }
}
void MMGActionStudioMode::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionStudioMode::change_options_final(MMGActionDisplayParams &val) {}
