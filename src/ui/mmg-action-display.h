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

#ifndef MMG_ACTION_DISPLAY_H
#define MMG_ACTION_DISPLAY_H

#include "../mmg-utils.h"
#include "mmg-number-display.h"
#include "mmg-string-display.h"

class MMGBinding;

struct MMGActionFieldRequest {
	OBSSourceAutoRelease source;
	MMGUtils::MMGJsonObject *json;
};

using MMGOBSFieldsList = QList<class MMGOBSFields *>;

class MMGActionDisplay : public QWidget {
	Q_OBJECT

public:
	MMGActionDisplay(QWidget *parent);
	~MMGActionDisplay() { reset(); };

	MMGStringDisplay *stringDisplay(int index) const { return string_fields.at(index); };
	MMGNumberDisplay *numberDisplay(int index) const { return number_fields.at(index); };

	MMGStringDisplay *addNew(MMGUtils::MMGString *storage, const QStringList &bounds = QStringList());
	MMGNumberDisplay *addNew(MMGUtils::MMGNumber *storage);
	void hideAll();

	void setFields(QWidget *widget);
	void reset();

	void setCustomOBSFields(const MMGActionFieldRequest &req);

	static void clearCustomOBSFields();

private:
	QScrollArea *scroll_area;
	QWidget *scroll_widget;
	QVBoxLayout *layout;

	QList<MMGStringDisplay *> string_fields;
	QList<MMGNumberDisplay *> number_fields;
	QWidget *fields = nullptr;

	static MMGOBSFieldsList mmg_custom_fields;
};

#endif // MMG_ACTION_DISPLAY_H
