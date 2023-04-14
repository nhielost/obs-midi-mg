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

#ifndef MMG_MIDIIN_H
#define MMG_MIDIIN_H

#include "obs-midi-mg.h"
#include "mmg-message.h"

class MMGDevice;

class MMGMIDIIn : public QObject {
  Q_OBJECT

  public:
  void blog(int log_status, const QString &message);

  void openInputPort(MMGDevice *device);
  void closeInputPort();
  bool isInputPortOpen();
  void setListening(bool listen) { listening = listen; };

  const QStringList inputDeviceNames();
  uint inputPort(MMGDevice *device);

  signals:
  void messageListened(const MMGSharedMessage &); // For listen buttons
  void messageReceived(const MMGSharedMessage &); // For action execution
  void sendThru(const MMGMessage &);              // For MIDI throughput

  private:
  libremidi::midi_in midi_in;

  MMGSharedMessage message;
  bool listening = false;

  void callback(const libremidi::message &);

  friend class MMGMIDIOut;
};

#endif // MMG_MIDIIN_H
