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

#include "mmg-action-video-sources.h"
#include "mmg-action-scenes.h"

using namespace MMGUtils;

#define ALIGN_ACTION(value)                            \
  if ((value) <= 2) align |= OBS_ALIGN_TOP;            \
  if ((value) >= 6) align |= OBS_ALIGN_BOTTOM;         \
  if ((uint)(value) % 3 == 0) align |= OBS_ALIGN_LEFT; \
  if ((uint)(value) % 3 == 2) align |= OBS_ALIGN_RIGHT

const QStringList MMGActionVideoSources::alignment_options{
  "Top Left",     "Top Center",  "Top Right",     "Middle Left",  "Middle Center",
  "Middle Right", "Bottom Left", "Bottom Center", "Bottom Right", "Use Message Value"};

const QStringList MMGActionVideoSources::boundingbox_options{"No Bounds",
							     "Stretch to Bounds",
							     "Scale to Inner Bounds",
							     "Scale to Outer Bounds",
							     "Scale to Width of Bounds",
							     "Scale to Height of Bounds",
							     "Maximum Size",
							     "Use Message Value"};

const QStringList MMGActionVideoSources::scalefilter_options{
  "Disable", "Point", "Bicubic", "Bilinear", "Lanczos", "Area", "Use Message Value"};

const QStringList MMGActionVideoSources::blendmode_options{
  "Normal", "Additive", "Subtract", "Screen", "Multiply", "Lighten", "Darken", "Use Message Value"};

MMGActionVideoSources::MMGActionVideoSources(const QJsonObject &json_obj)
  : parent_scene(json_obj, "scene", 1),
    source(json_obj, "source", 2),
    action(json_obj, "action", 3),
    json_str(json_obj, "json_str", 0),
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
  MMGAction::json(json_obj);

  parent_scene.json(json_obj, "scene", false);
  source.json(json_obj, "source", false);
  action.json(json_obj, "action");
  json_str.json(json_obj, "json_str", false);
  for (int i = 0; i < 4; ++i) {
    nums[i].json(json_obj, num_to_str(i + 1, "num"));
  }
}

void MMGActionVideoSources::execute(const MMGMessage *midi) const
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

  switch (sub()) {
    case MMGActionVideoSources::SOURCE_VIDEO_POSITION:
      obs_sceneitem_get_pos(obs_sceneitem, &get_vec2);
      vec2_set(&set_vec2, num1().choose(midi, get_vec2.x / num3()) * num3(),
	       num2().choose(midi, get_vec2.y / num3()) * num3());
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
      crop.top = num1().choose(midi, crop.top);
      crop.right = num2().choose(midi, crop.right);
      crop.bottom = num3().choose(midi, crop.bottom);
      crop.left = num4().choose(midi, crop.left);
      obs_sceneitem_set_crop(obs_sceneitem, &crop);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_ALIGNMENT:
      if (action.state() != 0) {
	if (midi->value() > 8) {
	  blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	  return;
	}
	ALIGN_ACTION(midi->value());
      } else {
	ALIGN_ACTION(alignment_options.indexOf(action));
      }
      obs_sceneitem_set_alignment(obs_sceneitem, align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_SCALE:
      // Multiplier
      align = 100 / num3();
      obs_sceneitem_get_scale(obs_sceneitem, &get_vec2);
      vec2_set(&set_vec2, num1().choose(midi, get_vec2.x * align) / align,
	       num2().choose(midi, get_vec2.y * align) / align);
      obs_sceneitem_set_scale(obs_sceneitem, &set_vec2);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_SCALEFILTER:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 6)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      align = !action.state() ? midi->value() : scalefilter_options.indexOf(action);
      obs_sceneitem_set_scale_filter(obs_sceneitem, (obs_scale_type)align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_ROTATION:
      obs_sceneitem_set_rot(obs_sceneitem, num1().choose(midi));
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BOUNDING_BOX_TYPE:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 7)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      align = !action.state() ? midi->value() : boundingbox_options.indexOf(action);
      obs_sceneitem_set_bounds_type(obs_sceneitem, (obs_bounds_type)align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BOUNDING_BOX_SIZE:
      obs_sceneitem_get_bounds(obs_sceneitem, &get_vec2);
      vec2_set(&set_vec2, num1().choose(midi, get_vec2.x), num2().choose(midi, get_vec2.y));
      obs_sceneitem_set_bounds(obs_sceneitem, &set_vec2);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
      if (action.state() != 0) {
	if (midi->value() > 8) {
	  blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	  return;
	}
	ALIGN_ACTION(midi->value());
      } else {
	ALIGN_ACTION(alignment_options.indexOf(action));
      }
      obs_sceneitem_set_bounds_alignment(obs_sceneitem, align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_BLEND_MODE:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num1(), 7)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeded choice options.");
	return;
      }
      align = !action.state() ? midi->value() : blendmode_options.indexOf(action);
      obs_sceneitem_set_blending_mode(obs_sceneitem, (obs_blending_type)align);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_SCREENSHOT:
      obs_frontend_take_source_screenshot(obs_source);
      break;
    case MMGActionVideoSources::SOURCE_VIDEO_CUSTOM:
      obs_source_custom_update(obs_source, json_from_str(json_str.mmgtocs()), midi);
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionVideoSources::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionVideoSources *>(dest);
  if (!casted) return;

  casted->parent_scene = parent_scene.copy();
  casted->source = source.copy();
  casted->action = action.copy();
  casted->json_str = json_str.copy();
  casted->nums[0] = num1().copy();
  casted->nums[1] = num2().copy();
  casted->nums[2] = num3().copy();
  casted->nums[3] = num4().copy();
}

