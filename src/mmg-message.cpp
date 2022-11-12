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

MMGMessage::MMGMessage()
{
  channel = 1;
  type = "Note On";
  note = 0;
  value = 0;
  value_require = false;
  value_toggle = false;
  type_toggle = false;
  blog(LOG_DEBUG, "Empty message created.");
}

MMGMessage::MMGMessage(const libremidi::message &message)
{
  channel = message.get_channel();
  type = get_midi_type(message);
  note = get_midi_note(message);
  value = get_midi_value(message);
  value_require = true;
  value_toggle = false;
  type_toggle = false;
}

MMGMessage::MMGMessage(const QJsonObject &obj)
{
  channel = obj["channel"].toInt(1);
  type = obj["type"].toString("Note On");
  note = obj["note"].toInt();
  value = obj["value"].toInt();
  value_require = obj["value_require"].toBool(value != -1);
  value_toggle = obj["value_toggle"].toBool();
  type_toggle = obj["type_toggle"].toBool();
  blog(LOG_DEBUG, "Message created.");
}

void MMGMessage::json(QJsonObject &message_obj) const
{
  message_obj["channel"] = channel;
  message_obj["type"] = type;
  message_obj["note"] = note;
  message_obj["value"] = value;
  message_obj["value_require"] = value_require;
  message_obj["value_toggle"] = value_toggle;
  message_obj["type_toggle"] = type_toggle;
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
  if (type_toggle) {
    if (type == "Note On") {
      type = "Note Off";
    } else if (type == "Note Off") {
      type = "Note On";
    }
  }
  if (value_toggle) {
    if (value == 127) {
      value = 0;
    } else if (value == 0) {
      value = 127;
    }
  }
}

bool MMGMessage::is_acceptable(const MMGMessage *test) const
{
  bool is_true = true;
  is_true &= (channel == test->get_channel());
  is_true &= (type == test->get_type());
  if (type != "Program Change" && type != "Pitch Bend")
    is_true &= (note == test->get_note());
  if (value_require || value_toggle)
    is_true &= (value == test->get_value());
  return is_true;
}

void MMGMessage::deep_copy(MMGMessage *dest)
{
  dest->set_channel(channel);
  dest->set_type(type);
  dest->set_note(note);
  dest->set_value(value);
  dest->set_value_required(value_require);
  dest->set_value_toggle(value_toggle);
  dest->set_type_toggle(type_toggle);
}
