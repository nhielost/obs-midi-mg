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

#pragma once
#include "../messages/mmg-message.h"
#include "mmg-action.h"

class MMGDevice;

namespace MMGActions {

class MMGActionMIDISend : public MMGAction {
	Q_OBJECT

public:
	MMGActionMIDISend(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0xf001); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "MIDI"; };
	const char *trActionName() const override { return "Input.Message"; };

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;

private:
	void execute(const MMGMappingTest &test) const override;

	void connectSignal(bool) override {};

private:
	MMGMessageManager *messages;
};
MMG_DECLARE_ACTION(MMGActionMIDISend);

class MMGActionMIDIConnection : public MMGAction {
	Q_OBJECT

public:
	MMGActionMIDIConnection(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0xf011); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "MIDI"; };
	const char *trActionName() const override { return "Connection"; };

	MMGDevice *device() const;

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onDeviceChange();

private:
	void execute(const MMGMappingTest &test) const override;

	void connectSignal(bool connect) override;
	void processMIDIState();

private:
	MMGStringID _device;
	MMGBoolean in_status;
	MMGBoolean out_status;

	QMetaObject::Connection device_connection;
};
MMG_DECLARE_ACTION(MMGActionMIDIConnection);

} // namespace MMGActions
