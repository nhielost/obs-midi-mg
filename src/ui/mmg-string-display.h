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

#ifndef MMG_STRING_DISPLAY_H
#define MMG_STRING_DISPLAY_H

#include "../mmg-utils.h"
#include "mmg-midi-buttons.h"

class MMGStringDisplay : public QWidget {
	Q_OBJECT

public:
	MMGStringDisplay(QWidget *parent);

	enum Mode { MODE_NORMAL, MODE_THIN, MODE_EDITABLE };

	const QStringList &bounds() const { return _bounds; };

	void setDisplayMode(Mode mode);
	void setStorage(MMGUtils::MMGString *storage, const QStringList &force_bounds = QStringList());

	void setDescription(const QString &desc);
	void setOptions(MIDIButtons options);
	void setBounds(const QStringList &bounds);
	void setDefaultValue(const QString &val) { defaults = val; };

	void display();
	void reset();

signals:
	void stringChanged();

public slots:
	void setComboBoxState(short state);

protected:
	MMGUtils::MMGString *string = nullptr;
	MMGUtils::MMGString defaults;
	QStringList _bounds;

	QLabel *label;
	QComboBox *combo_string;
	QComboBox *combo_min;
	QComboBox *combo_max;

	MMGMIDIButtons *midi_buttons;
};

#endif // MMG_STRING_DISPLAY_H
