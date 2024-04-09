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

#include "mmg-number-display.h"

using namespace MMGUtils;

MMGNumberDisplay::MMGNumberDisplay(QWidget *parent) : QWidget(parent)
{
	setVisible(true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	lcd_number = new MMGLCDNumber(this);
	lcd_number->setVisible(true);
	connect(lcd_number, &MMGLCDNumber::numberChanged, this, [&](double val) {
		number->setValue(val);
		emit numberChanged();
	});

	lcd_min = new MMGLCDNumber(this);
	lcd_min->setVisible(false);
	lcd_min->setGeometry(155, 0, 175, 30);
	connect(lcd_min, &MMGLCDNumber::numberChanged, this, [&](double val) {
		number->setMin(val);
		if (number->state() == STATE_TOGGLE) number->setValue(val);
		emit numberChanged();
	});

	lcd_max = new MMGLCDNumber(this);
	lcd_max->setVisible(false);
	lcd_max->setGeometry(155, 40, 175, 30);
	connect(lcd_max, &MMGLCDNumber::numberChanged, this, [&](double val) {
		number->setMax(val);
		emit numberChanged();
	});

	label = new QLabel(this);
	label->setVisible(true);
	label->setWordWrap(true);
	label->setGeometry(0, 0, 150, 30);

	midi_buttons = new MMGMIDIButtons(this);
	midi_buttons->setVisible(true);
	midi_buttons->move(0, 34);
	connect(midi_buttons, &MMGMIDIButtons::stateChanged, this, &MMGNumberDisplay::setLCDState);

	setDisplayMode(MODE_NORMAL);
}

void MMGNumberDisplay::setStorage(MMGNumber *storage, bool force_values)
{
	number = storage;
	if (!number) return;

	if (force_values) {
		setBounds(number->min(), number->max());
	} else {
		number->setMin(lcd_min->value());
		number->setMax(lcd_max->value());
	}
	display();
}

void MMGNumberDisplay::setDescription(const QString &desc)
{
	label->setText(desc);
}

void MMGNumberDisplay::setBounds(double lower, double upper, bool extend_bounds)
{
	if (defaults.min() == lower && defaults.max() == upper) return;

	if (extend_bounds) {
		double range = upper - lower;
		lcd_number->setBounds(lower - range, upper + range);
		lcd_min->setBounds(lower - range, upper + range);
		lcd_max->setBounds(lower - range, upper + range);
	} else {
		lcd_number->setBounds(lower, upper);
		lcd_min->setBounds(lower, upper);
		lcd_max->setBounds(lower, upper);
	}

	defaults.setValue(lower);
	defaults.setMin(lower);
	defaults.setMax(upper);

	if (number) {
		number->setMin(lower);
		number->setMax(upper);
		display();
	}
}

void MMGNumberDisplay::setOptions(MIDIButtons options)
{
	if (options == MIDIBUTTON_FIXED) {
		setDisplayMode(MODE_THIN);
	} else {
		setDisplayMode(MODE_NORMAL);
		midi_buttons->setOptions(options);
		midi_buttons->setState(0);
	}
}

void MMGNumberDisplay::setStep(double inc)
{
	lcd_number->setStep(inc < 0.01 ? 0.01 : inc);
	lcd_min->setStep(inc < 0.01 ? 0.01 : inc);
	lcd_max->setStep(inc < 0.01 ? 0.01 : inc);
}

void MMGNumberDisplay::setTimeFormat(bool format)
{
	lcd_number->setTimeFormat(format);
	lcd_min->setTimeFormat(format);
	lcd_max->setTimeFormat(format);
}

void MMGNumberDisplay::setLCDState(short state)
{
	if (midi_buttons->state() != state) midi_buttons->setState(state);

	if (number) {
		if (number->isEditable() && midi_buttons->isVisible()) {
			lcd_number->setValue(defaults.value());
			lcd_min->setValue(defaults.min());
			lcd_max->setValue(defaults.max());
			*number = defaults;
		}
		number->setState(state);
	}

	lcd_number->setVisible(state != 2 && state != 4);
	lcd_number->setEnabled(state == 0);
	lcd_number->display((MMGLCDNumber::State)(state > 2 ? 2 : state));
	lcd_min->setVisible(state == 2 || state == 4);
	lcd_max->setVisible(state == 2 || state == 4);

	emit numberChanged();
}

void MMGNumberDisplay::setDisplayMode(Mode mode)
{
	switch (mode) {
		case MODE_NORMAL:
			setFixedSize(330, 70);
			lcd_number->setGeometry(155, 20, 175, 30);
			midi_buttons->setVisible(true);
			break;

		case MODE_THIN:
			setFixedSize(330, 30);
			lcd_number->setGeometry(155, 0, 175, 30);
			setLCDState(0);
			midi_buttons->setVisible(false);
			break;
	}
}

void MMGNumberDisplay::display()
{
	lcd_number->setValue(number->value());
	lcd_min->setValue(number->min());
	lcd_max->setValue(number->max());
	setLCDState(number->state());
}

void MMGNumberDisplay::reset()
{
	if (number) *number = defaults.value();
	display();
}