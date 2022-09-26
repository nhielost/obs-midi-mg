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
void LCDData::reset(double value)
{
	internal_val = value;
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

template<> void format_help_text(QString &str, const MMGDevice *obj)
{
	str.replace("{name}", obj->get_name());
	str.replace("{in_status}", obj->input_device_status() == "Ready"
					   ? "has"
					   : "does not have");
	str.replace("{out_status}", obj->output_device_status() == "Ready"
					    ? "has"
					    : "does not have");
	str.replace(
		"{active?}",
		global()->get_active_device_name() == obj->get_name()
			? "the device that is receiving messages"
			: "not the device that is receiving messages. "
			  "Click the Set As Active Device button to change it to this device");
	str.replace("{size}", num_to_str(obj->get_bindings().size()));
}

template<> void format_help_text(QString &str, const MMGBinding *obj)
{
	str.replace("{name}", obj->get_name());
	QString reception_str;
	switch (obj->get_reception()) {
	case MMGBinding::Reception::MMGBINDING_CONSECUTIVE:
		reception_str =
			"Consecutive Reception Method. When the final message has been received, "
			"all actions will use that incoming message as their value";
		break;
	case MMGBinding::Reception::MMGBINDING_CORRESPONDENCE:
		reception_str =
			"Correspondence Reception Method. When the final message has been received, "
			"each action will use the corresponding message as their value. "
			"(The first action in the list will use the first message in the list, the second will use the second and so on)";
		break;
	case MMGBinding::Reception::MMGBINDING_MULTIPLY:
		reception_str =
			"Multiply Reception Method. When the final message has been received, "
			"each action will use every message that was used in the binding. "
			"(The first action will occur the same amount of times as there were messages, the second action will follow suit, etc.)";
		break;
	default:
		return;
	}
	str.replace("{reception}", reception_str);
	QString toggling_str;
	switch (obj->get_toggling()) {
	case MMGBinding::Toggling::MMGBINDING_TOGGLE_OFF:
		toggling_str =
			"Off. Messages will not be changed by the binding, and an unlimited amount can be used";
		break;
	case MMGBinding::Toggling::MMGBINDING_TOGGLE_NOTE:
		toggling_str =
			"Note Type. Note type toggling enables note messages to switch between Note On and Note Off when received. "
			"However, only one message is allowed in the binding if this is enabled, "
			"and the reception method doesn't matter when there is one message";
		break;
	case MMGBinding::Toggling::MMGBINDING_TOGGLE_VALUE:
		toggling_str =
			"Velocity. Velocity toggling enables messages to switch between a velocity of 127 and 0 when received. "
			"However, only one message is allowed in the binding if this is enabled, "
			"and the reception method doesn't matter when there is one message";
		break;
	case MMGBinding::Toggling::MMGBINDING_TOGGLE_BOTH:
		toggling_str =
			"Note Type and Velocity. This toggling enables messages to switch between both the Note On and Note Off types, "
			"as well as switching the velocity of 127 and 0 when received. "
			"Still, only one message is allowed in the binding if this is enabled, "
			"and the reception method doesn't matter when there is one message.";
		break;
	}
	str.replace("{toggling}", toggling_str);
	str.replace("{message_size}", num_to_str(obj->message_size()));
	str.replace("{action_size}", num_to_str(obj->action_size()));
}

template<> void format_help_text(QString &str, const MMGMessage *obj)
{
	str.replace("{name}", obj->get_name());
	str.replace(
		"{value}",
		obj->get_value() == -1
			? "Off, meaning that any value (0-127) can be received and the message will be fulfilled"
			: "" + num_to_str(obj->get_value()) +
				  ". To allow messages to use the full range of values (0-127), click the Require Value button in the bottom right");
}

template<> void format_help_text(QString &str, const MMGAction *obj)
{
	str.replace("{name}", obj->get_name());
	str.replace("{str1}", obj->get_str(0));
	str.replace("{str2}", obj->get_str(1));
	str.replace("{str3}", obj->get_str(2));
	str.replace("{num1}", num_to_str(obj->get_num(0)));
	str.replace("{num2}", num_to_str(obj->get_num(1)));
	str.replace("{num3}", num_to_str(obj->get_num(2)));
	str.replace("{num4}", num_to_str(obj->get_num(3)));
}

template<> void format_help_text(QString &str, const void *obj)
{
	str = "Invalid help text. Please report this as a bug.";
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
	// the MMGDevice destructor.
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
