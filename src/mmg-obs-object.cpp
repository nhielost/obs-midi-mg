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

#include "mmg-obs-object.h"
#include "mmg-config.h"
#include "ui/mmg-action-display.h"

#include <mutex>

#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>

namespace MMGOBSFields {

static std::mutex custom_update;

void addRefreshCallback(MMGWidgets::MMGValueManager *display, QObject *binder, const MMGCallback &cb)
{
	QObject::connect(display, &MMGWidgets::MMGValueManager::refreshRequested, binder, cb);
}

// MMGOBSProperty
MMGOBSProperty::MMGOBSProperty(MMGOBSPropertyManager *parent, obs_property_t *prop)
	: QObject(parent),
	  _name(obs_property_name(prop)),
	  _prop(prop)
{
	setObjectName(_name);
}
// End MMGOBSProperty

// int32_t
template <> MMGParams<int32_t> MMGOBSField<int32_t>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE |
			   OPTION_ALLOW_INCREMENT,
		.default_value = storageValue(),
		.lower_bound = (double)obs_property_int_min(_prop),
		.upper_bound = (double)obs_property_int_max(_prop),
		.step = (double)obs_property_int_step(_prop),
		.incremental_bound = (obs_property_int_max(_prop) - obs_property_int_min(_prop)) / 2.0,
	};
}
// End int32_t

// float
template <> MMGParams<float> MMGOBSField<float>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE |
			   OPTION_ALLOW_INCREMENT,
		.default_value = storageValue(),
		.lower_bound = obs_property_float_min(_prop),
		.upper_bound = obs_property_float_max(_prop),
		.step = obs_property_float_step(_prop),
		.incremental_bound = (obs_property_float_max(_prop) - obs_property_float_min(_prop)) / 2.0,
	};
}
// float

// MMGString
static MMGStringTranslationMap stringPropertyNames(obs_property_t *prop)
{
	MMGStringTranslationMap values;
	if (obs_property_get_type(prop) != OBS_PROPERTY_LIST) return values;

	for (size_t i = 0; i < obs_property_list_item_count(prop); ++i) {
		MMGString value;

		switch (obs_property_list_format(prop)) {
			case OBS_COMBO_FORMAT_FLOAT:
				value = std::to_string(obs_property_list_item_float(prop, i)).c_str();
				break;

			case OBS_COMBO_FORMAT_INT:
				value = std::to_string(obs_property_list_item_int(prop, i)).c_str();
				break;

			case OBS_COMBO_FORMAT_STRING:
				value = obs_property_list_item_string(prop, i);
				break;

			case OBS_COMBO_FORMAT_BOOL:
				value = obs_property_list_item_bool(prop, i) ? "true" : "false";
				break;

			default:
				break;
		}

		values.insert(value, nontr(obs_property_list_item_name(prop, i)));
	}

	return values;
}

template <> MMGString MMGOBSField<MMGString>::getPropertyValue(const QJsonObject &data) const
{
	switch (obs_property_list_format(_prop)) {
		case OBS_COMBO_FORMAT_INT:
			return std::to_string(MMGJson::getValue<long long>(data, _name)).c_str();

		case OBS_COMBO_FORMAT_FLOAT:
			return std::to_string(MMGJson::getValue<float>(data, _name)).c_str();

		case OBS_COMBO_FORMAT_BOOL:
			return MMGJson::getValue<bool>(data, _name) ? "true" : "false";

		default:
		case OBS_COMBO_FORMAT_STRING:
			return MMGJson::getValue<MMGString>(data, _name);
	}
}

template <> void MMGOBSField<MMGString>::setPropertyValue(QJsonObject &data, const MMGString &value) const
{
	switch (obs_property_list_format(_prop)) {
		case OBS_COMBO_FORMAT_INT:
			MMGJson::setValue(data, _name, std::strtoll(value, nullptr, 10));
			break;

		case OBS_COMBO_FORMAT_FLOAT:
			MMGJson::setValue(data, _name, std::strtof(value, nullptr));
			break;

		case OBS_COMBO_FORMAT_BOOL:
			MMGJson::setValue(data, _name, value == "true");
			break;

		default:
		case OBS_COMBO_FORMAT_STRING:
			MMGJson::setValue(data, _name, value);
			break;
	}
}

