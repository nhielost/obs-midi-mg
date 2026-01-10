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

#include "mmg-action-scene-items.h"
#include "mmg-action-scenes.h"

vec2 &operator*=(vec2 &lhs, float rhs)
{
	vec2_mulf(&lhs, &lhs, rhs);
	return lhs;
}

vec2 &operator/=(vec2 &lhs, float rhs)
{
	vec2_divf(&lhs, &lhs, rhs);
	return lhs;
}

bool operator==(const vec2 &v1, const vec2 &v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

bool operator==(const obs_sceneitem_crop &c1, const obs_sceneitem_crop &c2)
{
	return c1.left == c2.left && c1.top == c2.top && c1.right == c2.right && c1.bottom == c2.bottom;
}

namespace MMGActions {

const vec2 obsResolution()
{
	obs_video_info video_info;
	obs_get_video_info(&video_info);
	vec2 xy;
	vec2_set(&xy, video_info.base_width, video_info.base_height);
	return xy;
}

// MMGActionSceneItems
MMGParams<MMGString> MMGActionSceneItems::source_params {
	.desc = obstr("Basic.Main.Source"),
	.options = OPTION_NONE,
	.default_value = "",
	.bounds = {},
	.placeholder = mmgtr("Actions.SceneItems.Placeholder"),
};

MMGActionSceneItems::MMGActionSceneItems(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  scene(json_obj, "scene"),
	  source(json_obj, "source")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItems::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(scene, json_obj, "scene", 1, enumerateScenes());
	MMGCompatibility::initOldStringData(source, json_obj, "source", 2, enumerateSceneItems(scene, sourceBounds()));
}

void MMGActionSceneItems::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	scene->json(json_obj, "scene");
	source->json(json_obj, "source");
}

void MMGActionSceneItems::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItems *>(dest);
	if (!casted) return;

	scene.copy(casted->scene);
	source.copy(casted->source);
}

obs_sceneitem_t *MMGActionSceneItems::getSceneItem(const MMGString &source) const
{
	OBSSourceAutoRelease obs_scene = obs_get_source_by_uuid(scene.as<STATE_FIXED>()->value());
	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(source);
	if (!(obs_source && obs_scene)) return nullptr;

	return obs_scene_find_source_recursive(obs_scene_from_source(obs_scene), obs_source_get_name(obs_source));
}

void MMGActionSceneItems::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &scene, &sceneParams(2),
				      std::bind(&MMGActionSceneItems::onSceneChanged, this));
	MMGActions::createActionField(display, &source, &source_params,
				      std::bind(&MMGActionSceneItems::onSourceChanged, this));
}

void MMGActionSceneItems::onSceneChanged()
{
	source_params.bounds = enumerateSceneItems(scene, sourceBounds());
	source_params.default_value = !source_params.bounds.isEmpty() ? source_params.bounds.firstKey() : MMGString();
	source_params.options.setFlag(OPTION_DISABLED, source_params.bounds.isEmpty());
	emit refreshRequested();
}

void MMGActionSceneItems::onSourceChanged()
{
	onSourceFixedChanged(getSceneItem(source));
}

void MMGActionSceneItems::execute(const MMGMappingTest &test) const
{
	obs_sceneitem_t *obs_sceneitem = getSceneItem(source);
	ACTION_ASSERT(obs_sceneitem, "The scene item does not exist.");

	execute(test, obs_sceneitem);
}

void MMGActionSceneItems::processEvent(const calldata_t *cd) const
{
	obs_sceneitem_t *obs_sceneitem = (obs_sceneitem_t *)(calldata_ptr(cd, "item"));

	EventFulfillment fulfiller(this);
	const char *sceneitem_uuid = obs_source_get_uuid(obs_sceneitem_get_source(obs_sceneitem));
	fulfiller->addCondition(source == sceneitem_uuid);

	applied_source_id = sceneitem_uuid;
	return processEvent(*fulfiller, obs_sceneitem);
}
// End MMGActionSceneItems

// MMGActionSceneItemsVisible
MMGParams<bool> MMGActionSceneItemsVisible::display_params {
	.desc = obstr("Basic.Main.Sources.Visibility"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionSceneItemsVisible::MMGActionSceneItemsVisible(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  disp(json_obj, "visible")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsVisible::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldStringData(disp, json_obj, "action", 3,
					    {{true, obstr("Show")}, {false, obstr("Hide")}});
}

void MMGActionSceneItemsVisible::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	disp->json(json_obj, "visible");
}

void MMGActionSceneItemsVisible::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsVisible *>(dest);
	if (!casted) return;

	disp.copy(casted->disp);
}

