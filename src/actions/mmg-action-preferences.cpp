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

#include "mmg-action-preferences.h"
#include "../mmg-config.h"

using namespace MMGUtils;

MMGActionPreferences::MMGActionPreferences()
{
  blog(LOG_DEBUG, "Empty action created.");
};

MMGActionPreferences::MMGActionPreferences(const QJsonObject &json_obj)
  : option(json_obj, "option", 0)
{
  subcategory = 0;

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionPreferences::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Preferences] " + message);
}

void MMGActionPreferences::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  option.json(json_obj, "option");
}

void MMGActionPreferences::execute(const MMGMessage *midi) const
{
  Q_UNUSED(midi);

  switch (sub()) {
    case PREFERENCE_ACTIVITY:
      global()->preferences()->setActive(false);
      break;

    case PREFERENCE_THROUGHPUT:
      global()->preferences()->setThruDevice(option);
      break;

    case PREFERENCE_INTERNALBEHAVIOR:
      global()->preferences()->setInternalBehavior(
	(short)internalBehaviorOptions().indexOf(option));
      break;

    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionPreferences::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionPreferences *>(dest);
  if (!casted) return;

  casted->option = option.copy();
}

void MMGActionPreferences::setEditable(bool edit)
{
  option.set_edit(edit);
}

const QStringList MMGActionPreferences::throughputOptions()
{
  QStringList opts = {mmgtr("Plugin.Disabled")};
  return opts + output_device()->outputDeviceNames();
}

const QStringList MMGActionPreferences::internalBehaviorOptions()
{
  return mmgtr_all("UI.Preferences.InternalBehavior.Label", {"Reset", "NoReset"});
}

void MMGActionPreferences::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&option);
  _display->setStr1Description(mmgtr("Actions.Preferences.Option"));
}

void MMGActionPreferences::setSubOptions(QComboBox *sub)
{
  sub->addItems(mmgtr_all("UI.Preferences.Labels", {"Active", "Throughput", "InternalBehavior"}));
}

void MMGActionPreferences::setSubConfig()
{
  _display->setStr1Visible(true);

  switch (sub()) {
    case PREFERENCE_ACTIVITY:
      _display->setStr1Visible(false);
      break;
    case PREFERENCE_THROUGHPUT:
      _display->setStr1Options(throughputOptions());
      break;
    case PREFERENCE_INTERNALBEHAVIOR:
      _display->setStr1Options(internalBehaviorOptions());
      break;
    default:
      break;
  }
}
