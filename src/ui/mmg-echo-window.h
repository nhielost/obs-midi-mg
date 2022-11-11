/*
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

#include "ui_mmg-echo-window.h"
#include "../mmg-utils.h"
#include "../mmg-device.h"

class MMGEchoWindow : public QDialog {
	Q_OBJECT

public:
	explicit MMGEchoWindow(QWidget *parent);
	~MMGEchoWindow() override;

private:
	Ui::MMGEchoWindow *ui;

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

	void reject() override;
	void connect_ui_signals();
	void configure_lcd_widgets();
	void switch_structure_pane(int page);
	void set_message_view();
	void set_action_view();
	void set_preferences_view();
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
	void add_widget_item(const MMGBinding *binding) const;
	void export_bindings();
	void import_bindings();
	void i_need_help() const;
	void report_a_bug() const;

public slots:
	void show_window();
private slots:
	void on_device_change(const QString &name);
	void on_message_type_change(const QString &type);
	void on_message_listen_continuous(bool toggled);
	void on_message_listen_once(bool toggled);
	void on_message_value_button_change(int index);
	void on_action_cat_change(int index);
	void on_action_sub_change(int index);
	void on_action_double1_select(int index);
	void on_action_double2_select(int index);
	void on_action_double3_select(int index);
	void on_action_double4_select(int index);
	void on_active_change(bool toggle);
	void on_preferences_click(bool toggle);
	void on_transfer_mode_change(short index);
	void on_transfer_bindings_click();
	void on_interface_style_change(short index);
	void on_update_check();
	void on_add_click();
	void on_copy_click();
	void on_remove_click();
	void on_list_selection_change(QListWidgetItem *widget_item);
	void on_list_widget_state_change(QListWidgetItem *widget_item);
};
