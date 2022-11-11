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
#include <QFileDialog>
#include <QDesktopServices>

#include "mmg-echo-window.h"
#include "../mmg-config.h"
#include "../macros/mmg-window-macros.h"

using namespace MMGUtils;

MMGEchoWindow::MMGEchoWindow(QWidget *parent)
	: QDialog(parent, Qt::Dialog), ui(new Ui::MMGEchoWindow)
{
	this->setWindowFlags(this->windowFlags() &
			     ~Qt::WindowContextHelpButtonHint);
	ui->setupUi(this);
	ui->label_version->setText(OBS_MIDIMG_VERSION);

	ui->editor_structure->setDragEnabled(false);

	configure_lcd_widgets();

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
	connect(ui->editor_value,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_message_value_button_change);
	connect(ui->editor_note,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_message_value_button_change);
	// Action Display Connections
	connect(ui->editor_cat,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_action_cat_change);
	connect(ui->editor_sub,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_action_sub_change);
	connect(ui->editor_str1, &QComboBox::currentTextChanged, this,
		&MMGEchoWindow::set_str1);
	connect(ui->editor_str2, &QComboBox::currentTextChanged, this,
		&MMGEchoWindow::set_str2);
	connect(ui->editor_str3, &QComboBox::currentTextChanged, this,
		&MMGEchoWindow::set_str3);
	connect(ui->editor_double1,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_action_double1_select);
	connect(ui->editor_double2,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_action_double2_select);
	connect(ui->editor_double3,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_action_double3_select);
	connect(ui->editor_double4,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_action_double4_select);
	// UI Movement Buttons
	connect(ui->button_add, &QPushButton::clicked, this,
		&MMGEchoWindow::on_add_click);
	connect(ui->button_duplicate, &QPushButton::clicked, this,
		&MMGEchoWindow::on_copy_click);
	connect(ui->button_remove, &QPushButton::clicked, this,
		&MMGEchoWindow::on_remove_click);
	// Device Connections
	connect(ui->editor_devices, &QComboBox::currentTextChanged, this,
		&MMGEchoWindow::on_device_change);
	// Preferences Connections
	connect(ui->button_preferences, &QAbstractButton::toggled, this,
		&MMGEchoWindow::on_preferences_click);
	connect(ui->editor_global_enable, &QCheckBox::toggled, this,
		&MMGEchoWindow::on_active_change);
	connect(ui->editor_interface_style,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_interface_style_change);
	connect(ui->button_export, &QPushButton::clicked, this,
		&MMGEchoWindow::export_bindings);
	connect(ui->button_import, &QPushButton::clicked, this,
		&MMGEchoWindow::import_bindings);
	connect(ui->button_help_advanced, &QPushButton::clicked, this,
		&MMGEchoWindow::i_need_help);
	connect(ui->button_bug_report, &QPushButton::clicked, this,
		&MMGEchoWindow::report_a_bug);
	connect(ui->button_update_check, &QPushButton::clicked, this,
		&MMGEchoWindow::on_update_check);

	connect(ui->editor_transfer_mode,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGEchoWindow::on_transfer_mode_change);
	connect(ui->button_binding_transfer, &QPushButton::clicked, this,
		&MMGEchoWindow::on_transfer_bindings_click);

	// Table Widget Connections
	connect(ui->editor_structure, &QListWidget::itemClicked, this,
		&MMGEchoWindow::on_list_selection_change);
	connect(ui->editor_structure, &QListWidget::itemChanged, this,
		&MMGEchoWindow::on_list_widget_state_change);
}

void MMGEchoWindow::configure_lcd_widgets()
{
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
}

void MMGEchoWindow::set_message_view()
{
	// Because current_message is modified in these function calls (LCDData::reset),
	// this uses a const version of it to get its values
	const MMGMessage temp_message = *current_message;

	lcd_channel.set_value(temp_message.get_channel());
	lcd_note.set_value(temp_message.get_note());
	lcd_value.set_value(temp_message.get_value());

	if (temp_message.get_type_toggle()) {
		on_message_type_change("Note On / Note Off");
	} else {
		on_message_type_change(temp_message.get_type());
	}

	// Re-set current_message to the correct one
	*current_message = temp_message;
}

