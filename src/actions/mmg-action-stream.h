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

class MMGActionStream : public MMGAction {

public:
	MMGActionStream(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { STREAM_ON, STREAM_OFF, STREAM_TOGGLE_ONOFF };
	enum Events {
		STREAM_STARTING,
		STREAM_STARTED,
		STREAM_STOPPING,
		STREAM_STOPPED,
		STREAM_TOGGLE_STARTING,
		STREAM_TOGGLE_STARTED
	};

	Category category() const override { return MMGACTION_STREAM; };
	const QString trName() const override { return "Streaming"; };
	const QStringList subNames() const override;

	void execute(const MMGMessage *) const override;

private slots:
	void frontendEventReceived(obs_frontend_event event) override;
};
