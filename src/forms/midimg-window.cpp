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

#include "midimg-window.h"
#include "../mmg-config.h"

using namespace MMGUtils;

#define SET_LCD_STATUS(lcd, kind, status)        \
	ui->down_major_##lcd->set##kind(status); \
	ui->down_minor_##lcd->set##kind(status); \
	ui->up_minor_##lcd->set##kind(status);   \
	ui->up_major_##lcd->set##kind(status);   \
	ui->label_##lcd->set##kind(status);      \
	ui->lcd_##lcd->set##kind(status)

#define CONNECT_LCD(kind)                                                     \
	connect(ui->down_major_##kind, &QAbstractButton::clicked, this,       \
		[&]() {                                                       \
			lcd_##kind.down_major();                              \
			set_##kind(lcd_##kind.get_value());                   \
		});                                                           \
	connect(ui->down_minor_##kind, &QAbstractButton::clicked, this,       \
		[&]() {                                                       \
			lcd_##kind.down_minor();                              \
			set_##kind(lcd_##kind.get_value());                   \
		});                                                           \
	connect(ui->up_minor_##kind, &QAbstractButton::clicked, this, [&]() { \
		lcd_##kind.up_minor();                                        \
		set_##kind(lcd_##kind.get_value());                           \
	});                                                                   \
	connect(ui->up_major_##kind, &QAbstractButton::clicked, this, [&]() { \
		lcd_##kind.up_major();                                        \
		set_##kind(lcd_##kind.get_value());                           \
	})

#define INIT_LCD(kind)                        \
	lcd_##kind = LCDData(ui->lcd_##kind); \
	ui->lcd_##kind->installEventFilter(this)

MidiMGWindow::MidiMGWindow(QWidget *parent)
	: QDialog(parent, Qt::Dialog), ui(new Ui::MidiMGWindow)
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

void MidiMGWindow::show_window()
{
	setVisible(!isVisible());
	global()->load_new_devices();
	ui->editor_structure->clearSelection();
	switch_structure_pane(MMGModes::MMGMODE_DEVICE);
	ui->pages->setCurrentIndex(0);
}

void MidiMGWindow::reject()
{
	global()->save();
	QDialog::reject();
}

void MidiMGWindow::connect_ui_signals()
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
	connect(ui->editor_reception_method,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MidiMGWindow::on_binding_reception_select);
	connect(ui->editor_message_toggling,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MidiMGWindow::on_binding_toggling_select);
	connect(ui->button_edit_messages, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_MESSAGE); });
	connect(ui->button_edit_actions, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_ACTION); });
	// Message Display Connections
	connect(ui->editor_type, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::on_message_type_change);
	connect(ui->button_listen, &QAbstractButton::toggled, this,
		&MidiMGWindow::on_message_listen);
	connect(ui->button_value_require, &QAbstractButton::toggled, this,
		&MidiMGWindow::on_message_require_value);
	// Action Display Connections
	connect(ui->editor_cat,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MidiMGWindow::on_action_cat_change);
	connect(ui->editor_sub,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MidiMGWindow::on_action_sub_change);
	connect(ui->editor_str1, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::set_str1);
	connect(ui->editor_str2, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::set_str2);
	connect(ui->editor_str3, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::set_str3);
	connect(ui->label_double1, &QAbstractButton::toggled, this,
		&MidiMGWindow::on_action_double1_toggle);
	connect(ui->label_double2, &QAbstractButton::toggled, this,
		&MidiMGWindow::on_action_double2_toggle);
	connect(ui->label_double3, &QAbstractButton::toggled, this,
		&MidiMGWindow::on_action_double3_toggle);
	connect(ui->label_double4, &QAbstractButton::toggled, this,
		&MidiMGWindow::on_action_double4_toggle);
	// UI Movement Buttons
	connect(ui->button_add, &QPushButton::clicked, this,
		&MidiMGWindow::on_add_click);
	connect(ui->button_remove, &QPushButton::clicked, this,
		&MidiMGWindow::on_remove_click);
	connect(ui->button_return, &QPushButton::clicked, this,
		&MidiMGWindow::on_return_click);
	connect(ui->button_help_subject, &QPushButton::clicked, this,
		&MidiMGWindow::on_help_click);
	// Device Buttons
	connect(ui->button_active_device, &QAbstractButton::toggled, this,
		&MidiMGWindow::on_device_active_change);
	connect(ui->button_edit_input_bindings, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_BINDING); });
	// connect(ui->transfer_bindings_button, &QPushButton::clicked, this, [&]() { on_button_click(UiButtons::MIDIMGWINDOW_TRANSFER_BINDINGS); });
	// Preferences Buttons
	connect(ui->button_preferences, &QPushButton::clicked, this, [&]() {
		switch_structure_pane(MMGModes::MMGMODE_PREFERENCES);
	});
	connect(ui->editor_global_enable, &QCheckBox::toggled, this,
		[&](bool toggled) {
			global()->preferences().set_active(toggled);
		});
	connect(ui->button_export, &QPushButton::clicked, this,
		&MidiMGWindow::export_bindings);
	connect(ui->button_import, &QPushButton::clicked, this,
		&MidiMGWindow::import_bindings);
	connect(ui->button_help_advanced, &QPushButton::clicked, this,
		&MidiMGWindow::i_need_help);
	connect(ui->button_bug_report, &QPushButton::clicked, this,
		&MidiMGWindow::report_a_bug);
	// List Widget Connections
	connect(ui->editor_structure, &QListWidget::itemClicked, this,
		&MidiMGWindow::on_list_selection_change);
	connect(ui->editor_structure, &QListWidget::itemChanged, this,
		&MidiMGWindow::on_name_edit);
	connect(ui->editor_structure->model(), &QAbstractItemModel::rowsMoved,
		this, &MidiMGWindow::on_element_drag);
}

void MidiMGWindow::configure_lcd_widgets()
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

