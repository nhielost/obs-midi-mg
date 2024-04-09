/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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

#include "mmg-signal.h"
#include "mmg-config.h"
#include "ui/mmg-fields.h"

using namespace MMGUtils;

#define SEND_SOURCE_EVENT(...)                                     \
	auto signal = static_cast<MMGSourceSignal *>(ptr);         \
	if (!signal) return;                                       \
                                                                   \
	if (calldata_ptr(cd, "source") != signal->_source) return; \
	emit signal->sourceChanged(__VA_ARGS__)

// MMGSourceSignal
MMGSourceSignal::MMGSourceSignal(QObject *parent, obs_source_t *source)
	: QObject(parent),
	  _source(obs_source_get_ref(source))
{
	if (!source) {
		deleteLater();
		return;
	}

	obs_source_type source_type = obs_source_get_type(source);
	quint32 source_cap_flags = obs_source_get_output_flags(source);
	signal_handler_t *sh = obs_source_get_signal_handler(source);

	signal_handler_connect(sh, "update", sourceUpdateCallback, this);
	signal_handler_connect(sh, "reorder_filters", filterReorderCallback, this);

	if (source_type == OBS_SOURCE_TYPE_SCENE) {
		signal_handler_connect(sh, "item_visible", sourceVisibleCallback, this);
		signal_handler_connect(sh, "item_locked", sourceLockedCallback, this);
		signal_handler_connect(sh, "item_transform", sourceTransformCallback, this);
	}

	if (source_cap_flags & OBS_SOURCE_AUDIO) {
		signal_handler_connect(sh, "mute", sourceMuteCallback, this);
		signal_handler_connect(sh, "volume", sourceVolumeCallback, this);
		signal_handler_connect(sh, "audio_sync", sourceSyncOffsetCallback, this);
		signal_handler_connect(sh, "audio_monitoring", sourceMonitoringCallback, this);
	}

	if (source_cap_flags & OBS_SOURCE_CONTROLLABLE_MEDIA) {
		signal_handler_connect(sh, "media_play", mediaPlayCallback, this);
		signal_handler_connect(sh, "media_pause", mediaPauseCallback, this);
		signal_handler_connect(sh, "media_restart", mediaRestartCallback, this);
		signal_handler_connect(sh, "media_stopped", mediaStopCallback, this);
		signal_handler_connect(sh, "media_previous", mediaPreviousCallback, this);
		signal_handler_connect(sh, "media_next", mediaNextCallback, this);
	}

	if (source_type == OBS_SOURCE_TYPE_TRANSITION) {
		signal_handler_connect(sh, "transition_start", transitionStartedCallback, this);
		signal_handler_connect(sh, "transition_stop", transitionStoppedCallback, this);
	}

	if (source_type == OBS_SOURCE_TYPE_FILTER) signal_handler_connect(sh, "enable", filterEnableCallback, this);
}

void MMGSourceSignal::disconnectSignals()
{
	if (!_source) return;

	signal_handler_t *sh = obs_source_get_signal_handler(_source);

	signal_handler_disconnect(sh, "update", sourceUpdateCallback, this);
	signal_handler_disconnect(sh, "reorder_filters", filterReorderCallback, this);

	signal_handler_disconnect(sh, "item_visible", sourceVisibleCallback, this);
	signal_handler_disconnect(sh, "item_locked", sourceLockedCallback, this);
	signal_handler_disconnect(sh, "item_transform", sourceTransformCallback, this);

	signal_handler_disconnect(sh, "mute", sourceMuteCallback, this);
	signal_handler_disconnect(sh, "volume", sourceVolumeCallback, this);
	signal_handler_disconnect(sh, "audio_sync", sourceSyncOffsetCallback, this);
	signal_handler_disconnect(sh, "audio_monitoring", sourceMonitoringCallback, this);

	signal_handler_disconnect(sh, "media_play", mediaPlayCallback, this);
	signal_handler_disconnect(sh, "media_pause", mediaPauseCallback, this);
	signal_handler_disconnect(sh, "media_restart", mediaRestartCallback, this);
	signal_handler_disconnect(sh, "media_stopped", mediaStopCallback, this);
	signal_handler_disconnect(sh, "media_previous", mediaPreviousCallback, this);
	signal_handler_disconnect(sh, "media_next", mediaNextCallback, this);

	signal_handler_disconnect(sh, "transition_start", transitionStartedCallback, this);
	signal_handler_disconnect(sh, "transition_stop", transitionStoppedCallback, this);

	signal_handler_disconnect(sh, "enable", filterEnableCallback, this);

	obs_source_release(_source);
}

void MMGSourceSignal::sourceUpdateCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_UPDATE, QVariant::fromValue(calldata_ptr(cd, "source")));
}

void MMGSourceSignal::sourceVisibleCallback(void *ptr, calldata_t *cd)
{
	calldata_set_ptr(cd, "source", obs_scene_get_source((obs_scene_t *)calldata_ptr(cd, "scene")));
	SEND_SOURCE_EVENT(SIGNAL_VISIBILITY, QVariant::fromValue(calldata_ptr(cd, "item")));
}

