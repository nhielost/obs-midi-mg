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

#include "mmg-lcd-number.h"
#include "../mmg-utils.h"

#include <QKeyEvent>

using namespace MMGUtils;

MMGLCDNumber::MMGLCDNumber(QWidget *parent) : QWidget(parent)
{
	_step = 1.0;

	lcd = new QLCDNumber(this);
	inc_button = new QToolButton(this);
	dec_button = new QToolButton(this);
	auto_repeat_timer = new QTimer(this);
	repeat_delay_timer = new QTimer(this);
	repeat_delay_timer->setSingleShot(true);

	connect(auto_repeat_timer, &QTimer::timeout, this, &MMGLCDNumber::autoRepeat);
	connect(repeat_delay_timer, &QTimer::timeout, this, &MMGLCDNumber::initAutoRepeat);

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
	inc_button->setIcon(QIcon(":/icons/increase.svg"));
	inc_button->setIconSize(QSize(7, 7));
	connect(inc_button, &QAbstractButton::pressed, this, &MMGLCDNumber::delayAutoRepeatInc);
	connect(inc_button, &QAbstractButton::released, this, &MMGLCDNumber::autoRepeatCancel);

	dec_button->setVisible(true);
	dec_button->setGeometry(160, 15, 15, 15);
	dec_button->setStyleSheet("padding: 0; "
				  "border-radius: 0; "
				  "border-bottom-left-radius: 3px; "
				  "border-bottom-right-radius: 3px;");
	dec_button->setIcon(QIcon(":/icons/decrease.svg"));
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
	if (event->hasPixelDelta()) {
		event->pixelDelta().y() > 0 ? inc() : dec();
	} else if (!event->angleDelta().isNull()) {
		event->angleDelta().y() > 0 ? inc() : dec();
	} else {
		event->ignore();
	}
	event->accept();
}
