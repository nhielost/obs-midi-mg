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

#include "mmg-action-collections.h"

namespace MMGActions {

const MMGStringTranslationMap enumerateCollections()
{
	MMGStringTranslationMap map;

	char **collection_names = obs_frontend_get_scene_collections();
	for (int i = 0; collection_names[i] != 0; ++i) {
		map.insert(collection_names[i], nontr(collection_names[i]));
	}
	bfree(collection_names);

	return map;
}

const MMGString currentCollection()
{
	char *char_collection = obs_frontend_get_current_scene_collection();
	MMGString collection_text(char_collection);
	bfree(char_collection);
	return collection_text;
}

MMGParams<MMGString> MMGActionCollections::collection_params {
	.desc = obstr("Importer.Collection"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = "",
	.bounds = {},
};

MMGActionCollections::MMGActionCollections(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  collection(json_obj, "collection")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionCollections::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(collection, json_obj, "collection", 1, enumerateCollections());
}

void MMGActionCollections::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	collection->json(json_obj, "collection");
}

void MMGActionCollections::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionCollections *>(dest);
	if (!casted) return;

	collection.copy(casted->collection);
}

void MMGActionCollections::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	collection_params.bounds = enumerateCollections();
	collection_params.default_value = currentCollection();

	MMGActions::createActionField(display, &collection, &collection_params);
}

void MMGActionCollections::execute(const MMGMappingTest &test) const
{
	MMGString name = currentCollection();
	ACTION_ASSERT(test.applicable(collection, name), "A scene collection could not be selected. Check the Scene "
							 "Collection field and try again.");
	ACTION_ASSERT(name != currentCollection(), "The scene collection is already active.");

	runInMainThread([name]() { obs_frontend_set_current_scene_collection(name); });

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionCollections::processEvent(obs_frontend_event event) const
{
	if (event != OBS_FRONTEND_EVENT_SCENE_COLLECTION_CHANGED) return;

	EventFulfillment fulfillment(this);
	fulfillment->addAcceptable(collection, currentCollection());
}

} // namespace MMGActions
