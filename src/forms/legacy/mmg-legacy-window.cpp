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
#include <QEvent>
#include <QFileDialog>
#include <QDesktopServices>

#include "mmg-legacy-window.h"
#include "../../mmg-config.h"
#include "../macros/mmg-window-macros.h"

using namespace MMGUtils;

MMGLegacyWindow::MMGLegacyWindow(QWidget *parent)
	: QDialog(parent, Qt::Dialog), ui(new Ui::MMGLegacyWindow)
{
	this->setWindowFlags(this->windowFlags() &
			     ~Qt::WindowContextHelpButtonHint);
	ui->setupUi(this);
	ui->label_version->setText(OBS_MIDIMG_VERSION);

	ui->editor_structure->setDragEnabled(true);
	ui->editor_structure->setDragDropMode(
		QAbstractItemView::DragDropMode::InternalMove);
	ui->editor_structure->setAcceptDrops(true);
	ui->editor_structure->setSelectionMode(
		QAbstractItemView::SelectionMode::SingleSelection);
	ui->editor_structure->setDropIndicatorShown(true);

	configure_lcd_widgets();

	connect_ui_signals();

	global()->set_listening_callback([this](MMGMessage *incoming) {
		if (!incoming)
			return;
		// Check the validity of the message type (whether it is one of the five
		// supported types)
		if (ui->editor_type->findText(incoming->get_type()) == -1)
			return;
		current_message->set_type(incoming->get_type());
		current_message->set_channel(incoming->get_channel());
		current_message->set_note(incoming->get_note());
		current_message->set_value(incoming->get_value());
		set_message_view();
	});
}

void MMGLegacyWindow::show_window()
{
	setVisible(!isVisible());
	global()->load_new_devices();
	ui->editor_structure->clearSelection();
	switch_structure_pane(MMGModes::MMGMODE_DEVICE);
	ui->pages->setCurrentIndex(0);

	ui->editor_transfer_source->clear();
	ui->editor_transfer_dest->clear();
	ui->editor_transfer_mode->setCurrentIndex(0);
	on_transfer_mode_change(0);
	for (const QString &name : global()->get_device_names()) {
		ui->editor_transfer_source->addItem(name);
		ui->editor_transfer_dest->addItem(name);
	}
}

void MMGLegacyWindow::reject()
{
	if (global()->is_listening(nullptr)) {
		open_message_box(
			"Error",
			"Cannot close window: The MIDI device is being listened to.\n(The Listen to Message button is on.)");
		return;
	}
	global()->save();
	QDialog::reject();
}

