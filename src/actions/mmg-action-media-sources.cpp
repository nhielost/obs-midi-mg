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

#include "mmg-action-media-sources.h"

using namespace MMGUtils;

MMGActionMediaSources::MMGActionMediaSources(const QJsonObject &json_obj)
  : source(json_obj, "source", 1), num(json_obj, "num", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "<Media Sources> action created.");
}

void MMGActionMediaSources::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "<Media Sources> Action -> " + message);
}

void MMGActionMediaSources::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)get_category();
  json_obj["sub"] = (int)get_sub();
  source.json(json_obj, "source", false);
  num.json(json_obj, "num", true);
}

void MMGActionMediaSources::do_action(const MMGMessage *midi)
{
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  if (!obs_source) {
    blog(LOG_INFO, "FAILED: Source does not exist.");
    return;
  }
  if (!(obs_source_get_output_flags(obs_source) & OBS_SOURCE_CONTROLLABLE_MEDIA)) {
    blog(LOG_INFO, "FAILED: Source is not a media source.");
    return;
  }
  obs_media_state state = obs_source_media_get_state(obs_source);

  switch (get_sub()) {
    case MMGActionMediaSources::SOURCE_MEDIA_TOGGLE_PLAYPAUSE:
      switch (state) {
	case OBS_MEDIA_STATE_PLAYING:
	  obs_source_media_play_pause(obs_source, true);
	  break;
	case OBS_MEDIA_STATE_PAUSED:
	  obs_source_media_play_pause(obs_source, false);
	  break;
	case OBS_MEDIA_STATE_STOPPED:
	case OBS_MEDIA_STATE_ENDED:
	  obs_source_media_restart(obs_source);
	  break;
	default:
	  break;
      }
      break;
    case MMGActionMediaSources::SOURCE_MEDIA_RESTART:
      obs_source_media_restart(obs_source);
      break;
    case MMGActionMediaSources::SOURCE_MEDIA_STOP:
      obs_source_media_stop(obs_source);
      break;
    case MMGActionMediaSources::SOURCE_MEDIA_TIME:
      obs_source_media_set_time(obs_source,
				num.choose(midi, 0, get_obs_media_length(source)) * 1000);
      break;
    case MMGActionMediaSources::SOURCE_MEDIA_SKIP_FORWARD_TRACK:
      obs_source_media_next(obs_source);
      break;
    case MMGActionMediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TRACK:
      obs_source_media_previous(obs_source);
      break;
    case MMGActionMediaSources::SOURCE_MEDIA_SKIP_FORWARD_TIME:
      obs_source_media_set_time(obs_source, obs_source_media_get_time(obs_source) + (num * 1000));
      break;
    case MMGActionMediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TIME:
      obs_source_media_set_time(obs_source, obs_source_media_get_time(obs_source) - (num * 1000));
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
  executed = true;
}

void MMGActionMediaSources::deep_copy(MMGAction *dest) const
{
  dest->set_sub(subcategory);
  source.copy(&dest->str1());
  num.copy(&dest->num1());
}

const QStringList MMGActionMediaSources::enumerate()
{
  QStringList list;
  obs_enum_sources(
    [](void *param, obs_source_t *source) {
      auto _list = reinterpret_cast<QStringList *>(param);
      if (!(obs_source_get_output_flags(source) & OBS_SOURCE_CONTROLLABLE_MEDIA)) return true;

      _list->append(obs_source_get_name(source));
      return true;
    },
    &list);
  return list;
}

void MMGActionMediaSources::change_options_sub(MMGActionDisplayParams &val)
{
  val.list = {"Play or Pause",     "Restart",           "Stop",
	      "Set Track Time",    "Next Track",        "Previous Track",
	      "Skip Forward Time", "Skip Backward Time"};
}
void MMGActionMediaSources::change_options_str1(MMGActionDisplayParams &val)
{
  val.display = MMGActionDisplayParams::DISPLAY_STR1;
  val.label_text = "Media Source";
  val.list = enumerate();
}
void MMGActionMediaSources::change_options_str2(MMGActionDisplayParams &val)
{
  switch ((Actions)subcategory) {
    case SOURCE_MEDIA_TOGGLE_PLAYPAUSE:
    case SOURCE_MEDIA_RESTART:
    case SOURCE_MEDIA_STOP:
    case SOURCE_MEDIA_SKIP_FORWARD_TRACK:
    case SOURCE_MEDIA_SKIP_BACKWARD_TRACK:
      break;
    case SOURCE_MEDIA_TIME:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Time";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_MIDI |
			     MMGActionDisplayParams::COMBODISPLAY_MIDI_INVERT;
      val.lcds[0]->set_use_time(true);
      val.lcds[0]->set_range(0.0, get_obs_media_length(source));
      val.lcds[0]->set_step(1.0, 10.0);
      val.lcds[0]->set_default_value(0.0);

      break;
    case SOURCE_MEDIA_SKIP_FORWARD_TIME:
    case SOURCE_MEDIA_SKIP_BACKWARD_TIME:
      val.display |= MMGActionDisplayParams::DISPLAY_NUM1;

      val.label_lcds[0] = "Time Adjust";
      val.combo_display[0] = MMGActionDisplayParams::COMBODISPLAY_FIXED;
      val.lcds[0]->set_use_time(true);
      val.lcds[0]->set_range(0.0, get_obs_media_length(source));
      val.lcds[0]->set_step(1.0, 10.0);
      val.lcds[0]->set_default_value(0.0);

      break;
    default:
      break;
  }
}
void MMGActionMediaSources::change_options_str3(MMGActionDisplayParams &val) {}
void MMGActionMediaSources::change_options_final(MMGActionDisplayParams &val) {}
