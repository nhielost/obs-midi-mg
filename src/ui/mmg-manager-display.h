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

#ifndef MMG_MANAGER_DISPLAY_H
#define MMG_MANAGER_DISPLAY_H

#include "../mmg-config.h"
#include "mmg-value-widget.h"

#include <QListWidget>
#include <QPushButton>

namespace MMGWidgets {

class MMGManagerDisplayQWidget : public QWidget {
	Q_OBJECT

public:
	MMGManagerDisplayQWidget(QWidget *parent, QWidget *info_widget = nullptr, bool use_buttons = true);
	virtual ~MMGManagerDisplayQWidget() = default;

	void setEditable(bool editable) { buttons_header->setVisible(editable); };

signals:
	void currentValueChanged();
	void itemRenamed(const QString &);

private slots:
	virtual void itemShow() = 0;
	virtual void itemRename(QListWidgetItem *item) = 0;
	virtual void itemMoved(const QModelIndex &, int from, int, const QModelIndex &, int to) = 0;
	virtual void itemAdded() = 0;
	virtual void itemCopied() = 0;
	virtual void itemRemoved() = 0;

protected:
	QListWidget *list_widget;
	QWidget *info_widget = nullptr;

	QWidget *buttons_header;
	QPushButton *add_button = nullptr;
	QPushButton *copy_button = nullptr;
	QPushButton *remove_button = nullptr;

	static int dataRole() { return 260; };
};

template <typename T> class MMGManagerDisplay : public MMGManagerDisplayQWidget {

public:
	MMGManagerDisplay(QWidget *parent, QWidget *info_widget = nullptr, bool use_buttons = true);

	void setStorage(MMGManager<T> *manager);
	T *currentValue() const { return current_value; };
	void setCurrentValue(T *value);

	void refresh() { refreshCurrent(nullptr); };

private:
	MMGManager<T> *current_manager = nullptr;
	T *current_value = nullptr;

	void refreshCurrent(T *value);

	void itemShow() override;
	void itemRename(QListWidgetItem *item = nullptr) override;
	void itemMoved(const QModelIndex &, int from, int, const QModelIndex &, int to) override;
	void itemAdded() override;
	void itemCopied() override;
	void itemRemoved() override;
};

} // namespace MMGWidgets

#endif // MMG_MANAGER_DISPLAY_H
