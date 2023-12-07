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

#include "mmg-action-display.h"
#include "mmg-fields.h"

#include <QScrollBar>

using namespace MMGUtils;

MMGOBSFieldsList MMGActionDisplay::mmg_custom_fields;

MMGActionDisplay::MMGActionDisplay(QWidget *parent) : QWidget(parent)
{
	scroll_area = new QScrollArea(this);
	scroll_widget = new QWidget(scroll_area);

	scroll_area->setGeometry(0, 0, 350, 350);
	scroll_area->setWidgetResizable(true);
	scroll_area->setFrameShape(QFrame::NoFrame);
	scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scroll_area->verticalScrollBar()->setSingleStep(35);
	scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	layout = new QVBoxLayout(this);
	layout->setSpacing(10);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	layout->setContentsMargins(10, 10, 10, 10);

	string_fields = new MMGStringDisplayFields(scroll_widget);
	layout->addWidget(string_fields);

	number_fields = new MMGNumberDisplayFields(scroll_widget);
	reset();

	scroll_widget->setContentsMargins(0, 0, 0, 0);
	scroll_widget->setLayout(layout);
	scroll_area->setWidget(scroll_widget);
}

void MMGActionDisplay::setFields(QWidget *widget)
{
	reset();
	fields = widget;
	fields->setParent(scroll_widget);
	layout->removeWidget(number_fields);
	number_fields->setVisible(false);
	fields->setVisible(true);
	layout->addWidget(widget);
}

void MMGActionDisplay::reset()
{
	if (fields) {
		layout->removeWidget(fields);
		fields->setParent(nullptr);
		fields->setVisible(false);
		disconnect(fields, &QObject::destroyed, this, nullptr);
	}

	layout->addWidget(number_fields);
	number_fields->setVisible(true);
	fields = nullptr;
}

void MMGActionDisplay::setCustomOBSFields(const MMGActionFieldRequest &req)
{
	if (!req.source || !req.json) return;

	MMGOBSFields *fields_req = nullptr;
	for (MMGOBSFields *obs_fields : mmg_custom_fields) {
		if (!obs_fields->match(req.source)) continue;
		fields_req = obs_fields;
		fields_req->changeSource(req.source);
	}
	if (!fields_req) {
		fields_req = new MMGOBSFields(req.source);
		mmg_custom_fields.append(fields_req);
	}

	connect(fields_req, &QObject::destroyed, this, [&]() { fields = nullptr; });
	fields_req->setJsonDestination(req.json);
	setFields(fields_req);
}

void MMGActionDisplay::clearCustomOBSFields()
{
	qDeleteAll(mmg_custom_fields);
}
