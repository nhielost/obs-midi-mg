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

#include "mmg-midiout.h"
#include "mmg-midiin.h"
#include "mmg-config.h"

using namespace MMGUtils;

MMGMIDIOut::MMGMIDIOut()
{
  thru_timer = new MMGTimer(this);

  connect(input_device().get(), &MMGMIDIIn::sendThru, this, &MMGMIDIOut::sendThru);
  connect(thru_timer, &MMGTimer::stopping, this, &MMGMIDIOut::closeOutputPort);
}

void MMGMIDIOut::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "[MIDI Out] " + message);
}

void MMGMIDIOut::openOutputPort(MMGDevice *device)
{
  if (isOutputPortOpen()) closeOutputPort();

  blog(LOG_INFO,
       QString::asprintf("Opening output port for device <%s>...", device->name().qtocs()));

  if (outputPort(device) == (uint)-1) {
    blog(LOG_INFO, "Output port opening failed: Device is disconnected or does not exist.");
    return;
  }

  try {
    midi_out.open_port(outputPort(device));
    blog(LOG_INFO, "Output port successfully opened.");
  } catch (const libremidi::driver_error &err) {
    blog(LOG_INFO, err.what());
  } catch (const libremidi::invalid_parameter_error &err) {
    blog(LOG_INFO, err.what());
  } catch (const libremidi::system_error &err) {
    blog(LOG_INFO, err.what());
  }
}

void MMGMIDIOut::closeOutputPort()
{
  if (!isOutputPortOpen()) return;
  midi_out.close_port();
  blog(LOG_INFO, "Output port closed.");
}

bool MMGMIDIOut::isOutputPortOpen()
{
  return midi_out.is_port_open();
}

void MMGMIDIOut::sendMessage(const MMGMessage *midi)
{
  libremidi::message message;
  int channel = midi->channel();
  int note = midi->note();
  int value = midi->value();

  if (midi->type() == mmgtr("Message.Type.NoteOn")) {
    message = libremidi::message::note_on(channel, note, value);
  } else if (midi->type() == mmgtr("Message.Type.NoteOff")) {
    message = libremidi::message::note_off(channel, note, value);
  } else if (midi->type() == mmgtr("Message.Type.ControlChange")) {
    message = libremidi::message::control_change(channel, note, value);
  } else if (midi->type() == mmgtr("Message.Type.ProgramChange")) {
    message = libremidi::message::program_change(channel, value);
  } else if (midi->type() == mmgtr("Message.Type.PitchBend")) {
    message = libremidi::message::pitch_bend(channel, (value <= 64 ? 0 : value - 64) * 2, value);
  }

  try {
    if (isOutputPortOpen()) midi_out.send_message(message);
  } catch (const libremidi::driver_error &err) {
    global_blog(LOG_INFO, err.what());
  }
}

void MMGMIDIOut::sendThru(const MMGMessage &message)
{
  MMGDevice *device = global()->find(global()->preferences()->thruDevice());
  if (!device) return;

  if (!output_device()->isOutputPortOpen()) output_device()->openOutputPort(device);

  thru_timer->reset(1000);
  sendMessage(&message);
}

const QStringList MMGMIDIOut::outputDeviceNames()
{
  QStringList outputs;
  for (uint i = 0; i < midi_out.get_port_count(); ++i) {
    outputs.append(QString::fromStdString(midi_out.get_port_name(i)));
  }
  return outputs;
}

uint MMGMIDIOut::outputPort(MMGDevice *device)
{
  if (!device) return -1;
  return outputDeviceNames().indexOf(device->name());
}
