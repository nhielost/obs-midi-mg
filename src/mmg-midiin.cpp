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

#include "mmg-midiin.h"
#include "mmg-config.h"

using namespace MMGUtils;

void MMGMIDIIn::blog(int log_status, const QString &message)
{
  global_blog(log_status, "[MIDI In] " + message);
}

void MMGMIDIIn::openInputPort(MMGDevice *device)
{
  if (isInputPortOpen()) closeInputPort();

  blog(LOG_INFO,
       QString::asprintf("Opening input port for device <%s>...", device->name().qtocs()));

  if (inputPort(device) == (uint)-1) {
    blog(LOG_INFO, "Input port opening failed: Device is disconnected or does not exist.");
    return;
  }

  try {
    midi_in.set_callback([&](const libremidi::message &message) { callback(message); });
    midi_in.open_port(inputPort(device));
    blog(LOG_INFO, "Input port successfully opened.");
  } catch (const libremidi::driver_error &err) {
    blog(LOG_INFO, err.what());
  } catch (const libremidi::invalid_parameter_error &err) {
    blog(LOG_INFO, err.what());
  } catch (const libremidi::system_error &err) {
    blog(LOG_INFO, err.what());
  }
}

void MMGMIDIIn::closeInputPort()
{
  if (!isInputPortOpen()) return;
  midi_in.cancel_callback();
  midi_in.close_port();
  blog(LOG_INFO, "Input port closed.");
}

bool MMGMIDIIn::isInputPortOpen()
{
  return midi_in.is_port_open();
}

const QStringList MMGMIDIIn::inputDeviceNames()
{
  QStringList inputs;
  for (uint i = 0; i < midi_in.get_port_count(); ++i) {
    inputs.append(QString::fromStdString(midi_in.get_port_name(i)));
  }
  return inputs;
}

uint MMGMIDIIn::inputPort(MMGDevice *device)
{
  if (!device) return -1;
  return inputDeviceNames().indexOf(device->name());
}

void MMGMIDIIn::callback(const libremidi::message &msg)
{
  message.reset(new MMGMessage(msg));

  if (listening) {
    emit messageListened(message);
  } else {
    emit messageReceived(message);

    if (global()->preferences()->thruDevice() != mmgtr("Plugin.Disabled"))
      emit sendThru(*message.get());
  }
}