void MMGActionSceneItemsVisible::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &disp, &display_params);
}

void MMGActionSceneItemsVisible::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	display_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (display_params.options.testFlag(OPTION_HIDDEN)) return;

	display_params.default_value = !obs_sceneitem_visible(obs_sceneitem);
}

void MMGActionSceneItemsVisible::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	bool display = obs_sceneitem_visible(obs_sceneitem);
	ACTION_ASSERT(test.applicable(disp, display), "A visibility could not be selected. Check the Visibility "
						      "field and try again.");
	obs_sceneitem_set_visible(obs_sceneitem, display);
}

void MMGActionSceneItemsVisible::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	test.addAcceptable(disp, obs_sceneitem_visible(obs_sceneitem));
}
// End MMGActionSceneItemsVisible

// MMGActionSceneItemsLocked
MMGParams<bool> MMGActionSceneItemsLocked::lock_params {
	.desc = mmgtr("Actions.SceneItems.LockStatus"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = false,
};

MMGActionSceneItemsLocked::MMGActionSceneItemsLocked(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  lock(json_obj, "locked")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsLocked::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldStringData(lock, json_obj, "action", 3,
					    {{true, mmgtr("Actions.SceneItems.Locked")},
					     {false, mmgtr("Actions.SceneItems.Unlocked")}});
}

void MMGActionSceneItemsLocked::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	lock->json(json_obj, "locked");
}

void MMGActionSceneItemsLocked::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsLocked *>(dest);
	if (!casted) return;

	lock.copy(casted->lock);
}

void MMGActionSceneItemsLocked::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &lock, &lock_params);
}

void MMGActionSceneItemsLocked::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	lock_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (lock_params.options.testFlag(OPTION_HIDDEN)) return;

	lock_params.default_value = !obs_sceneitem_locked(obs_sceneitem);
}

void MMGActionSceneItemsLocked::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	bool locked = obs_sceneitem_locked(obs_sceneitem);
	ACTION_ASSERT(test.applicable(lock, locked),
		      "A lock could not be selected. Check the Lock field and try again.");
	obs_sceneitem_set_locked(obs_sceneitem, locked);
}

void MMGActionSceneItemsLocked::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	test.addAcceptable(lock, obs_sceneitem_locked(obs_sceneitem));
}
// End MMGActionSceneItemsLocked

// MMGActionSceneItemsPosition
MMGParams<float> MMGActionSceneItemsPosition::pos_x_params {
	.desc = obstr("Basic.TransformWindow.PositionX"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
		   OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = -90001.0,
	.upper_bound = 90001.0,
	.step = 0.1,
	.incremental_bound = 5000.0,
};

MMGParams<float> MMGActionSceneItemsPosition::pos_y_params {
	.desc = obstr("Basic.TransformWindow.PositionY"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
		   OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = -90001.0,
	.upper_bound = 90001.0,
	.step = 0.1,
	.incremental_bound = 5000.0,
};

MMGActionSceneItemsPosition::MMGActionSceneItemsPosition(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  pos_x(json_obj, "x"),
	  pos_y(json_obj, "y")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsPosition::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(pos_x, json_obj, "num1", 1);
	MMGCompatibility::initOldNumberData(pos_y, json_obj, "num2", 2);
}

void MMGActionSceneItemsPosition::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	pos_x->json(json_obj, "x");
	pos_y->json(json_obj, "y");
}

void MMGActionSceneItemsPosition::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsPosition *>(dest);
	if (!casted) return;

	pos_x.copy(casted->pos_x);
	pos_y.copy(casted->pos_y);
}

void MMGActionSceneItemsPosition::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &pos_x, &pos_x_params);
	MMGActions::createActionField(display, &pos_y, &pos_y_params);
}

void MMGActionSceneItemsPosition::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	pos_x_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	pos_y_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	obs_sceneitem_get_pos(obs_sceneitem, &current_pos);
	pos_x_params.default_value = current_pos.x;
	pos_y_params.default_value = current_pos.y;
}

void MMGActionSceneItemsPosition::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	obs_sceneitem_get_pos(obs_sceneitem, &current_pos);
	ACTION_ASSERT(test.applicable(pos_x, current_pos.x),
		      "An x-value could not be selected. Check the Position X field "
		      "and try again.");
	ACTION_ASSERT(test.applicable(pos_y, current_pos.y),
		      "A y-value could not be selected. Check the Position Y field "
		      "and try again.");
	obs_sceneitem_set_pos(obs_sceneitem, &current_pos);
}

