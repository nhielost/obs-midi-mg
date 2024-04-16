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

#include "mmg-lcd-number.h"
#include "../mmg-utils.h"

#include <QKeyEvent>

using namespace MMGUtils;

MMGLCDNumber::MMGLCDNumber(QWidget *parent) : QWidget(parent)
{
	lcd = new QLCDNumber(this);
	inc_button = new QToolButton(this);
	dec_button = new QToolButton(this);
	auto_repeat_timer = new QTimer(this);
	repeat_delay_timer = new QTimer(this);
	repeat_delay_timer->setSingleShot(true);
	blinking_timer = new QTimer(this);

	connect(auto_repeat_timer, &QTimer::timeout, this, &MMGLCDNumber::autoRepeat);
	connect(repeat_delay_timer, &QTimer::timeout, this, &MMGLCDNumber::initAutoRepeat);
	connect(blinking_timer, &QTimer::timeout, this, &MMGLCDNumber::blink);

	lcd->setDigitCount(8);
	lcd->setSmallDecimalPoint(true);
	QPalette p = parent->palette();
	p.setColor(QPalette::Dark, QColor("#ffffff"));
	lcd->setPalette(p);
	lcd->setSegmentStyle(QLCDNumber::Filled);
	lcd->setFrameShape(QFrame::NoFrame);
	lcd->setFrameShadow(QFrame::Raised);
	lcd->setLineWidth(0);
	lcd->setVisible(true);
	lcd->setGeometry(0, 0, 160, 30);

	inc_button->setVisible(true);
	inc_button->setGeometry(160, 0, 15, 15);
	inc_button->setStyleSheet("padding: 0; "
				  "border-radius: 0; "
				  "border-top-left-radius: 3px; "
				  "border-top-right-radius: 3px;");
	inc_button->setIcon(MMGInterface::icon("increase"));
	inc_button->setIconSize(QSize(7, 7));
	connect(inc_button, &QAbstractButton::pressed, this, &MMGLCDNumber::delayAutoRepeatInc);
	connect(inc_button, &QAbstractButton::released, this, &MMGLCDNumber::autoRepeatCancel);

	dec_button->setVisible(true);
	dec_button->setGeometry(160, 15, 15, 15);
	dec_button->setStyleSheet("padding: 0; "
				  "border-radius: 0; "
				  "border-bottom-left-radius: 3px; "
				  "border-bottom-right-radius: 3px;");
	dec_button->setIcon(MMGInterface::icon("decrease"));
	dec_button->setIconSize(QSize(7, 7));
	connect(dec_button, &QAbstractButton::pressed, this, &MMGLCDNumber::delayAutoRepeatDec);
	connect(dec_button, &QAbstractButton::released, this, &MMGLCDNumber::autoRepeatCancel);
}

void MMGLCDNumber::delayAutoRepeatInc()
{
	timer_kind = true;
	repeat_delay_timer->start(500);
	inc();
}

void MMGLCDNumber::delayAutoRepeatDec()
{
	timer_kind = false;
	repeat_delay_timer->start(500);
	dec();
}

void MMGLCDNumber::initAutoRepeat()
{
	auto_repeat_timer->start(40);
}

void MMGLCDNumber::autoRepeat()
{
	if (!fmod(_value, 10 * _step)) step_mult = 10;
	timer_kind ? inc() : dec();
}

void MMGLCDNumber::autoRepeatCancel()
{
	auto_repeat_timer->stop();
	repeat_delay_timer->stop();
	step_mult = 1;
}

void MMGLCDNumber::blink()
{
	if (lcd->digitCount() == 0) {
		lcd->setDigitCount(8);
		lcd->display(blink_press ? edit_string : original_value);
	} else {
		lcd->setDigitCount(0);
	}
}

void MMGLCDNumber::setBounds(double lower, double upper)
{
	_min = lower;
	_max = upper;
}

void MMGLCDNumber::setStep(double step)
{
	_step = step;
}

void MMGLCDNumber::setValue(double val)
{
	if (!editable) return;
	if (val > _max) {
		_value = _max;
	} else if (val < _min) {
		_value = _min;
	} else if (_step < 1 && qAbs(val - qFloor(val)) > 0 && qAbs(val - qFloor(val)) < adj_step()) {
		_value = qRound(val / _step) * _step;
	} else {
		_value = val;
	}
	emit numberChanged(_value);
	display(LCDNUMBER_ACTIVE);
}

void MMGLCDNumber::display(State state)
{
	switch (state) {
		case LCDNUMBER_ACTIVE:
			if (_time_format) {
				lcd->display(QTime(_value / 3600.0, fmod(_value / 60.0, 60.0), fmod(_value, 60.0))
						     .toString("hh:mm:ss"));
			} else {
				lcd->display(_value);
			}
			break;

		case LCDNUMBER_0_127:
			lcd->display("  0-127 ");
			break;

		case LCDNUMBER_INACTIVE:
			lcd->display("  ----- ");
			break;

		default:
			break;
	}
}

void MMGLCDNumber::wheelEvent(QWheelEvent *event)
{
	if (blinking_timer->isActive()) {
		event->ignore();
		return;
	}
	if (event->hasPixelDelta()) {
		event->pixelDelta().y() > 0 ? inc() : dec();
	} else if (!event->angleDelta().isNull()) {
		event->angleDelta().y() > 0 ? inc() : dec();
	}
	event->accept();
}

void MMGLCDNumber::mouseDoubleClickEvent(QMouseEvent *event)
{
	original_value = QString::number(_value);
	blinking_timer->start(500);
	setFocus();
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
			if (edit_string == "0") edit_string.clear();
			if (edit_string.size() == 8) break;
			edit_string += QString::number(event->key() - Qt::Key_0);
			break;

		case Qt::Key_Period:
			if (_step >= 1 || edit_string.contains(".")) break;
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

void MMGLCDNumber::focusOutEvent(QFocusEvent *event)
{
	blinking_timer->stop();
	lcd->setDigitCount(8);
	setValue(edit_string.toDouble());
	edit_string.clear();
	blink_press = false;
}