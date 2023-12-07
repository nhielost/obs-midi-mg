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

#ifndef MMG_BINDING_DISPLAY_H
#define MMG_BINDING_DISPLAY_H

#include "../mmg-binding.h"

#include <QScrollArea>

class MMGBindingDisplay : public QWidget {
	Q_OBJECT

public:
	MMGBindingDisplay(QWidget *parent, bool executable);

	void setStorage(MMGBinding *binding);
	void display();

signals:
	void edited(int page);

private slots:
	void emitEdited();

private:
	bool _executable = true;
	MMGBinding *current_binding = nullptr;

	QVBoxLayout *head_layout;

	QWidget *devices_widget;
	QWidget *messages_widget;
	QWidget *actions_widget;
	QWidget *settings_widget;

	QLabel *devices_label;
	QLabel *messages_label;
	QLabel *actions_label;
	QLabel *settings_label;

	QWidget *current_devices;
	QWidget *current_messages;
	QWidget *current_actions;
	QWidget *current_settings;
};

#endif // MMG_BINDING_DISPLAY_H
