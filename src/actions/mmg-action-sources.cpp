/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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

#include "mmg-action-sources.h"

namespace MMGActions {

static MMGParams<MMGString> source_params {
	.desc = obstr("Basic.Main.Source"),
	.options = OPTION_NONE,
	.default_value = "",
	.bounds = {},
	.placeholder = obstr("NoSources.Title"),
};

const MMGStringTranslationMap enumerateSources(uint64_t flags)
{
	struct SourceEnumeration {
		uint64_t source_flags;
		MMGStringTranslationMap source_names;
	} source_enum;
	source_enum.source_flags = flags;

	obs_enum_all_sources(
		[](void *param, obs_source_t *source) {
			auto _source_enum = reinterpret_cast<SourceEnumeration *>(param);

			if (obs_obj_is_private(source)) return true;
			if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT) return true;
			if (!(obs_source_get_output_flags(source) & _source_enum->source_flags)) return true;

			_source_enum->source_names.insert(obs_source_get_uuid(source),
							  nontr(obs_source_get_name(source)));
			return true;
		},
		&source_enum);

	return source_enum.source_names;
}

const MMGStringTranslationMap enumerateAllSources()
{
	return enumerateSources(-1);
}

const MMGStringTranslationMap enumerateAudioSources()
{
	return enumerateSources(OBS_SOURCE_AUDIO);
}

const MMGStringTranslationMap enumerateMediaSources()
{
	return enumerateSources(OBS_SOURCE_CONTROLLABLE_MEDIA);
}

const MMGParams<MMGString> &sourceParams(uint64_t type)
{
	source_params.bounds = enumerateSources(type);
	source_params.default_value = !source_params.bounds.isEmpty() ? source_params.bounds.firstKey() : MMGString();

	return source_params;
}

// MMGActionSources
MMGActionSources::MMGActionSources(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  source(json_obj, "source")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSources::initOldData(const QJsonObject &json_obj)
{
	int fallback = 1 + (json_obj["category"].toInt() == 7);
	MMGCompatibility::initOldStringData(source, json_obj, "source", fallback, sourceParams(sourceBounds()).bounds);
}

void MMGActionSources::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	source->json(json_obj, "source");
}

void MMGActionSources::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionSources *>(dest);
	if (!casted) return;

	source.copy(casted->source);
}

void MMGActionSources::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &source, &sourceParams(sourceBounds()),
				      std::bind(&MMGActionSources::onSourceChanged, this));
}

void MMGActionSources::onSourceChanged() const
{
	emit refreshRequested();
	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(MMGString(source));
	onSourceFixedChanged(obs_source);
}

void MMGActionSources::execute(const MMGMappingTest &test) const
{
	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(MMGString(source));
	ACTION_ASSERT(obs_source, "Source does not exist.");

	execute(test, obs_source);
}

void MMGActionSources::processEvent(const calldata_t *cd) const
{
	obs_source_t *signal_source = (obs_source_t *)(calldata_ptr(cd, "source"));
	current_uuid = obs_source_get_uuid(signal_source);

	EventFulfillment fulfiller(this);
	processEvent(*fulfiller, signal_source);
}
// End MMGActionSources

// MMGActionSourcesAudioVolume
MMGParams<MMGString> MMGActionSourcesAudioVolume::format_params {
	.desc = mmgtr("Actions.Sources.Format"),
	.options = OPTION_NONE,
	.default_value = "%",
	.bounds =
		{
			{"%", mmgtr("Actions.Sources.Format.Percent")},
			{"dB", mmgtr("Actions.Sources.Format.Decibels")},
		},
};

