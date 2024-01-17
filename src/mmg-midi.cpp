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

using namespace MMGUtils;

MMGMIDIPort::MMGMIDIPort(QObject *parent, const QJsonObject &json_obj)
	: QObject(parent),
	  _name(json_obj["name"].toString()),
	  midi_in(midi()->inputConfig(this)),
	  midi_out(midi()->outputConfig())
{
	//thru_timer = new MMGTimer(this);

	//connect(thru_timer, &MMGTimer::stopping, this, &MMGMIDIPort::closeOutputPort);
}

void MMGMIDIPort::blog(int log_status, const QString &_message) const
{
	global_blog(log_status, QString("[MIDI] <%1> %2").arg(_name).arg(_message));
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

		case TYPE_OUTPUT:
			_capable ^= 0b10;

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
		blog(LOG_INFO, "Device is not connected or does not exist.");
		return;
	}
	port->sendMessage(message.data());
}

void MMGMIDIPort::callback(const libremidi::message &msg)
{
	message.reset(new MMGMessage(msg));

	if (midi()->isListening()) {
		emit midi()->messageListened(message);
	} else {
		emit messageReceived(message);
		sendThru();
	}
}
// End MMGMIDIPort

// MMGMIDI
MMGMIDI::MMGMIDI(QObject *parent)
	: QObject(parent),
	  observer({
		  .on_error = [&](libremidi::midi_error e, std::string_view str) { backendError(e, str); },
		  .on_warning = [&](libremidi::midi_error e, std::string_view str) { backendError(e, str); },
		  .input_added = [&](libremidi::input_port port) { inputAdded(port); },
		  .input_removed = [&](libremidi::input_port port) { inputRemoved(port); },
		  .output_added = [&](libremidi::output_port port) { outputAdded(port); },
		  .output_removed = [&](libremidi::output_port port) { outputRemoved(port); },
	  })
{
}

void MMGMIDI::blog(int log_status, const QString &message) const
{
	global_blog(log_status, "[MIDI] " + message);
}

const libremidi::input_configuration MMGMIDI::inputConfig(MMGMIDIPort *port) const
{
	return {
		.on_message = [port](libremidi::message &&message) { port->callback(message); },
		.on_error = [&](libremidi::midi_error e, std::string_view str) { backendError(e, str); },
		.on_warning = [&](libremidi::midi_error e, std::string_view str) { backendError(e, str); },
	};
}

const libremidi::output_configuration MMGMIDI::outputConfig() const
{
	return {
		.on_error = [&](libremidi::midi_error e, std::string_view str) { backendError(e, str); },
		.on_warning = [&](libremidi::midi_error e, std::string_view str) { backendError(e, str); },
	};
}

void MMGMIDI::inputAdded(const libremidi::input_port &port)
{
	QString port_name = QString::fromStdString(port.port_name);

	MMGDevice *adding_device = manager(device)->find(port_name);
	if (!adding_device) {
		adding_device = manager(device)->add(port_name);
		blog(LOG_INFO, QString("New device <%1> detected.").arg(port_name));
	}

	adding_device->in_port_info = port;
	adding_device->setCapable(TYPE_INPUT, true);
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
}

void MMGMIDI::outputAdded(const libremidi::output_port &port)
{
	QString port_name = QString::fromStdString(port.port_name);

	MMGDevice *adding_device = manager(device)->find(port_name);
	if (!adding_device) {
		adding_device = manager(device)->add(port_name);
		blog(LOG_INFO, QString("New device <%1> detected.").arg(port_name));
	}

	adding_device->out_port_info = port;
	adding_device->setCapable(TYPE_OUTPUT, true);
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
}

void MMGMIDI::backendError(libremidi::midi_error error, std::string_view string) const
{
	QString err_string = string.data();

	switch (error) {
		case libremidi::WARNING:
			blog(LOG_INFO, "{WARNING} " + err_string);
			break;

		case libremidi::UNSPECIFIED:
		default:
			blog(LOG_INFO, "{ERROR (Unknown)} " + err_string);
			break;

		case libremidi::NO_DEVICES_FOUND:
			blog(LOG_INFO, "{ERROR (No Devices)} " + err_string);
			break;

		case libremidi::INVALID_DEVICE:
		case libremidi::INVALID_PARAMETER:
		case libremidi::INVALID_USE:
			blog(LOG_INFO, "{ERROR (Invalid Usage)} " + err_string);
			break;

		case libremidi::MEMORY_ERROR:
			blog(LOG_INFO, "{ERROR (Memory)} " + err_string);
			break;

		case libremidi::DRIVER_ERROR:
		case libremidi::SYSTEM_ERROR:
		case libremidi::THREAD_ERROR:
			blog(LOG_INFO, "{ERROR (System)} " + err_string);
			break;
	}
}
// End MMGMIDI

MMGMIDI *midi()
{
	return config()->midi();
}