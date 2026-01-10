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

namespace MMGActions {

class MMGActionStudioModeRunState : public MMGAction, public MMGSignal::MMGFrontendReceiver {
	Q_OBJECT

public:
	MMGActionStudioModeRunState(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1001); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "StudioMode"; };
	const char *trActionName() const override { return "ChangeState"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(obs_frontend_event event) const override;

private:
	MMGBoolean studio_state;

	static const MMGParams<bool> studio_params;
};
MMG_DECLARE_ACTION(MMGActionStudioModeRunState);

class MMGActionStudioModePreview : public MMGAction, public MMGSignal::MMGFrontendReceiver {
	Q_OBJECT

public:
	MMGActionStudioModePreview(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1002); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const override { return "StudioMode"; };
	const char *trActionName() const override { return "PreviewChange"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(obs_frontend_event event) const override;

private:
	MMGStringID scene;
};
MMG_DECLARE_ACTION(MMGActionStudioModePreview);

} // namespace MMGActions
