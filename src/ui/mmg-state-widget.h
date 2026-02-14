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

#ifndef MMG_STATES_WIDGET_H
#define MMG_STATES_WIDGET_H

#include "mmg-value-display.h"

namespace MMGWidgets {

namespace MMGStateWidgets {

class MMGStateQWidget : public QWidget {
	Q_OBJECT

public:
	MMGStateQWidget(QWidget *parent) : QWidget(parent)
	{
		setFixedWidth(350);

		QVBoxLayout *wrap_layout = new QVBoxLayout;
		wrap_layout->setContentsMargins(0, 0, 0, 0);
		wrap_layout->setSpacing(0);

		main_layout = new QVBoxLayout;
		main_layout->setContentsMargins(5, 5, 5, 5);
		main_layout->setSpacing(0);
		wrap_layout->addLayout(main_layout);
		wrap_layout->addStretch(1);

		setLayout(wrap_layout);
	};

	QWidget *bottomWidget() const { return bottom_widget; };

	virtual void refresh() = 0;

protected:
	QVBoxLayout *main_layout;
	QWidget *bottom_widget = nullptr;
};

template <typename T, ValueState State> class MMGStateWidget : public MMGStateQWidget {
protected:
	using StorageType = typename MMGStates::Map<T, State>::Type;

	MMGStateWidget(QWidget *parent, MMGValueStateDisplay<T> *storage)
		: MMGStateQWidget(parent),
		  storage_widget(storage)
	{
		_data = storage_widget->storage()->template changeTo<State>();
	};

	StorageType *_data;
	MMGValueStateDisplay<T> *storage_widget;
};

template <typename T> class Fixed : public MMGStateWidget<T, STATE_FIXED> {
	using QObject::connect;

public:
	Fixed(QWidget *parent, MMGValueStateDisplay<T> *storage);

	void refresh() override { _display->refresh(); };

protected:
	MMGValueFixedDisplay<T> *_display;

	using MMGStateWidget<T, STATE_FIXED>::_data;

	using MMGStateQWidget::main_layout;
};

template <typename T> class MIDIMap : public MMGStateWidget<T, STATE_MIDI> {
	using QObject::connect;
	using QObject::disconnect;

	using QWidget::setFocus;

	using StorageType = MMGStateWidget<T, STATE_MIDI>::StorageType;

public:
	MIDIMap(QWidget *parent, MMGValueStateDisplay<T> *storage, const MMGValueStateInfoList &refs);

private:
	void setReferenceIndex();

	void generateDataWidget(int64_t index);

	void onValueChange(MMGValueFixedDisplay<T> *value_display);
	void onEditRequest(MMGValueFixedDisplay<T> *refresher);
	void onAddClick();
	void onRemoveClick();

	void refresh() override;

protected:
	using MMGStateWidget<T, STATE_MIDI>::_data;
	using MMGStateWidget<T, STATE_MIDI>::storage_widget;

	using MMGStateQWidget::bottom_widget;
	using MMGStateQWidget::main_layout;

	QVBoxLayout *bottom_layout;
	QLabel *info_label;
	QPushButton *add_button;
	QPushButton *remove_button;

	MMGValueQWidget *current_mapping_ref_index_display = nullptr;
	MMGValueFixedDisplay<T> *current_mapping_display = nullptr;
	MMGValueFixedDisplay<MMGStates::ReferenceIndex> *index_display = nullptr;
	const MMGValueStateInfoList &_refs;
};

template <typename T> class MIDIRange : public MMGStateWidget<T, STATE_RANGE> {
	using QObject::connect;
	using QObject::disconnect;

	using StorageType = MMGStateWidget<T, STATE_RANGE>::StorageType;

public:
	MIDIRange(QWidget *parent, MMGValueStateDisplay<T> *storage, const MMGValueStateInfoList &refs);

private:
	void setReferenceIndex();
	void setMin() { _data->setMin(min_display->value()); };
	void setMax() { _data->setMax(max_display->value()); };

	void refresh() override;

protected:
	using MMGStateWidget<T, STATE_RANGE>::_data;
	using MMGStateWidget<T, STATE_RANGE>::storage_widget;

	using MMGStateQWidget::bottom_widget;
	using MMGStateQWidget::main_layout;

	MMGValueFixedDisplay<MMGStates::ReferenceIndex> *index_display = nullptr;
	MMGValueDisplay<T> *min_display;
	MMGValueDisplay<T> *max_display;
	QLabel *warning_label = nullptr;
	MMGParams<T> min_params;
	MMGParams<T> max_params;

	const MMGValueStateInfoList &_refs;
};

template <typename T> class Toggle : public MMGStateWidget<T, STATE_TOGGLE> {
	using QObject::connect;
	using QObject::disconnect;

	using StorageType = MMGStateWidget<T, STATE_TOGGLE>::StorageType;

public:
	Toggle(QWidget *parent, MMGValueStateDisplay<T> *storage);

private:
	void setToggleCount();
	void setToggledValue(int64_t index, const MMGValueFixedDisplay<T> *display)
	{
		_data->set(index, display->value());
	};

	void refresh() override;

protected:
	using MMGStateWidget<T, STATE_TOGGLE>::_data;
	using MMGStateWidget<T, STATE_TOGGLE>::storage_widget;

	using MMGStateQWidget::bottom_widget;
	using MMGStateQWidget::main_layout;

	MMGValueFixedDisplay<int64_t> *toggle_count_display;
};

template <typename T> class Increment : public MMGStateWidget<T, STATE_INCREMENT> {
	using QObject::connect;

public:
	Increment(QWidget *parent, MMGValueStateDisplay<T> *storage);

	void refresh() override;

private:
	void setIncrement() { _data->setIncrement(_display->value()); };

protected:
	MMGValueFixedDisplay<T> *_display = nullptr;

	using MMGStateWidget<T, STATE_INCREMENT>::_data;
	using MMGStateWidget<T, STATE_INCREMENT>::storage_widget;

	using MMGStateQWidget::main_layout;
};

template <typename T> class Ignore : public MMGStateWidget<T, STATE_IGNORE> {
	using StorageType = MMGStateWidget<T, STATE_IGNORE>::StorageType;

public:
	Ignore(QWidget *parent, MMGValueStateDisplay<T> *storage) : MMGStateWidget<T, STATE_IGNORE>(parent, storage) {};

	void refresh() override {};
};

// Specializations
template <> class Toggle<bool> : public MMGStateWidget<bool, STATE_TOGGLE> {
public:
	Toggle(QWidget *parent, MMGValueStateDisplay<bool> *storage)
		: MMGStateWidget<bool, STATE_TOGGLE>(parent, storage) {};

	void refresh() override {};
};
// End Specializations

class ConstructBase {
public:
	virtual ~ConstructBase() = default;
	virtual MMGStateQWidget *operator()(ValueState state) = 0;
};

template <typename T> class Construct : public ConstructBase {
public:
	Construct(QWidget *parent, MMGValueStateDisplay<T> *value_display, const MMGValueStateInfoList &refs);

	MMGStateQWidget *operator()(ValueState state) override;

private:
	QWidget *_parent;
	MMGValueStateDisplay<T> *_storage;
	const MMGValueStateInfoList &_refs;
};

} // namespace MMGStateWidgets

} // namespace MMGWidgets

#endif // MMG_STATES_WIDGET_H
