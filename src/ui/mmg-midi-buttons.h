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

#ifndef MMG_MIDI_BUTTONS_H
#define MMG_MIDI_BUTTONS_H

#include "../mmg-utils.h"

enum MIDIButton {
	MIDIBUTTON_FIXED = 0,
	MIDIBUTTON_MIDI = 1,
	MIDIBUTTON_CUSTOM = 2,
	MIDIBUTTON_IGNORE = 4,
	MIDIBUTTON_TOGGLE = 8
};
Q_DECLARE_FLAGS(MIDIButtons, MIDIButton);
Q_DECLARE_OPERATORS_FOR_FLAGS(MIDIButtons);

class MMGMIDIButtons : public QWidget {
	Q_OBJECT

public:
	MMGMIDIButtons(QWidget *parent);

	int state() const;
	void setOptions(MIDIButtons options);

signals:
	void stateChanged(int);

public slots:
	void setState(int state);

private slots:
	void setToggleIcon(bool toggled);

protected:
	QButtonGroup *button_group;
	QPushButton *fixed_button;
	QPushButton *midi_button;
	QPushButton *custom_button;
	QPushButton *ignore_button;
	QPushButton *toggle_button;
};
#endif // MMG_MIDI_BUTTONS_H
