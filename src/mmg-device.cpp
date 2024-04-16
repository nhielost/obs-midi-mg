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

#include "mmg-device.h"
#include "mmg-config.h"

using namespace MMGUtils;

// MMGDevice
MMGDevice::MMGDevice(MMGDeviceManager *parent, const QJsonObject &json_obj) : MMGMIDIPort(parent, json_obj)
{
	update(json_obj);
}

void MMGDevice::json(QJsonObject &device_obj) const
{
	device_obj["name"] = objectName();
	device_obj["active"] = (int)_active;
	if (!_thru.isEmpty()) device_obj["thru"] = _thru;
}

void MMGDevice::update(const QJsonObject &json_obj)
{
	setActive(TYPE_INPUT, json_obj["active"].toInt() & 0b01);
	setActive(TYPE_OUTPUT, json_obj["active"].toInt() & 0b10);
	_thru = json_obj["thru"].toString();
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
		MMGInterface::promptUser("PortOpenError");
		return;
	}

	switch (type) {
		case TYPE_INPUT:
			_active ^= 0b01;
			break;

		case TYPE_OUTPUT:
			_active ^= 0b10;
			break;

		default:
			break;
	}
}

void MMGDevice::checkCapable()
{
	uint active = _active;
	_active = 0;

	blog(LOG_INFO, "Checking device capabilities...");

	closePort(TYPE_INPUT);
	closePort(TYPE_OUTPUT);

	openPort(TYPE_INPUT);
	setCapable(TYPE_INPUT, isPortOpen(TYPE_INPUT));

	openPort(TYPE_OUTPUT);
	setCapable(TYPE_OUTPUT, isPortOpen(TYPE_OUTPUT));

	closePort(TYPE_INPUT);
	closePort(TYPE_OUTPUT);

	blog(LOG_INFO, "Device capabilities checked. Re-opening active ports...");

	setActive(TYPE_INPUT, active & 0b01);
	setActive(TYPE_OUTPUT, active & 0b10);
}
// End MMGDevice

// MMGDeviceManager
MMGDevice *MMGDeviceManager::add(const QJsonObject &json_obj)
{
	MMGDevice *current_device = find(json_obj["name"].toString());

	if (current_device) {
		current_device->update(json_obj);
		return current_device;
	} else {
		if (find(mmgtr("Device.Dummy"))) remove(find(mmgtr("Device.Dummy")));
		return MMGManager::add(new MMGDevice(this, json_obj));
	}
}

MMGDevice *MMGDeviceManager::add(const QString &name)
{
	QJsonObject json_obj;
	json_obj["name"] = name;
	return add(json_obj);
}

const QStringList MMGDeviceManager::capableDevices(DeviceType type) const
{
	QStringList devices;
	for (MMGDevice *device : _list) {
		if (device->isCapable(type)) devices += device->objectName();
	}
	return devices;
}
// End MMGDeviceManager