void MidiMGWindow::set_help_text(enum MMGModes mode)
{
	switch (mode) {
	case MMGModes::MMGMODE_DEVICE:
		help_name = "Device Properties";
		help_str =
			"The device {name} {in_status} a connected input port ready to be used.\n\n"
			"The device {name} {out_status} a connected output port ready to be used.\n\n"
			"The device {name} is currently {active?}.\n\n"
			"The device {name} currently has {size} bindings. Click Edit Input Bindings to view them.";
		format_help_text(help_str, current_device);
		break;
	case MMGModes::MMGMODE_BINDING:
		help_name = "Binding Properties";
		help_str =
			"The binding {name} is executing actions using the {reception}.\n\n"
			"The binding {name} has message toggling set to {toggling}.\n\n"
			"The binding {name} currently has {message_size} messages. Click Edit Messages to view them.\n\n"
			"The binding {name} currently has {action_size} actions. Click Edit Actions to view them.";
		format_help_text(help_str, current_binding);
		break;
	case MMGModes::MMGMODE_MESSAGE:
		help_name = "Message Properties";
		help_str =
			"The values set here are ones that the plugin will listen for in order to execute an action in OBS Studio. "
			"Using multiple messages in one binding will require that all messages in the binding be received (in order) "
			"before any actions are executed.\n\n"
			"The message {name} has the last value (velocity/value/pitch adj.) set to {value}.";
		format_help_text(help_str, current_message);
		break;
	case MMGModes::MMGMODE_ACTION:
		help_name = "Action Properties";
		help_str =
			"The values set here will execute some command in OBS Studio. "
			"When using multiple actions in one binding, each action will execute one after the other. "
			"The Binding Reception Method defines the behavior of where message values will go as parameters. "
			"View the Bindings help menu for more details.\n\n"
			"The current action category is {cat}, and the subcategory is {sub}.\n\n";
		help_str.replace("{cat}", ui->editor_cat->currentText());
		help_str.replace("{sub}", ui->editor_sub->currentText());
		format_help_text(help_action_str1, current_action);
		format_help_text(help_action_str2, current_action);
		format_help_text(help_action_str3, current_action);
		format_help_text(help_action_double1, current_action);
		format_help_text(help_action_double2, current_action);
		format_help_text(help_action_double3, current_action);
		format_help_text(help_action_double4, current_action);
		help_str += help_action_desc + help_action_str1 +
			    help_action_str2 + help_action_str3 +
			    help_action_double1 + help_action_double2 +
			    help_action_double3 + help_action_double4;
		break;
	default:
		help_name = "Error";
		format_help_text(help_str, (void *)0);
		break;
	}
}

void MidiMGWindow::set_device_view()
{
	// QStringList list = get_device_names();

	ui->text_device_name->setText(current_device->get_name());
	ui->text_status_input->setText(current_device->input_device_status());
	ui->text_status_output->setText(current_device->output_device_status());
	ui->button_active_device->setChecked(
		current_device->get_name() ==
		global()->get_active_device_name());
	on_device_active_change(current_device->get_name() ==
				global()->get_active_device_name());
	// list.removeOne(current->text());
	// ui->transfer_bindings_name_editor->clear();
	// ui->transfer_bindings_name_editor->addItems(list);
	ui->button_edit_input_bindings->setEnabled(
		ui->text_status_input->text() == "Ready");
	ui->button_edit_output_bindings->setEnabled(false);
}

void MidiMGWindow::set_binding_view()
{
	ui->text_binding_name->setText(current_binding->get_name());
	ui->editor_message_toggling->setCurrentIndex(
		(int)current_binding->get_toggling());
	ui->editor_reception_method->setCurrentIndex(
		(int)current_binding->get_reception() - 1);
}

