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

#include "mmg-action-transitions.h"
#include "mmg-action-scenes.h"

using namespace MMGUtils;

MMGActionTransitions::MMGActionTransitions(const QJsonObject &json_obj)
  : transition(json_obj, "transition", 1),
    parent_scene(json_obj, "scene", 2),
    source(json_obj, "source", 3),
    num(json_obj, "num", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Transitions> action created.");
}

void MMGActionTransitions::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Transitions> Action -> " + message);
}

void MMGActionTransitions::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  transition.json(json_obj, "transition", false);
  parent_scene.json(json_obj, "scene", false);
  source.json(json_obj, "source", false);
  num.json(json_obj, "num", true);
}

void MMGActionTransitions::do_action(const MMGMessage *midi)
{
  OBSSourceAutoRelease obs_transition = get_obs_transition_by_name(transition);
  if (!obs_transition && get_sub() != 3) {
    blog(LOG_INFO, "FAILED: Transition does not exist.");
    return;
  }

  int obs_transition_time = num.choose(midi, obs_frontend_get_transition_duration(), 3200);
  bool fixed = obs_transition_fixed(obs_transition);

  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  OBSSceneAutoRelease obs_scene = obs_get_scene_by_name(parent_scene.mmgtocs());
  OBSSceneItemAutoRelease obs_sceneitem = obs_scene_sceneitem_from_source(obs_scene, obs_source);

  switch (get_sub()) {
    case MMGActionTransitions::TRANSITION_CURRENT:
      obs_frontend_set_current_transition(obs_transition);
      if (!fixed) obs_frontend_set_transition_duration(obs_transition_time);
      break;
    case MMGActionTransitions::TRANSITION_SOURCE_SHOW:
      if (!obs_sceneitem) {
	blog(LOG_INFO, "FAILED: Source in scene does not exist.");
	return;
      }
      obs_sceneitem_set_transition(obs_sceneitem, true, obs_transition);
      if (!fixed) obs_sceneitem_set_transition_duration(obs_sceneitem, true, obs_transition_time);
      break;
    case MMGActionTransitions::TRANSITION_SOURCE_HIDE:
      if (!obs_sceneitem) {
	blog(LOG_INFO, "FAILED: Source in scene does not exist.");
	return;
      }
      obs_sceneitem_set_transition(obs_sceneitem, false, obs_transition);
      if (!fixed) obs_sceneitem_set_transition_duration(obs_sceneitem, false, obs_transition_time);
      break;
    case MMGActionTransitions::TRANSITION_TBAR:
      if (!obs_frontend_preview_program_mode_active()) {
	blog(LOG_INFO, "FAILED: Studio mode is inactive.");
	return;
      }
      obs_frontend_set_tbar_position((int)num.choose(midi, 0, 1024.0));
      obs_frontend_release_tbar();
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionTransitions::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  transition.copy(&dest->str1());
  parent_scene.copy(&dest->str2());
  source.copy(&dest->str3());
  num.copy(&dest->num1());
}

const QStringList MMGActionTransitions::enumerate()
{
  QStringList list;
  obs_frontend_source_list transition_list = {0};
  obs_frontend_get_transitions(&transition_list);
  for (size_t i = 0; i < transition_list.sources.num; ++i) {
    list.append(obs_source_get_name(transition_list.sources.array[i]));
  }
  obs_frontend_source_list_free(&transition_list);
  return list;
}

void MMGActionTransitions::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Change Current Transition", "Set Source Show Transition",
	      "Set Source Hide Transition", "Set Transition Bar Position (Studio Mode)"};
}
void MMGActionTransitions::change_options_str1(MMGActionDisplayParams &val)
{
  if (subcategory == 3) {
    val.display = MMGActionDisplayParams::DISPLAY_NUM1;

    val.label_lcds[0] = "Position";
    val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI |
			   MMGActionDisplayParams::COMBODISPLAY_MIDI_INVERT;
    val.lcds[0]->set_range(0.0, 1024.0);
    val.lcds[0]->set_step(1.0, 10.0);
    val.lcds[0]->set_default_value(0.0);
  } else {
    val.display = MMGActionDisplayParams::DISPLAY_STR1;
    val.label_text = "Transition";
    val.list = enumerate();
  }
}
void MMGActionTransitions::change_options_str2(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case TRANSITION_CURRENT:
      val.display &= ~MMGActionDisplayParams::DISPLAY_NUM1;
      val.lcds[0]->set_default_value(obs_frontend_get_transition_duration());
      if (get_obs_transition_fixed_length(transition)) break;

      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Duration";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI |
			     MMGActionDisplayParams::COMBODISPLAY_IGNORE;
      val.lcds[0]->set_range(25.0, 20000.0);
      val.lcds[0]->set_step(25.0, 250.0);

      break;
    case TRANSITION_SOURCE_SHOW:
    case TRANSITION_SOURCE_HIDE:
      val.display = MMGActionDisplayParams::DISPLAY_STR2;
      val.label_text = "Scene";
      val.list = MMGActionScenes::enumerate();
      break;
    default:
      break;
  }
}
void MMGActionTransitions::change_options_str3(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case TRANSITION_SOURCE_SHOW:
    case TRANSITION_SOURCE_HIDE:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;
      val.label_text = "Source";
      val.list = MMGActionScenes::enumerate_items(parent_scene);
      break;
    default:
      break;
  }
}
void MMGActionTransitions::change_options_final(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case TRANSITION_SOURCE_SHOW:
    case TRANSITION_SOURCE_HIDE:
      val.lcds[0]->set_default_value(0.0);
      if (get_obs_transition_fixed_length(transition)) break;

      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Duration";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI |
			     MMGActionDisplayParams::COMBODISPLAY_IGNORE;
      val.lcds[0]->set_range(25.0, 20000.0);
      val.lcds[0]->set_step(25.0, 250.0);

      break;
    default:
      break;
  }
}