MMGParams<float> MMGActionSourcesAudioVolume::volume_params {
	.desc = obstr("Basic.AdvAudio.Volume"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_INCREMENT,
	.default_value = 0.0,
	.lower_bound = 0.0,
	.upper_bound = 100.0,
	.step = 0.1,
	.incremental_bound = 50.0,
};

MMGActionSourcesAudioVolume::MMGActionSourcesAudioVolume(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSources(parent, json_obj),
	  format(json_obj, "format"),
	  volume(json_obj, "volume")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSourcesAudioVolume::initOldData(const QJsonObject &json_obj)
{
	MMGActionSources::initOldData(json_obj);

	MMGCompatibility::initOldStringData(format, json_obj, "action", 1, format_params.bounds);
	MMGCompatibility::initOldNumberData(volume, json_obj, "num", 1);
	if (json_obj["sub"].toInt() == 1) volume.changeTo<STATE_INCREMENT>();
}

void MMGActionSourcesAudioVolume::json(QJsonObject &json_obj) const
{
	MMGActionSources::json(json_obj);

	format->json(json_obj, "format");
	volume->json(json_obj, "volume");
}

void MMGActionSourcesAudioVolume::copy(MMGAction *dest) const
{
	MMGActionSources::copy(dest);

	auto casted = dynamic_cast<MMGActionSourcesAudioVolume *>(dest);
	if (!casted) return;

	format.copy(casted->format);
	volume.copy(casted->volume);
}

void MMGActionSourcesAudioVolume::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSources::createDisplay(display);

	MMGActions::createActionField(display, &format, &format_params,
				      std::bind(&MMGActionSourcesAudioVolume::onFormatChanged, this));
	MMGActions::createActionField(display, &volume, &volume_params);
}

void MMGActionSourcesAudioVolume::onSourceFixedChanged(const obs_source_t *obs_source) const
{
	format_params.options.setFlag(OPTION_HIDDEN, !obs_source);
	volume_params.options.setFlag(OPTION_HIDDEN, !obs_source);
	if (!obs_source) return;

	onFormatChanged();
	volume_params.default_value = convertIfToDecibels(obs_source_get_volume(obs_source));
}

void MMGActionSourcesAudioVolume::onFormatChanged() const
{
	bool use_decibel = format == "dB";

	volume_params.lower_bound = use_decibel ? -100.0 : 0.0;
	volume_params.upper_bound = use_decibel ? 0.0 : 100.0;
	volume_params.incremental_bound = 50.0;
}

void MMGActionSourcesAudioVolume::execute(const MMGMappingTest &test, obs_source_t *obs_source) const
{
	float current_volume = convertIfToDecibels(obs_source_get_volume(obs_source));
	ACTION_ASSERT(test.applicable(volume, current_volume),
		      "A volume could not be selected. Check the Volume field and try again.");
	obs_source_set_volume(obs_source, convertIfFromDecibels(current_volume));
}

void MMGActionSourcesAudioVolume::processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const
{
	test.addAcceptable(volume, convertIfToDecibels(obs_source_get_volume(obs_source)));
}
// End MMGActionSourcesAudioVolume

// MMGActionSourcesAudioMute
MMGParams<bool> MMGActionSourcesAudioMute::mute_params {
	.desc = mmgtr("Plugin.Status"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

MMGActionSourcesAudioMute::MMGActionSourcesAudioMute(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSources(parent, json_obj),
	  mute(json_obj, "muted")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSourcesAudioMute::initOldData(const QJsonObject &json_obj)
{
	MMGActionSources::initOldData(json_obj);

	MMGCompatibility::initOldBooleanData(mute, json_obj["sub"].toInt() - 2);
}

void MMGActionSourcesAudioMute::json(QJsonObject &json_obj) const
{
	MMGActionSources::json(json_obj);

	mute->json(json_obj, "muted");
}

void MMGActionSourcesAudioMute::copy(MMGAction *dest) const
{
	MMGActionSources::copy(dest);

	auto casted = dynamic_cast<MMGActionSourcesAudioMute *>(dest);
	if (!casted) return;

	mute.copy(casted->mute);
}

void MMGActionSourcesAudioMute::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSources::createDisplay(display);

	MMGActions::createActionField(display, &mute, &mute_params);
}

void MMGActionSourcesAudioMute::onSourceFixedChanged(const obs_source_t *obs_source) const
{
	mute_params.options.setFlag(OPTION_HIDDEN, !obs_source);
	if (!obs_source) return;

	mute_params.default_value = !obs_source_muted(obs_source);
}

void MMGActionSourcesAudioMute::execute(const MMGMappingTest &test, obs_source_t *obs_source) const
{
	bool muted = obs_source_muted(obs_source);
	ACTION_ASSERT(test.applicable(mute, muted), "A mute status could not be selected. Check the Status field "
						    "and try again.");
	obs_source_set_muted(obs_source, muted);
}

void MMGActionSourcesAudioMute::processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const
{
	test.addAcceptable(mute, obs_source_muted(obs_source));
}
// End MMGActionSourcesAudioMute

// MMGActionSourcesAudioSyncOffset
MMGParams<int64_t> MMGActionSourcesAudioSyncOffset::offset_params {
	.desc = obstr("Basic.AdvAudio.SyncOffset"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_INCREMENT,
	.default_value = 0,
	.lower_bound = 0.0,
	.upper_bound = 20000.0,
	.step = 25.0,
	.incremental_bound = 5000.0,
};

MMGActionSourcesAudioSyncOffset::MMGActionSourcesAudioSyncOffset(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSources(parent, json_obj),
	  offset(json_obj, "offset")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSourcesAudioSyncOffset::initOldData(const QJsonObject &json_obj)
{
	MMGActionSources::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(offset, json_obj, "num", 1);
}

void MMGActionSourcesAudioSyncOffset::json(QJsonObject &json_obj) const
{
	MMGActionSources::json(json_obj);

	offset->json(json_obj, "offset");
}

void MMGActionSourcesAudioSyncOffset::copy(MMGAction *dest) const
{
	MMGActionSources::copy(dest);

	auto casted = dynamic_cast<MMGActionSourcesAudioSyncOffset *>(dest);
	if (!casted) return;

	offset.copy(casted->offset);
}

void MMGActionSourcesAudioSyncOffset::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSources::createDisplay(display);

	MMGActions::createActionField(display, &offset, &offset_params);
}

void MMGActionSourcesAudioSyncOffset::onSourceFixedChanged(const obs_source_t *obs_source) const
{
	offset_params.options.setFlag(OPTION_HIDDEN, !obs_source);
	if (!obs_source) return;

	offset_params.default_value = getMillisecondOffset(obs_source);
}

void MMGActionSourcesAudioSyncOffset::execute(const MMGMappingTest &test, obs_source_t *obs_source) const
{
	int64_t current_offset = getMillisecondOffset(obs_source);
	ACTION_ASSERT(test.applicable(offset, current_offset),
		      "A sync offset could not be selected. Check the Sync Offset "
		      "field and try again.");
	obs_source_set_sync_offset(obs_source, current_offset * 1000000);
}

void MMGActionSourcesAudioSyncOffset::processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const
{
	test.addAcceptable(offset, getMillisecondOffset(obs_source));
}
// End MMGActionSourcesAudioSyncOffset

// MMGActionSourcesAudioMonitor
MMGParams<obs_monitoring_type> MMGActionSourcesAudioMonitor::monitor_params {
	.desc = obstr("Basic.AdvAudio.Monitoring"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = OBS_MONITORING_TYPE_NONE,
	.bounds =
		{
			{OBS_MONITORING_TYPE_NONE, obstr("Basic.AdvAudio.Monitoring.None")},
			{OBS_MONITORING_TYPE_MONITOR_ONLY, obstr("Basic.AdvAudio.Monitoring.MonitorOnly")},
			{OBS_MONITORING_TYPE_MONITOR_AND_OUTPUT, obstr("Basic.AdvAudio.Monitoring.Both")},
		},
};

MMGActionSourcesAudioMonitor::MMGActionSourcesAudioMonitor(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSources(parent, json_obj),
	  monitor(json_obj, "monitor")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSourcesAudioMonitor::initOldData(const QJsonObject &json_obj)
{
	MMGActionSources::initOldData(json_obj);

	MMGCompatibility::initOldStringData(monitor, json_obj, "action", 2, monitor_params.bounds);
}

void MMGActionSourcesAudioMonitor::json(QJsonObject &json_obj) const
{
	MMGActionSources::json(json_obj);

	monitor->json(json_obj, "monitor");
}

void MMGActionSourcesAudioMonitor::copy(MMGAction *dest) const
{
	MMGActionSources::copy(dest);

	auto casted = dynamic_cast<MMGActionSourcesAudioMonitor *>(dest);
	if (!casted) return;

	monitor.copy(casted->monitor);
}

void MMGActionSourcesAudioMonitor::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSources::createDisplay(display);

	MMGActions::createActionField(display, &monitor, &monitor_params);
}

void MMGActionSourcesAudioMonitor::onSourceFixedChanged(const obs_source_t *obs_source) const
{
	monitor_params.options.setFlag(OPTION_HIDDEN, !obs_source);
	if (!obs_source) return;

	monitor_params.default_value = obs_source_get_monitoring_type(obs_source);
}

void MMGActionSourcesAudioMonitor::execute(const MMGMappingTest &test, obs_source_t *obs_source) const
{
	obs_monitoring_type current_monitor = obs_source_get_monitoring_type(obs_source);
	ACTION_ASSERT(test.applicable(monitor, current_monitor),
		      "An audio monitor could not be selected. Check the Monitor "
		      "field and try again.");
	obs_source_set_monitoring_type(obs_source, current_monitor);
}

void MMGActionSourcesAudioMonitor::processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const
{
	test.addAcceptable(monitor, obs_source_get_monitoring_type(obs_source));
}
// End MMGActionSourcesAudioMonitor

// MMGActionSourcesMediaState
MMGParams<MMGActionSourcesMediaState::Actions> MMGActionSourcesMediaState::media_state_params {
	.desc = mmgtr("Actions.Sources.MediaOperation"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = MMGActionSourcesMediaState::PLAY,
	.bounds =
		{
			{MMGActionSourcesMediaState::PLAY, obstr("ContextBar.MediaControls.PlayMedia")},
			{MMGActionSourcesMediaState::PAUSE, obstr("ContextBar.MediaControls.PauseMedia")},
			{MMGActionSourcesMediaState::RESTART, obstr("ContextBar.MediaControls.RestartMedia")},
			{MMGActionSourcesMediaState::STOP, obstr("ContextBar.MediaControls.StopMedia")},
			{MMGActionSourcesMediaState::TRACK_FORWARD, obstr("ContextBar.MediaControls.PlaylistNext")},
			{MMGActionSourcesMediaState::TRACK_BACKWARD,
			 obstr("ContextBar.MediaControls.PlaylistPrevious")},
		},
};

MMGActionSourcesMediaState::MMGActionSourcesMediaState(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSources(parent, json_obj),
	  media_state(json_obj, "media_state")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSourcesMediaState::initOldData(const QJsonObject &json_obj)
{
	MMGActionSources::initOldData(json_obj);

	if (json_obj["type"].toInt() == TYPE_OUTPUT) {
		media_state = (Actions)(json_obj["sub"].toInt());
		return;
	}

	switch (json_obj["sub"].toInt()) {
		case 0: {
			auto *toggled_media = media_state.changeTo<STATE_TOGGLE>();
			toggled_media->setSize(2);
			toggled_media->set(0, PLAY);
			toggled_media->set(1, PAUSE);
			break;
		}

		case 1:
			media_state = RESTART;
			break;

		case 2:
			media_state = STOP;
			break;

		case 4:
			media_state = TRACK_FORWARD;
			break;

		case 5:
			media_state = TRACK_BACKWARD;
			break;
	}
}

void MMGActionSourcesMediaState::json(QJsonObject &json_obj) const
{
	MMGActionSources::json(json_obj);

	media_state->json(json_obj, "media_state");
}

void MMGActionSourcesMediaState::copy(MMGAction *dest) const
{
	MMGActionSources::copy(dest);

	auto casted = dynamic_cast<MMGActionSourcesMediaState *>(dest);
	if (!casted) return;

	media_state.copy(casted->media_state);
}

const char *MMGActionSourcesMediaState::sourceSignalName() const
{
	switch (media_state) {
		default:
		case PLAY:
			return "media_play";

		case PAUSE:
			return "media_pause";

		case RESTART:
			return "media_restart";

		case STOP:
			return "media_stopped";

		case TRACK_FORWARD:
			return "media_next";

		case TRACK_BACKWARD:
			return "media_previous";
	}
}

void MMGActionSourcesMediaState::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSources::createDisplay(display);

	media_state_params.options.setFlag(OPTION_ALLOW_MIDI, type() != TYPE_OUTPUT);
	media_state_params.options.setFlag(OPTION_ALLOW_TOGGLE, type() != TYPE_OUTPUT);

	MMGActions::createActionField(display, &media_state, &media_state_params,
				      std::bind(&MMGAction::refreshRequested, this));
}

void MMGActionSourcesMediaState::onSourceFixedChanged(const obs_source_t *obs_source) const
{
	media_state_params.options.setFlag(OPTION_HIDDEN, !obs_source);
}

void MMGActionSourcesMediaState::execute(const MMGMappingTest &test, obs_source_t *obs_source) const
{
	obs_media_state current_state = obs_source_media_get_state(obs_source);
	Actions new_state;
	ACTION_ASSERT(test.applicable(media_state, new_state),
		      "A media operation could not be selected. Check the Operation "
		      "field and try again.");

	switch (new_state) {
		case PAUSE:
			if (current_state == OBS_MEDIA_STATE_PLAYING) obs_source_media_play_pause(obs_source, true);
			break;

		case PLAY:
			if (current_state == OBS_MEDIA_STATE_PAUSED) {
				obs_source_media_play_pause(obs_source, false);
				break;
			} else if (current_state != OBS_MEDIA_STATE_STOPPED && current_state != OBS_MEDIA_STATE_ENDED) {
				break;
			}
			[[fallthrough]];

		case RESTART:
			obs_source_media_restart(obs_source);
			break;

		case STOP:
			obs_source_media_stop(obs_source);
			break;

		case TRACK_FORWARD:
			obs_source_media_next(obs_source);
			break;

		case TRACK_BACKWARD:
			obs_source_media_previous(obs_source);
			break;
	}
}
// End MMGActionSourcesMediaState

// MMGActionSourcesMediaTime
MMGParams<int64_t> MMGActionSourcesMediaTime::time_params {
	.desc = mmgtr("Actions.Sources.Time"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_INCREMENT | OPTION_SPECIAL_1,
	.default_value = 0,
	.lower_bound = 0.0,
	.upper_bound = 0.0,
	.step = 1.0,
	.incremental_bound = 6000.0,
};

MMGActionSourcesMediaTime::MMGActionSourcesMediaTime(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSources(parent, json_obj),
	  time(json_obj, "time")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSourcesMediaTime::initOldData(const QJsonObject &json_obj)
{
	MMGActionSources::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(time, json_obj, "num", 1);
	if (json_obj["sub"].toInt() >= 6 && time->state() == STATE_FIXED) time.changeTo<STATE_INCREMENT>();
}

void MMGActionSourcesMediaTime::json(QJsonObject &json_obj) const
{
	MMGActionSources::json(json_obj);

	time->json(json_obj, "time");
}

void MMGActionSourcesMediaTime::copy(MMGAction *dest) const
{
	MMGActionSources::copy(dest);

	auto casted = dynamic_cast<MMGActionSourcesMediaTime *>(dest);
	if (!casted) return;

	time.copy(casted->time);
}

void MMGActionSourcesMediaTime::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSources::createDisplay(display);

	MMGActions::createActionField(display, &time, &time_params);
}

void MMGActionSourcesMediaTime::onSourceFixedChanged(const obs_source_t *obs_source) const
{
	time_params.options.setFlag(OPTION_HIDDEN, !obs_source);
	if (!obs_source) return;

	time_params.upper_bound = obs_source_media_get_duration(const_cast<obs_source_t *>(obs_source)) / 1000ll;
}

void MMGActionSourcesMediaTime::execute(const MMGMappingTest &test, obs_source_t *obs_source) const
{
	int64_t new_time = obs_source_media_get_time(obs_source) / 1000ll;
	ACTION_ASSERT(test.applicable(time, new_time), "A media time could not be selected. Check the Time field and "
						       "try again.");
	obs_source_media_set_time(obs_source, new_time * 1000ll);
}
// End MMGActionSourcesMediaTime

// MMGActionSourcesCustom
MMGActionSourcesCustom::MMGActionSourcesCustom(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGActionSources(parent, json_obj),
	  custom_data(new MMGOBSFields::MMGOBSObject(this, sourceId(), json_obj))
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionSourcesCustom::initOldData(const QJsonObject &json_obj)
{
	MMGActionSources::initOldData(json_obj);

	custom_data->changeSource(sourceId(), json_obj);
}

void MMGActionSourcesCustom::json(QJsonObject &json_obj) const
{
	MMGActionSources::json(json_obj);

	custom_data->json(json_obj);
}

void MMGActionSourcesCustom::copy(MMGAction *dest) const
{
	MMGActionSources::copy(dest);

	auto casted = dynamic_cast<MMGActionSourcesCustom *>(dest);
	if (!casted) return;

	custom_data->copy(casted->custom_data);
}

void MMGActionSourcesCustom::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActionSources::createDisplay(display);

	custom_data->createDisplay(display);
}

void MMGActionSourcesCustom::onSourceFixedChanged(const obs_source_t *obs_source) const
{
	custom_data->changeSource(obs_source_get_uuid(obs_source));
}

void MMGActionSourcesCustom::execute(const MMGMappingTest &test, obs_source_t *) const
{
	custom_data->execute(test);
}

void MMGActionSourcesCustom::processEvent(MMGMappingTest &test, const obs_source_t *) const
{
	custom_data->processEvent(test);
}
// End MMGActionSourcesCustom

} // namespace MMGActions
