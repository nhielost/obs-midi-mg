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

#include "mmg-combo-box.h"

#include <QAbstractItemView>
#include <QStandardItemModel>

namespace MMGWidgets {

MMGComboBox::MMGComboBox(QWidget *parent) : MMGValueWidget(parent)
{
	combo = new QComboBox(this);
	main_layout->addWidget(combo);
	connect(combo, &QComboBox::currentIndexChanged, this, &MMGComboBox::onIndexChanged);
}

void MMGComboBox::setItemEnabled(int index, bool enable)
{
	qobject_cast<QStandardItemModel *>(combo->model())->item(index)->setEnabled(enable);
}

void MMGComboBox::display() const
{
	QSignalBlocker blocker_combo(combo);
	combo->setCurrentIndex(current_index);
}

void MMGComboBox::onIndexChanged(int index)
{
	current_index = index;

	updating = true;
	update();
	updating = false;
}

void MMGComboBox::resetWidth()
{
	// Original code found here:
	// https://stackoverflow.com/questions/30817328/fit-the-width-of-the-view-to-longest-item-of-the-qcombobox

	int32_t max = 0;
	QFontMetrics metrics = fontMetrics();

	for (int32_t i = 0; i < combo->count(); i++) {
		// 60 is calculated from maximum stylesheet padding values in OBS
		// combined together with potential scrollbars that may or may not exist
		int32_t width = metrics.horizontalAdvance(combo->itemText(i)) + 60;
		if (width > max) max = width;
	}
	max += combo->view()->autoScrollMargin();

	if (max > combo->view()->minimumWidth()) combo->view()->setMinimumWidth(max);
}

} // namespace MMGWidgets