void MMGActionSceneItemsPosition::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	vec2 prev_pos = current_pos;
	obs_sceneitem_get_pos(obs_sceneitem, &current_pos);

	test.addCondition(current_pos != prev_pos);
	test.addAcceptable(pos_x, current_pos.x, current_pos.x != prev_pos.x);
	test.addAcceptable(pos_y, current_pos.y, current_pos.y != prev_pos.y);
}
// End MMGActionSceneItemsPosition

// MMGActionSceneItemsScale
MMGParams<float> MMGActionSceneItemsScale::scale_x_params {
	.desc = mmgtr("Actions.SceneItems.ScaleX"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
		   OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = -0.0,
	.upper_bound = 0.0,
	.step = 0.01,
	.incremental_bound = 1000.0,
};

MMGParams<float> MMGActionSceneItemsScale::scale_y_params {
	.desc = mmgtr("Actions.SceneItems.ScaleY"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
		   OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = -0.0,
	.upper_bound = 0.0,
	.step = 0.01,
	.incremental_bound = 1000.0,
};

MMGActionSceneItemsScale::MMGActionSceneItemsScale(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  scale_x(json_obj, "x"),
	  scale_y(json_obj, "y")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsScale::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(scale_x, json_obj, "num1", 1);
	MMGCompatibility::initOldNumberData(scale_y, json_obj, "num2", 2);
}

void MMGActionSceneItemsScale::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	scale_x->json(json_obj, "x");
	scale_y->json(json_obj, "y");
}

void MMGActionSceneItemsScale::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsScale *>(dest);
	if (!casted) return;

	scale_x.copy(casted->scale_x);
	scale_y.copy(casted->scale_y);
}

void MMGActionSceneItemsScale::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &scale_x, &scale_x_params);
	MMGActions::createActionField(display, &scale_y, &scale_y_params);
}

void MMGActionSceneItemsScale::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	scale_x_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	scale_y_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	obs_source_t *obs_source = obs_sceneitem_get_source(obs_sceneitem);
	obs_sceneitem_get_scale(obs_sceneitem, &current_scale);

	scale_x_params.lower_bound = -double(9000100u / obs_source_get_width(obs_source));
	scale_x_params.upper_bound = -scale_x_params.lower_bound;
	scale_x_params.default_value = current_scale.x * 100;
	scale_y_params.lower_bound = -double(9000100u / obs_source_get_height(obs_source));
	scale_y_params.upper_bound = -scale_y_params.lower_bound;
	scale_y_params.default_value = current_scale.y * 100;
}

void MMGActionSceneItemsScale::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	obs_sceneitem_get_scale(obs_sceneitem, &current_scale);
	current_scale *= 100.0;
	ACTION_ASSERT(test.applicable(scale_x, current_scale.x),
		      "An x-value could not be selected. Check the Scale X field and "
		      "try again.");
	ACTION_ASSERT(test.applicable(scale_y, current_scale.y),
		      "A y-value could not be selected. Check the Scale Y field and "
		      "try again.");
	current_scale /= 100.0;
	obs_sceneitem_set_scale(obs_sceneitem, &current_scale);
}

void MMGActionSceneItemsScale::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	vec2 prev_scale = current_scale;
	obs_sceneitem_get_scale(obs_sceneitem, &current_scale);

	test.addCondition(current_scale != prev_scale);
	test.addAcceptable(scale_x, current_scale.x * 100.0, current_scale.x != prev_scale.x);
	test.addAcceptable(scale_y, current_scale.y * 100.0, current_scale.y != prev_scale.y);
}
// End MMGActionSceneItemsScale

// MMGActionSceneItemsRotation
MMGParams<float> MMGActionSceneItemsRotation::rotation_params {
	.desc = obstr("Basic.TransformWindow.Rotation"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = -360.0,
	.upper_bound = 360.0,
	.step = 0.01,
	.incremental_bound = 180.0,
};

MMGActionSceneItemsRotation::MMGActionSceneItemsRotation(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  rot(json_obj, "rot")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsRotation::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(rot, json_obj, "num1", 1);
}

void MMGActionSceneItemsRotation::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	rot->json(json_obj, "rot");
}

void MMGActionSceneItemsRotation::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsRotation *>(dest);
	if (!casted) return;

	rot.copy(casted->rot);
}

void MMGActionSceneItemsRotation::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &rot, &rotation_params);
}

void MMGActionSceneItemsRotation::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	rotation_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	current_rot = obs_sceneitem_get_rot(obs_sceneitem);
	rotation_params.default_value = current_rot;
}

