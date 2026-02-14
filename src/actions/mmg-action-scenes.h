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
#include "mmg-action.h"

namespace MMGActions {

const MMGParams<MMGString> &sceneParams(char type = 0);

const MMGStringTranslationMap enumerateScenes();
const MMGStringTranslationMap enumerateSceneItems(const MMGString &scene_uuid);
MMGString currentScene(bool preview = false);

class MMGActionScenesSwitch : public MMGAction, public MMGSignal::MMGFrontendReceiver {
	Q_OBJECT

public:
	MMGActionScenesSwitch(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1101); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const final override { return "Scenes"; };
	const char *trActionName() const override { return "Switch"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(obs_frontend_event event) const override;

private:
	MMGBoolean use_preview;
	MMGStringID scene;

	static const MMGParams<bool> preview_params;
};
MMG_DECLARE_ACTION(MMGActionScenesSwitch);

class MMGActionScenesScreenshot : public MMGAction, public MMGSignal::MMGFrontendReceiver {
	Q_OBJECT

public:
	MMGActionScenesScreenshot(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1111); };
	constexpr Id id() const final override { return actionId(); };
	const char *categoryName() const final override { return "Scenes"; };
	const char *trActionName() const override { return "Screenshot"; };

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(obs_frontend_event event) const override
	{
		if (event == OBS_FRONTEND_EVENT_SCREENSHOT_TAKEN) EventFulfillment fulfiller(this);
	};
};
MMG_DECLARE_ACTION(MMGActionScenesScreenshot);

} // namespace MMGActions
