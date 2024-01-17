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

MMGActionScenes::MMGActionScenes(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj), scene(json_obj, "scene", 1)
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionScenes::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	scene.json(json_obj, "scene");
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
	scene.setEditable(edit);
}

void MMGActionScenes::toggle()
{
	scene.toggle();
}

void MMGActionScenes::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	MMGStringDisplay *scene_display = display()->stringDisplays()->addNew(&scene);
	scene_display->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
}

void MMGActionScenes::setActionParams()
{
	MMGStringDisplay *scene_display = display()->stringDisplays()->fieldAt(0);
	scene_display->setVisible(true);
	scene_display->setDescription(obstr("Basic.Scene"));
	scene_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
	scene_display->setBounds(enumerate());
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

const QStringList MMGActionScenes::enumerateItems(const QString &scene)
{
	QStringList list;
	obs_scene_t *obs_scene = obs_scene_from_source(OBSSourceAutoRelease(obs_get_source_by_name(scene.qtocs())));

	obs_scene_enum_items(obs_scene, enumerateItems, &list);
	return list;
}

bool MMGActionScenes::enumerateItems(obs_scene_t *, obs_sceneitem_t *item, void *param)
{
	auto _list = reinterpret_cast<QStringList *>(param);

	_list->prepend(obs_source_get_name(obs_sceneitem_get_source(item)));
	if (obs_sceneitem_is_group(item)) obs_sceneitem_group_enum_items(item, enumerateItems, param);

	return true;
}

const QString MMGActionScenes::currentScene(bool preview)
{
	return obs_source_get_name(OBSSourceAutoRelease(preview ? obs_frontend_get_current_preview_scene()
								: obs_frontend_get_current_scene()));
}

void MMGActionScenes::execute(const MMGMessage *midi) const
{
	const QStringList scenes = enumerate();

	OBSSourceAutoRelease source_obs_scene = obs_get_source_by_name(scene.chooseFrom(midi, scenes).qtocs());

	ACTION_ASSERT(source_obs_scene, "Scene does not exist.");

	switch (sub()) {
		case SCENE_CHANGE:
			obs_frontend_set_current_scene(source_obs_scene);
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionScenes::connectOBSSignals()
{
	disconnectOBSSignals();
	connect(mmgsignals(), &MMGSignals::frontendEvent, this, &MMGActionScenes::frontendCallback);
}

void MMGActionScenes::disconnectOBSSignals()
{
	disconnect(mmgsignals(), &MMGSignals::frontendEvent, this, nullptr);
}

void MMGActionScenes::frontendCallback(obs_frontend_event event) const
{
	if (event != OBS_FRONTEND_EVENT_SCENE_CHANGED) return;

	MMGNumber values;
	QStringList scenes = enumerate();

	switch (sub()) {
		case SCENE_CHANGED:
			values = scenes.indexOf(currentScene());
			if (scene.chooseTo(values, scenes)) return;
			break;

		default:
			return;
	}

	emit eventTriggered({values});
}
