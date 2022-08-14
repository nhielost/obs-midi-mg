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
#include "mmg-action.h"

#include <obs-frontend-api.h>

namespace MMGUtils {

// LCDData
LCDData::LCDData(QLCDNumber *lcd_ptr, std::function<void(double)> func)
{
	lcd = lcd_ptr;
	value_func = func;
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
	internal_val = internal_val - major_step <= minimum
			       ? minimum
			       : internal_val - major_step;
	display();
}
void LCDData::down_minor()
{
	internal_val = internal_val - minor_step <= minimum
			       ? minimum
			       : internal_val - minor_step;
	display();
}
void LCDData::up_minor()
{
	internal_val = internal_val + minor_step >= maximum
			       ? maximum
			       : internal_val + minor_step;
	display();
}
void LCDData::up_major()
{
	internal_val = internal_val + major_step >= maximum
			       ? maximum
			       : internal_val + major_step;
	display();
}
void LCDData::reset(double value)
{
	internal_val = value;
	display();
}

void LCDData::display()
{
	if (use_time) {
		lcd->display(QTime(internal_val / 3600.0,
				   fmod(internal_val / 60.0, 60.0),
				   fmod(internal_val, 60.0))
				     .toString("hh:mm:ss"));
	} else {
		lcd->display(internal_val);
	}
	value_func(internal_val);
}
// End LCDData

bool json_key_exists(const QJsonObject &obj, QString key,
		     QJsonValue::Type value_type)
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

QString next_default_name(MMGModes mode)
{
	if (!global() || (mode != MMGModes::MMGMODE_DEVICE &&
			  !global()->get_active_device()))
		return "";

	switch (mode) {
	case MMGModes::MMGMODE_DEVICE:
		return "Untitled Device " +
		       QVariant(global()->get_next_error_default()).toString();
	case MMGModes::MMGMODE_BINDING:
		return "Untitled Binding " +
		       QVariant(global()->get_active_device()
					->get_next_binding_default())
			       .toString();
	case MMGModes::MMGMODE_MESSAGE:
		return "Untitled Message " +
		       QVariant(global()->get_active_device()
					->get_next_message_default())
			       .toString();
	case MMGModes::MMGMODE_ACTION:
		return "Untitled Action " +
		       QVariant(global()->get_active_device()
					->get_next_action_default())
			       .toString();
	default:
		return "";
	}
}

void call_midi_callback(const libremidi::message &message)
{
	if (!global()->is_running())
		return;
	MMGSharedMessage incoming(new MMGMessage(message));
	for (MMGDevice *const device : global()->get_devices()) {
		if (device->input_port_open())
			device->do_all_actions(incoming);
	}
}

bool bool_from_str(const QString &str)
{
	return str == "True" || str == "Yes" || str == "Show" ||
	       str == "Locked";
}

std::pair<uint, uint> get_obs_dimensions()
{
	obs_video_info video_info;
	obs_get_video_info(&video_info);
	return {video_info.output_width, video_info.output_height};
}

std::pair<uint, uint> get_obs_source_dimensions(const QString &name)
{
	OBSSourceAutoRelease source = obs_get_source_by_name(name.qtocs());
	return {obs_source_get_width(source), obs_source_get_height(source)};
}

uint get_obs_scene_count()
{
	char **scene_names = obs_frontend_get_scene_names();
	uint count = 0;
	while (scene_names[count] != 0) {
		++count;
	}
	bfree(scene_names);
	return count;
}

double get_obs_media_length(const QString &name)
{
	OBSSourceAutoRelease source = obs_get_source_by_name(name.qtocs());
	return obs_source_media_get_duration(source) / 1000.0;
}

qulonglong get_obs_source_filter_count(const QString &name)
{
	OBSSourceAutoRelease source = obs_get_source_by_name(name.qtocs());
	return obs_source_filter_count(source);
}

}
