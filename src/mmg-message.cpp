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

using namespace MMGUtils;

MMGMessage::MMGMessage() : _channel(), _type(), _note(), _value()
{
  _channel = 1;
  _type = "Note On";
  _value.set_state(MMGNumber::NUMBERSTATE_MIDI);
  blog(LOG_DEBUG, "Empty message created.");
}

MMGMessage::MMGMessage(const libremidi::message &message)
{
  _channel = message.get_channel();
  _type = get_midi_type(message);
  _note = get_midi_note(message);
  _value = get_midi_value(message);
}

MMGMessage::MMGMessage(const QJsonObject &obj)
  : _type(obj, "type", 0),
    _channel(obj, "channel", 0),
    _note(obj, "note", 0),
    _value(obj, "value", 0)
{
  if (_channel == 0) _channel = 1;

  if ((obj["value_require"].isBool() && !obj["value_require"].toBool()) || obj["value"].toInt() < 0)
    _value.set_state(MMGNumber::NUMBERSTATE_MIDI);

  if (obj["value_toggle"].isBool() && obj["value_toggle"].toBool())
    _value.set_state(MMGNumber::NUMBERSTATE_IGNORE);

  if (obj["type_toggle"].isBool() && obj["type_toggle"].toBool())
    _type.set_state(MMGString::STRINGSTATE_TOGGLE);

  blog(LOG_DEBUG, "Message created.");
}

void MMGMessage::json(QJsonObject &message_obj) const
{
  _channel.json(message_obj, "channel", false);
  _type.json(message_obj, "type", true);
  _note.json(message_obj, "note", false);
  _value.json(message_obj, "value", true);
}

void MMGMessage::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "Messages -> " + message);
}

int MMGMessage::get_midi_note(const libremidi::message &mess)
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

int MMGMessage::get_midi_value(const libremidi::message &mess)
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

void MMGMessage::toggle()
{
  if (_type.state() == MMGString::STRINGSTATE_TOGGLE) {
    if (_type == "Note On") {
      _type = "Note Off";
    } else if (_type == "Note Off") {
      _type = "Note On";
    }
  }
  if (_value.state() == MMGNumber::NUMBERSTATE_MIDI_INVERT) {
    if (_value == 127) {
      _value = 0;
    } else if (_value == 0) {
      _value = 127;
    }
  }
}

bool MMGMessage::is_acceptable(const MMGMessage *test) const
{
  bool is_true = true;
  is_true &= (_channel == test->channel());
  is_true &= (_type == test->type().str());
  if (_type != "Program Change" && _type != "Pitch Bend") is_true &= (_note == test->note().num());
  if (_value.state() != MMGNumber::NUMBERSTATE_MIDI) is_true &= (_value == test->value().num());
  return is_true;
}

void MMGMessage::deep_copy(MMGMessage *dest)
{
  _channel.copy(&dest->channel());
  _type.copy(&dest->type());
  _note.copy(&dest->note());
  _value.copy(&dest->value());
}
