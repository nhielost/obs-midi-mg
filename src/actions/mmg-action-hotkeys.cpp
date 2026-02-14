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

#include "mmg-action-hotkeys.h"

namespace MMGActions {

static const MMGString registerer(obs_hotkey_t *hotkey)
{
#define HOTKEY_REGISTERER_GET_NAME(kind)                                                     \
	ptr_##kind = obs_weak_##kind##_get_##kind(                                           \
		reinterpret_cast<obs_weak_##kind##_t *>(obs_hotkey_get_registerer(hotkey))); \
	if (!ptr_##kind) return "";                                                          \
	return obs_##kind##_get_name(ptr_##kind)

	OBSSourceAutoRelease ptr_source;
	OBSOutputAutoRelease ptr_output;
	OBSEncoderAutoRelease ptr_encoder;
	OBSServiceAutoRelease ptr_service;

	switch (obs_hotkey_get_registerer_type(hotkey)) {
		case OBS_HOTKEY_REGISTERER_FRONTEND:
			return mmgtr("Actions.Hotkeys.Frontend").translate();
		case OBS_HOTKEY_REGISTERER_SOURCE:
			HOTKEY_REGISTERER_GET_NAME(source);
		case OBS_HOTKEY_REGISTERER_OUTPUT:
			HOTKEY_REGISTERER_GET_NAME(output);
		case OBS_HOTKEY_REGISTERER_ENCODER:
			HOTKEY_REGISTERER_GET_NAME(encoder);
		case OBS_HOTKEY_REGISTERER_SERVICE:
			HOTKEY_REGISTERER_GET_NAME(service);
		default:
			return "";
	}

#undef HOTKEY_REGISTERER_GET_NAME
};

const MMGStringTranslationMap enumerateHotkeys(const MMGString &category)
{
	struct MMGHotkeyEnumeration {
		MMGString category;
		MMGStringTranslationMap list;
	} req {category};

	obs_enum_hotkeys(
		[](void *param, obs_hotkey_id, obs_hotkey_t *hotkey) {
			auto _req = reinterpret_cast<MMGHotkeyEnumeration *>(param);

			if (_req->category != registerer(hotkey)) return true;

			_req->list.insert(obs_hotkey_get_name(hotkey), nontr(obs_hotkey_get_description(hotkey)));
			return true;
		},
		&req);

	return req.list;
}

const MMGStringTranslationMap enumerateHotkeyCategories()
{
	MMGStringTranslationMap list;

	obs_enum_hotkeys(
		[](void *param, obs_hotkey_id, obs_hotkey_t *hotkey) {
			auto _list = reinterpret_cast<MMGStringTranslationMap *>(param);

			MMGString reg = registerer(hotkey);
			if (_list->contains(reg)) return true;

			_list->insert(reg, nontr(reg));
			return true;
		},
		&list);

	return list;
}

MMGParams<MMGString> MMGActionHotkeys::group_params {
	.desc = mmgtr("Actions.Hotkeys.Group"),
	.options = OPTION_NONE,
	.default_value = "",
	.bounds = {},
};

MMGParams<MMGString> MMGActionHotkeys::hotkey_params {
	.desc = mmgtr("Actions.Hotkeys.Hotkey"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = "",
	.bounds = {},
};

MMGActionHotkeys::MMGActionHotkeys(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  hotkey_group(json_obj, "hotkey_group"),
	  hotkey(json_obj, "hotkey")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionHotkeys::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(hotkey_group, json_obj, "hotkey_group", 0, enumerateHotkeyCategories());
	MMGCompatibility::initOldStringData(hotkey, json_obj, "hotkey", 1, enumerateHotkeys(hotkey_group));
}

void MMGActionHotkeys::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	hotkey_group->json(json_obj, "hotkey_group");
	hotkey->json(json_obj, "hotkey");
}

void MMGActionHotkeys::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionHotkeys *>(dest);
	if (!casted) return;

	hotkey_group.copy(casted->hotkey_group);
	hotkey.copy(casted->hotkey);
}

void MMGActionHotkeys::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	group_params.bounds = enumerateHotkeyCategories();
	group_params.default_value = !group_params.bounds.isEmpty() ? group_params.bounds.firstKey() : MMGString();

	MMGActions::createActionField(display, &hotkey_group, &group_params,
				      std::bind(&MMGActionHotkeys::onGroupChange, this));
	MMGActions::createActionField(display, &hotkey, &hotkey_params);
}

void MMGActionHotkeys::onGroupChange() const
{
	hotkey_params.options.setFlag(OPTION_HIDDEN, group_params.bounds.isEmpty());
	if (group_params.bounds.isEmpty()) return;

	hotkey_params.bounds = enumerateHotkeys(hotkey_group);
	hotkey_params.default_value = hotkey_params.bounds.firstKey();
}

void MMGActionHotkeys::execute(const MMGMappingTest &test) const
{
	hotkey_req.found = false;
	ACTION_ASSERT(test.applicable(hotkey, hotkey_req.name),
		      "A hotkey could not be selected. Check the Hotkey field and try again.");

	obs_enum_hotkeys(
		[](void *data, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			auto _req = reinterpret_cast<Request *>(data);

			if (obs_hotkey_get_name(hotkey) != _req->name) return true;

			_req->found = true;
			_req->id = id;
			return false;
		},
		&hotkey_req);

	ACTION_ASSERT(hotkey_req.found, "This hotkey does not exist.");

	runInMainThread([this]() {
		obs_hotkey_trigger_routed_callback(hotkey_req.id, false);
		obs_hotkey_trigger_routed_callback(hotkey_req.id, true);
		obs_hotkey_trigger_routed_callback(hotkey_req.id, false);
	});

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionHotkeys::processEvent(obs_hotkey_id id) const
{
	hotkey_req.found = false;
	hotkey_req.id = id;

	obs_enum_hotkeys(
		[](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			auto _req = reinterpret_cast<Request *>(param);

			if (id != _req->id) return true;

			_req->found = true;
			_req->name = obs_hotkey_get_name(hotkey);
			return false;
		},
		&hotkey_req);

	EventFulfillment fulfiller(this);
	fulfiller->addCondition(hotkey_req.found);
	fulfiller->addAcceptable(hotkey, hotkey_req.name);
}

} // namespace MMGActions