void MMGActionSceneItemsRotation::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	float rotation = obs_sceneitem_get_rot(obs_sceneitem);
	ACTION_ASSERT(test.applicable(rot, rotation), "A rotation could not be selected. Check the Rotation field "
						      "and try again.");
	obs_sceneitem_set_rot(obs_sceneitem, rotation);
}

void MMGActionSceneItemsRotation::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	float prev_rot = current_rot;
	current_rot = obs_sceneitem_get_rot(obs_sceneitem);

	test.addCondition(current_rot != prev_rot);
	test.addAcceptable(rot, current_rot);
}
// End MMGActionSceneItemsRotation

// MMGActionSceneItemsCrop
MMGParams<int32_t> MMGActionSceneItemsCrop::crop_params[4] {
	{
		// left
		.desc = obstr("Basic.TransformWindow.CropLeft"),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
			   OPTION_ALLOW_INCREMENT,
		.default_value = 0,

		.lower_bound = 0.0,
		.upper_bound = 0.0,
		.step = 1.0,
		.incremental_bound = 0.0,
	},
	{
		// top
		.desc = obstr("Basic.TransformWindow.CropTop"),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
			   OPTION_ALLOW_INCREMENT,
		.default_value = 0,

		.lower_bound = 0.0,
		.upper_bound = 0.0,
		.step = 1.0,
		.incremental_bound = 0.0,
	},
	{
		// right
		.desc = obstr("Basic.TransformWindow.CropRight"),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
			   OPTION_ALLOW_INCREMENT,
		.default_value = 0,

		.lower_bound = 0.0,
		.upper_bound = 0.0,
		.step = 1.0,
		.incremental_bound = 0.0,
	},
	{
		// bottom
		.desc = obstr("Basic.TransformWindow.CropBottom"),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
			   OPTION_ALLOW_INCREMENT,
		.default_value = 0,

		.lower_bound = 0.0,
		.upper_bound = 0.0,
		.step = 1.0,
		.incremental_bound = 0.0,
	},
};

MMGActionSceneItemsCrop::MMGActionSceneItemsCrop(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  crop_l(json_obj, "left"),
	  crop_t(json_obj, "top"),
	  crop_r(json_obj, "right"),
	  crop_b(json_obj, "bottom")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsCrop::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(crop_t, json_obj, "num1", 1);
	MMGCompatibility::initOldNumberData(crop_r, json_obj, "num2", 2);
	MMGCompatibility::initOldNumberData(crop_b, json_obj, "num3", 3);
	MMGCompatibility::initOldNumberData(crop_l, json_obj, "num4", 4);
}

void MMGActionSceneItemsCrop::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	crop_l->json(json_obj, "left");
	crop_t->json(json_obj, "top");
	crop_r->json(json_obj, "right");
	crop_b->json(json_obj, "bottom");
}

void MMGActionSceneItemsCrop::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsCrop *>(dest);
	if (!casted) return;

	crop_l.copy(casted->crop_l);
	crop_t.copy(casted->crop_t);
	crop_r.copy(casted->crop_r);
	crop_b.copy(casted->crop_b);
}

void MMGActionSceneItemsCrop::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &crop_l, &crop_params[0]);
	MMGActions::createActionField(display, &crop_t, &crop_params[1]);
	MMGActions::createActionField(display, &crop_r, &crop_params[2]);
	MMGActions::createActionField(display, &crop_b, &crop_params[3]);
}

void MMGActionSceneItemsCrop::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	crop_params[0].options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	crop_params[1].options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	crop_params[2].options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	crop_params[3].options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	obs_source_t *obs_source = obs_sceneitem_get_source(obs_sceneitem);
	obs_sceneitem_get_crop(obs_sceneitem, &current_crop);

	crop_params[0].upper_bound = obs_source_get_width(obs_source);
	crop_params[0].default_value = current_crop.left;
	crop_params[0].incremental_bound = crop_params[0].upper_bound / 2.0;

	crop_params[1].upper_bound = obs_source_get_height(obs_source);
	crop_params[1].default_value = current_crop.top;
	crop_params[1].incremental_bound = crop_params[1].upper_bound / 2.0;

	crop_params[2].upper_bound = obs_source_get_width(obs_source);
	crop_params[2].default_value = current_crop.right;
	crop_params[2].incremental_bound = crop_params[2].upper_bound / 2.0;

	crop_params[3].upper_bound = obs_source_get_height(obs_source);
	crop_params[3].default_value = current_crop.bottom;
	crop_params[3].incremental_bound = crop_params[3].upper_bound / 2.0;
}

