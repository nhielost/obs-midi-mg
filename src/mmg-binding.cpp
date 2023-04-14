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

#include "mmg-binding.h"
#include "mmg-config.h"

#include "actions/mmg-action.h"
#include "actions/mmg-action-none.h"
#include "actions/mmg-action-stream.h"
#include "actions/mmg-action-record.h"
#include "actions/mmg-action-virtualcam.h"
#include "actions/mmg-action-replaybuffer.h"
#include "actions/mmg-action-studiomode.h"
#include "actions/mmg-action-scenes.h"
#include "actions/mmg-action-video-sources.h"
#include "actions/mmg-action-audio-sources.h"
#include "actions/mmg-action-media-sources.h"
#include "actions/mmg-action-transitions.h"
#include "actions/mmg-action-filters.h"
#include "actions/mmg-action-hotkeys.h"
#include "actions/mmg-action-profiles.h"
#include "actions/mmg-action-collections.h"
#include "actions/mmg-action-midi.h"
#include "actions/mmg-action-internal.h"
#include "actions/mmg-action-preferences.h"

qulonglong MMGBinding::next_default = 0;

MMGBinding::MMGBinding()
{
  _name = get_next_default_name();
  setEnabled(true);
  _message = new MMGMessage;
  _action = new MMGActionNone;
  blog(LOG_DEBUG, "Empty binding created.");
}

MMGBinding::MMGBinding(const QJsonObject &obj) : _name(obj["name"].toString())
{
  if (_name.isEmpty()) _name = get_next_default_name();
  setEnabled(obj["enabled"].toBool(true));
  _message = new MMGMessage(obj["message"].toObject());
  setCategory(obj["action"].toObject());
  blog(LOG_DEBUG, "Binding created.");
}

void MMGBinding::json(QJsonObject &binding_obj) const
{
  binding_obj["name"] = _name;
  binding_obj["enabled"] = _enabled;
  QJsonObject msg;
  _message->json(msg);
  binding_obj["message"] = msg;
  QJsonObject act;
  _action->json(act);
  binding_obj["action"] = act;
}

void MMGBinding::blog(int log_status, const QString &message) const
{
  global_blog(log_status, QString::asprintf("[Bindings] <%s> ", _name.qtocs()) + message);
}

QString MMGBinding::get_next_default_name()
{
  return QVariant(++MMGBinding::next_default).toString().prepend(mmgtr("Binding.Untitled"));
}

void MMGBinding::execute(const MMGSharedMessage &incoming)
{
  if (!_message->acceptable(incoming.get())) {
    blog(LOG_DEBUG, "Message received is not acceptable.");
  } else {
    _action->execute(incoming.get());
    _message->toggle();
  }
}

void MMGBinding::copy(MMGBinding *dest)
{
  if (!_name.startsWith(mmgtr("Binding.Untitled"))) dest->setName(_name);
  _message->copy(dest->message());
  dest->setCategory((int)_action->category());
  _action->copy(dest->action());
}

void MMGBinding::setEnabled(bool val)
{
  _enabled = val;
  setConnected(val);
}