void MMGActionVideoSources::setEditable(bool edit)
{
  parent_scene.set_edit(edit);
  source.set_edit(edit);
  action.set_edit(edit);
  json_str.set_edit(edit);
  nums[0].set_edit(edit);
  nums[1].set_edit(edit);
  nums[2].set_edit(edit);
  nums[3].set_edit(edit);
}

const QStringList MMGActionVideoSources::enumerate()
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

void MMGActionVideoSources::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&parent_scene);
  _display->setStr2Storage(&source);
  _display->setStr3Storage(&action);

  MMGNumberDisplay *num1_display = new MMGNumberDisplay(_display->numberDisplays());
  num1_display->setStorage(&nums[0], true);
  _display->numberDisplays()->add(num1_display);
  MMGNumberDisplay *num2_display = new MMGNumberDisplay(_display->numberDisplays());
  num2_display->setStorage(&nums[1], true);
  _display->numberDisplays()->add(num2_display);
  MMGNumberDisplay *num3_display = new MMGNumberDisplay(_display->numberDisplays());
  num3_display->setStorage(&nums[2], true);
  _display->numberDisplays()->add(num3_display);
  MMGNumberDisplay *num4_display = new MMGNumberDisplay(_display->numberDisplays());
  num4_display->setStorage(&nums[3], true);
  _display->numberDisplays()->add(num4_display);

  _display->connect(_display, &MMGActionDisplay::str1Changed, [&]() { setList1Config(); });
  _display->connect(_display, &MMGActionDisplay::str2Changed, [&]() { setList2Config(); });
}

void MMGActionVideoSources::setSubOptions(QComboBox *sub)
{
  sub->addItems({"Move Source", "Display Source", "Source Locking", "Source Crop", "Align Source",
		 "Source Scale", "Source Scale Filtering", "Rotate Source",
		 "Source Bounding Box Type", "Resize Source Bounding Box",
		 "Align Source Bounding Box", "Source Blending Mode", "Take Source Screenshot",
		 "Custom Source Settings"});
}

void MMGActionVideoSources::setSubConfig()
{
  _display->setVisible(true);
  _display->setStr1Visible(false);
  _display->setStr2Visible(false);
  _display->setStr3Visible(false);

  _display->setStr1Visible(true);
  _display->setStr1Description("Scene");
  _display->setStr1Options(MMGActionScenes::enumerate());
}

void MMGActionVideoSources::setList1Config()
{
  _display->setStr2Visible(true);
  _display->setStr2Description("Source");
  _display->setStr2Options(MMGActionScenes::enumerate_items(parent_scene));
}