void MidiMGWindow::set_message_view()
{
	// Because current_message is modified in these function calls (LCDData::reset),
	// this uses a const version of it to get its values
	const MMGMessage temp = *current_message;

	ui->text_message_name->setText(temp.get_name());

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

void MidiMGWindow::set_action_view()
{
	// Because current_action is modified in these function calls (LCDData::reset),
	// this uses a const version of it to get its values
	const MMGAction temp = *current_action;
	// Set name
	ui->text_action_name->setText(temp.get_name());
	// Set category
	ui->editor_cat->setCurrentIndex((int)temp.get_category());
	// Set subcategory
	ui->editor_sub->setCurrentIndex(temp.get_sub());
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

void MidiMGWindow::set_strs_visible(bool str1, bool str2, bool str3) const
{
	ui->label_str1->setVisible(str1);
	ui->editor_str1->setVisible(str1);
	ui->label_str2->setVisible(str2);
	ui->editor_str2->setVisible(str2);
	ui->label_str3->setVisible(str3);
	ui->editor_str3->setVisible(str3);
}

void MidiMGWindow::set_doubles_visible(bool double1, bool double2, bool double3,
				       bool double4) const
{
	SET_LCD_STATUS(double1, Visible, double1);
	SET_LCD_STATUS(double2, Visible, double2);
	SET_LCD_STATUS(double3, Visible, double3);
	SET_LCD_STATUS(double4, Visible, double4);
}

void MidiMGWindow::on_device_active_change(bool toggled)
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

void MidiMGWindow::on_binding_reception_select(int index)
{
	current_binding->set_reception((MMGBinding::Reception)(index + 1));
}

void MidiMGWindow::on_binding_toggling_select(int index)
{
	current_binding->set_toggling((MMGBinding::Toggling)index);
}

void MidiMGWindow::on_message_type_change(const QString &type)
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

void MidiMGWindow::on_message_listen(bool toggled)
{
	global()->set_listening(toggled);
	ui->button_listen->setText(toggled ? "Cancel..."
					   : "Listen for Message...");
}

void MidiMGWindow::on_message_require_value(bool toggled)
{
	current_message->set_value_required(toggled);
	ui->button_value_require->setText(toggled ? "Value is Required"
						  : "Require Value...");
	set_message_view();
}

void MidiMGWindow::on_action_cat_change(int index)
{
	current_action->set_category((MMGAction::Category)index);

	set_strs_visible();
	current_action->set_sub(0);
	set_str1("");
	set_str2("");
	set_str3("");
	set_double1(0.0);
	set_double2(0.0);
	set_double3(0.0);
	set_double4(0.0);

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
		set_sub_options({"None"});
		break;
	case MMGAction::Category::MMGACTION_STREAM:
		set_sub_options({"Start Streaming", "Stop Streaming",
				 "Toggle Streaming"});
		break;
	case MMGAction::Category::MMGACTION_RECORD:
		set_sub_options({"Start Recording", "Stop Recording",
				 "Toggle Recording", "Pause Recording",
				 "Resume Recording", "Toggle Pause Recording"});
		break;
	case MMGAction::Category::MMGACTION_VIRCAM:
		set_sub_options({"Start Virtual Camera", "Stop Virtual Camera",
				 "Toggle Virtual Camera"});
		break;
	case MMGAction::Category::MMGACTION_REPBUF:
		set_sub_options({"Start Replay Buffer", "Stop Replay Buffer",
				 "Toggle Replay Buffer", "Save Replay Buffer"});
		break;
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		set_sub_options({"Turn On Studio Mode", "Turn Off Studio Mode",
				 "Toggle Studio Mode", "Change Preview Scene",
				 "Transition from Preview to Program"});
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		set_sub_options({"Scene Switching"});
		break;
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
		set_sub_options(
			{"Move Source", "Display Source", "Source Locking",
			 "Source Crop", "Align Source", "Source Scale",
			 "Source Scale Filtering", "Rotate Source",
			 "Source Bounding Box Type",
			 "Resize Source Bounding Box",
			 "Align Source Bounding Box", "Source Blending Mode",
			 "Take Source Screenshot"});
		break;
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
		set_sub_options(
			{"Change Source Volume To", "Change Source Volume By",
			 "Mute Source", "Unmute Source", "Toggle Source Mute",
			 "Source Audio Offset", "Source Audio Monitor"});
		break;
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:
		set_sub_options({"Play or Pause", "Restart", "Stop",
				 "Set Track Time", "Next Track",
				 "Previous Track", "Skip Forward Time",
				 "Skip Backward Time"});
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		set_sub_options(
			{"Change Current Transition",
			 /*"Set Transition Bar Position (Studio Mode)",*/
			 "Set Source Show Transition",
			 "Set Source Hide Transition"});
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		set_sub_options({"Show Filter", "Hide Filter",
				 "Toggle Filter Display",
				 "Reorder Filter Appearance"});
		break;
	case MMGAction::Category::MMGACTION_HOTKEY:
		set_sub_options({"Activate Hotkey"});
		break;
	case MMGAction::Category::MMGACTION_PROFILE:
		set_sub_options({"Switch Profiles"});
		break;
	case MMGAction::Category::MMGACTION_COLLECTION:
		set_sub_options({"Switch Scene Collections"});
		break;
	case MMGAction::Category::MMGACTION_MIDI:
		set_sub_options({"Send a MIDI Message"});
		break;
	case MMGAction::Category::MMGACTION_TIMEOUT:
		set_sub_options({"Wait in Milliseconds", "Wait in Seconds"});
		break;
	}
}

void MidiMGWindow::set_sub_options(std::initializer_list<QString> list) const
{
	ui->label_sub->setText("Options");
	ui->editor_sub->clear();
	ui->editor_sub->addItems(list);
	ui->editor_sub->setCurrentIndex(0);
}

void MidiMGWindow::on_action_sub_change(int index)
{
	current_action->set_sub(index);

	ui->editor_str1->clear();
	ui->editor_str2->clear();
	ui->editor_str3->clear();
	ui->lcd_double1->display(0);
	ui->lcd_double2->display(0);
	ui->lcd_double3->display(0);
	ui->lcd_double4->display(0);
	set_strs_visible();
	set_doubles_visible();

	help_action_desc.clear();
	help_action_str1.clear();
	help_action_str2.clear();
	help_action_str3.clear();
	help_action_double1.clear();
	help_action_double2.clear();
	help_action_double3.clear();
	help_action_double4.clear();

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
		ui->editor_sub->setCurrentText("None");
		help_action_desc =
			"DESCRIPTION: Does exactly what you'd expect.\n\n";
		break;
	case MMGAction::Category::MMGACTION_STREAM:
		help_action_desc =
			"DESCRIPTION: Streams can be started or stopped.\n\n";
		break;
	case MMGAction::Category::MMGACTION_RECORD:
		help_action_desc =
			"DESCRIPTION: Recordings can be started, stopped or paused.\n\n";
		break;
	case MMGAction::Category::MMGACTION_VIRCAM:
		help_action_desc =
			"DESCRIPTION: The virtual camera can be started or stopped.\n\n";
		break;
	case MMGAction::Category::MMGACTION_REPBUF:
		help_action_desc =
			"DESCRIPTION: Replay Buffers can be started, stopped, or saved.\n\n";
		break;
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		help_action_desc =
			"DESCRIPTION: Studio mode can be enabled or disabled, and preview scenes can be switched.\n\n";
		if (index == 3) {
			set_strs_visible(true);
			ui->label_str1->setText("Scene");
			MMGAction::do_obs_scene_enum(ui->editor_str1);
			ui->editor_str1->addItem("Use Message Value");
			help_action_desc =
				"DESCRIPTION: Changes the preview scene while in studio mode.\n\n";
			help_action_str1 =
				"SCENE: Select the scene to transition the preview to (if it is enabled). "
				"Currently, the preview scene will switch to {str1}.\n\n"
				"The option \"Use Message Value\" allows for the value of the message to be used in place of the scene name. "
				"The value 0 is for the first scene, 1 for the second, and so on. "
				"This means that if there are more than 128 scenes, this will not be able to switch to them.";
		}
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		set_strs_visible(true);
		ui->label_str1->setText("Scene");
		MMGAction::do_obs_scene_enum(ui->editor_str1);
		ui->editor_str1->addItem("Use Message Value");
		help_action_desc =
			"DESCRIPTION: Changes the scene using the set transition.\n\n";
		help_action_str1 =
			"SCENE: Select the scene to transition to. "
			"Currently, the scene will switch to {str1}.\n\n"
			"The option \"Use Message Value\" allows for the value of the message to be used in place of the scene name. "
			"The value 0 is for the first scene, 1 for the second, and so on. "
			"This means that if there are more than 128 scenes, this will not be able to switch to them.";
		break;
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
		set_strs_visible(true);
		ui->label_str1->setText("Scene");
		MMGAction::do_obs_scene_enum(ui->editor_str1);
		help_action_str1 =
			"SCENE: Select the scene in which the video source is to be used. "
			"Currently, the scene that will be used to find sources is {str1}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
		set_strs_visible(true);
		ui->label_str1->setText("Source");
		MMGAction::do_obs_source_enum(
			ui->editor_str1,
			MMGAction::Category::MMGACTION_SOURCE_AUDIO);
		help_action_str1 =
			"SOURCE: Select the audio source that is to be used. "
			"Currently, the audio source that will be used is {str1}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:
		set_strs_visible(true);
		ui->label_str1->setText("Source");
		MMGAction::do_obs_media_enum(ui->editor_str1);
		help_action_str1 =
			"SOURCE: Select the media source that is to be used. "
			"Currently, the media source that will be used is {str1}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		set_strs_visible(true);
		ui->label_str1->setText("Transition");
		MMGAction::do_obs_transition_enum(ui->editor_str1);
		help_action_str1 =
			"TRANSITION: Select the transition that is to be used. "
			"Currently, the transition that will be used is {str1}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		set_strs_visible(true);
		ui->label_str1->setText("Source");
		MMGAction::do_obs_source_enum(
			ui->editor_str1, MMGAction::Category::MMGACTION_FILTER);
		help_action_str1 =
			"SOURCE: Select the source containing the filter that is to be used. "
			"Currently, the source selected is {str1}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_HOTKEY:
		set_strs_visible(true);
		ui->label_str1->setText("Hotkey");
		MMGAction::do_obs_hotkey_enum(ui->editor_str1);
		help_action_desc =
			"DESCRIPTION: Triggers the provided hotkey.\n\n";
		help_action_str1 =
			"HOTKEY: Select the hotkey that is to be activated. "
			"Currently, the hotkey that will be activated is {str1}.";
		break;
	case MMGAction::Category::MMGACTION_PROFILE:
		set_strs_visible(true);
		ui->label_str1->setText("Profile");
		MMGAction::do_obs_profile_enum(ui->editor_str1);
		ui->editor_str1->addItem("Use Message Value");
		help_action_desc =
			"DESCRIPTION: Changes the current profile.\n\n";
		help_action_str1 =
			"PROFILE: Select the profile to switch to. "
			"Currently, the profile will switch to {str1}.\n\n"
			"The option \"Use Message Value\" allows for the value of the message to be used in place of the profile name. "
			"The value 0 is for the first profile, 1 for the second, and so on. "
			"This means that if there are more than 128 profiles, this will not be able to switch to them.";
		break;
	case MMGAction::Category::MMGACTION_COLLECTION:
		set_strs_visible(true);
		ui->label_str1->setText("Collection");
		MMGAction::do_obs_collection_enum(ui->editor_str1);
		ui->editor_str1->addItem("Use Message Value");
		help_action_desc =
			"DESCRIPTION: Changes the current scene collection.\n\n";
		help_action_str1 =
			"COLLECTION: Select the scene collection to switch to. "
			"Currently, the scene collection will switch to {str1}.\n\n"
			"The option \"Use Message Value\" allows for the value of the message to be used in place of the scene collection name. "
			"The value 0 is for the first scene collection, 1 for the second, and so on. "
			"This means that if there are more than 128 scene collections, this will not be able to switch to them.";
		break;
	case MMGAction::Category::MMGACTION_MIDI:
		set_strs_visible(true);
		ui->label_str1->setText("Device");
		ui->editor_str1->addItems(MMGDevice::get_output_device_names());
		help_action_desc =
			"DESCRIPTION: Sends a MIDI message to a connected device.\n\n";
		help_action_str1 =
			"DEVICE: Select the MIDI device that will be sent the message. "
			"Currently, the device that will be used is {str1}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_TIMEOUT:
		set_doubles_visible(true);
		ui->label_double1->setText("Time");
		lcd_double1.set_range(0.0, 1000.0);
		lcd_double1.set_step(1.0, 10.0);
		lcd_double1.reset();
		help_action_desc =
			"DESCRIPTION: Causes the plugin to wait for a specified amount of time before executing the next action.\n\n";
		help_action_double1 =
			"TIME: This is how much time the action will pause for before moving on to the next action (in the units specified). "
			"Using the value for this action limits the largest possible wait time to 128 instead of the default 1000 units.";
		break;
	default:
		break;
	}
}

