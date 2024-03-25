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

class MMGActionVirtualCam : public MMGAction {
	Q_OBJECT

public:
	MMGActionVirtualCam(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { VIRCAM_ON, VIRCAM_OFF, VIRCAM_TOGGLE_ONOFF };
	enum Events { VIRCAM_STARTED, VIRCAM_STOPPED, VIRCAM_TOGGLE_STARTED };

	Category category() const override { return MMGACTION_VIRCAM; };
	const QString trName() const override { return "VirtualCamera"; };
	const QStringList subNames() const override;

	void execute(const MMGMessage *) const override;

private slots:
	void frontendEventReceived(obs_frontend_event event) override;
};
