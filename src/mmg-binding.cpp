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

#include "mmg-binding.h"
#include "mmg-action-include.h"

qulonglong MMGBinding::next_default = 1;

MMGBinding::MMGBinding()
{
  name = get_next_default_name();
  enabled = true;
  message = new MMGMessage;
  action = new MMGActionNone;
  blog(LOG_DEBUG, "Empty binding created.");
}

MMGBinding::MMGBinding(const QJsonObject &obj) : name(obj["name"].toString())
{
  if (name.isEmpty()) name = get_next_default_name();
  enabled = obj["enabled"].toBool(true);
  message = new MMGMessage(obj["message"].toObject());
  set_action_type(obj["action"].toObject());
  blog(LOG_DEBUG, "Binding created.");
}

void MMGBinding::json(QJsonObject &binding_obj) const
{
  binding_obj["name"] = name;
  binding_obj["enabled"] = enabled;
  QJsonObject msg;
  message->json(msg);
  binding_obj["message"] = msg;
  QJsonObject act;
  action->json(act);
  binding_obj["action"] = act;
}

void MMGBinding::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "Binding {" + name + "} -> " + message);
}

QString MMGBinding::get_next_default_name()
{
  return QVariant(++MMGBinding::next_default).toString().prepend("Untitled Binding ");
}

void MMGBinding::do_action(const MMGSharedMessage &incoming)
{
  if (!enabled) return;

  if (!message->is_acceptable(incoming.get())) {
    blog(LOG_DEBUG, "Message received is not acceptable.");
    return;
  }

  action->do_action(incoming.get());

  message->toggle();
}

void MMGBinding::deep_copy(MMGBinding *dest)
{
  if (!name.contains("Untitled Binding")) dest->set_name(name);
  message->deep_copy(dest->get_message());
  dest->set_action_type((int)action->get_category());
  action->deep_copy(dest->get_action());
}

void MMGBinding::set_action_type(int index)
{
  delete action;
  switch ((MMGAction::Category)index) {
    case MMGAction::Category::MMGACTION_STREAM:
      action = new MMGActionStream();
      break;
    case MMGAction::Category::MMGACTION_RECORD:
      action = new MMGActionRecord();
      break;
    case MMGAction::Category::MMGACTION_VIRCAM:
      action = new MMGActionVirtualCam();
      break;
    case MMGAction::Category::MMGACTION_REPBUF:
      action = new MMGActionReplayBuffer();
      break;
    case MMGAction::Category::MMGACTION_STUDIOMODE:
      action = new MMGActionStudioMode();
      break;
    case MMGAction::Category::MMGACTION_SCENE:
      action = new MMGActionScenes();
      break;
    case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
      action = new MMGActionVideoSources();
      break;
    case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
      action = new MMGActionAudioSources();
      break;
    case MMGAction::Category::MMGACTION_SOURCE_MEDIA:
      action = new MMGActionMediaSources();
      break;
    case MMGAction::Category::MMGACTION_TRANSITION:
      action = new MMGActionTransitions();
      break;
    case MMGAction::Category::MMGACTION_FILTER:
      action = new MMGActionFilters();
      break;
    case MMGAction::Category::MMGACTION_HOTKEY:
      action = new MMGActionHotkeys();
      break;
    case MMGAction::Category::MMGACTION_PROFILE:
      action = new MMGActionProfiles();
      break;
    case MMGAction::Category::MMGACTION_COLLECTION:
      action = new MMGActionCollections();
      break;
    case MMGAction::Category::MMGACTION_MIDI:
      action = new MMGActionMIDI();
      break;
    case MMGAction::Category::MMGACTION_INTERNAL:
      action = new MMGActionInternal();
      break;
    case MMGAction::Category::MMGACTION_TIMEOUT:
      action = new MMGActionTimeout();
      break;
    default:
      action = new MMGActionNone();
      break;
  }
}

void MMGBinding::set_action_type(const QJsonObject &json_obj)
{
  switch ((MMGAction::Category)json_obj["category"].toInt()) {
    case MMGAction::Category::MMGACTION_STREAM:
      action = new MMGActionStream(json_obj);
      break;
    case MMGAction::Category::MMGACTION_RECORD:
      action = new MMGActionRecord(json_obj);
      break;
    case MMGAction::Category::MMGACTION_VIRCAM:
      action = new MMGActionVirtualCam(json_obj);
      break;
    case MMGAction::Category::MMGACTION_REPBUF:
      action = new MMGActionReplayBuffer(json_obj);
      break;
    case MMGAction::Category::MMGACTION_STUDIOMODE:
      action = new MMGActionStudioMode(json_obj);
      break;
    case MMGAction::Category::MMGACTION_SCENE:
      action = new MMGActionScenes(json_obj);
      break;
    case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
      action = new MMGActionVideoSources(json_obj);
      break;
    case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
      action = new MMGActionAudioSources(json_obj);
      break;
    case MMGAction::Category::MMGACTION_SOURCE_MEDIA:
      action = new MMGActionMediaSources(json_obj);
      break;
    case MMGAction::Category::MMGACTION_TRANSITION:
      action = new MMGActionTransitions(json_obj);
      break;
    case MMGAction::Category::MMGACTION_FILTER:
      action = new MMGActionFilters(json_obj);
      break;
    case MMGAction::Category::MMGACTION_HOTKEY:
      action = new MMGActionHotkeys(json_obj);
      break;
    case MMGAction::Category::MMGACTION_PROFILE:
      action = new MMGActionProfiles(json_obj);
      break;
    case MMGAction::Category::MMGACTION_COLLECTION:
      action = new MMGActionCollections(json_obj);
      break;
    case MMGAction::Category::MMGACTION_MIDI:
      action = new MMGActionMIDI(json_obj);
      break;
    case MMGAction::Category::MMGACTION_INTERNAL:
      action = new MMGActionInternal(json_obj);
      break;
    case MMGAction::Category::MMGACTION_TIMEOUT:
      action = new MMGActionTimeout(json_obj);
      break;
    default:
      action = new MMGActionNone();
      break;
  }
}
