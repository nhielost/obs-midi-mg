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

#include "mmg-action-collections.h"

using namespace MMGUtils;

MMGActionCollections::MMGActionCollections(const QJsonObject &json_obj)
  : collection(json_obj, "collection", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Collections> action created.");
}

void MMGActionCollections::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Collections> Action -> " + message);
}

void MMGActionCollections::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  collection.json(json_obj, "collection");
}

void MMGActionCollections::execute(const MMGMessage *midi) const
{
  const QStringList collections = MMGActionCollections::enumerate();

  if (MIDI_STRING_IS_NOT_IN_RANGE(collection, collections.size())) {
    blog(LOG_INFO, "FAILED: MIDI value exceeds scene collection count.");
    return;
  }

  // For new Scene Collection change (pre 28.0.0 method encounters threading errors)
  auto set_collection = [](const char *name) {
    char *current_collection = obs_frontend_get_current_scene_collection();
    if (name == current_collection) {
      bfree(current_collection);
      return;
    }
    bfree(current_collection);
    obs_queue_task(
      OBS_TASK_UI,
      [](void *param) {
	auto collection_name = (char **)param;
	obs_frontend_set_current_scene_collection(*collection_name);
      },
      &name, true);
  };

  if (sub() == 0) {
    set_collection((collection.state() == MMGString::STRINGSTATE_MIDI
		      ? collections[(int)midi->value()]
		      : collection)
		     .qtocs());
  }

  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionCollections::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionCollections *>(dest);
  if (!casted) return;

  casted->collection = collection.copy();
}

void MMGActionCollections::setEditable(bool edit)
{
  collection.set_edit(edit);
}

const QStringList MMGActionCollections::enumerate()
{
  QStringList list;

  char **collection_names = obs_frontend_get_scene_collections();
  for (int i = 0; collection_names[i] != 0; ++i) {
    list.append(collection_names[i]);
  }
  bfree(collection_names);

  return list;
}

void MMGActionCollections::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&collection);
}

void MMGActionCollections::setSubOptions(QComboBox *sub)
{
  sub->addItem("Switch Scene Collections");
}

void MMGActionCollections::setSubConfig()
{
  _display->setStr1Visible(true);
  _display->setStr1Description("Scene Collection");
  QStringList options = enumerate();
  options.append("Use Message Value");
  _display->setStr1Options(options);
}
