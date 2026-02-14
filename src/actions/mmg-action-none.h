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
#include "mmg-action.h"

namespace MMGActions {

class MMGActionNone : public MMGAction {
	Q_OBJECT

public:
	MMGActionNone(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x0000); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const final override { return "None"; };
	const char *trActionName() const final override { return "None"; };

private:
	void execute(const MMGMappingTest &) const override { blog(LOG_DEBUG, "Successfully executed."); };

	void connectSignal(bool) final override {};
};
MMG_DECLARE_ACTION(MMGActionNone);

} // namespace MMGActions
