/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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

#include "../mmg-binding.h"
#include "mmg-action-scenes.h"
#include "../ui/mmg-fields.h"

using namespace MMGUtils;

// MMGActionVideoSources
MMGActionVideoSources::MMGActionVideoSources(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  parent_scene(json_obj, "scene", 1),
	  source(json_obj, "source", 2),
	  action(json_obj, "action", 3),
	  nums{{json_obj, "num1", 1}, {json_obj, "num2", 2}, {json_obj, "num3", 3}, {json_obj, "num4", 4}},
	  _json(new MMGJsonObject(this, json_obj)),
	  at(new ActionTransform)
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionVideoSources::subNames() const
{
	return subModuleTextList({"Move", "Display", "Locking", "Crop", "Align", "Scale", "ScaleFiltering", "Rotate",
				  "BoundingBoxType", "BoundingBoxSize", "BoundingBoxAlign", "BlendingMode",
				  "Screenshot", "Custom"});
}

void MMGActionVideoSources::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	parent_scene.json(json_obj, "scene", false);
	source.json(json_obj, "source", false);
	action.json(json_obj, "action");
	for (int i = 0; i < 4; ++i) {
		nums[i].json(json_obj, MMGText::asString(i + 1, "num"));
	}
	_json->json(json_obj, "json");
}

void MMGActionVideoSources::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionVideoSources *>(dest);
	if (!casted) return;

	casted->parent_scene = parent_scene.copy();
	casted->source = source.copy();
	casted->action = action.copy();
	casted->nums[0] = num1().copy();
	casted->nums[1] = num2().copy();
	casted->nums[2] = num3().copy();
	casted->nums[3] = num4().copy();
	_json->copy(casted->_json);
}

void MMGActionVideoSources::setEditable(bool edit)
{
	parent_scene.setEditable(edit);
	source.setEditable(edit);
	action.setEditable(edit);
	nums[0].setEditable(edit);
	nums[1].setEditable(edit);
	nums[2].setEditable(edit);
	nums[3].setEditable(edit);
	_json->setEditable(edit);
}

void MMGActionVideoSources::toggle()
{
	action.toggle();
	nums[0].toggle();
	nums[1].toggle();
	nums[2].toggle();
	nums[3].toggle();
}

void MMGActionVideoSources::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	connect(display()->addNew(&parent_scene), &MMGStringDisplay::stringChanged, this,
		&MMGActionVideoSources::onList1Change);
	connect(display()->addNew(&source), &MMGStringDisplay::stringChanged, this,
		&MMGActionVideoSources::onList2Change);

	display()->addNew(&action)->setDisplayMode(MMGStringDisplay::MODE_NORMAL);

	display()->addNew(&nums[0]);
	display()->addNew(&nums[1]);
	display()->addNew(&nums[2]);
	display()->addNew(&nums[3]);
}

void MMGActionVideoSources::setActionParams()
{
	display()->hideAll();

	MMGStringDisplay *scene_display = display()->stringDisplay(0);
	scene_display->setVisible(true);
	scene_display->setDescription(obstr("Basic.Scene"));
	scene_display->setBounds(MMGActionScenes::enumerate());
}

void MMGActionVideoSources::onList1Change()
{
	connectSignals(true);

	MMGStringDisplay *source_display = display()->stringDisplay(1);
	source_display->setVisible(true);
	source_display->setDescription(obstr("Basic.Main.Source"));
	source_display->setBounds(MMGActionScenes::enumerateItems(parent_scene));
}

