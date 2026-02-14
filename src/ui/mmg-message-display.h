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

#ifndef MMG_MESSAGE_DISPLAY_H
#define MMG_MESSAGE_DISPLAY_H

#include "../messages/mmg-message.h"
#include "mmg-value-manager.h"

namespace MMGWidgets {

class MMGMessageDisplay : public MMGValueManager, public MMGMessageReceiver {
	Q_OBJECT

public:
	MMGMessageDisplay(QWidget *parent, MMGStateDisplay *state_display);

	MMGMessage *storage() const { return _storage; };
	void setStorage(DeviceType message_type, MMGMessageManager *parent, MMGMessage *storage);

	void resetListening();

signals:
	void messageChanged();

private:
	void setDevice();
	void setType();
	void resetMessage();

	void connectDevice(bool);
	void onListenClick();
	void processMessage(const MMGMessageData &) override;

private:
	MMGMessageManager *_parent = nullptr;
	MMGMessage *_storage = nullptr;

	MMGValue<MMGMIDIPort *> _device;
	MMGValue<MMGMessages::Id> _id;

	QPushButton *listen_button;
	short listening_mode = 0;

	static MMGParams<MMGMIDIPort *> device_params;
	static MMGParams<MMGMessages::Id> id_params;
};

#undef MMG_HAS_STORAGE

} // namespace MMGWidgets

#endif // MMG_MESSAGE_DISPLAY_H
