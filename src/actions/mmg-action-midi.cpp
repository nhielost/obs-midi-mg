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

#include "mmg-action-midi.h"
#include "../mmg-config.h"
#include "../mmg-midiout.h"

using namespace MMGUtils;

MMGActionMIDI::MMGActionMIDI(const QJsonObject &json_obj)
  : device(json_obj, "device", 1),
    type(json_obj, "type", 2),
    channel(json_obj, "channel", 1),
    note(json_obj, "note", 2),
    value(json_obj, "value", 3)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionMIDI::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[MIDI] " + message);
}

void MMGActionMIDI::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  device.json(json_obj, "device", false);
  type.json(json_obj, "type");
  channel.json(json_obj, "channel");
  note.json(json_obj, "note");
  value.json(json_obj, "value");
}

void MMGActionMIDI::execute(const MMGMessage *midi) const
{
  if (sub() == 0) {
    MMGDevice *output = global()->find(device);
    if (!output) {
      blog(LOG_INFO, "FAILED: Output device is not connected or does not exist.");
      return;
    }

    MMGMessage msg;
    msg.type()->set_str(type.state() == MMGString::STRINGSTATE_MIDI ? midi->type() : type);
    msg.channel()->set_num(channel.state() == MMGNumber::NUMBERSTATE_MIDI ? midi->channel()
									  : channel);
    msg.note()->set_num(note.state() == MMGNumber::NUMBERSTATE_MIDI ? midi->note() : note);
    msg.value()->set_num(value.state() == MMGNumber::NUMBERSTATE_MIDI ? midi->value() : value);

    if (!output_device()->isOutputPortOpen()) output_device()->openOutputPort(output);
    output_device()->sendMessage(&msg);
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionMIDI::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionMIDI *>(dest);
  if (!casted) return;

  casted->device = device.copy();
  casted->type = type.copy();
  casted->channel = channel.copy();
  casted->note = note.copy();
  casted->value = value.copy();
}

void MMGActionMIDI::setEditable(bool edit)
{
  device.set_edit(edit);
  type.set_edit(edit);
  channel.set_edit(edit);
  note.set_edit(edit);
  value.set_edit(edit);
}

void MMGActionMIDI::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&device);
  _display->setStr2Storage(&type);

  MMGNumberDisplay *channel_display = new MMGNumberDisplay(_display->numberDisplays());
  channel_display->setStorage(&channel, true);
  _display->numberDisplays()->add(channel_display);
  MMGNumberDisplay *note_display = new MMGNumberDisplay(_display->numberDisplays());
  note_display->setStorage(&note, true);
  _display->numberDisplays()->add(note_display);
  MMGNumberDisplay *value_display = new MMGNumberDisplay(_display->numberDisplays());
  value_display->setStorage(&value, true);
  _display->numberDisplays()->add(value_display);

  _display->connect(_display, &MMGActionDisplay::str1Changed, [&]() { setList1Config(); });
  _display->connect(_display, &MMGActionDisplay::str2Changed, [&]() { setList2Config(); });
}

void MMGActionMIDI::setSubOptions(QComboBox *sub)
{
  sub->addItem(mmgtr("Actions.MIDI.Sub.SendSingle"));
}

void MMGActionMIDI::setSubConfig()
{
  _display->setStr1Visible(false);
  _display->setStr2Visible(false);
  _display->setStr3Visible(false);

  _display->setStr1Visible(true);
  _display->setStr1Description(mmgtr("Actions.MIDI.Output"));
  _display->setStr1Options(output_device()->outputDeviceNames());
}

void MMGActionMIDI::setList1Config()
{
  _display->setStr2Visible(true);
  _display->setStr2Description(mmgtr("Message.Type.Text"));
  _display->setStr2Options(mmgtr_all(
    "Message.Type", {"NoteOn", "NoteOff", "ControlChange", "ProgramChange", "PitchBend"}, true));
}

void MMGActionMIDI::setList2Config()
{
  MMGNumberDisplay *channel_display = _display->numberDisplays()->fieldAt(0);
  MMGNumberDisplay *note_display = _display->numberDisplays()->fieldAt(1);
  MMGNumberDisplay *value_display = _display->numberDisplays()->fieldAt(2);

  channel_display->setDescription(mmgtr("Message.Channel"));
  channel_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_DEFAULT);
  channel_display->setBounds(1.0, 16.0);
  channel_display->setStep(1.0);
  channel_display->setDefaultValue(1.0);

  note_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_DEFAULT);
  note_display->setBounds(0.0, 127.0);
  note_display->setStep(1.0);
  note_display->setDefaultValue(0.0);

  value_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_DEFAULT);
  value_display->setBounds(0.0, 127.0);
  value_display->setStep(1.0);
  value_display->setDefaultValue(0.0);

  type.set_state(type == mmgtr("Fields.UseMessageValue"));

  setLabels();
}

void MMGActionMIDI::setLabels()
{
  if (!_display) return;

  MMGNumberDisplay *note_display = _display->numberDisplays()->fieldAt(1);
  MMGNumberDisplay *value_display = _display->numberDisplays()->fieldAt(2);

  set_message_labels(type.state() ? _display->parentBinding()->message()->type() : type,
		     note_display, value_display);
}
