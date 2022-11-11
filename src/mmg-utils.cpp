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

#include <QMessageBox>

struct C {
	QString title;
	QString text;
};

void global_blog(int log_status, const QString &message)
{
	blog(log_status, "[obs-midi-mg] %s", qPrintable(message));
}

size_t get_name_count(char **names)
{
	size_t count = 0;
	while (names[count] != 0) {
		++count;
	}
	bfree(names);
	return count;
}

namespace MMGUtils {

// LCDData
LCDData::LCDData(QLCDNumber *lcd_ptr)
{
	lcd = lcd_ptr;
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
void LCDData::set_default_value(double val)
{
	if (val > maximum || val < minimum)
		return;
	default_val = val;
	set_value(default_val);
}
void LCDData::set_value(double val)
{
	if (val > maximum || val < minimum)
		return;
	internal_val = val;
	display();
}

void LCDData::display()
{
	if (lcd->isEnabled()) {
		if (use_time) {
			lcd->display(QTime(internal_val / 3600.0,
					   fmod(internal_val / 60.0, 60.0),
					   fmod(internal_val, 60.0))
					     .toString("hh:mm:ss"));
		} else {
			lcd->display(internal_val);
		}
	} else {
		lcd->display(lcd->digitCount() == 8 ? "   ---  " : "---");
	}
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

void call_midi_callback(const libremidi::message &message)
{
	MMGSharedMessage incoming(new MMGMessage(message));
	if (global()->is_listening(incoming.get()))
		return;
	if (global()->find_current_device()->input_port_open())
		global()->find_current_device()->do_all_actions(incoming);
}

bool bool_from_str(const QString &str)
{
	return str == "True" || str == "Yes" || str == "Show" ||
	       str == "Locked";
}

template<typename N> const QString num_to_str(const N &num)
{
	return QVariant::fromValue(num).toString();
}

void open_message_box(const QString &title, const QString &text)
{
	C c{title, text};
	obs_queue_task(
		OBS_TASK_UI,
		[](void *param) {
			auto content = reinterpret_cast<C *>(param);
			QMessageBox::information(nullptr, content->title,
						 content->text);
		},
		&c, true);
}

void transfer_bindings(short mode, const QString &source, const QString &dest)
{
	if (mode < 0 || mode > 2)
		return;

	if (source == dest) {
		open_message_box(
			"Transfer Error",
			"Failed to transfer bindings: Cannot transfer bindings to the same device.");
		return;
	}

	MMGDevice *const source_device = global()->find_device(source);
	if (!source_device) {
		open_message_box(
			"Transfer Error",
			"Failed to transfer bindings: Source device is invalid.");
		return;
	}

	MMGDevice *const dest_device = global()->find_device(dest);
	if (!dest_device) {
		open_message_box(
			"Transfer Error",
			"Failed to transfer bindings: Destination device is invalid.");
		return;
	}

	if (source_device == dest_device) {
		open_message_box(
			"Transfer Error",
			"Failed to transfer bindings: Cannot transfer bindings to the same device.");
		return;
	}

	// Deep copy of bindings
	MMGDevice *source_copy = new MMGDevice;
	source_device->deep_copy(source_copy);

	// REMOVE (occurs for both move and replace)
	if (mode > 0)
		source_device->clear();

	// REPLACE
	if (mode == 2)
		dest_device->clear();

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

	open_message_box("Transfer Success",
			 "Bindings successfully transferred.");
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

size_t get_obs_scene_count()
{
	return get_name_count(obs_frontend_get_scene_names());
}

size_t get_obs_profile_count()
{
	return get_name_count(obs_frontend_get_profiles());
}

size_t get_obs_collection_count()
{
	return get_name_count(obs_frontend_get_scene_collections());
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
