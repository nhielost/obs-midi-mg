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

#include "mmg-action-collections.h"

using namespace MMGUtils;

MMGActionCollections::MMGActionCollections(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj), collection(json_obj, "collection", 1)
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionCollections::subNames() const
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
		default:
			opts << subModuleText("Switch");
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"Changing", "Changed", "Toggle"});
			break;
	}

	return opts;
}

void MMGActionCollections::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	collection.json(json_obj, "collection");
}

void MMGActionCollections::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionCollections *>(dest);
	if (!casted) return;

	casted->collection = collection.copy();
}

void MMGActionCollections::setEditable(bool edit)
{
	collection.setEditable(edit);
}

void MMGActionCollections::toggle()
{
	collection.toggle();
}

void MMGActionCollections::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	MMGStringDisplay *collection_display = display()->stringDisplays()->addNew(&collection);
	collection_display->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
}

void MMGActionCollections::setActionParams()
{
	MMGStringDisplay *collection_display = display()->stringDisplays()->fieldAt(0);
	collection_display->setVisible(true);
	collection_display->setDescription(obstr("Importer.Collection"));
	collection_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
	collection_display->setBounds(enumerate());
}

const QStringList MMGActionCollections::enumerate()
{
	QStringList list;

	char **collection_names = obs_frontend_get_scene_collections();
	for (int i = 0; collection_names[i] != 0; ++i) {
		list.append(collection_names[i]);
	}
	bfree(collection_names);

	return list;
}

const QString MMGActionCollections::currentCollection()
{
	char *char_collection = obs_frontend_get_current_scene_collection();
	QString q_collection = char_collection;
	bfree(char_collection);
	return q_collection;
}

void MMGActionCollections::execute(const MMGMessage *midi) const
{
	ACTION_ASSERT(sub() == COLLECTION_COLLECTION, "Invalid subcategory.");

	QString name = collection.chooseFrom(midi, enumerate());
	ACTION_ASSERT(name != currentCollection(), "The scene collection is already active.");

	obs_queue_task(
		OBS_TASK_UI,
		[](void *param) {
			auto collection_name = (QString *)param;
			obs_frontend_set_current_scene_collection(collection_name->qtocs());
		},
		&name, true);

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionCollections::frontendEventReceived(obs_frontend_event event)
{
	MMGNumber values;
	const QStringList collections = enumerate();

	switch (sub()) {
		case COLLECTION_CHANGING:
			if (event != OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING) return;
			values = collections.indexOf(currentCollection());
			break;

		case COLLECTION_CHANGED:
			if (event != OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED) return;
			values = collections.indexOf(currentCollection());
			break;

		case COLLECTION_TOGGLE_CHANGING:
			if (event != OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGING &&
			    event != OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED)
				return;
			break;

		default:
			return;
	}

	if (collection.chooseTo(values, collections)) return;
	triggerEvent({values});
}