void MMGLegacyWindow::connect_ui_signals()
{
	// LCD Connections
	CONNECT_LCD(channel);
	CONNECT_LCD(note);
	CONNECT_LCD(value);
	CONNECT_LCD(double1);
	CONNECT_LCD(double2);
	CONNECT_LCD(double3);
	CONNECT_LCD(double4);
	// Binding Display Connections
	connect(ui->editor_message_toggling,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGLegacyWindow::on_binding_toggling_select);
	connect(ui->editor_binding_enable,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGLegacyWindow::on_binding_enable_select);
	connect(ui->button_edit_messages, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_MESSAGE); });
	connect(ui->button_edit_actions, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_ACTION); });
	// Message Display Connections
	connect(ui->editor_type, &QComboBox::currentTextChanged, this,
		&MMGLegacyWindow::on_message_type_change);
	connect(ui->button_listen, &QAbstractButton::toggled, this,
		&MMGLegacyWindow::on_message_listen);
	connect(ui->button_value_require, &QAbstractButton::toggled, this,
		&MMGLegacyWindow::on_message_require_value);
	// Action Display Connections
	connect(ui->editor_cat,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGLegacyWindow::on_action_cat_change);
	connect(ui->editor_sub,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGLegacyWindow::on_action_sub_change);
	connect(ui->editor_str1, &QComboBox::currentTextChanged, this,
		&MMGLegacyWindow::set_str1);
	connect(ui->editor_str2, &QComboBox::currentTextChanged, this,
		&MMGLegacyWindow::set_str2);
	connect(ui->editor_str3, &QComboBox::currentTextChanged, this,
		&MMGLegacyWindow::set_str3);
	connect(ui->label_double1, &QAbstractButton::toggled, this,
		&MMGLegacyWindow::on_action_double1_toggle);
	connect(ui->label_double2, &QAbstractButton::toggled, this,
		&MMGLegacyWindow::on_action_double2_toggle);
	connect(ui->label_double3, &QAbstractButton::toggled, this,
		&MMGLegacyWindow::on_action_double3_toggle);
	connect(ui->label_double4, &QAbstractButton::toggled, this,
		&MMGLegacyWindow::on_action_double4_toggle);
	// UI Movement Buttons
	connect(ui->button_add, &QPushButton::clicked, this,
		&MMGLegacyWindow::on_add_click);
	connect(ui->button_remove, &QPushButton::clicked, this,
		&MMGLegacyWindow::on_remove_click);
	connect(ui->button_return, &QPushButton::clicked, this,
		&MMGLegacyWindow::on_return_click);
	// Device Buttons
	connect(ui->button_active_device, &QAbstractButton::toggled, this,
		&MMGLegacyWindow::on_device_active_change);
	connect(ui->button_edit_bindings, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_BINDING); });
	// Preferences Connections
	connect(ui->button_preferences, &QPushButton::clicked, this,
		&MMGLegacyWindow::set_preferences_view);
	connect(ui->editor_global_enable, &QCheckBox::toggled, this,
		&MMGLegacyWindow::on_active_change);
	connect(ui->editor_interface_style,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGLegacyWindow::on_interface_style_change);
	connect(ui->button_export, &QPushButton::clicked, this,
		&MMGLegacyWindow::export_bindings);
	connect(ui->button_import, &QPushButton::clicked, this,
		&MMGLegacyWindow::import_bindings);
	connect(ui->button_help_advanced, &QPushButton::clicked, this,
		&MMGLegacyWindow::i_need_help);
	connect(ui->button_bug_report, &QPushButton::clicked, this,
		&MMGLegacyWindow::report_a_bug);
	connect(ui->button_update_check, &QPushButton::clicked, this,
		&MMGLegacyWindow::on_update_check);

	connect(ui->editor_transfer_mode,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MMGLegacyWindow::on_transfer_mode_change);
	connect(ui->button_binding_transfer, &QPushButton::clicked, this,
		&MMGLegacyWindow::on_transfer_bindings_click);

	// List Widget Connections
	connect(ui->editor_structure, &QListWidget::itemClicked, this,
		&MMGLegacyWindow::on_list_selection_change);
	connect(ui->editor_structure, &QListWidget::itemChanged, this,
		&MMGLegacyWindow::on_name_edit);
}

void MMGLegacyWindow::configure_lcd_widgets()
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

void MMGLegacyWindow::set_device_view()
{
	ui->text_device_name->setText(current_device->get_name());
	ui->text_status_input->setText(current_device->input_device_status());
	ui->text_status_output->setText(current_device->output_device_status());
	ui->button_active_device->setChecked(
		current_device->get_name() ==
		global()->get_active_device_name());
	on_device_active_change(current_device->get_name() ==
				global()->get_active_device_name());
	ui->button_edit_bindings->setEnabled(ui->text_status_input->text() ==
					     "Ready");
}

void MMGLegacyWindow::set_binding_view()
{
	ui->text_binding_name->setText(current_binding->get_name());
	ui->editor_message_toggling->setCurrentIndex(
		(int)current_binding->get_toggling());
	ui->editor_binding_enable->setCurrentIndex(
		(int)(!current_binding->get_enabled()));
}

void MMGLegacyWindow::set_message_view()
{
	// Because current_message is modified in these function calls (LCDData::reset),
	// this uses a const version of it to get its values
	const MMGMessage temp = *current_message;

	lcd_channel.reset(temp.get_channel());
	lcd_note.reset(temp.get_note());
	SET_LCD_STATUS(value, Enabled, temp.get_value_required());
	lcd_value.reset(temp.get_value());
	if (ui->button_value_require->isChecked() != temp.get_value_required())
		ui->button_value_require->setChecked(temp.get_value_required());

	// Re-set current_message to the correct one
	*current_message = temp;

	on_message_type_change(temp.get_type());
}

