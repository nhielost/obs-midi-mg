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

class MMGActionCollections : public MMGAction {
	Q_OBJECT

public:
	MMGActionCollections(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { COLLECTION_COLLECTION };
	enum Events { COLLECTION_CHANGING, COLLECTION_CHANGED, COLLECTION_TOGGLE_CHANGING };

	Category category() const override { return MMGACTION_COLLECTION; };
	const QString trName() const override { return "Collections"; };
	const QStringList subNames() const override;

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;
	void setEditable(bool edit) override;
	void toggle() override;

	void createDisplay(QWidget *parent) override;
	void setActionParams() override;

	void execute(const MMGMessage *midi) const override;

	static const QStringList enumerate();
	static const QString currentCollection();

private:
	MMGUtils::MMGString collection;

private slots:
	void frontendEventReceived(obs_frontend_event) override;
};
