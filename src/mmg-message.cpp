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

#include "mmg-message.h"
#include "mmg-config.h"

using namespace MMGUtils;

// MMGMessage
MMGMessage::MMGMessage(QObject *parent) : QObject(parent)
{
	setRanges();

	setObjectName(mmgtr("Message.Untitled"));
	setDevice(manager(device)->at(0));
	_channel = 1;
	_type = mmgtr("Message.Type.NoteOn");
	_value.setState(STATE_MIDI);
}

MMGMessage::MMGMessage(MMGMIDIPort *device, const libremidi::message &message)
{
	setRanges();

	setDevice(device);
	_channel = message.get_channel();
	_type = getType(message);
	_note = getNote(message);
	_value = getValue(message);
}

MMGMessage::MMGMessage(const QJsonObject &json_obj)
	: _channel(json_obj, "channel"), _type(json_obj, "type"), _note(json_obj, "note"), _value(json_obj, "value")
{
	setRanges();

	setObjectName(json_obj["name"].toString(mmgtr("Message.Untitled")));

	setDevice(manager(device)->find(json_obj["device"].toString()));

	if (_channel == 0) _channel = 1;

	if ((json_obj["value_require"].isBool() && !(json_obj["value_require"].toBool())) || _value < 0)
		_value.setState(STATE_MIDI);

	if ((json_obj["value_toggle"].isBool() && json_obj["value_toggle"].toBool()) ||
	    json_obj["value_state"].toInt() == 2)
		_value.setState(STATE_TOGGLE);

	if (json_obj["type_toggle"].isBool() && json_obj["type_toggle"].toBool()) {
		_type.setState(STATE_TOGGLE);
		_type.setMax(mmgtr("Message.Type.NoteOff"));
	}
}

void MMGMessage::setDevice(MMGMIDIPort *device)
{
	if (_device) disconnect(_device, &QObject::destroyed, this, nullptr);

	_device = device;
	if (!device) return;

	connect(_device, &QObject::destroyed, this, [&]() { _device = nullptr; });
}

void MMGMessage::blog(int log_status, const QString &message) const
{
	global_blog(log_status, "[Messages] " + message);
}

void MMGMessage::json(QJsonObject &message_obj) const
{
	message_obj["name"] = objectName();
	message_obj["device"] = _device ? _device->objectName() : "";
	_channel.json(message_obj, "channel");
	_type.json(message_obj, "type");
	_note.json(message_obj, "note");
	_value.json(message_obj, "value");
}

void MMGMessage::copy(MMGMessage *dest) const
{
	dest->setObjectName(objectName());
	dest->_device = _device;
	dest->_channel = _channel.copy();
	dest->_type = _type.copy();
	dest->_note = _note.copy();
	dest->_value = _value.copy();
}

void MMGMessage::setEditable(bool edit)
{
	_channel.setEditable(edit);
	_type.setEditable(edit);
	_note.setEditable(edit);
	_value.setEditable(edit);
}

void MMGMessage::toggle()
{
	_type.toggle();
	_channel.toggle();
	_note.toggle();
	_value.toggle();
}

bool MMGMessage::acceptable(const MMGMessage *test) const
{
	bool accepted = true;
	accepted &= _device == test->_device;
	accepted &= _channel.acceptable(test->channel());
	accepted &= (_type == test->type().value());
	if (!valueOnlyType()) accepted &= _note.acceptable(test->note());

	return accepted && _value.acceptable(test->value());
}

void MMGMessage::applyValues(const MMGNumber &applied)
{
	if (_type.state() == STATE_MIDI) {
		MMGNumber number;
		number.setMax(acceptedTypes().size() - 1);
		_type = acceptedTypes().value(applied.map(number));
	}
	_channel.chooseOther(applied);
	_note.chooseOther(applied);
	_value.chooseOther(applied);
}

void MMGMessage::send()
{
	if (_device) _device->sendMessage(this);
}

bool MMGMessage::valueOnlyType() const
{
	return _type == mmgtr("Message.Type.ProgramChange") || _type == mmgtr("Message.Type.PitchBend");
}

