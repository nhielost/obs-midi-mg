/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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

MMGMessage::MMGMessage(const libremidi::message &message)
{
	name = "";
	channel = message.get_channel();
	type = get_midi_type(message);
	note = get_midi_note(message);
	value = get_midi_value(message);
}

MMGMessage::MMGMessage(const QJsonObject &obj)
{
	name = obj["name"].toString(
		MMGUtils::next_default_name(MMGModes::MMGMODE_MESSAGE));
	channel = obj["channel"].toInt(1);
	type = obj["type"].toString("Note On");
	note = obj["note"].toInt(0);
	value = obj["value"].toInt(-1);
}

void MMGMessage::json(QJsonObject &message_obj) const
{
	message_obj["name"] = name;
	message_obj["channel"] = channel;
	message_obj["type"] = type;
	message_obj["note"] = note;
	message_obj["value"] = value;
}

int MMGMessage::get_midi_note(const libremidi::message &mess)
{
	int part = -1;
	switch (mess.get_message_type()) {
	case libremidi::message_type::NOTE_OFF:
	case libremidi::message_type::NOTE_ON:
	case libremidi::message_type::CONTROL_CHANGE:
		part = 1;
		break;
	default:
		return 0;
	}
	return mess[part];
}

int MMGMessage::get_midi_value(const libremidi::message &mess)
{
	int part = -1;
	switch (mess.get_message_type()) {
	case libremidi::message_type::NOTE_ON:
	case libremidi::message_type::NOTE_OFF:
	case libremidi::message_type::CONTROL_CHANGE:
	case libremidi::message_type::PITCH_BEND:
		part = 2;
		break;
	case libremidi::message_type::PROGRAM_CHANGE:
		part = 1;
		break;
	default:
		break;
	}
	return mess[part];
}

QString MMGMessage::get_midi_type(const libremidi::message &mess)
{
	switch (mess.get_message_type()) {
	// Standard Messages
	case libremidi::message_type::NOTE_OFF:
		return "Note Off";
	case libremidi::message_type::NOTE_ON:
		return "Note On";
	case libremidi::message_type::POLY_PRESSURE:
		return "Polyphonic Pressure";
	case libremidi::message_type::CONTROL_CHANGE:
		return "Control Change";
	case libremidi::message_type::PROGRAM_CHANGE:
		return "Program Change";
	case libremidi::message_type::AFTERTOUCH:
		return "Channel Aftertouch";
	case libremidi::message_type::PITCH_BEND:
		return "Pitch Bend";
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

bool MMGMessage::is_acceptable(MMGMessage *test) const
{
	bool isTrue = true;
	isTrue &= (channel == test->get_channel());
	isTrue &= (type == test->get_type());
	if (type != "Program Change" && type != "Pitch Bend")
		isTrue &= (note == test->get_note());
	if (value != -1)
		isTrue &= (value == test->get_value());
	return isTrue;
}

const libremidi::message MMGMessage::to_libremidi_message() const
{
	if (type == "Note On") {
		return libremidi::message::note_on(channel, note, value);
	} else if (type == "Note Off") {
		return libremidi::message::note_off(channel, note, value);
	} else if (type == "Control Change") {
		return libremidi::message::control_change(channel, note, value);
	} else if (type == "Program Change") {
		return libremidi::message::program_change(channel, value);
	} else if (type == "Pitch Bend") {
		return libremidi::message::pitch_bend(channel, note, value);
	}
	throw 1;
}
