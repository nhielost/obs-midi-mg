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

#include "mmg-action-filters.h"
#include "mmg-action-scenes.h"
#include "../ui/mmg-fields.h"

using namespace MMGUtils;

MMGActionFilters::MMGActionFilters(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  source(json_obj, "source", 1),
	  filter(json_obj, "filter", 2),
	  num(json_obj, "num", 1),
	  _json(new MMGJsonObject(this, json_obj))
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionFilters::subNames() const
{
	return subModuleTextList({"Show", "Hide", "ToggleDisplay", "Reorder", "Custom"});
}

void MMGActionFilters::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	source.json(json_obj, "source", false);
	filter.json(json_obj, "filter", false);
	num.json(json_obj, "num");
	_json->json(json_obj, "json");
}

void MMGActionFilters::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionFilters *>(dest);
	if (!casted) return;

	casted->source = source.copy();
	casted->filter = filter.copy();
	casted->num = num.copy();
	_json->copy(casted->_json);
}

void MMGActionFilters::setEditable(bool edit)
{
	source.setEditable(edit);
	filter.setEditable(edit);
	num.setEditable(edit);
	_json->setEditable(edit);
}

void MMGActionFilters::toggle()
{
	num.toggle();
}

void MMGActionFilters::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	connect(display()->addNew(&source), &MMGStringDisplay::stringChanged, this, &MMGActionFilters::onList1Change);
	connect(display()->addNew(&filter), &MMGStringDisplay::stringChanged, this, &MMGActionFilters::onList2Change);

	display()->addNew(&num);
}

void MMGActionFilters::setActionParams()
{
	display()->hideAll();

	MMGStringDisplay *source_display = display()->stringDisplay(0);
	source_display->setVisible(true);
	source_display->setDescription(obstr("Basic.Main.Source"));
	source_display->setBounds(enumerateEligible());
}

void MMGActionFilters::onList1Change()
{
	connectSignals(true);

	MMGStringDisplay *filter_display = display()->stringDisplay(1);
	filter_display->setVisible(true);
	filter_display->setDescription(mmgtr("Actions.Filters.Filter"));
	filter_display->setBounds(enumerate(source));
}

void MMGActionFilters::onList2Change()
{
	connectSignals(true);

	display()->reset();
	display()->stringDisplay(1)->setVisible(true);

	MMGNumberDisplay *num_display = display()->numberDisplay(0);
	num_display->setVisible(false);

	OBSSourceAutoRelease obs_source;

	switch (sub()) {
		case FILTER_SHOW:
		case FILTER_HIDE:
		case FILTER_TOGGLE_SHOWHIDE:
			break;

		case FILTER_REORDER:
			if (type() == TYPE_OUTPUT) {
				display()->stringDisplay(1)->setVisible(false);
			} else {
				num_display->setVisible(!filter.value().isEmpty());
				num_display->setDescription(obstr("Basic.TransformWindow.Position"));
				num_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
				num_display->setBounds(1.0, enumerate(source).size());
				num_display->setStep(1.0);
				num_display->setDefaultValue(1.0);
				num_display->reset();
			}
			break;

		case FILTER_CUSTOM:
			obs_source = obs_get_source_by_name(source.mmgtocs());
			obs_source = obs_source_get_filter_by_name(obs_source, filter.mmgtocs());
			display()->setFields(MMGOBSFields::registerSource(obs_source, _json));
			break;

		default:
			return;
	}
}

const QStringList MMGActionFilters::enumerate(const QString &source)
{
	QStringList list;
	OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.qtocs());
	obs_source_enum_filters(
		obs_source,
		[](obs_source_t *, obs_source_t *filter, void *param) {
			auto _list = reinterpret_cast<QStringList *>(param);
			_list->append(obs_source_get_name(filter));
		},
		&list);
	return list;
}

const QStringList MMGActionFilters::enumerateEligible()
{
	QStringList list;

	obs_enum_all_sources(
		[](void *param, obs_source_t *source) {
			auto _list = reinterpret_cast<QStringList *>(param);

			if (obs_obj_is_private(source)) return true;
			if (obs_source_filter_count(source) < 1) return true;

			_list->append(obs_source_get_name(source));
			return true;
		},
		&list);

	return list;
}

void MMGActionFilters::execute(const MMGMessage *midi) const
{
	OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
	OBSSourceAutoRelease obs_filter = obs_source_get_filter_by_name(obs_source, filter.mmgtocs());
	ACTION_ASSERT(obs_filter, "Filter in source does not exist.");

	switch (sub()) {
		case FILTER_SHOW:
			obs_source_set_enabled(obs_filter, true);
			break;

		case FILTER_HIDE:
			obs_source_set_enabled(obs_filter, false);
			break;

		case FILTER_TOGGLE_SHOWHIDE:
			obs_source_set_enabled(obs_filter, !obs_source_enabled(obs_filter));
			break;

		case FILTER_REORDER:
			obs_source_filter_set_order(obs_source, obs_filter, OBS_ORDER_MOVE_TOP);
			for (int i = 0; i < num.chooseFrom(midi, true) - 1; ++i) {
				obs_source_filter_set_order(obs_source, obs_filter, OBS_ORDER_MOVE_DOWN);
			}
			break;

		case FILTER_CUSTOM:
			MMGOBSFields::execute(obs_filter, _json, midi);
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionFilters::connectSignals(bool _connect)
{
	MMGAction::connectSignals(_connect);
	if (!_connected && type() == TYPE_OUTPUT) return;

	OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
	OBSSourceAutoRelease obs_filter = obs_source_get_filter_by_name(obs_source, filter.mmgtocs());

	if (type() == TYPE_OUTPUT) {
		connectSourceSignal(mmgsignals()->requestSourceSignal(obs_source));
		connectSourceSignal(mmgsignals()->requestSourceSignal(obs_filter));
	}

	MMGOBSFields::registerSource(obs_filter, _json);
}

void MMGActionFilters::sourceEventReceived(MMGSourceSignal::Event event, QVariant data)
{
	OBSSourceAutoRelease obs_source;

	switch (sub()) {
		case FILTER_SHOWN:
			if (event != MMGSourceSignal::SIGNAL_FILTER_ENABLE) return;
			if (!data.toBool()) return;
			break;

		case FILTER_HIDDEN:
			if (event != MMGSourceSignal::SIGNAL_FILTER_ENABLE) return;
			if (data.toBool()) return;
			break;

		case FILTER_TOGGLE_SHOWN:
			if (event != MMGSourceSignal::SIGNAL_FILTER_ENABLE) return;
			break;

		case FILTER_REORDERED:
			if (event != MMGSourceSignal::SIGNAL_FILTER_REORDER) return;
			break;

		case FILTER_CUSTOM_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_UPDATE) return;

			obs_source = obs_get_source_by_name(source.mmgtocs());
			obs_source = obs_source_get_filter_by_name(obs_source, filter.mmgtocs());
			if (data.value<void *>() != obs_source) return;

			triggerEvent(MMGOBSFields::customEventReceived(obs_source, _json));
			return;

		default:
			return;
	}

	triggerEvent();
}