template <> MMGParams<MMGString> MMGOBSField<MMGString>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE,
		.default_value = storageValue(),
		.bounds = stringPropertyNames(_prop),
		.placeholder = MMGText(),
		.text_editable = obs_property_list_type(_prop) == OBS_COMBO_TYPE_EDITABLE,
	};
}
// MMGString

// bool
template <> MMGParams<bool> MMGOBSField<bool>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE,
		.default_value = storageValue(),
	};
}
// End bool

// QColor
template <> MMGParams<QColor> MMGOBSField<QColor>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE,
		.default_value = storageValue(),
		.alpha = obs_property_get_type(_prop) == OBS_PROPERTY_COLOR_ALPHA,
	};
}
// End QColor

// QFont
template <> MMGParams<QFont> MMGOBSField<QFont>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE,
		.default_value = storageValue(),
	};
}
// End QFont

// MMGOBSGroupField
MMGOBSGroupField::MMGOBSGroupField(MMGOBSPropertyManager *parent, obs_property_t *prop) : MMGOBSProperty(parent, prop)
{
	group_props = new MMGOBSPropertyManager(qobject_cast<MMGOBSObject *>(parent->parent()),
						obs_property_group_content(prop));
}

MMGOBSGroupField::~MMGOBSGroupField()
{
	group_props->deleteLater();
}

void MMGOBSGroupField::loadProperty(const QJsonObject &json_obj)
{
	if (obs_property_group_type(_prop) == OBS_GROUP_CHECKABLE)
		controller = MMGJson::getValue<bool>(json_obj, _name);
	group_props->loadProperties(json_obj);
}

void MMGOBSGroupField::loadData(const QJsonObject &json_obj)
{
	if (obs_property_group_type(_prop) == OBS_GROUP_CHECKABLE)
		controller = MMGJson::getValue<bool>(json_obj, _name);
	group_props->loadData(json_obj);
}

void MMGOBSGroupField::jsonProperty(QJsonObject &json_obj) const
{
	if (obs_property_group_type(_prop) == OBS_GROUP_CHECKABLE) MMGJson::setValue<bool>(json_obj, _name, controller);
	group_props->jsonProperties(json_obj);
}

void MMGOBSGroupField::jsonData(QJsonObject &json_obj) const
{
	if (obs_property_group_type(_prop) == OBS_GROUP_CHECKABLE) MMGJson::setValue<bool>(json_obj, _name, controller);
	group_props->jsonData(json_obj);
}

void MMGOBSGroupField::createDisplay(MMGWidgets::MMGValueManager *display)
{
	auto *group_display = new MMGWidgets::MMGValueManager(display);

	if (obs_property_group_type(_prop) == OBS_GROUP_NORMAL) {
		QLabel *label = new QLabel(group_display);
		label->setWordWrap(true);
		label->setStyleSheet("QLabel {font-weight: bold;}");

		connect(display, &MMGWidgets::MMGValueManager::refreshRequested, label,
			std::bind(&MMGOBSGroupField::displayNormalChanged, this, label));
		group_display->addCustom(label);
	} else if (obs_property_group_type(_prop) == OBS_GROUP_CHECKABLE) {
		if (!controller_params) controller_params.reset(new MMGParams<bool>(controllerParams()));
		connect(display, &MMGWidgets::MMGValueManager::refreshRequested, group_display,
			std::bind(&MMGOBSGroupField::displayCheckableChanged, this, group_display));
		display->addNew(&controller, controller_params.get(), std::bind(&MMGOBSProperty::valueChanged, this));
	}

	group_props->createDisplays(group_display);
	display->addManager(group_display);
}