void MidiMGWindow::set_channel(double value)
{
	current_message->set_channel(value);
}

void MidiMGWindow::set_note(double value)
{
	if (ui->lcd_value->isVisible()) {
		current_message->set_note(value);
	} else {
		set_value(value);
	}
}

void MidiMGWindow::set_value(double value)
{
	current_message->set_value(value);
}

void MidiMGWindow::set_str1(const QString &value)
{
	current_action->set_str(0, value);

	set_strs_visible(true);
	set_doubles_visible();

	if (value.isEmpty())
		return;

	ui->editor_str2->clear();
	lcd_double1.set_use_time(false);

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
		set_strs_visible(true, true);
		ui->label_str2->setText("Source");
		MMGAction::do_obs_source_enum(
			ui->editor_str2, MMGAction::Category::MMGACTION_SCENE,
			value);
		help_action_str2 =
			"SOURCE: Select the video source within the scene that is to be used. "
			"Currently, the source that will be used is {str2}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
		switch ((MMGAction::AudioSources)current_action->get_sub()) {
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGETO:
			set_doubles_visible(true);
			ui->label_double1->setText("Volume");
			lcd_double1.set_range(0.0, 100.0);
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Adjusts the source volume to a certain percentage.\n\n";
			help_action_double1 =
				"VOLUME: Sets the volume of the audio source (as a percentage). "
				"When using the message value, the volume will smoothly transition from 0% to 100%, "
				"with 0 corresponding to 0% and 127 corresponding to 100%. "
				"There is not a gain option for this setting.";
			break;
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGEBY:
			set_doubles_visible(true);
			ui->label_double1->setText("Volume Adj.");
			lcd_double1.set_range(-50.0, 50.0);
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Increase or decrease the source volume by a certain percentage.\n\n";
			help_action_double1 =
				"VOLUME ADJUSTMENT: Changes the volume of the audio source (by a percentage). "
				"When using the message value, 0 corresponds to -50% and 127 corresponding to +50%. "
				"Use a value of 64 to change by 0%. "
				"The volume will cap at 100%, so there is not a gain option for this setting.";
			break;
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_MUTE_ON:
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_MUTE_OFF:
		case MMGAction::AudioSources::
			SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF:
			help_action_desc =
				"DESCRIPTION: Mutes or unmutes the source.\n\n";
			break;
		case MMGAction::AudioSources::SOURCE_AUDIO_OFFSET:
			set_doubles_visible(true);
			ui->label_double1->setText("Sync Offset");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Change the audio source's sync offset.\n\n";
			help_action_double1 =
				"SYNC OFFSET: Changes the sync offset of the audio source (in milliseconds). "
				"When using the message value, the hard limit for using the value is 3175ms, "
				"as each MIDI value increments the sync offset by 25ms. "
				"This is compared to the fixed 20,000ms limit when not using the MIDI value.";
			break;
		case MMGAction::AudioSources::SOURCE_AUDIO_MONITOR:
			set_strs_visible(true, true);
			ui->label_str2->setText("Monitor");
			ui->editor_str2->addItems({"Off", "Monitor Only",
						   "Monitor & Output",
						   "Use Message Value"});
			help_action_desc =
				"DESCRIPTION: Change the audio source's audio monitor.\n\n";
			help_action_str2 =
				"MONITOR: Changes the audio monitor of the audio source. "
				"When using the message value, the values of 0-2 correspond to the options. "
				"Other values will do nothing.";
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:
		lcd_double1.set_use_time(true);
		switch ((MMGAction::MediaSources)current_action->get_sub()) {
		case MMGAction::MediaSources::SOURCE_MEDIA_TOGGLE_PLAYPAUSE:
			help_action_desc =
				"DESCRIPTION: Pause or play the media source.\n\n";
			break;
		case MMGAction::MediaSources::SOURCE_MEDIA_RESTART:
			help_action_desc =
				"DESCRIPTION: Restart the media source.\n\n";
			break;
		case MMGAction::MediaSources::SOURCE_MEDIA_STOP:
			help_action_desc =
				"DESCRIPTION: Stop or play the media source.\n\n";
			break;
		case MMGAction::MediaSources::SOURCE_MEDIA_TIME:
			set_doubles_visible(true);
			ui->label_double1->setText("Time");
			lcd_double1.set_range(
				0.0, get_obs_media_length(
					     current_action->get_str(0)));
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Set the media source's track time.\n\n";
			help_action_double1 =
				"TIME: Changes the time to set for the media source. "
				"When using the message value, each value will divide the track into equal sections. "
				"The value of 0 is the beginning, and the value of 127 is just before the end.";
			break;
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_FORWARD_TRACK:
			help_action_desc =
				"DESCRIPTION: Increase the track of the media source. (Track)\n\n";
			break;
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TRACK:
			help_action_desc =
				"DESCRIPTION: Decrease the track of the media source. (Seek)\n\n";
			break;
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_FORWARD_TIME:
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TIME:
			set_doubles_visible(true);
			ui->label_double1->setText("Time Adj.");
			lcd_double1.set_range(
				0.0, get_obs_media_length(
					     current_action->get_str(0)));
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Move around the media source's current track by some time. (Skipping Track Time)\n\n";
			help_action_double1 =
				"TIME: Changes the time interval to move for the media source. "
				"When using the message value, each value will divide the track into equal sections. "
				"The value of 0 is the current time, and the intervals get larger as the values increase.";
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		switch ((MMGAction::Transitions)current_action->get_sub()) {
		case MMGAction::Transitions::TRANSITION_CURRENT:
			set_doubles_visible(true);
			ui->label_double1->setText("Duration");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Change the current transition and/or its duration.\n\n";
			help_action_double1 =
				"DURATION: Changes the duration of the transition. "
				"Setting this field to 0 will use the default or current value for the duration. "
				"When using the message value, the hard limit for using the value is 3175ms, "
				"as each MIDI value increments the duration by 25ms. "
				"This is compared to the fixed 20,000ms limit when not using the MIDI value.";
			break;
		/*case MMGAction::Transitions::TRANSITION_TBAR:
			set_doubles_visible(true);
			ui->label_double1->setText("Position (%)");
			lcd_double1.set_range(0.0, 100.0);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			break;*/
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_strs_visible(true, true);
			ui->label_str2->setText("Scene");
			MMGAction::do_obs_scene_enum(ui->editor_str2);
			help_action_str2 =
				"SCENE: Select the scene of the source to apply the transition. "
				"Currently, the scene to be used is {str2}.\n\n";
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		set_strs_visible(true, true);
		ui->label_str2->setText("Filter");
		MMGAction::do_obs_filter_enum(
			ui->editor_str2,
			MMGAction::Category::MMGACTION_SOURCE_VIDEO, value);
		help_action_str2 =
			"FILTER: Select the filter to use from the source {str1}. "
			"Currently, the filter to be used is {str2}.\n\n";
		break;
	case MMGAction::Category::MMGACTION_MIDI:
		set_strs_visible(true, true);
		ui->label_str2->setText("Type");
		for (int i = 0; i < ui->editor_type->count(); ++i) {
			ui->editor_str2->addItem(ui->editor_type->itemText(i));
		}
		help_action_str2 =
			"TYPE: Select the type of MIDI message to send. "
			"Currently, the type of the message is {str2}.\n\n";
		break;
	default:
		break;
	}
}

void MidiMGWindow::set_str2(const QString &value)
{
	current_action->set_str(1, value);

	if (value.isEmpty())
		return;

	ui->editor_str3->clear();

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
		switch ((MMGAction::VideoSources)current_action->get_sub()) {
		case MMGAction::VideoSources::SOURCE_VIDEO_POSITION:
			set_doubles_visible(true, true);
			ui->label_double1->setText("Pos X");
			ui->label_double2->setText("Pos Y");
			lcd_double1.set_range(0.0, get_obs_dimensions().first);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(0.0, get_obs_dimensions().second);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			help_action_desc =
				"DESCRIPTION: Change the source's position in the scene.\n\n";
			help_action_double1 =
				"POSITION X: Sets the x-coordinate (horizontal position) in pixels of the source on the scene. "
				"When using the message value, the display width splits the values. "
				"A value of 0 corresponds to the far left, and a value of 127 corresponds to the far right.\n\n";
			help_action_double2 =
				"POSITION Y: Sets the y-coordinate (vertical position) in pixels of the source on the scene. "
				"When using the message value, the display height splits the values. "
				"A value of 0 corresponds to the top, and a value of 127 corresponds to the bottom.\n\n";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_DISPLAY:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Action");
			ui->editor_str3->addItems({"Show", "Hide", "Toggle"});
			help_action_desc =
				"DESCRIPTION: Show or hide the source in the scene.\n\n";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_LOCKED:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Action");
			ui->editor_str3->addItems(
				{"Locked", "Unlocked", "Toggle"});
			help_action_desc =
				"DESCRIPTION: Lock or unlock the source in the scene.\n\n";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_CROP:
			set_doubles_visible(true, true, true, true);
			ui->label_double1->setText("Top");
			ui->label_double2->setText("Right");
			ui->label_double3->setText("Bottom");
			ui->label_double4->setText("Left");
			lcd_double1.set_range(
				0.0,
				get_obs_source_dimensions(value).second >> 1);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(
				0.0,
				get_obs_source_dimensions(value).first >> 1);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			lcd_double3.set_range(
				0.0,
				get_obs_source_dimensions(value).second >> 1);
			lcd_double3.set_step(0.5, 5.0);
			lcd_double3.reset();
			lcd_double4.set_range(
				0.0,
				get_obs_source_dimensions(value).first >> 1);
			lcd_double4.set_step(0.5, 5.0);
			lcd_double4.reset();
			help_action_desc =
				"DESCRIPTION: Crop the source in the scene.\n\n";
			help_action_double1 =
				"CROP TOP: Sets the cropped pixels from the top of the source in the scene. "
				"When using the message value, the source height splits the values. "
				"A value of 0 corresponds to the top, and a value of 127 corresponds to the center of the source.\n\n";
			help_action_double2 =
				"CROP RIGHT: Sets the cropped pixels from the right of the source in the scene. "
				"When using the message value, the source width splits the values. "
				"A value of 0 corresponds to the far right, and a value of 127 corresponds to the center of the source.\n\n";
			help_action_double3 =
				"CROP BOTTOM: Sets the cropped pixels from the bottom of the source in the scene. "
				"When using the message value, the source height splits the values. "
				"A value of 0 corresponds to the bottom, and a value of 127 corresponds to the center of the source.\n\n";
			help_action_double4 =
				"CROP LEFT: Sets the cropped pixels from the left of the source in the scene. "
				"When using the message value, the source width splits the values. "
				"A value of 0 corresponds to the far left, and a value of 127 corresponds to the center of the source.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_ALIGNMENT:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Alignment");
			ui->editor_str3->addItems(
				{"Top Left", "Top Center", "Top Right",
				 "Middle Left", "Middle Center", "Middle Right",
				 "Bottom Left", "Bottom Center", "Bottom Right",
				 "Use Message Value"});
			help_action_desc =
				"DESCRIPTION: Align the position of the source in the scene.\n\n";
			help_action_str3 =
				"ALIGNMENT: Sets the alignment (based on position) of the source in the scene. "
				"When using the message value, the values of 0-8 correspond to the options. "
				"Other values will do nothing.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_SCALE:
			set_doubles_visible(true, true, true);
			ui->label_double1->setText("Scale X");
			ui->label_double2->setText("Scale Y");
			ui->label_double3->setText("Magn.");
			lcd_double1.set_range(0.0, 100.0);
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset(0.0);
			lcd_double2.set_range(0.0, 100.0);
			lcd_double2.set_step(1.0, 10.0);
			lcd_double2.reset(0.0);
			lcd_double3.set_range(0.5, 100.0);
			lcd_double3.set_step(0.5, 5.0);
			lcd_double3.reset(1.0);
			help_action_desc =
				"DESCRIPTION: Scale the source in the scene.\n\n";
			help_action_double1 =
				"SCALE X: Sets the size of the source in the scene based in the x-axis. "
				"This value is calculated by a percentage. "
				"When using the message value, a value of 0 corresponds to a 0 by y source, "
				"and a value of 127 corresponds to a full x-axis size source.\n\n";
			help_action_double2 =
				"SCALE Y: Sets the size of the source in the scene based in the y-axis. "
				"This value is calculated by a percentage. "
				"When using the message value, a value of 0 corresponds to an x by 0 source, "
				"and a value of 127 corresponds to a full y-axis size source.\n\n";
			help_action_double3 =
				"MAGNITUDE: Sets the scale of the scale of the source in the scene. "
				"A magnitude of 1 is the default - it will scale to the source's normal size. "
				"A magnitude of 2 will double the normal size of the source, etc.\n"
				"NOTE: The message value cannot be used for this field!! If this is tried, it will default to 1.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_SCALEFILTER:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Filtering");
			ui->editor_str3->addItems(
				{"Disable", "Point", "Bilinear", "Bicubic",
				 "Lanczos", "Area", "Use Message Value"});
			help_action_desc =
				"DESCRIPTION: Set the scale filtering of the source in the scene.\n\n";
			help_action_str3 =
				"SCALE FILTERING: Sets the scale filtering of the source in the scene. "
				"When using the message value, the values of 0-5 correspond to the options. "
				"Other values will do nothing.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_ROTATION:
			set_doubles_visible(true);
			ui->label_double1->setText("Rotation");
			lcd_double1.set_range(0.0, 360.0);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Rotate the source in the scene.\n\n";
			help_action_double1 =
				"ROTATION: Sets the amount of rotation (in degrees) of the source in the scene counterclockwise. "
				"When using the message value, a value of 0 corresponds to 0 degrees, "
				"and a value of 127 corresponds to about 357 degrees.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_TYPE:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Type");
			ui->editor_str3->addItems(
				{"No Bounds", "Stretch to Bounds",
				 "Scale to Inner Bounds",
				 "Scale to Outer Bounds",
				 "Scale to Width of Bounds",
				 "Scale to Height of Bounds", "Maximum Size",
				 "Use Message Value"});
			help_action_desc =
				"DESCRIPTION: Set the bounding box type of the source in the scene.\n\n";
			help_action_str3 =
				"BOUNDING BOX TYPE: Sets the bounding box type of the source in the scene. "
				"When using the message value, the values of 0-6 correspond to the options. "
				"Other values will do nothing.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_SIZE:
			set_doubles_visible(true, true);
			ui->label_double1->setText("Size X");
			ui->label_double2->setText("Size Y");
			lcd_double1.set_range(
				0.0, get_obs_source_dimensions(
					     ui->editor_str1->currentText())
					     .first);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(
				0.0, get_obs_source_dimensions(
					     ui->editor_str1->currentText())
					     .second);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			help_action_desc =
				"DESCRIPTION: Change the size of the bounding box in the scene.\n\n";
			help_action_double1 =
				"SIZE X: Sets the size in pixels of the width of the bounding box source on the scene. "
				"When using the message value, the source width splits the values. "
				"A value of 0 corresponds to the far left, and a value of 127 corresponds to the far right.\n\n";
			help_action_double2 =
				"SIZE Y: Sets the size in pixels of the height of the bounding box source on the scene. "
				"When using the message value, the display height splits the values. "
				"A value of 0 corresponds to the top, and a value of 127 corresponds to the bottom.\n\n";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Alignment");
			ui->editor_str3->addItems(
				{"Top Left", "Top Center", "Top Right",
				 "Middle Left", "Middle Center", "Middle Right",
				 "Bottom Left", "Bottom Center", "Bottom Right",
				 "Use Message Value"});
			help_action_desc =
				"DESCRIPTION: Align the bounding box of the source in the scene.\n\n";
			help_action_str3 =
				"ALIGNMENT: Sets the alignment (based on the bounding box) of the source in the scene. "
				"When using the message value, the values of 0-8 correspond to the options. "
				"Other values will do nothing.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_BLEND_MODE:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Blend Mode");
			ui->editor_str3->addItems(
				{"Normal", "Additive", "Subtract", "Screen",
				 "Multiply", "Lighten", "Darken",
				 "Use Message Value"});
			help_action_desc =
				"DESCRIPTION: Set the blend mode of the source in the scene.\n\n";
			help_action_str3 =
				"BLEND MODE: Sets the blend mode of the source in the scene. "
				"When using the message value, the values of 0-6 correspond to the options. "
				"Other values will do nothing.";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_SCREENSHOT:
			help_action_desc =
				"DESCRIPTION: Screenshots the source.\n\n";
			break;
		case MMGAction::VideoSources::SOURCE_VIDEO_CUSTOM:
			help_action_desc = "DESCRIPTION: Does nothing.\n\n";
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
		if (current_action->get_sub() == 6) {
			if (value == "Use Message Value") {
				set_double1(-1);
			} else {
				set_double1(ui->editor_str2->currentIndex());
			}
		}
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		switch ((MMGAction::Transitions)current_action->get_sub()) {
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Source");
			MMGAction::do_obs_source_enum(
				ui->editor_str3,
				MMGAction::Category::MMGACTION_SCENE, value);
			help_action_desc =
				"DESCRIPTION: Sets the show or hide transition of the source in the scene.\n\n";
			help_action_str3 =
				"SOURCE: Select the source within the scene that is to be used. "
				"Currently, the source that will be used is {str3}.\n\n";
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		switch ((MMGAction::Filters)current_action->get_sub()) {
		case MMGAction::Filters::FILTER_REORDER:
			set_doubles_visible(true);
			ui->label_double1->setText("Position");
			lcd_double1.set_range(
				1.0, get_obs_source_filter_count(
					     ui->editor_str1->currentText()));
			lcd_double1.set_step(1.0, 5.0);
			lcd_double1.reset(1.0);
			help_action_desc =
				"DESCRIPTION: Moves the filter position in a source.\n\n";
			help_action_double1 =
				"POSITION: Sets the position or index of the filter in the scene. "
				"When using the message value, the index is equivalent to the values. "
				"If a value is larger than the filter count, it is ignored. "
				"If there are more than 128 filters, MIDI values cannot access those filter positions.";
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_MIDI:
		ui->label_double1->setText("Channel");
		lcd_double1.set_range(1.0, 16.0);
		lcd_double1.set_step(1.0, 5.0);
		lcd_double1.reset(1.0);
		lcd_double2.set_range(0.0, 127.0);
		lcd_double2.set_step(1.0, 10.0);
		lcd_double2.reset();
		lcd_double3.set_range(0.0, 127.0);
		lcd_double3.set_step(1.0, 10.0);
		lcd_double3.reset();
		if (value == "Note On" || value == "Note Off") {
			set_doubles_visible(true, true, true);
			ui->label_double2->setText("Note #");
			ui->label_double3->setText("Velocity");
		} else if (value == "Control Change") {
			set_doubles_visible(true, true, true);
			ui->label_double2->setText("Control");
			ui->label_double3->setText("Value");
		} else if (value == "Program Change") {
			set_doubles_visible(true, true);
			ui->label_double2->setText("Program");
		} else if (value == "Pitch Bend") {
			set_doubles_visible(true, true);
			ui->label_double2->setText("Pitch Adj");
		}
		help_action_double1 =
			"CHANNEL: Sets the channel of the message to send.\n"
			"NOTE: This value cannot be used with MIDI values!! Doing so will cause the channel to default to 1.\n\n";
		help_action_double2 =
			"DATA 1: Sets the note or control or program or pitch of the message to send.\n"
			"When using the message note or control or program or pitch, the correspondence is 1:1.\n\n";
		help_action_double3 =
			"DATA 2: Sets the velocity or value of the message to send.\n"
			"When using the message velocity or value, the correspondence is 1:1.\n\n";
		break;
	default:
		break;
	}
}

