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

#include "mmg-value-display.h"

#include <QPaintEngine>

namespace MMGWidgets {

// MMGValueQWidget
inline MMGValueQWidget::MMGValueQWidget(QWidget *parent) : QWidget(parent)
{
	setVisible(true);
	setAttribute(Qt::WA_Hover, true);
	setMinimumHeight(40);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

	main_layout = new QHBoxLayout;
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(10);
	setLayout(main_layout);
}

inline void MMGValueQWidget::setEditingProperty(int8_t value)
{
	setProperty("editing", value);
	setStyleSheet("/* */");
	if (hasStorage()) refresh();
}

inline void MMGValueQWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	QStyleOption opt;
	opt.initFrom(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

inline void MMGValueQWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (e->button() != Qt::LeftButton) return;

	setFocus(Qt::MouseFocusReason);
	emit editRequested();
	e->accept();
};
// End MMGValueQWidget

// MMGValueDisplay<T>
template <typename T>
MMGValueDisplay<T>::MMGValueDisplay(QWidget *parent, const MMGParams<T> *params)
	: MMGValueQWidget(parent),
	  _params(params)
{
	_editor = new ValueWidgetMap<T>::DisplayType(this);
	_editor->setMinimumHeight(40);

	main_layout->addWidget(_editor, _editor->layout()->sizeConstraint() != QLayout::SetMinAndMaxSize);
}

template <typename T> T MMGValueDisplay<T>::value() const
{
	if constexpr (std::is_same_v<decltype(_editor), MMGComboBox *>) {
		return _editor->template value<T>();
	} else {
		return _editor->value();
	}
}

template <typename T> void MMGValueDisplay<T>::setValue(const T &value)
{
	_editor->setValue(value);
	_editor->update();
}

template <typename T> void MMGValueDisplay<T>::refresh()
{
	setVisible(!(_params->options & OPTION_HIDDEN));
	setEnabled(!(_params->options & OPTION_DISABLED));

	_editor->setParams(*_params);
	_editor->update();
}
// MMGValueDisplay<T>

// MMGValueFixedDisplay<T>
template <typename T>
MMGValueFixedDisplay<T>::MMGValueFixedDisplay(QWidget *parent, const MMGParams<T> *params)
	: MMGValueDisplay<T>(parent, params)
{
	_label = new QLabel(this);
	_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	_label->setMinimumSize(100, 40);
	_label->setWordWrap(true);
	_label->setAlignment(Qt::AlignVCenter);
	main_layout->insertWidget(0, _label, 0, Qt::AlignTop);

	connect(_editor, &MMGValueWidget::valueChanged, this, &MMGValueQWidget::valueChanged);
}

template <typename T>
MMGValueFixedDisplay<T>::MMGValueFixedDisplay(QWidget *parent, const MMGParams<T> *params, MMGValue<T> *storage)
	: MMGValueDisplay<T>(parent, params),
	  _storage(storage)
{
	main_layout->setContentsMargins(5, 5, 5, 5);

	_label = new QLabel(this);
	_label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
	_label->setMinimumSize(100, 40);
	_label->setWordWrap(true);
	_label->setAlignment(Qt::AlignVCenter);
	main_layout->insertWidget(0, _label, 0, Qt::AlignTop);

	connect(_editor, &MMGValueWidget::valueChanged, this, &MMGValueFixedDisplay<T>::setFixedValue);
	connectEditor();
}

template <typename T> void MMGValueFixedDisplay<T>::setDescription(const QString &desc)
{
	_label->setText(desc);
}

template <typename T> void MMGValueFixedDisplay<T>::setFixedValue()
{
	if (!_storage) return;
	_storage->template as<STATE_FIXED>()->setValue(value());
}

template <typename T> void MMGValueFixedDisplay<T>::refresh()
{
	setVisible(!(_params->options & OPTION_HIDDEN));
	setEnabled(!(_params->options & OPTION_DISABLED));

	_label->setStyleSheet(_params->options & OPTION_EMPHASIS ? "font-weight: bold;" : "");
	setDescription(_params->desc);
	_editor->setParams(*_params);

	if (!!_storage) setValue(*_storage);
}
// End MMGValueFixedDisplay<T>

// MMGValueStateDisplay<T>
template <typename T>
MMGValueStateDisplay<T>::MMGValueStateDisplay(QWidget *parent, const MMGParams<T> *params, MMGValue<T> *storage)
	: MMGValueFixedDisplay<T>(parent, params, storage)
{
	setProperty("editing", -1);

	state_label = new QLabel(this);
	state_label->setVisible(false);
	state_label->setFixedSize(60, 40);
	state_label->setAlignment(Qt::AlignCenter);
	main_layout->addWidget(state_label);
}

template <typename T> void MMGValueStateDisplay<T>::setModifiable(bool modify)
{
	MMGValueDisplay<T>::setModifiable(modify);
	setEditingProperty(modify ? -1 : 0);
}

template <typename T> void MMGValueStateDisplay<T>::refresh()
{
	setVisible(!(_params->options & OPTION_HIDDEN));
	setEnabled(!(_params->options & OPTION_DISABLED));

	_label->setStyleSheet(_params->options & OPTION_EMPHASIS ? "font-weight: bold;" : "");
	setDescription(_params->desc);
	_editor->setParams(*_params);

	if (property("editing").toInt() == 1) {
		state_label->setVisible(true);
		_editor->setVisible(false);

		state_label->setPixmap(mmgicon("edit").pixmap(22, 22));
		state_label->setToolTip(mmgtr("Fields.Editing"));
	} else if ((*_storage)->state() == STATE_FIXED) {
		state_label->setVisible(false);
		_editor->setVisible(true);

		setValue(*_storage);
	} else {
		state_label->setVisible(true);
		_editor->setVisible(false);

		setStateIconAndToolTip(state_label, (*_storage)->state());
	}
}

template <typename T> int64_t MMGValueStateDisplay<T>::getStateSize() const
{
	if (!isReferenceEligible()) return 0;
	return std::clamp<int64_t>(_storage->template as<MMGStates::MMGReferenceIndexHandler>()->referenceSize(), 0,
				   128);
}

template <typename T> bool MMGValueStateDisplay<T>::hasReferenceValue(int64_t index) const
{
	if (!isReferenceEligible()) return false;
	return _storage->template as<MMGStates::MMGReferenceIndexHandler>()->hasReferenceValue(index);
}

template <typename T>
MMGValueQWidget *MMGValueStateDisplay<T>::generateFromIndex(QWidget *parent, int64_t ref_index) const
{
	T ref_index_value = _params->default_value;

	if ((*_storage)->state() == STATE_MIDI)
		ref_index_value = _storage->template as<STATE_MIDI>()->getValue(ref_index);

	if constexpr (MMGStates::ExtraFeatures<T>) {
		if ((*_storage)->state() == STATE_RANGE) {
			auto *range_state = _storage->template as<STATE_RANGE>();
			double ref_factor = std::clamp<double>(range_state->referenceSize() - 1.0, 1.0, 127.0);
			ref_index_value = double(range_state->referenceSize() - 1.0) * (ref_index / ref_factor) +
					  range_state->min();
		}
	}

	auto *new_self = new MMGValueFixedDisplay<T>(parent, _params);
	new_self->setContentsMargins(5, 5, 5, 5);
	new_self->setModifiable(false);
	new_self->refresh();
	new_self->setValue(ref_index_value);
	new_self->setDescription(
		mmgtr("MIDIButtons.MIDI.DisplayLabel").arg(_params->desc).arg(ref_index + 1, 3, 10, QChar('0')));
	return new_self;
}

// End MMGValueStateDisplay<T>

} // namespace MMGWidgets
