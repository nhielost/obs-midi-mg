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

#include "mmg-midi.h"
#include "mmg-config.h"
#include "mmg-preference-defs.h"

static std::unique_ptr<libremidi::observer> observer;
static bool api_changing = false;

static void midiblog(int log_status, const QString &message)
{
	mmgblog(log_status, "[MIDI] " + message);
}

static libremidi_api getCurrentAPI()
{
	return libremidi_api(MMGPreferences::MMGPreferenceMIDI::currentAPI());
}

static void backendError(std::string_view msg, const libremidi::source_location &loc)
{
	midiblog(LOG_INFO, QString("ERROR: ") + msg.data());
	midiblog(LOG_DEBUG,
		 QString("Debug Info: %1 at %2:%3").arg(loc.function_name()).arg(loc.file_name()).arg(loc.line()));
}

static QString getPortDisplayName(const libremidi::port_information &pi)
{
	switch (getCurrentAPI()) {
		case WINDOWS_MM:
		case WINDOWS_MIDI_SERVICES:
			return QString::fromStdString(pi.port_name);

		default:
			return QString::fromStdString(pi.display_name);
	}
}

static MMGDevice *addDetectedDevice(const QString &name)
{
	QJsonObject json_obj;
	json_obj["name"] = name;
	return manager(device)->add(json_obj);
}

static void inputAdded(const libremidi::input_port &port)
{
	QString port_name = getPortDisplayName(port);

	MMGDevice *adding_device = manager(device)->find(port_name);
	if (!adding_device) {
		adding_device = addDetectedDevice(port_name);
		midiblog(LOG_INFO, QString("Device <%1> detected.").arg(port_name));
	}

	adding_device->in_port_info.reset(new libremidi::input_port(port));
	adding_device->refreshPort();
	if (!api_changing) emit config() -> midiStateChanged();
}

static void inputRemoved(const libremidi::input_port &port)
{
	QString port_name = getPortDisplayName(port);

	MMGDevice *removing_device = manager(device)->find(port_name);
	if (!removing_device) return;

	removing_device->in_port_info.reset();
	removing_device->refreshPort();
	if (!removing_device->isCapable(TYPE_NONE)) return;

	manager(device)->remove(removing_device);
	midiblog(LOG_INFO, QString("Device <%1> removed.").arg(port_name));
	if (!api_changing) emit config() -> midiStateChanged();
}

static void outputAdded(const libremidi::output_port &port)
{
	QString port_name = getPortDisplayName(port);

	MMGDevice *adding_device = manager(device)->find(port_name);
	if (!adding_device) {
		adding_device = addDetectedDevice(port_name);
		midiblog(LOG_INFO, QString("Device <%1> detected.").arg(port_name));
	}

	adding_device->out_port_info.reset(new libremidi::output_port(port));
	adding_device->refreshPort();
	if (!api_changing) emit config() -> midiStateChanged();
}

static void outputRemoved(const libremidi::output_port &port)
{
	QString port_name = getPortDisplayName(port);

	MMGDevice *removing_device = manager(device)->find(port_name);
	if (!removing_device) return;

	removing_device->out_port_info.reset();
	removing_device->refreshPort();
	if (!removing_device->isCapable(TYPE_NONE)) return;

	manager(device)->remove(removing_device);
	midiblog(LOG_INFO, QString("Device <%1> removed.").arg(port_name));
	if (!api_changing) emit config() -> midiStateChanged();
}

template <typename T> static void callInMainThread(const std::function<void(const T &)> f, const T &arg)
{
	runInMainThread(std::bind(f, arg));
}

void resetMIDIAPI(libremidi_api api)
{
	auto &p1 = std::placeholders::_1;
	api_changing = true;

	for (MMGMIDIPort *device : *manager(device)) {
		device->in_port_info.reset();
		device->out_port_info.reset();
	}

	observer.reset(new libremidi::observer(
		{
			.on_error = backendError,
			.on_warning = backendError,
			.input_added = std::bind(callInMainThread<libremidi::input_port>, inputAdded, p1),
			.input_removed = std::bind(callInMainThread<libremidi::input_port>, inputRemoved, p1),
			.output_added = std::bind(callInMainThread<libremidi::output_port>, outputAdded, p1),
			.output_removed = std::bind(callInMainThread<libremidi::output_port>, outputRemoved, p1),
			.track_virtual = true,
			.track_any = true,
		},
		api));

	for (MMGDevice *device : *manager(device))
		device->refreshPort();

	api_changing = false;
	emit config() -> midiStateChanged();
}

// MMGMIDIPort
MMGMIDIPort::MMGMIDIPort(QObject *parent, const QJsonObject &json_obj) : QObject(parent)
{
	setObjectName(json_obj["name"].toString(mmgtr("Device.Dummy")));
}

