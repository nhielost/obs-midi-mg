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
#include "mmg-fields.h"
#include "../mmg-device.h"

#define IF_ACTION_ENABLED if (!action_display_params.initializing)

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

  QList<MMGFields *> field_groups;
  MMGUtils::MMGActionDisplayParams action_display_params;

  MMGDevice *current_device = nullptr;
  MMGBinding *current_binding = nullptr;
  MMGMessage *current_message = nullptr;
  MMGAction *current_action = nullptr;

  void reject() override;
  void connect_ui_signals();
  void switch_structure_pane(int page);
  void set_message_view();
  void set_action_view();
  void set_preferences_view();
  void change_sub_options();
  void set_channel(double value) { current_message->channel() = value; }
  void set_note(double value)
  {
    (ui->lcd_value->isVisible() ? current_message->note() : current_message->value()) = value;
  };
  void set_value(double value) { current_message->value() = value; };
  void set_str1(const QString &value) { IF_ACTION_ENABLED current_action->str1() = value; };
  void set_str2(const QString &value) { IF_ACTION_ENABLED current_action->str2() = value; };
  void set_str3(const QString &value) { IF_ACTION_ENABLED current_action->str3() = value; };
  void set_double1(double value) { IF_ACTION_ENABLED current_action->num1() = value; };
  void set_double2(double value) { IF_ACTION_ENABLED current_action->num2() = value; };
  void set_double3(double value) { IF_ACTION_ENABLED current_action->num3() = value; };
  void set_double4(double value) { IF_ACTION_ENABLED current_action->num4() = value; };
  void display_action_fields();
  void change_str1_options(const QString &value);
  void change_str2_options(const QString &value);
  void change_str3_options(const QString &value);
  void change_action_secondary();
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
  void on_binding_drag(const QModelIndex &parent, int start, int end, const QModelIndex &dest,
		       int row) const;
};