void MMGActionVideoSources::onList2Change()
{
	connectSignals(true);
	updateTransform();

	display()->reset();
	display()->stringDisplay(0)->setVisible(true);
	display()->stringDisplay(1)->setVisible(true);

	MMGStringDisplay *action_display = display()->stringDisplay(2);
	action_display->setVisible(false);

	MMGNumberDisplay *num1_display = display()->numberDisplay(0);
	MMGNumberDisplay *num2_display = display()->numberDisplay(1);
	MMGNumberDisplay *num3_display = display()->numberDisplay(2);
	MMGNumberDisplay *num4_display = display()->numberDisplay(3);

	num1_display->setVisible(false);
	num2_display->setVisible(false);
	num3_display->setVisible(false);
	num4_display->setVisible(false);

	bool source_exists = !source.value().isEmpty();

	switch (sub()) {
		case SOURCE_VIDEO_POSITION:
			num1_display->setVisible(source_exists);
			num1_display->setDescription(obstr("Basic.TransformWindow.PositionX"));
			num1_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num1_display->setBounds(0.0, obsResolution().x, true);
			num1_display->setStep(1.0);
			num1_display->setDefaultValue(at->ti.pos.x);

			num2_display->setVisible(source_exists);
			num2_display->setDescription(obstr("Basic.TransformWindow.PositionY"));
			num2_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num2_display->setBounds(0.0, obsResolution().y, true);
			num2_display->setStep(1.0);
			num2_display->setDefaultValue(at->ti.pos.y);

			num3_display->setVisible(source_exists);
			num3_display->setDescription(mmgtr("Actions.VideoSources.Magnitude"));
			num3_display->setOptions(MIDIBUTTON_FIXED);
			num3_display->setBounds(0.01, 100.0);
			num3_display->setStep(0.01);
			num3_display->setDefaultValue(1.0);

			break;

		case SOURCE_VIDEO_DISPLAY:
			action_display->setVisible(true);
			action_display->setDescription(mmgtr("Actions.VideoSources.DisplayState"));
			action_display->setOptions(MIDIBUTTON_TOGGLE);
			action_display->setBounds({obstr("Show"), obstr("Hide")});
			break;

		case SOURCE_VIDEO_LOCKED:
			action_display->setVisible(true);
			action_display->setDescription(mmgtr("Actions.VideoSources.LockState"));
			action_display->setOptions(MIDIBUTTON_TOGGLE);
			action_display->setBounds(
				{mmgtr("Actions.VideoSources.Locked"), mmgtr("Actions.VideoSources.Unlocked")});
			break;

		case SOURCE_VIDEO_CROP:
			num1_display->setVisible(source_exists);
			num1_display->setDescription(obstr("Basic.TransformWindow.CropTop"));
			num1_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num1_display->setBounds(0.0, sourceResolution().y);
			num1_display->setStep(1.0);
			num1_display->setDefaultValue(at->crop.top);

			num2_display->setVisible(source_exists);
			num2_display->setDescription(obstr("Basic.TransformWindow.CropRight"));
			num2_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num2_display->setBounds(0.0, sourceResolution().x);
			num2_display->setStep(1.0);
			num2_display->setDefaultValue(at->crop.right);

			num3_display->setVisible(source_exists);
			num3_display->setDescription(obstr("Basic.TransformWindow.CropBottom"));
			num3_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num3_display->setBounds(0.0, sourceResolution().y);
			num3_display->setStep(1.0);
			num3_display->setDefaultValue(at->crop.bottom);

			num4_display->setVisible(source_exists);
			num4_display->setDescription(obstr("Basic.TransformWindow.CropLeft"));
			num4_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num4_display->setBounds(0.0, sourceResolution().x);
			num4_display->setStep(1.0);
			num4_display->setDefaultValue(at->crop.left);

			break;

		case SOURCE_VIDEO_ALIGNMENT:
		case SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
			action_display->setVisible(true);
			action_display->setDescription(obstr("Basic.TransformWindow.Alignment"));
			action_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
			action_display->setBounds(alignmentOptions());
			break;

		case SOURCE_VIDEO_SCALE:
			num1_display->setVisible(source_exists);
			num1_display->setDescription(mmgtr("Actions.VideoSources.ScaleX"));
			num1_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num1_display->setBounds(0.0, 100.0);
			num1_display->setStep(0.5);
			num1_display->setDefaultValue(100.0);

			num2_display->setVisible(source_exists);
			num2_display->setDescription(mmgtr("Actions.VideoSources.ScaleY"));
			num2_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num2_display->setBounds(0.0, 100.0);
			num2_display->setStep(0.5);
			num2_display->setDefaultValue(100.0);

			num3_display->setVisible(source_exists);
			num3_display->setDescription(mmgtr("Actions.VideoSources.Magnitude"));
			num3_display->setOptions(MIDIBUTTON_FIXED);
			num3_display->setBounds(0.01, 100.0);
			num3_display->setStep(0.01);
			num3_display->setDefaultValue(1.0);

			break;

		case SOURCE_VIDEO_SCALEFILTER:
			action_display->setVisible(true);
			action_display->setDescription(obstr("ScaleFiltering"));
			action_display->setBounds(scaleFilterOptions());
			action_display->setDefaultValue(scaleFilterOptions().value(at->scale_type));
			break;

		case SOURCE_VIDEO_ROTATION:
			num1_display->setVisible(source_exists);
			num1_display->setDescription(obstr("Basic.TransformWindow.Rotation"));
			num1_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_TOGGLE);
			num1_display->setBounds(0.0, 360.0);
			num1_display->setStep(0.1);
			num1_display->setDefaultValue(at->ti.rot);
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_TYPE:
			action_display->setVisible(true);
			action_display->setDescription(obstr("Basic.TransformWindow.BoundsType"));
			action_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
			action_display->setBounds(boundingBoxOptions());
			action_display->setDefaultValue(boundingBoxOptions().value(at->ti.bounds_type));
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_SIZE:
			num1_display->setVisible(source_exists);
			num1_display->setDescription(obstr("Basic.TransformWindow.BoundsWidth"));
			num1_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num1_display->setBounds(0.0, obsResolution().x, true);
			num1_display->setStep(1.0);
			num1_display->setDefaultValue(at->ti.bounds.x);

			num2_display->setVisible(source_exists);
			num2_display->setDescription(obstr("Basic.TransformWindow.BoundsHeight"));
			num2_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE);
			num2_display->setBounds(0.0, obsResolution().y, true);
			num2_display->setStep(1.0);
			num2_display->setDefaultValue(at->ti.bounds.y);

			break;

		case SOURCE_VIDEO_BLEND_MODE:
			action_display->setVisible(true);
			action_display->setDescription(obstr("BlendingMode"));
			action_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
			action_display->setBounds(blendModeOptions());
			action_display->setDefaultValue(blendModeOptions().value(at->blend_type));
			break;

		case SOURCE_VIDEO_SCREENSHOT:
			if (type() == TYPE_OUTPUT) display()->hideAll();
			break;

		case SOURCE_VIDEO_CUSTOM:
			display()->setFields(
				MMGOBSFields::registerSource(OBSSourceAutoRelease(currentSource()), _json));
			break;

		default:
			return;
	}

	num1_display->reset();
	num2_display->reset();
	num3_display->reset();
	num4_display->reset();
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

