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

#include "mmg-signal.h"
#include "mmg-config.h"

#include <QMessageBox>

namespace MMGSignal {

struct SourceIdentifier {
	MMGString uuid;
	const char *signal_name;
};

static QList<MMGFrontendReceiver *> frontend_recs;
static QMap<MMGSourceReceiver *, SourceIdentifier> source_recs;
static QList<MMGHotkeyReceiver *> hotkey_recs;
static bool signal_init = false;
static bool allow_events = false;

void frontendCallback(obs_frontend_event event, void *)
{
	switch (event) {
		case OBS_FRONTEND_EVENT_FINISHED_LOADING:
			config()->finishLoad();
			allow_events = true;
			break;

		case OBS_FRONTEND_EVENT_EXIT:
			obs_frontend_remove_event_callback(frontendCallback, nullptr);
			break;

		default:
			if (!allow_events) break;
			for (MMGFrontendReceiver *rec : frontend_recs)
				rec->processEvent(event);
			break;
	}
}

void sourceCallback(void *ptr, calldata_t *cd)
{
	MMGSourceReceiver *rec = static_cast<MMGSourceReceiver *>(ptr);
	if (!!rec) rec->processEvent(cd);
}

void destroyCallback(void *ptr, calldata_t *)
{
	MMGSourceReceiver *rec = static_cast<MMGSourceReceiver *>(ptr);
	if (!!rec) source_recs.remove(rec);
}

void hotkeyCallback(void *, obs_hotkey_id id, bool pressed)
{
	QMetaObject::invokeMethod(static_cast<QObject *>(obs_frontend_get_main_window()), "ProcessHotkey",
				  Q_ARG(obs_hotkey_id, id), Q_ARG(bool, pressed));

	if (!pressed) return;

	for (MMGHotkeyReceiver *rec : hotkey_recs)
		rec->processEvent(id);
}

void disconnectSource(MMGSourceReceiver *rec)
{
	if (!source_recs.contains(rec)) return;
	SourceIdentifier source_id = source_recs[rec];

	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(source_id.uuid);
	if (!obs_source) return;

	signal_handler_t *sh = obs_source_get_signal_handler(obs_source);
	signal_handler_disconnect(sh, source_id.signal_name, sourceCallback, rec);

	source_recs.remove(rec);
}

void connectSource(MMGSourceReceiver *rec)
{
	if (!rec || !rec->sourceId()) return;
	if (std::strcmp(rec->sourceSignalName(), "n/a") == 0) return;

	OBSSourceAutoRelease obs_source = obs_get_source_by_uuid(rec->sourceId());
	if (!obs_source) return;

	disconnectSource(rec);
	source_recs.insert(rec, {rec->sourceId(), rec->sourceSignalName()});

	signal_handler_t *sh = obs_source_get_signal_handler(obs_source);
	signal_handler_connect(sh, rec->sourceSignalName(), sourceCallback, rec);
	signal_handler_connect(sh, "destroy", destroyCallback, rec);
}

void connectMMGSignal(MMGFrontendReceiver *rec, bool connect)
{
	if (connect) {
		if (!frontend_recs.contains(rec)) frontend_recs += rec;
	} else {
		frontend_recs.removeOne(rec);
	}
}

void connectMMGSignal(MMGSourceReceiver *rec, bool connect)
{
	if (connect) {
		connectSource(rec);
	} else {
		disconnectSource(rec);
	}
}

void connectMMGSignal(MMGHotkeyReceiver *rec, bool connect)
{
	if (connect) {
		if (!hotkey_recs.contains(rec)) hotkey_recs += rec;
	} else {
		hotkey_recs.removeOne(rec);
	}
}

void initSignals()
{
	if (signal_init) return;
	signal_init = true;

	obs_frontend_add_event_callback(frontendCallback, nullptr);
	obs_hotkey_enable_callback_rerouting(true);
	obs_hotkey_set_callback_routing_func(hotkeyCallback, nullptr);
}

} // namespace MMGSignal
