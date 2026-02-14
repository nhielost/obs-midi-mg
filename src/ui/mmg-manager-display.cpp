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

#include "mmg-manager-display.h"

namespace MMGWidgets {

// MMGManagerDisplayQWidget
MMGManagerDisplayQWidget::MMGManagerDisplayQWidget(QWidget *parent, QWidget *_info_widget, bool use_buttons)
	: QWidget(parent)
{
	setContentsMargins(10, 10, 10, 10);
	setFixedSize(parent->width(), parent->height());

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(10);

	if (use_buttons) {
		buttons_header = new QWidget(this);
		QHBoxLayout *buttons_header_layout = new QHBoxLayout;
		buttons_header_layout->setContentsMargins(0, 0, 0, 0);
		buttons_header_layout->setSpacing(10);

		add_button = new QPushButton(buttons_header);
		add_button->setFixedSize(40, 40);
		add_button->setCursor(Qt::PointingHandCursor);
		add_button->setIcon(mmgicon("add"));
		add_button->setIconSize({20, 20});
		add_button->setToolTip(mmgtr("UI.Buttons.New"));
		connect(add_button, &QPushButton::clicked, this, &MMGManagerDisplayQWidget::itemAdded);
		buttons_header_layout->addWidget(add_button);

		buttons_header_layout->addStretch(1);

		copy_button = new QPushButton(buttons_header);
		copy_button->setFixedSize(40, 40);
		copy_button->setCursor(Qt::PointingHandCursor);
		copy_button->setIcon(mmgicon("copy"));
		copy_button->setIconSize({20, 20});
		copy_button->setToolTip(mmgtr("UI.Buttons.Copy"));
		connect(copy_button, &QPushButton::clicked, this, &MMGManagerDisplayQWidget::itemCopied);
		buttons_header_layout->addWidget(copy_button);

		remove_button = new QPushButton(buttons_header);
		remove_button->setFixedSize(40, 40);
		remove_button->setCursor(Qt::PointingHandCursor);
		remove_button->setIcon(mmgicon("delete"));
		remove_button->setIconSize({20, 20});
		remove_button->setToolTip(mmgtr("UI.Buttons.Delete"));
		connect(remove_button, &QPushButton::clicked, this, &MMGManagerDisplayQWidget::itemRemoved);
		buttons_header_layout->addWidget(remove_button);

		buttons_header->setLayout(buttons_header_layout);
		layout->addWidget(buttons_header);
	}

	list_widget = new QListWidget(this);
	list_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	list_widget->setFocusPolicy(Qt::ClickFocus);
	list_widget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	list_widget->setStyleSheet("border-color: #000000;");
	list_widget->setWordWrap(true);
	list_widget->setDragDropMode(QListWidget::InternalMove);
	list_widget->setDragEnabled(use_buttons);
	list_widget->setAcceptDrops(use_buttons);
	connect(list_widget, &QListWidget::itemSelectionChanged, this, &MMGManagerDisplayQWidget::itemShow);
	connect(list_widget, &QListWidget::itemChanged, this, &MMGManagerDisplayQWidget::itemRename);
	connect(list_widget->model(), &QAbstractItemModel::rowsMoved, this, &MMGManagerDisplayQWidget::itemMoved);
	layout->addWidget(list_widget, 1);

	info_widget = _info_widget;
	if (!!info_widget) layout->addWidget(info_widget);

	setLayout(layout);
}
// End MMGManagerDisplayQWidget

// MMGManagerDisplay<T>
template <typename T>
MMGManagerDisplay<T>::MMGManagerDisplay(QWidget *parent, QWidget *info_widget, bool use_buttons)
	: MMGManagerDisplayQWidget(parent, info_widget, use_buttons)
{
}

template <typename T> void MMGManagerDisplay<T>::setStorage(MMGManager<T> *manager)
{
	if (current_manager == manager) return;

	current_manager = manager;
	refresh();
}

template <typename T> void MMGManagerDisplay<T>::refreshCurrent(T *value)
{
	list_widget->blockSignals(true);

	list_widget->clear();

	if (!!current_manager) {
		for (T *val : *current_manager) {
			QListWidgetItem *new_item = new QListWidgetItem;
			new_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled |
					   (list_widget->acceptDrops() ? Qt::ItemIsEditable : Qt::NoItemFlags));
			new_item->setText(val->objectName());
			list_widget->addItem(new_item);
		}
	}

	setCurrentValue(value);

	list_widget->blockSignals(false);

	itemShow();
}

template <typename T> void MMGManagerDisplay<T>::setCurrentValue(T *value)
{
	int index = current_manager->indexOf(value);
	list_widget->setCurrentRow(!!value ? index : -1);
	if (current_value != value) itemShow();
}

template <typename T> void MMGManagerDisplay<T>::itemShow()
{
	int index = list_widget->currentRow();
	current_value = index >= 0 ? current_manager->at(index) : nullptr;

	if (!!current_value) emit itemRenamed(current_value->objectName());

	if (!!info_widget) info_widget->setVisible(!!current_value);
	if (!!copy_button) copy_button->setVisible(!!current_value);
	if (!!remove_button) remove_button->setVisible(!!current_value && current_manager->size() > 1);

	emit currentValueChanged();
}

template <typename T> void MMGManagerDisplay<T>::itemRename(QListWidgetItem *item)
{
	if (!item) return;
	currentValue()->setObjectName(item->text());
	emit itemRenamed(item->text());
}

template <typename T>
void MMGManagerDisplay<T>::itemMoved(const QModelIndex &, int from, int, const QModelIndex &, int to)
{
	current_manager->move(from, to);
}

template <typename T> void MMGManagerDisplay<T>::itemAdded()
{
	current_manager->add();
	refreshCurrent(currentValue());
}

template <typename T> void MMGManagerDisplay<T>::itemCopied()
{
	if (!currentValue()) return;
	current_manager->copy(currentValue());
	refreshCurrent(currentValue());
}

template <typename T> void MMGManagerDisplay<T>::itemRemoved()
{
	if (!currentValue()) return;
	if (!prompt_question("PermanentRemove")) return;

	current_manager->remove(currentValue());
	current_value = nullptr;

	refresh();
}
// End MMGManagerDisplay<T>

template class MMGManagerDisplay<MMGMessage>;
template class MMGManagerDisplay<MMGAction>;
template class MMGManagerDisplay<MMGBinding>;
template class MMGManagerDisplay<MMGBindingManager>;
template class MMGManagerDisplay<MMGDevice>;
template class MMGManagerDisplay<MMGPreference>;

} // namespace MMGWidgets
