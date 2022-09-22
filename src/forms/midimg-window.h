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
#include "../mmg-utils.h"
#include "../mmg-device.h"

class MidiMGWindow : public QDialog {
	Q_OBJECT
public:
	explicit MidiMGWindow(QWidget *parent);
	~MidiMGWindow() override;

private:
	Ui::MidiMGWindow *ui;

	MMGUtils::LCDData lcd_channel;
	MMGUtils::LCDData lcd_note;
	MMGUtils::LCDData lcd_value;
	MMGUtils::LCDData lcd_double1;
	MMGUtils::LCDData lcd_double2;
	MMGUtils::LCDData lcd_double3;
	MMGUtils::LCDData lcd_double4;

	MMGDevice *current_device = nullptr;
	MMGBinding *current_binding = nullptr;
	MMGMessage *current_message = nullptr;
	MMGAction *current_action = nullptr;

	QString help_name;
	QString help_str;

	QString help_action_desc;
	QString help_action_str1;
	QString help_action_str2;
	QString help_action_str3;
	QString help_action_double1;
	QString help_action_double2;
	QString help_action_double3;
	QString help_action_double4;

	/* enum class BindingTransfer {
		MIDIMGWINDOW_KEEP_APPEND,
		MIDIMGWINDOW_REMOVE_APPEND,
		MIDIMGWINDOW_REMOVE_REPLACE
	};*/

	void reject() override;
	void connect_ui_signals();
	void configure_lcd_widgets();

	void switch_structure_pane(enum MMGModes mode);
	void set_help_text(enum MMGModes mode);
	void set_device_view();
	void set_binding_view();
	void set_message_view();
	void set_action_view();
	void set_strs_visible(bool str1 = false, bool str2 = false,
			      bool str3 = false) const;
	void set_doubles_visible(bool double1 = false, bool double2 = false,
				 bool double3 = false,
				 bool double4 = false) const;
	void set_sub_options(std::initializer_list<QString> list) const;
	void set_channel(double value);
	void set_note(double value);
	void set_value(double value);
	void set_str1(const QString &value);
	void set_str2(const QString &value);
	void set_str3(const QString &value);
	void set_double1(double value);
	void set_double2(double value);
	void set_double3(double value);
	void set_double4(double value);
	void add_widget_item(MMGModes type, const QString &name) const;
	void export_bindings();
	void import_bindings();
	void i_need_help() const;
	void report_a_bug() const;

public slots:
	void show_window();
private slots:
	void on_device_active_change(bool toggled);
	void on_binding_reception_select(int index);
	void on_binding_toggling_select(int index);
	void on_message_type_change(const QString &type);
	void on_message_listen(bool toggled);
	void on_message_require_value(bool toggled);
	void on_action_cat_change(int index);
	void on_action_sub_change(int index);
	void on_action_double1_toggle(bool toggle);
	void on_action_double2_toggle(bool toggle);
	void on_action_double3_toggle(bool toggle);
	void on_action_double4_toggle(bool toggle);
	void on_list_selection_change(const QListWidgetItem *current);
	void on_add_click();
	void on_remove_click();
	void on_return_click();
	void on_help_click();
	void on_name_edit(QListWidgetItem *widgetItem);
	void on_element_drag(const QModelIndex &parent, int start, int end,
			     const QModelIndex &dest, int row) const;
};
