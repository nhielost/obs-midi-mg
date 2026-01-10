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

#ifndef MMG_ACTION_DISPLAY_H
#define MMG_ACTION_DISPLAY_H

#include "../actions/mmg-action.h"
#include "mmg-value-manager.h"

namespace MMGWidgets {

class MMGActionDisplay : public MMGValueManager {
	Q_OBJECT

public:
	MMGActionDisplay(QWidget *parent, MMGStateDisplay *state_display);

	void setStorage(DeviceType action_type, MMGActionManager *parent, MMGAction *storage);

signals:
	void actionChanged();

private:
	void setCategory();
	void setSub();
	void resetAction();

private:
	MMGActionManager *_parent = nullptr;
	MMGAction *_storage = nullptr;
	DeviceType _type;

	MMGValue<MMGActions::Id> cat;
	MMGValue<MMGActions::Id> sub;

	static MMGParams<MMGActions::Id> cat_params;
	static MMGParams<MMGActions::Id> sub_params;
};

} // namespace MMGWidgets

#endif // MMG_ACTION_DISPLAY_H
