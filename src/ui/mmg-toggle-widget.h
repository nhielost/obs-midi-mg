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

#ifndef MMG_TOGGLE_WIDGET_H
#define MMG_TOGGLE_WIDGET_H

#include "mmg-value-widget.h"

#include <QPushButton>

namespace MMGWidgets {

class MMGToggleWidget : public MMGValueWidget {
	Q_OBJECT

public:
	/*struct Params : public MMGValueWidget::Params {
          bool default_value = true;
  };*/

public:
	MMGToggleWidget(QWidget *parent = nullptr);

	bool value() const { return _value; };
	void setValue(bool value) { _value = value; };

	void setDefaultValue(bool value) { default_value = value; };
	void reset() override;

	void setParams(const MMGParams<bool> &params) { setDefaultValue(params.default_value); };

public slots:
	void display() const override;

private slots:
	void buttonClicked();

private:
	QPushButton *button;

	bool _value = false;
	bool default_value = true;
};

} // namespace MMGWidgets

#endif // MMG_TOGGLE_WIDGET_H
