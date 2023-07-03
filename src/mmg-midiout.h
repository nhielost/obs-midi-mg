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

#ifndef MMG_MIDIOUT_H
#define MMG_MIDIOUT_H

#include "obs-midi-mg.h"
#include "mmg-message.h"

class MMGDevice;

class MMGMIDIOut : public QObject {
  Q_OBJECT

  public:
  MMGMIDIOut();
  void blog(int log_status, const QString &message) const;

  bool isOutputPortOpen();

  const QStringList outputDeviceNames();
  uint outputPort(MMGDevice *device);

  public slots:
  void openOutputPort(MMGDevice *device);
  void closeOutputPort();
  void sendMessage(const MMGMessage *midi);

  private slots:
  void sendThru(MMGDevice *);

  private:
  libremidi::midi_out midi_out;
  MMGUtils::MMGTimer *thru_timer;

  friend class MMGMIDIIn;
};

#endif // MMG_MIDIOUT_H
