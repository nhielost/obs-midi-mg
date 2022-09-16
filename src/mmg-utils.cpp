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
		lcd->display(lcd->digitCount() == 8 ? "   OFF  " : "OFF");
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

void set_help_text(MMGModes mode, QLabel *name, QLabel *text, const void *obj)
{
	QString str;
	if (mode == MMGModes::MMGMODE_DEVICE) {
		const MMGDevice *casted = static_cast<const MMGDevice *>(obj);
		name->setText("Device Properties");
		str += "The device **";
		str += casted->get_name();
		str += "** ";
		str += casted->input_device_status() == "Ready"
			       ? "has"
			       : "does not have";
		str += " a connected input port ready to be used.\n\nThe device **";
		str += casted->get_name();
		str += "** ";
		str += casted->output_device_status() == "Ready"
			       ? "has"
			       : "does not have";
		str += " a connected output port ready to be used.\n\nThe device **";
		str += casted->get_name();
		str += "** is currently ";
		str += global()->get_active_device_name() == casted->get_name()
			       ? "the device that is receiving messages."
			       : "not the device that is receiving messages. "
				 "Click the *Set As Active Device* button to change it to this device.";
		str += "\n\nThis device currently has *";
		str += QVariant::fromValue(casted->get_bindings().size())
			       .toString();
		str += "* bindings. Click *Edit Input Bindings* to view them.";
	} else if (mode == MMGModes::MMGMODE_BINDING) {
		const MMGBinding *casted = static_cast<const MMGBinding *>(obj);
		name->setText("Binding Properties");
		str += "The binding **";
		str += casted->get_name();
		str += "** is executing actions using the *";
		switch (casted->get_reception()) {
		case MMGBinding::Reception::MMGBINDING_CONSECUTIVE:
			str += "Consecutive Reception Method*. When the final message has been received, "
			       "all actions will use that incoming message as their value.";
			break;
		case MMGBinding::Reception::MMGBINDING_CORRESPONDENCE:
			str += "Correspondence Reception Method*. When the final message has been received, "
			       "each action will use the corresponding message as their value. "
			       "(The first action in the list will use the first message in the list, the second will use the second and so on.)";
			break;
		case MMGBinding::Reception::MMGBINDING_MULTIPLY:
			str += "Multiply Reception Method*. When the final message has been received, "
			       "each action will use every message that was used in the binding. "
			       "(The first action will occur the same amount of times as there were messages, the second action will follow suit, etc.)";
			break;
		default:
			return;
		}
		str += "\n\nThe binding **";
		str += casted->get_name();
		str += "** has toggling set to *";
		switch (casted->get_toggling()) {
		case MMGBinding::Toggling::MMGBINDING_TOGGLE_OFF:
			str += "Off*. Messages will not be changed by the binding, and an unlimited amount can be used.";
			break;
		case MMGBinding::Toggling::MMGBINDING_TOGGLE_NOTE:
			str += "Note Type*. Note type toggling enables note messages to switch between Note On and Note Off when received. "
			       "However, only one message is allowed in the binding if this is enabled, "
			       "and the reception method doesn't matter when there is one message.";
			break;
		case MMGBinding::Toggling::MMGBINDING_TOGGLE_VALUE:
			str += "Velocity*. Velocity toggling enables messages to switch between a velocity of 127 and 0 when received. "
			       "However, only one message is allowed in the binding if this is enabled, "
			       "and the reception method doesn't matter when there is one message.";
			break;
		case MMGBinding::Toggling::MMGBINDING_TOGGLE_BOTH:
			str += "Note Type and Velocity*. This toggling enables messages to switch between both the Note On and Note Off types, "
			       "as well as switching the velocity of 127 and 0 when received. "
			       "Still, only one message is allowed in the binding if this is enabled, "
			       "and the reception method doesn't matter when there is one message.";
			break;
		}
		str += "\n\nThis binding currently has *";
		str += QVariant::fromValue(casted->message_size()).toString();
		str += "* messages. Click *Edit Messages* to view them.";
		str += "\n\nThis binding currently has *";
		str += QVariant::fromValue(casted->action_size()).toString();
		str += "* actions. Click *Edit Actions* to view them.";
	} else if (mode == MMGModes::MMGMODE_MESSAGE) {
		name->setText("Message Properties");
		str = "This menu is currently under construction.";
	} else if (mode == MMGModes::MMGMODE_ACTION) {
		name->setText("Action Properties");
		str = "This menu is currently under construction.";
	}
	text->setText(str);
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
