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

namespace MMGActions {

const MMGStringTranslationMap enumerateSourcesWithFilters();
const MMGStringTranslationMap enumerateFilters(const MMGString &source_uuid);

class MMGActionFilters : public MMGAction, public MMGSignal::MMGSourceReceiver {
	Q_OBJECT

public:
	MMGActionFilters(MMGActionManager *parent, const QJsonObject &json_obj);
	virtual ~MMGActionFilters() = default;

	const char *categoryName() const final override { return "Filters"; };
	virtual const char *trActionName() const override = 0;

	virtual MMGString sourceId() const override { return filter; };
	virtual const char *sourceSignalName() const override = 0;

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceChanged() const;
	void onFilterChanged() const;
	virtual void onFilterFixedChanged(const obs_source_t *obs_filter) const = 0;

private:
	void execute(const MMGMappingTest &test) const override;
	void connectSignal(bool connect) final override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(const calldata_t *cd) const override;

protected:
	virtual void execute(const MMGMappingTest &test, obs_source_t *obs_filter) const = 0;
	virtual void processEvent(const obs_source_t *obs_filter) const = 0;

protected:
	obs_source_t *sourceParent() const { return OBSSourceAutoRelease(obs_get_source_by_uuid((MMGString)source)); };

private:
	MMGStringID source;
	MMGStringID filter;

	mutable const char *current_uuid;

	static MMGParams<MMGString> source_params;
	static MMGParams<MMGString> filter_params;
};

class MMGActionFiltersVisible : public MMGActionFilters {
	Q_OBJECT

public:
	MMGActionFiltersVisible(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1501); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Visibility"; };

	const char *sourceSignalName() const override { return "enable"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onFilterFixedChanged(const obs_source_t *obs_filter) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_filter) const override;
	void processEvent(const obs_source_t *obs_filter) const override;

private:
	MMGBoolean visible;

	static MMGParams<bool> visible_params;
};
MMG_DECLARE_ACTION(MMGActionFiltersVisible);

class MMGActionFiltersReorder : public MMGActionFilters {
	Q_OBJECT

public:
	MMGActionFiltersReorder(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1581); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Reorder"; };

	MMGString sourceId() const override { return obs_source_get_uuid(sourceParent()); };
	const char *sourceSignalName() const override { return "reorder_filters"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onFilterFixedChanged(const obs_source_t *obs_filter) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(const obs_source_t *obs_source) const override;

private:
	MMGValue<uint64_t> index;

	mutable uint64_t current_index;

	static MMGParams<uint64_t> reorder_params;
};
MMG_DECLARE_ACTION(MMGActionFiltersReorder);

class MMGActionFiltersCustom : public MMGActionFilters {
public:
	MMGActionFiltersCustom(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x15ff); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Custom"; };

	const char *sourceSignalName() const override { return "update"; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onFilterFixedChanged(const obs_source_t *obs_filter) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *) const override;
	void processEvent(const obs_source_t *) const override;

private:
	MMGOBSFields::MMGOBSObject *custom_data;
};
MMG_DECLARE_ACTION(MMGActionFiltersCustom);

} // namespace MMGActions
