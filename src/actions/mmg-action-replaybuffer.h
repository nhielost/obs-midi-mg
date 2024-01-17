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

class MMGActionReplayBuffer : public MMGAction {
	Q_OBJECT

public:
	MMGActionReplayBuffer(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { REPBUF_ON, REPBUF_OFF, REPBUF_TOGGLE_ONOFF, REPBUF_SAVE };
	enum Events {
		REPBUF_STARTING,
		REPBUF_STARTED,
		REPBUF_STOPPING,
		REPBUF_STOPPED,
		REPBUF_TOGGLE_STARTING,
		REPBUF_TOGGLE_STARTED,
		REPBUF_SAVED
	};

	Category category() const override { return MMGACTION_REPBUF; };
	const QString trName() const override { return "ReplayBuffer"; };
	const QStringList subNames() const override;

	void execute(const MMGMessage *midi) const override;
	void connectOBSSignals() override;
	void disconnectOBSSignals() override;

private:
	void frontendCallback(obs_frontend_event event) const;
};
