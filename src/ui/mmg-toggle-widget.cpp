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

#include "mmg-toggle-widget.h"

namespace MMGWidgets {

MMGToggleWidget::MMGToggleWidget(QWidget *parent) : MMGValueWidget(parent)
{
	main_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);

	button = new QPushButton(this);
	button->setFixedSize(40, 40);
	button->setIconSize(QSize(32, 32));
	button->setCheckable(true);
	button->setCursor(Qt::PointingHandCursor);
	connect(button, &QPushButton::clicked, this, &MMGToggleWidget::buttonClicked);
	main_layout->addWidget(button, 0, Qt::AlignTop | Qt::AlignRight);
}

void MMGToggleWidget::reset()
{
	_value = default_value;
	update();
}

void MMGToggleWidget::buttonClicked()
{
	_value = button->isChecked();
	update();
}

void MMGToggleWidget::display() const
{
	QSignalBlocker blocker_button(button);
	button->setChecked(_value);
	button->setIcon(_value ? mmgicon("on") : mmgicon("off"));
}

} // namespace MMGWidgets
