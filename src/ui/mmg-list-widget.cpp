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

#include "mmg-list-widget.h"
#include "../mmg-binding.h"

#include <QMimeData>

using namespace MMGUtils;

MMGListWidget::MMGListWidget(QWidget *parent) : QListWidget(parent)
{
	setDragEnabled(true);
	viewport()->setAcceptDrops(true);
	setDropIndicatorShown(true);
	setDragDropMode(QAbstractItemView::InternalMove);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setStyleSheet("border-color: rgb(0, 0, 0);");
	setWordWrap(true);

	connect(this, &QListWidget::itemClicked, this, &MMGListWidget::selected);
	connect(this, &QListWidget::itemChanged, this, &MMGListWidget::rename);
	connect(model(), &QAbstractItemModel::rowsMoved, this, &MMGListWidget::move);
}

QMimeData *MMGListWidget::mimeData(const QList<QListWidgetItem *> &items) const
{
	QMimeData *mime = new QMimeData;
	QByteArray ba;
	QDataStream stream(&ba, QIODeviceBase::WriteOnly);

	for (QListWidgetItem *item : items) {
		void *_data = item->data(dataRole()).data();
		stream << *(quint64 *)(&_data);
	}

	mime->setData(mimeTypes()[0], ba);
	return mime;
}

void MMGListWidget::setItemsEditable(bool editable)
{
	items_editable = editable;
	displayAll();
}
