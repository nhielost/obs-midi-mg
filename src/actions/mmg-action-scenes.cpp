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

#include "mmg-action-scenes.h"

using namespace MMGUtils;

MMGActionScenes::MMGActionScenes(const QJsonObject &json_obj) : scene(json_obj, "scene", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Scenes> action created.");
}

void MMGActionScenes::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Scenes> Action -> " + message);
}

void MMGActionScenes::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  scene.json(json_obj, "scene");
}

void MMGActionScenes::execute(const MMGMessage *midi) const
{
  const QStringList scenes = MMGActionScenes::enumerate();

  if (MIDI_STRING_IS_NOT_IN_RANGE(scene, scenes.size())) {
    blog(LOG_INFO, "FAILED: MIDI value exceeded scene count.");
    return;
  }
  OBSSourceAutoRelease source_obs_scene = obs_get_source_by_name(
    (scene.state() == MMGString::STRINGSTATE_MIDI ? scenes[(int)midi->value()] : scene).qtocs());

  if (!source_obs_scene) {
    blog(LOG_INFO, "FAILED: Scene does not exist.");
    return;
  }

  switch (sub()) {
    case MMGActionScenes::SCENE_SCENE:
      obs_frontend_set_current_scene(source_obs_scene);
      break;
    default:
      break;
  }

  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionScenes::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionScenes *>(dest);
  if (!casted) return;

  casted->scene = scene.copy();
}

void MMGActionScenes::setEditable(bool edit)
{
  scene.set_edit(edit);
}

const QStringList MMGActionScenes::enumerate()
{
  QStringList list;

  char **scene_names = obs_frontend_get_scene_names();
  for (int i = 0; scene_names[i] != 0; ++i) {
    list.append(scene_names[i]);
  }
  bfree(scene_names);

  return list;
}

const QStringList MMGActionScenes::enumerate_items(const QString &scene)
{
  R r{QStringList(), scene};
  obs_enum_all_sources(
    [](void *param, obs_source_t *source) {
      auto r = reinterpret_cast<R *>(param);

      if (!OBSSceneItemAutoRelease(obs_scene_sceneitem_from_source(
	    OBSSceneAutoRelease(obs_get_scene_by_name(r->str.qtocs())), source)))
	return true;

      r->list.append(obs_source_get_name(source));
      return true;
    },
    &r);
  return r.list;
}

void MMGActionScenes::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&scene);
}

void MMGActionScenes::setSubOptions(QComboBox *sub)
{
  sub->addItem("Scene Switching");
}

void MMGActionScenes::setSubConfig()
{
  _display->setStr1Visible(true);
  _display->setStr1Description("Scene");
  QStringList options = enumerate();
  options.append("Use Message Value");
  _display->setStr1Options(options);
}