int MMGMessage::getNote(const libremidi::message &mess)
{
	switch (mess.get_message_type()) {
		case libremidi::message_type::NOTE_OFF:
		case libremidi::message_type::NOTE_ON:
		case libremidi::message_type::CONTROL_CHANGE:
			return mess[1];
		default:
			return 0;
	}
}

int MMGMessage::getValue(const libremidi::message &mess)
{
	switch (mess.get_message_type()) {
		case libremidi::message_type::NOTE_ON:
		case libremidi::message_type::NOTE_OFF:
		case libremidi::message_type::CONTROL_CHANGE:
		case libremidi::message_type::PITCH_BEND:
			return mess[2];
		case libremidi::message_type::PROGRAM_CHANGE:
			return mess[1];
		default:
			return -1;
	}
}

QString MMGMessage::getType(const libremidi::message &mess)
{
	switch (mess.get_message_type()) {
		// Standard Messages
		case libremidi::message_type::NOTE_OFF:
			return mmgtr("Message.Type.NoteOff");
		case libremidi::message_type::NOTE_ON:
			return mmgtr("Message.Type.NoteOn");
		case libremidi::message_type::POLY_PRESSURE:
			return "Polyphonic Pressure";
		case libremidi::message_type::CONTROL_CHANGE:
			return mmgtr("Message.Type.ControlChange");
		case libremidi::message_type::PROGRAM_CHANGE:
			return mmgtr("Message.Type.ProgramChange");
		case libremidi::message_type::AFTERTOUCH:
			return "Channel Aftertouch";
		case libremidi::message_type::PITCH_BEND:
			return mmgtr("Message.Type.PitchBend");
		// System Common Messages
		case libremidi::message_type::SYSTEM_EXCLUSIVE:
			return "System Exclusive";
		case libremidi::message_type::TIME_CODE:
			return "Time Code";
		case libremidi::message_type::SONG_POS_POINTER:
			return "Song Position Pointer";
		case libremidi::message_type::SONG_SELECT:
			return "Song Select";
		case libremidi::message_type::RESERVED1:
			return "Reserved (1)";
		case libremidi::message_type::RESERVED2:
			return "Reserved (2)";
		case libremidi::message_type::TUNE_REQUEST:
			return "Tune Request";
		case libremidi::message_type::EOX:
			return "End of System Exclusive";
		// System Realtime Messages
		case libremidi::message_type::TIME_CLOCK:
			return "Time Clock";
		case libremidi::message_type::RESERVED3:
			return "Reserved (3)";
		case libremidi::message_type::START:
			return "Start File";
		case libremidi::message_type::CONTINUE:
			return "Continue File";
		case libremidi::message_type::STOP:
			return "Stop File";
		case libremidi::message_type::RESERVED4:
			return "Reserved (4)";
		case libremidi::message_type::ACTIVE_SENSING:
			return "Active Sensing";
		case libremidi::message_type::SYSTEM_RESET:
			return "System Reset";
		default:
			return "Error";
	}
}

const QStringList MMGMessage::acceptedTypes()
{
	return mmgtr_all("Message.Type", {"NoteOn", "NoteOff", "ControlChange", "ProgramChange", "PitchBend"});
}

void MMGMessage::setRanges()
{
	if (_channel.state() == STATE_FIXED && _channel.max() == 100) {
		_channel.setMin(1);
		_channel.setMax(16);
	}

	if (_type.state() == STATE_FIXED && _type.max() == mmgtr("Messages.Type.NoteOn"))
		_type.setMax(mmgtr("Message.Type.PitchBend"));

	if (_note.state() == STATE_FIXED && _note.max() == 100) _note.setMax(127);
	if (_value.state() == STATE_FIXED && _value.max() == 100) _value.setMax(127);
}

QDataStream &operator<<(QDataStream &out, const MMGMessage *&obj)
{
	return out << (const QObject *&)obj;
}

QDataStream &operator>>(QDataStream &in, MMGMessage *&obj)
{
	return in >> (QObject *&)obj;
}
// End MMGMessage

// MMGMessageManager
MMGMessage *MMGMessageManager::add(const QJsonObject &json_obj)
{
	MMGMessage *message = json_obj.isEmpty() ? new MMGMessage(this) : new MMGMessage(json_obj);
	return MMGManager::add(message);
}
// End MMGMessageManager
