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

#ifndef MMG_STRING_WIDGET_H
#define MMG_STRING_WIDGET_H

#include "mmg-value-widget.h"

#include <QLineEdit>
#include <QTextEdit>

namespace MMGWidgets {

class MMGStringWidget : public MMGValueWidget {
	Q_OBJECT

public:
	MMGStringWidget(QWidget *parent = nullptr);

	const QString &value() const { return _value; };
	void setValue(const QString &value) { _value = value; };

	void setDefaultValue(const QString &value) { default_value = value; };
	void reset() override;

	void setParams(const MMGParams<QString> &params);

public slots:
	void display() const override;

private slots:
	void textChanged();
	void lineChanged(const QString &);

private:
	union Widget {
		QTextEdit *text_edit;
		QLineEdit *line_edit;
	} d;

	QString _value;
	QString default_value;

	obs_text_type widget_type = (obs_text_type)(-1);
};

} // namespace MMGWidgets

#endif // MMG_STRING_WIDGET_H
