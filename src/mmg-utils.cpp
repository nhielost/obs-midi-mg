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

#include "mmg-utils.h"
#include "mmg-config.h"
#include "ui/mmg-number-display.h"
#include "actions/mmg-action.h"
#include "ui/mmg-fields.h"

#include <obs-frontend-api.h>

#include <QMessageBox>
#include <QStandardItemModel>

#include <mutex>

static std::mutex custom_update;

void global_blog(int log_status, const QString &message)
{
  blog(log_status, "[obs-midi-mg] %s", qPrintable(message));
}

namespace MMGUtils {

// MMGNumber
MMGNumber::MMGNumber(const QJsonObject &json_obj, const QString &preferred, int fallback_num)
{
  double fallback = json_obj[num_to_str(fallback_num, "num")].toDouble();

  if (json_obj[preferred].isObject()) {
    // v2.3.0+ (with bounds)
    QJsonObject number_obj = json_obj[preferred].toObject();
    set_num(number_obj["number"].toDouble());
    set_min(number_obj["lower"].toDouble());
    set_max(number_obj["higher"].toDouble());
    set_state(number_obj["state"].toInt());
  } else if (json_obj[preferred].isDouble()) {
    // v2.2.0+ (no bounds)
    set_num(json_obj[preferred].toDouble());
    short nums_state = json_obj[preferred + "_state"].toInt();
    set_state(nums_state == 2 ? 1 : nums_state);
    set_min(number < lower ? number : lower);
    set_max(number > higher ? number : higher);
  } else if (json_obj["nums_state"].isDouble()) {
    // v2.1.0 - v2.1.1
    set_num(fallback);
    int nums_state = (json_obj["nums_state"].toInt() & (3 << (fallback_num * 2))) >>
		     (fallback_num * 2);
    set_state(nums_state == 2 ? 3 : nums_state);
    set_min(number < lower ? number : lower);
    set_max(number > higher ? number : higher);
  } else {
    // pre v2.1.0
    set_num(fallback == -1 ? 0 : fallback);
    set_state(fallback == -1);
    set_min(number < lower ? number : lower);
    set_max(number > higher ? number : higher);
  }
  if ((uint)num_state > 3) num_state = NUMBERSTATE_FIXED;
}

void MMGNumber::json(QJsonObject &json_obj, const QString &prefix, bool use_bounds) const
{
  if (!use_bounds) {
    json_obj[prefix] = number;
    return;
  }
  QJsonObject number_json_obj;
  number_json_obj["number"] = number;
  number_json_obj["lower"] = lower;
  number_json_obj["higher"] = higher;
  number_json_obj["state"] = num_state;
  json_obj[prefix] = number_json_obj;
}

double MMGNumber::choose(const MMGMessage *midi, double default_value) const
{
  switch (num_state) {
    case MMGNumber::NUMBERSTATE_MIDI:
    case MMGNumber::NUMBERSTATE_CUSTOM:
      return qRound((midi->value() / 127.0) * (higher - lower) + lower);
    case MMGNumber::NUMBERSTATE_IGNORE:
      return default_value;
    default:
      return number;
  }
}
// End MMGNumber

// MMGString
MMGString::MMGString(const QJsonObject &json_obj, const QString &preferred, int fallback_num)
{
  if (json_obj[preferred].isObject()) {
    // v2.3.0+
    QJsonObject string_obj = json_obj[preferred].toObject();
    set_str(string_obj["string"].toString());
    set_state(string_obj["state"].toInt());
  } else if (json_obj[preferred].isString()) {
    // v2.1.0+
    set_str(json_obj[preferred].toString());
    set_state(json_obj[preferred + "_state"].toInt());
    if ((uint)str_state > 2) str_state = STRINGSTATE_FIXED;
  } else {
    // pre v2.1.0
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
  if (use_state) {
    QJsonObject string_obj;
    string_obj["string"] = string;
    string_obj["state"] = (int)str_state;
    json_obj[prefix] = string_obj;
  } else {
    json_obj[prefix] = string;
  }
}
// End MMGString

// MMGTimer
MMGTimer::MMGTimer(QObject *parent) : QTimer(parent)
{
  connect(this, &QTimer::timeout, this, &MMGTimer::stopTimer);
  connect(this, &MMGTimer::stopping, this, &QTimer::stop);
  connect(this, &MMGTimer::resetting, this, QOverload<int>::of(&QTimer::start));
}

void MMGTimer::reset(int time)
{
  emit resetting(time);
};
// End MMGTimer

const QByteArray json_to_str(const QJsonObject &json_obj)
{
  return QJsonDocument(json_obj).toJson(QJsonDocument::Compact);
}

const QJsonObject json_from_str(const QByteArray &str)
{
  return QJsonDocument::fromJson(str).object();
}

void set_message_labels(const QString &type, MMGNumberDisplay *note_display,
			MMGNumberDisplay *value_display)
{
  if (type.contains("Note")) {
    note_display->setVisible(true);
    note_display->setDescription("Note #");
    value_display->setDescription("Velocity");
  } else if (type == "Control Change") {
    note_display->setVisible(true);
    note_display->setDescription("Control #");
    value_display->setDescription("Value");
  } else if (type == "Program Change") {
    note_display->setVisible(false);
    value_display->setDescription("Program #");
  } else if (type == "Pitch Bend") {
    note_display->setVisible(false);
    value_display->setDescription("Pitch Adjust");
  }
}

bool bool_from_str(const QString &str)
{
  return str == "True" || str == "Yes" || str == "Show" || str == "Locked";
}

double num_from_str(const QString &str)
{
  return str.toDouble();
}

QString num_to_str(int num, const QString &prefix)
{
  return QString::number(num).prepend(prefix);
}

void open_message_box(const QString &title, const QString &text)
{
  MMGPair c{title, text};

  obs_queue_task(
    OBS_TASK_UI,
    [](void *param) {
      auto content = reinterpret_cast<MMGPair *>(param);
      QMessageBox::information(nullptr, content->key, content->val.value<QString>());
    },
    &c, true);
}

void transfer_bindings(short mode, const QString &dest, const QString &source)
{
  if (mode < 0 || mode > 2) return;

  if (source == dest) {
    open_message_box("Transfer Error",
		     "Failed to transfer bindings: Cannot transfer bindings to the same device.");
    return;
  }

  MMGDevice *dest_device = global()->find(dest);
  if (!dest_device) {
    open_message_box("Transfer Error",
		     "Failed to transfer bindings: Destination device is invalid.");
    return;
  }

  MMGDevice *source_device = global()->find(source);
  if (!source_device) {
    open_message_box("Transfer Error", "Failed to transfer bindings: Source device is invalid.");
    return;
  }

  if (source_device == dest_device) {
    open_message_box("Transfer Error",
		     "Failed to transfer bindings: Cannot transfer bindings to the same device.");
    return;
  }

  MMGDevice *source_copy = new MMGDevice;
  source_device->copy(source_copy);
  source_copy->setName(dest);

  // REMOVE (occurs for both move and replace)
  if (mode > 0) source_device->clear();

  // REPLACE
  if (mode == 2) dest_device->clear();

  // COPY (occurs for all three modes)
  source_copy->copy(dest_device);

  // Clearing before deleting is important because it allows the
  // destination device to take control of the pointers. If this
  // were not the case, the copied pointers would be deleted by
  // the this MMGDevice's destructor.
  source_copy->clear();
  delete source_copy;

  open_message_box("Transfer Success", "Bindings successfully transferred.");
}

const vec3 get_obs_property_bounds(obs_property_t *prop)
{
  vec3 bounds = {0};
  if (!prop) return bounds;

  if (obs_property_get_type(prop) == OBS_PROPERTY_INT) {
    vec3_set(&bounds, obs_property_int_min(prop), obs_property_int_max(prop),
	     obs_property_int_step(prop));
  } else if (obs_property_get_type(prop) == OBS_PROPERTY_FLOAT) {
    vec3_set(&bounds, obs_property_float_min(prop), obs_property_float_max(prop),
	     obs_property_float_step(prop));
  }

  return bounds;
}

const vec3 get_obs_property_bounds(obs_source_t *source, const QString &field)
{
  if (!source) return vec3();

  obs_properties_t *props = obs_source_properties(source);
  if (!props) {
    obs_properties_destroy(props);
    return vec3();
  }
  obs_property_t *prop = obs_properties_get(props, field.qtocs());
  const vec3 bounds = get_obs_property_bounds(prop);
  obs_properties_destroy(props);
  return bounds;
}

const QList<MMGPair> get_obs_property_options(obs_property_t *prop)
{
  QList<MMGPair> options;

  for (size_t i = 0; i < obs_property_list_item_count(prop); ++i) {
    MMGPair strs;

    strs.key = obs_property_list_item_name(prop, i);

    switch (obs_property_list_format(prop)) {
      case OBS_COMBO_FORMAT_FLOAT:
	strs.val = obs_property_list_item_float(prop, i);
	break;
      case OBS_COMBO_FORMAT_INT:
	strs.val = obs_property_list_item_int(prop, i);
	break;
      case OBS_COMBO_FORMAT_STRING:
	strs.val = obs_property_list_item_string(prop, i);
	break;
      default:
	break;
    }

    options.append(strs);
  }

  return options;
}

const QList<MMGPair> get_obs_property_options(obs_source_t *source, const QString &field)
{
  if (!source) return QList<MMGPair>();

  obs_properties_t *props = obs_source_properties(source);
  if (!props) {
    obs_properties_destroy(props);
    return QList<MMGPair>();
  }
  obs_property_t *prop = obs_properties_get(props, field.qtocs());
  const QList<MMGPair> options = get_obs_property_options(prop);
  obs_properties_destroy(props);
  return options;
}

void debug_json(const QJsonValue &val)
{
  qDebug("%s", val.toVariant().toString().qtocs());
}

uint argb_abgr(uint rgb)
{
  return (rgb & 0xFF000000) | ((rgb & 0xFF0000) >> 16) | (rgb & 0xFF00) | ((rgb & 0xFF) << 16);
}

void obs_source_custom_update(obs_source_t *source, const QJsonObject &action_json,
			      const MMGMessage *midi)
{
  if (!source) return;

  std::lock_guard guard(custom_update);

  OBSDataAutoRelease source_data = obs_source_get_settings(source);
  QJsonObject source_json = json_from_str(obs_data_get_json(source_data));
  QJsonObject final_json;

  for (const QString &key : action_json.keys()) {
    QJsonObject key_obj = action_json[key].toObject();
    switch (key_obj["state"].toInt()) {
      case 1:
	// MIDI
	if (key_obj.contains("number")) {
	  // Number Field
	  final_json[key] =
	    (midi->value() / 127.0) * (key_obj["higher"].toDouble() - key_obj["lower"].toDouble()) +
	    key_obj["lower"].toDouble();
	} else if (key_obj.contains("value")) {
	  // String Fields
	  QList<MMGPair> options = get_obs_property_options(source, key);
	  if (midi->value() >= options.size()) break;
	  final_json[key] = QJsonValue::fromVariant(options[(int)midi->value()].val);
	} else if (key_obj.contains("string")) {
	  // Text and Path Fields
	  QString str = action_json[key].toString();
	  str.replace("${type}", midi->type());
	  str.replace("${channel}", num_to_str(midi->channel()));
	  str.replace("${note}", num_to_str(midi->note()));
	  str.replace("${value}", num_to_str(midi->value()));
	  str.replace("${control}", num_to_str(midi->note()));
	  final_json[key] = str;
	}
	break;

      case 2:
	// TOGGLE (and CUSTOM)
	if (key_obj.contains("number")) {
	  // Number Field
	  final_json[key] =
	    (midi->value() / 127.0) * (key_obj["higher"].toDouble() - key_obj["lower"].toDouble()) +
	    key_obj["lower"].toDouble();
	} else {
	  // Boolean Field
	  final_json[key] = !source_json[key].toBool();
	}
	break;

      case 3:
	// IGNORE
	break;

      default:
	// NORMAL
	if (key_obj.contains("number")) {
	  // Number Field
	  final_json[key] = key_obj["number"];
	} else if (key_obj.contains("value")) {
	  // String Fields
	  final_json[key] = key_obj["value"];
	} else if (key_obj.contains("color")) {
	  // Color Field
	  final_json[key] = key_obj["color"];
	} else if (key_obj.contains("string")) {
	  // Text and Path Fields
	  final_json[key] = key_obj["string"];
	} else {
	  // Font Field
	  final_json[key] = key_obj;
	}
	break;
    }
  }
  obs_source_update(source, OBSDataAutoRelease(obs_data_create_from_json(json_to_str(final_json))));
}

}