void MidiMGWindow::set_str3(const QString &value)
{
	current_action->set_str(2, value);

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
		switch ((MMGAction::VideoSources)current_action->get_sub()) {
		case MMGAction::VideoSources::SOURCE_VIDEO_ALIGNMENT:
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_TYPE:
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
		case MMGAction::VideoSources::SOURCE_VIDEO_SCALEFILTER:
		case MMGAction::VideoSources::SOURCE_VIDEO_BLEND_MODE:
			if (value == "Use Message Value") {
				set_double1(-1);
			} else {
				set_double1(ui->editor_str3->currentIndex());
			}
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		switch ((MMGAction::Transitions)current_action->get_sub()) {
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_doubles_visible(true);
			ui->label_double1->setText("Duration");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
			help_action_desc =
				"DESCRIPTION: Change the source show or hide transition and/or its duration.\n\n";
			help_action_double1 =
				"DURATION: Changes the duration of the transition. "
				"Setting this field to 0 will use the default or current value for the duration. "
				"When using the message value, the hard limit for using the value is 3175ms, "
				"as each MIDI value increments the duration by 25ms. "
				"This is compared to the fixed 20,000ms limit when not using the MIDI value.";
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void MidiMGWindow::set_double1(double value)
{
	current_action->set_num(0, value);
}

void MidiMGWindow::set_double2(double value)
{
	current_action->set_num(1, value);
}

void MidiMGWindow::set_double3(double value)
{
	current_action->set_num(2, value);
}

void MidiMGWindow::set_double4(double value)
{
	current_action->set_num(3, value);
}

void MidiMGWindow::on_action_double1_toggle(bool toggle)
{
	set_double1(0 - toggle);
	SET_LCD_STATUS(double1, Disabled, toggle);
	ui->label_double1->setDisabled(false);
	lcd_double1.display();
}

void MidiMGWindow::on_action_double2_toggle(bool toggle)
{
	set_double2(0 - toggle);
	SET_LCD_STATUS(double2, Disabled, toggle);
	ui->label_double2->setDisabled(false);
	lcd_double2.display();
}

void MidiMGWindow::on_action_double3_toggle(bool toggle)
{
	set_double3(0 - toggle);
	SET_LCD_STATUS(double3, Disabled, toggle);
	ui->label_double3->setDisabled(false);
	lcd_double3.display();
}

void MidiMGWindow::on_action_double4_toggle(bool toggle)
{
	set_double4(0 - toggle);
	SET_LCD_STATUS(double4, Disabled, toggle);
	ui->label_double4->setDisabled(false);
	lcd_double4.display();
}

void MidiMGWindow::on_add_click()
{
	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_BINDING:
		current_binding = current_device->add();
		add_widget_item(MMGModes::MMGMODE_BINDING,
				current_binding->get_name());
		break;

	case MMGModes::MMGMODE_MESSAGE:
		if ((int)current_binding->get_toggling() < 1 &&
		    current_binding->message_size() < 1) {
			current_message = current_binding->add_message();
			add_widget_item(MMGModes::MMGMODE_MESSAGE,
					current_message->get_name());
		}
		break;

	case MMGModes::MMGMODE_ACTION:
		current_action = current_binding->add_action();
		add_widget_item(MMGModes::MMGMODE_ACTION,
				current_action->get_name());
		break;

	default:
		break;
	}
}

void MidiMGWindow::on_remove_click()
{
	QListWidgetItem *current = ui->editor_structure->currentItem();
	if (!current)
		return;
	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_BINDING:
		current_device->remove(current_binding);
		delete current_binding;
		break;

	case MMGModes::MMGMODE_MESSAGE:
		current_binding->remove(current_message);
		delete current_message;
		break;

	case MMGModes::MMGMODE_ACTION:
		current_binding->remove(current_action);
		delete current_action;
		break;

	default:
		return;
	}

	ui->editor_structure->removeItemWidget(current);
	delete current;
	ui->button_remove->setEnabled(false);
	on_list_selection_change(nullptr);
}

void MidiMGWindow::on_return_click()
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

void MidiMGWindow::on_help_click()
{
	set_help_text(ui->editor_structure->property("mode").value<MMGModes>());
	open_message_box(help_name, help_str);
}

void MidiMGWindow::on_name_edit(QListWidgetItem *widget_item)
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

	case MMGModes::MMGMODE_MESSAGE:
		if (!current_message || current_message->get_name() == str)
			break;
		if (!!current_binding->find_message(str)) {
			widget_item->setText(current_message->get_name());
			break;
		}
		current_message->set_name(str);
		ui->text_message_name->setText(str);
		break;

	case MMGModes::MMGMODE_ACTION:
		if (!current_action || current_action->get_name() == str)
			break;
		if (!!current_binding->find_action(str)) {
			widget_item->setText(current_action->get_name());
			break;
		}
		current_action->set_name(str);
		ui->text_action_name->setText(str);
		break;

	default:
		break;
	}
}

