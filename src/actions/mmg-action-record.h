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

#pragma once
#include "mmg-action.h"

class MMGActionRecord : public MMGAction {
	Q_OBJECT

public:
	MMGActionRecord(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { RECORD_ON, RECORD_OFF, RECORD_TOGGLE_ONOFF, RECORD_PAUSE, RECORD_RESUME, RECORD_TOGGLE_PAUSE };
	enum Events {
		RECORD_STARTING,
		RECORD_STARTED,
		RECORD_STOPPING,
		RECORD_STOPPED,
		RECORD_TOGGLE_STARTING,
		RECORD_TOGGLE_STARTED,
		RECORD_PAUSED,
		RECORD_RESUMED,
		RECORD_TOGGLE_PAUSED
	};

	Category category() const override { return MMGACTION_RECORD; };
	const QString trName() const override { return "Recording"; };
	const QStringList subNames() const override;

	void execute(const MMGMessage *) const override;

private slots:
	void frontendEventReceived(obs_frontend_event event) override;
};