void MMGBinding::setCategory(int index)
{
  delete _action;
  switch ((MMGAction::Category)index) {
    case MMGAction::MMGACTION_STREAM:
      _action = new MMGActionStream();
      break;
    case MMGAction::MMGACTION_RECORD:
      _action = new MMGActionRecord();
      break;
    case MMGAction::MMGACTION_VIRCAM:
      _action = new MMGActionVirtualCam();
      break;
    case MMGAction::MMGACTION_REPBUF:
      _action = new MMGActionReplayBuffer();
      break;
    case MMGAction::MMGACTION_STUDIOMODE:
      _action = new MMGActionStudioMode();
      break;
    case MMGAction::MMGACTION_SCENE:
      _action = new MMGActionScenes();
      break;
    case MMGAction::MMGACTION_SOURCE_VIDEO:
      _action = new MMGActionVideoSources();
      break;
    case MMGAction::MMGACTION_SOURCE_AUDIO:
      _action = new MMGActionAudioSources();
      break;
    case MMGAction::MMGACTION_SOURCE_MEDIA:
      _action = new MMGActionMediaSources();
      break;
    case MMGAction::MMGACTION_TRANSITION:
      _action = new MMGActionTransitions();
      break;
    case MMGAction::MMGACTION_FILTER:
      _action = new MMGActionFilters();
      break;
    case MMGAction::MMGACTION_HOTKEY:
      _action = new MMGActionHotkeys();
      break;
    case MMGAction::MMGACTION_PROFILE:
      _action = new MMGActionProfiles();
      break;
    case MMGAction::MMGACTION_COLLECTION:
      _action = new MMGActionCollections();
      break;
    case MMGAction::MMGACTION_MIDI:
      _action = new MMGActionMIDI();
      break;
    case MMGAction::MMGACTION_INTERNAL:
      _action = new MMGActionInternal();
      break;
    case MMGAction::MMGACTION_PREFERENCE:
      _action = new MMGActionPreferences();
      break;
    default:
      _action = new MMGActionNone();
      break;
  }
}

void MMGBinding::setCategory(const QJsonObject &json_obj)
{
  switch ((MMGAction::Category)json_obj["category"].toInt()) {
    case MMGAction::MMGACTION_STREAM:
      _action = new MMGActionStream(json_obj);
      break;
    case MMGAction::MMGACTION_RECORD:
      _action = new MMGActionRecord(json_obj);
      break;
    case MMGAction::MMGACTION_VIRCAM:
      _action = new MMGActionVirtualCam(json_obj);
      break;
    case MMGAction::MMGACTION_REPBUF:
      _action = new MMGActionReplayBuffer(json_obj);
      break;
    case MMGAction::MMGACTION_STUDIOMODE:
      _action = new MMGActionStudioMode(json_obj);
      break;
    case MMGAction::MMGACTION_SCENE:
      _action = new MMGActionScenes(json_obj);
      break;
    case MMGAction::MMGACTION_SOURCE_VIDEO:
      _action = new MMGActionVideoSources(json_obj);
      break;
    case MMGAction::MMGACTION_SOURCE_AUDIO:
      _action = new MMGActionAudioSources(json_obj);
      break;
    case MMGAction::MMGACTION_SOURCE_MEDIA:
      _action = new MMGActionMediaSources(json_obj);
      break;
    case MMGAction::MMGACTION_TRANSITION:
      _action = new MMGActionTransitions(json_obj);
      break;
    case MMGAction::MMGACTION_FILTER:
      _action = new MMGActionFilters(json_obj);
      break;
    case MMGAction::MMGACTION_HOTKEY:
      _action = new MMGActionHotkeys(json_obj);
      break;
    case MMGAction::MMGACTION_PROFILE:
      _action = new MMGActionProfiles(json_obj);
      break;
    case MMGAction::MMGACTION_COLLECTION:
      _action = new MMGActionCollections(json_obj);
      break;
    case MMGAction::MMGACTION_MIDI:
      _action = new MMGActionMIDI(json_obj);
      break;
    case MMGAction::MMGACTION_INTERNAL:
      _action = new MMGActionInternal(json_obj);
      break;
    case MMGAction::MMGACTION_PREFERENCE:
      _action = new MMGActionPreferences(json_obj);
      break;
    default:
      _action = new MMGActionNone();
      break;
  }
}

void MMGBinding::setConnected(bool connected)
{
  if (connected) {
    if (_enabled)
      connect(input_device().get(), &MMGMIDIIn::messageReceived, this, &MMGBinding::execute);
  } else {
    disconnect(input_device().get(), &MMGMIDIIn::messageReceived, this, &MMGBinding::execute);
  }
}
