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

#include "mmg-action-hotkeys.h"

using namespace MMGUtils;

struct MMGHotkeyRequest {
	obs_hotkey_id hotkey_id;
	QString hotkey_group;
	QString hotkey_name;
	bool hotkey_found = false;
};

MMGActionHotkeys::MMGActionHotkeys(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj), group(json_obj, "hotkey_group", 0), hotkey(json_obj, "hotkey", 1)
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionHotkeys::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	group.json(json_obj, "hotkey_group", false);
	hotkey.json(json_obj, "hotkey");
}

void MMGActionHotkeys::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionHotkeys *>(dest);
	if (!casted) return;

	casted->group = group.copy();
	casted->hotkey = hotkey.copy();
}

void MMGActionHotkeys::setEditable(bool edit)
{
	group.setEditable(edit);
	hotkey.setEditable(edit);
}

void MMGActionHotkeys::toggle()
{
	hotkey.toggle();
}

void MMGActionHotkeys::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	MMGStringDisplay *group_display = display()->stringDisplays()->addNew(&group);
	display()->connect(group_display, &MMGStringDisplay::stringChanged, [&]() { onList1Change(); });

	MMGStringDisplay *hotkey_display = display()->stringDisplays()->addNew(&hotkey);
	hotkey_display->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
}

void MMGActionHotkeys::setActionParams()
{
	display()->stringDisplays()->hideAll();

	MMGStringDisplay *group_display = display()->stringDisplays()->fieldAt(0);
	group_display->setVisible(true);
	group_display->setDescription(mmgtr("Actions.Hotkeys.Group"));
	group_display->setBounds(enumerateCategories());
}

void MMGActionHotkeys::onList1Change()
{
	MMGStringDisplay *hotkey_display = display()->stringDisplays()->fieldAt(1);
	hotkey_display->setVisible(true);
	hotkey_display->setDescription(mmgtr("Actions.Hotkeys.Hotkey"));
	hotkey_display->setOptions(MIDIBUTTON_TOGGLE);
	hotkey_display->setBounds(enumerate(group).values());
}

const QMap<QString, QString> MMGActionHotkeys::enumerate(const QString &category)
{
	struct MMGHotkeyEnumeration {
		QString category;
		QMap<QString, QString> list;
	} req{category};

	obs_enum_hotkeys(
		[](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			Q_UNUSED(id);
			auto _req = reinterpret_cast<MMGHotkeyEnumeration *>(param);

			if (_req->category != registerer(hotkey)) return true;

			_req->list.insert(obs_hotkey_get_name(hotkey), obs_hotkey_get_description(hotkey));
			return true;
		},
		&req);

	return req.list;
}

const QStringList MMGActionHotkeys::enumerateCategories()
{
	QStringList list;

	obs_enum_hotkeys(
		[](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			Q_UNUSED(id);
			auto _list = reinterpret_cast<QStringList *>(param);
			QString hotkey_group = registerer(hotkey);

			if (_list->contains(hotkey_group)) return true;

			_list->append(hotkey_group);
			return true;
		},
		&list);

	return list;
}

const QString MMGActionHotkeys::registerer(obs_hotkey_t *hotkey)
{
#define HOTKEY_REGISTERER_GET_NAME(kind)                                                     \
	ptr_##kind = obs_weak_##kind##_get_##kind(                                           \
		reinterpret_cast<obs_weak_##kind##_t *>(obs_hotkey_get_registerer(hotkey))); \
	if (!ptr_##kind) return "";                                                          \
	return obs_##kind##_get_name(ptr_##kind);

	OBSSourceAutoRelease ptr_source;
	OBSOutputAutoRelease ptr_output;
	OBSEncoderAutoRelease ptr_encoder;
	OBSServiceAutoRelease ptr_service;

	switch (obs_hotkey_get_registerer_type(hotkey)) {
		case OBS_HOTKEY_REGISTERER_FRONTEND:
			return mmgtr("Actions.Hotkeys.Frontend");
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
}

void MMGActionHotkeys::execute(const MMGMessage *) const
{
	ACTION_ASSERT(sub() == HOTKEY_PREDEF, "Invalid action.");

	MMGHotkeyRequest req;
	req.hotkey_group = group;
	req.hotkey_name = enumerate(group).key(hotkey);

	obs_enum_hotkeys(
		[](void *data, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			auto _req = reinterpret_cast<MMGHotkeyRequest *>(data);

			if (obs_hotkey_get_name(hotkey) != _req->hotkey_name) return true;
			if (registerer(hotkey) != _req->hotkey_group) return true;

			_req->hotkey_found = true;
			_req->hotkey_id = id;
			return false;
		},
		&req);

	ACTION_ASSERT(req.hotkey_found, "Hotkey does not exist.");

	obs_queue_task(
		OBS_TASK_UI,
		[](void *param) {
			auto id = (size_t *)param;
			obs_hotkey_trigger_routed_callback(*id, false);
			obs_hotkey_trigger_routed_callback(*id, true);
			obs_hotkey_trigger_routed_callback(*id, false);
		},
		&req.hotkey_id, true);

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionHotkeys::connectSignals(bool _connect)
{
	MMGAction::connectSignals(_connect);
	if (!_connect) return;

	connect(mmgsignals(), &MMGSignals::hotkeyEvent, this, &MMGActionHotkeys::hotkeyEventReceived);
}

void MMGActionHotkeys::hotkeyEventReceived(obs_hotkey_id id)
{
	MMGHotkeyRequest req;
	req.hotkey_id = id;
	req.hotkey_group = group;
	req.hotkey_name = MMGActionHotkeys::enumerate(group).key(hotkey);

	obs_enum_hotkeys(
		[](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			auto _req = reinterpret_cast<MMGHotkeyRequest *>(param);

			if (id != _req->hotkey_id) return true;

			if (registerer(hotkey) != _req->hotkey_group) return false;

			_req->hotkey_found = obs_hotkey_get_name(hotkey) == _req->hotkey_name;
			return false;
		},
		&req);

	if (req.hotkey_found) triggerEvent();
}
