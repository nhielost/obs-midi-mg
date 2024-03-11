/*
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

#ifndef MMG_MANAGER_CONTROL_H
#define MMG_MANAGER_CONTROL_H

#include "../mmg-manager.h"
#include "../mmg-config.h"

#include <QListWidget>

template<typename T> struct MultiModeMap;

template<typename T> class MMGManagerControl : public QObject {

public:
	MMGManagerControl(QListWidget *widget);
	void setStorage(MMGManager<T> *manager);
	bool isEnabled() const { return list_widget->property("kind").toInt() == MultiModeMap<T>::value; };
	void refresh();
	T *currentValue() const { return current_value; };

	void connectButtons(QPushButton *add, QPushButton *remove, QPushButton *copy = nullptr);

	// public slots:
	void itemShow();
	void itemRenamed(QListWidgetItem *item = nullptr);
	void itemMoved(const QModelIndex &, int from, int, const QModelIndex &, int to);
	void itemAdded();
	void itemCopied();
	void itemRemoved();

private:
	QListWidget *list_widget;

	MMGManager<T> *current_manager = nullptr;
	T *current_value = nullptr;

	static int dataRole() { return 260; };
};

#include "mmg-manager-control.cpp"

#endif // MMG_MANAGER_CONTROL_H
