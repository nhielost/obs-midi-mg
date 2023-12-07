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

#ifndef MMG_MANAGER_DISPLAY_H
#define MMG_MANAGER_DISPLAY_H

#include "mmg-list-widget.h"
#include "../mmg-utils.h"
#include "../mmg-config.h"

template<class T> class MMGManagerDisplay : public MMGListWidget {

public:
	MMGManagerDisplay(QWidget *parent, MMGManager<T> *storage) : MMGListWidget(parent), _manager(storage){};

	T *getValue(QListWidgetItem *item) const;
	T *currentValue() const { return getValue(currentItem()); };

	void selectValues(const QList<T *> &values);
	const QList<T *> selectedValues() const;

	void displayAll() override;
	void insertData(int index, T *data);
	void updateData();

	void addNew() override { insertData(count(), _manager->add()); };
	void copy() override { insertData(count(), _manager->copy(currentValue())); };
	void remove() override;

protected:
	MMGManager<T> *_manager;

	void rename(QListWidgetItem *widget_item) override;
	void move(const QModelIndex &, int start, int, const QModelIndex &, int row) override;
};

#include "mmg-manager-display.cpp"

#endif // MMG_MANAGER_DISPLAY_H
