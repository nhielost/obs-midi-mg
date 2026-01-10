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

#ifndef MMG_VALUE_DISPLAY_H
#define MMG_VALUE_DISPLAY_H

#include "mmg-combo-box.h"
#include "mmg-dialog-widgets.h"
#include "mmg-lcd-number.h"
#include "mmg-string-widget.h"
#include "mmg-toggle-widget.h"

#include <QButtonGroup>
#include <QPainter>

namespace MMGWidgets {

template <typename T> struct ValueWidgetMap {
	using DisplayType = MMGComboBox;
};
template <typename T> requires MMGIsNumeric<T> struct ValueWidgetMap<T> {
	using DisplayType = MMGLCDNumber;
};
template <> struct ValueWidgetMap<bool> {
	using DisplayType = MMGToggleWidget;
};
template <> struct ValueWidgetMap<QColor> {
	using DisplayType = MMGColorDialogWidget;
};
template <> struct ValueWidgetMap<QFont> {
	using DisplayType = MMGFontDialogWidget;
};
template <> struct ValueWidgetMap<QDir> {
	using DisplayType = MMGPathDialogWidget;
};
template <> struct ValueWidgetMap<QString> {
	using DisplayType = MMGStringWidget;
};

class MMGValueQWidget : public QWidget {
	Q_OBJECT

public:
	MMGValueQWidget(QWidget *parent);
	virtual ~MMGValueQWidget() = default;

signals:
	void editRequested();
	void valueChanged();

public:
	virtual bool hasStorage() const = 0;
	virtual const MMGText &description() const = 0;

	virtual void setModifiable(bool) = 0;
	void setEditingProperty(int8_t value);

	virtual void refresh() = 0;

private:
	void paintEvent(QPaintEvent *) override;
	void mouseDoubleClickEvent(QMouseEvent *e) override;

protected:
	QHBoxLayout *main_layout;
};

template <typename T> class MMGValueDisplay : public MMGValueQWidget {

public:
	MMGValueDisplay(QWidget *parent, const MMGParams<T> *params);
	virtual ~MMGValueDisplay() = default;

	void connectEditor()
	{
		connect(_editor, &MMGValueWidget::valueChanged, this, &MMGValueQWidget::valueChanged,
			Qt::UniqueConnection);
	};

	T value() const;
	void setValue(const T &value);
	bool hasStorage() const override { return false; };

	bool isModifiable() const { return _editor->isModifiable(); };
	void setModifiable(bool modify) override { _editor->setModifiable(modify); };

	void setIncrementing() requires MMGStates::ExtraFeatures<T> { _editor->setIncrementing(); };
	void setItemEnabled(int index,
			    bool enabled) requires std::same_as<typename ValueWidgetMap<T>::DisplayType, MMGComboBox>
	{
		_editor->setItemEnabled(index, enabled);
	};

	const MMGText &description() const final override { return _params->desc; };
	const MMGParams<T> *params() const { return _params; };

	void refresh() override;

protected:
	using MMGValueQWidget::main_layout;

	const MMGParams<T> *_params = nullptr;
	ValueWidgetMap<T>::DisplayType *_editor;
};

template <typename T> class MMGValueFixedDisplay : public MMGValueDisplay<T> {

public:
	using QObject::connect;
	using QObject::property;
	using QObject::setProperty;

	using QWidget::setEnabled;
	using QWidget::setFocus;
	using QWidget::setMinimumWidth;
	using QWidget::setVisible;

public:
	MMGValueFixedDisplay(QWidget *parent, const MMGParams<T> *params);
	MMGValueFixedDisplay(QWidget *parent, const MMGParams<T> *params, MMGValue<T> *storage);
	virtual ~MMGValueFixedDisplay() = default;

	using MMGValueDisplay<T>::connectEditor;

	using MMGValueDisplay<T>::value;
	using MMGValueDisplay<T>::setValue;

	bool hasStorage() const override { return !!_storage; };
	using MMGValueDisplay<T>::setModifiable;
	void setDescription(const QString &desc);
	void refresh() override;

private:
	void setFixedValue();

protected:
	using MMGValueQWidget::main_layout;

	using MMGValueDisplay<T>::_params;
	using MMGValueDisplay<T>::_editor;

	MMGValue<T> *_storage = nullptr;
	QLabel *_label;
};

struct MMGValueStateWidgetInfo {
	virtual ~MMGValueStateWidgetInfo() = default;

	virtual bool isWidgetVisible() const = 0;
	virtual bool isReferenceEligible() const = 0;

	virtual const MMGText &getDescription() const = 0;
	virtual int64_t getStateSize() const = 0;

	virtual bool hasReferenceValue(int64_t index) const = 0;
	virtual MMGValueQWidget *generateFromIndex(QWidget *parent, int64_t index) const = 0;
};

template <typename T> class MMGValueStateDisplay : public MMGValueFixedDisplay<T>, public MMGValueStateWidgetInfo {

public:
	using QObject::connect;
	using QObject::property;
	using QObject::setProperty;

	using QWidget::isEnabled;
	using QWidget::isVisibleTo;
	using QWidget::parentWidget;
	using QWidget::setEnabled;
	using QWidget::setFocus;
	using QWidget::setStyleSheet;
	using QWidget::setVisible;

protected:
	using MMGValueQWidget::setEditingProperty;

	using MMGValueDisplay<T>::setModifiable;
	using MMGValueDisplay<T>::description;
	using MMGValueDisplay<T>::value;
	using MMGValueDisplay<T>::setValue;

public:
	MMGValueStateDisplay(QWidget *parent, const MMGParams<T> *params, MMGValue<T> *storage);
	virtual ~MMGValueStateDisplay() = default;

	MMGValue<T> *storage() const { return _storage; };

	using MMGValueFixedDisplay<T>::setDescription;
	void setModifiable(bool modify) override;

	void refresh() override;

	void resetToFixed() { _storage->template changeTo<STATE_FIXED>()->setValue(_params->default_value); };

protected:
	bool isWidgetVisible() const override
	{
		return !_params->options.testAnyFlags(OPTION_HIDDEN | OPTION_DISABLED);
	};
	bool isReferenceEligible() const override { return _storage->usesReference(); };

	const MMGText &getDescription() const override { return description(); };
	int64_t getStateSize() const override;

	bool hasReferenceValue(int64_t index) const override;
	MMGValueQWidget *generateFromIndex(QWidget *parent, int64_t ref_index) const override;

private:
	using MMGValueQWidget::main_layout;

	using MMGValueDisplay<T>::_params;
	using MMGValueDisplay<T>::_editor;

	using MMGValueFixedDisplay<T>::_storage;
	using MMGValueFixedDisplay<T>::_label;

	QLabel *state_label = nullptr;
};

using MMGValueStateInfoList = QList<MMGValueStateWidgetInfo *>;

} // namespace MMGWidgets

#endif // MMG_VALUE_DISPLAY_H
