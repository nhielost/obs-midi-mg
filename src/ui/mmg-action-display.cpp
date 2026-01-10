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

#include "mmg-action-display.h"

namespace MMGWidgets {

MMGParams<MMGActions::Id> MMGActionDisplay::cat_params {
	.desc = mmgtr("Actions.Category"),
	.options = OPTION_NONE,
	.default_value = MMGActions::Id(0x0000),
	.bounds = {},
};

MMGParams<MMGActions::Id> MMGActionDisplay::sub_params {
	.desc = mmgtr("Actions.Name"),
	.options = OPTION_NONE,
	.default_value = MMGActions::Id(0x0000),
	.bounds = {},
};

MMGActionDisplay::MMGActionDisplay(QWidget *parent, MMGStateDisplay *state_display)
	: MMGValueManager(parent, state_display)
{
	state_display->setActionReferences(state_infos);

	cat_params.bounds = MMGActions::availableActionCategories();

	addFixed(&cat, &cat_params, std::bind(&MMGActionDisplay::setCategory, this));
	addFixed(&sub, &sub_params, std::bind(&MMGActionDisplay::setSub, this));
}

void MMGActionDisplay::setStorage(DeviceType action_type, MMGActionManager *parent, MMGAction *storage)
{
	if (_parent == parent && _storage == storage) {
		refreshAll();
		return;
	}

	if (!!_storage) disconnect(_storage, &QObject::destroyed, this, nullptr);
	_parent = parent;
	_storage = storage;
	_type = action_type;
	if (!parent || !storage) {
		clear();
		return;
	}
	connect(_storage, &QObject::destroyed, this, [&]() { _storage = nullptr; });

	cat = MMGActions::Id(storage->id() & 0xff00);
	sub = storage->id();

	resetAction();
}

void MMGActionDisplay::setCategory()
{
	if (!_storage) return;
	sub_params.bounds = MMGActions::availableActions(cat, _type);
	sub_params.default_value = sub_params.bounds.firstKey();
}

void MMGActionDisplay::setSub()
{
	if (MMGActions::changeAction(_parent, _storage, sub)) {
		resetAction();
		emit actionChanged();
	}
}

void MMGActionDisplay::resetAction()
{
	clear();

	_storage->createDisplay(this);
	refresh_sender = nullptr;
	refreshAll();
}

} // namespace MMGWidgets
