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

#include "mmg-state-display.h"
#include "mmg-state-widget.h"

namespace MMGWidgets {

MMGStateDisplay::MMGStateDisplay(QWidget *parent) : QWidget(parent)
{
	main_layout = new QVBoxLayout;
	main_layout->setSpacing(0);
	main_layout->setContentsMargins(0, 0, 0, 0);

	states_group = new QButtonGroup(this);
	connect(states_group, &QButtonGroup::idClicked, this, &MMGStateDisplay::setButtonState);

	QVBoxLayout *top_layout = new QVBoxLayout;
	top_layout->setContentsMargins(10, 10, 10, 10);
	top_layout->setSpacing(10);

	QHBoxLayout *buttons_layout = new QHBoxLayout;
	buttons_layout->setContentsMargins(10, 10, 10, 10);
	buttons_layout->setSpacing(10);

	QPushButton *fixed_button = new QPushButton(this);
	fixed_button->setFixedSize(40, 40);
	fixed_button->setCheckable(true);
	fixed_button->setCursor(Qt::PointingHandCursor);
	setStateIconAndToolTip(fixed_button, STATE_FIXED);
	states_group->addButton(fixed_button, STATE_FIXED);
	buttons_layout->addWidget(fixed_button);

	QPushButton *midi_button = new QPushButton(this);
	midi_button->setFixedSize(40, 40);
	midi_button->setCheckable(true);
	midi_button->setCursor(Qt::PointingHandCursor);
	setStateIconAndToolTip(midi_button, STATE_MIDI);
	states_group->addButton(midi_button, STATE_MIDI);
	buttons_layout->addWidget(midi_button);

	QPushButton *range_button = new QPushButton(this);
	range_button->setFixedSize(40, 40);
	range_button->setCheckable(true);
	range_button->setCursor(Qt::PointingHandCursor);
	setStateIconAndToolTip(range_button, STATE_RANGE);
	states_group->addButton(range_button, STATE_RANGE);
	buttons_layout->addWidget(range_button);

	QPushButton *toggle_button = new QPushButton(this);
	toggle_button->setFixedSize(40, 40);
	toggle_button->setCheckable(true);
	toggle_button->setCursor(Qt::PointingHandCursor);
	setStateIconAndToolTip(toggle_button, STATE_TOGGLE);
	states_group->addButton(toggle_button, STATE_TOGGLE);
	buttons_layout->addWidget(toggle_button);

	QPushButton *increment_button = new QPushButton(this);
	increment_button->setFixedSize(40, 40);
	increment_button->setCheckable(true);
	increment_button->setCursor(Qt::PointingHandCursor);
	setStateIconAndToolTip(increment_button, STATE_INCREMENT);
	states_group->addButton(increment_button, STATE_INCREMENT);
	buttons_layout->addWidget(increment_button);

	buttons_layout->addStretch(1);

	QPushButton *ignore_button = new QPushButton(this);
	ignore_button->setFixedSize(40, 40);
	ignore_button->setCheckable(true);
	ignore_button->setCursor(Qt::PointingHandCursor);
	setStateIconAndToolTip(ignore_button, STATE_IGNORE);
	states_group->addButton(ignore_button, STATE_IGNORE);
	buttons_layout->addWidget(ignore_button);

	top_layout->addLayout(buttons_layout);

	states_desc = new QLabel(this);
	states_desc->setAlignment(Qt::AlignCenter);
	states_desc->setWordWrap(true);
	top_layout->addWidget(states_desc);

	main_layout->addLayout(top_layout);

	QFrame *sep_upper = new QFrame(this);
	sep_upper->setObjectName("sep");
	sep_upper->setFrameShape(QFrame::HLine);
	sep_upper->setFixedHeight(1);
	main_layout->addWidget(sep_upper, 0, Qt::AlignTop);

	scroll_area = new QScrollArea(this);
	scroll_area->setWidgetResizable(true);
	scroll_area->setFrameShape(QFrame::NoFrame);
	scroll_area->verticalScrollBar()->setSingleStep(35);
	scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	createScrollBarOverlay(scroll_area);
	main_layout->addWidget(scroll_area, 1);

	sep_lower = new QFrame(this);
	sep_lower->setObjectName("sep");
	sep_lower->setFrameShape(QFrame::HLine);
	sep_lower->setFixedHeight(1);
	main_layout->addWidget(sep_lower);

	setLayout(main_layout);
}

void MMGStateDisplay::refreshStorage()
{
	if (!!state_widget) state_widget->refresh();
}

void MMGStateDisplay::clearStorage()
{
	setVisible(false);
	constructor.reset();

	if (!!state_widget) {
		state_widget->deleteLater();
		state_widget = nullptr;
		scroll_area->setWidget(nullptr);
	}
}

void MMGStateDisplay::applyReferences(DeviceType desired_type, DeviceType current_type)
{
	if (desired_type == current_type) {
		used_refs = *(desired_type == TYPE_OUTPUT ? action_refs : message_refs);
	} else {
		used_refs = MMGValueStateInfoList({nullptr});
	}
}

void MMGStateDisplay::setButtonState(uint16_t state)
{
	if (!states_group->button(state)->isChecked()) states_group->button(state)->setChecked(true);
	states_group->button(STATE_TOGGLE)
		->setIcon(state == STATE_TOGGLE ? mmgicon("toggle-on") : mmgicon("toggle-off"));

	auto state_text_prefix = MMGText::join("MIDIButtons", getStateName(ValueState(state)));
	auto state_text = MMGText::choose(state_text_prefix, "InputDescription", "OutputDescription",
					  used_refs.contains(nullptr));
	states_desc->setText(mmgtr(state_text));

	state_widget = (*constructor)((ValueState)state);

	scroll_area->setWidget(state_widget);
	scroll_area->show();

	QWidget *bottom_widget = main_layout->itemAt(main_layout->count() - 1)->widget();
	if (bottom_widget != sep_lower) {
		main_layout->removeWidget(bottom_widget);
		bottom_widget->deleteLater();
	}
	sep_lower->setVisible(!!state_widget->bottomWidget());
	if (!!state_widget->bottomWidget()) main_layout->addWidget(state_widget->bottomWidget());
}

} // namespace MMGWidgets
