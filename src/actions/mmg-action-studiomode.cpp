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
  MMGAction::json(json_obj);

  scene.json(json_obj, "scene");
}

void MMGActionStudioMode::execute(const MMGMessage *midi) const
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

  switch (sub()) {
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
}

void MMGActionStudioMode::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionStudioMode *>(dest);
  if (!casted) return;

  casted->scene = scene.copy();
}

void MMGActionStudioMode::setEditable(bool edit)
{
  scene.set_edit(edit);
}

void MMGActionStudioMode::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&scene);
}

void MMGActionStudioMode::setSubOptions(QComboBox *sub)
{
  sub->addItems({"Turn On Studio Mode", "Turn Off Studio Mode", "Toggle Studio Mode",
		 "Change Preview Scene", "Preview to Program"});
}

void MMGActionStudioMode::setSubConfig()
{
  _display->setStr1Visible(false);
  if (subcategory == 3) {
    _display->setStr1Visible(true);
    _display->setStr1Description("Scene");
    QStringList options = MMGActionScenes::enumerate();
    options.append("Use Message Value");
    _display->setStr1Options(options);
  }
}
