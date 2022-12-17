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

#include "mmg-action-video-sources.h"
#include "mmg-action-scenes.h"

using namespace MMGUtils;

MMGActionVideoSources::MMGActionVideoSources(const QJsonObject &json_obj)
  : parent_scene(json_obj, "scene", 1),
    source(json_obj, "source", 2),
    action(json_obj, "action", 3),
    nums{{json_obj, "num1", 1}, {json_obj, "num2", 2}, {json_obj, "num3", 3}, {json_obj, "num4", 4}}
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Video Sources> action created.");
}

void MMGActionVideoSources::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Video Sources> Action -> " + message);
}

void MMGActionVideoSources::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  parent_scene.json(json_obj, "scene", false);
  source.json(json_obj, "source", false);
  action.json(json_obj, "action", true);
  for (int i = 0; i < 4; ++i) {
    nums[i].json(json_obj, num_to_str(i + 1, "num"), true);
  }
}

void MMGActionVideoSources::do_action(const MMGMessage *midi)
{
  OBSSourceAutoRelease obs_scene_source = obs_get_source_by_name(parent_scene.mmgtocs());
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  if (!obs_source || !obs_scene_source) {
    blog(LOG_INFO, "FAILED: Scene or source does not exist.");
    return;
  }
  OBSSceneItemAutoRelease obs_sceneitem =
    obs_scene_sceneitem_from_source(obs_scene_from_source(obs_scene_source), obs_source);
  if (!obs_sceneitem) {
    blog(LOG_INFO, "FAILED: Source does not exist in scene.");
    return;
  }
  vec2 set_vec2;
  vec2 get_vec2;
  obs_sceneitem_crop crop;
  uint32_t align = 0;

  switch (get_sub()) {
    case MMGActionVideoSources::SOURCE_VIDEO_POSITION:
      obs_sceneitem_get_pos(obs_sceneitem, &get_vec2);
      vec2_set(&set_vec2, num1().choose(midi, get_vec2.x / num3(), get_obs_dimensions().x) * num3(),
	       num2().choose(midi, get_vec2.y / num3(), get_obs_dimensions().y) * num3());
      obs_sceneitem_set_pos(obs_sceneitem, &set_vec2);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_DISPLAY:
      if (action.state() == MMGString::STRINGSTATE_TOGGLE) {
	obs_sceneitem_set_visible(obs_sceneitem, !obs_sceneitem_visible(obs_sceneitem));
      } else {
	obs_sceneitem_set_visible(obs_sceneitem, bool_from_str(action));
      }
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_LOCKED:
      if (action.state() == MMGString::STRINGSTATE_TOGGLE) {
	obs_sceneitem_set_locked(obs_sceneitem, !obs_sceneitem_locked(obs_sceneitem));
      } else {
	obs_sceneitem_set_locked(obs_sceneitem, bool_from_str(action));
      }
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_CROP:
      obs_sceneitem_get_crop(obs_sceneitem, &crop);
      set_vec2 = get_obs_source_dimensions(source);
      crop.top = num1().choose(midi, crop.top, set_vec2.y);
      crop.right = num2().choose(midi, crop.right, set_vec2.x);
      crop.bottom = num3().choose(midi, crop.bottom, set_vec2.y);
      crop.left = num4().choose(midi, crop.left, set_vec2.x);
      obs_sceneitem_set_crop(obs_sceneitem, &crop);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_ALIGNMENT:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 9)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      if (num1().choose(midi) <= 2) align |= OBS_ALIGN_TOP;
      if (num1().choose(midi) >= 6) align |= OBS_ALIGN_BOTTOM;
      if ((uint)num1().choose(midi) % 3 == 0) align |= OBS_ALIGN_LEFT;
      if ((uint)num1().choose(midi) % 3 == 2) align |= OBS_ALIGN_RIGHT;
      obs_sceneitem_set_alignment(obs_sceneitem, align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_SCALE:
      // Multiplier
      align = 100 / num3();
      obs_sceneitem_get_scale(obs_sceneitem, &get_vec2);
      vec2_set(&set_vec2, num1().choose(midi, get_vec2.x * align, 100, true) / align,
	       num2().choose(midi, get_vec2.y * align, 100, true) / align);
      obs_sceneitem_set_scale(obs_sceneitem, &set_vec2);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_SCALEFILTER:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 6)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      align = num1().choose(midi);
      obs_sceneitem_set_scale_filter(obs_sceneitem, (obs_scale_type)align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_ROTATION:
      obs_sceneitem_set_rot(obs_sceneitem, num1().choose(midi, 0, 360.0));
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BOUNDING_BOX_TYPE:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 7)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      obs_sceneitem_set_bounds_type(obs_sceneitem, (obs_bounds_type)num1().choose(midi));
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BOUNDING_BOX_SIZE:
      obs_sceneitem_get_bounds(obs_sceneitem, &get_vec2);
      vec2_set(&set_vec2, num1().choose(midi, get_vec2.x, get_obs_dimensions().x),
	       num2().choose(midi, get_vec2.y, get_obs_dimensions().y));
      obs_sceneitem_set_bounds(obs_sceneitem, &set_vec2);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 9)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      if (num1().choose(midi) <= 2) align |= OBS_ALIGN_TOP;
      if (num1().choose(midi) >= 6) align |= OBS_ALIGN_BOTTOM;
      if ((uint)num1().choose(midi) % 3 == 0) align |= OBS_ALIGN_LEFT;
      if ((uint)num1().choose(midi) % 3 == 2) align |= OBS_ALIGN_RIGHT;
      obs_sceneitem_set_bounds_alignment(obs_sceneitem, align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BLEND_MODE:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 7)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      obs_sceneitem_set_blending_mode(obs_sceneitem, (obs_blending_type)num1().choose(midi));
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_SCREENSHOT:
      obs_frontend_take_source_screenshot(obs_source);
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionVideoSources::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  parent_scene.copy(&dest->str1());
  source.copy(&dest->str2());
  action.copy(&dest->str3());
  num1().copy(&dest->num1());
  num2().copy(&dest->num2());
  num3().copy(&dest->num3());
  num4().copy(&dest->num4());
}

