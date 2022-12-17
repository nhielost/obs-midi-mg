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
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  collection.json(json_obj, "collection", true);
}

void MMGActionCollections::do_action(const MMGMessage *midi)
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

  if (get_sub() == 0) {
    set_collection((collection.state() == MMGString::STRINGSTATE_MIDI
		      ? collections[(int)midi->value()]
		      : collection)
		     .qtocs());
  }

  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionCollections::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  collection.copy(&dest->str1());
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

void MMGActionCollections::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Switch Scene Collections"};
}
void MMGActionCollections::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Scene Collection";
  val.list = enumerate();
  val.list.append("Use Message Value");
}
void MMGActionCollections::change_options_str2(MMGActionDisplayParams &val)
{
  collection.set_state(collection == "Use Message Value" ? MMGString::STRINGSTATE_MIDI
							 : MMGString::STRINGSTATE_FIXED);
}
void MMGActionCollections::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionCollections::change_options_final(MMGActionDisplayParams &val) {}
