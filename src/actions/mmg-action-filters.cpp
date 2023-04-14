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

#include "mmg-action-filters.h"
#include "mmg-action-scenes.h"

using namespace MMGUtils;

MMGActionFilters::MMGActionFilters(const QJsonObject &json_obj)
  : source(json_obj, "source", 1),
    filter(json_obj, "filter", 2),
    num(json_obj, "num", 1),
    json_str(json_obj, "json", 0)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionFilters::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Filters] " + message);
}

void MMGActionFilters::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  source.json(json_obj, "source", false);
  filter.json(json_obj, "filter", false);
  num.json(json_obj, "num");
  json_str.json(json_obj, "json", false);
}

void MMGActionFilters::execute(const MMGMessage *midi) const
{
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  OBSSourceAutoRelease obs_filter = obs_source_get_filter_by_name(obs_source, filter.mmgtocs());
  if (!obs_filter) {
    blog(LOG_INFO, "FAILED: Filter in source does not exist.");
    return;
  }

  switch (sub()) {
    case MMGActionFilters::FILTER_SHOW:
      obs_source_set_enabled(obs_filter, true);
      break;
    case MMGActionFilters::FILTER_HIDE:
      obs_source_set_enabled(obs_filter, false);
      break;
    case MMGActionFilters::FILTER_TOGGLE_SHOWHIDE:
      obs_source_set_enabled(obs_filter, !obs_source_enabled(obs_filter));
      break;
    case MMGActionFilters::FILTER_REORDER:
      if (num.choose(midi) - 1 >= obs_source_filter_count(obs_source)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeds filter count in source.");
	return;
      }
      obs_source_filter_set_order(obs_source, obs_filter, OBS_ORDER_MOVE_TOP);
      for (int i = 0; i < num.choose(midi) - 1; ++i) {
	obs_source_filter_set_order(obs_source, obs_filter, OBS_ORDER_MOVE_DOWN);
      }
      break;
    case MMGActionFilters::FILTER_CUSTOM:
      obs_source_custom_update(obs_filter, json_from_str(json_str.mmgtocs()), midi);
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionFilters::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionFilters *>(dest);
  if (!casted) return;

  casted->source = source.copy();
  casted->filter = filter.copy();
  casted->json_str = json_str.copy();
  casted->num = num.copy();
}

void MMGActionFilters::setEditable(bool edit)
{
  source.set_edit(edit);
  filter.set_edit(edit);
  json_str.set_edit(edit);
  num.set_edit(edit);
}

const QStringList MMGActionFilters::enumerate(const QString &source)
{
  QStringList list;
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.qtocs());
  obs_source_enum_filters(
    obs_source,
    [](obs_source_t *source, obs_source_t *filter, void *param) {
      Q_UNUSED(source);
      auto _list = reinterpret_cast<QStringList *>(param);
      _list->append(obs_source_get_name(filter));
    },
    &list);
  return list;
}

const QStringList MMGActionFilters::enumerate_eligible()
{
  QStringList list;
  list.append(MMGActionScenes::enumerate());

  obs_enum_all_sources(
    [](void *param, obs_source_t *source) {
      auto _list = reinterpret_cast<QStringList *>(param);

      if (obs_obj_is_private(source)) return true;
      if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT) {
	// For Group Sources
	if (obs_source_get_type(source) != OBS_SOURCE_TYPE_SCENE ||
	    _list->contains(obs_source_get_name(source))) {
	  return true;
	}
      }

      _list->append(obs_source_get_name(source));
      return true;
    },
    &list);

  return list;
}

void MMGActionFilters::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&source);
  _display->setStr2Storage(&filter);

  MMGNumberDisplay *num_display = new MMGNumberDisplay(_display->numberDisplays());
  num_display->setStorage(&num, true);
  _display->numberDisplays()->add(num_display);

  _display->connect(_display, &MMGActionDisplay::str1Changed, [&]() { setList1Config(); });
  _display->connect(_display, &MMGActionDisplay::str2Changed, [&]() { setList2Config(); });
}

void MMGActionFilters::setSubOptions(QComboBox *sub)
{
  sub->addItems(
    mmgtr_all("Actions.Filters.Sub", {"Show", "Hide", "ToggleDisplay", "Reorder", "Custom"}));
}

void MMGActionFilters::setSubConfig()
{
  _display->setStr1Visible(false);
  _display->setStr2Visible(false);
  _display->setStr3Visible(false);

  _display->setStr1Visible(true);
  _display->setStr1Description(obstr("Basic.Main.Source"));
  _display->setStr1Options(enumerate_eligible());
}

void MMGActionFilters::setList1Config()
{
  _display->setStr2Visible(true);
  _display->setStr2Description(mmgtr("Actions.Filters.Filter"));
  _display->setStr2Options(enumerate(source));
}

void MMGActionFilters::setList2Config()
{
  _display->resetScrollWidget();

  MMGNumberDisplay *num_display = _display->numberDisplays()->fieldAt(0);
  num_display->setVisible(false);

  OBSSourceAutoRelease obs_source;
  OBSSourceAutoRelease obs_filter;

  switch ((Actions)subcategory) {
    case FILTER_SHOW:
    case FILTER_HIDE:
    case FILTER_TOGGLE_SHOWHIDE:
      break;

    case FILTER_REORDER:
      num_display->setVisible(!filter.str().isEmpty());
      num_display->setDescription(obstr("Basic.TransformWindow.Position"));
      num_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_DEFAULT);
      num_display->setBounds(1.0, enumerate(source).size());
      num_display->setStep(1.0);
      num_display->setDefaultValue(1.0);
      num_display->reset();
      break;

    case FILTER_CUSTOM:
      obs_source = obs_get_source_by_name(source.mmgtocs());
      obs_filter = obs_source_get_filter_by_name(obs_source, filter.mmgtocs());
      emit _display->customFieldRequest(obs_filter, &json_str);
      break;

    default:
      return;
  }
}