const QStringList MMGActionVideoSources::enumerate(const QString &restriction)
{
  QStringList list;
  obs_enum_all_sources(
    [](void *param, obs_source_t *source) {
      auto _list = reinterpret_cast<QStringList *>(param);

      if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT) return true;
      if (!(obs_source_get_output_flags(source) & OBS_SOURCE_VIDEO)) return true;

      _list->append(obs_source_get_name(source));
      return true;
    },
    &list);
  return list;
}

void MMGActionVideoSources::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Move Source",
	      "Display Source",
	      "Source Locking",
	      "Source Crop",
	      "Align Source",
	      "Source Scale",
	      "Source Scale Filtering",
	      "Rotate Source",
	      "Source Bounding Box Type",
	      "Resize Source Bounding Box",
	      "Align Source Bounding Box",
	      "Source Blending Mode",
	      "Take Source Screenshot"};
}
void MMGActionVideoSources::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Scene";
  val.list = MMGActionScenes::enumerate();
}
void MMGActionVideoSources::change_options_str2(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR2;
  val.label_text = "Source";
  val.list = MMGActionScenes::enumerate_items(parent_scene);
}
void MMGActionVideoSources::change_options_str3(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case SOURCE_VIDEO_POSITION:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM3;

      val.label_lcds[0] = "Position X";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[0]->set_range(0.0, get_obs_dimensions().x);
      val.lcds[0]->set_step(0.5, 5.0);
      val.lcds[0]->set_default_value(0.0);

      val.label_lcds[1] = "Position Y";
      val.combo_display[1] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[1]->set_range(0.0, get_obs_dimensions().y);
      val.lcds[1]->set_step(0.5, 5.0);
      val.lcds[1]->set_default_value(0.0);

      val.label_lcds[2] = "Magnitude";
      val.combo_display[2] = MMGActionDisplayParams::COMBODISPLAY_FIXED;
      val.lcds[2]->set_range(0.5, 100.0);
      val.lcds[2]->set_step(0.5, 5.0);
      val.lcds[2]->set_default_value(1.0);

      break;
    case SOURCE_VIDEO_DISPLAY:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;

      val.label_text = "State";
      val.list = {"Show", "Hide", "Toggle"};

      break;
    case SOURCE_VIDEO_LOCKED:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;

      val.label_text = "State";
      val.list = {"Locked", "Unlocked", "Toggle"};

      break;
    case SOURCE_VIDEO_CROP:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM4;

      val.label_lcds[0] = "Top";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[0]->set_range(0.0, get_obs_source_dimensions(source).y);
      val.lcds[0]->set_step(0.5, 5.0);
      val.lcds[0]->set_default_value(0.0);

      val.label_lcds[1] = "Right";
      val.combo_display[1] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[1]->set_range(0.0, get_obs_source_dimensions(source).x);
      val.lcds[1]->set_step(0.5, 5.0);
      val.lcds[1]->set_default_value(0.0);

      val.label_lcds[2] = "Bottom";
      val.combo_display[2] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[2]->set_range(0.0, get_obs_source_dimensions(source).y);
      val.lcds[2]->set_step(0.5, 5.0);
      val.lcds[2]->set_default_value(0.0);

      val.label_lcds[3] = "Left";
      val.combo_display[3] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[3]->set_range(0.0, get_obs_source_dimensions(source).x);
      val.lcds[3]->set_step(0.5, 5.0);
      val.lcds[3]->set_default_value(0.0);

      break;
    case SOURCE_VIDEO_ALIGNMENT:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;

      val.label_text = "Alignment";
      val.list = {"Top Left",      "Top Center",       "Top Right",   "Middle Left",
		  "Middle Center", "Middle Right",     "Bottom Left", "Bottom Center",
		  "Bottom Right",  "Use Message Value"};

      break;
    case SOURCE_VIDEO_SCALE:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM3;

      val.label_lcds[0] = "Scale X";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[0]->set_range(0.0, 100.0);
      val.lcds[0]->set_step(0.5, 5.0);
      val.lcds[0]->set_default_value(0.0);

      val.label_lcds[1] = "Scale Y";
      val.combo_display[1] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[1]->set_range(0.0, 100.0);
      val.lcds[1]->set_step(0.5, 5.0);
      val.lcds[1]->set_default_value(0.0);

      val.label_lcds[2] = "Magnitude";
      val.combo_display[2] = MMGActionDisplayParams::COMBODISPLAY_FIXED;
      val.lcds[2]->set_range(0.5, 100.0);
      val.lcds[2]->set_step(0.5, 5.0);
      val.lcds[2]->set_default_value(1.0);

      break;
    case SOURCE_VIDEO_SCALEFILTER:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;

      val.label_text = "Scale Filtering";
      val.list = {"Disable", "Point", "Bicubic",          "Bilinear",
		  "Lanczos", "Area",  "Use Message Value"};

      break;
    case SOURCE_VIDEO_ROTATION:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Rotation";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI |
			     MMGActionDisplayParams::COMBODISPLAY_MIDI_INVERT;
      val.lcds[0]->set_range(0.0, 360.0);
      val.lcds[0]->set_step(0.5, 5.0);
      val.lcds[0]->set_default_value(0.0);

      break;
    case SOURCE_VIDEO_BOUNDING_BOX_TYPE:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;

      val.label_text = "Bounding Box Type";
      val.list = {"No Bounds",
		  "Stretch to Bounds",
		  "Scale to Inner Bounds",
		  "Scale to Outer Bounds",
		  "Scale to Width of Bounds",
		  "Scale to Height of Bounds",
		  "Maximum Size",
		  "Use Message Value"};

      break;
    case SOURCE_VIDEO_BOUNDING_BOX_SIZE:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM2;

      val.label_lcds[0] = "Bounding Box X";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[0]->set_range(0.0, get_obs_source_dimensions(parent_scene).x);
      val.lcds[0]->set_step(0.5, 5.0);
      val.lcds[0]->set_default_value(0.0);

      val.label_lcds[1] = "Bounding Box Y";
      val.combo_display[1] = MMGActionDisplayParams::COMBODISPLAY_ALL;
      val.lcds[1]->set_range(0.0, get_obs_source_dimensions(parent_scene).y);
      val.lcds[1]->set_step(0.5, 5.0);
      val.lcds[1]->set_default_value(0.0);

      break;
    case SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;

      val.label_text = "Bounding Box Alignment";
      val.list = {"Top Left",      "Top Center",       "Top Right",   "Middle Left",
		  "Middle Center", "Middle Right",     "Bottom Left", "Bottom Center",
		  "Bottom Right",  "Use Message Value"};

      break;
    case SOURCE_VIDEO_BLEND_MODE:
      val.display = MMGActionDisplayParams::DISPLAY_STR3;

      val.label_text = "Blend Mode";
      val.list = {"Normal",   "Additive", "Subtract", "Screen",
		  "Multiply", "Lighten",  "Darken",   "Use Message Value"};

      break;
    case SOURCE_VIDEO_SCREENSHOT:
    case SOURCE_VIDEO_CUSTOM:
    default:
      break;
  }
}
void MMGActionVideoSources::change_options_final(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case SOURCE_VIDEO_DISPLAY:
    case SOURCE_VIDEO_LOCKED:
      action.set_state(action == "Toggle" ? MMGString::STRINGSTATE_TOGGLE
					  : MMGString::STRINGSTATE_FIXED);
      break;
    case SOURCE_VIDEO_ALIGNMENT:
    case SOURCE_VIDEO_BOUNDING_BOX_TYPE:
    case SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
    case SOURCE_VIDEO_SCALEFILTER:
    case SOURCE_VIDEO_BLEND_MODE:
      num1() = val.list.indexOf(action);
      num1().set_state(action == "Use Message Value" ? MMGNumber::NUMBERSTATE_MIDI
						     : MMGNumber::NUMBERSTATE_FIXED);
      break;
    default:
      break;
  }
}
