/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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

namespace MMGActions {

static void setStudioMode(bool active)
{
	if (obs_frontend_preview_program_mode_active() == active) return;
	runInMainThread([active]() { obs_frontend_set_preview_program_mode(active); });
}

// MMGActionStudioModeRunState
const MMGParams<bool> MMGActionStudioModeRunState::studio_params {
	.desc = mmgtr("Plugin.Status"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionStudioModeRunState::MMGActionStudioModeRunState(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  studio_state(json_obj, "studio_state")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionStudioModeRunState::initOldData(const QJsonObject &json_obj)
{
	int sub = json_obj["sub"].toInt();

	MMGCompatibility::initOldBooleanData(studio_state, sub);
}

void MMGActionStudioModeRunState::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	studio_state->json(json_obj, "studio_state");
}

void MMGActionStudioModeRunState::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionStudioModeRunState *>(dest);
	if (!casted) return;

	studio_state.copy(casted->studio_state);
}

void MMGActionStudioModeRunState::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &studio_state, &studio_params);
}

void MMGActionStudioModeRunState::execute(const MMGMappingTest &test) const
{
	bool value = obs_frontend_preview_program_mode_active();
	ACTION_ASSERT(test.applicable(studio_state, value),
		      "A status could not be selected. Check the Status Field and try again.");

	if (value && !obs_frontend_preview_program_mode_active())
		setStudioMode(true);
	else if (!value && obs_frontend_preview_program_mode_active())
		setStudioMode(false);

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionStudioModeRunState::processEvent(obs_frontend_event event) const
{
	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(studio_state, event, OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED,
				 OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED);
}
// End MMGActionStudioModeRunState

// MMGActionStudioModePreview
MMGActionStudioModePreview::MMGActionStudioModePreview(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  scene(json_obj, "scene")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionStudioModePreview::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(scene, json_obj, "scene", 1, enumerateScenes());
}

void MMGActionStudioModePreview::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	scene->json(json_obj, "scene");
}

void MMGActionStudioModePreview::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionStudioModePreview *>(dest);
	if (!casted) return;

	scene.copy(casted->scene);
}

void MMGActionStudioModePreview::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &scene, &sceneParams(1));
}

void MMGActionStudioModePreview::execute(const MMGMappingTest &test) const
{
	runInMainThread([this, test]() {
		MMGString scene_name;
		ACTION_ASSERT(test.applicable(scene, scene_name),
			      "A scene could not be selected. Check the Scene Field and try again.");
		OBSSourceAutoRelease obs_preview_scene = obs_get_source_by_uuid(scene_name);
		ACTION_ASSERT(obs_preview_scene, "Either Studio Mode is disabled, or the scene does not exist.");
		obs_frontend_set_current_preview_scene(obs_preview_scene);
	});

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionStudioModePreview::processEvent(obs_frontend_event event) const
{
	if (event != OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED || !obs_frontend_preview_program_mode_active()) return;

	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(scene, currentScene(true));
}
// End MMGActionStudioModePreview

} // namespace MMGActions