void MMGEchoWindow::set_action_view()
{
	// Because current_action is modified in these function calls (LCDData::set_default_value),
	// this uses a const version of it to get its values
	const MMGAction temp = *current_action;
	// Set category
	ui->editor_cat->setCurrentIndex((int)temp.get_category());
	on_action_cat_change((int)temp.get_category());
	// Set subcategory
	ui->editor_sub->setCurrentIndex(temp.get_sub());
	on_action_sub_change(temp.get_sub());
	// Set strings (even if they are invalid)
	ui->editor_str1->setCurrentText(temp.get_str(0));
	ui->editor_str2->setCurrentText(temp.get_str(1));
	ui->editor_str3->setCurrentText(temp.get_str(2));
	// Set doubles (extra jargon for using the message value)
	ui->editor_double1->setCurrentIndex(temp.get_num_state(0));
	ui->editor_double2->setCurrentIndex(temp.get_num_state(1));
	ui->editor_double3->setCurrentIndex(temp.get_num_state(2));
	ui->editor_double4->setCurrentIndex(temp.get_num_state(3));
	lcd_double1.set_value(temp.get_num_state(0) == 1 ? 0 : temp.get_num(0));
	lcd_double2.set_value(temp.get_num_state(1) == 1 ? 0 : temp.get_num(1));
	lcd_double3.set_value(temp.get_num_state(2) == 1 ? 0 : temp.get_num(2));
	lcd_double4.set_value(temp.get_num_state(3) == 1 ? 0 : temp.get_num(3));
	// Re-set current_action to the correct one
	*current_action = temp;
}

void MMGEchoWindow::set_preferences_view()
{
	ui->editor_global_enable->setChecked(
		global()->preferences().get_active());
}

void MMGEchoWindow::set_strs_visible(bool str1, bool str2, bool str3) const
{
	ui->label_str1->setVisible(str1);
	ui->editor_str1->setVisible(str1);
	ui->label_str2->setVisible(str2);
	ui->editor_str2->setVisible(str2);
	ui->label_str3->setVisible(str3);
	ui->editor_str3->setVisible(str3);
}

void MMGEchoWindow::set_doubles_visible(bool double1, bool double2,
					bool double3, bool double4) const
{
	SET_LCD_STATUS(double1, Visible, double1);
	ui->editor_double1->setVisible(double1);
	SET_LCD_STATUS(double2, Visible, double2);
	ui->editor_double2->setVisible(double2);
	ui->sep_action_1->setVisible(double2);
	SET_LCD_STATUS(double3, Visible, double3);
	ui->editor_double3->setVisible(double3);
	ui->sep_action_2->setVisible(double3);
	SET_LCD_STATUS(double4, Visible, double4);
	ui->editor_double4->setVisible(double4);
	ui->sep_action_3->setVisible(double4);
}

void MMGEchoWindow::on_device_change(const QString &name)
{
	if (name.isEmpty())
		return;
	current_device = global()->find_device(name);
	global()->set_active_device_name(name);
	switch_structure_pane(1);
}

