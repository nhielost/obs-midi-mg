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

#ifndef MMG_LCD_NUMBER_H
#define MMG_LCD_NUMBER_H

#include "mmg-value-widget.h"

#include <QKeyEvent>
#include <QLCDNumber>
#include <QTimer>
#include <QToolButton>

namespace MMGWidgets {

class MMGLCDNumber : public MMGValueWidget {
	Q_OBJECT

public:
	MMGLCDNumber(QWidget *parent = nullptr);
	~MMGLCDNumber() = default;

	double value() const { return _value; };
	double min() const { return _min; };
	double max() const { return _max; };
	double step() const { return _step; };

	void setValue(double val);
	void setBounds(double lower, double upper);
	void setStep(double step) { _step = step; };

	void setTimeFormat(bool time_format) { _time_format = time_format; };
	void setIncrementing() { incrementing = true; };

	void setDefaultValue(double default_val) { default_value = default_val; };
	void reset() override { setValue(default_value); };

	template <typename T> requires MMGIsNumeric<T> void setParams(const MMGParams<T> &params)
	{
		double lower_bound = incrementing ? -params.incremental_bound : params.lower_bound;
		double upper_bound = incrementing ? +params.incremental_bound : params.upper_bound;

		setDefaultValue(incrementing ? 0 : params.default_value);
		setBounds(lower_bound, upper_bound);
		setStep(params.step);
		setTimeFormat(params.options & OPTION_SPECIAL_1);
	};

protected:
	void keyPressEvent(QKeyEvent *) override;

	bool focusAcquired() override;
	bool focusLost() override;

public slots:
	void display() const override;

private:
	void setHeldValue(double inc);
	void delayAutoRepeat();
	void initAutoRepeat() { auto_repeat_timer->start(40); };
	void autoRepeat();
	void cancelAutoRepeat();

	void blink();

	short digitCount() const { return 11; };
	qsizetype decimalsAllowed() const { return qCeil(-std::log10(_step)); };

private:
	QLCDNumber *lcd;
	QToolButton *inc;
	QToolButton *dec;

	double _value = 0.0;
	double held_value = 0.0;
	double default_value = 0.0;
	double _min = 0.0;
	double _max = 100.0;
	double _step = 1.0;

	bool incrementing = false;
	bool _time_format = false;

	bool blink_press = false;
	bool auto_repeat_dir = false;
	QTimer *auto_repeat_timer;
	QTimer *auto_repeat_hold;

	QTimer *blinking_timer;
	QString edit_string;
	QString original_value;
};

} // namespace MMGWidgets

#endif // MMG_LCD_NUMBER_H