const QStringList MMGActionVideoSources::alignmentOptions()
{
	return MMGText::batch(TEXT_OBS, "Basic.TransformWindow.Alignment",
			      {"TopLeft", "TopCenter", "TopRight", "CenterLeft", "Center", "CenterRight", "BottomLeft",
			       "BottomCenter", "BottomRight"});
}

const QStringList MMGActionVideoSources::boundingBoxOptions()
{
	return MMGText::batch(TEXT_OBS, "Basic.TransformWindow.BoundsType",
			      {"None", "Stretch", "ScaleInner", "ScaleOuter", "ScaleToWidth", "ScaleToHeight",
			       "MaxOnly"});
}

const QStringList MMGActionVideoSources::scaleFilterOptions()
{
	QStringList opts =
		MMGText::batch(TEXT_OBS, "ScaleFiltering", {"Point", "Bicubic", "Bilinear", "Lanczos", "Area"});
	opts.prepend(obstr("Disable"));
	return opts;
}

const QStringList MMGActionVideoSources::blendModeOptions()
{
	return MMGText::batch(TEXT_OBS, "BlendingMode",
			      {"Normal", "Additive", "Subtract", "Screen", "Multiply", "Lighten", "Darken"});
}

const vec2 MMGActionVideoSources::obsResolution()
{
	obs_video_info video_info;
	obs_get_video_info(&video_info);
	vec2 xy;
	vec2_set(&xy, video_info.base_width, video_info.base_height);
	return xy;
}

