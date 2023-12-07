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

#include "mmg-device.h"
#include "mmg-config.h"

using namespace MMGUtils;

// MMGDevice
MMGDevice::MMGDevice(MMGDeviceManager *parent, const QJsonObject &json_obj) : MMGMIDIPort(parent, json_obj)
{
	update(json_obj);

	connect(this, &QObject::destroyed, [&]() { emit deleting(this); });
}

void MMGDevice::json(QJsonObject &device_obj) const
{
	device_obj["name"] = _name;
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

	switch (type) {
		case TYPE_INPUT:
			_active ^= 0b01; // 00 -> 01, 10 -> 11, 01 -> 00, 11 -> 10

		case TYPE_OUTPUT:
			_active ^= 0b10; // 00 -> 10, 01 -> 11, 10 -> 00, 11 -> 01

		default:
			break;
	}

	isActive(type) ? openPort(type) : closePort(type);
}

void MMGDevice::addConnection(MMGBinding *binding)
{
	connect(this, &MMGDevice::deleting, binding, &MMGBinding::removeDevice);
}

void MMGDevice::removeConnection(MMGBinding *binding)
{
	disconnect(this, &MMGDevice::deleting, binding, nullptr);
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
		return MMGManager::add(new MMGDevice(this, json_obj));
	}
}

MMGDevice *MMGDeviceManager::add(const QString &name)
{
	QJsonObject json_obj;
	json_obj["name"] = name;
	return add(json_obj);
}

bool MMGDeviceManager::filter(DeviceType type, MMGDevice *check) const
{
	return check->isCapable(type);
}

const QStringList MMGDeviceManager::capableDevices(DeviceType type) const
{
	QStringList devices;
	for (MMGDevice *device : _list) {
		if (device->isCapable(type)) devices += device->name();
	}
	return devices;
}
// End MMGDeviceManager