void MMGOBSGroupField::displayNormalChanged(QLabel *label) const
{
	label->setText(nontr(obs_property_description(_prop)));
}

void MMGOBSGroupField::displayCheckableChanged(MMGWidgets::MMGValueManager *display) const
{
	controller_params->desc = nontr(obs_property_description(_prop));
	display->setEnabled(controller);
}

MMGParams<bool> MMGOBSGroupField::controllerParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_EMPHASIS,
		.default_value = controller,
	};
}

void MMGOBSGroupField::execute(const MMGMappingTest &test, QJsonObject &data) const
{
	group_props->execute(test, data);
}

void MMGOBSGroupField::processEvent(MMGMappingTest &test, const QJsonObject &data) const
{
	group_props->processEvent(test, data);
}
// End MMGOBSGroupField

// QDir
template <> MMGParams<QDir> MMGOBSField<QDir>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE,
		.default_value = storageValue(),
		.default_ask_path = obs_property_path_default_path(_prop),
		.path_filters = obs_property_path_filter(_prop),
		.dialog_type = obs_property_path_type(_prop),
	};
}
// End QDir

// QString
template <> MMGParams<QString> MMGOBSField<QString>::widgetParams() const
{
	return {
		.desc = nontr(obs_property_description(_prop)),
		.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_IGNORE | OPTION_ALLOW_TOGGLE,
		.text_type = obs_property_text_type(_prop),
		.default_value = storageValue(),
	};
}
// End QString

// MMGOBSPropertyManager
MMGOBSPropertyManager::MMGOBSPropertyManager(MMGOBSObject *parent, obs_properties_t *props) : QObject(parent)
{
	obs_property_t *prop = obs_properties_first(props);

	do {
		MMGOBSProperty *new_prop;

		switch (obs_property_get_type(prop)) {
			case OBS_PROPERTY_INT:
				new_prop = new MMGOBSField<int32_t>(this, prop);
				break;

			case OBS_PROPERTY_FLOAT:
				new_prop = new MMGOBSField<float>(this, prop);
				break;

			case OBS_PROPERTY_LIST:
				new_prop = new MMGOBSField<MMGString>(this, prop);
				break;

			case OBS_PROPERTY_BOOL:
				new_prop = new MMGOBSField<bool>(this, prop);
				break;

			case OBS_PROPERTY_COLOR:
			case OBS_PROPERTY_COLOR_ALPHA:
				new_prop = new MMGOBSField<QColor>(this, prop);
				break;

			case OBS_PROPERTY_FONT:
				new_prop = new MMGOBSField<QFont>(this, prop);
				break;

			case OBS_PROPERTY_GROUP:
				new_prop = new MMGOBSGroupField(this, prop);
				break;

			case OBS_PROPERTY_PATH:
				new_prop = new MMGOBSField<QDir>(this, prop);
				break;

			case OBS_PROPERTY_TEXT:
				if (obs_property_text_type(prop) != OBS_TEXT_INFO) {
					new_prop = new MMGOBSField<QString>(this, prop);
					break;
				}
				[[fallthrough]];

			case OBS_PROPERTY_EDITABLE_LIST:
			case OBS_PROPERTY_FRAME_RATE:
			default:
				continue;
		}

		connect(new_prop, &MMGOBSProperty::valueChanged, this, &MMGOBSPropertyManager::propertyChanged);
		_props += new_prop;
	} while (obs_property_next(&prop) != 0);
}

const char *MMGOBSPropertyManager::sourceId() const
{
	return qobject_cast<MMGOBSObject *>(parent())->sourceId();
}

void MMGOBSPropertyManager::loadProperties(const QJsonObject &current_json, const QJsonObject &default_json) const
{
	for (MMGOBSProperty *prop : _props) {
		prop->loadProperty(current_json.contains(prop->objectName()) ? current_json : default_json);
		prop->setDefaultValue(default_json);
	}
}
// End MMGOBSPropertyManager

