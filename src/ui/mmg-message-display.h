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

#ifndef MMG_MESSAGE_DISPLAY_H
#define MMG_MESSAGE_DISPLAY_H

#include "../mmg-utils.h"
#include "../mmg-config.h"

class MMGMessageDisplay : public QWidget {
	Q_OBJECT

public:
	MMGMessageDisplay(QWidget *parent);

	void setStorage(MMGMessage *storage);
	void connectDevice(bool);

public slots:
	void setLabels();
	void setDevice();
	void onListenClick();

	void deactivate();
	void updateMessage(const MMGSharedMessage &);

private:
	MMGMessage *_storage = nullptr;
	MMGUtils::MMGString _device;

	MMGStringDisplay *device_display;
	MMGStringDisplay *type_display;
	MMGNumberDisplay *channel_display;
	MMGNumberDisplay *note_display;
	MMGNumberDisplay *value_display;

	QPushButton *listen_button;
	short listening_mode = 0;
};

#endif // MMG_MESSAGE_DISPLAY_H