void MMGLegacyWindow::set_action_view()
{
	// Because current_action is modified in these function calls (LCDData::reset),
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
	ui->label_double1->setChecked(temp.get_num(0) == -1);
	ui->label_double2->setChecked(temp.get_num(1) == -1);
	ui->label_double3->setChecked(temp.get_num(2) == -1);
	ui->label_double4->setChecked(temp.get_num(3) == -1);
	lcd_double1.reset(temp.get_num(0) == -1 ? 0 : temp.get_num(0));
	lcd_double2.reset(temp.get_num(1) == -1 ? 0 : temp.get_num(1));
	lcd_double3.reset(temp.get_num(2) == -1 ? 0 : temp.get_num(2));
	lcd_double4.reset(temp.get_num(3) == -1 ? 0 : temp.get_num(3));
	// Re-set current_action to the correct one
	*current_action = temp;
}

void MMGLegacyWindow::set_preferences_view()
{
	switch_structure_pane(MMGModes::MMGMODE_PREFERENCES);
	ui->editor_global_enable->setChecked(
		global()->preferences().get_active());
	ui->editor_interface_style->setCurrentIndex(
		global()->preferences().get_ui_style());
}

void MMGLegacyWindow::set_strs_visible(bool str1, bool str2, bool str3) const
{
	ui->label_str1->setVisible(str1);
	ui->editor_str1->setVisible(str1);
	ui->label_str2->setVisible(str2);
	ui->editor_str2->setVisible(str2);
	ui->label_str3->setVisible(str3);
	ui->editor_str3->setVisible(str3);
}

void MMGLegacyWindow::set_doubles_visible(bool double1, bool double2,
					  bool double3, bool double4) const
{
	SET_LCD_STATUS(double1, Visible, double1);
	SET_LCD_STATUS(double2, Visible, double2);
	SET_LCD_STATUS(double3, Visible, double3);
	SET_LCD_STATUS(double4, Visible, double4);
}

void MMGLegacyWindow::on_device_active_change(bool toggled)
{
	if (!toggled &&
	    global()->get_active_device_name() == current_device->get_name()) {
		ui->button_active_device->setChecked(true);
		return;
	}
	if (toggled)
		global()->set_active_device_name(current_device->get_name());
	ui->button_active_device->setText(toggled ? "Device is Active"
						  : "Set As Active Device...");
}

void MMGLegacyWindow::on_binding_toggling_select(int index)
{
	current_binding->set_toggling(index);
}

void MMGLegacyWindow::on_binding_enable_select(int index)
{
	current_binding->set_enabled(!index);
}

void MMGLegacyWindow::on_message_type_change(const QString &type)
{
	current_message->set_type(type);
	ui->editor_type->setCurrentText(type);

	SET_LCD_STATUS(note, Enabled, true);
	SET_LCD_STATUS(value, Visible, true);
	SET_LCD_STATUS(value, Enabled, true);

	if (type == "Note On" || type == "Note Off") {
		ui->label_note->setText("Note #");
		ui->label_value->setText("Velocity");
		SET_LCD_STATUS(value, Enabled,
			       current_message->get_value_required());
		lcd_note.reset(current_message->get_note());
		lcd_value.display();
	} else if (type == "Control Change") {
		ui->label_note->setText("Control");
		ui->label_value->setText("Value");
		SET_LCD_STATUS(value, Enabled,
			       current_message->get_value_required());
		lcd_note.reset(current_message->get_note());
		lcd_value.display();
	} else if (type == "Program Change") {
		ui->label_note->setText("Program");
		SET_LCD_STATUS(value, Visible, false);
		SET_LCD_STATUS(note, Enabled,
			       current_message->get_value_required());
		lcd_note.reset(current_message->get_value());
	} else if (type == "Pitch Bend") {
		ui->label_note->setText("Pitch Adj.");
		SET_LCD_STATUS(value, Visible, false);
		SET_LCD_STATUS(note, Enabled,
			       current_message->get_value_required());
		lcd_note.reset(current_message->get_value());
	}
}

void MMGLegacyWindow::on_message_listen(bool toggled)
{
	global()->set_listening(toggled);
	ui->button_listen->setText(toggled ? "Cancel..."
					   : "Listen for Message...");
}

void MMGLegacyWindow::on_message_require_value(bool toggled)
{
	current_message->set_value_required(toggled);
	ui->button_value_require->setText(toggled ? "Value is Required"
						  : "Require Value...");
	set_message_view();
}

void MMGLegacyWindow::on_action_cat_change(int index)
{
	INSERT_SUB_OPTIONS()
}

