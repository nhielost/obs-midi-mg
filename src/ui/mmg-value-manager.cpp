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

#include "mmg-value-manager.h"

#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>

namespace MMGWidgets {

MMGValueManager::MMGValueManager(QWidget *parent, MMGStateDisplay *state_display)
	: QWidget(parent),
	  state_display(state_display)
{
	QVBoxLayout *main_layout = new QVBoxLayout;
	main_layout->setSpacing(0);
	main_layout->setContentsMargins(0, 0, 0, 0);

	fixed_layout = new QVBoxLayout;
	fixed_layout->setSpacing(0);
	fixed_layout->setContentsMargins(5, 5, 5, 5);
	main_layout->addLayout(fixed_layout, 0);

	QFrame *sep = new QFrame(this);
	sep->setObjectName("sep");
	sep->setFrameShape(QFrame::HLine);
	sep->setFixedHeight(1);
	main_layout->addWidget(sep, 0, Qt::AlignTop);

	QScrollArea *scroll_area = new QScrollArea(this);
	scroll_area->setWidgetResizable(true);
	scroll_area->setFrameShape(QFrame::NoFrame);
	scroll_area->verticalScrollBar()->setSingleStep(35);
	scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	createScrollBarOverlay(scroll_area);

	QVBoxLayout *scroll_layout_wrap = new QVBoxLayout;
	scroll_layout_wrap->setContentsMargins(0, 0, 0, 0);
	scroll_layout_wrap->setSpacing(0);

	scroll_layout = new QVBoxLayout;
	scroll_layout->setContentsMargins(5, 5, 5, 5);
	scroll_layout->setSpacing(0);
	scroll_layout_wrap->addLayout(scroll_layout);
	scroll_layout_wrap->addStretch(1);

	QWidget *scroll_widget = new QWidget(this);
	scroll_widget->setFixedWidth(350);
	scroll_widget->setLayout(scroll_layout_wrap);

	scroll_area->setWidget(scroll_widget);
	main_layout->addWidget(scroll_area, 1);

	bottom_layout = new QVBoxLayout;
	bottom_layout->setSpacing(10);
	bottom_layout->setContentsMargins(10, 10, 10, 10);
	main_layout->addLayout(bottom_layout, 0);

	setLayout(main_layout);

	if (!!state_display)
		connect(this, &MMGValueManager::refreshRequested, state_display, &MMGStateDisplay::refreshStorage);
}

int32_t MMGValueManager::count() const
{
	return fixed_layout->count() + scroll_layout->count() + bottom_layout->count();
}

void MMGValueManager::addManager(MMGValueManager *manager)
{
	if (!manager) return;

	manager->fixed_layout->setContentsMargins(0, 0, 0, 0);
	manager->layout()->itemAt(1)->widget()->hide();             // Hide the separator
	manager->scroll_layout->parentWidget()->setFixedWidth(340); // Prevent overlap under the scrollbar
	manager->scroll_layout->setContentsMargins(0, 0, 0, 0);

	for (int i = 0; !!manager->bottom_layout->itemAt(i); i++)
		manager->bottom_layout->itemAt(i)->widget()->hide();

	connect(this, &MMGValueManager::refreshRequested, manager, &MMGValueManager::refreshAll);
	manager->state_display = state_display;
	scroll_layout->addWidget(manager);
}

void MMGValueManager::addCustom(QWidget *other)
{
	if (!!other) scroll_layout->addWidget(other);
}

void MMGValueManager::removeExcept(int32_t remaining)
{
	QLayoutItem *next;
	while (scroll_layout->count() > remaining) {
		next = scroll_layout->takeAt(scroll_layout->count() - 1);

		if (auto *state_info = dynamic_cast<MMGValueStateWidgetInfo *>(next->widget());
		    !!state_info && state_infos.contains(state_info))
			state_infos.removeOne(state_info);

		disconnect(this, &MMGValueManager::refreshRequested, next->widget(), nullptr);
		delete next->widget();
		delete next;
	}
}

void MMGValueManager::clear()
{
	removeAll();

	current_value_display_editor = nullptr;
	if (!!state_display) state_display->clearStorage();
}

void MMGValueManager::clearEditRequest()
{
	if (!!current_value_display_editor) {
		disconnect(current_value_display_editor, &QObject::destroyed, this, nullptr);
		current_value_display_editor->setEditingProperty(-1);
	}
	current_value_display_editor = nullptr;
}

void MMGValueManager::connectNew(MMGValueQWidget *value_display, const MMGCallback &cb)
{
	if (cb) connect(value_display, &MMGValueQWidget::valueChanged, this, cb);
	value_display->refresh();
	if (cb) connect(value_display, &MMGValueQWidget::valueChanged, this, &MMGValueManager::refreshAll);

	connect(this, &MMGValueManager::refreshRequested, value_display,
		std::bind(&MMGValueManager::sendRefresh, this, value_display));
	connect(this, &MMGValueManager::modifyRequested, value_display, &MMGValueQWidget::setModifiable);
}

void MMGValueManager::sendRefresh(MMGValueQWidget *value_display)
{
	if (value_display != refresh_sender) value_display->refresh();
}

void MMGValueManager::refreshAll()
{
	if (!!refresh_sender) return;

	refresh_sender = !!sender() ? sender() : this;
	emit refreshRequested();
	refresh_sender = nullptr;
}

} // namespace MMGWidgets
