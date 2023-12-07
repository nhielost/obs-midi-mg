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

#ifndef MMG_SIGNAL_H
#define MMG_SIGNAL_H

#include "obs-midi-mg.h"

class MMGSourceSignal : public QObject {
	Q_OBJECT

public:
	MMGSourceSignal(QObject *parent, obs_source_t *source);

	bool match(const obs_source_t *source) const { return _source == source; };
	bool valid() const { return !!_source; };

	void disconnectSignals();

signals:
	void sourceUpdated(void *);

	void sourceVisibilityChanged(void *, bool);
	void sourceLocked(void *, bool);
	void sourceTransformed(void *);

	void sourceMuted(bool);
	void sourceVolumeChanged(double);
	void sourceSyncOffsetChanged(int64_t);
	void sourceMonitoringChanged(int);

	void mediaPlayed();
	void mediaPaused();
	void mediaRestarted();
	void mediaStopped();
	void mediaPreviousChange();
	void mediaNextChange();

	void filterEnabled(bool);
	void filterReordered();

	void transitionStarted();
	void transitionStopped();

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
	MMGSignals(QObject *parent = nullptr);

	void enableHotkeyCallbacks(bool enabled);
	const MMGSourceSignal *sourceSignal(obs_source_t *source);

signals:
	void frontendEvent(obs_frontend_event);
	void hotkeyEvent(obs_hotkey_id);

private:
	QList<MMGSourceSignal *> source_signals;

	static void frontendCallback(obs_frontend_event event, void *ptr);
	static void hotkeyCallback(void *ptr, obs_hotkey_id id, bool pressed);

private slots:
	void shutdownCallback(obs_frontend_event event);
};

MMGSignals *mmgsignals();

#endif // MMG_SIGNAL_H