void MMGLegacyWindow::set_sub_options(std::initializer_list<QString> list) const
{
	ui->label_sub->setText("Options");
	ui->editor_sub->clear();
	ui->editor_sub->addItems(list);
	ui->editor_sub->setCurrentIndex(0);
}

void MMGLegacyWindow::on_action_sub_change(int index)
{
	INSERT_FIRST_OPTION();
}

void MMGLegacyWindow::set_channel(double value)
{
	current_message->set_channel(value);
}

void MMGLegacyWindow::set_note(double value)
{
	if (ui->lcd_value->isVisible()) {
		current_message->set_note(value);
	} else {
		set_value(value);
	}
}

void MMGLegacyWindow::set_value(double value)
{
	current_message->set_value(value);
}

void MMGLegacyWindow::set_str1(const QString &value)
{
	INSERT_SECOND_OPTION();
}

void MMGLegacyWindow::set_str2(const QString &value)
{
	INSERT_THIRD_OPTION();
}

void MMGLegacyWindow::set_str3(const QString &value)
{
	INSERT_FOURTH_OPTION();
}

void MMGLegacyWindow::set_double1(double value)
{
	current_action->set_num(0, value);
}

void MMGLegacyWindow::set_double2(double value)
{
	current_action->set_num(1, value);
}

void MMGLegacyWindow::set_double3(double value)
{
	current_action->set_num(2, value);
}

void MMGLegacyWindow::set_double4(double value)
{
	current_action->set_num(3, value);
}

void MMGLegacyWindow::on_action_double1_toggle(bool toggle)
{
	set_double1(0 - toggle);
	SET_LCD_STATUS(double1, Disabled, toggle);
	ui->label_double1->setDisabled(false);
	lcd_double1.display();
}

void MMGLegacyWindow::on_action_double2_toggle(bool toggle)
{
	set_double2(0 - toggle);
	SET_LCD_STATUS(double2, Disabled, toggle);
	ui->label_double2->setDisabled(false);
	lcd_double2.display();
}

void MMGLegacyWindow::on_action_double3_toggle(bool toggle)
{
	set_double3(0 - toggle);
	SET_LCD_STATUS(double3, Disabled, toggle);
	ui->label_double3->setDisabled(false);
	lcd_double3.display();
}

void MMGLegacyWindow::on_action_double4_toggle(bool toggle)
{
	set_double4(0 - toggle);
	SET_LCD_STATUS(double4, Disabled, toggle);
	ui->label_double4->setDisabled(false);
	lcd_double4.display();
}

void MMGLegacyWindow::on_add_click()
{
	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_BINDING:
		current_binding = current_device->add();
		add_widget_item(MMGModes::MMGMODE_BINDING,
				current_binding->get_name());
		break;
	default:
		break;
	}
}

void MMGLegacyWindow::on_remove_click()
{
	QListWidgetItem *current = ui->editor_structure->currentItem();
	if (!current)
		return;
	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_BINDING:
		current_device->remove(current_binding);
		delete current_binding;
		break;

	default:
		return;
	}

	ui->editor_structure->removeItemWidget(current);
	delete current;
	ui->button_remove->setEnabled(false);
	on_list_selection_change(nullptr);
}

void MMGLegacyWindow::on_return_click()
{
	QListWidgetItem *current_item = nullptr;
	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_PREFERENCES:
		switch_structure_pane(MMGModes::MMGMODE_DEVICE);
		on_list_selection_change(nullptr);
		break;
	case MMGModes::MMGMODE_BINDING:
		switch_structure_pane(MMGModes::MMGMODE_DEVICE);
		current_item = ui->editor_structure->findItems(
			current_device->get_name(), Qt::MatchCaseSensitive)[0];
		current_item->setSelected(true);
		on_list_selection_change(current_item);
		break;

	case MMGModes::MMGMODE_MESSAGE:
	case MMGModes::MMGMODE_ACTION:
		switch_structure_pane(MMGModes::MMGMODE_BINDING);
		current_item = ui->editor_structure->findItems(
			current_binding->get_name(), Qt::MatchCaseSensitive)[0];
		current_item->setSelected(true);
		on_list_selection_change(current_item);
		break;
	default:
		return;
	}
}