uint32_t MMGActionVideoSources::convertAlignment(bool to_align, uint32_t value)
{
	uint32_t result;

	if (to_align) {
		result = 0;
		if (value <= 2) result |= OBS_ALIGN_TOP;
		if (value >= 6) result |= OBS_ALIGN_BOTTOM;
		if (value % 3 == 0) result |= OBS_ALIGN_LEFT;
		if (value % 3 == 2) result |= OBS_ALIGN_RIGHT;
	} else {
		result = 4;
		if (value & OBS_ALIGN_TOP) result = 1;
		if (value & OBS_ALIGN_BOTTOM) result = 7;
		if (value & OBS_ALIGN_LEFT) --result;
		if (value & OBS_ALIGN_RIGHT) ++result;
	}

	return result;
}

obs_scene_t *MMGActionVideoSources::currentScene() const
{
	return obs_get_scene_by_name(parent_scene.mmgtocs());
}

obs_source_t *MMGActionVideoSources::currentSource() const
{
	return obs_get_source_by_name(source.mmgtocs());
}

const vec2 MMGActionVideoSources::sourceResolution() const
{
	OBSSourceAutoRelease obs_source = currentSource();
	vec2 xy;
	vec2_set(&xy, obs_source_get_width(obs_source), obs_source_get_height(obs_source));
	return xy;
}

void MMGActionVideoSources::updateTransform() const
{
	OBSSceneAutoRelease obs_scene = currentScene();
	OBSSourceAutoRelease obs_source = currentSource();
	if (!obs_source || !obs_scene) return;

	obs_sceneitem_t *obs_sceneitem = obs_scene_find_source_recursive(obs_scene, obs_source_get_name(obs_source));
	if (!obs_sceneitem) return;

	obs_sceneitem_get_info2(obs_sceneitem, &at->ti);
	obs_sceneitem_get_crop(obs_sceneitem, &at->crop);
	at->scale_type = obs_sceneitem_get_scale_filter(obs_sceneitem);
	at->blend_type = obs_sceneitem_get_blending_mode(obs_sceneitem);
}

