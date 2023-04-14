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
#include "mmg-number-display.h"
#include "mmg-fields.h"
#include "../mmg-device.h"

class MMGEchoWindow : public QDialog {
  Q_OBJECT

  public:
  explicit MMGEchoWindow(QWidget *parent);
  ~MMGEchoWindow() override;

  private:
  Ui::MMGEchoWindow *ui;

  MMGNumberDisplay *channel_display;
  MMGNumberDisplay *note_display;
  MMGNumberDisplay *value_display;

  MMGBinding *current_binding = nullptr;
  MMGMessage *current_message = nullptr;
  MMGAction *current_action = nullptr;

  QList<MMGOBSFields *> custom_fields;
  MMGOBSFields *current_fields = nullptr;

  short listening_mode = 0;

  void reject() override;
  void connect_ui_signals();
  void translate();
  void switch_structure_pane(int page);
  void set_message_view();
  void set_action_view();
  void set_preferences_view();
  void change_sub_options();
  void add_widget_item(const MMGBinding *binding) const;
  void export_bindings();
  void import_bindings();
  void i_need_help() const;
  void report_a_bug() const;

  public slots:
  void show_window();
  void listen_update(const MMGSharedMessage &);
  void custom_field_request(void *, MMGUtils::MMGString *);

  private slots:
  void on_device_change(const QString &name);
  void on_message_type_change(const QString &type);
  void on_message_listen_continuous(bool toggled);
  void on_message_listen_once(bool toggled);
  void on_message_type_toggle(bool toggled);
  void on_message_value_toggle(bool toggled);
  void on_action_cat_change(int index);
  void on_action_sub_change(int index);
  void on_preferences_click(bool toggle);
  void on_active_change(bool toggle);
  void on_midi_thru_device_change(const QString &device);
  void on_internal_behavior_change(int index);
  void on_transfer_mode_change(short index);
  void on_transfer_bindings_click();
  void on_update_check();
  void on_add_click();
  void on_copy_click();
  void on_remove_click();
  void on_list_selection_change(QListWidgetItem *widget_item);
  void on_list_widget_state_change(QListWidgetItem *widget_item);
  void on_binding_drag(const QModelIndex &parent, int start, int end, const QModelIndex &dest,
		       int row) const;
};
