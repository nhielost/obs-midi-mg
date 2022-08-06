/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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

#pragma once
#include <QDialog>

#include "ui_midimg-window.h"
#include "../mmg-device.h"

struct LCDData;

class MidiMGWindow : public QDialog {
	Q_OBJECT
public:
	explicit MidiMGWindow(QWidget *parent);
	~MidiMGWindow() override;

private:
	Ui::MidiMGWindow *ui;

	enum class LCDButtons {
		MIDIMGWINDOW_NEUTRAL_RESET,
		MIDIMGWINDOW_DOWN_MAJOR,
		MIDIMGWINDOW_DOWN_MINOR,
		MIDIMGWINDOW_UP_MINOR,
		MIDIMGWINDOW_UP_MAJOR
	};

	/* enum class BindingTransfer {
		MIDIMGWINDOW_KEEP_APPEND,
		MIDIMGWINDOW_REMOVE_APPEND,
		MIDIMGWINDOW_REMOVE_REPLACE
	};*/

	void reject() override;
	void connect_ui_signals();
	void configure_lcd_widgets();

	const QStringList get_device_names() const;
	QString binding_mode_description(enum MMGBinding::Mode mode) const;
	void set_lcd_value(QLCDNumber *lcd, enum LCDButtons kind) const;
	void display_lcd_value(QLCDNumber *lcd, const LCDData &data) const;
	void switch_structure_pane(enum MMGModes mode);
	void set_message_view();
	void set_action_view();
	void set_sub_visible(bool visible = false) const;
	void set_lists_visible(bool str1 = false, bool str2 = false,
			       bool str3 = false) const;
	void set_doubles_visible(bool double1 = false, bool double2 = false,
				 bool double3 = false,
				 bool double4 = false) const;
	void set_doubles_usevalue(short which, bool disabled) const;
	void set_sub_options(std::initializer_list<QString> list) const;
	void set_message_channel(double value);
	void set_message_note(double value);
	void set_message_value(double value);
	void set_list1(const QString &value);
	void set_list2(const QString &value);
	void set_list3(const QString &value);
	void set_double1(double value);
	void set_double2(double value);
	void set_double3(double value);
	void set_double4(double value);
	void add_widget_item(MMGModes type, const QString &name) const;
	void export_bindings();
	void import_bindings();
	void i_need_help() const;
	void report_a_bug() const;

	MMGDevice *current_device = nullptr;
	MMGBinding *current_binding = nullptr;
	MMGMessage *current_message = nullptr;
	MMGAction *current_action = nullptr;

public slots:
	void show_window();
private slots:
	void on_message_type_change(const QString &type);
	void on_action_cat_change(const QString &cat);
	void on_action_sub_change(int index);
	void on_list_selection_change(const QListWidgetItem *current);
	void on_add_click();
	void on_remove_click();
	void on_return_click();
	void on_binding_mode_select(enum MMGBinding::Mode mode);
	void on_name_edit(QListWidgetItem *widgetItem);
	void on_element_drag(const QModelIndex &parent, int start, int end,
			     const QModelIndex &dest, int row) const;
};
