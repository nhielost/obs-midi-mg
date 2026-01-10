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

#include "mmg-value-widget.h"

#include <QAbstractButton>
#include <QLabel>
#include <QMouseEvent>

namespace MMGWidgets {

struct ButtonFace {
	uint8_t size;
	const char *name;
	const char *icon_name;
};

static QMap<ValueState, ButtonFace> state_data {
	{STATE_FIXED, {16u, "Fixed", icon_url("fixed")}},
	{STATE_MIDI, {24u, "MIDI", icon_url("midi")}},
	{STATE_RANGE, {20u, "Range", icon_url("custom")}},
	{STATE_TOGGLE, {22u, "Toggle", icon_url("toggle-on")}},
	{STATE_INCREMENT, {22u, "Increment", icon_url("increment")}},
	{STATE_IGNORE, {20u, "Ignore", icon_url("disable")}},
};

void clearLayout(QLayout *layout)
{
	QLayoutItem *next;
	while (layout->count() > 0) {
		next = layout->takeAt(0);
		if (!!next->layout()) clearLayout(next->layout());
		if (!!next->widget()) delete next->widget();
		delete next;
	}
}

void createScrollBarOverlay(QAbstractScrollArea *scroller)
{
	// Modified from https://stackoverflow.com/a/68797937

	QScrollBar *scroll_bar = scroller->verticalScrollBar();
	scroll_bar->setFixedWidth(10);
	scroll_bar->setParent(scroller);

	QHBoxLayout *scroller_layout = new QHBoxLayout;
	scroller_layout->setContentsMargins(0, 0, 0, 0);
	scroller_layout->addStretch(1);
	scroller_layout->addWidget(scroll_bar);
	scroller->setLayout(scroller_layout);
}

const char *getStateName(ValueState state)
{
	return state_data[state].name;
}

void setStateIconAndToolTip(QWidget *icon_display, ValueState state)
{
	const ButtonFace &icon_info = state_data[state];
	MMGString tooltip = MMGText::join("MIDIButtons", icon_info.name);

	if (auto *button = qobject_cast<QAbstractButton *>(icon_display); !!button) {
		button->setIcon(QIcon(icon_info.icon_name));
		button->setIconSize(QSize(icon_info.size, icon_info.size));
		button->setToolTip(mmgtr(tooltip));
	} else if (auto *label = qobject_cast<QLabel *>(icon_display); !!label) {
		label->setPixmap(QIcon(icon_info.icon_name).pixmap(icon_info.size, icon_info.size));
		label->setToolTip(mmgtr(tooltip));
	}
}

MMGValueWidget::MMGValueWidget(QWidget *parent) : QWidget(parent)
{
	setMinimumHeight(40);
	setFocusPolicy(Qt::ClickFocus);

	main_layout = new QHBoxLayout;
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(10);

	setLayout(main_layout);
}

void MMGValueWidget::setModifiable(bool modify)
{
	if (modify && !pixmap_view) return;
	if (!pixmap_view) pixmap_view = new QLabel(this);
	pixmap_view->setHidden(modify);
	resizePixmap(size());
}

void MMGValueWidget::focusInEvent(QFocusEvent *event)
{
	if (event->reason() != Qt::MouseFocusReason) {
		clearFocus();
	} else {
		focusAcquired() ? event->accept() : event->ignore();
	}
}

void MMGValueWidget::focusOutEvent(QFocusEvent *event)
{
	focusLost() ? event->accept() : event->ignore();
}

void MMGValueWidget::resizeEvent(QResizeEvent *event)
{
	resizePixmap(event->size());
}

void MMGValueWidget::resizePixmap(const QSize &size)
{
	if (!pixmap_view) return;

	pixmap_view->setFixedSize(size);
	QPixmap p(size);
	p.fill(Qt::transparent);
	pixmap_view->setPixmap(p);
}

void MMGValueWidget::update() const
{
	emit valueChanged();
	display();
}

} // namespace MMGWidgets
