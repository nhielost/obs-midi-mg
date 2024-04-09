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

#ifndef MMG_SIGNAL_H
#define MMG_SIGNAL_H

#include "mmg-utils.h"

#include <QEvent>

class MMGSourceSignal : public QObject {
	Q_OBJECT

public:
	MMGSourceSignal(QObject *parent, obs_source_t *source);

	enum Event {
		SIGNAL_UPDATE,
		SIGNAL_VISIBILITY,
		SIGNAL_LOCK,
		SIGNAL_TRANSFORM,
		SIGNAL_MUTE,
		SIGNAL_VOLUME,
		SIGNAL_SYNC_OFFSET,
		SIGNAL_MONITOR,
		SIGNAL_MEDIA_PLAY,
		SIGNAL_MEDIA_PAUSE,
		SIGNAL_MEDIA_RESTART,
		SIGNAL_MEDIA_STOP,
		SIGNAL_MEDIA_PREVIOUS,
		SIGNAL_MEDIA_NEXT,
		SIGNAL_FILTER_ENABLE,
		SIGNAL_FILTER_REORDER,
		SIGNAL_TRANSITION_START,
		SIGNAL_TRANSITION_STOP,
	};

	obs_source_t *source() const { return _source; };
	bool match(const obs_source_t *source) const { return _source == source; };

	void disconnectSignals();

signals:
	void sourceChanged(Event, QVariant = QVariant());

private:
	obs_source_t *_source = nullptr;

	static void sourceUpdateCallback(void *ptr, calldata_t *cd);

	static void sourceVisibleCallback(void *ptr, calldata_t *cd);
	static void sourceLockedCallback(void *ptr, calldata_t *cd);
	static void sourceTransformCallback(void *ptr, calldata_t *cd);

	static void sourceMuteCallback(void *ptr, calldata_t *cd);
	static void sourceVolumeCallback(void *ptr, calldata_t *cd);
	static void sourceSyncOffsetCallback(void *ptr, calldata_t *cd);
	static void sourceMonitoringCallback(void *ptr, calldata_t *cd);

	static void mediaPlayCallback(void *ptr, calldata_t *cd);
	static void mediaPauseCallback(void *ptr, calldata_t *cd);
	static void mediaRestartCallback(void *ptr, calldata_t *cd);
	static void mediaStopCallback(void *ptr, calldata_t *cd);
	static void mediaPreviousCallback(void *ptr, calldata_t *cd);
	static void mediaNextCallback(void *ptr, calldata_t *cd);

	static void transitionStartedCallback(void *ptr, calldata_t *cd);
	static void transitionStoppedCallback(void *ptr, calldata_t *cd);

	static void filterEnableCallback(void *ptr, calldata_t *cd);
	static void filterReorderCallback(void *ptr, calldata_t *cd);
};

class MMGSignals : public QObject {
	Q_OBJECT

public:
	MMGSignals(QObject *parent);

	const MMGSourceSignal *requestSourceSignal(obs_source_t *source);
	const MMGSourceSignal *requestSourceSignalByName(const MMGUtils::MMGString &name);
	void disconnectAllSignals(QObject *obj);

signals:
	void frontendEvent(obs_frontend_event);
	void hotkeyEvent(obs_hotkey_id);

private:
	QList<MMGSourceSignal *> source_signals;
	bool allow_events = false;

	static void frontendCallback(obs_frontend_event event, void *ptr);
	static void hotkeyCallback(void *ptr, obs_hotkey_id id, bool pressed);
};

MMGSignals *mmgsignals();

#endif // MMG_SIGNAL_H
