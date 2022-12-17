/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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
    json_str(json_obj, "json", 0),
    num(json_obj, "num", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Filters> action created.");
}

void MMGActionFilters::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Filters> Action -> " + message);
}

void MMGActionFilters::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  source.json(json_obj, "source", false);
  filter.json(json_obj, "filter", false);
  num.json(json_obj, "num", true);
  json_str.json(json_obj, "json", false);
}

void MMGActionFilters::do_action(const MMGMessage *midi)
{
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  OBSSourceAutoRelease obs_filter = obs_source_get_filter_by_name(obs_source, filter.mmgtocs());
  if (!obs_filter) {
    blog(LOG_INFO, "FAILED: Filter in source does not exist.");
    return;
  }
  OBSDataAutoRelease filter_data = obs_source_get_settings(obs_filter);
  QJsonObject filter_json = json_from_str(obs_data_get_json(filter_data));
  QJsonObject action_json = json_from_str(json_str.mmgtocs());
  QJsonObject final_json;

  switch (get_sub()) {
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
      if (num.choose(midi, 0, 128, true) - 1 >= obs_source_filter_count(obs_source)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeds filter count in source.");
	return;
      }
      obs_source_filter_set_order(obs_source, obs_filter, OBS_ORDER_MOVE_TOP);
      for (int i = 0; i < num.choose(midi, 0, 128, true) - 1; ++i) {
	obs_source_filter_set_order(obs_source, obs_filter, OBS_ORDER_MOVE_DOWN);
      }
      break;
    case MMGActionFilters::FILTER_CUSTOM:
      for (const QString &key : action_json.keys()) {
	if (key.endsWith("_state")) continue;
	QString _key = key.endsWith("_state_") ? key.chopped(1) : key;

	vec3 bounds = get_obs_filter_property_bounds(obs_filter, _key);
	QHash<QString, QString> options = get_obs_filter_property_options(obs_filter, _key);
	switch (action_json[key + "_state"].toInt()) {
	  case 1: // MIDI
	    switch (action_json[key].type()) {
	      case QJsonValue::Double:
		final_json[_key] = (midi->value() / 127) * (bounds.y - bounds.x) + bounds.x;
		break;
	      case QJsonValue::String:
		if (midi->value() >= options.keys().size()) break;
		final_json[_key] = options.keys()[(int)midi->value()];
		break;
	    }
	    break;
	  case 2: // INVERSE or TOGGLE
	    switch (action_json[key].type()) {
	      case QJsonValue::Double:
		final_json[_key] = (1 - (midi->value() / 127)) * (bounds.y - bounds.x) + bounds.x;
		break;
	      case QJsonValue::Bool:
		final_json[_key] = !filter_json[_key].toBool();
		break;
	    }
	    break;
	  case 3: // IGNORE
	    break;
	  default: // NORMAL
	    final_json[_key] = action_json[key];
	    break;
	}
      }
      obs_source_update(obs_filter,
			OBSDataAutoRelease(obs_data_create_from_json(json_to_str(final_json))));
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionFilters::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  source.copy(&dest->str1());
  filter.copy(&dest->str2());
  json_str.copy(&dest->str3());
  num.copy(&dest->num1());
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
      if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT) return true;

      _list->append(obs_source_get_name(source));
      return true;
    },
    &list);

  return list;
}

void MMGActionFilters::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Show Filter", "Hide Filter", "Toggle Filter Display", "Reorder Filter Appearance",
	      "Custom Filter Settings"};
}
void MMGActionFilters::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Source";
  val.list = enumerate_eligible();
}
void MMGActionFilters::change_options_str2(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR2;
  val.label_text = "Filter";
  val.list = enumerate(source);
}
void MMGActionFilters::change_options_str3(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case FILTER_SHOW:
    case FILTER_HIDE:
    case FILTER_TOGGLE_SHOWHIDE:
      break;
    case FILTER_REORDER:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Position";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI;
      val.lcds[0]->set_range(1.0, enumerate(source).size());
      val.lcds[0]->set_step(1.0, 5.0);
      val.lcds[0]->set_default_value(1.0);

      break;
    case FILTER_CUSTOM:
      val.display |= MMGActionDisplayParams::DISPLAY_SEC;
      break;
    default:
      break;
  }
}
void MMGActionFilters::change_options_final(MMGActionDisplayParams &val) {}
