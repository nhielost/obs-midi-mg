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

#include "mmg-string-display.h"

#include <QAbstractItemView>

using namespace MMGUtils;

MMGStringDisplay::MMGStringDisplay(QWidget *parent) : QWidget(parent)
{
	setVisible(true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	label = new QLabel(this);
	label->setVisible(true);
	label->setWordWrap(true);
	label->setGeometry(0, 0, 150, 40);

	combo_string = new QComboBox(this);
	combo_string->setVisible(true);
	connect(combo_string, &QComboBox::currentTextChanged, this, [&](const QString &val) {
		string->setValue(val);
		emit stringChanged();
	});

	combo_min = new QComboBox(this);
	combo_min->setVisible(false);
	combo_min->setGeometry(160, 0, 170, 40);
	connect(combo_min, &QComboBox::currentTextChanged, this, [&](const QString &val) {
		string->setMin(val);
		if (string->state() == STATE_TOGGLE) string->setValue(val);
		emit stringChanged();
	});

	combo_max = new QComboBox(this);
	combo_max->setVisible(false);
	combo_max->setGeometry(160, 50, 170, 40);
	connect(combo_max, &QComboBox::currentTextChanged, this, [&](const QString &val) {
		string->setMax(val);
		emit stringChanged();
	});

	midi_buttons = new MMGMIDIButtons(this);
	midi_buttons->setVisible(true);
	midi_buttons->move(0, 45);
	connect(midi_buttons, &MMGMIDIButtons::stateChanged, this, &MMGStringDisplay::setComboBoxState);

	setDisplayMode(MODE_THIN);
}

void MMGStringDisplay::setStorage(MMGString *storage, const QStringList &force_bounds)
{
	string = storage;
	if (!string) return;

	if (!force_bounds.isEmpty()) {
		setBounds(force_bounds);
	} else {
		string->setMin(combo_min->currentText());
		string->setMax(combo_max->currentText());
	}
	display();
}

void MMGStringDisplay::setDescription(const QString &desc)
{
	label->setText(desc);
}

void MMGStringDisplay::setBounds(const QStringList &bounds)
{
	_bounds = bounds;

	QSignalBlocker blocker_str(combo_string);
	QSignalBlocker blocker_min(combo_min);
	QSignalBlocker blocker_max(combo_max);

	QString first = bounds.isEmpty() ? "" : bounds.first();
	QString last = bounds.isEmpty() ? "" : bounds.last();

	combo_string->clear();
	combo_min->clear();
	combo_max->clear();

	combo_string->addItems(bounds);
	combo_min->addItems(bounds);
	combo_max->addItems(bounds);

	combo_string->view()->setMinimumWidth(combo_string->minimumSizeHint().width());
	combo_min->view()->setMinimumWidth(combo_min->minimumSizeHint().width());
	combo_max->view()->setMinimumWidth(combo_max->minimumSizeHint().width());

	defaults.setValue(first);
	defaults.setMin(first);
	defaults.setMax(last);

	if (string) {
		if (bounds.contains(*string)) combo_string->setCurrentText(*string);
		string->setValue(combo_string->currentText());
		string->setMin(first);
		string->setMax(last);
		display();
	}
}

void MMGStringDisplay::setOptions(MIDIButtons options)
{
	midi_buttons->setOptions(options);
	midi_buttons->setState(0);
}

void MMGStringDisplay::setComboBoxState(short state)
{
	if (midi_buttons->state() != state) midi_buttons->setState(state);

	if (string) {
		if (string->isEditable() && midi_buttons->isVisible()) {
			combo_string->setCurrentText(defaults.value());
			combo_min->setCurrentText(defaults.min());
			combo_max->setCurrentText(defaults.max());
			*string = defaults;
		}
		string->setState(state);
	}

	combo_string->setVisible(state != 2 && state != 4);
	combo_string->setEnabled(state == 0);
	if (!_bounds.isEmpty()) {
		int idx = combo_string->currentIndex();
		combo_string->setItemText(idx, state == 1 ? QString(mmgtr("Fields.AnyValue")).arg(label->text())
							  : _bounds[idx]);
	}
	combo_min->setVisible(state == 2 || (state == 4 && _bounds.size() > 2));
	combo_max->setVisible(state == 2 || (state == 4 && _bounds.size() > 2));

	emit stringChanged();
}

void MMGStringDisplay::setDisplayMode(Mode mode)
{
	switch (mode) {
		case MODE_NORMAL:
		case MODE_EDITABLE:
			setFixedSize(330, 90);
			combo_string->setGeometry(160, 25, 170, 40);
			midi_buttons->setVisible(true);
			break;

		case MODE_THIN:
			setFixedSize(330, 40);
			combo_string->setGeometry(160, 0, 170, 40);
			setComboBoxState(0);
			midi_buttons->setVisible(false);
			break;
	}

	combo_string->setEditable(mode == MODE_EDITABLE);
	combo_min->setEditable(mode == MODE_EDITABLE);
	combo_max->setEditable(mode == MODE_EDITABLE);
}

void MMGStringDisplay::display()
{
	combo_string->setCurrentText(string->value());
	combo_min->setCurrentText(string->min());
	combo_max->setCurrentText(string->max());
	setComboBoxState(string->state());
}

void MMGStringDisplay::reset()
{
	if (string) *string = defaults.value();
	display();
}
