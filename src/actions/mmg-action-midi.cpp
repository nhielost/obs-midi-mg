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

#include "mmg-action-midi.h"
#include "../mmg-config.h"

using namespace MMGUtils;

MMGActionMIDI::MMGActionMIDI(const QJsonObject &json_obj)
  : device(json_obj, "device", 1),
    type(json_obj, "type", 2),
    channel(json_obj, "channel", 1),
    note(json_obj, "note", 2),
    value(json_obj, "value", 3)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<MIDI> action created.");
}

void MMGActionMIDI::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<MIDI> Action -> " + message);
}

void MMGActionMIDI::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  device.json(json_obj, "device", false);
  type.json(json_obj, "type", true);
  channel.json(json_obj, "channel", true);
  note.json(json_obj, "note", true);
  value.json(json_obj, "value", true);
}

void MMGActionMIDI::do_action(const MMGMessage *midi)
{
  if (get_sub() == 0) {
    MMGDevice *const output = global()->find_device(device);
    if (!output) {
      blog(LOG_INFO, "FAILED: Output device is not connected or does not exist.");
      return;
    }
    libremidi::message msg;
    QString _type = type.state() == MMGString::STRINGSTATE_MIDI ? midi->type() : type;
    int _channel = channel.state() == MMGNumber::NUMBERSTATE_MIDI ? midi->channel() : channel;
    int _note = note.state() == MMGNumber::NUMBERSTATE_MIDI ? midi->note() : note;
    int _value = value.choose(midi);
    if (_type == "Note On") {
      msg = libremidi::message::note_on(_channel, _note, _value);
    } else if (_type == "Note Off") {
      msg = libremidi::message::note_off(_channel, _note, _value);
    } else if (_type == "Control Change") {
      msg = libremidi::message::control_change(_channel, _note, _value);
    } else if (_type == "Program Change") {
      msg = libremidi::message::program_change(_channel, _value);
    } else if (_type == "Pitch Bend") {
      msg = libremidi::message::pitch_bend(_channel, (_value <= 64 ? 0 : _value - 64) * 2, _value);
    } else {
      msg = libremidi::message();
    }
    if (!MMGDevice::output_port_open()) MMGDevice::open_output_port(output);
    output->output_send(msg);
    MMGDevice::close_output_port();
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionMIDI::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  device.copy(&dest->str1());
  type.copy(&dest->str2());
  channel.copy(&dest->num1());
  note.copy(&dest->num2());
  value.copy(&dest->num3());
}

void MMGActionMIDI::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Send Single Message"};
}
void MMGActionMIDI::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Output Device";
  val.list = MMGDevice::get_output_device_names();
}
void MMGActionMIDI::change_options_str2(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR2;
  val.label_text = "Message Type";
  val.list = {"Note On",        "Note Off",   "Control Change",
	      "Program Change", "Pitch Bend", "Use Message Type"};
}
void MMGActionMIDI::change_options_str3(MMGActionDisplayParams &val)
{
  val.label_lcds[0] = "Channel";
  val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI;
  val.lcds[0]->set_range(1.0, 16.0);
  val.lcds[0]->set_step(1.0, 5.0);
  val.lcds[0]->set_default_value(1.0);

  val.combo_display[1] = MMGActionDisplayParams::COMBODISPLAY_MIDI;
  val.lcds[1]->set_range(0.0, 127.0);
  val.lcds[1]->set_step(1.0, 10.0);
  val.lcds[1]->set_default_value(0.0);

  val.combo_display[2] = MMGActionDisplayParams::COMBODISPLAY_MIDI;
  val.lcds[2]->set_range(0.0, 127.0);
  val.lcds[2]->set_step(1.0, 10.0);
  val.lcds[2]->set_default_value(0.0);

  type.set_state(type == "Use Message Type" ? MMGString::STRINGSTATE_MIDI
					    : MMGString::STRINGSTATE_FIXED);

  QString actual_type = type.state() > 0 ? val.extra_data : type;
  val.display &= ~MMGActionDisplayParams::DISPLAY_NUM3;
  if (actual_type.contains("Note")) {
    val.display |= MMGActionDisplayParams::DISPLAY_NUM3;
    val.label_lcds[1] = "Note #";
    val.label_lcds[2] = "Velocity";
  } else if (actual_type == "Control Change") {
    val.display |= MMGActionDisplayParams::DISPLAY_NUM3;
    val.label_lcds[1] = "Control #";
    val.label_lcds[2] = "Value";
  } else if (actual_type == "Program Change") {
    val.display |= MMGActionDisplayParams::DISPLAY_NUM2;
    val.label_lcds[1] = "Program #";
  } else if (actual_type == "Pitch Bend") {
    val.display |= MMGActionDisplayParams::DISPLAY_NUM2;
    val.label_lcds[1] = "Pitch Adjust";
  }
}
void MMGActionMIDI::change_options_final(MMGActionDisplayParams &val) {}
