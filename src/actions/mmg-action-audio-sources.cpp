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

#include "mmg-action-audio-sources.h"

using namespace MMGUtils;

MMGActionAudioSources::MMGActionAudioSources(const QJsonObject &json_obj)
  : source(json_obj, "source", 1), action(json_obj, "action", 2), num(json_obj, "num", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionAudioSources::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Audio Sources] " + message);
}

void MMGActionAudioSources::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  source.json(json_obj, "source", false);
  action.json(json_obj, "action");
  num.json(json_obj, "num");
}

void MMGActionAudioSources::execute(const MMGMessage *midi) const
{
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  if (!obs_source) {
    blog(LOG_INFO, "FAILED: Source does not exist.");
    return;
  }
  if (!(obs_source_get_output_flags(obs_source) & OBS_SOURCE_AUDIO)) {
    blog(LOG_INFO, "FAILED: Source is not an audio source.");
    return;
  }

  switch (sub()) {
    case MMGActionAudioSources::SOURCE_AUDIO_VOLUME_CHANGETO:
      // Now divided by 127, no need for adding one
      obs_source_set_volume(obs_source, std::pow((num.choose(midi) / 100.0), 3.0));
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_VOLUME_CHANGEBY:
      if (std::cbrt(obs_source_get_volume(obs_source)) * 100.0 + num >= 100.0) {
	obs_source_set_volume(obs_source, 1);
      } else if (std::cbrt(obs_source_get_volume(obs_source)) * 100.0 + num <= 0.0) {
	obs_source_set_volume(obs_source, 0);
      } else {
	obs_source_set_volume(
	  obs_source, std::pow(std::cbrt(obs_source_get_volume(obs_source)) + (num / 100.0), 3.0));
      }
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_VOLUME_MUTE_ON:
      obs_source_set_muted(obs_source, true);
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_VOLUME_MUTE_OFF:
      obs_source_set_muted(obs_source, false);
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF:
      obs_source_set_muted(obs_source, !obs_source_muted(obs_source));
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_OFFSET:
      obs_source_set_sync_offset(obs_source, (num.choose(midi) * 1000000));
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_MONITOR:
      if (MIDI_STRING_IS_NOT_IN_RANGE(action, 3)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeds audio monitor option count.");
	return;
      }
      obs_source_set_monitoring_type(
	obs_source, (obs_monitoring_type)(action.state() ? midi->value()
							 : audioMonitorOptions().indexOf(action)));
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionAudioSources::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionAudioSources *>(dest);
  if (!casted) return;

  casted->source = source.copy();
  casted->action = action.copy();
  casted->num = num.copy();
}

void MMGActionAudioSources::setEditable(bool edit)
{
  source.set_edit(edit);
  action.set_edit(edit);
  num.set_edit(edit);
}

const QStringList MMGActionAudioSources::enumerate()
{
  QStringList list;
  obs_enum_all_sources(
    [](void *param, obs_source_t *source) {
      auto _list = reinterpret_cast<QStringList *>(param);

      if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT) return true;
      if (!(obs_source_get_output_flags(source) & OBS_SOURCE_AUDIO)) return true;

      _list->append(obs_source_get_name(source));
      return true;
    },
    &list);
  return list;
}

const QStringList MMGActionAudioSources::audioMonitorOptions()
{
  return obstr_all("Basic.AdvAudio.Monitoring", {"None", "MonitorOnly", "Both"}, true);
}

void MMGActionAudioSources::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&source);
  _display->setStr2Storage(&action);

  MMGNumberDisplay *num_display = new MMGNumberDisplay(_display->numberDisplays());
  num_display->setStorage(&num, true);
  _display->numberDisplays()->add(num_display);

  _display->connect(_display, &MMGActionDisplay::str1Changed, [&]() { setList1Config(); });
  _display->connect(_display, &MMGActionDisplay::str2Changed, [&]() { setList2Config(); });
}

void MMGActionAudioSources::setSubOptions(QComboBox *sub)
{
  sub->addItems(
    mmgtr_all("Actions.AudioSources.Sub", {"ChangeVolumeTo", "ChangeVolumeBy", "Mute", "Unmute",
					   "ToggleMute", "AudioOffset", "AudioMonitor"}));
}

void MMGActionAudioSources::setSubConfig()
{
  _display->setStr1Visible(false);
  _display->setStr2Visible(false);
  _display->setStr3Visible(false);

  _display->setStr1Visible(true);
  _display->setStr1Description(mmgtr("Actions.AudioSources.AudioSource"));
  _display->setStr1Options(enumerate());
}

void MMGActionAudioSources::setList1Config()
{
  MMGNumberDisplay *num_display = _display->numberDisplays()->fieldAt(0);
  num_display->setVisible(false);

  switch ((Actions)subcategory) {
    case SOURCE_AUDIO_VOLUME_CHANGETO:
      num_display->setVisible(!source.str().isEmpty());
      num_display->setDescription(mmgtr("Actions.AudioSources.Volume"));
      num_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_CUSTOM);
      num_display->setBounds(0.0, 100.0);
      num_display->setStep(0.5);
      num_display->setDefaultValue(0.0);
      break;

    case SOURCE_AUDIO_VOLUME_CHANGEBY:
      num_display->setVisible(!source.str().isEmpty());
      num_display->setDescription(mmgtr("Actions.AudioSources.Volume"));
      num_display->setOptions(MMGNumberDisplay::OPTIONS_FIXED_ONLY);
      num_display->setBounds(-50.0, 50.0);
      num_display->setStep(0.5);
      num_display->setDefaultValue(0.0);
      break;

    case SOURCE_AUDIO_VOLUME_MUTE_ON:
    case SOURCE_AUDIO_VOLUME_MUTE_OFF:
    case SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF:
      return;

    case SOURCE_AUDIO_OFFSET:
      num_display->setVisible(!source.str().isEmpty());
      num_display->setDescription(obstr("Basic.AdvAudio.SyncOffset"));
      num_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_CUSTOM);
      num_display->setBounds(0.0, 20000.0);
      num_display->setStep(25.0);
      num_display->setDefaultValue(0.0);
      break;

    case SOURCE_AUDIO_MONITOR:
      _display->setStr2Visible(true);
      _display->setStr2Description(obstr("Basic.AdvAudio.Monitoring"));
      _display->setStr2Options(audioMonitorOptions());
      return;

    default:
      return;
  }
  num_display->reset();
}

void MMGActionAudioSources::setList2Config()
{
  if (subcategory == SOURCE_AUDIO_MONITOR) {
    num = audioMonitorOptions().indexOf(action);
    num.set_state(action.state());
  }
}
