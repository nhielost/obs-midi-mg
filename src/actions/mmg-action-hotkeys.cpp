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

#include "mmg-action-hotkeys.h"

using namespace MMGUtils;

#define HOTKEY_REGISTERER_GETTER(kind)                                           \
  ptr_##kind = obs_weak_##kind##_get_##kind(                                     \
    reinterpret_cast<obs_weak_##kind##_t *>(obs_hotkey_get_registerer(hotkey))); \
  if (!ptr_##kind) return true;

#define HOTKEY_REGISTERER_GETTER_ELIGIBLE(kind)                        \
  HOTKEY_REGISTERER_GETTER(kind);                                      \
  if (_list->contains(obs_##kind##_get_name(ptr_##kind))) return true; \
  append = obs_##kind##_get_name(ptr_##kind);

#define HOTKEY_REGISTERER_GETTER_EXECUTE(kind) \
  HOTKEY_REGISTERER_GETTER(kind);              \
  if (obs_##kind##_get_name(ptr_##kind) != req->hotkey_group) return true;

#define HOTKEY_REGISTERER_GETTER_NORMAL(kind) \
  HOTKEY_REGISTERER_GETTER(kind);             \
  if (helper->str != obs_##kind##_get_name(ptr_##kind)) return true;

MMGActionHotkeys::MMGActionHotkeys(const QJsonObject &json_obj)
  : hotkey_group(json_obj, "hotkey_group", 0), hotkey(json_obj, "hotkey", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Hotkeys> action created.");
}

void MMGActionHotkeys::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Hotkeys> Action -> " + message);
}

void MMGActionHotkeys::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  hotkey_group.json(json_obj, "hotkey_group", false);
  hotkey.json(json_obj, "hotkey", false);
}

void MMGActionHotkeys::do_action(const MMGMessage *midi)
{
  Q_UNUSED(midi);
  struct HotkeyRequestBody {
    QString hotkey_name;
    QString hotkey_group;
    obs_hotkey_id hotkey_id = -1;
  } req;
  req.hotkey_group = hotkey_group.str();
  req.hotkey_name =
    enumerate_names(hotkey_group).value(enumerate_descriptions(hotkey_group).indexOf(hotkey.str()));

  obs_enum_hotkeys(
    [](void *data, obs_hotkey_id id, obs_hotkey_t *hotkey) {
      auto *req = reinterpret_cast<HotkeyRequestBody *>(data);

      OBSSourceAutoRelease ptr_source;
      OBSOutputAutoRelease ptr_output;
      OBSEncoderAutoRelease ptr_encoder;
      OBSServiceAutoRelease ptr_service;

      if (obs_hotkey_get_name(hotkey) != req->hotkey_name) return true;

      switch (obs_hotkey_get_registerer_type(hotkey)) {
	case OBS_HOTKEY_REGISTERER_FRONTEND:
	  break;
	case OBS_HOTKEY_REGISTERER_SOURCE:
	  HOTKEY_REGISTERER_GETTER_EXECUTE(source);
	  break;
	case OBS_HOTKEY_REGISTERER_OUTPUT:
	  HOTKEY_REGISTERER_GETTER_EXECUTE(output);
	  break;
	case OBS_HOTKEY_REGISTERER_ENCODER:
	  HOTKEY_REGISTERER_GETTER_EXECUTE(encoder);
	  break;
	case OBS_HOTKEY_REGISTERER_SERVICE:
	  HOTKEY_REGISTERER_GETTER_EXECUTE(service);
	  break;
	default:
	  return true;
      }
      req->hotkey_id = id;
      return false;
    },
    &req);

  if (req.hotkey_id == (size_t)(-1)) {
    blog(LOG_INFO, "FAILED: Hotkey does not exist.");
    return;
  }

  switch (get_sub()) {
    case MMGActionHotkeys::HOTKEY_PREDEF:
      obs_queue_task(
	OBS_TASK_UI,
	[](void *param) {
	  auto id = (size_t *)param;
	  obs_hotkey_trigger_routed_callback(*id, false);
	  obs_hotkey_trigger_routed_callback(*id, true);
	  obs_hotkey_trigger_routed_callback(*id, false);
	},
	&req.hotkey_id, true);
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionHotkeys::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  hotkey_group.copy(&dest->str1());
  hotkey_desc.copy(&dest->str2());
  hotkey.copy(&dest->str3());
}

const QStringList MMGActionHotkeys::enumerate_names(const QString &category)
{
  R r{QStringList(), category};
  obs_enum_hotkeys(
    [](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
      Q_UNUSED(id);
      auto helper = reinterpret_cast<R *>(param);

      OBSSourceAutoRelease ptr_source;
      OBSOutputAutoRelease ptr_output;
      OBSEncoderAutoRelease ptr_encoder;
      OBSServiceAutoRelease ptr_service;

      switch (obs_hotkey_get_registerer_type(hotkey)) {
	case OBS_HOTKEY_REGISTERER_FRONTEND:
	  if (helper->str != "Frontend") return true;
	  break;
	case OBS_HOTKEY_REGISTERER_SOURCE:
	  HOTKEY_REGISTERER_GETTER_NORMAL(source);
	  break;
	case OBS_HOTKEY_REGISTERER_OUTPUT:
	  HOTKEY_REGISTERER_GETTER_NORMAL(output);
	  break;
	case OBS_HOTKEY_REGISTERER_ENCODER:
	  HOTKEY_REGISTERER_GETTER_NORMAL(encoder);
	  break;
	case OBS_HOTKEY_REGISTERER_SERVICE:
	  HOTKEY_REGISTERER_GETTER_NORMAL(service);
	  break;
	default:
	  return true;
      }
      helper->list.append(obs_hotkey_get_name(hotkey));
      return true;
    },
    &r);
  return r.list;
}

const QStringList MMGActionHotkeys::enumerate_descriptions(const QString &category)
{
  R r{QStringList(), category};
  obs_enum_hotkeys(
    [](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
      Q_UNUSED(id);
      auto helper = reinterpret_cast<R *>(param);

      OBSSourceAutoRelease ptr_source;
      OBSOutputAutoRelease ptr_output;
      OBSEncoderAutoRelease ptr_encoder;
      OBSServiceAutoRelease ptr_service;

      switch (obs_hotkey_get_registerer_type(hotkey)) {
	case OBS_HOTKEY_REGISTERER_FRONTEND:
	  if (helper->str != "Frontend") return true;
	  break;
	case OBS_HOTKEY_REGISTERER_SOURCE:
	  HOTKEY_REGISTERER_GETTER_NORMAL(source);
	  break;
	case OBS_HOTKEY_REGISTERER_OUTPUT:
	  HOTKEY_REGISTERER_GETTER_NORMAL(output);
	  break;
	case OBS_HOTKEY_REGISTERER_ENCODER:
	  HOTKEY_REGISTERER_GETTER_NORMAL(encoder);
	  break;
	case OBS_HOTKEY_REGISTERER_SERVICE:
	  HOTKEY_REGISTERER_GETTER_NORMAL(service);
	  break;
	default:
	  return true;
      }
      helper->list.append(obs_hotkey_get_description(hotkey));
      return true;
    },
    &r);
  return r.list;
}

const QStringList MMGActionHotkeys::enumerate_eligible()
{
  QStringList list;

  obs_enum_hotkeys(
    [](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
      Q_UNUSED(id);
      auto _list = reinterpret_cast<QStringList *>(param);

      OBSSourceAutoRelease ptr_source;
      OBSOutputAutoRelease ptr_output;
      OBSEncoderAutoRelease ptr_encoder;
      OBSServiceAutoRelease ptr_service;

      QString append;

      switch (obs_hotkey_get_registerer_type(hotkey)) {
	case OBS_HOTKEY_REGISTERER_FRONTEND:
	  if (_list->contains("Frontend")) return true;
	  append = "Frontend";
	  break;
	case OBS_HOTKEY_REGISTERER_SOURCE:
	  HOTKEY_REGISTERER_GETTER_ELIGIBLE(source);
	  break;
	case OBS_HOTKEY_REGISTERER_OUTPUT:
	  HOTKEY_REGISTERER_GETTER_ELIGIBLE(output);
	  break;
	case OBS_HOTKEY_REGISTERER_ENCODER:
	  HOTKEY_REGISTERER_GETTER_ELIGIBLE(encoder);
	  break;
	case OBS_HOTKEY_REGISTERER_SERVICE:
	  HOTKEY_REGISTERER_GETTER_ELIGIBLE(service);
	  break;
	default:
	  return true;
      }
      _list->append(append);
      return true;
    },
    &list);
  return list;
}

void MMGActionHotkeys::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Activate Predefined Hotkey"};
}
void MMGActionHotkeys::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Group";
  val.list = enumerate_eligible();
}
void MMGActionHotkeys::change_options_str2(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR2;
  val.label_text = "Hotkey";
  val.list = enumerate_descriptions(hotkey_group);
}
void MMGActionHotkeys::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionHotkeys::change_options_final(MMGActionDisplayParams &val) {}