void MidiMGWindow::on_list_selection_change(const QListWidgetItem *current)
{
	if (!current) {
		// No selection
		ui->pages->setCurrentIndex(0);
		return;
	}

	ui->button_add->setEnabled(true);
	ui->button_remove->setEnabled(true);
	ui->button_return->setEnabled(true);
	ui->button_help_subject->setEnabled(true);

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
		set_binding_view();
		ui->pages->setCurrentIndex(2);
		break;
	case MMGModes::MMGMODE_MESSAGE:
		current_message =
			current_binding->find_message(current->text());
		set_message_view();
		ui->pages->setCurrentIndex(3);
		break;
	case MMGModes::MMGMODE_ACTION:
		current_action = current_binding->find_action(current->text());
		set_action_view();
		ui->pages->setCurrentIndex(4);
		break;
	default:
		switch_structure_pane(MMGModes::MMGMODE_NONE);
		break;
	}
}

void MidiMGWindow::on_element_drag(const QModelIndex &parent, int start,
				   int end, const QModelIndex &dest,
				   int row) const
{
	Q_UNUSED(parent);
	Q_UNUSED(end);
	Q_UNUSED(dest);
	current_binding->move_elements(
		ui->editor_structure->property("mode").value<MMGModes>(), start,
		row);
}

