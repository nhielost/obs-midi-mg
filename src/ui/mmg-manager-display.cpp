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

#ifndef MMG_MANAGER_DISPLAY_CPP
#define MMG_MANAGER_DISPLAY_CPP

#include "mmg-manager-display.h"

template<class T> T *MMGManagerDisplay<T>::getValue(QListWidgetItem *widget_item) const
{
	if (!widget_item) return nullptr;
	return widget_item->data(dataRole()).value<T *>();
}

template<class T> void MMGManagerDisplay<T>::selectValues(const QList<T *> &values)
{
	for (int i = 0; i < count(); ++i) {
		QListWidgetItem *_item = item(i);
		_item->setSelected(values.contains(getValue(_item)));
	}
}

template<class T> const QList<T *> MMGManagerDisplay<T>::selectedValues() const
{
	QList<T *> selected;
	selected.resize(count());

	for (QListWidgetItem *item : selectedItems())
		selected[indexOf(item)] = getValue(item);

	for (int i = 0; i < selected.size(); ++i)
		if (!selected[i]) selected.removeAt(i--);

	return selected;
}

template<class T> void MMGManagerDisplay<T>::displayAll()
{
	clear();
	for (T *value : *_manager)
		if (_manager->filter(filter_type, value)) insertData(count(), value);

	clearSelection();
	emit allDisplayed();
}

template<class T> void MMGManagerDisplay<T>::insertData(int index, T *data)
{
	QListWidgetItem *new_item = new QListWidgetItem;
	new_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled |
			   (items_editable ? Qt::ItemIsEditable : Qt::NoItemFlags));
	new_item->setText(data->name());
	new_item->setData(dataRole(), QVariant::fromValue(data));
	insertItem(index, new_item);
}

template<class T> void MMGManagerDisplay<T>::updateData()
{
	for (int i = 0; i < count(); ++i) {
		QListWidgetItem *widget_item = item(i);
		T *value = _manager->at(i);
		if (getValue(widget_item) != value) widget_item->setData(dataRole(), QVariant::fromValue(value));
	}
}

template<class T> void MMGManagerDisplay<T>::remove()
{
	T *data = currentValue();
	if (!data) return;

	_manager->remove(data);

	delete currentItem();

	emit selected();
}

template<class T> void MMGManagerDisplay<T>::rename(QListWidgetItem *widget_item)
{
	if (!widget_item) return;

	if (currentRow() != row(widget_item)) setCurrentItem(widget_item);

	T *value = getValue(widget_item);
	QString new_name = widget_item->text();

	if (value->name() == new_name) return;
	if (!!_manager->find(new_name)) {
		currentItem()->setText(value->name());
	} else {
		value->setName(new_name);
	}
}

template<class T> void MMGManagerDisplay<T>::move(const QModelIndex &, int start, int, const QModelIndex &, int row)
{
	if (selectionMode() != SingleSelection) return;
	_manager->move(start, row);
	emit itemMoved();
}

#endif // MMG_MANAGER_DISPLAY_CPP
