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

enum Alignment {
	OBS_ALIGN_TOP_LEFT = OBS_ALIGN_TOP | OBS_ALIGN_LEFT,
	OBS_ALIGN_TOP_CENTER = OBS_ALIGN_TOP,
	OBS_ALIGN_TOP_RIGHT = OBS_ALIGN_TOP | OBS_ALIGN_RIGHT,
	OBS_ALIGN_CENTER_LEFT = OBS_ALIGN_LEFT,
	OBS_ALIGN_CENTER_CENTER = OBS_ALIGN_CENTER,
	OBS_ALIGN_CENTER_RIGHT = OBS_ALIGN_RIGHT,
	OBS_ALIGN_BOTTOM_LEFT = OBS_ALIGN_BOTTOM | OBS_ALIGN_LEFT,
	OBS_ALIGN_BOTTOM_CENTER = OBS_ALIGN_BOTTOM,
	OBS_ALIGN_BOTTOM_RIGHT = OBS_ALIGN_BOTTOM | OBS_ALIGN_RIGHT,
};

class MMGActionSceneItems : public MMGAction, public MMGSignal::MMGSourceReceiver {
	Q_OBJECT

public:
	MMGActionSceneItems(MMGActionManager *parent, const QJsonObject &json_obj);
	virtual ~MMGActionSceneItems() = default;

	const char *categoryName() const final override { return "SceneItems"; };
	virtual const char *trActionName() const override = 0;

	MMGString sourceId() const final override { return scene; };
	virtual const char *sourceSignalName() const override = 0;
	virtual uint64_t sourceBounds() const = 0;

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;

private:
	void onSceneChanged();
	void onSourceChanged();

	void execute(const MMGMappingTest &test) const final override;
	void connectSignal(bool connect) final override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(const calldata_t *cd) const final override;

protected:
	virtual void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const = 0;

	virtual void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const = 0;
	virtual void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const = 0;

private:
	obs_sceneitem_t *getSceneItem(const MMGString &source) const;

private:
	MMGStringID scene;
	MMGStringID source;

	mutable const char *applied_source_id;

	static MMGParams<MMGString> source_params;
};

class MMGActionSceneItemsVisible : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsVisible(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1201); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Display"; };

	const char *sourceSignalName() const override { return "item_visible"; };
	uint64_t sourceBounds() const override { return 0; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGBoolean disp;

	static MMGParams<bool> display_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsVisible);

class MMGActionSceneItemsLocked : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsLocked(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1202); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Locking"; };

	const char *sourceSignalName() const override { return "item_locked"; };
	uint64_t sourceBounds() const override { return 0; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGBoolean lock;

	static MMGParams<bool> lock_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsLocked);

class MMGActionSceneItemsPosition : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsPosition(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1211); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Move"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGFloat pos_x;
	MMGFloat pos_y;

	mutable vec2 current_pos;

	static MMGParams<float> pos_x_params;
	static MMGParams<float> pos_y_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsPosition);

class MMGActionSceneItemsScale : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsScale(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1212); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Scale"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGFloat scale_x;
	MMGFloat scale_y;

	mutable vec2 current_scale;

	static MMGParams<float> scale_x_params;
	static MMGParams<float> scale_y_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsScale);

class MMGActionSceneItemsRotation : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsRotation(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1213); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Rotate"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGFloat rot;

	mutable float current_rot;

	static MMGParams<float> rotation_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsRotation);

class MMGActionSceneItemsCrop : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsCrop(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1214); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Crop"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGInteger crop_l;
	MMGInteger crop_t;
	MMGInteger crop_r;
	MMGInteger crop_b;

	mutable obs_sceneitem_crop current_crop;

	static MMGParams<int32_t> crop_params[4];
};
MMG_DECLARE_ACTION(MMGActionSceneItemsCrop);

class MMGActionSceneItemsAlignment : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsAlignment(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1281); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Align"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGValue<Alignment> alignment;

	mutable Alignment current_alignment;

	static MMGParams<Alignment> alignment_params;

	friend class MMGActionSceneItemsBoundingBoxAlignment;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsAlignment);

class MMGActionSceneItemsScaleFilter : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsScaleFilter(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1282); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "ScaleFiltering"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGValue<obs_scale_type> scale_filter;

	mutable obs_scale_type current_scale_filter;

	static MMGParams<obs_scale_type> scale_filter_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsScaleFilter);

class MMGActionSceneItemsBlendingMode : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsBlendingMode(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1283); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "BlendingMode"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGValue<obs_blending_type> blending_mode;

	mutable obs_blending_type current_blending_mode;

	static MMGParams<obs_blending_type> blending_mode_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsBlendingMode);

class MMGActionSceneItemsBoundingBoxType : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsBoundingBoxType(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x12c1); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "BoundingBoxType"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGValue<obs_bounds_type> bounds_type;

	mutable obs_bounds_type current_bounds;

	static MMGParams<obs_bounds_type> bounds_type_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsBoundingBoxType);

class MMGActionSceneItemsBoundingBoxSize : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsBoundingBoxSize(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x12c2); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "BoundingBoxSize"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGFloat bounds_x;
	MMGFloat bounds_y;

	mutable vec2 current_bounds_size;

	static MMGParams<float> bounds_x_params;
	static MMGParams<float> bounds_y_params;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsBoundingBoxSize);

class MMGActionSceneItemsBoundingBoxAlignment : public MMGActionSceneItems {
	Q_OBJECT

public:
	MMGActionSceneItemsBoundingBoxAlignment(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x12c3); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "BoundingBoxAlign"; };

	const char *sourceSignalName() const override { return "item_transform"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_VIDEO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_sceneitem_t *obs_sceneitem) const override;

private:
	void execute(const MMGMappingTest &test, obs_sceneitem_t *obs_sceneitem) const override;
	void processEvent(MMGMappingTest &test, const obs_sceneitem_t *obs_sceneitem) const override;

private:
	MMGValue<Alignment> alignment;

	mutable Alignment current_alignment;
};
MMG_DECLARE_ACTION(MMGActionSceneItemsBoundingBoxAlignment);

} // namespace MMGActions
