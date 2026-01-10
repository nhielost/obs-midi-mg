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

#include "mmg-action.h"
#include "../mmg-config.h"

namespace MMGActions {

struct ActionTypeInfo {
	const char *cat_name;
	const char *action_name;
	ConstructBase *init;
};

static QMap<Id, ActionTypeInfo> all_action_types;

ConstructBase::ConstructBase(MMGAction *action)
{
	all_action_types.insert(action->id(), {action->categoryName(), action->trActionName(), this});
};

const MMGTranslationMap<Id> availableActionCategories()
{
	MMGTranslationMap<Id> categories;

	for (auto [id, info] : all_action_types.asKeyValueRange()) {
		Id category = Id(id & 0xff00);
		if (categories.contains(category)) continue;

		categories.insert(category, mmgtr(MMGText::join("Actions.Titles", info.cat_name)));
	}

	return categories;
};

const MMGTranslationMap<Id> availableActions(Id category_id, DeviceType type)
{
	MMGTranslationMap<Id> actions;
	std::string tr;

	const char *input_str = "Input";
	const char *output_str = "Output";
	const char *type_str = type == TYPE_OUTPUT ? output_str : input_str;

	for (auto [id, info] : all_action_types.asKeyValueRange()) {
		if ((id & 0xff00) < category_id) continue;
		if ((id & 0xff00) > category_id) break;

		if (!!std::strstr(info.action_name, input_str) && type == TYPE_OUTPUT) continue;
		if (!!std::strstr(info.action_name, output_str) && type != TYPE_OUTPUT) continue;

		tr = std::format("Actions.{}.Sub.{}.{}", info.cat_name, type_str, info.action_name);

		actions.insert(id, mmgtr(tr.c_str()));
	}

	return actions;
};

// For compatibility with file version 4 configurations
static QMap<uint16_t, uint16_t> old_ids {
	{0x01ff, 0x0101}, {0x02a0, 0x0201}, {0x03ff, 0x0301}, {0x04a0, 0x0401}, {0x05b0, 0x1001}, {0x0503, 0x1002},
	{0x05c4, 0x1101}, {0x06ff, 0x1101}, {0x0700, 0x1211}, {0x0701, 0x1201}, {0x0702, 0x1202}, {0x0703, 0x1214},
	{0x0704, 0x1281}, {0x0705, 0x1212}, {0x0706, 0x1282}, {0x0707, 0x1213}, {0x0708, 0x12c1}, {0x0709, 0x12c2},
	{0x070a, 0x12c3}, {0x070b, 0x1283}, {0x070c, 0x1111}, {0x070d, 0x13ff}, {0x0800, 0x1301}, {0x0801, 0x1301},
	{0x08b2, 0x1302}, {0x0805, 0x1351}, {0x0806, 0x1352}, {0x09ff, 0x13a1}, {0x09c3, 0x13a2}, {0x09c6, 0x13a2},
	{0x09c7, 0x13a2}, {0x0a00, 0x1401}, {0x0ad1, 0x1401}, {0x0ac3, 0x1481}, {0x0ad5, 0x1481}, {0x0ac5, 0x14ff},
	{0x0ad6, 0x14ff}, {0x0bb0, 0x1501}, {0x0b03, 0x1581}, {0x0b04, 0x15ff}, {0x0cff, 0x1601}, {0x0dff, 0x2001},
	{0x0eff, 0x2101}, {0x0fff, 0xf001},
};

static void findOldActionId(QJsonObject &json_obj)
{
	ushort id = 0x0000;

	uchar cat = json_obj["category"].toInt();
	uchar sub = json_obj["sub"].toInt();
	DeviceType type = (DeviceType)json_obj["type"].toInt();
	uchar secondary_threshold = type == TYPE_OUTPUT ? 6 : 3;

	for (auto [old_id, new_id] : old_ids.asKeyValueRange()) {
		if ((old_id >> 8) > cat) break;
		if ((old_id >> 8) < cat) continue;

		uchar arg = old_id & 0x000f;
		switch (old_id & 0x00f0) {
			case 0xf0: // all possible
				id = new_id;
				continue;

			case 0xa0: // double output actions with additional shared at the end
				id = new_id + (0x0080) * (sub >= secondary_threshold);
				continue;

			case 0xb0: // on off toggle
				if (arg <= sub && sub <= arg + 2) id = new_id;
				continue;

			case 0xc0: // input only
				if (type != TYPE_OUTPUT && sub == arg) {
					id = new_id;
					break;
				}
				continue;

			case 0xd0: // output only
				if (type == TYPE_OUTPUT && sub == arg) {
					id = new_id;
					break;
				}
				continue;

			default:
				if (sub == arg) id = new_id;
				continue;
		}
		break;
	}

	json_obj["id"] = id;
}

MMGAction *generateAction(MMGActionManager *parent, const QJsonObject &json_obj)
{
	QJsonObject init_obj = json_obj;
	bool old_json = config()->fileVersion() < MMGConfig::VERSION_3_1;
	if (old_json) findOldActionId(init_obj);
	if (!init_obj.contains("id")) init_obj["id"] = 0x0000;

	Id id = MMGJson::getValue<Id>(init_obj, "id");
	if (!all_action_types.contains(id)) {
		mmgblog(LOG_INFO, QString("[Actions] Action ID *0x%1* is invalid - reverting to None Action.")
					  .arg(uint16_t(id), 4, 16, QChar('0')));
		id = Id(0x0000);
	}

	auto init = all_action_types[id].init;
	MMGAction *new_action = (*init)(parent, init_obj);
	if (old_json) new_action->initOldData(init_obj);

	return new_action;
}

bool changeAction(MMGActionManager *parent, MMGAction *&action, Id new_id)
{
	if (!parent || !action) return false;
	if (action->id() == new_id) return false;

	QJsonObject json_obj;
	MMGJson::setValue(json_obj, "name", action->objectName());
	MMGJson::setValue(json_obj, "id", new_id);

	qsizetype index = parent->indexOf(action);
	parent->remove(action);
	action = parent->add(json_obj);
	parent->move(parent->size() - 1, index);

	return true;
}

} // namespace MMGActions

// MMGAction
MMGAction::MMGAction(MMGActionManager *parent, const QJsonObject &json_obj) : QObject(parent)
{
	setObjectName(json_obj["name"].toString(mmgtr("Actions.Untitled")));
}

DeviceType MMGAction::type() const
{
	return qobject_cast<MMGBinding *>(parent()->parent())->type();
}

void MMGAction::blog(int log_status, const QString &message) const
{
	mmgblog(log_status, QString("[Actions] <%1> %2").arg(objectName()).arg(message));
}

void MMGAction::json(QJsonObject &json_obj) const
{
	json_obj["name"] = objectName();
	json_obj["id"] = (short)id();
}

void MMGAction::copy(MMGAction *dest) const
{
	dest->setObjectName(objectName());
}

MMGAction *MMGAction::generate(MMGActionManager *parent, const QJsonObject &json_obj)
{
	return MMGActions::generateAction(parent, json_obj);
}
// End MMGAction
