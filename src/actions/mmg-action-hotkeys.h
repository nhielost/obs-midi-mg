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

#pragma once
#include "mmg-action.h"

namespace MMGActions {

const MMGStringTranslationMap enumerateHotkeys(const MMGString &category);
const MMGStringTranslationMap enumerateHotkeyCategories();

class MMGActionHotkeys : public MMGAction, public MMGSignal::MMGHotkeyReceiver {
	Q_OBJECT

public:
	MMGActionHotkeys(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1601); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "Hotkeys"; };
	const char *trActionName() const override { return "Activate"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onGroupChange() const;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(obs_hotkey_id id) const override;

private:
	MMGStringID hotkey_group;
	MMGStringID hotkey;

	mutable struct Request {
		obs_hotkey_id id;
		MMGString name;
		bool found = false;
	} hotkey_req;

	static MMGParams<MMGString> group_params;
	static MMGParams<MMGString> hotkey_params;
};
MMG_DECLARE_ACTION(MMGActionHotkeys);

} // namespace MMGActions
