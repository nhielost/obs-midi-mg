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
#include "../mmg-obs-object.h"
#include "mmg-action.h"

#include <QTimer>

namespace MMGActions {

const MMGStringTranslationMap enumerateTransitions();
const MMGString findTransitionId(const MMGString &name);
MMGString currentTransition();

class MMGActionTransitionsCurrent : public MMGAction, public MMGSignal::MMGFrontendReceiver {
	Q_OBJECT

public:
	MMGActionTransitionsCurrent(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1401); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "Transitions"; };
	const char *trActionName() const override { return "CurrentChange"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onTransitionChanged() const;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(obs_frontend_event event) const override;

private:
	MMGStringID transition;
	MMGInteger duration;

	static MMGParams<int32_t> duration_params;
};
MMG_DECLARE_ACTION(MMGActionTransitionsCurrent);

class MMGActionTransitionsTBar : public MMGAction, public MMGSignal::MMGFrontendReceiver {
	Q_OBJECT

public:
	MMGActionTransitionsTBar(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1481); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "Transitions"; };
	const char *trActionName() const override { return "TBarChange"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(obs_frontend_event event) const override;

private:
	MMGInteger tbar;
	MMGInteger held_duration;

	static MMGParams<int32_t> tbar_params;
	static MMGParams<int32_t> held_duration_params;

	static struct Timer {
		Timer()
		{
			timer = new QTimer;
			timer->setSingleShot(true);
			timer->callOnTimeout(obs_frontend_release_tbar);
		}
		~Timer() { delete timer; };

		void restart(int ms) { timer->start(ms); };

		QTimer *timer;
	} tbar_timer;
};
MMG_DECLARE_ACTION(MMGActionTransitionsTBar);

class MMGActionTransitionsCustom : public MMGAction, public MMGSignal::MMGSourceReceiver {
	Q_OBJECT

public:
	MMGActionTransitionsCustom(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x14ff); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "Transitions"; };
	const char *trActionName() const override { return "Custom"; };

	MMGString sourceId() const final override { return sourceFromName(); };
	const char *sourceSignalName() const final override { return "update"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onTransitionChanged() const;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(const calldata_t *cd) const override;

private:
	MMGStringID transition;
	MMGOBSFields::MMGOBSObject *custom_data;

	MMGString sourceFromName() const { return findTransitionId(transition); }
};
MMG_DECLARE_ACTION(MMGActionTransitionsCustom);

} // namespace MMGActions