void MMGActionSceneItemsCrop::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	obs_sceneitem_get_crop(obs_sceneitem, &current_crop);
	ACTION_ASSERT(test.applicable(crop_l, current_crop.left),
		      "A crop element could not be selected. Check the Crop Left and "
		      "try again.");
	ACTION_ASSERT(test.applicable(crop_t, current_crop.top),
		      "A crop element could not be selected. Check the Crop Top and "
		      "try again.");
	ACTION_ASSERT(test.applicable(crop_r, current_crop.right),
		      "A crop element could not be selected. Check the Crop Right "
		      "and try again.");
	ACTION_ASSERT(test.applicable(crop_b, current_crop.bottom),
		      "A crop element could not be selected. Check the Crop Bottom "
		      "and try again.");
	obs_sceneitem_set_crop(obs_sceneitem, &current_crop);
}

void MMGActionSceneItemsCrop::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	obs_sceneitem_crop prev_crop = current_crop;
	obs_sceneitem_get_crop(obs_sceneitem, &current_crop);

	test.addCondition(current_crop != prev_crop);
	test.addAcceptable(crop_l, current_crop.left, current_crop.left != prev_crop.left);
	test.addAcceptable(crop_t, current_crop.top, current_crop.top != prev_crop.top);
	test.addAcceptable(crop_r, current_crop.right, current_crop.right != prev_crop.right);
	test.addAcceptable(crop_b, current_crop.bottom, current_crop.bottom != prev_crop.bottom);
}
// End MMGActionSceneItemsCrop

// MMGActionSceneItemsAlignment
MMGParams<Alignment> MMGActionSceneItemsAlignment::alignment_params {
	.desc = obstr("Basic.TransformWindow.Alignment"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = OBS_ALIGN_CENTER_CENTER,
	.bounds =
		{
			{OBS_ALIGN_TOP_LEFT, obstr("Basic.TransformWindow.Alignment.TopLeft")},
			{OBS_ALIGN_TOP_CENTER, obstr("Basic.TransformWindow.Alignment.TopCenter")},
			{OBS_ALIGN_TOP_RIGHT, obstr("Basic.TransformWindow.Alignment.TopRight")},
			{OBS_ALIGN_CENTER_LEFT, obstr("Basic.TransformWindow.Alignment.CenterLeft")},
			{OBS_ALIGN_CENTER_CENTER, obstr("Basic.TransformWindow.Alignment.Center")},
			{OBS_ALIGN_CENTER_RIGHT, obstr("Basic.TransformWindow.Alignment.CenterRight")},
			{OBS_ALIGN_BOTTOM_LEFT, obstr("Basic.TransformWindow.Alignment.BottomLeft")},
			{OBS_ALIGN_BOTTOM_CENTER, obstr("Basic.TransformWindow.Alignment.BottomCenter")},
			{OBS_ALIGN_BOTTOM_RIGHT, obstr("Basic.TransformWindow.Alignment.BottomRight")},
		},
};

MMGActionSceneItemsAlignment::MMGActionSceneItemsAlignment(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  alignment(json_obj, "alignment")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsAlignment::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldStringData(alignment, json_obj, "action", 3, alignment_params.bounds);
}

void MMGActionSceneItemsAlignment::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	alignment->json(json_obj, "alignment");
}

void MMGActionSceneItemsAlignment::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsAlignment *>(dest);
	if (!casted) return;

	alignment.copy(casted->alignment);
}

void MMGActionSceneItemsAlignment::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &alignment, &alignment_params);
}

void MMGActionSceneItemsAlignment::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	alignment_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	current_alignment = (Alignment)obs_sceneitem_get_alignment(obs_sceneitem);
	alignment_params.default_value = current_alignment;
}

void MMGActionSceneItemsAlignment::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	Alignment align = Alignment(obs_sceneitem_get_alignment(obs_sceneitem));
	ACTION_ASSERT(test.applicable(alignment, align),
		      "An alignment could not be selected. Check the Alignment field "
		      "and try again.");
	obs_sceneitem_set_alignment(obs_sceneitem, align);
}

void MMGActionSceneItemsAlignment::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	Alignment prev_align = current_alignment;
	current_alignment = (Alignment)obs_sceneitem_get_alignment(obs_sceneitem);

	test.addCondition(current_alignment != prev_align);
	test.addAcceptable(alignment, current_alignment);
}
// End MMGActionSceneItemsAlignment

