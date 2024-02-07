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

MMGActionMediaSources::MMGActionMediaSources(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj), source(json_obj, "source", 1), num(json_obj, "num", 1)
{
	if (num.state() == STATE_CUSTOM) num.setState(STATE_MIDI);
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionMediaSources::subNames() const
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
		default:
			opts << subModuleText("PlayPause")
			     << obstr_all("ContextBar.MediaControls", {"RestartMedia", "StopMedia"})
			     << subModuleText("SetTime")
			     << obstr_all("ContextBar.MediaControls", {"PlaylistNext", "PlaylistPrevious"});
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"Played", "Paused", "Restarted", "Stopped"});
			break;
	}

	opts << subModuleTextList({"SkipForward", "SkipBackward"});
	return opts;
}

void MMGActionMediaSources::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	source.json(json_obj, "source", false);
	num.json(json_obj, "num");
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
	source.setEditable(edit);
	num.setEditable(edit);
}

void MMGActionMediaSources::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	MMGStringDisplay *source_display = display()->stringDisplays()->addNew(&source);
	display()->connect(source_display, &MMGStringDisplay::stringChanged, [&]() { onList1Change(); });

	display()->numberDisplays()->addNew(&num);
}

void MMGActionMediaSources::setActionParams()
{
	MMGStringDisplay *source_display = display()->stringDisplays()->fieldAt(0);
	source_display->setVisible(true);
	source_display->setDescription(mmgtr("Actions.MediaSources.MediaSource"));
	source_display->setBounds(enumerate());
}

void MMGActionMediaSources::onList1Change()
{
	if (type() == TYPE_OUTPUT) connectOBSSignals();

	MMGNumberDisplay *num_display = display()->numberDisplays()->fieldAt(0);
	num_display->setVisible(false);

	if (type() != TYPE_INPUT) return;

	switch (sub()) {
		case SOURCE_MEDIA_TIME:
			num_display->setVisible(!source.value().isEmpty());
			num_display->setDescription(mmgtr("Actions.MediaSources.Time"));
			num_display->setOptions(MIDIBUTTON_MIDI);
			num_display->setTimeFormat(true);
			num_display->setBounds(0.0, sourceDuration());
			num_display->setStep(1.0);
			num_display->setDefaultValue(0.0);
			break;

		case SOURCE_MEDIA_SKIP_FORWARD_TIME:
		case SOURCE_MEDIA_SKIP_BACKWARD_TIME:
			num_display->setVisible(!source.value().isEmpty());
			num_display->setDescription(mmgtr("Actions.MediaSources.TimeAdjust"));
			num_display->setOptions(MIDIBUTTON_FIXED);
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

double MMGActionMediaSources::sourceDuration() const
{
	return obs_source_media_get_duration(OBSSourceAutoRelease(obs_get_source_by_name(source.mmgtocs()))) / 1000.0;
}

void MMGActionMediaSources::execute(const MMGMessage *midi) const
{
	OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
	ACTION_ASSERT(obs_source, "Source does not exist.");

	ACTION_ASSERT(obs_source_get_output_flags(obs_source) & OBS_SOURCE_CONTROLLABLE_MEDIA,
		      "Source is not a media source.");

	MMGNumber time;
	time.setMax(sourceDuration());
	time = num.map(time, false);
	time.setState(num.state());

	switch (sub()) {
		case SOURCE_MEDIA_TOGGLE_PLAYPAUSE:
			switch (obs_source_media_get_state(obs_source)) {
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

		case SOURCE_MEDIA_RESTART:
			obs_source_media_restart(obs_source);
			break;

		case SOURCE_MEDIA_STOP:
			obs_source_media_stop(obs_source);
			break;

		case SOURCE_MEDIA_TIME:
			obs_source_media_set_time(obs_source, time.chooseFrom(midi) * 1000);
			break;

		case SOURCE_MEDIA_SKIP_FORWARD_TRACK:
			obs_source_media_next(obs_source);
			break;

		case SOURCE_MEDIA_SKIP_BACKWARD_TRACK:
			obs_source_media_previous(obs_source);
			break;

		case SOURCE_MEDIA_SKIP_FORWARD_TIME:
			obs_source_media_set_time(obs_source, obs_source_media_get_time(obs_source) + (time * 1000));
			break;

		case SOURCE_MEDIA_SKIP_BACKWARD_TIME:
			obs_source_media_set_time(obs_source, obs_source_media_get_time(obs_source) - (time * 1000));
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionMediaSources::connectOBSSignals()
{
	disconnectOBSSignals();

	active_source_signal =
		mmgsignals()->sourceSignal(OBSSourceAutoRelease(obs_get_source_by_name(source.mmgtocs())));
	if (!active_source_signal) return;

	switch (sub()) {
		case SOURCE_MEDIA_PLAYED:
			connect(active_source_signal, &MMGSourceSignal::mediaPlayed, this,
				&MMGActionMediaSources::mediaPlayedCallback);
			break;

		case SOURCE_MEDIA_PAUSED:
			connect(active_source_signal, &MMGSourceSignal::mediaPaused, this,
				&MMGActionMediaSources::mediaPausedCallback);
			break;

		case SOURCE_MEDIA_RESTARTED:
			connect(active_source_signal, &MMGSourceSignal::mediaRestarted, this,
				&MMGActionMediaSources::mediaRestartedCallback);
			break;

		case SOURCE_MEDIA_STOPPED:
			connect(active_source_signal, &MMGSourceSignal::mediaStopped, this,
				&MMGActionMediaSources::mediaStoppedCallback);
			break;

		case SOURCE_MEDIA_SKIPPED_FORWARD_TRACK:
			connect(active_source_signal, &MMGSourceSignal::mediaNextChange, this,
				&MMGActionMediaSources::mediaNextCallback);
			break;

		case SOURCE_MEDIA_SKIPPED_BACKWARD_TRACK:
			connect(active_source_signal, &MMGSourceSignal::mediaPreviousChange, this,
				&MMGActionMediaSources::mediaPreviousCallback);
			break;

		default:
			break;
	}
}

void MMGActionMediaSources::disconnectOBSSignals()
{
	if (!!active_source_signal) disconnect(active_source_signal, nullptr, this, nullptr);
	active_source_signal = nullptr;
}

void MMGActionMediaSources::mediaPlayedCallback()
{
	if (sub() != SOURCE_MEDIA_PLAYED) return;
	triggerEvent();
}

void MMGActionMediaSources::mediaPausedCallback()
{
	if (sub() != SOURCE_MEDIA_PAUSED) return;
	triggerEvent();
}

void MMGActionMediaSources::mediaRestartedCallback()
{
	if (sub() != SOURCE_MEDIA_RESTARTED) return;
	triggerEvent();
}

void MMGActionMediaSources::mediaStoppedCallback()
{
	if (sub() != SOURCE_MEDIA_STOPPED) return;
	triggerEvent();
}

void MMGActionMediaSources::mediaNextCallback()
{
	if (sub() != SOURCE_MEDIA_SKIPPED_FORWARD_TRACK) return;
	triggerEvent();
}

void MMGActionMediaSources::mediaPreviousCallback()
{
	if (sub() != SOURCE_MEDIA_SKIPPED_BACKWARD_TRACK) return;
	triggerEvent();
}