void MidiMGWindow::switch_structure_pane(enum MMGModes mode)
{
	ui->editor_structure->clear();
	ui->editor_structure->setProperty("mode", QVariant::fromValue(mode));
	ui->button_add->setEnabled(false);
	ui->button_remove->setEnabled(false);
	ui->button_return->setEnabled(true);
	ui->button_help_subject->setEnabled(false);
	switch (mode) {
	case MMGModes::MMGMODE_PREFERENCES:
		ui->label_structure->setText(tr("Preferences"));
		ui->pages->setCurrentIndex(5);
		return;
	case MMGModes::MMGMODE_DEVICE:
		ui->button_return->setEnabled(false);
		for (const QString &name : global()->get_device_names()) {
			add_widget_item(mode, name);
		}
		ui->label_structure->setText(tr("Devices"));
		break;
	case MMGModes::MMGMODE_BINDING:
		ui->button_add->setEnabled(true);
		for (const MMGBinding *const binding_el :
		     current_device->get_bindings()) {
			add_widget_item(mode, binding_el->get_name());
		}
		ui->label_structure->setText(tr("Bindings"));
		break;
	case MMGModes::MMGMODE_MESSAGE:
		ui->button_add->setEnabled(true);
		for (const MMGMessage *const message_el :
		     current_binding->get_messages()) {
			add_widget_item(mode, message_el->get_name());
		}
		ui->label_structure->setText(tr("Messages"));
		break;
	case MMGModes::MMGMODE_ACTION:
		ui->button_add->setEnabled(true);
		for (const MMGAction *const action_el :
		     current_binding->get_actions()) {
			add_widget_item(mode, action_el->get_name());
		}
		ui->label_structure->setText(tr("Actions"));
		break;
	default:
		ui->button_return->setEnabled(false);
		ui->label_structure->setText(tr("Error"));
		ui->pages->setCurrentIndex(0);
		return;
	}
	on_list_selection_change(nullptr);
}