// MMGActionSceneItemsScaleFilter
MMGParams<obs_scale_type> MMGActionSceneItemsScaleFilter::scale_filter_params {
	.desc = obstr("ScaleFiltering"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = OBS_SCALE_DISABLE,
	.bounds =
		{
			{OBS_SCALE_DISABLE, obstr("Disable")},
			{OBS_SCALE_POINT, obstr("ScaleFiltering.Point")},
			{OBS_SCALE_BICUBIC, obstr("ScaleFiltering.Bicubic")},
			{OBS_SCALE_BILINEAR, obstr("ScaleFiltering.Bilinear")},
			{OBS_SCALE_LANCZOS, obstr("ScaleFiltering.Lanczos")},
			{OBS_SCALE_AREA, obstr("ScaleFiltering.Area")},
		},
};

MMGActionSceneItemsScaleFilter::MMGActionSceneItemsScaleFilter(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  scale_filter(json_obj, "scale_filter")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsScaleFilter::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldStringData(scale_filter, json_obj, "action", 3, scale_filter_params.bounds);
}

void MMGActionSceneItemsScaleFilter::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	scale_filter->json(json_obj, "scale_filter");
}

void MMGActionSceneItemsScaleFilter::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsScaleFilter *>(dest);
	if (!casted) return;

	scale_filter.copy(casted->scale_filter);
}

void MMGActionSceneItemsScaleFilter::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &scale_filter, &scale_filter_params);
}

void MMGActionSceneItemsScaleFilter::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	scale_filter_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	// OBS function written poorly
	current_scale_filter = obs_sceneitem_get_scale_filter(const_cast<obs_sceneitem_t *>(obs_sceneitem));
	scale_filter_params.default_value = current_scale_filter;
}

void MMGActionSceneItemsScaleFilter::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	obs_scale_type _current_scale_filter = obs_sceneitem_get_scale_filter(obs_sceneitem);
	ACTION_ASSERT(test.applicable(scale_filter, _current_scale_filter),
		      "A scale filter could not be selected. Check the Scale Filter "
		      "field and try again.");
	obs_sceneitem_set_scale_filter(obs_sceneitem, _current_scale_filter);
}

void MMGActionSceneItemsScaleFilter::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	uint32_t prev_scale_filter = current_scale_filter;
	// OBS function written poorly
	current_scale_filter = obs_sceneitem_get_scale_filter(const_cast<obs_sceneitem_t *>(obs_sceneitem));

	test.addCondition(current_scale_filter != prev_scale_filter);
	test.addAcceptable(scale_filter, current_scale_filter);
}
// End MMGActionSceneItemsScaleFilter

// MMGActionSceneItemsBlendingMode
MMGParams<obs_blending_type> MMGActionSceneItemsBlendingMode::blending_mode_params {
	.desc = obstr("BlendingMode"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = OBS_BLEND_NORMAL,
	.bounds =
		{
			{OBS_BLEND_NORMAL, obstr("BlendingMode.Normal")},
			{OBS_BLEND_ADDITIVE, obstr("BlendingMode.Additive")},
			{OBS_BLEND_SUBTRACT, obstr("BlendingMode.Subtract")},
			{OBS_BLEND_SCREEN, obstr("BlendingMode.Screen")},
			{OBS_BLEND_MULTIPLY, obstr("BlendingMode.Multiply")},
			{OBS_BLEND_LIGHTEN, obstr("BlendingMode.Lighten")},
			{OBS_BLEND_DARKEN, obstr("BlendingMode.Darken")},
		},
};

MMGActionSceneItemsBlendingMode::MMGActionSceneItemsBlendingMode(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  blending_mode(json_obj, "blending_mode")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsBlendingMode::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldStringData(blending_mode, json_obj, "action", 3, blending_mode_params.bounds);
}

void MMGActionSceneItemsBlendingMode::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	blending_mode->json(json_obj, "blending_mode");
}

void MMGActionSceneItemsBlendingMode::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsBlendingMode *>(dest);
	if (!casted) return;

	blending_mode.copy(casted->blending_mode);
}

void MMGActionSceneItemsBlendingMode::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &blending_mode, &blending_mode_params);
}

void MMGActionSceneItemsBlendingMode::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	blending_mode_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	// OBS function written poorly
	current_blending_mode = obs_sceneitem_get_blending_mode(const_cast<obs_sceneitem_t *>(obs_sceneitem));
	blending_mode_params.default_value = current_blending_mode;
}

void MMGActionSceneItemsBlendingMode::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	obs_blending_type blend_mode = obs_sceneitem_get_blending_mode(obs_sceneitem);
	ACTION_ASSERT(test.applicable(blending_mode, blend_mode),
		      "A blending mode could not be selected. Check the Blending "
		      "Mode field and try again.");
	obs_sceneitem_set_blending_mode(obs_sceneitem, blend_mode);
}

