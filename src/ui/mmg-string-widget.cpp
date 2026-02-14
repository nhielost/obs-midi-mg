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

#include "mmg-string-widget.h"

namespace MMGWidgets {

MMGStringWidget::MMGStringWidget(QWidget *parent) : MMGValueWidget(parent) {}

void MMGStringWidget::setParams(const MMGParams<QString> &params)
{
	setDefaultValue(params.default_value);

	if (widget_type != (obs_text_type)(-1)) return;
	widget_type = params.text_type;

	if (widget_type == OBS_TEXT_MULTILINE) {
		d.text_edit = new QTextEdit(this);
		d.text_edit->connect(d.text_edit, &QTextEdit::textChanged, this, &MMGStringWidget::textChanged);
		main_layout->addWidget(d.text_edit);
	} else {
		d.line_edit = new QLineEdit(this);
		d.line_edit->setEchoMode((QLineEdit::EchoMode)(widget_type == OBS_TEXT_PASSWORD ? 3 : 0));
		connect(d.line_edit, &QLineEdit::textChanged, this, &MMGStringWidget::textChanged);
		main_layout->addWidget(d.line_edit);
	}
}

void MMGStringWidget::reset()
{
	_value = default_value;
	update();
}

void MMGStringWidget::textChanged()
{
	_value = d.text_edit->toPlainText();
	update();
}

void MMGStringWidget::lineChanged(const QString &value)
{
	_value = value;
	update();
}

void MMGStringWidget::display() const
{
	if (widget_type == OBS_TEXT_MULTILINE) {
		QSignalBlocker blocker_text_edit(d.text_edit);
		d.text_edit->setPlainText(_value);
	} else {
		QSignalBlocker blocker_line_edit(d.line_edit);
		d.line_edit->setText(_value);
	}
}

} // namespace MMGWidgets
