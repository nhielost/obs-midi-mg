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

#include "mmg-state-widget.h"

namespace MMGWidgets {

namespace MMGStateWidgets {

// Fixed<T>
template <typename T>
Fixed<T>::Fixed(QWidget *parent, MMGValueStateDisplay<T> *storage) : MMGStateWidget<T, STATE_FIXED>(parent, storage)
{
	_display = new MMGValueFixedDisplay<T>(this, storage->params(), storage->storage());
	_display->refresh();

	connect(_display, &MMGValueQWidget::valueChanged, storage, &MMGValueStateDisplay<T>::refresh);
	main_layout->addWidget(_display);
}
// End Fixed<T>

// MIDIMap<T>
static inline MMGValueStateWidgetInfo *getReferenceInfo(const MMGValueStateInfoList &refs,
							MMGStates::ReferenceIndex index)
{
	if (!refs.value(0)) return nullptr;
	if (index >= refs.size()) index = MMGStates::ReferenceIndex(0);
	return refs[index];
};

static MMGParams<MMGStates::ReferenceIndex> value_index_params {
	.desc = mmgtr("MIDIButtons.MIDI.Reference"),
	.options = OPTION_NONE,
	.default_value = MMGStates::ReferenceIndex(0),

	.bounds = {},
};

template <typename T>
MIDIMap<T>::MIDIMap(QWidget *parent, MMGValueStateDisplay<T> *storage, const MMGValueStateInfoList &refs)
	: MMGStateWidget<T, STATE_MIDI>(parent, storage),
	  _refs(refs)
{
	if (_data->referenceIndex() >= _refs.size()) _data->setReferenceIndex(MMGStates::ReferenceIndex(0));

	bottom_layout = new QVBoxLayout;
	bottom_layout->setContentsMargins(5, 5, 5, 5);
	bottom_layout->setSpacing(0);

	auto *ref_info = getReferenceInfo(_refs, _data->referenceIndex());

	info_label = new QLabel(this);
	info_label->setMargin(5);
	info_label->setAlignment(Qt::AlignCenter);
	info_label->setWordWrap(true);
	info_label->setText(mmgtr(choosetr("MIDIButtons.MIDI", "AcceptableEdit", "ApplicableEdit", !ref_info)));

	if (!ref_info) {
		QHBoxLayout *edit_layout = new QHBoxLayout;
		edit_layout->setContentsMargins(5, 5, 5, 5);
		edit_layout->setSpacing(10);
		bottom_layout->addLayout(edit_layout);

		edit_layout->addWidget(info_label);

		QVBoxLayout *button_layout = new QVBoxLayout;
		button_layout->setContentsMargins(0, 0, 0, 0);
		button_layout->setSpacing(5);
		edit_layout->addLayout(button_layout);

		add_button = new QPushButton(this);
		add_button->setFixedSize(40, 40);
		add_button->setCursor(Qt::PointingHandCursor);
		add_button->setIcon(mmgicon("add"));
		add_button->setIconSize({20, 20});
		connect(add_button, &QPushButton::clicked, this, &MIDIMap<T>::onAddClick);
		button_layout->addWidget(add_button, 0, Qt::AlignVCenter);

		remove_button = new QPushButton(this);
		remove_button->setFixedSize(40, 40);
		remove_button->setCursor(Qt::PointingHandCursor);
		remove_button->setIcon(mmgicon("delete"));
		remove_button->setIconSize({20, 20});
		connect(remove_button, &QPushButton::clicked, this, &MIDIMap<T>::onRemoveClick);
		button_layout->addWidget(remove_button, 0, Qt::AlignVCenter);
	} else {
		bottom_layout->addWidget(info_label);

		value_index_params.bounds.clear();
		for (int64_t i = 0; i < _refs.size(); i++)
			value_index_params.bounds.insert(MMGStates::ReferenceIndex(i), _refs[i]->getDescription());

		index_display = new MMGValueFixedDisplay<MMGStates::ReferenceIndex>(this, &value_index_params);
		index_display->setContentsMargins(5, 5, 5, 5);
		index_display->refresh();
		index_display->setValue(_data->referenceIndex());
		connect(index_display, &MMGValueQWidget::valueChanged, this, &MIDIMap<T>::setReferenceIndex);
		for (int64_t i = 0; i < _refs.size(); i++) {
			if (!_refs[i]->isWidgetVisible()) index_display->setItemEnabled(i, false);
		}
		bottom_layout->addWidget(index_display);
	}

	bottom_widget = new QWidget(this);
	bottom_widget->setLayout(bottom_layout);

	refresh();
}

template <typename T> void MIDIMap<T>::setReferenceIndex()
{
	_data->setReferenceIndex(index_display->value());
	_data->clearAll();
	refresh();
}

template <typename T> void MIDIMap<T>::generateDataWidget(int64_t index)
{
	auto *value_display = new MMGValueFixedDisplay<T>(this, storage_widget->params());
	value_display->setContentsMargins(5, 5, 5, 5);
	value_display->setProperty("index", qlonglong(index));
	value_display->setEditingProperty(-1);
	value_display->refresh();
	value_display->setValue(_data->getValue(index));
	value_display->setDescription(mmgtr("MIDIButtons.MIDI.DisplayLabel")
					      .arg(storage_widget->params()->desc)
					      .arg(index + 1, 3, 10, QChar('0')));
	connect(value_display, &MMGValueQWidget::valueChanged, this,
		std::bind(&MIDIMap<T>::onValueChange, this, value_display));
	connect(value_display, &MMGValueQWidget::editRequested, this,
		std::bind(&MIDIMap<T>::onEditRequest, this, value_display));
	main_layout->addWidget(value_display);
}

template <typename T> void MIDIMap<T>::onValueChange(MMGValueFixedDisplay<T> *value_display)
{
	int64_t mapping_display_index = value_display->property("index").toLongLong();
	if (_data->getValue(mapping_display_index) == value_display->value()) return;

	if (!getReferenceInfo(_refs, _data->referenceIndex())) {
		if (int64_t index_check = _data->indexOf(value_display->value());
		    index_check != -1 && index_check != mapping_display_index) {
			value_display->setValue(_data->getValue(mapping_display_index));
			prompt_info("MIDIMapExists");
			return;
		}
	}

	_data->setValue(mapping_display_index, value_display->value());
	onEditRequest(nullptr);
}

template <typename T> void MIDIMap<T>::onEditRequest(MMGValueFixedDisplay<T> *refresher)
{
	if (!!current_mapping_display) {
		disconnect(current_mapping_display, &QObject::destroyed, this, nullptr);
		current_mapping_display->setEditingProperty(-1);
	}
	if (!!current_mapping_ref_index_display) {
		bottom_layout->removeWidget(current_mapping_ref_index_display);
		delete current_mapping_ref_index_display;
		current_mapping_ref_index_display = nullptr;
	}

	current_mapping_display = current_mapping_display != refresher ? refresher : nullptr;

	if (!!current_mapping_display) {
		connect(current_mapping_display, &QObject::destroyed, this,
			[&]() { current_mapping_display = nullptr; });
		current_mapping_display->setEditingProperty(1);
	}

	auto *ref_info = getReferenceInfo(_refs, _data->referenceIndex());
	if (!!ref_info) {
		info_label->setVisible(!current_mapping_display && ref_info->isReferenceEligible());
		if (!current_mapping_display) return;

		current_mapping_ref_index_display =
			ref_info->generateFromIndex(this, current_mapping_display->property("index").toLongLong());
		bottom_layout->insertWidget(0, current_mapping_ref_index_display);
	} else {
		add_button->setEnabled(_data->referenceSize() < storage_widget->params()->boundsSize());
		remove_button->setEnabled(!!current_mapping_display);
	}
}

template <typename T> void MIDIMap<T>::onAddClick()
{
	int64_t ref_size = _data->referenceSize();
	if (ref_size == storage_widget->params()->boundsSize()) return;
	_data->setSize(ref_size + 1);

	T new_value = storage_widget->params()->boundsValue(ref_size);
	if (_data->indexOf(new_value) != -1) {
		// Find the next valid value to add (i.e. the next unique value)
		int64_t next_index = ref_size;
		do {
			if (++next_index >= storage_widget->params()->boundsSize()) next_index = 0;
			new_value = storage_widget->params()->boundsValue(next_index);
		} while (_data->indexOf(new_value) != -1);
	}

	_data->setValue(ref_size, new_value);
	generateDataWidget(ref_size);
	onEditRequest(nullptr);
}

template <typename T> void MIDIMap<T>::onRemoveClick()
{
	_data->clearValue(current_mapping_display->property("index").toLongLong());
	delete current_mapping_display;
	current_mapping_display = nullptr;
	refresh();
}

template <typename T> void MIDIMap<T>::refresh()
{
	clearLayout(main_layout);

	MMGValueStateWidgetInfo *ref = getReferenceInfo(_refs, _data->referenceIndex());
	int64_t old_size = _data->referenceSize();
	int64_t init_size = !!ref ? ref->getStateSize() : storage_widget->params()->boundsSize();

	if (old_size == 0 || (!!ref && old_size != init_size)) {
		if (!ref && init_size >= 128) init_size = 8;
		_data->setSize(init_size);

		while (old_size < init_size) {
			_data->setValue(old_size, storage_widget->params()->boundsValue(old_size));
			old_size++;
		}
	}

	if (!ref || ref->isReferenceEligible()) {
		for (int64_t i = 0; i < _data->referenceSize(); i++)
			generateDataWidget(i);
	} else {
		QLabel *warning_label = new QLabel(this);
		warning_label->setMargin(20);
		warning_label->setAlignment(Qt::AlignCenter);
		warning_label->setWordWrap(true);
		warning_label->setText(mmgtr("MIDIButtons.MIDI.ReferenceIneligible"));
		main_layout->addWidget(warning_label, 1);
	}

	onEditRequest(nullptr);
}
// End MIDIMap<T>

// MIDIRange<T>
template <typename T>
MIDIRange<T>::MIDIRange(QWidget *parent, MMGValueStateDisplay<T> *storage, const MMGValueStateInfoList &refs)
	: MMGStateWidget<T, STATE_RANGE>(parent, storage),
	  _refs(refs)
{
	min_display = new MMGValueFixedDisplay<T>(this, &min_params);
	min_display->setContentsMargins(5, 5, 5, 5);
	connect(min_display, &MMGValueQWidget::valueChanged, this, &MIDIRange<T>::setMin);
	main_layout->addWidget(min_display);

	max_display = new MMGValueFixedDisplay<T>(this, &max_params);
	max_display->setContentsMargins(5, 5, 5, 5);
	connect(max_display, &MMGValueQWidget::valueChanged, this, &MIDIRange<T>::setMax);
	main_layout->addWidget(max_display);

	main_layout->addStretch(1);

	if (getReferenceInfo(_refs, _data->referenceIndex()) != nullptr) {
		warning_label = new QLabel(this);
		warning_label->setMargin(20);
		warning_label->setAlignment(Qt::AlignCenter);
		warning_label->setWordWrap(true);
		warning_label->setText(mmgtr("MIDIButtons.MIDI.ReferenceIneligible"));
		main_layout->addWidget(warning_label, 1);

		value_index_params.bounds.clear();
		for (int64_t i = 0; i < _refs.size(); i++)
			value_index_params.bounds.insert(MMGStates::ReferenceIndex(i), _refs[i]->getDescription());

		index_display = new MMGValueFixedDisplay<MMGStates::ReferenceIndex>(this, &value_index_params);
		index_display->setContentsMargins(10, 10, 10, 10);
		index_display->refresh();
		index_display->setValue(_data->referenceIndex());
		connect(index_display, &MMGValueQWidget::valueChanged, this, &MIDIRange<T>::setReferenceIndex);
		for (int64_t i = 0; i < _refs.size(); i++) {
			if (!_refs[i]->isWidgetVisible()) index_display->setItemEnabled(i, false);
		}

		bottom_widget = index_display;
	}

	refresh();
}

template <typename T> void MIDIRange<T>::setReferenceIndex()
{
	auto ref_index = index_display->value();
	if (_data->referenceIndex() == ref_index) return;

	_data->setReferenceIndex(ref_index);
	_data->setMin(0);
	_data->setMax(0);
	refresh();
}

template <typename T> void MIDIRange<T>::refresh()
{
	min_params = *storage_widget->params();
	min_params.desc = nontr(qUtf8Printable(mmgtr("MIDIButtons.Range.Minimum").arg(min_params.desc)));
	min_params.default_value = min_params.lower_bound;

	max_params = *storage_widget->params();
	max_params.desc = nontr(qUtf8Printable(mmgtr("MIDIButtons.Range.Maximum").arg(max_params.desc)));
	max_params.default_value = max_params.upper_bound;

	if (_data->min() == 0 && _data->max() == 0) {
		_data->setMin(min_params.default_value);
		_data->setMax(max_params.default_value);
	}

	min_display->refresh();
	min_display->setValue(_data->min());

	max_display->refresh();
	max_display->setValue(_data->max());

	auto *ref = getReferenceInfo(_refs, _data->referenceIndex());
	bool warning_hidden = !ref || ref->isReferenceEligible();

	min_display->setVisible(warning_hidden);
	max_display->setVisible(warning_hidden);
	if (!!warning_label) warning_label->setVisible(!warning_hidden);
}
// End MIDIRange<T>

// Toggle<T>
static const MMGParams<int64_t> toggle_count_params {
	.desc = mmgtr("MIDIButtons.Toggle.CountText"),
	.options = OPTION_NONE,
	.default_value = 2,

	.lower_bound = 2.0,
	.upper_bound = 16.0,
	.step = 1.0,
};

template <typename T>
Toggle<T>::Toggle(QWidget *parent, MMGValueStateDisplay<T> *storage) : MMGStateWidget<T, STATE_TOGGLE>(parent, storage)
{
	toggle_count_display = new MMGValueFixedDisplay<int64_t>(this, &toggle_count_params);
	toggle_count_display->setContentsMargins(10, 10, 10, 10);
	toggle_count_display->refresh();

	if (_data->size() < 2) {
		toggle_count_display->setValue(2);
		setToggleCount();
	} else {
		toggle_count_display->setValue(_data->size());
		refresh();
	}

	connect(toggle_count_display, &MMGValueQWidget::valueChanged, this, &Toggle<T>::setToggleCount);
	bottom_widget = toggle_count_display;
}

template <typename T> void Toggle<T>::setToggleCount()
{
	auto new_toggle_count = toggle_count_display->value();
	if (_data->size() == new_toggle_count) return;

	_data->setSize(new_toggle_count);
	for (int64_t i = 0; i < new_toggle_count; i++)
		_data->set(i, storage_widget->params()->boundsValue(i));

	refresh();
}

template <typename T> void Toggle<T>::refresh()
{
	clearLayout(main_layout);
	_data->setCurrentIndex(0);

	for (int i = 0; i < _data->size(); i++) {
		auto *value_display = new MMGValueFixedDisplay<T>(this, storage_widget->params());
		value_display->setContentsMargins(5, 5, 5, 5);
		value_display->refresh();
		value_display->setValue(_data->get(i));
		value_display->setDescription(
			mmgtr("MIDIButtons.Toggle.DisplayDescription").arg(storage_widget->params()->desc).arg(i + 1));
		connect(value_display, &MMGValueQWidget::valueChanged,
			std::bind(&Toggle<T>::setToggledValue, this, i, value_display));
		main_layout->addWidget(value_display);
	}
}
// End Toggle<T>

// Increment<T>
template <typename T>
Increment<T>::Increment(QWidget *parent, MMGValueStateDisplay<T> *storage)
	: MMGStateWidget<T, STATE_INCREMENT>(parent, storage)
{
	if constexpr (MMGStates::ExtraFeatures<T>) {
		_display = new MMGValueFixedDisplay<T>(this, storage->params());
		_display->setContentsMargins(5, 5, 5, 5);
		_display->setIncrementing();
		connect(_display, &MMGValueQWidget::valueChanged, this, &Increment<T>::setIncrement);
		main_layout->addWidget(_display);
		refresh();
	}
}

template <typename T> void Increment<T>::refresh()
{
	if (!_display) return;
	_display->refresh();
	_display->setValue(_data->increment());
	_display->setDescription(mmgtr("MIDIButtons.Increment.Label").arg(storage_widget->params()->desc));
}
// End Increment<T>

// Construct<T>
template <typename T>
Construct<T>::Construct(QWidget *parent, MMGValueStateDisplay<T> *value_display, const MMGValueStateInfoList &refs)
	: _parent(parent),
	  _storage(value_display),
	  _refs(refs)
{
}

template <typename T> MMGStateQWidget *Construct<T>::operator()(ValueState state)
{
	switch (state) {
		case STATE_MIDI:
			return new MIDIMap<T>(_parent, _storage, _refs);

		case STATE_TOGGLE:
			return new Toggle<T>(_parent, _storage);

		case STATE_IGNORE:
			return new Ignore<T>(_parent, _storage);

		case STATE_RANGE:
			if constexpr (MMGStates::ExtraFeatures<T>) return new MIDIRange<T>(_parent, _storage, _refs);
			[[fallthrough]];

		case STATE_INCREMENT:
			if constexpr (MMGStates::ExtraFeatures<T>) return new Increment<T>(_parent, _storage);
			[[fallthrough]];

		default:
			return new Fixed<T>(_parent, _storage);
	}
};
// End Construct<T>

} // namespace MMGStateWidgets

} // namespace MMGWidgets
