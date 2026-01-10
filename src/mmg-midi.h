/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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

#ifndef MMG_MIDI_H
#define MMG_MIDI_H

#include "messages/mmg-message-data.h"

#include <libremidi/libremidi.hpp>

static void inputAdded(const libremidi::input_port &port);
static void inputRemoved(const libremidi::input_port &port);
static void outputAdded(const libremidi::output_port &port);
static void outputRemoved(const libremidi::output_port &port);

class MMGMIDIPort : public QObject {
	Q_OBJECT

public:
	MMGMIDIPort *thru() const { return _thru; };
	void setThru(MMGMIDIPort *device);

	bool isPortOpen(DeviceType type) const;
	bool isCapable(DeviceType type) const;
	QString status(DeviceType type) const;

	void blockReceiver(MMGMessageReceiver *rec, bool block) { blocking_rec = block ? rec : nullptr; };
	void connectReceiver(MMGMessageReceiver *rec, bool connect);

protected:
	MMGMIDIPort(QObject *parent, const QJsonObject &json_obj);

	void blog(int log_status, const QString &message) const;

	void openPort(DeviceType type);
	void closePort(DeviceType type);
	void refreshPortAPI();

public slots:
	void sendMessage(const MMGMessageData &midi) const;

protected:
	QList<MMGMessageReceiver *> recs;
	MMGMessageReceiver *blocking_rec = nullptr;

	MMGMIDIPort *_thru = nullptr;

private:
	std::unique_ptr<libremidi::input_port> in_port_info;
	std::unique_ptr<libremidi::midi_in> midi_in;

	std::unique_ptr<libremidi::output_port> out_port_info;
	std::unique_ptr<libremidi::midi_out> midi_out;

	void callback(const MMGMessageData &incoming);
	void sendThru(const MMGMessageData &incoming);

	friend void inputAdded(const libremidi::input_port &port);
	friend void inputRemoved(const libremidi::input_port &port);
	friend void outputAdded(const libremidi::output_port &port);
	friend void outputRemoved(const libremidi::output_port &port);
	friend void resetMIDIAPI(libremidi_api api);
};

void resetMIDIAPI(libremidi_api api);

#endif // MMG_MIDI_H