void MMGEchoWindow::on_message_type_change(const QString &type)
{
	SET_LCD_STATUS(note, Enabled, true);
	SET_LCD_STATUS(value, Visible, true);
	SET_LCD_STATUS(value, Enabled, true);
	ui->editor_note->setVisible(true);
	ui->editor_value->setVisible(true);
	COMBOBOX_ITEM_STATE(editor_value, 2, false);

	bool temp_value_toggle = current_message->get_value_toggle();

	ui->editor_value->setCurrentIndex(
		current_message->get_value_required());
	on_message_value_button_change(current_message->get_value_required());

	if (type == "Note On / Note Off") {
		ui->editor_type->setCurrentText("Note On / Note Off");
		current_message->set_type("Note On");
		ui->label_note->setText("Note #");
		ui->label_value->setText("Velocity");
		current_message->set_type_toggle(true);
		COMBOBOX_ITEM_STATE(editor_value, 2, true);
		if (temp_value_toggle) {
			ui->editor_value->setCurrentIndex(2);
			on_message_value_button_change(2);
		}
		ui->editor_note->setVisible(false);
		lcd_note.set_value(current_message->get_note());
		lcd_value.display();
		return;
	}

	current_message->set_type(type);
	ui->editor_type->setCurrentText(type);

	current_message->set_type_toggle(false);

	if (type == "Note On" || type == "Note Off") {
		ui->label_note->setText("Note #");
		ui->label_value->setText("Velocity");
		ui->editor_note->setVisible(false);
		COMBOBOX_ITEM_STATE(editor_value, 2, true);
		if (temp_value_toggle) {
			ui->editor_value->setCurrentIndex(2);
			on_message_value_button_change(2);
		}
		lcd_note.set_value(current_message->get_note());
		lcd_value.display();
	} else if (type == "Control Change") {
		ui->label_note->setText("Control #");
		ui->label_value->setText("Value");
		ui->editor_note->setVisible(false);
		lcd_note.set_value(current_message->get_note());
		lcd_value.display();
	} else if (type == "Program Change") {
		ui->label_note->setText("Program #");
		SET_LCD_STATUS(value, Visible, false);
		ui->editor_value->setVisible(false);
		ui->editor_note->setCurrentIndex(
			current_message->get_value_required());
		lcd_note.set_value(current_message->get_value());
	} else if (type == "Pitch Bend") {
		ui->label_note->setText("Pitch Adjust");
		SET_LCD_STATUS(value, Visible, false);
		ui->editor_value->setVisible(false);
		ui->editor_note->setCurrentIndex(
			current_message->get_value_required());
		lcd_note.set_value(current_message->get_value());
	}
}

void MMGEchoWindow::on_message_listen_once(bool toggled)
{
	global()->set_listening(toggled);
	ui->button_listen_once->setText(toggled ? "Cancel..."
						: "Listen Once...");
	if (!toggled)
		return;
	global()->set_listening_callback([this](MMGMessage *incoming) {
		if (!incoming)
			return;
		// Check the validity of the message type (whether it is one of the five
		// supported types)
		if (ui->editor_type->findText(incoming->get_type()) == -1)
			return;
		global()->set_listening(false);
		ui->button_listen_once->setText("Listen Once...");
		ui->button_listen_once->setChecked(false);
		current_message->set_type_toggle(false);
		current_message->set_value_toggle(false);
		current_message->set_type(incoming->get_type());
		current_message->set_channel(incoming->get_channel());
		current_message->set_note(incoming->get_note());
		current_message->set_value_required(true);
		current_message->set_value(incoming->get_value());
		set_message_view();
	});
}

void MMGEchoWindow::on_message_listen_continuous(bool toggled)
{
	global()->set_listening(toggled);
	ui->button_listen_continuous->setText(toggled ? "Cancel..."
						      : "Listen Continuous...");
	if (!toggled)
		return;
	global()->set_listening_callback([this](MMGMessage *incoming) {
		if (!incoming)
			return;
		// Check the validity of the message type (whether it is one of the five
		// supported types)
		if (ui->editor_type->findText(incoming->get_type()) == -1)
			return;
		current_message->set_type_toggle(false);
		current_message->set_value_toggle(false);
		current_message->set_type(incoming->get_type());
		current_message->set_channel(incoming->get_channel());
		current_message->set_note(incoming->get_note());
		current_message->set_value_required(true);
		current_message->set_value(incoming->get_value());
		set_message_view();
	});
}

void MMGEchoWindow::on_message_value_button_change(int index)
{
	current_message->set_value_required(index == 1);
	current_message->set_value_toggle(index == 2);
	if (index == 2)
		current_message->set_value(127);

	if (ui->editor_type->currentText() == "Program Change" ||
	    ui->editor_type->currentText() == "Pitch Bend") {
		SET_LCD_STATUS(note, Enabled, index == 1);
	} else {
		SET_LCD_STATUS(value, Enabled, index == 1);
	}
	lcd_note.display();
	lcd_value.display();
}

void MMGEchoWindow::on_action_cat_change(int index)
{
	INSERT_SUB_OPTIONS();
}

void MMGEchoWindow::set_sub_options(std::initializer_list<QString> list) const
{
	ui->editor_sub->clear();
	ui->editor_sub->addItems(list);
	ui->editor_sub->setCurrentIndex(0);
}

