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

#include "mmg-midi-buttons.h"

#include <QButtonGroup>

using namespace MMGUtils;

MMGMIDIButtons::MMGMIDIButtons(QWidget *parent) : QWidget(parent)
{
	setVisible(true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	setFixedSize(150, 33);

	auto layout = new QHBoxLayout(this);
	layout->setSpacing(10);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	layout->setContentsMargins(0, 0, 0, 0);

	fixed_button = new QPushButton(this);
	fixed_button->setIcon(mmg_icon("fixed"));
	fixed_button->setIconSize({12, 12});
	fixed_button->setToolTip(mmgtr("MIDIButtons.Fixed"));

	midi_button = new QPushButton(this);
	midi_button->setIcon(mmg_icon("midi"));
	midi_button->setIconSize({18, 18});
	midi_button->setToolTip(mmgtr("MIDIButtons.MIDI"));

	custom_button = new QPushButton(this);
	custom_button->setIcon(mmg_icon("custom"));
	custom_button->setToolTip(mmgtr("MIDIButtons.Custom"));

	ignore_button = new QPushButton(this);
	ignore_button->setIcon(mmg_icon("disable"));
	ignore_button->setToolTip(mmgtr("MIDIButtons.Ignore"));

	toggle_button = new QPushButton(this);
	toggle_button->setToolTip(mmgtr("MIDIButtons.Toggle"));
	connect(toggle_button, &QPushButton::toggled, this, &MMGMIDIButtons::setToggleIcon);

	button_group = new QButtonGroup(this);
	button_group->setExclusive(true);
	button_group->addButton(fixed_button, 0);
	button_group->addButton(midi_button, 1);
	button_group->addButton(custom_button, 2);
	button_group->addButton(ignore_button, 3);
	button_group->addButton(toggle_button, 4);
	connect(button_group, &QButtonGroup::idClicked, this, &MMGMIDIButtons::setState);

	for (QAbstractButton *button : button_group->buttons()) {
		button->setFixedSize(30, 30);
		button->setCheckable(true);
		button->setCursor(Qt::PointingHandCursor);
		layout->addWidget(button);
	}
	fixed_button->setChecked(true);

	setLayout(layout);
}

int MMGMIDIButtons::state() const
{
	return button_group->checkedId();
}

void MMGMIDIButtons::setOptions(MIDIButtons options)
{
	fixed_button->setVisible(false);
	midi_button->setVisible(false);
	custom_button->setVisible(false);
	ignore_button->setVisible(false);
	toggle_button->setVisible(false);

	if (!options) return;
	if (options & MIDIBUTTON_MIDI) midi_button->setVisible(true);
	if (options & MIDIBUTTON_CUSTOM) custom_button->setVisible(true);
	if (options & MIDIBUTTON_IGNORE) ignore_button->setVisible(true);
	if (options & MIDIBUTTON_TOGGLE) toggle_button->setVisible(true);
	fixed_button->setVisible(true);
}

void MMGMIDIButtons::setState(int state)
{
	if (state > 4) return;
	if (button_group->checkedId() != state) button_group->button(state)->setChecked(true);
	setToggleIcon(state == 4);

	emit stateChanged(state);
}

void MMGMIDIButtons::setToggleIcon(bool toggled)
{
	toggle_button->setIcon(mmg_icon(toggled ? "toggle-on" : "toggle-off"));
}
