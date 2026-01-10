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

#include "mmg-string.h"

namespace MMGSignal {

void initSignals();

class MMGFrontendReceiver;
class MMGSourceReceiver;
class MMGHotkeyReceiver;

void connectMMGSignal(MMGFrontendReceiver *rec, bool connect);
void connectMMGSignal(MMGSourceReceiver *rec, bool connect);
void connectMMGSignal(MMGHotkeyReceiver *rec, bool connect);

class MMGFrontendReceiver {
public:
	virtual ~MMGFrontendReceiver() { connectMMGSignal(this, false); };

	virtual void processEvent(obs_frontend_event event) const = 0;
};

class MMGSourceReceiver {
public:
	virtual ~MMGSourceReceiver() { connectMMGSignal(this, false); };

	virtual MMGString sourceId() const = 0;
	virtual const char *sourceSignalName() const = 0;

	virtual void processEvent(const calldata_t *cd) const = 0;
};

class MMGHotkeyReceiver {
public:
	virtual ~MMGHotkeyReceiver() { connectMMGSignal(this, false); };

	virtual void processEvent(obs_hotkey_id id) const = 0;
};

} // namespace MMGSignal

#endif // MMG_SIGNAL_H