void MMGSourceSignal::sourceLockedCallback(void *ptr, calldata_t *cd)
{
	calldata_set_ptr(cd, "source", obs_scene_get_source((obs_scene_t *)calldata_ptr(cd, "scene")));
	SEND_SOURCE_EVENT(SIGNAL_LOCK, QVariant::fromValue(calldata_ptr(cd, "item")));
}

void MMGSourceSignal::sourceTransformCallback(void *ptr, calldata_t *cd)
{
	calldata_set_ptr(cd, "source", obs_scene_get_source((obs_scene_t *)calldata_ptr(cd, "scene")));
	SEND_SOURCE_EVENT(SIGNAL_TRANSFORM, QVariant::fromValue(calldata_ptr(cd, "item")));
}

void MMGSourceSignal::sourceMuteCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MUTE, calldata_bool(cd, "muted"));
}

void MMGSourceSignal::sourceVolumeCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_VOLUME, calldata_float(cd, "volume"));
}

void MMGSourceSignal::sourceSyncOffsetCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_SYNC_OFFSET, calldata_int(cd, "offset"));
}

void MMGSourceSignal::sourceMonitoringCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MONITOR, calldata_int(cd, "type"));
}

void MMGSourceSignal::mediaPlayCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MEDIA_PLAY);
}

void MMGSourceSignal::mediaPauseCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MEDIA_PAUSE);
}

void MMGSourceSignal::mediaRestartCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MEDIA_RESTART);
}

void MMGSourceSignal::mediaStopCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MEDIA_STOP);
}

void MMGSourceSignal::mediaPreviousCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MEDIA_PREVIOUS);
}

void MMGSourceSignal::mediaNextCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_MEDIA_NEXT);
}

void MMGSourceSignal::transitionStartedCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_TRANSITION_START);
}

void MMGSourceSignal::transitionStoppedCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_TRANSITION_STOP);
}

void MMGSourceSignal::filterEnableCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_FILTER_ENABLE, calldata_bool(cd, "enabled"));
}

void MMGSourceSignal::filterReorderCallback(void *ptr, calldata_t *cd)
{
	SEND_SOURCE_EVENT(SIGNAL_FILTER_REORDER);
}
// End MMGSourceSignal

// MMGSignals
MMGSignals::MMGSignals(QObject *parent) : QObject(parent)
{
	obs_frontend_add_event_callback(frontendCallback, this);
	obs_hotkey_enable_callback_rerouting(true);
	obs_hotkey_set_callback_routing_func(hotkeyCallback, this);
}

const MMGSourceSignal *MMGSignals::requestSourceSignal(obs_source_t *source)
{
	if (!source) return nullptr;

	MMGSourceSignal *signal = nullptr;
	for (MMGSourceSignal *source_signal : source_signals) {
		if (source_signal->match(source)) {
			signal = source_signal;
			break;
		}
	}

	if (!signal) {
		signal = new MMGSourceSignal(this, source);
		source_signals.append(signal);
	}

	return signal;
}

const MMGSourceSignal *MMGSignals::requestSourceSignalByName(const MMGString &name)
{
	return requestSourceSignal(OBSSourceAutoRelease(obs_get_source_by_name(name.mmgtocs())));
}

void MMGSignals::disconnectAllSignals(QObject *obj)
{
	for (MMGSourceSignal *source_signal : source_signals)
		disconnect(source_signal, &MMGSourceSignal::sourceChanged, obj, nullptr);

	disconnect(this, &MMGSignals::frontendEvent, obj, nullptr);
	disconnect(this, &MMGSignals::hotkeyEvent, obj, nullptr);
}

void MMGSignals::frontendCallback(obs_frontend_event event, void *ptr)
{
	MMGSignals *signal = static_cast<MMGSignals *>(ptr);
	if (!signal) return;

	switch (event) {
		case OBS_FRONTEND_EVENT_FINISHED_LOADING:
			for (MMGBindingManager *manager : *manager(collection))
				manager->refreshAll();

			signal->allow_events = true;
			break;

		case OBS_FRONTEND_EVENT_EXIT:
			obs_frontend_remove_event_callback(frontendCallback, signal);

			for (MMGSourceSignal *signal : signal->source_signals)
				signal->disconnectSignals();

			MMGOBSFields::clearFields();
			qDeleteAll(signal->source_signals);
			break;

		default:
			if (signal->allow_events) emit signal->frontendEvent(event);
			break;
	}
}

void MMGSignals::hotkeyCallback(void *ptr, obs_hotkey_id id, bool pressed)
{
	QMetaObject::invokeMethod(static_cast<QObject *>(obs_frontend_get_main_window()), "ProcessHotkey",
				  Q_ARG(obs_hotkey_id, id), Q_ARG(bool, pressed));

	if (!pressed) return;

	emit mmgsignals()->hotkeyEvent(id);
}
// End MMGSignals

MMGSignals *mmgsignals()
{
	return config()->mmgsignals();
}