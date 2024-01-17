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

#ifndef MMG_MIDI_H
#define MMG_MIDI_H

#include "obs-midi-mg.h"
#include "mmg-message.h"

class MMGMIDIPort : public QObject {
	Q_OBJECT

public:
	const QString &thru() const { return _thru; };
	void setThru(const QString &device) { _thru = device; };

	bool isListening() const { return listening > 0; };
	void incListening(bool listen);

	void incConnection(bool connect);

	bool isPortOpen(MMGUtils::DeviceType type) const;
	bool isCapable(MMGUtils::DeviceType type) const;
	QString status(MMGUtils::DeviceType type) const;

protected:
	MMGMIDIPort(QObject *parent, const QJsonObject &json_obj);

	void blog(int log_status, const QString &message) const;

	void openPort(MMGUtils::DeviceType type);
	void closePort(MMGUtils::DeviceType type);

signals:
	void messageListened(const MMGSharedMessage &);
	void messageReceived(const MMGSharedMessage &);

public slots:
	void sendMessage(const MMGMessage *midi);

protected:
	QString _thru;
	uint _capable : 2 = 0;

	int listening = 0;
	int connections = 0;

	void setCapable(MMGUtils::DeviceType type, bool capable);

private:
	libremidi::input_port in_port_info;
	libremidi::midi_in midi_in;

	libremidi::output_port out_port_info;
	libremidi::midi_out midi_out;

	MMGSharedMessage message;

	void callback(const libremidi::message &msg);
	void sendThru();

	friend class MMGMIDI;
};

class MMGMIDI : public QObject {
	Q_OBJECT

public:
	MMGMIDI(QObject *parent);

	void blog(int log_status, const QString &message) const;

	const libremidi::input_configuration inputConfig(MMGMIDIPort *port) const;
	const libremidi::output_configuration outputConfig() const;

signals:
	void deviceCapableChange();

private:
	libremidi::observer observer;

	void inputAdded(const libremidi::input_port &port);
	void inputRemoved(const libremidi::input_port &port);
	void outputAdded(const libremidi::output_port &port);
	void outputRemoved(const libremidi::output_port &port);

	void backendError(libremidi::midi_error, std::string_view) const;
};

#endif // MMG_MIDI_H
