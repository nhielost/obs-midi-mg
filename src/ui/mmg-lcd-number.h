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

#pragma once
#include <QToolButton>
#include <QLCDNumber>
#include <QTimer>
#include <QWheelEvent>
#include <QKeyEvent>

class MMGLCDNumber : public QWidget {
	Q_OBJECT

public:
	MMGLCDNumber(QWidget *parent = nullptr);
	~MMGLCDNumber() = default;

	enum State { LCDNUMBER_ACTIVE, LCDNUMBER_0_127, LCDNUMBER_INACTIVE };

	double value() const { return _value; };
	double min() const { return _min; };
	double max() const { return _max; };
	double step() const { return _step; };

	void setEditable(bool edit) { editable = edit; };
	void setBounds(double lower, double upper);
	void setStep(double step);
	void setTimeFormat(bool time_format) { _time_format = time_format; };

	void setValue(double val);
	void display(State state);

protected:
	void wheelEvent(QWheelEvent *) override;

signals:
	void numberChanged(double);

private:
	QLCDNumber *lcd;
	QToolButton *inc_button;
	QToolButton *dec_button;

	double _value = 0.0;
	bool editable = true;

	double _min = 0.0;
	double _max = 100.0;
	double _step = 1.0;

	QTimer *repeat_delay_timer;
	QTimer *auto_repeat_timer;
	bool timer_kind = false;
	double step_mult = 1;

	bool _time_format = false;

	double adj_step() const { return _step * step_mult; };

private slots:
	void inc() { setValue(_value + adj_step()); };
	void dec() { setValue(_value - adj_step()); };

	void delayAutoRepeatInc();
	void delayAutoRepeatDec();
	void initAutoRepeat();
	void autoRepeat();
	void autoRepeatCancel();
};
