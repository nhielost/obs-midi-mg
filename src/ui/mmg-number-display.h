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

#ifndef MMG_NUMBER_DISPLAY_H
#define MMG_NUMBER_DISPLAY_H

#include "../mmg-utils.h"
#include "mmg-lcd-number.h"
#include "mmg-midi-buttons.h"

class MMGNumberDisplay : public QWidget {
	Q_OBJECT

public:
	MMGNumberDisplay(QWidget *parent);

	enum Mode { MODE_NORMAL, MODE_THIN };

	void setDisplayMode(Mode mode);
	void setStorage(MMGUtils::MMGNumber *storage, bool force_values = false);

	void setDescription(const QString &desc);
	void setOptions(MIDIButtons options);
	void setBounds(double lower, double upper, bool extend_bounds = false);
	void setStep(double inc);
	void setDefaultValue(double val) { defaults = val; };
	void setTimeFormat(bool format);

	void display();
	void reset();

signals:
	void numberChanged();

public slots:
	void setLCDState(short state);

protected:
	MMGUtils::MMGNumber *number = nullptr;
	MMGUtils::MMGNumber defaults;

	QLabel *label;
	MMGLCDNumber *lcd_number;
	MMGLCDNumber *lcd_min;
	MMGLCDNumber *lcd_max;

	MMGMIDIButtons *midi_buttons;
};

#endif // MMG_NUMBER_DISPLAY_H
