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

#ifndef MMG_DEVICE_H
#define MMG_DEVICE_H

#include "mmg-utils.h"
#include "mmg-midi.h"

#define MMG_ENABLED if (editable)

class MMGBinding;
class MMGDeviceManager;

class MMGDevice : public MMGMIDIPort {
	Q_OBJECT

public:
	MMGDevice(MMGDeviceManager *parent, const QJsonObject &json_obj);

	void json(QJsonObject &device_obj) const;
	void update(const QJsonObject &json_obj);
	void copy(MMGDevice *) const {};
	void setEditable(bool edit) { editable = edit; };

	bool isActive(MMGUtils::DeviceType type) const;
	void setActive(MMGUtils::DeviceType type, bool active);

	void addConnection(MMGBinding *binding);
	void removeConnection(MMGBinding *binding);

signals:
	void deleting(MMGDevice *device);

private:
	uint _active : 2 = 0;

	bool editable = true;
};
using MMGDeviceList = QList<MMGDevice *>;

class MMGDeviceManager : public MMGManager<MMGDevice> {

public:
	MMGDeviceManager(QObject *parent) : MMGManager(parent){};

	MMGDevice *add(const QJsonObject &json_obj = QJsonObject()) override;
	MMGDevice *add(const QString &name);

	bool filter(MMGUtils::DeviceType type, MMGDevice *check) const override;

	const QStringList capableDevices(MMGUtils::DeviceType) const;
};

#undef MMG_ENABLED

#endif // MMG_DEVICE_H
