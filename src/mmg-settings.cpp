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

#include "mmg-settings.h"
#include "mmg-config.h"

#include <QAbstractItemView>

using namespace MMGUtils;

// MMGSettings
MMGSettings::MMGSettings(MMGSettingsManager *parent) : QObject(parent), editable(&parent->isEditable()) {}
// End MMGSettings

// MMGGeneralSettings
MMGGeneralSettings::MMGGeneralSettings(MMGSettingsManager *parent, const QJsonObject &json_obj) : MMGSettings(parent)
{
	Q_UNUSED(json_obj);
}

void MMGGeneralSettings::json(QJsonObject &settings_obj) const
{
	Q_UNUSED(settings_obj);
}

void MMGGeneralSettings::createDisplay(QWidget *parent)
{
	MMGSettings::createDisplay(parent);
}
// End MMGGeneralSettings

// MMGBindingSettings
MMGBindingSettings::MMGBindingSettings(MMGSettingsManager *parent, const QJsonObject &json_obj) : MMGSettings(parent)
{
	reset_behavior = resetBehaviorOptions()[json_obj["reset_behavior"].toInt()];
}

void MMGBindingSettings::json(QJsonObject &settings_obj) const
{
	settings_obj["reset_behavior"] = resetBehavior();
}

void MMGBindingSettings::copy(MMGSettings *dest)
{
	auto casted = dynamic_cast<MMGBindingSettings *>(dest);
	if (!casted) return;

	casted->reset_behavior = reset_behavior.copy();
}

void MMGBindingSettings::createDisplay(QWidget *parent)
{
	MMGSettings::createDisplay(parent);

	MMGStringDisplay *reset_behavior_display = new MMGStringDisplay(display());
	reset_behavior_display->setDisplayMode(MMGStringDisplay::MODE_THIN);
	reset_behavior_display->setDescription(mmgtr("Preferences.Binding.ResetBehavior"));
	reset_behavior_display->setStorage(&reset_behavior, resetBehaviorOptions());
	connect(reset_behavior_display, &MMGStringDisplay::stringChanged, this,
		&MMGBindingSettings::resetBehaviorChanged);
	connect(reset_behavior_display, &MMGStringDisplay::stringChanged, this, &MMGBindingSettings::setLabel);

	reset_behavior_desc = new QLabel(display());
	reset_behavior_desc->setGeometry(0, 50, 330, 80);
	reset_behavior_desc->setWordWrap(true);
	reset_behavior_desc->setAlignment(Qt::AlignVCenter);
	setLabel();
}

const QStringList MMGBindingSettings::resetBehaviorOptions()
{
	return mmgtr_all("Preferences.Binding.ResetBehavior.Option", {"Yes", "No"});
}

void MMGBindingSettings::setLabel()
{
	reset_behavior_desc->setText(mmgtr_two("Preferences.Binding.ResetBehavior.Text", "No", "Yes", resetBehavior()));
}

// End MMGBindingSettings

// MMGMessageLogSettings
void MMGMessageLogSettings::json(QJsonObject &settings_obj) const
{
	Q_UNUSED(settings_obj);
}

void MMGMessageLogSettings::createDisplay(QWidget *parent)
{
	MMGSettings::createDisplay(parent);
}
// End MMGMessageLogSettings

// MMGSettingsManager
MMGSettings *MMGSettingsManager::add(const QJsonObject &json_obj)
{
	MMGSettings *settings = nullptr;

	switch (_list.size()) {
		case 0:
			settings = new MMGGeneralSettings(this, json_obj);
			break;

		case 1:
			settings = new MMGBindingSettings(this, json_obj);
			break;

		case 2:
			//settings = new MMGMessageLogSettings(this);
			break;

		default:
			return nullptr;
	}

	_list += settings;
	return settings;
}
// End MMGSettingsManager
