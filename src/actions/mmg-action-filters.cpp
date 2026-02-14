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

#include "mmg-action-filters.h"
#include "mmg-action-scenes.h"

namespace MMGActions {

const MMGStringTranslationMap enumerateFilters(const MMGString &source_uuid)
{
	MMGStringTranslationMap map;
	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(source_uuid);

	obs_source_enum_filters(
		obs_source,
		[](obs_source_t *, obs_source_t *obs_filter, void *param) {
			auto _map = reinterpret_cast<MMGStringTranslationMap *>(param);
			_map->insert(obs_source_get_uuid(obs_filter), nontr(obs_source_get_name(obs_filter)));
		},
		&map);

	return map;
}

const MMGStringTranslationMap enumerateSourcesWithFilters()
{
	MMGStringTranslationMap map;

	obs_enum_all_sources(
		[](void *param, obs_source_t *obs_source) {
			auto _map = reinterpret_cast<MMGStringTranslationMap *>(param);

			if (obs_obj_is_private(obs_source)) return true;
			if (obs_source_filter_count(obs_source) < 1) return true;

			_map->insert(obs_source_get_uuid(obs_source), nontr(obs_source_get_name(obs_source)));
			return true;
		},
		&map);

	return map;
}

// MMGActionFilters
MMGParams<MMGString> MMGActionFilters::source_params {
	.desc = obstr("Basic.Main.Source"),
	.options = OPTION_NONE,
	.default_value = "",
	.bounds = {},
	.placeholder = obstr("NoSources.Title"),
};

MMGParams<MMGString> MMGActionFilters::filter_params {
	.desc = mmgtr("Actions.Filters.Filter"),
	.options = OPTION_NONE,
	.default_value = "",
	.bounds = {},
};

MMGActionFilters::MMGActionFilters(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  source(json_obj, "source"),
	  filter(json_obj, "filter")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionFilters::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(source, json_obj, "source", 1, enumerateSourcesWithFilters());
	MMGCompatibility::initOldStringData(filter, json_obj, "filter", 2, enumerateFilters(source));
}

void MMGActionFilters::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	source->json(json_obj, "source");
	filter->json(json_obj, "filter");
}

void MMGActionFilters::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionFilters *>(dest);
	if (!casted) return;

	source.copy(casted->source);
	filter.copy(casted->filter);
}

void MMGActionFilters::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	source_params.bounds = enumerateSourcesWithFilters();

	MMGActions::createActionField(display, &source, &source_params,
				      std::bind(&MMGActionFilters::onSourceChanged, this));
	MMGActions::createActionField(display, &filter, &filter_params,
				      std::bind(&MMGActionFilters::onFilterChanged, this));
}

void MMGActionFilters::onSourceChanged() const
{
	emit refreshRequested();
	filter_params.bounds = enumerateFilters(source);
}

void MMGActionFilters::onFilterChanged() const
{
	emit refreshRequested();
	OBSSourceAutoRelease obs_filter = obs_get_source_by_uuid(MMGString(filter));
	onFilterFixedChanged(obs_filter);
}

void MMGActionFilters::execute(const MMGMappingTest &test) const
{
	OBSSourceAutoRelease obs_filter = obs_get_source_by_uuid(MMGString(filter));
	ACTION_ASSERT(obs_filter, "The specified filter does not exist. Check the "
				  "Filter field and try again.");

	execute(test, obs_filter);
}

void MMGActionFilters::processEvent(const calldata_t *cd) const
{
	obs_source_t *signal_source = (obs_source_t *)(calldata_ptr(cd, "source"));
	current_uuid = obs_source_get_uuid(signal_source);

	if (current_uuid == source) {
		OBSSourceAutoRelease obs_filter = obs_get_source_by_uuid(MMGString(filter));
		processEvent(obs_filter);
	} else {
		processEvent(signal_source);
	}
}
// End MMGActionFilters

