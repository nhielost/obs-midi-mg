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
#include <QFileDialog>
#include <QDesktopServices>

#include "midimg-window.h"
#include "../mmg-config.h"

using namespace MMGUtils;

#define SET_LCD_STATUS(lcd, kind, visible)        \
	ui->down_major_##lcd->set##kind(visible); \
	ui->down_minor_##lcd->set##kind(visible); \
	ui->up_minor_##lcd->set##kind(visible);   \
	ui->up_major_##lcd->set##kind(visible);   \
	ui->lcd_##lcd->set##kind(visible);

#define CONNECT_LCD(kind)                                           \
	connect(ui->down_major_##kind, &QPushButton::clicked, this, \
		[&]() { lcd_##kind.down_major(); });                \
	connect(ui->down_minor_##kind, &QPushButton::clicked, this, \
		[&]() { lcd_##kind.down_minor(); });                \
	connect(ui->up_minor_##kind, &QPushButton::clicked, this,   \
		[&]() { lcd_##kind.up_minor(); });                  \
	connect(ui->up_major_##kind, &QPushButton::clicked, this,   \
		[&]() { lcd_##kind.up_major(); });

#define INIT_LCD(kind) \
	lcd_##kind =   \
		LCDData(ui->lcd_##kind, [&](double val) { set_##kind(val); })

MidiMGWindow::MidiMGWindow(QWidget *parent)
	: QDialog(parent, Qt::Dialog), ui(new Ui::MidiMGWindow)
{
	this->setWindowFlags(this->windowFlags() &
			     ~Qt::WindowContextHelpButtonHint);
	ui->setupUi(this);
	ui->version_label->setText(OBS_MIDIMG_VERSION);

	ui->structure_editor->setDragEnabled(true);
	ui->structure_editor->setDragDropMode(
		QAbstractItemView::DragDropMode::InternalMove);
	ui->structure_editor->setAcceptDrops(true);
	ui->structure_editor->setSelectionMode(
		QAbstractItemView::SelectionMode::SingleSelection);
	ui->structure_editor->setDropIndicatorShown(true);

	configure_lcd_widgets();

	connect_ui_signals();
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
	connect(ui->binding_mode_editor,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		[&](int id) {
			on_binding_mode_select((MMGBinding::Mode)(id + 1));
		});
	connect(ui->view_message_button, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_MESSAGE); });
	connect(ui->view_action_button, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_ACTION); });
	// Message Display Connections
	connect(ui->message_type_editor, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::on_message_type_change);
	connect(ui->usevalue_value, &QCheckBox::toggled, this,
		[&](bool enabled) { SET_LCD_STATUS(value, Enabled, enabled); });
	connect(ui->usevalue_value, &QCheckBox::toggled, this,
		[&](bool toggled) { current_message->set_value(toggled - 1); });
	// Action Display Connections
	connect(ui->action_cat_editor, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::on_action_cat_change);
	connect(ui->action_sub_editor,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		&MidiMGWindow::on_action_sub_change);
	connect(ui->action_list1_editor, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::set_list1);
	connect(ui->action_list2_editor, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::set_list2);
	connect(ui->action_list3_editor, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::set_list3);
	connect(ui->usevalue_double1, &QCheckBox::toggled, this,
		[&](bool toggled) { set_doubles_usevalue(0, toggled); });
	connect(ui->usevalue_double2, &QCheckBox::toggled, this,
		[&](bool toggled) { set_doubles_usevalue(1, toggled); });
	connect(ui->usevalue_double3, &QCheckBox::toggled, this,
		[&](bool toggled) { set_doubles_usevalue(2, toggled); });
	connect(ui->usevalue_double4, &QCheckBox::toggled, this,
		[&](bool toggled) { set_doubles_usevalue(3, toggled); });
	// UI Movement Buttons
	connect(ui->add_button, &QPushButton::clicked, this,
		&MidiMGWindow::on_add_click);
	connect(ui->remove_button, &QPushButton::clicked, this,
		&MidiMGWindow::on_remove_click);
	connect(ui->return_button, &QPushButton::clicked, this,
		&MidiMGWindow::on_return_click);
	// Device Buttons
	connect(ui->view_input_bindings_button, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_BINDING); });
	// connect(ui->transfer_bindings_button, &QPushButton::clicked, this, [&]() { on_button_click(UiButtons::MIDIMGWINDOW_TRANSFER_BINDINGS); });
	// Preferences Buttons
	connect(ui->preferences_button, &QPushButton::clicked, this, [&]() {
		switch_structure_pane(MMGModes::MMGMODE_PREFERENCES);
	});
	connect(ui->enable_this_toggle, &QCheckBox::toggled, this,
		[&](bool toggled) { global()->set_running(toggled); });
	connect(ui->export_button, &QPushButton::clicked, this,
		&MidiMGWindow::export_bindings);
	connect(ui->import_button, &QPushButton::clicked, this,
		&MidiMGWindow::import_bindings);
	connect(ui->help_button, &QPushButton::clicked, this,
		&MidiMGWindow::i_need_help);
	connect(ui->bug_report_button, &QPushButton::clicked, this,
		&MidiMGWindow::report_a_bug);
	// List Widget Connections
	connect(ui->structure_editor, &QListWidget::itemClicked, this,
		&MidiMGWindow::on_list_selection_change);
	connect(ui->structure_editor, &QListWidget::itemChanged, this,
		&MidiMGWindow::on_name_edit);
	connect(ui->structure_editor->model(), &QAbstractItemModel::rowsMoved,
		this, &MidiMGWindow::on_element_drag);
}

void MidiMGWindow::show_window()
{
	setVisible(!isVisible());
	ui->structure_editor->clearSelection();
	current_device = global()->get_active_device();
	switch_structure_pane(MMGModes::MMGMODE_DEVICE);
	ui->pages->setCurrentIndex(0);
}

void MidiMGWindow::reject()
{
	global()->save();
	QDialog::reject();
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

const QStringList MidiMGWindow::get_device_names() const
{
	QStringList midi_in_names;
	for (MMGDevice *const device : global()->get_devices()) {
		midi_in_names.append(device->get_name());
	}
	return midi_in_names;
}

void MidiMGWindow::on_message_type_change(const QString &type)
{
	current_message->set_type(type);
	ui->message_type_editor->setCurrentText(type);

	ui->message_value_label->setVisible(true);
	ui->usevalue_value->setVisible(true);
	SET_LCD_STATUS(value, Visible, true);
	SET_LCD_STATUS(value, Enabled, true);

	if (type == "Note On" || type == "Note Off") {
		ui->message_note_label->setText("Note #");
		ui->message_value_label->setText("Velocity");
		SET_LCD_STATUS(value, Enabled, ui->usevalue_value->isChecked());
		ui->message_value_label->setEnabled(
			ui->usevalue_value->isChecked());
	} else if (type == "Control Change") {
		ui->message_note_label->setText("Control #");
		ui->message_value_label->setText("Value");
		SET_LCD_STATUS(value, Enabled, ui->usevalue_value->isChecked());
		ui->message_value_label->setEnabled(
			ui->usevalue_value->isChecked());
	} else if (type == "Program Change") {
		ui->message_note_label->setText("Program #");
		ui->message_value_label->setVisible(false);
		ui->usevalue_value->setVisible(false);
		SET_LCD_STATUS(value, Visible, false);
	} else if (type == "Pitch Bend") {
		ui->message_note_label->setText("Pitch Adj.");
		ui->message_value_label->setVisible(false);
		ui->usevalue_value->setVisible(false);
		SET_LCD_STATUS(value, Visible, false);
	}
}

void MidiMGWindow::set_device_view()
{
	// QStringList list = get_device_names();

	ui->device_name_text->setText(current_device->get_name());
	ui->device_input_text->setText(current_device->input_device_status());
	ui->device_output_text->setText(current_device->output_device_status());
	// list.removeOne(current->text());
	// ui->transfer_bindings_name_editor->clear();
	// ui->transfer_bindings_name_editor->addItems(list);
	ui->view_input_bindings_button->setEnabled(
		ui->device_input_text->text() == "Active");
	ui->view_output_bindings_button->setEnabled(false);
}

void MidiMGWindow::set_binding_view()
{
	ui->binding_name_text->setText(current_binding->get_name());
	ui->binding_mode_editor->setCurrentIndex(
		(int)current_binding->get_mode() - 1);
	on_binding_mode_select(current_binding->get_mode());
}

void MidiMGWindow::set_message_view()
{
	// Because current_message is modified in these function calls (LCDData::reset),
	// this uses a const version of it to get its values
	const MMGMessage temp = *current_message;

	ui->message_name_text->setText(temp.get_name());

	lcd_channel.reset(temp.get_channel());
	lcd_note.reset(temp.get_note());
	lcd_value.reset(temp.get_value() < 0 ? 0.0 : temp.get_value());
	ui->usevalue_value->setChecked(temp.get_value() >= 0);

	on_message_type_change(temp.get_type());
	// Re-set current_message to the correct one
	*current_message = temp;
}

void MidiMGWindow::set_action_view()
{
	// Because current_action is modified in these function calls (LCDData::reset),
	// this uses a const version of it to get its values
	const MMGAction temp = *current_action;
	// Set name
	ui->action_name_text->setText(temp.get_name());
	// Set category
	ui->action_cat_editor->setCurrentIndex((int)temp.get_category());
	on_action_cat_change(
		ui->action_cat_editor->itemText((int)temp.get_category()));
	// Set subcategory
	ui->action_sub_editor->setCurrentIndex(temp.get_sub());
	on_action_sub_change(temp.get_sub());
	// Set strings (even if they are invalid)
	ui->action_list1_editor->setCurrentText(temp.get_str(0));
	ui->action_list2_editor->setCurrentText(temp.get_str(1));
	ui->action_list3_editor->setCurrentText(temp.get_str(2));
	// Set doubles (extra jargon for using the message value)
	for (int i = 0; i < 4; ++i) {
		set_doubles_usevalue(i, temp.get_num(i) == -1);
	}
	ui->usevalue_double1->setChecked(temp.get_num(0) == -1);
	ui->usevalue_double2->setChecked(temp.get_num(1) == -1);
	ui->usevalue_double3->setChecked(temp.get_num(2) == -1);
	ui->usevalue_double4->setChecked(temp.get_num(3) == -1);
	lcd_double1.reset(temp.get_num(0) == -1 ? 0 : temp.get_num(0));
	lcd_double2.reset(temp.get_num(1) == -1 ? 0 : temp.get_num(1));
	lcd_double3.reset(temp.get_num(2) == -1 ? 0 : temp.get_num(2));
	lcd_double4.reset(temp.get_num(3) == -1 ? 0 : temp.get_num(3));
	// Re-set current_action to the correct one
	*current_action = temp;
}

void MidiMGWindow::on_action_cat_change(const QString &cat)
{
	current_action->set_category(MMGAction::categoryFromString(cat));

	set_sub_visible();
	set_lists_visible();
	current_action->set_sub(0);
	set_list1("");
	set_list2("");
	set_list3("");
	set_double1(0.0);
	set_double2(0.0);
	set_double3(0.0);
	set_double4(0.0);

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
		on_action_sub_change(0);
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
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		set_sub_options({"Turn On Studio Mode", "Turn Off Studio Mode",
				 "Toggle Studio Mode", "Change Preview Scene",
				 "Transition from Preview to Program"});
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		on_action_sub_change(0);
		break;
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		set_sub_options(
			{"Move Source", "Display Source", "Source Locking",
			 "Source Crop", "Source Scale", "Rotate Source",
			 "Source Bounding Box", "Reset Source Transform"});
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		set_sub_options(
			{"Change Source Volume To", "Change Source Volume By",
			 "Mute Source", "Unmute Source", "Toggle Source Mute",
			 "Take Source Screenshot", "Source Audio Offset",
			 "Source Audio Monitor"});
		break;
	case MMGAction::Category::MMGACTION_MEDIA:
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
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		on_action_sub_change(0);
		break;
	case MMGAction::Category::MMGACTION_WAIT:
		set_sub_options({"Wait in Milliseconds", "Wait in Seconds"});
		break;
	}
}

void MidiMGWindow::set_sub_visible(bool visible) const
{
	ui->action_sub_label->setVisible(visible);
	ui->action_sub_editor->setVisible(visible);
}

void MidiMGWindow::set_lists_visible(bool str1, bool str2, bool str3) const
{
	ui->action_list1_label->setVisible(str1);
	ui->action_list1_editor->setVisible(str1);
	ui->action_list2_label->setVisible(str2);
	ui->action_list2_editor->setVisible(str2);
	ui->action_list3_label->setVisible(str3);
	ui->action_list3_editor->setVisible(str3);
}

void MidiMGWindow::set_doubles_visible(bool double1, bool double2, bool double3,
				       bool double4) const
{
	ui->action_double1_label->setVisible(double1);
	ui->usevalue_double1->setVisible(double1);
	SET_LCD_STATUS(double1, Visible, double1);
	ui->action_double2_label->setVisible(double2);
	ui->usevalue_double2->setVisible(double2);
	SET_LCD_STATUS(double2, Visible, double2);
	ui->action_double3_label->setVisible(double3);
	ui->usevalue_double3->setVisible(double3);
	SET_LCD_STATUS(double3, Visible, double3);
	ui->action_double4_label->setVisible(double4);
	ui->usevalue_double4->setVisible(double4);
	SET_LCD_STATUS(double4, Visible, double4);
}

void MidiMGWindow::set_doubles_usevalue(short which, bool disabled) const
{
	switch (which) {
	case 0:
		SET_LCD_STATUS(double1, Disabled, disabled);
		break;
	case 1:
		SET_LCD_STATUS(double2, Disabled, disabled);
		break;
	case 2:
		SET_LCD_STATUS(double3, Disabled, disabled);
		break;
	case 3:
		SET_LCD_STATUS(double4, Disabled, disabled);
		break;
	default:
		return;
	}
	current_action->set_num(which, 0 - disabled);
}

void MidiMGWindow::set_sub_options(std::initializer_list<QString> list) const
{
	set_sub_visible(true);
	ui->action_sub_label->setText("Options");
	ui->action_sub_editor->clear();
	ui->action_sub_editor->addItems(list);
	ui->action_sub_editor->setCurrentIndex(0);
}

void MidiMGWindow::on_action_sub_change(int index)
{
	current_action->set_sub(index);

	ui->action_list1_editor->clear();
	ui->action_list2_editor->clear();
	ui->action_list3_editor->clear();
	ui->lcd_double1->display(0);
	ui->lcd_double2->display(0);
	ui->lcd_double3->display(0);
	ui->lcd_double4->display(0);
	set_lists_visible();
	set_doubles_visible();

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
	case MMGAction::Category::MMGACTION_STREAM:
	case MMGAction::Category::MMGACTION_RECORD:
	case MMGAction::Category::MMGACTION_VIRCAM:
		break;
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		if (index == 3) {
			set_lists_visible(true);
			ui->action_list1_label->setText("Scene");
			MMGAction::do_obs_scene_enum(ui->action_list1_editor);
		}
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		set_lists_visible(true);
		ui->action_list1_label->setText("Scene");
		MMGAction::do_obs_scene_enum(ui->action_list1_editor);
		ui->action_list1_editor->addItem("Use Message Value");
		break;
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		set_lists_visible(true);
		ui->action_list1_label->setText("Scene");
		MMGAction::do_obs_scene_enum(ui->action_list1_editor);
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		set_lists_visible(true);
		ui->action_list1_label->setText("Source");
		MMGAction::do_obs_source_enum(
			ui->action_list1_editor,
			MMGAction::Category::MMGACTION_SOURCE_PROPS,
			QString::number(index));
		break;
	case MMGAction::Category::MMGACTION_MEDIA:
		set_lists_visible(true);
		ui->action_list1_label->setText("Source");
		MMGAction::do_obs_media_enum(ui->action_list1_editor);
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		set_lists_visible(true);
		ui->action_list1_label->setText("Transition");
		MMGAction::do_obs_transition_enum(ui->action_list1_editor);
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		set_lists_visible(true);
		ui->action_list1_label->setText("Source");
		MMGAction::do_obs_source_enum(ui->action_list1_editor);
		break;
	case MMGAction::Category::MMGACTION_HOTKEY:
		set_lists_visible(true);
		ui->action_list1_label->setText("Hotkey");
		MMGAction::do_obs_hotkey_enum(ui->action_list1_editor);
		break;
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		set_lists_visible(true);
		ui->action_list1_label->setText("Device");
		ui->action_list1_editor->addItems(
			MMGDevice::get_output_device_names());
		break;
	case MMGAction::Category::MMGACTION_WAIT:
		set_doubles_visible(true);
		ui->action_double1_label->setText("Time");
		lcd_double1.set_range(0.0, 1000.0);
		lcd_double1.set_step(1.0, 10.0);
		lcd_double1.reset();
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
	current_message->set_note(value);
}

void MidiMGWindow::set_value(double value)
{
	current_message->set_value(value);
}

void MidiMGWindow::set_list1(const QString &value)
{
	current_action->set_str(0, value);

	set_lists_visible(true);
	set_doubles_visible();

	if (value.isEmpty())
		return;

	ui->action_list2_editor->clear();
	lcd_double1.set_use_time(false);

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		set_lists_visible(true, true);
		ui->action_list2_label->setText("Source");
		MMGAction::do_obs_source_enum(
			ui->action_list2_editor,
			MMGAction::Category::MMGACTION_SCENE, value);
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		switch ((MMGAction::SourceProperties)current_action->get_sub()) {
		case MMGAction::SourceProperties::PROPERTY_VOLUME_CHANGETO:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Volume");
			lcd_double1.set_range(0.0, 100.0);
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			break;
		case MMGAction::SourceProperties::PROPERTY_VOLUME_CHANGEBY:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Volume Adj.");
			lcd_double1.set_range(-50.0, 50.0);
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			break;
		case MMGAction::SourceProperties::PROPERTY_VOLUME_MUTE_ON:
		case MMGAction::SourceProperties::PROPERTY_VOLUME_MUTE_OFF:
		case MMGAction::SourceProperties::
			PROPERTY_VOLUME_MUTE_TOGGLE_ONOFF:
		case MMGAction::SourceProperties::PROPERTY_SCREENSHOT:
			break;
		case MMGAction::SourceProperties::PROPERTY_AUDIO_OFFSET:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Sync Offset");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
			break;
		case MMGAction::SourceProperties::PROPERTY_AUDIO_MONITOR:
			set_lists_visible(true, true);
			ui->action_list2_label->setText("Monitor");
			ui->action_list2_editor->addItems(
				{"Off", "Monitor Only", "Monitor & Output"});
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_MEDIA:
		lcd_double1.set_use_time(true);
		switch ((MMGAction::Media)current_action->get_sub()) {
		case MMGAction::Media::MEDIA_TIME:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Time");
			lcd_double1.set_range(
				0.0, get_obs_media_length(
					     current_action->get_str(0)));
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			break;
		case MMGAction::Media::MEDIA_SKIP_FORWARD_TIME:
		case MMGAction::Media::MEDIA_SKIP_BACKWARD_TIME:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Time Adj.");
			lcd_double1.set_range(
				0.0, get_obs_media_length(
					     current_action->get_str(0)));
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		switch ((MMGAction::Transitions)current_action->get_sub()) {
		case MMGAction::Transitions::TRANSITION_CURRENT:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Duration");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
			break;
		/*case MMGAction::Transitions::TRANSITION_TBAR:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Position (%)");
			lcd_double1.set_range(0.0, 100.0);
			lcd_double1.set_step(0.5, 5.0);
			set_lcd_value(ui->lcd_double1, LCDButtons::MIDIMGWINDOW_NEUTRAL_RESET);
			break;*/
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_lists_visible(true, true);
			ui->action_list2_label->setText("Scene");
			MMGAction::do_obs_scene_enum(ui->action_list2_editor);
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		set_lists_visible(true, true);
		ui->action_list2_label->setText("Filter");
		MMGAction::do_obs_filter_enum(
			ui->action_list2_editor,
			MMGAction::Category::MMGACTION_SOURCE_PROPS, value);
		break;
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		set_lists_visible(true, true);
		ui->action_list2_label->setText("Type");
		for (int i = 0; i < ui->message_type_editor->count(); ++i) {
			ui->action_list2_editor->addItem(
				ui->message_type_editor->itemText(i));
		}
		break;
	default:
		break;
	}
}

void MidiMGWindow::set_list2(const QString &value)
{
	current_action->set_str(1, value);

	if (value.isEmpty())
		return;

	QString str1;

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		switch ((MMGAction::SourceTransform)current_action->get_sub()) {
		case MMGAction::SourceTransform::TRANSFORM_POSITION:
			set_doubles_visible(true, true);
			ui->action_double1_label->setText("Pos X");
			ui->action_double2_label->setText("Pos Y");
			lcd_double1.set_range(0.0, get_obs_dimensions().first);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(0.0, get_obs_dimensions().second);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			break;
		case MMGAction::SourceTransform::TRANSFORM_DISPLAY:
			set_lists_visible(true, true, true);
			ui->action_list3_label->setText("Action");
			ui->action_list3_editor->addItems(
				{"Show", "Hide", "Toggle"});
			break;
		case MMGAction::SourceTransform::TRANSFORM_LOCKED:
			set_lists_visible(true, true, true);
			ui->action_list3_label->setText("Action");
			ui->action_list3_editor->addItems(
				{"Locked", "Unlocked", "Toggle"});
			break;
		case MMGAction::SourceTransform::TRANSFORM_CROP:
			set_doubles_visible(true, true, true, true);
			ui->action_double1_label->setText("Top");
			ui->action_double2_label->setText("Right");
			ui->action_double3_label->setText("Bottom");
			ui->action_double4_label->setText("Left");
			lcd_double1.set_range(
				0.0,
				get_obs_source_dimensions(
					ui->action_list1_editor->currentText())
						.second >>
					1);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(
				0.0,
				get_obs_source_dimensions(
					ui->action_list1_editor->currentText())
						.first >>
					1);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			lcd_double3.set_range(
				0.0,
				get_obs_source_dimensions(
					ui->action_list1_editor->currentText())
						.second >>
					1);
			lcd_double3.set_step(0.5, 5.0);
			lcd_double3.reset();
			lcd_double4.set_range(
				0.0,
				get_obs_source_dimensions(
					ui->action_list1_editor->currentText())
						.first >>
					1);
			lcd_double4.set_step(0.5, 5.0);
			lcd_double4.reset();
			break;
		case MMGAction::SourceTransform::TRANSFORM_SCALE:
			set_doubles_visible(true, true);
			ui->action_double1_label->setText("Scale X");
			ui->action_double2_label->setText("Scale Y");
			lcd_double1.set_range(0.2, 5.0);
			lcd_double1.set_step(0.05, 0.5);
			lcd_double1.reset(1.0);
			lcd_double2.set_range(0.2, 5.0);
			lcd_double2.set_step(0.05, 0.5);
			lcd_double2.reset(1.0);
			break;
		case MMGAction::SourceTransform::TRANSFORM_ROTATION:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Rotation (°)");
			lcd_double1.set_range(0.0, 360.0);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			break;
		case MMGAction::SourceTransform::TRANSFORM_BOUNDINGBOX:
			set_doubles_visible(true, true);
			ui->action_double1_label->setText("Size X");
			ui->action_double2_label->setText("Size Y");
			lcd_double1.set_range(
				0.0,
				get_obs_source_dimensions(
					ui->action_list1_editor->currentText())
					.first);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(
				0.0,
				get_obs_source_dimensions(
					ui->action_list1_editor->currentText())
					.second);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			break;
		case MMGAction::SourceTransform::TRANSFORM_RESET:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		if (current_action->get_sub() == 7) {
			current_action->set_num(
				0, ui->action_list2_editor->currentIndex());
		}
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		switch ((MMGAction::Transitions)current_action->get_sub()) {
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_lists_visible(true, true, true);
			ui->action_list3_label->setText("Source");
			MMGAction::do_obs_source_enum(
				ui->action_list3_editor,
				MMGAction::Category::MMGACTION_SCENE, value);
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		switch ((MMGAction::Filters)current_action->get_sub()) {
		case MMGAction::Filters::FILTER_REORDER:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Position");
			lcd_double1.set_range(
				1.0,
				get_obs_source_filter_count(
					ui->action_list1_editor->currentText()));
			lcd_double1.set_step(1.0, 5.0);
			lcd_double1.reset();
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		ui->action_double1_label->setText("Channel");
		lcd_double1.set_range(1.0, 16.0);
		lcd_double1.set_step(1.0, 5.0);
		lcd_double1.reset();
		lcd_double2.set_range(0.0, 127.0);
		lcd_double2.set_step(1.0, 10.0);
		lcd_double2.reset();
		lcd_double3.set_range(0.0, 127.0);
		lcd_double3.set_step(1.0, 10.0);
		lcd_double3.reset();
		str1 = ui->action_list2_editor->currentText();
		if (str1 == "Note On" || str1 == "Note Off") {
			set_doubles_visible(true, true, true);
			ui->action_double2_label->setText("Note #");
			ui->action_double3_label->setText("Velocity");
		} else if (str1 == "Control Change") {
			set_doubles_visible(true, true, true);
			ui->action_double2_label->setText("Control #");
			ui->action_double3_label->setText("Value");
		} else if (str1 == "Program Change") {
			set_doubles_visible(true, true);
			ui->action_double2_label->setText("Program #");
		} else if (str1 == "Pitch Bend") {
			set_doubles_visible(true, true);
			ui->action_double2_label->setText("Pitch Adj.");
		}
		break;
	default:
		break;
	}
}

void MidiMGWindow::set_list3(const QString &value)
{
	current_action->set_str(2, value);

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_TRANSITION:
		switch ((MMGAction::Transitions)current_action->get_sub()) {
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_doubles_visible(true);
			ui->action_double1_label->setText("Duration");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
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

void MidiMGWindow::on_add_click()
{
	switch (ui->structure_editor->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_BINDING:
		current_binding = current_device->add();
		add_widget_item(MMGModes::MMGMODE_BINDING,
				current_binding->get_name());
		break;

	case MMGModes::MMGMODE_MESSAGE:
		current_message = current_binding->add_message();
		add_widget_item(MMGModes::MMGMODE_MESSAGE,
				current_message->get_name());
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
	QListWidgetItem *current = ui->structure_editor->currentItem();
	if (!current)
		return;
	switch (ui->structure_editor->property("mode").value<MMGModes>()) {

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

	ui->structure_editor->removeItemWidget(current);
	delete current;
	ui->remove_button->setEnabled(false);
	on_list_selection_change(nullptr);
}

void MidiMGWindow::on_return_click()
{
	QListWidgetItem *current_item = nullptr;
	switch (ui->structure_editor->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_PREFERENCES:
	case MMGModes::MMGMODE_BINDING:
		switch_structure_pane(MMGModes::MMGMODE_DEVICE);
		current_item = ui->structure_editor->findItems(
			current_device->get_name(), Qt::MatchCaseSensitive)[0];
		current_item->setSelected(true);
		on_list_selection_change(current_item);
		break;

	case MMGModes::MMGMODE_MESSAGE:
	case MMGModes::MMGMODE_ACTION:
		switch_structure_pane(MMGModes::MMGMODE_BINDING);
		current_item = ui->structure_editor->findItems(
			current_binding->get_name(), Qt::MatchCaseSensitive)[0];
		current_item->setSelected(true);
		on_list_selection_change(current_item);
		break;

	default:
		return;
	}
}

void MidiMGWindow::on_binding_mode_select(enum MMGBinding::Mode mode)
{
	current_binding->set_mode(mode);
	ui->binding_mode_description->setText(
		tr(binding_mode_description(mode).qtocs()));
}

QString MidiMGWindow::binding_mode_description(enum MMGBinding::Mode mode) const
{
	switch (mode) {
	case MMGBinding::Mode::MMGBINDING_CONSECUTIVE:
		return "All actions will execute in order after the final message is received, and will receive that message as a parameter (if applicable).\n\nThis setting is the default.";
	case MMGBinding::Mode::MMGBINDING_CORRESPONDENCE:
		return "When multiple messages are positioned consecutively, all action(s) will receive their corresponding message as a parameter (if applicable).\n\nExample: There are three messages and three actions. When all three messages have been heard, the first message received will be sent as a parameter to the first action, the second message to the second action, and so on.";
	case MMGBinding::Mode::MMGBINDING_MULTIPLY:
		return "When multiple messages are positioned consecutively, all action(s) will each receive ALL of the messages as parameters (if applicable).\n\nExample: There are three messages before three actions. When all three messages have been heard, the first action receives all three messages as parameters (in order), then the second action receives all three messages, and so on.";
	default:
		return "Error: Invalid description. Report this as a bug from the Preferences page.";
	}
}

void MidiMGWindow::on_name_edit(QListWidgetItem *widget_item)
{
	QString str = widget_item->text();

	switch (ui->structure_editor->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_BINDING:
		if (!current_binding || current_binding->get_name() == str)
			break;
		if (!!current_device->find_binding(str)) {
			widget_item->setText(current_binding->get_name());
			break;
		}
		current_binding->set_name(str);
		ui->binding_name_text->setText(str);
		break;

	case MMGModes::MMGMODE_MESSAGE:
		if (!current_message || current_message->get_name() == str)
			break;
		if (!!current_binding->find_message(str)) {
			widget_item->setText(current_message->get_name());
			break;
		}
		current_message->set_name(str);
		ui->message_name_text->setText(str);
		break;

	case MMGModes::MMGMODE_ACTION:
		if (!current_action || current_action->get_name() == str)
			break;
		if (!!current_binding->find_action(str)) {
			widget_item->setText(current_action->get_name());
			break;
		}
		current_action->set_name(str);
		ui->action_name_text->setText(str);
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

	ui->add_button->setEnabled(true);
	ui->remove_button->setEnabled(true);
	ui->return_button->setEnabled(true);

	switch (ui->structure_editor->property("mode").value<MMGModes>()) {
	case MMGModes::MMGMODE_DEVICE:
		current_device = global()->find_device(current->text());
		ui->add_button->setEnabled(false);
		ui->remove_button->setEnabled(false);
		ui->return_button->setEnabled(false);
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
		ui->structure_editor->property("mode").value<MMGModes>(), start,
		row);
}

void MidiMGWindow::switch_structure_pane(enum MMGModes mode)
{
	ui->structure_editor->clear();
	ui->structure_editor->setProperty("mode", QVariant::fromValue(mode));
	ui->add_button->setEnabled(true);
	ui->remove_button->setEnabled(false);
	ui->return_button->setEnabled(true);
	switch (mode) {
	case MMGModes::MMGMODE_PREFERENCES:
		ui->add_button->setEnabled(false);
		ui->structure_label->setText(tr("Preferences"));
		ui->pages->setCurrentIndex(5);
		return;
	case MMGModes::MMGMODE_DEVICE:
		ui->add_button->setEnabled(false);
		ui->return_button->setEnabled(false);
		for (const QString &name : get_device_names()) {
			add_widget_item(mode, name);
		}
		ui->structure_label->setText(tr("Devices"));
		break;
	case MMGModes::MMGMODE_BINDING:
		for (const MMGBinding *const binding_el :
		     current_device->get_bindings()) {
			add_widget_item(mode, binding_el->get_name());
		}
		ui->structure_label->setText(tr("Bindings"));
		break;
	case MMGModes::MMGMODE_MESSAGE:
		for (const MMGMessage *const message_el :
		     current_binding->get_messages()) {
			add_widget_item(mode, message_el->get_name());
		}
		ui->structure_label->setText(tr("Messages"));
		break;
	case MMGModes::MMGMODE_ACTION:
		for (const MMGAction *const action_el :
		     current_binding->get_actions()) {
			add_widget_item(mode, action_el->get_name());
		}
		ui->structure_label->setText(tr("Actions"));
		break;
	default:
		ui->add_button->setEnabled(false);
		ui->return_button->setEnabled(false);
		ui->structure_label->setText(tr("Error"));
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
	ui->structure_editor->addItem(widget_item);
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
	current_device = global()->get_active_device();
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