void MMGActionSceneItemsBlendingMode::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	uint32_t prev_blending_mode = current_blending_mode;
	// OBS function written poorly
	current_blending_mode = obs_sceneitem_get_blending_mode(const_cast<obs_sceneitem_t *>(obs_sceneitem));

	test.addCondition(current_blending_mode != prev_blending_mode);
	test.addAcceptable(blending_mode, current_blending_mode);
}
// End MMGActionSceneItemsBlendingMode

// MMGActionSceneItemsBoundingBoxType
MMGParams<obs_bounds_type> MMGActionSceneItemsBoundingBoxType::bounds_type_params {
	.desc = obstr("Basic.TransformWindow.BoundsType"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = OBS_BOUNDS_NONE,
	.bounds =
		{
			{OBS_BOUNDS_NONE, obstr("Basic.TransformWindow.BoundsType.None")},
			{OBS_BOUNDS_STRETCH, obstr("Basic.TransformWindow.BoundsType.Stretch")},
			{OBS_BOUNDS_SCALE_INNER, obstr("Basic.TransformWindow.BoundsType.ScaleInner")},
			{OBS_BOUNDS_SCALE_OUTER, obstr("Basic.TransformWindow.BoundsType.ScaleOuter")},
			{OBS_BOUNDS_SCALE_TO_WIDTH, obstr("Basic.TransformWindow.BoundsType.ScaleToWidth")},
			{OBS_BOUNDS_SCALE_TO_HEIGHT, obstr("Basic.TransformWindow.BoundsType.ScaleToHeight")},
			{OBS_BOUNDS_MAX_ONLY, obstr("Basic.TransformWindow.BoundsType.MaxOnly")},
		},
};

MMGActionSceneItemsBoundingBoxType::MMGActionSceneItemsBoundingBoxType(MMGActionManager *parent,
								       const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  bounds_type(json_obj, "bounds_type")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsBoundingBoxType::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldStringData(bounds_type, json_obj, "action", 3, bounds_type_params.bounds);
}

void MMGActionSceneItemsBoundingBoxType::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	bounds_type->json(json_obj, "bounds_type");
}

void MMGActionSceneItemsBoundingBoxType::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsBoundingBoxType *>(dest);
	if (!casted) return;

	bounds_type.copy(casted->bounds_type);
}

void MMGActionSceneItemsBoundingBoxType::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &bounds_type, &bounds_type_params);
}

void MMGActionSceneItemsBoundingBoxType::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	bounds_type_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	current_bounds = obs_sceneitem_get_bounds_type(obs_sceneitem);
	bounds_type_params.default_value = current_bounds;
}

void MMGActionSceneItemsBoundingBoxType::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	obs_bounds_type current_bounds_type = obs_sceneitem_get_bounds_type(obs_sceneitem);
	ACTION_ASSERT(test.applicable(bounds_type, current_bounds_type),
		      "A bounding box type could not be selected. Check the Type "
		      "field and try again.");
	obs_sceneitem_set_bounds_type(obs_sceneitem, current_bounds_type);
}

void MMGActionSceneItemsBoundingBoxType::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	obs_bounds_type prev_bounds = current_bounds;
	current_bounds = obs_sceneitem_get_bounds_type(obs_sceneitem);

	test.addCondition(current_bounds != prev_bounds);
	test.addAcceptable(bounds_type, current_bounds);
}
// End MMGActionSceneItemsBoundingBoxType

// MMGActionSceneItemsBoundingBoxSize
MMGParams<float> MMGActionSceneItemsBoundingBoxSize::bounds_x_params {
	.desc = obstr("Basic.TransformWindow.BoundsWidth"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
		   OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = 0.0,
	.upper_bound = 90001.0,
	.step = 0.1,
	.incremental_bound = 5000.0,
};

MMGParams<float> MMGActionSceneItemsBoundingBoxSize::bounds_y_params {
	.desc = obstr("Basic.TransformWindow.BoundsHeight"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE |
		   OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = 0.0,
	.upper_bound = 90001.0,
	.step = 0.1,
	.incremental_bound = 5000.0,
};

MMGActionSceneItemsBoundingBoxSize::MMGActionSceneItemsBoundingBoxSize(MMGActionManager *parent,
								       const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  bounds_x(json_obj, "x"),
	  bounds_y(json_obj, "y")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsBoundingBoxSize::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(bounds_x, json_obj, "num1", 1);
	MMGCompatibility::initOldNumberData(bounds_y, json_obj, "num2", 2);
}

void MMGActionSceneItemsBoundingBoxSize::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	bounds_x->json(json_obj, "x");
	bounds_y->json(json_obj, "y");
}

void MMGActionSceneItemsBoundingBoxSize::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsBoundingBoxSize *>(dest);
	if (!casted) return;

	bounds_x.copy(casted->bounds_x);
	bounds_y.copy(casted->bounds_y);
}