// MMGActionFiltersVisible
MMGParams<bool> MMGActionFiltersVisible::visible_params {
	.desc = obstr("Basic.Main.Sources.Visibility"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionFiltersVisible::MMGActionFiltersVisible(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionFilters(parent, json_obj),
	  visible(json_obj, "visible")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionFiltersVisible::initOldData(const QJsonObject &json_obj)
{
	MMGActionFilters::initOldData(json_obj);

	MMGCompatibility::initOldBooleanData(visible, json_obj["sub"].toInt());
}

void MMGActionFiltersVisible::json(QJsonObject &json_obj) const
{
	MMGActionFilters::json(json_obj);

	visible->json(json_obj, "visible");
}

void MMGActionFiltersVisible::copy(MMGAction *dest) const
{
	MMGActionFilters::copy(dest);

	auto casted = dynamic_cast<MMGActionFiltersVisible *>(dest);
	if (!casted) return;

	visible.copy(casted->visible);
}

void MMGActionFiltersVisible::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionFilters::createDisplay(display);

	MMGActions::createActionField(display, &visible, &visible_params);
}

void MMGActionFiltersVisible::onFilterFixedChanged(const obs_source_t *obs_filter) const
{
	visible_params.default_value = !obs_source_enabled(obs_filter);
}

void MMGActionFiltersVisible::execute(const MMGMappingTest &test, obs_source_t *obs_filter) const
{
	bool is_visible = obs_source_enabled(obs_filter);
	ACTION_ASSERT(test.applicable(visible, is_visible), "A visibility state could not be selected. Check the "
							    "Visibility field and try again.");
	obs_source_set_enabled(obs_filter, is_visible);
}

void MMGActionFiltersVisible::processEvent(const obs_source_t *obs_filter) const
{
	EventFulfillment fulfillment(this);
	fulfillment->addAcceptable(visible, obs_source_enabled(obs_filter));
}
// End MMGActionFiltersVisible

// MMGActionFiltersReorder
MMGParams<uint64_t> MMGActionFiltersReorder::reorder_params {
	.desc = obstr("Basic.TransformWindow.Position"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_INCREMENT,
	.default_value = 1,
	.lower_bound = 1.0,
	.upper_bound = 1.0,
	.step = 1.0,
	.incremental_bound = 1.0,
};

MMGActionFiltersReorder::MMGActionFiltersReorder(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionFilters(parent, json_obj),
	  index(json_obj, "index")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionFiltersReorder::initOldData(const QJsonObject &json_obj)
{
	MMGActionFilters::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(index, json_obj, "num", 1);
}

void MMGActionFiltersReorder::json(QJsonObject &json_obj) const
{
	MMGActionFilters::json(json_obj);

	index->json(json_obj, "index");
}

void MMGActionFiltersReorder::copy(MMGAction *dest) const
{
	MMGActionFilters::copy(dest);

	auto casted = dynamic_cast<MMGActionFiltersReorder *>(dest);
	if (!casted) return;

	index.copy(casted->index);
}

void MMGActionFiltersReorder::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionFilters::createDisplay(display);

	MMGActions::createActionField(display, &index, &reorder_params);
}

void MMGActionFiltersReorder::onFilterFixedChanged(const obs_source_t *obs_filter) const
{
	reorder_params.upper_bound = obs_source_filter_count(sourceParent());
	reorder_params.default_value =
		obs_source_filter_get_index(sourceParent(), const_cast<obs_source_t *>(obs_filter));
}

void MMGActionFiltersReorder::execute(const MMGMappingTest &test, obs_source_t *obs_filter) const
{
	uint64_t new_index;
	ACTION_ASSERT(test.applicable(index, new_index), "A position could not be selected. Check the Position field "
							 "and try again.");
	obs_source_filter_set_index(sourceParent(), obs_filter, new_index);
}

void MMGActionFiltersReorder::processEvent(const obs_source_t *obs_filter) const
{
	uint64_t prev_index = current_index;
	current_index = obs_source_filter_get_index(sourceParent(), const_cast<obs_source_t *>(obs_filter));

	EventFulfillment fulfillment(this);
	fulfillment->addCondition(current_index != prev_index);

	// In OBS, indices are 0-based ordered from bottom to top
	// In obs-midi-mg, indices are 1-based from top to bottom
	// This accounts for the difference between the two
	fulfillment->addAcceptable(index, obs_source_filter_count(sourceParent()) - current_index);
}
// End MMGActionFiltersReorder

// MMGActionFiltersCustom
MMGActionFiltersCustom::MMGActionFiltersCustom(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionFilters(parent, json_obj),
	  custom_data(new MMGOBSFields::MMGOBSObject(this, sourceId(), json_obj))
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionFiltersCustom::initOldData(const QJsonObject &json_obj)
{
	MMGActionFilters::initOldData(json_obj);

	custom_data->changeSource(sourceId(), json_obj);
}

void MMGActionFiltersCustom::json(QJsonObject &json_obj) const
{
	MMGActionFilters::json(json_obj);

	custom_data->json(json_obj);
}

void MMGActionFiltersCustom::copy(MMGAction *dest) const
{
	MMGActionFilters::copy(dest);

	auto casted = dynamic_cast<MMGActionFiltersCustom *>(dest);
	if (!casted) return;

	custom_data->copy(casted->custom_data);
}

void MMGActionFiltersCustom::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionFilters::createDisplay(display);

	custom_data->createDisplay(display);
}

void MMGActionFiltersCustom::onFilterFixedChanged(const obs_source_t *obs_filter) const
{
	custom_data->changeSource(obs_source_get_uuid(obs_filter));
}

void MMGActionFiltersCustom::execute(const MMGMappingTest &test, obs_source_t *) const
{
	custom_data->execute(test);
}

void MMGActionFiltersCustom::processEvent(const obs_source_t *) const
{
	EventFulfillment fulfiller(this);
	custom_data->processEvent(*fulfiller);
}
// End MMGActionFiltersCustom

} // namespace MMGActions
