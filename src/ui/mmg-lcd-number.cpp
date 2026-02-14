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

#include "mmg-lcd-number.h"

#include <QToolButton>

namespace MMGWidgets {

MMGLCDNumber::MMGLCDNumber(QWidget *parent) : MMGValueWidget(parent)
{
	setFixedHeight(40);

	auto_repeat_timer = new QTimer(this);
	auto_repeat_timer->callOnTimeout(this, &MMGLCDNumber::autoRepeat);

	auto_repeat_hold = new QTimer(this);
	auto_repeat_hold->setSingleShot(true);
	auto_repeat_hold->callOnTimeout(this, &MMGLCDNumber::initAutoRepeat);

	blinking_timer = new QTimer(this);
	blinking_timer->callOnTimeout(this, &MMGLCDNumber::blink);

	main_layout->setContentsMargins(0, 3, 0, 3);
	main_layout->setSpacing(0);
	main_layout->addStretch(1);

	lcd = new QLCDNumber(this);
	lcd->setFixedSize(203, 34);
	lcd->setDigitCount(digitCount());
	lcd->setSmallDecimalPoint(true);
	lcd->setCursor(Qt::PointingHandCursor);
	lcd->setSegmentStyle(QLCDNumber::Flat);
	lcd->setFrameShape(QFrame::NoFrame);
	main_layout->addWidget(lcd);

	QVBoxLayout *buttons_layout = new QVBoxLayout;
	buttons_layout->setContentsMargins(0, 0, 0, 0);
	buttons_layout->setSpacing(0);

	inc = new QToolButton(this);
	inc->setFixedSize(17, 17);
	inc->setStyleSheet("border-radius: 0; "
			   "border-top-left-radius: 3px; "
			   "border-top-right-radius: 3px;");
	inc->setIcon(mmgicon("increase"));
	inc->setIconSize({8, 8});
	connect(inc, &QAbstractButton::pressed, this, &MMGLCDNumber::delayAutoRepeat);
	connect(inc, &QAbstractButton::released, this, &MMGLCDNumber::cancelAutoRepeat);
	buttons_layout->addWidget(inc, 1);

	dec = new QToolButton(this);
	dec->setFixedSize(17, 17);
	dec->setStyleSheet("border-radius: 0; "
			   "border-bottom-left-radius: 3px; "
			   "border-bottom-right-radius: 3px;");
	dec->setIcon(mmgicon("decrease"));
	dec->setIconSize({8, 8});
	connect(dec, &QAbstractButton::pressed, this, &MMGLCDNumber::delayAutoRepeat);
	connect(dec, &QAbstractButton::released, this, &MMGLCDNumber::cancelAutoRepeat);
	buttons_layout->addWidget(dec, 1);

	main_layout->addLayout(buttons_layout);
}

void MMGLCDNumber::setValue(double val)
{
	held_value = val;
	setHeldValue(0);
	/*if (val > _max) {
          _value = _max;
  } else if (val < _min) {
          _value = _min;
  } else if (_step < 1.0) {
          _value = std::round(val / _step) * _step;
  } else {
          _value = val;
  }*/

	_value = held_value;
	// original_value = QString::number(_value, 'g', digitCount());
	emit valueChanged();
	// update();
}

void MMGLCDNumber::setBounds(double lower, double upper)
{
	_min = lower;
	_max = upper;

	lcd->setDigitCount(digitCount());
}

void MMGLCDNumber::display() const
{
	if (_time_format) {
		int min = qAbs(held_value / 60);
		int sec = fmod(qAbs(held_value), 60);
		lcd->display(QString("%1%2:%3").arg(held_value < 0.0 ? "-" : "").arg(min).arg(sec, 2, 10, QChar('0')));
	} else {
		lcd->display(held_value);
	}
}

void MMGLCDNumber::setHeldValue(double inc)
{
	held_value += inc;

	if (held_value > _max) {
		held_value = _max;
	} else if (held_value < _min) {
		held_value = _min;
	} else if (_step < 1.0) {
		held_value = std::round(held_value / _step) * _step;
	}

	original_value = QString::number(held_value, 'g', digitCount());
	display();
}

void MMGLCDNumber::delayAutoRepeat()
{
	auto_repeat_dir = sender() == inc;
	setHeldValue(auto_repeat_dir ? _step : -_step);

	auto_repeat_hold->start(500);
}

void MMGLCDNumber::autoRepeat()
{
	double adj_step = !fmod(held_value, 10.0 * _step) ? _step * 10.0 : _step;
	setHeldValue(auto_repeat_dir ? adj_step : -adj_step);
}

void MMGLCDNumber::cancelAutoRepeat()
{
	auto_repeat_hold->stop();
	auto_repeat_timer->stop();
	setValue(held_value);
}

void MMGLCDNumber::blink()
{
	if (lcd->digitCount() == 0) {
		lcd->setDigitCount(digitCount());
		lcd->display(blink_press ? edit_string : original_value);
	} else {
		lcd->setDigitCount(0);
	}
}

void MMGLCDNumber::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) {
		case Qt::Key_0:
		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		case Qt::Key_6:
		case Qt::Key_7:
		case Qt::Key_8:
		case Qt::Key_9:
			if (edit_string == "0" || edit_string.size() == digitCount())
				edit_string.remove(edit_string.size() - 1, 1);
			else if (edit_string.contains(".") && !edit_string.endsWith(".") &&
				 edit_string.indexOf(".") == edit_string.size() - decimalsAllowed() - 1)
				edit_string.remove(edit_string.size() - 1, 1);

			edit_string += QString::number(event->key() - Qt::Key_0);
			break;

		case Qt::Key_Period:
			if (decimalsAllowed() == 0 || edit_string.contains(".")) break;
			edit_string += ".";
			break;

		case Qt::Key_Minus:
			if (edit_string.contains("-")) {
				edit_string.remove(0, 1);
			} else {
				edit_string.prepend("-");
			}
			break;

		case Qt::Key_Backspace:
			edit_string.remove(edit_string.size() - 1, 1);
			if (edit_string.isEmpty()) edit_string = "0";
			break;

		case Qt::Key_Return:
		case Qt::Key_Enter:
		case Qt::Key_Escape:
			clearFocus();
			event->accept();
			return;

		default:
			event->ignore();
			return;
	}

	if (lcd->digitCount() != 0) lcd->display(edit_string);
	event->accept();
	blink_press = true;
}

bool MMGLCDNumber::focusAcquired()
{
	if (!lcd->underMouse()) return false;

	original_value = QString::number(_value, 'g', digitCount());
	blinking_timer->start(500);
	setFocus();
	blink();

	return true;
}

bool MMGLCDNumber::focusLost()
{
	blinking_timer->stop();
	lcd->setDigitCount(digitCount());
	if (!edit_string.isEmpty()) setValue(edit_string.toDouble());
	edit_string.clear();
	blink_press = false;

	return true;
}

} // namespace MMGWidgets