void MidiMGWindow::add_widget_item(MMGModes type, const QString &name) const
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
	case MMGModes::MMGMODE_MESSAGE:
		widget_item->setFlags((
			Qt::ItemFlag)0b100111); // Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled
		widget_item->setBackground(QColor::fromRgb(0, 40, 0, 128));
		break;
	case MMGModes::MMGMODE_ACTION:
		widget_item->setFlags((
			Qt::ItemFlag)0b100111); // Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled
		widget_item->setBackground(QColor::fromRgb(40, 0, 0, 128));
		break;
	default:
		delete widget_item;
		return;
	}
	ui->editor_structure->addItem(widget_item);
}

void MidiMGWindow::export_bindings()
{
	QString filepath = QFileDialog::getSaveFileName(
		this, tr("Save Bindings..."), MMGConfig::get_filepath(),
		"JSON Files (*.json)");
	if (!filepath.isNull())
		global()->save(filepath);
}

void MidiMGWindow::import_bindings()
{
	QString filepath = QFileDialog::getOpenFileName(
		this, tr("Open Bindings File..."), "", "JSON Files (*.json)");
	if (!filepath.isNull())
		global()->load(filepath);
}

void MidiMGWindow::i_need_help() const
{
	QDesktopServices::openUrl(QUrl(
		"https://github.com/nhielost/obs-midi-mg/blob/master/README.md"));
}

void MidiMGWindow::report_a_bug() const
{
	QDesktopServices::openUrl(
		QUrl("https://github.com/nhielost/obs-midi-mg/issues"));
}

MidiMGWindow::~MidiMGWindow()
{
	delete ui;
}

#undef SET_LCD_STATUS
#undef CONNECT_LCD
#undef INIT_LCD
