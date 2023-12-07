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

#ifndef MMG_LIST_WIDGET_H
#define MMG_LIST_WIDGET_H

#include "../mmg-utils.h"

#include <QListWidget>
#include <QPushButton>

class MMGBinding;

class MMGListWidget : public QListWidget {
	Q_OBJECT

public:
	MMGListWidget(QWidget *parent);
	virtual ~MMGListWidget() = default;

	enum SelectionType { SELECTION_ONE, SELECTION_MULTIPLE };

	MMGUtils::DeviceType filterType() const { return filter_type; };
	void setFilterType(MMGUtils::DeviceType device_type) { filter_type = device_type; };

	int indexOf(const QListWidgetItem *item) const { return indexFromItem(item).row(); };
	virtual void displayAll() = 0;
	virtual void updateData() = 0;
	static int dataRole() { return 260; };

	void setItemsEditable(bool editable);

signals:
	void allDisplayed();
	void selected();
	void itemMoved();

public slots:
	virtual void addNew() = 0;
	virtual void copy() = 0;
	virtual void remove() = 0;

protected slots:
	virtual void rename(QListWidgetItem *widget_item) = 0;
	virtual void move(const QModelIndex &parent, int start, int end, const QModelIndex &dest, int row) = 0;

protected:
	QMimeData *mimeData(const QList<QListWidgetItem *> &items) const override;
	QStringList mimeTypes() const override { return {"application/x-mmg-manager-data"}; }

	bool items_editable = true;
	MMGUtils::DeviceType filter_type = MMGUtils::TYPE_NONE;
};

#endif // MMG_LIST_WIDGET_H
