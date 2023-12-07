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

#include "mmg-signal.h"
#include "mmg-config.h"
#include "ui/mmg-action-display.h"

// MMGSourceSignal
#define GET_SIGNAL_OBJ()                     \
	static_cast<MMGSourceSignal *>(ptr); \
	if (!signal) return;

MMGSourceSignal::MMGSourceSignal(QObject *parent, obs_source_t *source)
	: QObject(parent), _source(obs_source_get_ref(source))
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
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceUpdated(calldata_ptr(cd, "source"));
}

void MMGSourceSignal::sourceVisibleCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceVisibilityChanged(calldata_ptr(cd, "source"), calldata_bool(cd, "visible"));
}

void MMGSourceSignal::sourceLockedCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceLocked(calldata_ptr(cd, "source"), calldata_bool(cd, "locked"));
}

void MMGSourceSignal::sourceTransformCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceTransformed(calldata_ptr(cd, "item"));
}

void MMGSourceSignal::sourceMuteCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceMuted(calldata_bool(cd, "muted"));
}

void MMGSourceSignal::sourceVolumeCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceVolumeChanged(calldata_float(cd, "volume"));
}

void MMGSourceSignal::sourceSyncOffsetCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceSyncOffsetChanged(calldata_int(cd, "offset"));
}

void MMGSourceSignal::sourceMonitoringCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->sourceMonitoringChanged(calldata_int(cd, "type"));
}

void MMGSourceSignal::mediaPlayCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->mediaPlayed();
}

void MMGSourceSignal::mediaPauseCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->mediaPaused();
}

void MMGSourceSignal::mediaRestartCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->mediaRestarted();
}

void MMGSourceSignal::mediaStopCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->mediaStopped();
}

void MMGSourceSignal::mediaPreviousCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->mediaPreviousChange();
}

void MMGSourceSignal::mediaNextCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->mediaNextChange();
}

void MMGSourceSignal::transitionStartedCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->transitionStarted();
}

void MMGSourceSignal::transitionStoppedCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->transitionStopped();
}

void MMGSourceSignal::filterEnableCallback(void *ptr, calldata_t *cd)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->filterEnabled(calldata_bool(cd, "enabled"));
}

void MMGSourceSignal::filterReorderCallback(void *ptr, calldata_t *)
{
	auto signal = GET_SIGNAL_OBJ();
	emit signal->filterReordered();
}
// End MMGSourceSignal

// MMGSignals
MMGSignals::MMGSignals(QObject *parent) : QObject(parent)
{
	obs_frontend_add_event_callback(frontendCallback, this);
	connect(this, &MMGSignals::frontendEvent, &MMGSignals::shutdownCallback);
	blockSignals(true);
}

void MMGSignals::enableHotkeyCallbacks(bool enabled)
{
	if (enabled) {
		obs_hotkey_enable_callback_rerouting(true);
		obs_hotkey_set_callback_routing_func(hotkeyCallback, this);
	} else {
		// Imitate OBS
		auto routing_func = [](void *data, obs_hotkey_id id, bool pressed) {
			QMetaObject::invokeMethod(static_cast<QObject *>(data), "ProcessHotkey",
						  Q_ARG(obs_hotkey_id, id), Q_ARG(bool, pressed));
		};
		obs_hotkey_set_callback_routing_func(routing_func, obs_frontend_get_main_window());
	}
}

const MMGSourceSignal *MMGSignals::sourceSignal(obs_source_t *source)
{
	for (const MMGSourceSignal *source_signal : source_signals) {
		if (source_signal->match(source)) return source_signal;
	}
	auto new_signal = new MMGSourceSignal(this, source);
	if (!new_signal->valid()) return nullptr;
	source_signals.append(new_signal);
	return new_signal;
}

void MMGSignals::frontendCallback(obs_frontend_event event, void *ptr)
{
	auto _signals = static_cast<MMGSignals *>(ptr);
	if (!_signals) return;

	emit _signals->frontendEvent(event);

	if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
		manager(binding)->resetConnections();
		_signals->blockSignals(false);
	}
}

void MMGSignals::hotkeyCallback(void *ptr, obs_hotkey_id id, bool pressed)
{
	obs_hotkey_trigger_routed_callback(id, pressed);
	if (!pressed) return;

	auto _signals = static_cast<MMGSignals *>(ptr);
	if (!_signals) return;

	emit _signals->hotkeyEvent(id);
}

void MMGSignals::shutdownCallback(obs_frontend_event event)
{
	if (event != OBS_FRONTEND_EVENT_EXIT) return;
	obs_frontend_remove_event_callback(frontendCallback, this);
	enableHotkeyCallbacks(false);

	for (MMGSourceSignal *signal : source_signals)
		signal->disconnectSignals();

	MMGActionDisplay::clearCustomOBSFields();
}
// End MMGSignals

MMGSignals *mmgsignals()
{
	return config()->mmgsignals();
}