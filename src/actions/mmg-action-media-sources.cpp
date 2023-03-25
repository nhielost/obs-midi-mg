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
  MMGAction::json(json_obj);

  source.json(json_obj, "source", false);
  num.json(json_obj, "num");
}

void MMGActionMediaSources::execute(const MMGMessage *midi) const
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

  switch (sub()) {
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
      obs_source_media_set_time(obs_source, num.choose(midi) * 1000);
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
}

void MMGActionMediaSources::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionMediaSources *>(dest);
  if (!casted) return;

  casted->source = source.copy();
  casted->num = num.copy();
}

void MMGActionMediaSources::setEditable(bool edit)
{
  source.set_edit(edit);
  num.set_edit(edit);
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

void MMGActionMediaSources::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&source);

  MMGNumberDisplay *num_display = new MMGNumberDisplay(_display->numberDisplays());
  num_display->setStorage(&num, true);
  num_display->setVisible(false);
  _display->numberDisplays()->add(num_display);

  _display->connect(_display, &MMGActionDisplay::str1Changed, [&]() { setList1Config(); });
}

void MMGActionMediaSources::setSubOptions(QComboBox *sub)
{
  sub->addItems({"Play or Pause", "Restart", "Stop", "Set Track Time", "Next Track",
		 "Previous Track", "Skip Forward Time", "Skip Backward Time"});
}

void MMGActionMediaSources::setSubConfig()
{
  _display->setStr1Visible(true);
  _display->setStr1Description("Media Source");
  _display->setStr1Options(enumerate());
}

void MMGActionMediaSources::setList1Config()
{
  MMGNumberDisplay *num_display = _display->numberDisplays()->fieldAt(0);
  num_display->setVisible(false);

  switch ((Actions)subcategory) {
    case SOURCE_MEDIA_TIME:
      num_display->setVisible(!source.str().isEmpty());
      num_display->setDescription("Time");
      num_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_CUSTOM);
      num_display->setTimeFormat(true);
      num_display->setBounds(0.0, sourceDuration());
      num_display->setStep(1.0);
      num_display->setDefaultValue(0.0);
      break;

    case SOURCE_MEDIA_SKIP_FORWARD_TIME:
    case SOURCE_MEDIA_SKIP_BACKWARD_TIME:
      num_display->setVisible(!source.str().isEmpty());
      num_display->setDescription("Time Adjust");
      num_display->setOptions(MMGNumberDisplay::OPTIONS_FIXED_ONLY);
      num_display->setTimeFormat(true);
      num_display->setBounds(0.0, sourceDuration());
      num_display->setStep(1.0);
      num_display->setDefaultValue(0.0);
      break;

    default:
      return;
  }
  num_display->reset();
}

double MMGActionMediaSources::sourceDuration() const
{
  return obs_source_media_get_duration(
	   OBSSourceAutoRelease(obs_get_source_by_name(source.mmgtocs()))) /
	 1000.0;
}