void MMGActionVideoSources::setList2Config()
{
  _display->resetScrollWidget();

  MMGNumberDisplay *num1_display = _display->numberDisplays()->fieldAt(0);
  MMGNumberDisplay *num2_display = _display->numberDisplays()->fieldAt(1);
  MMGNumberDisplay *num3_display = _display->numberDisplays()->fieldAt(2);
  MMGNumberDisplay *num4_display = _display->numberDisplays()->fieldAt(3);

  num1_display->setVisible(false);
  num2_display->setVisible(false);
  num3_display->setVisible(false);
  num4_display->setVisible(false);

  bool source_exists = !source.str().isEmpty();

  switch ((Actions)subcategory) {
    case SOURCE_VIDEO_POSITION:
      num1_display->setVisible(source_exists);
      num1_display->setDescription("Position X");
      num1_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num1_display->setBounds(0.0, obsResolution().x, true);
      num1_display->setStep(0.5);
      num1_display->setDefaultValue(0.0);

      num2_display->setVisible(source_exists);
      num2_display->setDescription("Position Y");
      num2_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num2_display->setBounds(0.0, obsResolution().y, true);
      num2_display->setStep(0.5);
      num2_display->setDefaultValue(0.0);

      num3_display->setVisible(source_exists);
      num3_display->setDescription("Magnitude");
      num3_display->setOptions(MMGNumberDisplay::OPTIONS_FIXED_ONLY);
      num3_display->setBounds(0.01, 100.0);
      num3_display->setStep(0.01);
      num3_display->setDefaultValue(1.0);

      break;

    case SOURCE_VIDEO_DISPLAY:
      _display->setStr3Visible(true);
      _display->setStr3Description("State");
      _display->setStr3Options({"Show", "Hide", "Toggle"});
      break;

    case SOURCE_VIDEO_LOCKED:
      _display->setStr3Visible(true);
      _display->setStr3Description("State");
      _display->setStr3Options({"Locked", "Unlocked", "Toggle"});
      break;

    case SOURCE_VIDEO_CROP:
      num1_display->setVisible(source_exists);
      num1_display->setDescription("Top");
      num1_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num1_display->setBounds(0.0, sourceResolution().y);
      num1_display->setStep(0.5);
      num1_display->setDefaultValue(0.0);

      num2_display->setVisible(source_exists);
      num2_display->setDescription("Right");
      num2_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num2_display->setBounds(0.0, sourceResolution().x);
      num2_display->setStep(0.5);
      num2_display->setDefaultValue(0.0);

      num3_display->setVisible(source_exists);
      num3_display->setDescription("Bottom");
      num3_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num3_display->setBounds(0.0, sourceResolution().y);
      num3_display->setStep(0.5);
      num3_display->setDefaultValue(0.0);

      num4_display->setVisible(source_exists);
      num4_display->setDescription("Left");
      num4_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num4_display->setBounds(0.0, sourceResolution().x);
      num4_display->setStep(0.5);
      num4_display->setDefaultValue(0.0);

      break;

    case SOURCE_VIDEO_ALIGNMENT:
      _display->setStr3Visible(true);
      _display->setStr3Description("Alignment");
      _display->setStr3Options(alignment_options);
      break;

    case SOURCE_VIDEO_SCALE:
      num1_display->setVisible(source_exists);
      num1_display->setDescription("Scale X");
      num1_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num1_display->setBounds(0.0, 100.0);
      num1_display->setStep(0.5);
      num1_display->setDefaultValue(0.0);

      num2_display->setVisible(source_exists);
      num2_display->setDescription("Scale Y");
      num2_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num2_display->setBounds(0.0, 100.0);
      num2_display->setStep(0.5);
      num2_display->setDefaultValue(0.0);

      num3_display->setVisible(source_exists);
      num3_display->setDescription("Magnitude");
      num3_display->setOptions(MMGNumberDisplay::OPTIONS_FIXED_ONLY);
      num3_display->setBounds(0.01, 100.0);
      num3_display->setStep(0.01);
      num3_display->setDefaultValue(1.0);

      break;

    case SOURCE_VIDEO_SCALEFILTER:
      _display->setStr3Visible(true);
      _display->setStr3Description("Scale Filtering");
      _display->setStr3Options(scalefilter_options);
      break;

    case SOURCE_VIDEO_ROTATION:
      num1_display->setVisible(source_exists);
      num1_display->setDescription("Rotation");
      num1_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_CUSTOM);
      num1_display->setBounds(0.0, 360.0);
      num1_display->setStep(0.1);

      break;

    case SOURCE_VIDEO_BOUNDING_BOX_TYPE:
      _display->setStr3Visible(true);
      _display->setStr3Description("Bounding Box Type");
      _display->setStr3Options(boundingbox_options);
      break;

    case SOURCE_VIDEO_BOUNDING_BOX_SIZE:
      num1_display->setVisible(source_exists);
      num1_display->setDescription("Bounding Box X");
      num1_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num1_display->setBounds(0.0, obsResolution().x, true);
      num1_display->setStep(0.5);
      num1_display->setDefaultValue(0.0);

      num2_display->setVisible(source_exists);
      num2_display->setDescription("Bounding Box Y");
      num2_display->setOptions(MMGNumberDisplay::OPTIONS_ALL);
      num2_display->setBounds(0.0, obsResolution().y, true);
      num2_display->setStep(0.5);
      num2_display->setDefaultValue(0.0);

      break;

    case SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
      _display->setStr3Visible(true);
      _display->setStr3Description("Bounding Box Alignment");
      _display->setStr3Options(alignment_options);
      break;

    case SOURCE_VIDEO_BLEND_MODE:
      _display->setStr3Visible(true);
      _display->setStr3Description("Blend Mode");
      _display->setStr3Options(blendmode_options);
      break;

    case SOURCE_VIDEO_SCREENSHOT:
      break;

    case SOURCE_VIDEO_CUSTOM:
      emit _display->customFieldRequest(
	OBSSourceAutoRelease(obs_get_source_by_name(source.mmgtocs())), &json_str);
      break;

    default:
      return;
  }

  num1_display->reset();
  num2_display->reset();
  num3_display->reset();
  num4_display->reset();
}

const vec2 MMGActionVideoSources::obsResolution()
{
  obs_video_info video_info;
  obs_get_video_info(&video_info);
  vec2 xy;
  vec2_set(&xy, video_info.base_width, video_info.base_height);
  return xy;
}

const vec2 MMGActionVideoSources::sourceResolution() const
{
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  vec2 xy;
  vec2_set(&xy, obs_source_get_width(obs_source), obs_source_get_height(obs_source));
  return xy;
}
