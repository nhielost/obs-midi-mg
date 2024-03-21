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

#include "mmg-midi.h"
#include "mmg-config.h"

#include <QMetaMethod>

using namespace MMGUtils;

MMGMIDIPort::MMGMIDIPort(QObject *parent, const QJsonObject &json_obj)
	: QObject(parent),
	  midi_in(midi()->inputConfig(this)),
	  midi_out(midi()->outputConfig())
{
	setObjectName(json_obj["name"].toString(mmgtr("Device.Dummy")));

	//thru_timer = new MMGTimer(this);
	//connect(thru_timer, &MMGTimer::stopping, this, &MMGMIDIPort::closeOutputPort);
}

void MMGMIDIPort::blog(int log_status, const QString &_message) const
{
	global_blog(log_status, QString("[MIDI] <%1> %2").arg(objectName()).arg(_message));
}

void MMGMIDIPort::connectNotify(const QMetaMethod &signal)
{
	if (signal == QMetaMethod::fromSignal(&MMGMIDIPort::messageListened)) {
		listening++;
	} else if (signal == QMetaMethod::fromSignal(&MMGMIDIPort::messageReceived)) {
		connections++;
	}
}

void MMGMIDIPort::disconnectNotify(const QMetaMethod &signal)
{
	if (signal == QMetaMethod::fromSignal(&MMGMIDIPort::messageListened)) {
		listening--;
	} else if (signal == QMetaMethod::fromSignal(&MMGMIDIPort::messageReceived)) {
		connections--;
	}
}

void MMGMIDIPort::openPort(DeviceType type)
{
	switch (type) {
		case TYPE_INPUT:
		default:
			if (midi_in.is_port_open()) return;

			blog(LOG_INFO, "Opening input port...");
			midi_in.open_port(in_port_info);
			if (midi_in.is_port_open()) blog(LOG_INFO, "Input port successfully opened.");
			break;

		case TYPE_OUTPUT:
			if (midi_out.is_port_open()) return;

			blog(LOG_INFO, "Opening output port...");
			midi_out.open_port(out_port_info);
			if (midi_out.is_port_open()) blog(LOG_INFO, "Output port successfully opened.");
			break;
	}
}

void MMGMIDIPort::closePort(DeviceType type)
{
	switch (type) {
		case TYPE_INPUT:
		default:
			if (!midi_in.is_port_open()) return;
			midi_in.close_port();
			blog(LOG_INFO, "Input port closed.");
			break;

		case TYPE_OUTPUT:
			if (!midi_out.is_port_open()) return;
			midi_out.close_port();
			blog(LOG_INFO, "Output port closed.");
			break;
	}
}

bool MMGMIDIPort::isPortOpen(DeviceType type) const
{
	switch (type) {
		case TYPE_INPUT:
		default:
			return midi_in.is_port_open();

		case TYPE_OUTPUT:
			return midi_out.is_port_open();
	}
}

bool MMGMIDIPort::isCapable(DeviceType type) const
{
	switch (type) {
		case TYPE_INPUT:
			return _capable & 0b01;

		case TYPE_OUTPUT:
			return _capable & 0b10;

		default:
			return _capable > 0;
	}
}

void MMGMIDIPort::setCapable(DeviceType type, bool capable)
{
	if (isCapable(type) == capable) return;

	switch (type) {
		case TYPE_INPUT:
			_capable ^= 0b01;
			break;

		case TYPE_OUTPUT:
			_capable ^= 0b10;
			break;

		default:
			break;
	}
}

QString MMGMIDIPort::status(DeviceType type) const
{
	if (!isCapable(type)) return mmgtr("Plugin.Unavailable");
	return mmgtr_two("Device.Status", "Connected", "Disconnected", isPortOpen(type));
}

void MMGMIDIPort::sendMessage(const MMGMessage *midi)
{
	if (!midi_out.is_port_open()) {
		blog(LOG_INFO, "Cannot send message: Output device is not connected. "
			       "(Is the output device enabled?)");
		return;
	}

	libremidi::message _message;
	int channel = midi->channel() - 1;
	int note = midi->note();
	int value = midi->value();

	if (midi->type() == mmgtr("Message.Type.NoteOn")) {
		_message = libremidi::channel_events::note_on(channel, note, value);
	} else if (midi->type() == mmgtr("Message.Type.NoteOff")) {
		_message = libremidi::channel_events::note_off(channel, note, value);
	} else if (midi->type() == mmgtr("Message.Type.ControlChange")) {
		_message = libremidi::channel_events::control_change(channel, note, value);
	} else if (midi->type() == mmgtr("Message.Type.ProgramChange")) {
		_message = libremidi::channel_events::program_change(channel, value);
	} else if (midi->type() == mmgtr("Message.Type.PitchBend")) {
		_message = libremidi::channel_events::pitch_bend(channel, value);
	}

	midi_out.send_message(_message);
}

