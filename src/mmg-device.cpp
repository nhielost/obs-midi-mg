/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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

#include "mmg-device.h"
#include "mmg-config.h"

#include "ui/mmg-value-widget.h"

// MMGDevice
MMGDevice::MMGDevice(MMGDeviceManager *parent, const QJsonObject &json_obj) : MMGMIDIPort(parent, json_obj)
{
	update(json_obj);
}

void MMGDevice::json(QJsonObject &device_obj) const
{
	device_obj["name"] = objectName();
	device_obj["active"] = (int)_active;
	device_obj["thru"] = !!_thru ? _thru->objectName() : "";
}

void MMGDevice::update(const QJsonObject &json_obj)
{
	setActive(TYPE_INPUT, json_obj["active"].toInt() & 0b01);
	setActive(TYPE_OUTPUT, json_obj["active"].toInt() & 0b10);
	_thru = manager(device)->find(json_obj["thru"].toString());
}

bool MMGDevice::isActive(DeviceType type) const
{
	switch (type) {
		case TYPE_INPUT:
			return _active & 0b01;

		case TYPE_OUTPUT:
			return _active & 0b10;

		default:
			return _active > 0;
	}
}

void MMGDevice::setActive(DeviceType type, bool active)
{
	if (!editable || isActive(type) == active || !isCapable(type)) return;

	!isActive(type) ? openPort(type) : closePort(type);
	// Is the port not in the correct state?
	if (isPortOpen(type) == isActive(type)) {
		prompt_info("PortOpenError");
		return;
	}

	switch (type) {
		case TYPE_INPUT:
		default:
			_active ^= 0b01;
			break;

		case TYPE_OUTPUT:
			_active ^= 0b10;
			break;
	}

	emit activeStateChanged();
}

void MMGDevice::refreshPort()
{
	uint8_t active = _active;
	_active = 0;

	closePort(TYPE_INPUT);
	closePort(TYPE_OUTPUT);

	refreshPortAPI();

	setActive(TYPE_INPUT, active & 0b01);
	setActive(TYPE_OUTPUT, active & 0b10);
}
// End MMGDevice

MMGDevice *MMGDevice::generate(MMGDeviceManager *parent, const QJsonObject &json_obj)
{
	MMGDevice *current_device = parent->find(json_obj["name"].toString());

	if (current_device) {
		current_device->update(json_obj);
		return nullptr;
	} else {
		if (auto *found = parent->find(mmgtr("Device.Dummy")); !!found) parent->remove(found);
		return new MMGDevice(parent, json_obj);
	}
}