void MMGActionVideoSources::execute(const MMGMessage *midi) const
{
	OBSSceneAutoRelease obs_scene = currentScene();
	OBSSourceAutoRelease obs_source = currentSource();
	ACTION_ASSERT(obs_source && obs_scene, "Scene or source does not exist.");

	obs_sceneitem_t *obs_sceneitem = obs_scene_find_source_recursive(obs_scene, obs_source_get_name(obs_source));
	ACTION_ASSERT(obs_sceneitem, "Scene does not contain source.");

	updateTransform();
	double util_value = 0;
	QStringList util_list;

	switch (sub()) {
		case SOURCE_VIDEO_POSITION:
			at->ti.pos.x = num1().chooseFrom(midi, false, at->ti.pos.x / num3()) * num3();
			at->ti.pos.y = num2().chooseFrom(midi, false, at->ti.pos.y / num3()) * num3();
			obs_sceneitem_set_pos(obs_sceneitem, &at->ti.pos);
			break;

		case SOURCE_VIDEO_DISPLAY:
			if (action.state() == STATE_TOGGLE) {
				obs_sceneitem_set_visible(obs_sceneitem, !obs_sceneitem_visible(obs_sceneitem));
			} else {
				obs_sceneitem_set_visible(obs_sceneitem, action == obstr("Show"));
			}
			break;

		case SOURCE_VIDEO_LOCKED:
			if (action.state() == STATE_TOGGLE) {
				obs_sceneitem_set_locked(obs_sceneitem, !obs_sceneitem_locked(obs_sceneitem));
			} else {
				obs_sceneitem_set_locked(obs_sceneitem, action == mmgtr("Actions.VideoSources.Locked"));
			}
			break;

		case SOURCE_VIDEO_CROP:
			at->crop.top = num1().chooseFrom(midi, false, at->crop.top);
			at->crop.right = num2().chooseFrom(midi, false, at->crop.right);
			at->crop.bottom = num3().chooseFrom(midi, false, at->crop.bottom);
			at->crop.left = num4().chooseFrom(midi, false, at->crop.left);
			obs_sceneitem_set_crop(obs_sceneitem, &at->crop);
			break;

		case SOURCE_VIDEO_ALIGNMENT:
			util_list = alignmentOptions();
			util_value = util_list.indexOf(action.chooseFrom(midi, util_list));
			obs_sceneitem_set_alignment(obs_sceneitem, convertAlignment(true, util_value));
			break;

		case SOURCE_VIDEO_SCALE:
			util_value = 100 / num3(); // Multiplier
			at->ti.scale.x = num1().chooseFrom(midi, false, at->ti.scale.x * util_value) / util_value;
			at->ti.scale.y = num2().chooseFrom(midi, false, at->ti.scale.y * util_value) / util_value;
			obs_sceneitem_set_scale(obs_sceneitem, &at->ti.scale);
			break;

		case SOURCE_VIDEO_SCALEFILTER:
			util_list = scaleFilterOptions();
			util_value = util_list.indexOf(action.chooseFrom(midi, util_list));
			obs_sceneitem_set_scale_filter(obs_sceneitem, (obs_scale_type)(util_value));
			break;

		case SOURCE_VIDEO_ROTATION:
			obs_sceneitem_set_rot(obs_sceneitem, num1().chooseFrom(midi));
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_TYPE:
			util_list = boundingBoxOptions();
			util_value = util_list.indexOf(action.chooseFrom(midi, util_list));
			obs_sceneitem_set_bounds_type(obs_sceneitem, (obs_bounds_type)(util_value));
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_SIZE:
			at->ti.bounds.x = num1().chooseFrom(midi, false, at->ti.bounds.x);
			at->ti.bounds.y = num2().chooseFrom(midi, false, at->ti.bounds.y);
			obs_sceneitem_set_bounds(obs_sceneitem, &at->ti.bounds);
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
			util_list = alignmentOptions();
			util_value = util_list.indexOf(action.chooseFrom(midi, util_list));
			obs_sceneitem_set_bounds_alignment(obs_sceneitem, convertAlignment(true, util_value));
			break;

		case SOURCE_VIDEO_BLEND_MODE:
			util_list = blendModeOptions();
			util_value = util_list.indexOf(action.chooseFrom(midi, util_list));
			obs_sceneitem_set_blending_mode(obs_sceneitem, (obs_blending_type)(util_value));
			break;

		case SOURCE_VIDEO_SCREENSHOT:
			obs_frontend_take_source_screenshot(obs_source);
			break;

		case SOURCE_VIDEO_CUSTOM:
			MMGOBSFields::execute(obs_source, _json, midi);
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionVideoSources::connectSignals(bool _connect)
{
	MMGAction::connectSignals(_connect);
	if (!_connected && type() == TYPE_OUTPUT) return;

	if (type() == TYPE_OUTPUT) {
		connectSourceSignal(mmgsignals()->requestSourceSignalByName(parent_scene));
		connectSourceSignal(mmgsignals()->requestSourceSignalByName(source));
		connectSceneGroups(OBSSceneAutoRelease(currentScene()));
	}

	MMGOBSFields::registerSource(OBSSourceAutoRelease(currentSource()), _json);
}

void MMGActionVideoSources::connectSceneGroups(obs_scene_t *scene)
{
	obs_scene_enum_items(
		scene,
		[](obs_scene_t *, obs_sceneitem_t *sceneitem, void *ptr) {
			if (!obs_sceneitem_is_group(sceneitem)) return true;

			auto action = static_cast<MMGActionVideoSources *>(ptr);
			if (!action) return false;

			action->connectSourceSignal(
				mmgsignals()->requestSourceSignal(obs_sceneitem_get_source(sceneitem)));
			action->connectSceneGroups(obs_sceneitem_group_get_scene(sceneitem));
			return true;
		},
		this);
}

void MMGActionVideoSources::frontendEventReceived(obs_frontend_event event)
{
	if (sub() != SOURCE_VIDEO_SCREENSHOT_TAKEN) return;
	if (event != OBS_FRONTEND_EVENT_SCREENSHOT_TAKEN) return;
	triggerEvent();
}

void MMGActionVideoSources::sourceEventReceived(MMGSourceSignal::Event event, QVariant data)
{
	OBSSceneAutoRelease obs_scene = currentScene();
	OBSSourceAutoRelease obs_source = currentSource();
	ACTION_ASSERT(obs_source && obs_scene, "Scene or source does not exist.");

	obs_sceneitem_t *obs_sceneitem = obs_scene_find_source_recursive(obs_scene, obs_source_get_name(obs_source));
	ACTION_ASSERT(obs_sceneitem, "Scene does not contain source.");

	switch (sub()) {
		case SOURCE_VIDEO_DISPLAY_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_VISIBILITY) return;
			if (data.value<void *>() != obs_sceneitem) return;
			if ((action == obstr("Show")) != obs_sceneitem_visible(obs_sceneitem)) return;

			triggerEvent();
			break;

		case SOURCE_VIDEO_LOCK_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_LOCK) return;
			if (data.value<void *>() != obs_sceneitem) return;
			if ((action == mmgtr("Actions.VideoSources.Locked")) != obs_sceneitem_locked(obs_sceneitem))
				return;

			triggerEvent();
			break;

		case SOURCE_VIDEO_MOVED:
		case SOURCE_VIDEO_CROPPED:
		case SOURCE_VIDEO_ALIGNED:
		case SOURCE_VIDEO_SCALED:
		case SOURCE_VIDEO_SCALEFILTER_CHANGED:
		case SOURCE_VIDEO_ROTATED:
		case SOURCE_VIDEO_BOUNDING_BOX_TYPE_CHANGED:
		case SOURCE_VIDEO_BOUNDING_BOX_RESIZED:
		case SOURCE_VIDEO_BOUNDING_BOX_ALIGNED:
		case SOURCE_VIDEO_BLEND_MODE_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_TRANSFORM) return;
			if (data.value<void *>() != obs_sceneitem) return;

			sourceTransformCallback(obs_sceneitem);
			break;

		case SOURCE_VIDEO_CUSTOM_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_UPDATE) return;
			if (data.value<void *>() != obs_source) return;

			triggerEvent(MMGOBSFields::customEventReceived(obs_source, _json));
			break;

		default:
			break;
	}
}

