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

#ifndef MMG_MANAGER_CONTROL_CPP
#define MMG_MANAGER_CONTROL_CPP

#include "mmg-manager-control.h"

using namespace MMGUtils;

template<typename T> MMGManagerControl<T>::MMGManagerControl(QListWidget *widget) : QObject(widget)
{
	list_widget = widget;
	list_widget->setProperty("kind", MultiModeMap<T>::value);

	connect(list_widget, &QListWidget::itemSelectionChanged, this, &MMGManagerControl<T>::itemShow);
	connect(list_widget, &QListWidget::itemChanged, this, &MMGManagerControl<T>::itemRenamed);
	connect(list_widget->model(), &QAbstractItemModel::rowsMoved, this, &MMGManagerControl<T>::itemMoved);
}

template<typename T> void MMGManagerControl<T>::setStorage(MMGManager<T> *manager)
{
	current_manager = manager;
	refresh();
}

template<typename T> void MMGManagerControl<T>::refresh()
{
	if (!isEnabled()) return;

	list_widget->clear();
	for (T *val : *current_manager) {
		QListWidgetItem *new_item = new QListWidgetItem;
		new_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled |
				   (list_widget->property("editable").isNull() ? Qt::ItemIsEditable : Qt::NoItemFlags));
		new_item->setText(val->objectName());
		new_item->setData(dataRole(), QVariant::fromValue(val));
		list_widget->addItem(new_item);
	}

	itemShow();
}

template<typename T> void MMGManagerControl<T>::connectButtons(QPushButton *add, QPushButton *remove, QPushButton *copy)
{
	connect(add, &QPushButton::clicked, this, &MMGManagerControl<T>::itemAdded);
	connect(remove, &QPushButton::clicked, this, &MMGManagerControl<T>::itemRemoved);
	if (!!copy) connect(copy, &QPushButton::clicked, this, &MMGManagerControl<T>::itemCopied);
}

template<typename T> void MMGManagerControl<T>::itemShow()
{
	QListWidgetItem *item = list_widget->currentItem();
	if (!isEnabled() || !item) {
		current_value = nullptr;
		return;
	}

	current_value = item->data(dataRole()).value<T *>();
}

template<typename T> void MMGManagerControl<T>::itemRenamed(QListWidgetItem *item)
{
	if (!isEnabled() || !item) return;
	currentValue()->setObjectName(item->text());
}

template<typename T>
void MMGManagerControl<T>::itemMoved(const QModelIndex &, int from, int, const QModelIndex &, int to)
{
	if (!isEnabled()) return;

	current_manager->move(from, to);
}

template<typename T> void MMGManagerControl<T>::itemAdded()
{
	if (!isEnabled()) return;

	current_manager->add();
	refresh();
}

template<typename T> void MMGManagerControl<T>::itemCopied()
{
	if (!isEnabled() || !currentValue()) return;
	current_manager->copy(currentValue());
	refresh();
}

template<typename T> void MMGManagerControl<T>::itemRemoved()
{
	if (!isEnabled() || !currentValue()) return;
	if (!open_message_box("PermanentRemove", false)) return;

	current_manager->remove(currentValue());
	current_value = nullptr;

	list_widget->blockSignals(true);
	refresh();
	list_widget->blockSignals(false);

	emit list_widget->itemSelectionChanged();
}

#endif