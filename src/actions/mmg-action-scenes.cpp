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

#include "mmg-action-scenes.h"

namespace MMGActions {

static MMGParams<MMGString> scene_params {
	.desc = obstr("Basic.Scene"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = "",
	.bounds = {},
	.placeholder = mmgtr("Actions.Scenes.Placeholder"),
};

const MMGParams<MMGString> &sceneParams(char type)
{
	scene_params.options.setFlag(OPTION_ALLOW_MIDI, type < 2);
	scene_params.options.setFlag(OPTION_ALLOW_TOGGLE, type < 2);

	scene_params.bounds = enumerateScenes();
	scene_params.default_value = currentScene(type == 1 && obs_frontend_preview_program_mode_active());

	return scene_params;
};

const MMGStringTranslationMap enumerateScenes()
{
	MMGStringTranslationMap list;

	char **scene_names = obs_frontend_get_scene_names();
	for (int i = 0; scene_names[i] != 0; ++i) {
		OBSSourceAutoRelease obs_source = obs_get_source_by_name(scene_names[i]);
		list.insert(obs_source_get_uuid(obs_source), nontr(scene_names[i]));
	}
	bfree(scene_names);

	return list;
}

struct SceneItemEnumerator {
	MMGStringTranslationMap names;
	uint64_t bounds;
};

static bool enumerateThruSceneItems(obs_scene_t *, obs_sceneitem_t *item, void *param)
{
	auto _enumerator = reinterpret_cast<SceneItemEnumerator *>(param);

	obs_source_t *obs_source = obs_sceneitem_get_source(item);
	if (_enumerator->bounds == OBS_SOURCE_VIDEO) {
		if (obs_source_get_width(obs_source) == 0) return true;
		if (obs_source_get_height(obs_source) == 0) return true;
	}

	_enumerator->names.insert(obs_source_get_uuid(obs_source), nontr(obs_source_get_name(obs_source)));
	if (obs_sceneitem_is_group(item)) obs_sceneitem_group_enum_items(item, enumerateThruSceneItems, param);

	return true;
}

const MMGStringTranslationMap enumerateSceneItems(const MMGString &scene_uuid, uint64_t bounds)
{
	SceneItemEnumerator enumerator;
	enumerator.bounds = bounds;

	obs_scene_t *obs_scene = obs_scene_from_source(OBSSourceAutoRelease(obs_get_source_by_uuid(scene_uuid)));
	if (!obs_scene) return enumerator.names;

	obs_scene_enum_items(obs_scene, enumerateThruSceneItems, &enumerator);
	return enumerator.names;
}

MMGString currentScene(bool preview)
{
	return MMGString(obs_source_get_uuid(OBSSourceAutoRelease(preview ? obs_frontend_get_current_preview_scene()
									  : obs_frontend_get_current_scene())));
}

// MMGActionScenesSwitch
const MMGParams<bool> MMGActionScenesSwitch::preview_params {
	.desc = mmgtr("Actions.Scenes.UsePreview"),
	.options = OPTION_NONE,
	.default_value = false,
};

MMGActionScenesSwitch::MMGActionScenesSwitch(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  use_preview(json_obj, "use_preview"),
	  scene(json_obj, "scene")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionScenesSwitch::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(scene, json_obj, "scene", 1, enumerateScenes());
	use_preview = json_obj["sub"].toInt() == 4;
}

void MMGActionScenesSwitch::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	use_preview->json(json_obj, "use_preview");
	scene->json(json_obj, "scene");
}

void MMGActionScenesSwitch::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionScenesSwitch *>(dest);
	if (!casted) return;

	use_preview.copy(casted->use_preview);
	scene.copy(casted->scene);
}

void MMGActionScenesSwitch::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	if (type() != TYPE_OUTPUT) MMGActions::createActionField(display, &use_preview, &preview_params);
	MMGActions::createActionField(display, &scene, &sceneParams());
}

void MMGActionScenesSwitch::execute(const MMGMappingTest &test) const
{
	runInMainThread([this, test]() {
		MMGString scene_name;

		if (use_preview && obs_frontend_preview_program_mode_active()) {
			scene_name = currentScene(true);
		} else {
			ACTION_ASSERT(test.applicable(scene, scene_name),
				      "A scene could not be selected. Check the Scene Field and "
				      "try again.");
		}

		OBSSourceAutoRelease source_obs_scene = obs_get_source_by_uuid(scene_name);
		ACTION_ASSERT(source_obs_scene, "This scene does not exist.")

		obs_frontend_set_current_scene(source_obs_scene);
	});

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionScenesSwitch::processEvent(obs_frontend_event event) const
{
	if (event != OBS_FRONTEND_EVENT_SCENE_CHANGED) return;

	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(scene, currentScene(false));
}
// End MMGActionScenesSwitch

// MMGActionScenesScreenshot
MMGActionScenesScreenshot::MMGActionScenesScreenshot(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj)
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionScenesScreenshot::execute(const MMGMappingTest &) const
{
	obs_frontend_take_screenshot();
	blog(LOG_DEBUG, "Successfully executed.");
}
// End MMGActionScenesScreenshot

} // namespace MMGActions
