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

#include "mmg-action-audio-sources.h"

using namespace MMGUtils;

MMGActionAudioSources::MMGActionAudioSources(const QJsonObject &json_obj)
  : source(json_obj, "source", 1), action(json_obj, "action", 2), num(json_obj, "num", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Audio Sources> action created.");
}

void MMGActionAudioSources::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Audio Sources> Action -> " + message);
}

void MMGActionAudioSources::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  source.json(json_obj, "source", false);
  action.json(json_obj, "action", true);
  num.json(json_obj, "num", true);
}

void MMGActionAudioSources::do_action(const MMGMessage *midi)
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

  switch (get_sub()) {
    case MMGActionAudioSources::SOURCE_AUDIO_VOLUME_CHANGETO:
      // Value only has range 0-127, meaning full volume cannot be achieved (as it is divided by 128).
      // Adding one allows the capability of full volume.
      // ADDITION: Removing 1% from the value and changing it to 0% for muting is not as noticable.
      if (num.state() == MMGNumber::NUMBERSTATE_MIDI && midi->value() == 0) {
	obs_source_set_volume(obs_source, 0);
      } else {
	obs_source_set_volume(obs_source,
			      std::pow((num.choose(midi, 0, 100.0, true) / 100.0), 3.0));
      }
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
      // Multiplier is 3200 here to make it so that the sync offset is incremented by 25.
      // Hard limit is set at 3175 ms
      obs_source_set_sync_offset(obs_source, (num.choose(midi, 0, 3200.0) * 1000000));
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_MONITOR:
      if (MIDI_NUMBER_IS_NOT_IN_RANGE(num, 3)) {
	blog(LOG_INFO, "FAILED: MIDI value exceeds audio monitor option count.");
	return;
      }
      obs_source_set_monitoring_type(obs_source, (obs_monitoring_type)(num.choose(midi)));
      break;
    case MMGActionAudioSources::SOURCE_AUDIO_CUSTOM:
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionAudioSources::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  source.copy(&dest->str1());
  action.copy(&dest->str2());
  num.copy(&dest->num1());
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

void MMGActionAudioSources::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Change Source Volume To", "Change Source Volume By", "Mute Source",
	      "Unmute Source",           "Toggle Source Mute",      "Source Audio Offset",
	      "Source Audio Monitor"};
}
void MMGActionAudioSources::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Audio Source";
  val.list = enumerate();
}
void MMGActionAudioSources::change_options_str2(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case SOURCE_AUDIO_VOLUME_CHANGETO:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Volume (%)";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI |
			     MMGActionDisplayParams::COMBODISPLAY_MIDI_INVERT;
      val.lcds[0]->set_range(0.0, 100.0);
      val.lcds[0]->set_step(0.5, 5.0);
      val.lcds[0]->set_default_value(0.0);

      break;
    case SOURCE_AUDIO_VOLUME_CHANGEBY:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Volume (%)";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_FIXED;
      val.lcds[0]->set_range(-50.0, 50.0);
      val.lcds[0]->set_step(0.5, 5.0);
      val.lcds[0]->set_default_value(0.0);

      break;
    case SOURCE_AUDIO_VOLUME_MUTE_ON:
    case SOURCE_AUDIO_VOLUME_MUTE_OFF:
    case SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF:
      break;
    case SOURCE_AUDIO_OFFSET:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Offset";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI;
      val.lcds[0]->set_range(0.0, 20000.0);
      val.lcds[0]->set_step(25.0, 250.0);
      val.lcds[0]->set_default_value(0.0);

      break;
    case SOURCE_AUDIO_MONITOR:
      val.display = MMGActionDisplayParams::DISPLAY_STR2;

      val.label_text = "Audio Monitor";
      val.list = {"Off", "Monitor Only", "Monitor & Output", "Use Message Value"};

      break;
    case SOURCE_AUDIO_CUSTOM:
    default:
      break;
  }
}
void MMGActionAudioSources::change_options_str3(MMGActionDisplayParams &val)
{
  if (subcategory == 6) {
    num = val.list.indexOf(action);
    num.set_state(action == "Use Message Value" ? MMGNumber::NUMBERSTATE_MIDI
						: MMGNumber::NUMBERSTATE_FIXED);
  }
}
void MMGActionAudioSources::change_options_final(MMGActionDisplayParams &val) {}
