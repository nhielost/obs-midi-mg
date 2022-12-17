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

#include "mmg-utils.h"
#include "mmg-config.h"
#include "actions/mmg-action.h"

#include <obs-frontend-api.h>

#include <QMessageBox>

void global_blog(int log_status, const QString &message)
{
  blog(log_status, "[obs-midi-mg] %s", qPrintable(message));
}

namespace MMGUtils {

// MMGNumber
MMGNumber::MMGNumber(const QJsonObject &json_obj, const QString &preferred, int fallback_num)
{
  double fallback = json_obj[num_to_str(fallback_num, "num")].toDouble();

  if (json_obj[preferred].isDouble()) {
    // v2.2.0+
    set_num(json_obj[preferred].toDouble());
    num_state = (State)json_obj[preferred + "_state"].toInt();
  } else if (json_obj["nums_state"].isDouble()) {
    // v2.1.0 - v2.1.1
    set_num(fallback);
    int nums_state = (json_obj["nums_state"].toInt() & (3 << (fallback_num * 2))) >>
		     (fallback_num * 2);
    num_state = (State)(nums_state == 2 ? 3 : nums_state);
  } else {
    // pre v2.1.0
    set_num(fallback == -1 ? 0 : fallback);
    num_state = (State)(fallback == -1);
  }
  if ((uint)num_state > 3) num_state = NUMBERSTATE_FIXED;
}

void MMGNumber::json(QJsonObject &json_obj, const QString &prefix, bool use_state) const
{
  json_obj[prefix] = number;
  if (use_state) json_obj[prefix + "_state"] = (int)num_state;
}

double MMGNumber::choose(const MMGMessage *midi, double default_value, double mult, bool off_by_one)
{
  switch (num_state) {
    case MMGNumber::NUMBERSTATE_MIDI:
      return qRound(((midi->value() + off_by_one) / 128.0) * mult);
    case MMGNumber::NUMBERSTATE_MIDI_INVERT:
      return qRound(((128 - midi->value() - off_by_one) / 128.0) * mult);
    case MMGNumber::NUMBERSTATE_IGNORE:
      return default_value;
    default:
      return number;
  }
}

void MMGNumber::copy(MMGNumber *dest) const
{
  dest->set_num(number);
  dest->set_state(num_state);
}
// End MMGNumber

// MMGString
MMGString::MMGString(const QJsonObject &json_obj, const QString &preferred, int fallback_num)
{
  if (json_obj[preferred].isString()) {
    set_str(json_obj[preferred].toString());
    str_state = (State)json_obj[preferred + "_state"].toInt();
    if ((uint)str_state > 2) str_state = STRINGSTATE_FIXED;
  } else {
    set_str(json_obj[num_to_str(fallback_num, "str")].toString());
    if (string == "Use Message Value") {
      str_state = STRINGSTATE_MIDI;
    } else if (string == "Toggle") {
      str_state = STRINGSTATE_TOGGLE;
    }
  }
}

void MMGString::json(QJsonObject &json_obj, const QString &prefix, bool use_state) const
{
  json_obj[prefix] = string;
  if (use_state) json_obj[prefix + "_state"] = (int)str_state;
}

void MMGString::copy(MMGString *dest) const
{
  dest->set_str(string);
  dest->set_state(str_state);
}
// End MMGString

// LCDData
LCDData::LCDData(QLCDNumber *lcd_ptr) : lcd(lcd_ptr) {}

void LCDData::set_storage(MMGNumber *number)
{
  storage = number;
  set_value(*number);
  set_state(number->state());
}

void LCDData::set_range(double min, double max)
{
  minimum = min;
  maximum = max;
};
void LCDData::set_step(double minor, double major)
{
  minor_step = minor;
  major_step = major;
};

void LCDData::down_major()
{
  set_value(*storage - major_step);
}
void LCDData::down_minor()
{
  set_value(*storage - minor_step);
}
void LCDData::up_minor()
{
  set_value(*storage + minor_step);
}
void LCDData::up_major()
{
  set_value(*storage + major_step);
}

void LCDData::reset()
{
  if (editable) storage->set_num(default_val);
  display();
}
void LCDData::set_value(double val)
{
  if (!editable) return;
  if (val > maximum) {
    storage->set_num(maximum);
  } else if (val < minimum) {
    storage->set_num(minimum);
  } else if (minor_step < 1 && qAbs(val - qFloor(val)) > 0 &&
	     qAbs(val - qFloor(val)) < major_step) {
    storage->set_num(qRound(val / minor_step) * minor_step);
  } else {
    storage->set_num(val);
  }
  display();
}
void LCDData::set_state(int index)
{
  if (editable) storage->set_state((MMGNumber::State)index);
  lcd->setEnabled(index == 0);
  display();
}

void LCDData::display()
{
  if (lcd->isEnabled()) {
    if (use_time) {
      lcd->display(QTime(*storage / 3600.0, fmod(*storage / 60.0, 60.0), fmod(*storage, 60.0))
		     .toString("hh:mm:ss"));
    } else {
      lcd->display(*storage);
    }
  } else {
    lcd->display(lcd->digitCount() == 8 ? "   ---  " : "---");
  }
}
// End LCDData

bool json_key_exists(const QJsonObject &obj, const QString &key, QJsonValue::Type value_type)
{
  return obj.contains(key) && json_is_valid(obj[key], value_type);
}

bool json_is_valid(const QJsonValue &value, QJsonValue::Type value_type)
{
  switch (value_type) {
    case QJsonValue::Type::Double:
      return value.isDouble();
    case QJsonValue::Type::String:
      return value.isString();
    case QJsonValue::Type::Bool:
      return value.isBool();
    case QJsonValue::Type::Object:
      return value.isObject();
    case QJsonValue::Type::Array:
      return value.isArray();
    case QJsonValue::Type::Null:
      return value.isNull();
    case QJsonValue::Type::Undefined:
      return value.isUndefined();
    default:
      return false;
  }
}

const QByteArray json_to_str(const QJsonObject &json_obj)
{
  return QJsonDocument(json_obj).toJson(QJsonDocument::Compact);
}

const QJsonObject json_from_str(const QByteArray &str)
{
  return QJsonDocument::fromJson(str).object();
}

void call_midi_callback(const libremidi::message &message)
{
  MMGSharedMessage incoming(new MMGMessage(message));
  if (global()->is_listening(incoming.get())) return;
  if (global()->find_current_device()->input_port_open())
    global()->find_current_device()->do_all_actions(incoming);
}

bool bool_from_str(const QString &str)
{
  return str == "True" || str == "Yes" || str == "Show" || str == "Locked";
}

QString num_to_str(int num, const QString &prefix)
{
  return QString::number(num).prepend(prefix);
}

void open_message_box(const QString &title, const QString &text)
{
  struct C {
    QString title;
    QString text;
  } c{title, text};
  obs_queue_task(
    OBS_TASK_UI,
    [](void *param) {
      auto content = reinterpret_cast<C *>(param);
      QMessageBox::information(nullptr, content->title, content->text);
    },
    &c, true);
}

void transfer_bindings(short mode, const QString &source, const QString &dest)
{
  if (mode < 0 || mode > 2) return;

  if (source == dest) {
    open_message_box("Transfer Error",
		     "Failed to transfer bindings: Cannot transfer bindings to the same device.");
    return;
  }

  MMGDevice *const source_device = global()->find_device(source);
  if (!source_device) {
    open_message_box("Transfer Error", "Failed to transfer bindings: Source device is invalid.");
    return;
  }

  MMGDevice *const dest_device = global()->find_device(dest);
  if (!dest_device) {
    open_message_box("Transfer Error",
		     "Failed to transfer bindings: Destination device is invalid.");
    return;
  }

  if (source_device == dest_device) {
    open_message_box("Transfer Error",
		     "Failed to transfer bindings: Cannot transfer bindings to the same device.");
    return;
  }

  // Deep copy of bindings
  MMGDevice *source_copy = new MMGDevice;
  source_device->deep_copy(source_copy);

  // REMOVE (occurs for both move and replace)
  if (mode > 0) source_device->clear();

  // REPLACE
  if (mode == 2) dest_device->clear();

  // COPY (occurs for all three modes)
  for (MMGBinding *const binding : source_copy->get_bindings()) {
    dest_device->add(binding);
  }

  // Clearing before deleting is important because it allows the
  // destination device to take control of the pointers. If this
  // were not the case, the copied pointers would be deleted by
  // the this MMGDevice's destructor.
  source_copy->clear();
  delete source_copy;

  open_message_box("Transfer Success", "Bindings successfully transferred.");
}

vec2 get_obs_dimensions()
{
  obs_video_info video_info;
  obs_get_video_info(&video_info);
  vec2 xy;
  vec2_set(&xy, video_info.base_width, video_info.base_height);
  return xy;
}

vec2 get_obs_source_dimensions(const QString &name)
{
  OBSSourceAutoRelease source = obs_get_source_by_name(name.qtocs());
  vec2 xy;
  vec2_set(&xy, obs_source_get_width(source), obs_source_get_height(source));
  return xy;
}

double get_obs_media_length(const QString &name)
{
  OBSSourceAutoRelease source = obs_get_source_by_name(name.qtocs());
  return obs_source_media_get_duration(source) / 1000.0;
}

obs_source_t *get_obs_transition_by_name(const QString &name)
{
  obs_frontend_source_list transition_list = {0};
  obs_frontend_get_transitions(&transition_list);
  for (size_t i = 0; i < transition_list.sources.num; ++i) {
    obs_source_t *ptr = transition_list.sources.array[i];
    if (obs_source_get_name(ptr) == name) {
      obs_frontend_source_list_free(&transition_list);
      return obs_source_get_ref(ptr);
    }
  }
  obs_frontend_source_list_free(&transition_list);
  return nullptr;
}

bool get_obs_transition_fixed_length(const QString &name)
{
  OBSSourceAutoRelease transition = obs_get_transition_by_name(name.qtocs());
  return obs_transition_fixed(transition);
}

const vec3 get_obs_filter_property_bounds(obs_property_t *prop)
{
  vec3 bounds = {0};
  if (obs_property_get_type(prop) != OBS_PROPERTY_INT &&
      obs_property_get_type(prop) != OBS_PROPERTY_FLOAT)
    return bounds;
  if (obs_property_get_type(prop) == OBS_PROPERTY_INT) {
    vec3_set(&bounds, obs_property_int_min(prop), obs_property_int_max(prop),
	     obs_property_int_step(prop));
  } else {
    vec3_set(&bounds, obs_property_float_min(prop), obs_property_float_max(prop),
	     obs_property_float_step(prop));
  }
  return bounds;
}

const vec3 get_obs_filter_property_bounds(obs_source_t *filter, const QString &field)
{
  if (obs_source_get_type(filter) != OBS_SOURCE_TYPE_FILTER) return vec3();

  obs_properties_t *props = obs_source_properties(filter);
  obs_property_t *prop = obs_properties_get(props, field.qtocs());
  const vec3 bounds = get_obs_filter_property_bounds(prop);
  obs_properties_destroy(props);
  return bounds;
}

const QHash<QString, QString> get_obs_filter_property_options(obs_property_t *prop)
{
  QHash<QString, QString> options;
  if (obs_property_get_type(prop) != OBS_PROPERTY_LIST)
    for (size_t i = 0; i < obs_property_list_item_count(prop); ++i) {
      switch (obs_property_list_format(prop)) {
	case OBS_COMBO_FORMAT_FLOAT:
	  options[obs_property_list_item_name(prop, i)] =
	    QString::number(obs_property_list_item_float(prop, i));
	  break;
	case OBS_COMBO_FORMAT_INT:
	  options[obs_property_list_item_name(prop, i)] =
	    QString::number(obs_property_list_item_int(prop, i));
	  break;
	case OBS_COMBO_FORMAT_STRING:
	  options[obs_property_list_item_name(prop, i)] = obs_property_list_item_string(prop, i);
	  break;
	default:
	  break;
      }
    }
  return options;
}

const QHash<QString, QString> get_obs_filter_property_options(obs_source_t *filter,
							      const QString &field)
{
  if (obs_source_get_type(filter) != OBS_SOURCE_TYPE_FILTER) return QHash<QString, QString>();

  obs_properties_t *props = obs_source_properties(filter);
  obs_property_t *prop = obs_properties_get(props, field.qtocs());
  const QHash<QString, QString> options = get_obs_filter_property_options(prop);
  obs_properties_destroy(props);
  return options;
}

}