void MMGMIDIPort::sendThru()
{
	if (_thru.isEmpty()) return;

	MMGMIDIPort *port = manager(device)->find(_thru);
	if (!port) {
		blog(LOG_INFO, "Thru device is not connected or does not exist.");
		return;
	}
	port->sendMessage(message.get());
}

void MMGMIDIPort::callback(const libremidi::message &msg)
{
	message.reset(new MMGMessage(this, msg));

	if (listening) {
		emit messageListened(message);
	} else if (connections) {
		emit messageReceived(message);
		sendThru();
	}
}
// End MMGMIDIPort

// MMGMIDI
MMGMIDI::MMGMIDI(QObject *parent)
	: QObject(parent),
	  observer({.on_error = backendError,
		    .on_warning = backendError,
		    .input_added = [&](libremidi::input_port port) { inputAdded(port); },
		    .input_removed = [&](libremidi::input_port port) { inputRemoved(port); },
		    .output_added = [&](libremidi::output_port port) { outputAdded(port); },
		    .output_removed = [&](libremidi::output_port port) { outputRemoved(port); },
		    .track_virtual = true})
{
}

void MMGMIDI::blog(int log_status, const QString &message) const
{
	global_blog(log_status, "[MIDI] " + message);
}

const libremidi::input_configuration MMGMIDI::inputConfig(MMGMIDIPort *port) const
{
	return {.on_message = [port](libremidi::message &&message) { port->callback(message); },
		.on_error = backendError,
		.on_warning = backendError};
}

const libremidi::output_configuration MMGMIDI::outputConfig() const
{
	return {.on_error = backendError, .on_warning = backendError};
}

void MMGMIDI::inputAdded(const libremidi::input_port &port)
{
	QString port_name = QString::fromStdString(port.port_name);

	MMGDevice *adding_device = manager(device)->find(port_name);
	if (!adding_device) {
		adding_device = manager(device)->add(port_name);
		blog(LOG_INFO, QString("Device <%1> detected.").arg(port_name));
	}

	adding_device->in_port_info = port;
	adding_device->setCapable(TYPE_INPUT, true);
	emit deviceCapableChange();
}

void MMGMIDI::inputRemoved(const libremidi::input_port &port)
{
	QString port_name = QString::fromStdString(port.port_name);

	MMGDevice *removing_device = manager(device)->find(port_name);
	if (!removing_device) return;
	removing_device->setCapable(TYPE_INPUT, false);

	if (!removing_device->isCapable(TYPE_NONE)) return;
	manager(device)->remove(removing_device);
	blog(LOG_INFO, QString("Device <%1> removed.").arg(port_name));
	emit deviceCapableChange();
}

void MMGMIDI::outputAdded(const libremidi::output_port &port)
{
	QString port_name = QString::fromStdString(port.port_name);

	MMGDevice *adding_device = manager(device)->find(port_name);
	if (!adding_device) {
		adding_device = manager(device)->add(port_name);
		blog(LOG_INFO, QString("Device <%1> detected.").arg(port_name));
	}

	adding_device->out_port_info = port;
	adding_device->setCapable(TYPE_OUTPUT, true);
	emit deviceCapableChange();
}

void MMGMIDI::outputRemoved(const libremidi::output_port &port)
{
	QString port_name = QString::fromStdString(port.port_name);

	MMGDevice *removing_device = manager(device)->find(port_name);
	if (!removing_device) return;
	removing_device->setCapable(TYPE_OUTPUT, false);

	if (!removing_device->isCapable(TYPE_NONE)) return;
	manager(device)->remove(removing_device);
	blog(LOG_INFO, QString("Device <%1> removed.").arg(port_name));
	emit deviceCapableChange();
}

void MMGMIDI::backendError(std::string_view msg, const libremidi::source_location &loc)
{
	midi()->blog(LOG_INFO, QString("ERROR: ") + msg.data());
	midi()->blog(LOG_DEBUG,
	     QString("Debug Info: %1 at %2:%3").arg(loc.function_name()).arg(loc.file_name()).arg(loc.line()));
}
// End MMGMIDI