void MMGActionSceneItemsBoundingBoxSize::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &bounds_x, &bounds_x_params);
	MMGActions::createActionField(display, &bounds_y, &bounds_y_params);
}

void MMGActionSceneItemsBoundingBoxSize::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	bounds_x_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	bounds_y_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	obs_sceneitem_get_bounds(obs_sceneitem, &current_bounds_size);
	bounds_x_params.default_value = current_bounds_size.x;
	bounds_y_params.default_value = current_bounds_size.y;
}

void MMGActionSceneItemsBoundingBoxSize::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	obs_sceneitem_get_bounds(obs_sceneitem, &current_bounds_size);
	ACTION_ASSERT(test.applicable(bounds_x, current_bounds_size.x),
		      "An x-value could not be selected. Check the Bounds X field "
		      "and try again.");
	ACTION_ASSERT(test.applicable(bounds_y, current_bounds_size.y),
		      "A y-value could not be selected. Check the Bounds Y field and "
		      "try again.");
	obs_sceneitem_set_bounds(obs_sceneitem, &current_bounds_size);
}

void MMGActionSceneItemsBoundingBoxSize::processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const
{
	vec2 prev_bounds_size = current_bounds_size;
	obs_sceneitem_get_bounds(obs_sceneitem, &current_bounds_size);

	test.addCondition(current_bounds_size != prev_bounds_size);
	test.addAcceptable(bounds_x, current_bounds_size.x, current_bounds_size.x != prev_bounds_size.x);
	test.addAcceptable(bounds_y, current_bounds_size.y, current_bounds_size.y != prev_bounds_size.y);
}
// End MMGActionSceneItemsBoundingBoxSize

// MMGActionSceneItemsBoundingBoxAlignment
MMGActionSceneItemsBoundingBoxAlignment::MMGActionSceneItemsBoundingBoxAlignment(MMGActionManager *parent,
										 const QJsonObject &json_obj)
	: MMGActionSceneItems(parent, json_obj),
	  alignment(json_obj, "alignment")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSceneItemsBoundingBoxAlignment::initOldData(const QJsonObject &json_obj)
{
	MMGActionSceneItems::initOldData(json_obj);

	MMGCompatibility::initOldStringData(alignment, json_obj, "action", 3,
					    MMGActionSceneItemsAlignment::alignment_params.bounds);
}

void MMGActionSceneItemsBoundingBoxAlignment::json(QJsonObject &json_obj) const
{
	MMGActionSceneItems::json(json_obj);

	alignment->json(json_obj, "alignment");
}

void MMGActionSceneItemsBoundingBoxAlignment::copy(MMGAction *dest) const
{
	MMGActionSceneItems::copy(dest);

	auto casted = dynamic_cast<MMGActionSceneItemsBoundingBoxAlignment *>(dest);
	if (!casted) return;

	alignment.copy(casted->alignment);
}

void MMGActionSceneItemsBoundingBoxAlignment::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSceneItems::createDisplay(display);

	MMGActions::createActionField(display, &alignment, &MMGActionSceneItemsAlignment::alignment_params);
}

void MMGActionSceneItemsBoundingBoxAlignment::onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const
{
	MMGActionSceneItemsAlignment::alignment_params.options.setFlag(OPTION_HIDDEN, !obs_sceneitem);
	if (!obs_sceneitem) return;

	current_alignment = (Alignment)obs_sceneitem_get_bounds_alignment(obs_sceneitem);
	MMGActionSceneItemsAlignment::alignment_params.default_value = current_alignment;
}

void MMGActionSceneItemsBoundingBoxAlignment::execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const
{
	Alignment align = Alignment(obs_sceneitem_get_bounds_alignment(obs_sceneitem));
	ACTION_ASSERT(test.applicable(alignment, align),
		      "An alignment could not be selected. Check the Alignment field "
		      "and try again.");
	obs_sceneitem_set_bounds_alignment(obs_sceneitem, align);
}

void MMGActionSceneItemsBoundingBoxAlignment::processEvent(MMGMappingTest &test,
							   const obs_sceneitem_t *obs_sceneitem) const
{
	Alignment prev_bounds_align = current_alignment;
	current_alignment = (Alignment)obs_sceneitem_get_bounds_alignment(obs_sceneitem);

	test.addCondition(current_alignment != prev_bounds_align);
	test.addAcceptable(alignment, current_alignment);
}
// End MMGActionSceneItemsBoundingBoxAlignment

} // namespace MMGActions