void MMGEchoWindow::on_action_sub_change(int index)
{
	INSERT_FIRST_OPTION();
}

void MMGEchoWindow::set_channel(double value)
{
	current_message->set_channel(value);
}

void MMGEchoWindow::set_note(double value)
{
	if (ui->lcd_value->isVisible()) {
		current_message->set_note(value);
	} else {
		set_value(value);
	}
}

void MMGEchoWindow::set_value(double value)
{
	current_message->set_value(value);
}

void MMGEchoWindow::set_str1(const QString &value)
{
	INSERT_SECOND_OPTION();
}

void MMGEchoWindow::set_str2(const QString &value)
{
	INSERT_THIRD_OPTION();
}

void MMGEchoWindow::set_str3(const QString &value)
{
	INSERT_FOURTH_OPTION();
}

void MMGEchoWindow::set_double1(double value)
{
	current_action->set_num(0, value);
}

void MMGEchoWindow::set_double2(double value)
{
	current_action->set_num(1, value);
}

void MMGEchoWindow::set_double3(double value)
{
	current_action->set_num(2, value);
}

void MMGEchoWindow::set_double4(double value)
{
	current_action->set_num(3, value);
}

void MMGEchoWindow::on_action_double1_select(int index)
{
	current_action->set_num_state(0, index);
	SET_LCD_STATUS(double1, Disabled, index != 0);
	ui->label_double1->setDisabled(false);
	lcd_double1.display();
}

void MMGEchoWindow::on_action_double2_select(int index)
{
	current_action->set_num_state(1, index);
	SET_LCD_STATUS(double2, Disabled, index != 0);
	ui->label_double2->setDisabled(false);
	lcd_double2.display();
}

void MMGEchoWindow::on_action_double3_select(int index)
{
	current_action->set_num_state(2, index);
	SET_LCD_STATUS(double3, Disabled, index != 0);
	ui->label_double3->setDisabled(false);
	lcd_double3.display();
}

void MMGEchoWindow::on_action_double4_select(int index)
{
	current_action->set_num_state(3, index);
	SET_LCD_STATUS(double4, Disabled, index != 0);
	ui->label_double4->setDisabled(false);
	lcd_double4.display();
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
	if (!current)
		return;
	current_device->remove(current_binding);
	delete current_binding;

	delete current;

	ui->button_duplicate->setEnabled(false);
	ui->button_remove->setEnabled(false);
	on_list_selection_change(nullptr);
}

void MMGEchoWindow::on_preferences_click(bool toggle)
{
	switch_structure_pane(1 + toggle);
	ui->button_preferences->setText(toggle ? "Return to Bindings..."
					       : "Preferences...");
}

void MMGEchoWindow::on_list_widget_state_change(QListWidgetItem *widget_item)
{
	if (!current_binding || !widget_item)
		return;

	if (ui->editor_structure->currentRow() !=
	    ui->editor_structure->row(widget_item)) {
		ui->editor_structure->setCurrentItem(widget_item);
		on_list_selection_change(widget_item);
	}

	if (current_binding->get_name() != widget_item->text()) {
		QString name{widget_item->text()};
		if (!!current_device->find_binding(name)) {
			ui->editor_structure->currentItem()->setText(
				current_binding->get_name());
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

	if (page != 1)
		return;

	ui->button_add->setEnabled(true);
	if (!current_device)
		return;
	for (MMGBinding *const binding_el : current_device->get_bindings()) {
		add_widget_item(binding_el);
	}
	on_list_selection_change(nullptr);
}

void MMGEchoWindow::add_widget_item(const MMGBinding *binding) const
{
	QListWidgetItem *new_item = new QListWidgetItem;
	new_item->setFlags((Qt::ItemFlag)0b110011);
	new_item->setCheckState(
		(Qt::CheckState)(binding->get_enabled() ? 2 : 0));
	new_item->setText(binding->get_name());
	ui->editor_structure->addItem(new_item);
}

PREFERENCE_FUNCTION_DECL(MMGEchoWindow);

MMGEchoWindow::~MMGEchoWindow()
{
	delete ui;
}
