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

struct ActionTransform {
	obs_transform_info ti = {0};
	obs_sceneitem_crop crop = {0};
	obs_blending_type blend_type;
	obs_scale_type scale_type;
};

class MMGActionVideoSources : public MMGAction {
	Q_OBJECT

public:
	MMGActionVideoSources(MMGActionManager *parent, const QJsonObject &json_obj);
	~MMGActionVideoSources() { delete at; };

	enum Actions {
		SOURCE_VIDEO_POSITION,
		SOURCE_VIDEO_DISPLAY,
		SOURCE_VIDEO_LOCKED,
		SOURCE_VIDEO_CROP,
		SOURCE_VIDEO_ALIGNMENT,
		SOURCE_VIDEO_SCALE,
		SOURCE_VIDEO_SCALEFILTER,
		SOURCE_VIDEO_ROTATION,
		SOURCE_VIDEO_BOUNDING_BOX_TYPE,
		SOURCE_VIDEO_BOUNDING_BOX_SIZE,
		SOURCE_VIDEO_BOUNDING_BOX_ALIGN,
		SOURCE_VIDEO_BLEND_MODE,
		SOURCE_VIDEO_SCREENSHOT,
		SOURCE_VIDEO_CUSTOM
	};
	enum Events {
		SOURCE_VIDEO_MOVED,
		SOURCE_VIDEO_DISPLAY_CHANGED,
		SOURCE_VIDEO_LOCK_CHANGED,
		SOURCE_VIDEO_CROPPED,
		SOURCE_VIDEO_ALIGNED,
		SOURCE_VIDEO_SCALED,
		SOURCE_VIDEO_SCALEFILTER_CHANGED,
		SOURCE_VIDEO_ROTATED,
		SOURCE_VIDEO_BOUNDING_BOX_TYPE_CHANGED,
		SOURCE_VIDEO_BOUNDING_BOX_RESIZED,
		SOURCE_VIDEO_BOUNDING_BOX_ALIGNED,
		SOURCE_VIDEO_BLEND_MODE_CHANGED,
		SOURCE_VIDEO_SCREENSHOT_TAKEN,
		SOURCE_VIDEO_CUSTOM_CHANGED
	};

	Category category() const override { return MMGACTION_SOURCE_VIDEO; };
	const QString trName() const override { return "VideoSources"; };
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
	static const QStringList alignmentOptions();
	static const QStringList boundingBoxOptions();
	static const QStringList scaleFilterOptions();
	static const QStringList blendModeOptions();
	static const vec2 obsResolution();

	const vec2 sourceResolution() const;
	void updateTransform() const;
	uint32_t convertAlignment(bool to_align, uint32_t value) const;

private:
	MMGUtils::MMGString parent_scene;
	MMGUtils::MMGString source;
	MMGUtils::MMGString action;
	MMGUtils::MMGNumber nums[4];
	MMGUtils::MMGJsonObject *_json;

	const MMGUtils::MMGNumber &num1() const { return nums[0]; };
	const MMGUtils::MMGNumber &num2() const { return nums[1]; };
	const MMGUtils::MMGNumber &num3() const { return nums[2]; };
	const MMGUtils::MMGNumber &num4() const { return nums[3]; };

	ActionTransform *at = nullptr;

	void connectSceneGroups(obs_scene_t *);

private slots:
	void onList1Change();
	void onList2Change();

	void frontendEventReceived(obs_frontend_event) override;
	void sourceEventReceived(MMGSourceSignal::Event, QVariant) override;
	void sourceTransformCallback(obs_sceneitem_t *sceneitem);
};
