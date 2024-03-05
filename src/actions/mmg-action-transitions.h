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

class MMGActionTransitions : public MMGAction {
	Q_OBJECT

public:
	MMGActionTransitions(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions {
		TRANSITION_CURRENT,
		TRANSITION_SOURCE_SHOW,
		TRANSITION_SOURCE_HIDE,
		TRANSITION_TBAR_ACTIVATE,
		TRANSITION_TBAR_TOGGLE,
		TRANSITION_CUSTOM
	};
	enum Events {
		TRANSITION_CURRENT_CHANGED,
		TRANSITION_CURRENT_DURATION_CHANGED,
		TRANSITION_STARTED,
		TRANSITION_STOPPED,
		TRANSITION_TOGGLE_STARTED,
		TRANSITION_TBAR_CHANGED,
		TRANSITION_CUSTOM_CHANGED
	};

	Category category() const override { return MMGACTION_TRANSITION; };
	const QString trName() const override { return "Transitions"; };
	const QStringList subNames() const override;

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;
	void setEditable(bool edit) override;
	void toggle() override;

	void createDisplay(QWidget *parent) override;
	void setActionParams() override;

	void execute(const MMGMessage *midi) const override;
	void connectSignals(bool connect) override;

	static const QStringList enumerate();
	static const QString currentTransition();
	static obs_source_t *sourceByName(const QString &name);
	bool transitionFixed() const;

private:
	MMGUtils::MMGString transition;
	MMGUtils::MMGString parent_scene;
	MMGUtils::MMGString source;
	MMGUtils::MMGNumber num;
	MMGUtils::MMGJsonObject *_json;

private slots:
	void onList1Change();
	void onList2Change();
	void onList3Change();

	void frontendEventReceived(obs_frontend_event event) override;
	void sourceEventReceived(MMGSourceSignal::Event, QVariant) override;
};