void MMGLegacyWindow::on_name_edit(QListWidgetItem *widget_item)
{
	QString str = widget_item->text();

	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_BINDING:
		if (!current_binding || current_binding->get_name() == str)
			break;
		if (!!current_device->find_binding(str)) {
			widget_item->setText(current_binding->get_name());
			break;
		}
		current_binding->set_name(str);
		ui->text_binding_name->setText(str);
		break;
	default:
		break;
	}
}

void MMGLegacyWindow::on_list_selection_change(const QListWidgetItem *current)
{
	if (!current) {
		// No selection
		ui->pages->setCurrentIndex(0);
		return;
	}

	ui->button_add->setEnabled(true);
	ui->button_remove->setEnabled(true);
	ui->button_return->setEnabled(true);

	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_DEVICE:
		current_device = global()->find_device(current->text());
		ui->button_add->setEnabled(false);
		ui->button_remove->setEnabled(false);
		ui->button_return->setEnabled(false);
		set_device_view();
		ui->pages->setCurrentIndex(1);
		break;
	case MMGModes::MMGMODE_BINDING:
		current_binding = current_device->find_binding(current->text());
		current_message = current_binding->get_message();
		current_action = current_binding->get_action();
		set_binding_view();
		ui->pages->setCurrentIndex(2);
		break;
	case MMGModes::MMGMODE_PREFERENCES:
		ui->button_add->setEnabled(false);
		ui->button_remove->setEnabled(false);
		ui->pages->setCurrentIndex(
			ui->editor_structure->indexFromItem(current).row() + 5);
		break;
	default:
		switch_structure_pane(MMGModes::MMGMODE_NONE);
		break;
	}
}

void MMGLegacyWindow::switch_structure_pane(enum MMGModes mode)
{
	ui->editor_structure->clear();
	ui->editor_structure->setProperty("mode", QVariant::fromValue(mode));
	ui->button_add->setEnabled(false);
	ui->button_remove->setEnabled(false);
	ui->button_return->setEnabled(true);
	switch (mode) {
	case MMGModes::MMGMODE_PREFERENCES:
		ui->pages->setCurrentIndex(5);
		add_widget_item(mode, "General");
		add_widget_item(mode, "Binding Transfer");
		return;
	case MMGModes::MMGMODE_DEVICE:
		ui->button_return->setEnabled(false);
		for (const QString &name : global()->get_device_names()) {
			add_widget_item(mode, name);
		}
		break;
	case MMGModes::MMGMODE_BINDING:
		ui->button_add->setEnabled(true);
		for (const MMGBinding *const binding_el :
		     current_device->get_bindings()) {
			add_widget_item(mode, binding_el->get_name());
		}
		break;
	case MMGModes::MMGMODE_MESSAGE:
		ui->pages->setCurrentIndex(3);
		set_message_view();
		return;
	case MMGModes::MMGMODE_ACTION:
		ui->pages->setCurrentIndex(4);
		set_action_view();
		return;
	default:
		ui->button_return->setEnabled(false);
		ui->pages->setCurrentIndex(0);
		return;
	}
	on_list_selection_change(nullptr);
}

void MMGLegacyWindow::add_widget_item(MMGModes type, const QString &name) const
{
	QListWidgetItem *widget_item = new QListWidgetItem;
	widget_item->setText(name);
	switch (type) {
	case MMGModes::MMGMODE_DEVICE:
		widget_item->setFlags((
			Qt::ItemFlag)0b100001); // Qt::ItemIsEnabled | Qt::ItemIsSelectable
		widget_item->setBackground(QColor::fromRgb(12, 12, 12, 128));
		break;
	case MMGModes::MMGMODE_BINDING:
		widget_item->setFlags((
			Qt::ItemFlag)0b100011); // Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable
		widget_item->setBackground(QColor::fromRgb(0, 0, 40, 128));
		break;
	case MMGModes::MMGMODE_PREFERENCES:
		widget_item->setFlags((
			Qt::ItemFlag)0b100001); // Qt::ItemIsEnabled | Qt::ItemIsSelectable
		widget_item->setBackground(QColor::fromRgb(12, 12, 12, 128));
		break;
	default:
		delete widget_item;
		return;
	}
	ui->editor_structure->addItem(widget_item);
}

PREFERENCE_FUNCTION_DECL(MMGLegacyWindow);

MMGLegacyWindow::~MMGLegacyWindow()
{
	delete ui;
}