void MMGMIDIPort::blog(int log_status, const QString &_message) const
{
	mmgblog(log_status, QString("[MIDI] <%1> %2").arg(objectName()).arg(_message));
}

void MMGMIDIPort::openPort(DeviceType type)
{
	if (!isCapable(type)) return;

	switch (type) {
		case TYPE_INPUT:
		default:
			if (midi_in->is_port_open()) return;

			blog(LOG_INFO, "Opening input port...");
			midi_in->open_port(*in_port_info);
			if (midi_in->is_port_open()) blog(LOG_INFO, "Input port successfully opened.");
			break;

		case TYPE_OUTPUT:
			if (midi_out->is_port_open()) return;

			blog(LOG_INFO, "Opening output port...");
			midi_out->open_port(*out_port_info);
			if (midi_out->is_port_open()) blog(LOG_INFO, "Output port successfully opened.");
			break;
	}
}

void MMGMIDIPort::closePort(DeviceType type)
{
	if (!isCapable(type)) return;

	switch (type) {
		case TYPE_INPUT:
		default:
			if (!midi_in || !midi_in->is_port_open()) return;
			midi_in->close_port();
			blog(LOG_INFO, "Input port closed.");
			break;

		case TYPE_OUTPUT:
			if (!midi_out || !midi_out->is_port_open()) return;
			midi_out->close_port();
			blog(LOG_INFO, "Output port closed.");
			break;
	}
}

bool MMGMIDIPort::isPortOpen(DeviceType type) const
{
	switch (type) {
		case TYPE_INPUT:
		default:
			return !!midi_in && midi_in->is_port_open();

		case TYPE_OUTPUT:
			return !!midi_out && midi_out->is_port_open();
	}
}

bool MMGMIDIPort::isCapable(DeviceType type) const
{
	switch (type) {
		case TYPE_INPUT:
			return bool(in_port_info);

		case TYPE_OUTPUT:
			return bool(out_port_info);

		default:
			return bool(in_port_info) || bool(out_port_info);
	}
}

QString MMGMIDIPort::status(DeviceType type) const
{
	if (!isCapable(type)) return mmgtr("Plugin.Unavailable");
	return mmgtr(choosetr("Device.Status", "Connected", "Disconnected", isPortOpen(type)));
}

void MMGMIDIPort::connectReceiver(MMGMessageReceiver *rec, bool connect)
{
	if (connect) {
		if (!recs.contains(rec)) recs += rec;
	} else {
		recs.removeOne(rec);
	}
}

void MMGMIDIPort::sendMessage(const MMGMessageData &midi) const
{
	if (!midi_out->is_port_open()) {
		blog(LOG_INFO, "Cannot send message: Output device is not connected. "
			       "(Is the output device enabled?)");
		return;
	}

	if (MMGMessages::usingMIDI2()) {
		midi_out->send_ump(midi);
	} else {
		midi_out->send_message(midi);
	}
}

void MMGMIDIPort::refreshPortAPI()
{
	if (MMGMessages::usingMIDI2()) {
		midi_in.reset(new libremidi::midi_in(
			{
				.on_message = [this](libremidi::ump &&incoming) { callback(MMGMessageData(incoming)); },
				.on_error = backendError,
				.on_warning = backendError,
			},
			getCurrentAPI()));
	} else {
		midi_in.reset(new libremidi::midi_in(
			{
				.on_message =
					[this](libremidi::message &&incoming) { callback(MMGMessageData(incoming)); },
				.on_error = backendError,
				.on_warning = backendError,
			},
			getCurrentAPI()));
	}

	midi_out.reset(new libremidi::midi_out(
		{
			.on_error = backendError,
			.on_warning = backendError,
		},
		getCurrentAPI()));
}

void MMGMIDIPort::setThru(MMGMIDIPort *device)
{
	if (!!_thru) disconnect(_thru, &QObject::destroyed, this, nullptr);

	_thru = device;
	if (!device) return;

	connect(_thru, &QObject::destroyed, this, [&]() { _thru = nullptr; });
}

void MMGMIDIPort::sendThru(const MMGMessageData &incoming)
{
	if (!_thru) return;
	_thru->sendMessage(incoming);
}

void MMGMIDIPort::callback(const MMGMessageData &incoming)
{
	if (!incoming.isCV()) return; // Only using Channel Voice Messages

	if (!!blocking_rec) {
		blocking_rec->processMessage(incoming);
		return;
	};

	for (auto *rec : recs)
		rec->processMessage(incoming);

	sendThru(incoming);
}
// End MMGMIDIPort