void MMGActionVideoSources::sourceTransformCallback(obs_sceneitem_t *obs_sceneitem)
{
	if (source != obs_source_get_name(obs_sceneitem_get_source(obs_sceneitem))) return;

	QList<MMGNumber> values;
	MMGNumber current_value;

	ActionTransform old_at = *at;
	updateTransform();

	switch (sub()) {
		case SOURCE_VIDEO_MOVED:
			if (at->ti.pos.x == old_at.ti.pos.x && at->ti.pos.y == old_at.ti.pos.y) return;

			if (at->ti.pos.x != old_at.ti.pos.x && num1().acceptable(at->ti.pos.x)) {
				current_value = num1();
				current_value = at->ti.pos.x / num3();
				values += current_value;
			}
			if (at->ti.pos.y != old_at.ti.pos.y && num2().acceptable(at->ti.pos.y)) {
				current_value = num2();
				current_value = at->ti.pos.y / num3();
				values += current_value;
			}
			break;

		case SOURCE_VIDEO_CROPPED:
			if (at->crop.top == old_at.crop.top && at->crop.right == old_at.crop.right &&
			    at->crop.bottom == old_at.crop.bottom && at->crop.left == old_at.crop.left)
				return;

			if (at->crop.top != old_at.crop.top && num1().acceptable(at->crop.top)) {
				current_value = num1();
				current_value = at->crop.top;
				values += current_value;
			}
			if (at->crop.right != old_at.crop.right && num2().acceptable(at->crop.right)) {
				current_value = num2();
				current_value = at->crop.right;
				values += current_value;
			}
			if (at->crop.bottom != old_at.crop.bottom && num3().acceptable(at->crop.bottom)) {
				current_value = num3();
				current_value = at->crop.bottom;
				values += current_value;
			}
			if (at->crop.left != old_at.crop.left && num4().acceptable(at->crop.left)) {
				current_value = num4();
				current_value = at->crop.left;
				values += current_value;
			}
			break;

		case SOURCE_VIDEO_ALIGNED:
			if (at->ti.alignment == old_at.ti.alignment) return;
			current_value = convertAlignment(false, at->ti.alignment);
			if (action.chooseTo(current_value, alignmentOptions())) return;
			values += current_value;
			break;

		case SOURCE_VIDEO_SCALED:
			if (at->ti.scale.x == old_at.ti.scale.x && at->ti.scale.y == old_at.ti.scale.y) return;

			if (at->ti.scale.x != old_at.ti.scale.x && num1().acceptable(at->ti.scale.x)) {
				current_value = num1();
				current_value = at->ti.scale.x * 100 / num3();
				values += current_value;
			}
			if (at->ti.scale.y != old_at.ti.scale.y && num2().acceptable(at->ti.scale.y)) {
				current_value = num2();
				current_value = at->ti.scale.y * 100 / num3();
				values += current_value;
			}
			break;

		case SOURCE_VIDEO_SCALEFILTER_CHANGED:
			if (at->scale_type == old_at.scale_type) return;
			current_value = at->scale_type;
			if (action.chooseTo(current_value, scaleFilterOptions())) return;
			values += current_value;
			break;

		case SOURCE_VIDEO_ROTATED:
			if (at->ti.rot == old_at.ti.rot || !num1().acceptable(at->ti.rot)) return;
			current_value = num1();
			current_value = at->ti.rot;
			values += current_value;
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_TYPE_CHANGED:
			if (at->ti.bounds_type == old_at.ti.bounds_type) return;
			current_value = at->ti.bounds_type;
			if (action.chooseTo(current_value, boundingBoxOptions())) return;
			values += current_value;
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_RESIZED:
			if (at->ti.bounds.x == old_at.ti.bounds.x && at->ti.bounds.y == old_at.ti.bounds.y) return;

			if (at->ti.bounds.x != old_at.ti.bounds.x && num1().acceptable(at->ti.bounds.x)) {
				current_value = num1();
				current_value = at->ti.bounds.x;
				values += current_value;
			}
			if (at->ti.bounds.y != old_at.ti.bounds.y && num2().acceptable(at->ti.bounds.y)) {
				current_value = num2();
				current_value = at->ti.bounds.y;
				values += current_value;
			}
			break;

		case SOURCE_VIDEO_BOUNDING_BOX_ALIGNED:
			if (at->ti.bounds_alignment == old_at.ti.bounds_alignment) return;
			current_value = convertAlignment(false, at->ti.bounds_alignment);
			if (action.chooseTo(current_value, alignmentOptions())) return;
			values += current_value;
			break;

		case SOURCE_VIDEO_BLEND_MODE_CHANGED:
			if (at->blend_type == old_at.blend_type) return;
			current_value = at->blend_type;
			if (action.chooseTo(current_value, blendModeOptions())) return;
			values += current_value;
			break;

		default:
			return;
	}

	triggerEvent(values);
}