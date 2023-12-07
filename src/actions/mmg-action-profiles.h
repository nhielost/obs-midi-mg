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

class MMGActionProfiles : public MMGAction {
	Q_OBJECT

public:
	MMGActionProfiles(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { PROFILE_PROFILE };
	enum Events { PROFILE_CHANGING, PROFILE_CHANGED, PROFILE_TOGGLE_CHANGING };

	Category category() const override { return MMGACTION_PROFILE; };
	const QString trName() const override { return "Profiles"; };

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;
	void setEditable(bool edit) override;
	void toggle() override;

	void createDisplay(QWidget *parent) override;
	void setComboOptions(QComboBox *sub) override;
	void setActionParams() override;

	void execute(const MMGMessage *midi) const override;
	void connectOBSSignals() override;
	void disconnectOBSSignals() override;

	static const QStringList enumerate();
	static const QString currentProfile();

private:
	MMGUtils::MMGString profile;

private slots:
	void frontendCallback(obs_frontend_event event) const;
};
