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

#include "mmg-binding-display.h"
#include "../mmg-settings.h"

using namespace MMGUtils;

void generateLabel(QWidget *parent, const QString &text)
{
	QLabel *label = new QLabel(parent);
	label->setMinimumSize(150, 40);
	label->setAlignment(Qt::AlignCenter);
	label->setWordWrap(true);
	label->setText(text);
	parent->layout()->addWidget(label);
}

template<class T> void generateContentsLabels(QWidget *parent, const QList<T *> &values)
{
	QLayoutItem *item;
	while (!!(item = parent->layout()->takeAt(0))) {
		delete item->widget();
		delete item;
	}

	if (values.isEmpty()) {
		generateLabel(parent, mmgtr("Actions.Titles.None"));
		return;
	}

	for (T *data : values)
		generateLabel(parent, data->name());
}

MMGBindingDisplay::MMGBindingDisplay(QWidget *parent, bool executable) : QWidget(parent), _executable(executable)
{
	head_layout = new QVBoxLayout(this);
	head_layout->setContentsMargins(20, 20, 20, 20);
	head_layout->setSpacing(40);
	head_layout->setSizeConstraint(QLayout::SetMinimumSize);

#define CREATE_ROW(which, name, rgb, index)                                                    \
	which##s_widget = new QWidget(this);                                                   \
	which##s_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);     \
	which##s_widget->setMinimumSize(351, 40);                                              \
	which##s_widget->setMaximumSize(351, 16777215);                                        \
                                                                                               \
	QHBoxLayout *which##s_layout = new QHBoxLayout(which##s_widget);                       \
	which##s_layout->setContentsMargins(0, 0, 0, 0);                                       \
	which##s_layout->setSpacing(15);                                                       \
	which##s_layout->setSizeConstraint(QLayout::SetMinimumSize);                           \
                                                                                               \
	QPushButton *which##s_button = new QPushButton(which##s_widget);                       \
	which##s_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);                \
	which##s_button->setFixedSize(40, 40);                                                 \
	which##s_button->setCursor(Qt::PointingHandCursor);                                    \
	which##s_button->setStyleSheet("border: 1px solid " rgb ";");                          \
	which##s_button->setIcon(QIcon(":/icons/edit.svg"));                                   \
	which##s_button->setIconSize(QSize(20, 20));                                           \
	which##s_button->setProperty("page", index);                                           \
	connect(which##s_button, &QPushButton::clicked, this, &MMGBindingDisplay::emitEdited); \
	which##s_layout->addWidget(which##s_button, 0, Qt::AlignTop);                          \
                                                                                               \
	which##s_label = new QLabel(which##s_widget);                                          \
	which##s_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);                 \
	which##s_label->setFixedSize(90, 40);                                                  \
	which##s_label->setText(mmgtr(name));                                                  \
	which##s_label->setWordWrap(true);                                                     \
	which##s_label->setMargin(5);                                                          \
	which##s_layout->addWidget(which##s_label, 0, Qt::AlignTop);                           \
                                                                                               \
	current_##which##s = new QWidget(which##s_widget);                                     \
	current_##which##s->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);  \
	current_##which##s->setMinimumSize(150, 40);                                           \
                                                                                               \
	QVBoxLayout *current_##which##s_layout = new QVBoxLayout(current_##which##s);          \
	current_##which##s_layout->setContentsMargins(0, 0, 0, 0);                             \
	current_##which##s_layout->setSpacing(10);                                             \
	current_##which##s_layout->setSizeConstraint(QLayout::SetMinimumSize);                 \
                                                                                               \
	which##s_layout->addWidget(current_##which##s, 0, Qt::AlignCenter);                    \
                                                                                               \
	head_layout->addWidget(which##s_widget)

	CREATE_ROW(device, "UI.Buttons.Devices", "rgb(0, 0, 255)", 1);
	CREATE_ROW(message, "UI.Buttons.Messages", "rgb(0, 255, 0)", 2);
	if (executable) {
		CREATE_ROW(action, "UI.Buttons.Actions", "rgb(255, 0, 0)", 3);
		CREATE_ROW(setting, "UI.Buttons.Preferences", "rgb(224, 224, 0)", 4);
	}

#undef CREATE_ROW
}

void MMGBindingDisplay::setStorage(MMGBinding *binding)
{
	current_binding = binding;
	display();
}

void MMGBindingDisplay::display()
{
	generateContentsLabels(current_devices, current_binding->usedDevices());
	generateContentsLabels(current_messages, current_binding->usedMessages());
	if (!_executable) return;

	generateContentsLabels(current_actions, current_binding->usedActions());

	bool is_output = current_binding->type() == TYPE_OUTPUT;
	head_layout->removeWidget(messages_widget);
	head_layout->removeWidget(actions_widget);

	head_layout->insertWidget(1, is_output ? actions_widget : messages_widget);
	head_layout->insertWidget(2, is_output ? messages_widget : actions_widget);
	messages_label->setText(mmgtr(is_output ? "UI.Buttons.Messages" : "Message.Name"));
	actions_label->setText(mmgtr(is_output ? "Actions.Name" : "UI.Buttons.Actions"));
}

void MMGBindingDisplay::emitEdited()
{
	emit edited(sender()->property("page").toInt());
}
