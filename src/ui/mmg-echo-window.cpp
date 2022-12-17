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

#include <obs-frontend-api.h>

#include <QListWidget>
#include <QStandardItemModel>
#include <QEvent>
#include <QLayout>
#include <QFileDialog>
#include <QDesktopServices>

#include "mmg-echo-window.h"
#include "../mmg-config.h"

using namespace MMGUtils;

#define SET_LCD_STATUS(lcd, kind, status)  \
  ui->down_major_##lcd->set##kind(status); \
  ui->down_minor_##lcd->set##kind(status); \
  ui->up_minor_##lcd->set##kind(status);   \
  ui->up_major_##lcd->set##kind(status);   \
  ui->label_##lcd->set##kind(status);      \
  ui->lcd_##lcd->set##kind(status)

#define CONNECT_LCD(kind)                                                                          \
  connect(ui->down_major_##kind, &QAbstractButton::clicked, this,                                  \
	  [&]() { lcd_##kind.down_major(); });                                                     \
  connect(ui->down_minor_##kind, &QAbstractButton::clicked, this,                                  \
	  [&]() { lcd_##kind.down_minor(); });                                                     \
  connect(ui->up_minor_##kind, &QAbstractButton::clicked, this, [&]() { lcd_##kind.up_minor(); }); \
  connect(ui->up_major_##kind, &QAbstractButton::clicked, this, [&]() { lcd_##kind.up_major(); }); \
  connect(ui->editor_##kind, QOverload<int>::of(&QComboBox::currentIndexChanged), this,            \
	  [&](int index) {                                                                         \
	    SET_LCD_STATUS(kind, Enabled, index == 0);                                             \
	    lcd_##kind.set_state(index);                                                           \
	  })

#define INIT_LCD(kind) lcd_##kind = LCDData(ui->lcd_##kind)

#define COMBOBOX_ITEM_STATE(list, index, state) \
  qobject_cast<QStandardItemModel *>(ui->list->model())->item(index)->setEnabled(state);

MMGEchoWindow::MMGEchoWindow(QWidget *parent)
  : QDialog(parent, Qt::Dialog), ui(new Ui::MMGEchoWindow)
{
  this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
  ui->setupUi(this);
  ui->label_version->setText(OBS_MIDIMG_VERSION);

  ui->editor_channel->setVisible(false);

  INIT_LCD(channel);
  INIT_LCD(note);
  INIT_LCD(value);
  INIT_LCD(double1);
  INIT_LCD(double2);
  INIT_LCD(double3);
  INIT_LCD(double4);

  // Channel settings
  lcd_channel.set_range(1.0, 16.0);
  lcd_channel.set_step(1.0, 5.0);
  // Note / Value settings
  lcd_note.set_range(0.0, 127.0);
  lcd_value.set_range(0.0, 127.0);
  // Doubles settings
  lcd_double1.set_step(0.1, 1.0);
  lcd_double2.set_step(0.1, 1.0);
  lcd_double3.set_step(0.1, 1.0);
  lcd_double4.set_step(0.1, 1.0);

  action_display_params.lcds[0] = &lcd_double1;
  action_display_params.lcds[1] = &lcd_double2;
  action_display_params.lcds[2] = &lcd_double3;
  action_display_params.lcds[3] = &lcd_double4;

  connect_ui_signals();
}

void MMGEchoWindow::show_window()
{
  global()->load_new_devices();

  QString current_device_name = global()->get_active_device_name();

  ui->editor_transfer_source->clear();
  ui->editor_transfer_dest->clear();
  ui->editor_transfer_mode->setCurrentIndex(0);
  ui->editor_devices->clear();
  on_transfer_mode_change(0);
  for (const QString &name : global()->get_device_names()) {
    ui->editor_devices->addItem(name);
    ui->editor_transfer_source->addItem(name);
    ui->editor_transfer_dest->addItem(name);
  }

  ui->editor_devices->setCurrentText(current_device_name);

  ui->editor_structure->clearSelection();
  switch_structure_pane(1);
  ui->button_preferences->setChecked(false);
  ui->pages->setCurrentIndex(0);

  setVisible(true);
}

void MMGEchoWindow::reject()
{
  if (global()->is_listening(nullptr)) {
    open_message_box(
      "Error",
      "Cannot close window: The MIDI device is being listened to.\n(A Listen button is on.)");
    return;
  }
  global()->save();
  QDialog::reject();
}

void MMGEchoWindow::connect_ui_signals()
{
  // LCD Connections
  CONNECT_LCD(channel);
  CONNECT_LCD(note);
  CONNECT_LCD(value);
  CONNECT_LCD(double1);
  CONNECT_LCD(double2);
  CONNECT_LCD(double3);
  CONNECT_LCD(double4);
  // Message Display Connections
  connect(ui->editor_type, &QComboBox::currentTextChanged, this,
	  &MMGEchoWindow::on_message_type_change);
  connect(ui->button_listen_continuous, &QAbstractButton::toggled, this,
	  &MMGEchoWindow::on_message_listen_continuous);
  connect(ui->button_listen_once, &QAbstractButton::toggled, this,
	  &MMGEchoWindow::on_message_listen_once);
  connect(ui->editor_value, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
	  &MMGEchoWindow::on_message_value_button_change);
  connect(ui->editor_note, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
	  &MMGEchoWindow::on_message_value_button_change);
  // Action Display Connections
  connect(ui->editor_cat, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
	  &MMGEchoWindow::on_action_cat_change);
  connect(ui->editor_sub, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
	  &MMGEchoWindow::on_action_sub_change);
  connect(ui->editor_str1, &QComboBox::currentTextChanged, this,
	  &MMGEchoWindow::change_str1_options);
  connect(ui->editor_str2, &QComboBox::currentTextChanged, this,
	  &MMGEchoWindow::change_str2_options);
  connect(ui->editor_str3, &QComboBox::currentTextChanged, this,
	  &MMGEchoWindow::change_str3_options);
  // UI Movement Buttons
  connect(ui->button_add, &QPushButton::clicked, this, &MMGEchoWindow::on_add_click);
  connect(ui->button_duplicate, &QPushButton::clicked, this, &MMGEchoWindow::on_copy_click);
  connect(ui->button_remove, &QPushButton::clicked, this, &MMGEchoWindow::on_remove_click);
  // Device Connections
  connect(ui->editor_devices, &QComboBox::currentTextChanged, this,
	  &MMGEchoWindow::on_device_change);
  // Preferences Connections
  connect(ui->button_preferences, &QAbstractButton::toggled, this,
	  &MMGEchoWindow::on_preferences_click);
  connect(ui->editor_global_enable, &QCheckBox::toggled, this, &MMGEchoWindow::on_active_change);
  connect(ui->editor_interface_style, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
	  &MMGEchoWindow::on_interface_style_change);
  connect(ui->button_export, &QPushButton::clicked, this, &MMGEchoWindow::export_bindings);
  connect(ui->button_import, &QPushButton::clicked, this, &MMGEchoWindow::import_bindings);
  connect(ui->button_help_advanced, &QPushButton::clicked, this, &MMGEchoWindow::i_need_help);
  connect(ui->button_bug_report, &QPushButton::clicked, this, &MMGEchoWindow::report_a_bug);
  connect(ui->button_update_check, &QPushButton::clicked, this, &MMGEchoWindow::on_update_check);

  connect(ui->editor_transfer_mode, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
	  &MMGEchoWindow::on_transfer_mode_change);
  connect(ui->button_binding_transfer, &QPushButton::clicked, this,
	  &MMGEchoWindow::on_transfer_bindings_click);

  // List Widget Connections
  connect(ui->editor_structure, &QListWidget::itemClicked, this,
	  &MMGEchoWindow::on_list_selection_change);
  connect(ui->editor_structure, &QListWidget::itemChanged, this,
	  &MMGEchoWindow::on_list_widget_state_change);
  connect(ui->editor_structure->model(), &QAbstractItemModel::rowsMoved, this,
	  &MMGEchoWindow::on_binding_drag);
}

void MMGEchoWindow::set_message_view()
{
  lcd_channel.set_storage(&current_message->channel());
  lcd_note.set_storage(&current_message->note());
  lcd_value.set_storage(&current_message->value());

  if (current_message->type().state() == MMGString::STRINGSTATE_TOGGLE) {
    on_message_type_change("Note On / Note Off");
  } else {
    on_message_type_change(current_message->type());
  }
}

void MMGEchoWindow::set_action_view()
{
  // Preparations (turn off action editing)
  action_display_params.initializing = true;
  lcd_double1.can_set(false);
  lcd_double2.can_set(false);
  lcd_double3.can_set(false);
  lcd_double4.can_set(false);

  // Set storage locations for LCDData objects to set to
  lcd_double1.set_storage(&current_action->num1());
  lcd_double2.set_storage(&current_action->num2());
  lcd_double3.set_storage(&current_action->num3());
  lcd_double4.set_storage(&current_action->num4());

  // Set category (and add sub list)
  ui->editor_cat->setCurrentIndex((int)current_action->get_category());
  change_sub_options();

  // Set subcategory view
  ui->editor_sub->setCurrentIndex(current_action->get_sub());
  on_action_sub_change(current_action->get_sub());

  // Set strings (even if they are invalid)
  ui->editor_str1->setCurrentText(current_action->str1());
  change_str1_options(current_action->str1());
  ui->editor_str2->setCurrentText(current_action->str2());
  change_str2_options(current_action->str2());
  ui->editor_str3->setCurrentText(current_action->str3());
  change_str3_options(current_action->str3());

  // Set LCD ComboBox indices to match their respective states
  ui->editor_double1->setCurrentIndex(current_action->num1().state());
  ui->editor_double2->setCurrentIndex(current_action->num2().state());
  ui->editor_double3->setCurrentIndex(current_action->num3().state());
  ui->editor_double4->setCurrentIndex(current_action->num4().state());

  // Turn on action editing so that LCDData's and potential MMGFields can be set
  action_display_params.initializing = false;
  lcd_double1.can_set(true);
  lcd_double2.can_set(true);
  lcd_double3.can_set(true);
  lcd_double4.can_set(true);

  change_action_secondary();
}

void MMGEchoWindow::set_preferences_view()
{
  ui->editor_global_enable->setChecked(global()->preferences().get_active());
}

void MMGEchoWindow::on_device_change(const QString &name)
{
  if (name.isEmpty()) return;
  current_device = global()->find_device(name);
  global()->set_active_device_name(name);
  switch_structure_pane(1);
}

void MMGEchoWindow::on_message_type_change(const QString &type)
{
  lcd_note.set_storage(&current_message->note());
  SET_LCD_STATUS(note, Enabled, true);
  SET_LCD_STATUS(value, Visible, true);
  SET_LCD_STATUS(value, Enabled, true);
  ui->editor_note->setVisible(true);
  ui->editor_value->setVisible(true);
  COMBOBOX_ITEM_STATE(editor_value, 2, false);

  ui->editor_value->setCurrentIndex(current_message->value().state());
  on_message_value_button_change(current_message->value().state());

  current_message->type() = type;
  ui->editor_type->setCurrentText(type);

  if (current_action) {
    if (current_action->get_category() == MMGAction::Category::MMGACTION_MIDI &&
	current_action->str2().state() != 0) {
      MMGString::State state = current_action->str2().state();
      change_str2_options(type);
      current_action->str2().set_state(state);
    }
  }

  if (type == "Note On / Note Off") {
    ui->editor_type->setCurrentText("Note On / Note Off");
    current_message->type() = "Note On";
    ui->label_note->setText("Note #");
    ui->label_value->setText("Velocity");
    current_message->type().set_state(MMGString::STRINGSTATE_TOGGLE);
    COMBOBOX_ITEM_STATE(editor_value, 2, true);
    ui->editor_note->setVisible(false);
    lcd_value.display();
    return;
  }

  current_message->type().set_state(MMGString::STRINGSTATE_FIXED);

  if (type == "Note On" || type == "Note Off") {
    ui->label_note->setText("Note #");
    ui->label_value->setText("Velocity");
    ui->editor_note->setVisible(false);
    COMBOBOX_ITEM_STATE(editor_value, 2, true);
    lcd_value.display();
  } else if (type == "Control Change") {
    ui->label_note->setText("Control #");
    ui->label_value->setText("Value");
    ui->editor_note->setVisible(false);
    lcd_value.display();
  } else if (type == "Program Change") {
    ui->label_note->setText("Program #");
    SET_LCD_STATUS(value, Visible, false);
    ui->editor_value->setVisible(false);
    ui->editor_note->setCurrentIndex(current_message->value().state());
    lcd_note.set_storage(&current_message->value());
  } else if (type == "Pitch Bend") {
    ui->label_note->setText("Pitch Adjust");
    SET_LCD_STATUS(value, Visible, false);
    ui->editor_value->setVisible(false);
    ui->editor_note->setCurrentIndex(current_message->value().state());
    lcd_note.set_storage(&current_message->value());
  }
}

void MMGEchoWindow::on_message_listen_once(bool toggled)
{
  global()->set_listening(toggled);
  ui->button_listen_once->setText(toggled ? "Cancel..." : "Listen Once...");
  if (!toggled) return;
  global()->set_listening_callback([this](MMGMessage *incoming) {
    if (!incoming) return;
    // Check the validity of the message type (whether it is one of the five
    // supported types)
    if (ui->editor_type->findText(incoming->type()) == -1) return;
    global()->set_listening(false);
    ui->button_listen_once->setText("Listen Once...");
    ui->button_listen_once->setChecked(false);
    incoming->deep_copy(current_message);
    set_message_view();
  });
}

void MMGEchoWindow::on_message_listen_continuous(bool toggled)
{
  global()->set_listening(toggled);
  ui->button_listen_continuous->setText(toggled ? "Cancel..." : "Listen Continuous...");
  if (!toggled) return;
  global()->set_listening_callback([this](MMGMessage *incoming) {
    if (!incoming) return;
    // Check the validity of the message type (whether it is one of the five
    // supported types)
    if (ui->editor_type->findText(incoming->type()) == -1) return;
    incoming->deep_copy(current_message);
    set_message_view();
  });
}

void MMGEchoWindow::on_message_value_button_change(int index)
{
  current_message->value().set_state((MMGNumber::State)index);

  if (ui->editor_type->currentText() == "Program Change" ||
      ui->editor_type->currentText() == "Pitch Bend") {
    if (index == 2) current_message->value().set_state(MMGNumber::NUMBERSTATE_MIDI);
    SET_LCD_STATUS(note, Enabled, index == 0);
  } else {
    if (index == 2) current_message->value() = 127;
    SET_LCD_STATUS(value, Enabled, index == 0);
  }
  lcd_note.display();
  lcd_value.display();
}

void MMGEchoWindow::on_action_cat_change(int index)
{
  IF_ACTION_ENABLED current_binding->set_action_type(index);
  IF_ACTION_ENABLED current_action = current_binding->get_action();
  lcd_double1.set_storage(&current_action->num1());
  lcd_double2.set_storage(&current_action->num2());
  lcd_double3.set_storage(&current_action->num3());
  lcd_double4.set_storage(&current_action->num4());
  change_sub_options();
}

void MMGEchoWindow::change_sub_options()
{
  ui->editor_sub->clear();
  MMGActionDisplayParams params;
  current_action->change_options_sub(params);
  ui->editor_sub->addItems(params.list);
  ui->editor_sub->setCurrentIndex(0);
}

void MMGEchoWindow::on_action_sub_change(int index)
{
  IF_ACTION_ENABLED current_action->set_sub(index);
  action_display_params.clear();
  QString current_field_vals[3] = {ui->editor_str1->currentText(), ui->editor_str2->currentText(),
				   ui->editor_str3->currentText()};
  ui->editor_str1->clear();
  ui->editor_str2->clear();
  ui->editor_str3->clear();
  ui->editor_double1->setCurrentIndex(0);
  ui->editor_double2->setCurrentIndex(0);
  ui->editor_double3->setCurrentIndex(0);
  ui->editor_double4->setCurrentIndex(0);
  lcd_double1.set_state(0);
  lcd_double2.set_state(0);
  lcd_double3.set_state(0);
  lcd_double4.set_state(0);
  lcd_double1.set_use_time(false);
  ui->action_secondary_fields->setCurrentIndex(0);

  switch (current_action->get_category()) {
    case MMGAction::Category::MMGACTION_MIDI:
      action_display_params.extra_data = current_message->type();
      break;
    case MMGAction::Category::MMGACTION_INTERNAL:
      action_display_params.extra_data = current_binding->get_name();
      break;
    default:
      break;
  }
  current_action->change_options_str1(action_display_params);
  display_action_fields();
  ui->label_str1->setText(action_display_params.label_text);
  ui->editor_str1->addItems(action_display_params.list);

  lcd_double1.reset();
  lcd_double2.reset();
  lcd_double3.reset();
  lcd_double4.reset();
  if (!current_field_vals[0].isEmpty()) ui->editor_str1->setCurrentText(current_field_vals[0]);
  if (!current_field_vals[1].isEmpty()) ui->editor_str2->setCurrentText(current_field_vals[1]);
  if (!current_field_vals[2].isEmpty()) ui->editor_str3->setCurrentText(current_field_vals[2]);
  change_action_secondary();
}

void MMGEchoWindow::change_str1_options(const QString &value)
{
  if (value.isEmpty()) return;
  set_str1(value);
  ui->editor_str2->clear();

  switch (current_action->get_category()) {
    case MMGAction::Category::MMGACTION_MIDI:
      action_display_params.extra_data = current_message->type();
      break;
    case MMGAction::Category::MMGACTION_INTERNAL:
      action_display_params.extra_data = current_binding->get_name();
      break;
    default:
      break;
  }
  current_action->change_options_str2(action_display_params);
  display_action_fields();
  ui->label_str2->setText(action_display_params.label_text);
  ui->editor_str2->addItems(action_display_params.list);
}

void MMGEchoWindow::change_str2_options(const QString &value)
{
  ui->action_secondary_fields->setCurrentIndex(0);
  if (value.isEmpty()) return;
  set_str2(value);
  ui->editor_str3->clear();

  switch (current_action->get_category()) {
    case MMGAction::Category::MMGACTION_MIDI:
      action_display_params.extra_data = current_message->type();
      break;
    case MMGAction::Category::MMGACTION_INTERNAL:
      action_display_params.extra_data = current_binding->get_name();
      break;
    default:
      break;
  }
  current_action->change_options_str3(action_display_params);
  display_action_fields();
  ui->label_str3->setText(action_display_params.label_text);
  ui->editor_str3->addItems(action_display_params.list);
}

void MMGEchoWindow::change_str3_options(const QString &value)
{
  if (value.isEmpty()) return;
  set_str3(value);
  current_action->change_options_final(action_display_params);
  display_action_fields();
}

void MMGEchoWindow::change_action_secondary()
{
  if (action_display_params.initializing) return;

  int index = 0;
  MMGFields::Kind kind;

  switch (current_action->get_category()) {
    case MMGAction::Category::MMGACTION_FILTER:
      if (current_action->get_sub() != 4) return;
      kind = MMGFields::MMGFIELDS_FILTER;
      break;
    case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
      if (current_action->get_sub() != 13) return;
      kind = MMGFields::MMGFIELDS_SOURCE;
      break;
    case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
      if (current_action->get_sub() != 7) return;
      kind = MMGFields::MMGFIELDS_SOURCE;
      break;
    case MMGAction::Category::MMGACTION_TRANSITION:
      if (current_action->get_sub() != 4) return;
      kind = MMGFields::MMGFIELDS_TRANSITION;
      break;
    default:
      return;
  }

  for (MMGFields *fields : field_groups) {
    if (fields->ensure_validity(current_action)) {
      index = fields->get_index();
      fields->json();
      break;
    }
  }

  if (index == 0) { // Index will never be 0 if the fields exist (it's the index of the doubles)
    field_groups.append(new MMGFields(kind, ui->action_secondary_fields, current_action));
    index = field_groups.last()->get_index();
  }

  ui->action_secondary_fields->setCurrentIndex(index);
}

void MMGEchoWindow::display_action_fields()
{
  if (action_display_params.display & MMGActionDisplayParams::DISPLAY_SEC) {
    change_action_secondary();
    return;
  }

  ui->label_str3->setVisible(false);
  ui->editor_str3->setVisible(false);
  ui->label_str2->setVisible(false);
  ui->editor_str2->setVisible(false);
  ui->label_str1->setVisible(false);
  ui->editor_str1->setVisible(false);

  SET_LCD_STATUS(double4, Visible, false);
  ui->editor_double4->setVisible(false);
  ui->sep_action_3->setVisible(false);
  SET_LCD_STATUS(double3, Visible, false);
  ui->editor_double3->setVisible(false);
  ui->sep_action_2->setVisible(false);
  SET_LCD_STATUS(double2, Visible, false);
  ui->editor_double2->setVisible(false);
  ui->sep_action_1->setVisible(false);
  SET_LCD_STATUS(double1, Visible, false);
  ui->editor_double1->setVisible(false);

  switch (action_display_params.display & 0b111) {
    case MMGActionDisplayParams::DISPLAY_STR3:
      ui->label_str3->setVisible(true);
      ui->editor_str3->setVisible(true);
      [[fallthrough]];
    case MMGActionDisplayParams::DISPLAY_STR2:
      ui->label_str2->setVisible(true);
      ui->editor_str2->setVisible(true);
      [[fallthrough]];
    case MMGActionDisplayParams::DISPLAY_STR1:
      ui->label_str1->setVisible(true);
      ui->editor_str1->setVisible(true);
      break;
    default:
      break;
  }

  switch (action_display_params.display & 0b1111000) {
    case MMGActionDisplayParams::DISPLAY_NUM4:
      SET_LCD_STATUS(double4, Visible, true);
      ui->editor_double4->setVisible(true);
      ui->sep_action_3->setVisible(true);
      [[fallthrough]];
    case MMGActionDisplayParams::DISPLAY_NUM3:
      SET_LCD_STATUS(double3, Visible, true);
      ui->editor_double3->setVisible(true);
      ui->sep_action_2->setVisible(true);
      [[fallthrough]];
    case MMGActionDisplayParams::DISPLAY_NUM2:
      SET_LCD_STATUS(double2, Visible, true);
      ui->editor_double2->setVisible(true);
      ui->sep_action_1->setVisible(true);
      [[fallthrough]];
    case MMGActionDisplayParams::DISPLAY_NUM1:
      SET_LCD_STATUS(double1, Visible, true);
      ui->editor_double1->setVisible(true);
      break;
    default:
      break;
  }

  ui->label_double1->setText(action_display_params.label_lcds[0]);
  ui->label_double2->setText(action_display_params.label_lcds[1]);
  ui->label_double3->setText(action_display_params.label_lcds[2]);
  ui->label_double4->setText(action_display_params.label_lcds[3]);

  for (int i = 0; i < 3; i++) {
    COMBOBOX_ITEM_STATE(editor_double1, i + 1,
			action_display_params.combo_display[0] &
			  (MMGActionDisplayParams::LCDComboDisplay)(1 << i));
    COMBOBOX_ITEM_STATE(editor_double2, i + 1,
			action_display_params.combo_display[1] &
			  (MMGActionDisplayParams::LCDComboDisplay)(1 << i));
    COMBOBOX_ITEM_STATE(editor_double3, i + 1,
			action_display_params.combo_display[2] &
			  (MMGActionDisplayParams::LCDComboDisplay)(1 << i));
    COMBOBOX_ITEM_STATE(editor_double4, i + 1,
			action_display_params.combo_display[3] &
			  (MMGActionDisplayParams::LCDComboDisplay)(1 << i));
  }
}

void MMGEchoWindow::on_add_click()
{
  add_widget_item(current_device->add());
}

void MMGEchoWindow::on_copy_click()
{
  add_widget_item(current_device->copy(current_binding));
}

void MMGEchoWindow::on_remove_click()
{
  QListWidgetItem *current = ui->editor_structure->currentItem();
  if (!current) return;
  current_device->remove(current_binding);
  delete current_binding;

  delete current;

  current = ui->editor_structure->currentItem();
  if (!current) {
    on_list_selection_change(nullptr);
    return;
  }

  on_list_selection_change(current);
}

void MMGEchoWindow::on_preferences_click(bool toggle)
{
  switch_structure_pane(1 + toggle);
  ui->button_preferences->setText(toggle ? "Return to Bindings..." : "Preferences...");
  ui->editor_devices->setDisabled(toggle);
}

void MMGEchoWindow::on_list_widget_state_change(QListWidgetItem *widget_item)
{
  if (!current_binding || !widget_item) return;

  if (ui->editor_structure->currentRow() != ui->editor_structure->row(widget_item)) {
    ui->editor_structure->setCurrentItem(widget_item);
    on_list_selection_change(widget_item);
  }

  if (current_binding->get_name() != widget_item->text()) {
    QString name{widget_item->text()};
    if (!!current_device->find_binding(name)) {
      ui->editor_structure->currentItem()->setText(current_binding->get_name());
      return;
    }
    current_binding->set_name(name);
  }

  current_binding->set_enabled((bool)widget_item->checkState());
}

void MMGEchoWindow::on_list_selection_change(QListWidgetItem *widget_item)
{
  if (!widget_item) {
    // No selection
    ui->button_duplicate->setEnabled(false);
    ui->button_remove->setEnabled(false);
    ui->pages->setCurrentIndex(0);
    return;
  }

  ui->button_add->setEnabled(true);
  ui->button_duplicate->setEnabled(true);
  ui->button_remove->setEnabled(true);

  current_binding = current_device->find_binding(widget_item->text());
  current_message = current_binding->get_message();
  current_action = current_binding->get_action();
  set_message_view();
  set_action_view();
  ui->pages->setCurrentIndex(1);
}

void MMGEchoWindow::switch_structure_pane(int page)
{
  ui->editor_structure->clear();
  ui->button_add->setEnabled(false);
  ui->button_duplicate->setEnabled(false);
  ui->button_remove->setEnabled(false);
  ui->pages->setCurrentIndex(page);

  if (page != 1) return;

  ui->button_add->setEnabled(true);
  if (!current_device) return;
  for (MMGBinding *const binding_el : current_device->get_bindings()) {
    add_widget_item(binding_el);
  }
  on_list_selection_change(nullptr);
}

void MMGEchoWindow::add_widget_item(const MMGBinding *binding) const
{
  QListWidgetItem *new_item = new QListWidgetItem;
  new_item->setFlags((Qt::ItemFlag)0b110111);
  new_item->setCheckState((Qt::CheckState)(binding->get_enabled() ? 2 : 0));
  new_item->setText(binding->get_name());
  ui->editor_structure->addItem(new_item);
}

void MMGEchoWindow::on_binding_drag(const QModelIndex &parent, int start, int end,
				    const QModelIndex &dest, int row) const
{
  Q_UNUSED(parent);
  Q_UNUSED(end);
  Q_UNUSED(dest);
  current_device->move(start, row);
}

void MMGEchoWindow::on_active_change(bool toggle)
{
  global()->preferences().set_active(toggle);
}

void MMGEchoWindow::export_bindings()
{
  QString filepath = QFileDialog::getSaveFileName(this, tr("Save Bindings..."),
						  MMGConfig::get_filepath(), "JSON Files (*.json)");
  if (!filepath.isNull()) global()->save(filepath);
}

void MMGEchoWindow::import_bindings()
{
  QString filepath =
    QFileDialog::getOpenFileName(this, tr("Open Bindings File..."), "", "JSON Files (*.json)");
  if (!filepath.isNull()) {
    global()->load(filepath);
    show_window();
  }
}

void MMGEchoWindow::i_need_help() const
{
  QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/blob/master/HELP.md"));
}

void MMGEchoWindow::report_a_bug() const
{
  QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/issues"));
}

void MMGEchoWindow::on_transfer_mode_change(short index)
{
  switch (index) {
    case 0:
      ui->text_transfer_mode->setText(
	"In this mode, bindings will be copied from the source device "
	"to the destination device. "
	"The destination device will then contain both device's bindings.");
      break;
    case 1:
      ui->text_transfer_mode->setText(
	"In this mode, bindings will be removed from the source device "
	"and added to the destination device. "
	"The destination device will then contain both device's bindings.");
      break;
    case 2:
      ui->text_transfer_mode->setText(
	"In this mode, bindings will be removed from the source device "
	"and will replace existing bindings in the destination device. "
	"NOTE: This will remove all existing bindings in the destination device.");
      break;
  }
}

void MMGEchoWindow::on_transfer_bindings_click()
{
  transfer_bindings(ui->editor_transfer_mode->currentIndex(),
		    ui->editor_transfer_source->currentText(),
		    ui->editor_transfer_dest->currentText());
}

void MMGEchoWindow::on_interface_style_change(short index)
{ /* global()->preferences().set_ui_style(index); */
}

void MMGEchoWindow::on_update_check()
{
  QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/releases"));
}

MMGEchoWindow::~MMGEchoWindow()
{
  delete ui;
  qDeleteAll(field_groups);
}