// MMGOBSObject
MMGOBSObject::MMGOBSObject(QObject *parent, const MMGString &source_id, const QJsonObject &json_obj) : QObject(parent)
{
	if (!source_id.isEmpty()) changeSource(source_id, json_obj);
}

void MMGOBSObject::changeSource(const MMGString &source_id, const QJsonObject &json_obj)
{
	if (source_id == source_uuid) return;

	if (!!props_manager) {
		delete props_manager;
		props_manager = nullptr;
	}

	source_uuid = source_id;
	property_json = {};
	if (config()->fileVersion() < MMGConfig::VERSION_3_0) {
		if (json_obj["json"].isString()) {
			property_json = MMGJson::toObject(json_obj["json"].toString().toUtf8());
		} else if (json_obj["json_str"].isString()) {
			property_json = MMGJson::toObject(json_obj["json_str"].toString().toUtf8());
		} else if (json_obj["str4"].isString()) {
			property_json = MMGJson::toObject(json_obj["str4"].toString().toUtf8());
		}
	} else {
		property_json = json_obj["json"].toObject();
	}

	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(source_uuid);
	if (!obs_source) return;

	if (!!props) obs_properties_destroy(props);
	props = obs_source_properties(obs_source);
	props_manager = new MMGOBSPropertyManager(this, props);
	connect(props_manager, &MMGOBSPropertyManager::propertyChanged, this, &MMGOBSObject::updateProperties);

	loadProperties();
	if (!property_json.isEmpty()) {
		props_manager->loadData(property_json);
		props_manager->jsonProperties(property_json);
	}

	if (!!created_display) {
		created_display->removeExcept(front_widgets_num);
		createDisplay(created_display);
	}
	emit sourceChanged();
}

void MMGOBSObject::loadProperties()
{
	if (props_manager->size() == 0) return;

	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(source_uuid);

	OBSDataAutoRelease source_data = obs_source_get_settings(obs_source);
	OBSDataAutoRelease source_defaults = obs_data_get_defaults(source_data);

	QJsonObject current_json = MMGJson::toObject(obs_data_get_json(source_data));
	QJsonObject default_json = MMGJson::toObject(obs_data_get_json(source_defaults));

	props_manager->loadProperties(current_json, default_json);
}

void MMGOBSObject::updateProperties()
{
	props_manager->jsonProperties(property_json);
	OBSDataAutoRelease obs_data = obs_data_create_from_json(MMGJson::toString(property_json));
	obs_properties_apply_settings(props, obs_data);
	obs_source_update_properties(OBSSourceAutoRelease(obs_get_source_by_uuid(source_uuid)));

	emit propertyChanged();
}

void MMGOBSObject::json(QJsonObject &json_obj) const
{
	QJsonObject save_obj;
	if (!!props_manager) props_manager->jsonData(save_obj);
	json_obj["json"] = save_obj;
}

void MMGOBSObject::copy(MMGOBSObject *dest) const
{
	QJsonObject this_json;
	if (!!props_manager) props_manager->jsonData(this_json);
	dest->changeSource(source_uuid, this_json);
}

void MMGOBSObject::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	front_widgets_num = display->scrollCount();
	props_manager->createDisplays(display);
	created_display = display;
}

void MMGOBSObject::execute(const MMGMappingTest &test) const
{
	std::lock_guard custom_guard(custom_update);
	QJsonObject final_json = property_json;

	props_manager->execute(test, final_json);

	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(source_uuid);
	OBSDataAutoRelease obs_source_data = obs_data_create_from_json(MMGJson::toString(final_json));
	obs_source_update(obs_source, obs_source_data);

	property_json = final_json;
}

void MMGOBSObject::processEvent(MMGMappingTest &test) const
{
	std::lock_guard custom_guard(custom_update);

	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(source_uuid);
	OBSDataAutoRelease source_data = obs_source_get_settings(obs_source);
	property_json = MMGJson::toObject(obs_data_get_json(source_data));

	props_manager->processEvent(test, property_json);
}
// End MMGOBSObject

} // namespace MMGOBSFields
