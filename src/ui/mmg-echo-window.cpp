/*
obs-midi-mg
Copyright (C) 2022-2023 nhielost <nhielost@gmail.com>

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
#include "../mmg-midiin.h"
#include "../mmg-midiout.h"
#include "../actions/mmg-action-midi.h"

using namespace MMGUtils;

#define COMBOBOX_ITEM_STATE(list, index, state) \
  qobject_cast<QStandardItemModel *>(ui->list->model())->item(index)->setEnabled(state);

MMGEchoWindow::MMGEchoWindow(QWidget *parent)
  : QDialog(parent, Qt::Dialog), ui(new Ui::MMGEchoWindow)
{
  setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
  ui->setupUi(this);
  ui->label_version->setText(OBS_MIDIMG_VERSION);

  channel_display = new MMGNumberDisplay(ui->message_frame);
  channel_display->setDisplayMode(MMGNumberDisplay::MODE_THIN);
  channel_display->move(250, 70);
  channel_display->setDescription("Channel");
  channel_display->setBounds(1, 16);

  note_display = new MMGNumberDisplay(ui->message_frame);
  note_display->setDisplayMode(MMGNumberDisplay::MODE_THIN);
  note_display->move(250, 120);
  note_display->setDescription("Note");
  note_display->setBounds(0, 127);

  value_display = new MMGNumberDisplay(ui->message_frame);
  value_display->setDisplayMode(MMGNumberDisplay::MODE_DEFAULT);
  value_display->move(250, 170);
  value_display->setDescription("Velocity");
  value_display->setBounds(0, 127);
  value_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_CUSTOM);

  connect_ui_signals();
}

void MMGEchoWindow::show_window()
{
  global()->refresh();

  QString current_device_name = global()->activeDeviceName();

  ui->editor_transfer_source->clear();
  ui->editor_transfer_dest->clear();
  ui->editor_transfer_mode->setCurrentIndex(0);
  ui->editor_devices->clear();
  on_transfer_mode_change(0);
  for (const QString &name : global()->allDeviceNames()) {
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
  if (ui->button_listen_once->isChecked() || ui->button_listen_continuous->isChecked()) {
    ui->button_listen_once->setChecked(false);
    ui->button_listen_continuous->setChecked(false);
  }

  global()->save();

  QDialog::reject();
}

void MMGEchoWindow::connect_ui_signals()
{
  // Message Display Connections
  connect(ui->editor_type, &QComboBox::currentTextChanged, this,
	  &MMGEchoWindow::on_message_type_change);
  connect(ui->button_listen_continuous, &QAbstractButton::toggled, this,
	  &MMGEchoWindow::on_message_listen_continuous);
  connect(ui->button_listen_once, &QAbstractButton::toggled, this,
	  &MMGEchoWindow::on_message_listen_once);
  connect(ui->editor_type_toggle, &QCheckBox::toggled, this,
	  &MMGEchoWindow::on_message_type_toggle);
  connect(ui->editor_value_toggle, &QCheckBox::toggled, this,
	  &MMGEchoWindow::on_message_value_toggle);

  // Action Display Connections
  connect(ui->editor_cat, &QComboBox::currentIndexChanged, this,
	  &MMGEchoWindow::on_action_cat_change);
  connect(ui->editor_sub, &QComboBox::currentIndexChanged, this,
	  &MMGEchoWindow::on_action_sub_change);

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
  connect(ui->editor_midi_thru_toggle, &QCheckBox::toggled, this,
	  &MMGEchoWindow::on_midi_thru_change);
  connect(ui->editor_midi_thru_device, &QComboBox::currentTextChanged, this,
	  &MMGEchoWindow::on_midi_thru_device_change);
  connect(ui->button_export, &QPushButton::clicked, this, &MMGEchoWindow::export_bindings);
  connect(ui->button_import, &QPushButton::clicked, this, &MMGEchoWindow::import_bindings);
  connect(ui->button_help_advanced, &QPushButton::clicked, this, &MMGEchoWindow::i_need_help);
  connect(ui->button_bug_report, &QPushButton::clicked, this, &MMGEchoWindow::report_a_bug);
  connect(ui->button_update_check, &QPushButton::clicked, this, &MMGEchoWindow::on_update_check);

  connect(ui->editor_transfer_mode, &QComboBox::currentIndexChanged, this,
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

  // Listening Buttons
  connect(input_device().get(), &MMGMIDIIn::messageListened, this, &MMGEchoWindow::listen_update);
}

void MMGEchoWindow::set_message_view()
{
  current_message->setEditable(false);

  channel_display->setStorage(current_message->channel());
  note_display->setStorage(current_message->note());
  value_display->setStorage(current_message->value(), true);
  value_display->setBounds(0, 127);

  ui->editor_type->setCurrentText(*current_message->type());
  on_message_type_change(*current_message->type());

  ui->editor_type_toggle->setChecked(current_message->type()->state() ==
				     MMGString::STRINGSTATE_TOGGLE);
  on_message_type_toggle(current_message->type()->state() == MMGString::STRINGSTATE_TOGGLE);

  ui->editor_value_toggle->setChecked(current_message->value()->state() ==
				      MMGNumber::NUMBERSTATE_IGNORE);
  on_message_value_toggle(current_message->value()->state() == MMGNumber::NUMBERSTATE_IGNORE);

  current_message->setEditable(true);
}

void MMGEchoWindow::set_action_view()
{
  // Preparations (turn off action editing)
  current_action->setEditable(false);
  ui->editor_cat->blockSignals(true);
  ui->editor_sub->blockSignals(true);

  // Set category (and add sub list)
  ui->editor_cat->setCurrentIndex((int)current_action->category());
  on_action_cat_change((int)current_action->category());

  // Set subcategory view
  ui->editor_sub->setCurrentIndex(current_action->sub());
  on_action_sub_change(current_action->sub());

  // Turn on action editing
  current_action->setEditable(true);
  ui->editor_cat->blockSignals(false);
  ui->editor_sub->blockSignals(false);
}

void MMGEchoWindow::set_preferences_view()
{
  ui->editor_global_enable->setChecked(global()->preferences()->active());

  ui->editor_midi_thru_device->blockSignals(true);
  ui->editor_midi_thru_toggle->setChecked(!global()->preferences()->thruDevice().isEmpty());
  ui->editor_midi_thru_device->setEnabled(!global()->preferences()->thruDevice().isEmpty());
  ui->editor_midi_thru_device->clear();
  ui->editor_midi_thru_device->addItems(output_device()->outputDeviceNames());
  ui->editor_midi_thru_device->setCurrentText(global()->preferences()->thruDevice());
  ui->editor_midi_thru_device->blockSignals(false);
}

void MMGEchoWindow::on_device_change(const QString &name)
{
  if (name.isEmpty()) return;
  current_device = global()->find(name);
  global()->setActiveDeviceName(name);
  switch_structure_pane(1);
}

void MMGEchoWindow::on_message_type_change(const QString &type)
{
  ui->editor_type_toggle->setVisible(false);
  ui->editor_value_toggle->setVisible(false);

  current_message->type()->set_str(type);

  if (current_action) {
    // Slightly hacky, but it works without adding unnecessary functions
    if (current_action->category() == MMGAction::MMGACTION_MIDI) {
      auto midi_action = dynamic_cast<MMGActionMIDI *>(current_action);
      midi_action->setLabels();
    }
  }

  set_message_labels(type, note_display, value_display);

  if (type == "Note On" || type == "Note Off") {
    ui->editor_type_toggle->setVisible(true);
    ui->editor_value_toggle->setVisible(true);
    return;
  }
  ui->editor_type_toggle->setChecked(false);
  ui->editor_value_toggle->setChecked(false);
}

void MMGEchoWindow::on_message_listen_once(bool toggled)
{
  ui->button_listen_once->setText(toggled ? "Cancel..." : "Listen Once...");
  if (listening_mode == 2) {
    ui->button_listen_continuous->blockSignals(true);
    ui->button_listen_continuous->setChecked(false);
    ui->button_listen_continuous->blockSignals(false);
    on_message_listen_continuous(0);
  }
  input_device()->setListening(toggled);
  listening_mode = toggled;
}

void MMGEchoWindow::on_message_listen_continuous(bool toggled)
{
  ui->button_listen_continuous->setText(toggled ? "Cancel..." : "Listen Continuous...");
  if (listening_mode == 1) {
    ui->button_listen_once->blockSignals(true);
    ui->button_listen_once->setChecked(false);
    ui->button_listen_once->blockSignals(false);
    on_message_listen_once(0);
  }
  input_device()->setListening(toggled);
  listening_mode = toggled ? 2 : 0;
}

void MMGEchoWindow::listen_update(const MMGSharedMessage &incoming)
{
  if (!incoming) return;
  if (listening_mode < 1) return;
  // Check the validity of the message type (whether it is one of the five
  // supported types)
  if (ui->editor_type->findText(*incoming->type()) == -1) return;
  if (listening_mode == 1) {
    ui->button_listen_once->setText("Listen Once...");
    ui->button_listen_once->setChecked(false);
    input_device()->setListening(false);
  }
  incoming->copy(current_message);
  set_message_view();
}

void MMGEchoWindow::on_message_type_toggle(bool toggled)
{
  current_message->type()->set_state(toggled << 1);
}

void MMGEchoWindow::on_message_value_toggle(bool toggled)
{
  value_display->setLCDMode(
    toggled ? 3 : (current_message->value()->state() == 3 ? 1 : current_message->value()->state()));
  current_message->value()->set_num(toggled ? 127 : 0);
  value_display->setDisabled(toggled);
}

void MMGEchoWindow::on_action_cat_change(int index)
{
  if (!ui->editor_cat->signalsBlocked()) current_binding->setCategory(index);
  current_action = current_binding->action();
  change_sub_options();
}

void MMGEchoWindow::change_sub_options()
{
  ui->editor_sub->clear();
  current_action->setSubOptions(ui->editor_sub);
}

void MMGEchoWindow::on_action_sub_change(int index)
{
  current_action->setSub(index);

  if (current_action->display() == nullptr) {
    current_action->createDisplay(ui->action_display_editor);

    // Custom Fields Request System
    connect(current_action->display(), &MMGActionDisplay::customFieldRequest, this,
	    &MMGEchoWindow::custom_field_request);

    ui->action_display_editor->addWidget(current_action->display());
  }

  current_action->display()->setParent(ui->action_display_editor);
  current_action->display()->setParentBinding(current_binding);
  current_action->setSubConfig();
  ui->action_display_editor->setCurrentWidget(current_action->display());
}

void MMGEchoWindow::custom_field_request(void *ptr, MMGString *action_json)
{
  obs_source_t *source = static_cast<obs_source_t *>(ptr);
  if (source == nullptr) return;

  QWidget *parent = current_action->display()->scrollWidget()->parentWidget();
  bool prev_fields_json_match = !current_fields ||
				current_fields->jsonDestination()->str() != action_json->str();

  for (MMGOBSFields *fields : custom_fields) {
    if (fields->match(source)) {
      current_fields = fields;
      fields->setParent(parent);
      fields->setJsonDestination(action_json, prev_fields_json_match);
      current_action->display()->setScrollWidget(fields);
      return;
    }
  }
  MMGOBSFields *new_fields = new MMGOBSFields(parent, source);
  new_fields->setJsonDestination(action_json, prev_fields_json_match);
  current_fields = new_fields;
  custom_fields.append(new_fields);
  current_action->display()->setScrollWidget(new_fields);
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
  if (current->text() != current_binding->name())
    current_binding = current_device->find(current->text());

  current_device->remove(current_binding);
  delete current_binding;
  current_binding = nullptr;
  current_message = nullptr;
  current_action = nullptr;

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
  set_preferences_view();
}

void MMGEchoWindow::on_list_widget_state_change(QListWidgetItem *widget_item)
{
  if (!widget_item) return;

  if (ui->editor_structure->currentRow() != ui->editor_structure->row(widget_item)) {
    ui->editor_structure->setCurrentItem(widget_item);
    on_list_selection_change(widget_item);
  }

  if (current_binding->name() != widget_item->text()) {
    QString name{widget_item->text()};
    if (!!current_device->find(name)) {
      ui->editor_structure->currentItem()->setText(current_binding->name());
      return;
    }
    current_binding->setName(name);
  }

  current_binding->setEnabled((bool)widget_item->checkState());
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

  current_binding = current_device->find(widget_item->text());
  current_message = current_binding->message();
  current_action = current_binding->action();

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
  for (MMGBinding *const binding_el : current_device->bindings()) {
    add_widget_item(binding_el);
  }
  on_list_selection_change(nullptr);
}

void MMGEchoWindow::add_widget_item(const MMGBinding *binding) const
{
  QListWidgetItem *new_item = new QListWidgetItem;
  new_item->setFlags((Qt::ItemFlag)0b110111);
  new_item->setCheckState((Qt::CheckState)(binding->enabled() ? 2 : 0));
  new_item->setText(binding->name());
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
  global()->preferences()->setActive(toggle);
}

void MMGEchoWindow::on_midi_thru_change(bool toggle)
{
  global()->preferences()->setThruDevice(toggle ? ui->editor_midi_thru_device->currentText() : "");
  ui->editor_midi_thru_device->setEnabled(toggle);
}

void MMGEchoWindow::on_midi_thru_device_change(const QString &device)
{
  global()->preferences()->setThruDevice(device);
}

void MMGEchoWindow::export_bindings()
{
  QString filepath = QFileDialog::getSaveFileName(this, tr("Save Bindings..."),
						  MMGConfig::filepath(), "JSON Files (*.json)");
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
      ui->label_transfer_dest->setText("to");
      ui->label_transfer_source->setText("from");
      ui->button_binding_transfer->setText("Copy...");
      break;
    case 1:
      ui->label_transfer_dest->setText("to");
      ui->label_transfer_source->setText("from");
      ui->button_binding_transfer->setText("Move...");
      break;
    case 2:
      ui->label_transfer_dest->setText("in");
      ui->label_transfer_source->setText("with");
      ui->button_binding_transfer->setText("Replace...");
      break;
    default:
      break;
  }
}

void MMGEchoWindow::on_transfer_bindings_click()
{
  transfer_bindings(ui->editor_transfer_mode->currentIndex(),
		    ui->editor_transfer_dest->currentText(),
		    ui->editor_transfer_source->currentText());
}

void MMGEchoWindow::on_update_check()
{
  QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/releases"));
}

MMGEchoWindow::~MMGEchoWindow()
{
  delete ui;
}
