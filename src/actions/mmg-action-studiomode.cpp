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

#include "mmg-action-studiomode.h"
#include "mmg-action-scenes.h"

using namespace MMGUtils;

MMGActionStudioMode::MMGActionStudioMode(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  scene(json_obj, "scene", 1)
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionStudioMode::subNames() const
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
		default:
			opts << subModuleTextList(
				{"Activate", "Deactivate", "ToggleActivate", "PreviewChange", "Transition"});
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"Activate", "Deactivate", "ToggleActivate", "PreviewChange"});
			break;
	}

	return opts;
}

void MMGActionStudioMode::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	scene.json(json_obj, "scene");
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
	scene.setEditable(edit);
}

void MMGActionStudioMode::toggle()
{
	scene.toggle();
}

void MMGActionStudioMode::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	display()->addNew(&scene)->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
}

void MMGActionStudioMode::setActionParams()
{
	MMGStringDisplay *scene_display = display()->stringDisplay(0);
	scene_display->setVisible(false);
	if (sub() == 3) {
		scene_display->setVisible(true);
		scene_display->setDescription(obstr("Basic.Scene"));
		scene_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
		scene_display->setBounds(MMGActionScenes::enumerate());
	}
}

void MMGActionStudioMode::execute(const MMGMessage *midi) const
{
	const QStringList scenes = MMGActionScenes::enumerate();

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

	OBSSourceAutoRelease source_obs_scene = obs_get_source_by_name(scene.chooseFrom(midi, scenes).qtocs());
	OBSSourceAutoRelease obs_preview_scene = obs_frontend_get_current_preview_scene();

	switch (sub()) {
		case STUDIOMODE_ON:
			set_studio_mode(true);
			break;

		case STUDIOMODE_OFF:
			set_studio_mode(false);
			break;

		case STUDIOMODE_TOGGLE_ONOFF:
			set_studio_mode(!obs_frontend_preview_program_mode_active());
			break;

		case STUDIOMODE_CHANGEPREVIEW:
			ACTION_ASSERT(source_obs_scene, "Scene does not exist.");
			obs_frontend_set_current_preview_scene(source_obs_scene);
			break;

		case STUDIOMODE_TRANSITION:
			ACTION_ASSERT(obs_preview_scene, "Either Studio Mode is disabled, "
							 "or the scene does not exist.");
			obs_frontend_set_current_scene(obs_preview_scene);
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionStudioMode::frontendEventReceived(obs_frontend_event event)
{
	MMGNumber values;
	QStringList scenes = MMGActionScenes::enumerate();

	switch (sub()) {
		case STUDIOMODE_ENABLED:
			if (event != OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED) return;
			break;

		case STUDIOMODE_DISABLED:
			if (event != OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED) return;
			break;

		case STUDIOMODE_TOGGLE_ENABLED:
			if (event != OBS_FRONTEND_EVENT_STUDIO_MODE_ENABLED &&
			    event != OBS_FRONTEND_EVENT_STUDIO_MODE_DISABLED)
				return;
			break;

		case STUDIOMODE_PREVIEWCHANGED:
			if (event != OBS_FRONTEND_EVENT_PREVIEW_SCENE_CHANGED ||
			    !obs_frontend_preview_program_mode_active())
				return;

			values = scenes.indexOf(MMGActionScenes::currentScene(true));
			if (scene.chooseTo(values, scenes)) return;
			break;

		default:
			return;
	}

	triggerEvent({values});
}
