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

#ifndef MMG_DEVICE_H
#define MMG_DEVICE_H

#include "mmg-midi.h"

class MMGDevice;
template <class T> class MMGManager;
using MMGDeviceManager = MMGManager<MMGDevice>;

class MMGDevice : public MMGMIDIPort {
	Q_OBJECT

public:
	MMGDevice(MMGDeviceManager *parent, const QJsonObject &json_obj);

	void json(QJsonObject &device_obj) const;
	void update(const QJsonObject &json_obj);
	void copy(MMGDevice *) const {};

	bool isEditable() const { return editable; };
	void setEditable(bool edit) { editable = edit; };

	bool isActive(DeviceType type) const;
	void setActive(DeviceType type, bool active);

	void refreshPort();

	static MMGDevice *generate(MMGDeviceManager *parent, const QJsonObject &json_obj);

signals:
	void activeStateChanged();

private:
	uint8_t _active : 2 = 0;

	bool editable = true;
};

#endif // MMG_DEVICE_H
