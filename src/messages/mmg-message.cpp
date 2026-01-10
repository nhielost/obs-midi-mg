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

#include "mmg-message.h"
#include "../mmg-config.h"
#include "../mmg-preference-defs.h"

namespace MMGMessages {

struct MessageTypeInfo {
	const char *type_name;
	const char *message_name;
	ConstructBase *init;
};

static QMap<Id, MessageTypeInfo> all_message_types;

ConstructBase::ConstructBase(MMGMessage *message)
{
	all_message_types.insert(message->id(), {message->typeName(), message->trMessageName(), this});
};

bool usingMIDI2()
{
	return MMGPreferences::MMGPreferenceMIDI::currentMessageMode() ==
	       MMGPreferences::MMGPreferenceMIDI::MIDI_ALWAYS_2;
}

const MMGTranslationMap<Id> availableMessageTypes()
{
	MMGTranslationMap<Id> types;

	for (auto [id, info] : all_message_types.asKeyValueRange()) {
		Id type = Id(id & 0xff00);
		if (types.contains(type)) continue;

		types.insert(type, mmgtr(MMGText::join("Message.Titles", info.type_name)));
	}

	return types;
};

const MMGTranslationMap<Id> availableMessages(Id message_type)
{
	MMGTranslationMap<Id> messages;
	std::string tr;

	for (auto [id, info] : all_message_types.asKeyValueRange()) {
		if ((id & 0xff00) < message_type) continue;
		if ((id & 0xff00) > message_type) break;

		tr = std::format("Message.{}.Status.{}", info.type_name, info.message_name);

		messages.insert(id, mmgtr(tr.c_str()));
	}

	return messages;
};

static MMGTranslationMap<uint16_t> old_ids {
	{0x4090, nontr("Note On")},        {0x4080, nontr("Note Off")},   {0x40b0, nontr("Control Change")},
	{0x40c0, nontr("Program Change")}, {0x40e0, nontr("Pitch Bend")},
};

MMGMessage *generateMessage(MMGMessageManager *parent, const QJsonObject &json_obj)
{
	QJsonObject init_obj = json_obj;

	if (config()->fileVersion() < MMGConfig::VERSION_3_1) {
		MMG16Bit _type(init_obj, "type");
		MMGCompatibility::initOldStringData(_type, init_obj, "type", 0, old_ids);

		// No MIDI for types
		if (_type->state() == STATE_MIDI) _type = 0x4080;

		// For the toggle functionality
		if (json_obj["type_toggle"].isBool() && json_obj["type_toggle"].toBool()) _type = 0x409f;

		if (!all_message_types.contains(Id(uint16_t(_type)))) _type = 0x4080;
		MMGJson::setValue<uint16_t>(init_obj, "id", _type);
	} else if (!init_obj.contains("id")) {
		MMGJson::setValue<uint16_t>(init_obj, "id", 0x4080);
	}

	Id id = MMGJson::getValue<Id>(init_obj, "id");
	if (!all_message_types.contains(id)) {
		mmgblog(LOG_INFO, QString("[Messages] Message ID *0x%1* is invalid - reverting to Note Off Message.")
					  .arg(uint16_t(id), 4, 16, QChar('0')));
		id = Id(0x4080);
	}

	auto init = all_message_types[id].init;
	MMGMessage *new_message = (*init)(parent, init_obj);
	if (config()->fileVersion() < MMGConfig::VERSION_3_1) new_message->initOldData(init_obj);

	return new_message;
}

bool changeMessage(MMGMessageManager *parent, MMGMessage *&message, Id new_id)
{
	if (!parent || !message) return false;
	if (message->id() == new_id) return false;

	QJsonObject json_obj;
	MMGJson::setValue(json_obj, "name", message->objectName());
	MMGJson::setValue(json_obj, "id", new_id);

	qsizetype index = parent->indexOf(message);
	parent->remove(message);
	message = parent->add(json_obj);
	parent->move(parent->size() - 1, index);

	return true;
}

// void visitMessage(const std::function<void(const MMGMessage *)> &cb, const
// MMGMessageData &midi)
//{
//	QJsonObject json_obj;
//	json_obj["type"] = (uint8_t)midi.type();
//	std::unique_ptr<MMGMessage> message(generateMessage(nullptr, json_obj));
//	message->copyFromMessageData(midi);
//	cb(message.get());
// }
//
// void replaceMessageString(QString &str, const MMGMessageData &midi)
//{
//	visitMessage([&](const MMGMessage *msg) { msg->replaceString(str); },
// midi);
// }

} // namespace MMGMessages

MMGMessage::MMGMessage(MMGMessageManager *parent, const QJsonObject &json_obj) : QObject(parent)
{
	setObjectName(json_obj["name"].toString(mmgtr("Message.Untitled")));

	if (!json_obj.isEmpty()) { setDevice(manager(device)->find(json_obj["device"].toString())); }
}

void MMGMessage::setDevice(MMGMIDIPort *device)
{
	if (!!_device) {
		disconnect(_device, &QObject::destroyed, this, nullptr);
		disconnect(this, &QObject::destroyed, _device, nullptr);
		_device->connectReceiver(this, false);
	}

	_device = device;
	if (!device) return;

	connect(_device, &QObject::destroyed, this, [this]() { _device = nullptr; });
	connect(this, &QObject::destroyed, _device, [this]() { _device->connectReceiver(this, false); });
}

void MMGMessage::blog(int log_status, const QString &message) const
{
	mmgblog(log_status, QString("[Messages] <%1> %2").arg(objectName()).arg(message));
}

void MMGMessage::json(QJsonObject &message_obj) const
{
	message_obj["name"] = objectName();
	message_obj["device"] = _device ? _device->objectName() : "";
	MMGJson::setValue(message_obj, "id", id());
}

void MMGMessage::copy(MMGMessage *dest) const
{
	dest->setObjectName(objectName());
	dest->setDevice(_device);
}

void MMGMessage::replaceString(QString &str) const
{
	str.replace("${name}", objectName());
	if (!!_device) str.replace("${device}", _device->objectName());
}

void MMGMessage::connectDevice(bool _connect)
{
	if (!_device) return;
	_device->connectReceiver(this, _connect);
}

void MMGMessage::send(const MMGMappingTest &test) const
{
	if (!_device) return;

	MMGMessageData message;
	message.set<0, 4>(MMGMessages::usingMIDI2() ? MMGMessages::MIDI2_CV : MMGMessages::MIDI1_CV);
	copyToMessageData(message, test);
	_device->sendMessage(message);
}

MMGMessage *MMGMessage::generate(MMGMessageManager *parent, const QJsonObject &json_obj)
{
	return MMGMessages::generateMessage(parent, json_obj);
}
// End MMGMessage
