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

#ifndef MMG_COMBO_BOX_H
#define MMG_COMBO_BOX_H

#include "mmg-value-widget.h"

#include <QComboBox>

namespace MMGWidgets {

class MMGComboBox : public MMGValueWidget {
	Q_OBJECT

public:
	MMGComboBox(QWidget *parent = nullptr);
	~MMGComboBox() = default;

	template <typename T> T value() const
	{
		return t_vals.isEmpty() ? T() : fromVariant<T>(t_vals[current_index]);
	};
	template <typename T> void setValue(const T &value)
	{
		current_index = t_vals.indexOf(toVariant<T>(value));
		if (current_index < 0) reset();
	};

	void reset() override { current_index = default_index; };

	void setItemEnabled(int index, bool enabled);
	void resetWidth();

	template <typename T> void setParams(const MMGParams<T> &params)
	{
		if (updating) return;

		combo->blockSignals(true);
		combo->clear();
		t_vals.clear();
		for (auto &[val, _tr] : params.bounds) {
			combo->addItem(_tr);
			t_vals += toVariant<T>(val);
		}
		combo->setCurrentIndex(0);
		combo->blockSignals(false);

		int index = params.bounds.indexOf(params.default_value);
		default_index = index + (index < 0);

		combo->setPlaceholderText(!params.placeholder.isEmpty() ? params.placeholder : QString());
		combo->setEditable(params.text_editable);
		resetWidth();
	};

	void display() const override;

private slots:
	void onIndexChanged(int);

private:
	template <typename T> QVariant toVariant(const T &value) const { return MMGJson::convertFrom(value); };
	template <typename T> T fromVariant(const QVariant &variant) const
	{
		return MMGJson::convertTo<T>(variant.toJsonValue());
	};

	template <typename T> QVariant toVariant(const T &value) const requires MMGJson::MMGJsonIneligible<T>
	{
		return QVariant::fromValue(value);
	};
	template <typename T> T fromVariant(const QVariant &variant) const requires MMGJson::MMGJsonIneligible<T>
	{
		return variant.value<T>();
	};

private:
	QComboBox *combo;

	QVariantList t_vals;
	int current_index;
	int default_index;

	mutable bool updating = false;
};

} // namespace MMGWidgets

#endif // MMG_COMBO_BOX_H
