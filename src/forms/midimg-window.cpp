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

#define SET_TOOLTIP(element, text) ui->element->setToolTip(tr(text))

#define SET_LCD_TOOLTIP(element, text)                         \
	SET_TOOLTIP(lcd_##element, text);                      \
	SET_TOOLTIP(label_##element, text);                    \
	SET_TOOLTIP(down_major_##element,                      \
		    QVariant(lcd_##element.get_major_step())   \
			    .toString()                        \
			    .prepend("Adjust this value by -") \
			    .qtocs());                         \
	SET_TOOLTIP(down_minor_##element,                      \
		    QVariant(lcd_##element.get_minor_step())   \
			    .toString()                        \
			    .prepend("Adjust this value by -") \
			    .qtocs());                         \
	SET_TOOLTIP(up_minor_##element,                        \
		    QVariant(lcd_##element.get_minor_step())   \
			    .toString()                        \
			    .prepend("Adjust this value by ")  \
			    .qtocs());                         \
	SET_TOOLTIP(up_major_##element,                        \
		    QVariant(lcd_##element.get_major_step())   \
			    .toString()                        \
			    .prepend("Adjust this value by ")  \
			    .qtocs())

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
}

bool MidiMGWindow::eventFilter(QObject *obj, QEvent *event)
{
	if (!obj->objectName().startsWith("lcd"))
		return QObject::eventFilter(obj, event);

	if (event->type() == QEvent::MouseButtonRelease) {
		QString name = obj->objectName();
		if (name == "lcd_note") {
			/*set_value(0 - ui->lcd_note->isEnabled());
			SET_LCD_STATUS(note, Disabled,
				       ui->lcd_note->isEnabled());
			lcd_note.display();*/
		} else if (name == "lcd_value") {
			set_value(0 - ui->lcd_value->isEnabled());
			SET_LCD_STATUS(value, Disabled,
				       ui->lcd_value->isEnabled());
			lcd_value.display();
		} else if (name == "lcd_double1") {
			set_double1(0 - ui->lcd_double1->isEnabled());
			SET_LCD_STATUS(double1, Disabled,
				       ui->lcd_double1->isEnabled());
			lcd_double1.display();
		} else if (name == "lcd_double2") {
			set_double2(0 - ui->lcd_double2->isEnabled());
			SET_LCD_STATUS(double2, Disabled,
				       ui->lcd_double2->isEnabled());
			lcd_double2.display();
		} else if (name == "lcd_double3") {
			set_double3(0 - ui->lcd_double3->isEnabled());
			SET_LCD_STATUS(double3, Disabled,
				       ui->lcd_double3->isEnabled());
			lcd_double3.display();
		} else if (name == "lcd_double4") {
			set_double4(0 - ui->lcd_double4->isEnabled());
			SET_LCD_STATUS(double4, Disabled,
				       ui->lcd_double4->isEnabled());
			lcd_double4.display();
		}
	}

	return QObject::eventFilter(obj, event);
}

void MidiMGWindow::show_window()
{
	setVisible(!isVisible());
	ui->editor_structure->clearSelection();
	current_device = global()->get_active_device();
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
	connect(ui->editor_binding_mode,
		QOverload<int>::of(&QComboBox::currentIndexChanged), this,
		[&](int id) {
			on_binding_mode_select((MMGBinding::Mode)(id + 1));
		});
	connect(ui->button_edit_messages, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_MESSAGE); });
	connect(ui->button_edit_actions, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_ACTION); });
	// Message Display Connections
	connect(ui->editor_type, &QComboBox::currentTextChanged, this,
		&MidiMGWindow::on_message_type_change);
	// Action Display Connections
	connect(ui->editor_cat, &QComboBox::currentTextChanged, this,
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
	// UI Movement Buttons
	connect(ui->button_add, &QPushButton::clicked, this,
		&MidiMGWindow::on_add_click);
	connect(ui->button_remove, &QPushButton::clicked, this,
		&MidiMGWindow::on_remove_click);
	connect(ui->button_return, &QPushButton::clicked, this,
		&MidiMGWindow::on_return_click);
	// Device Buttons
	connect(ui->button_edit_input_bindings, &QPushButton::clicked, this,
		[&]() { switch_structure_pane(MMGModes::MMGMODE_BINDING); });
	// connect(ui->transfer_bindings_button, &QPushButton::clicked, this, [&]() { on_button_click(UiButtons::MIDIMGWINDOW_TRANSFER_BINDINGS); });
	// Preferences Buttons
	connect(ui->button_preferences, &QPushButton::clicked, this, [&]() {
		switch_structure_pane(MMGModes::MMGMODE_PREFERENCES);
	});
	connect(ui->editor_global_enable, &QCheckBox::toggled, this,
		[&](bool toggled) { global()->set_running(toggled); });
	connect(ui->button_export, &QPushButton::clicked, this,
		&MidiMGWindow::export_bindings);
	connect(ui->button_import, &QPushButton::clicked, this,
		&MidiMGWindow::import_bindings);
	connect(ui->button_help, &QPushButton::clicked, this,
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

	SET_LCD_TOOLTIP(
		channel,
		"This is the channel that the binding will look for in an incoming message.\nThis value will always be required.");
	SET_LCD_TOOLTIP(
		note,
		"This is the note or control that the binding will look for in an incoming message.\nIf this is set to OFF, this value is not required.");
	SET_LCD_TOOLTIP(
		value,
		"This is the value that the binding will look for in an incoming message.\nIf this set to OFF, this value is not required.");
}

void MidiMGWindow::set_device_view()
{
	// QStringList list = get_device_names();

	ui->text_device_name->setText(current_device->get_name());
	ui->text_status_input->setText(current_device->input_device_status());
	ui->text_status_output->setText(current_device->output_device_status());
	// list.removeOne(current->text());
	// ui->transfer_bindings_name_editor->clear();
	// ui->transfer_bindings_name_editor->addItems(list);
	ui->button_edit_input_bindings->setEnabled(
		ui->text_status_input->text() == "Active");
	ui->button_edit_output_bindings->setEnabled(false);
}

void MidiMGWindow::set_binding_view()
{
	ui->text_binding_name->setText(current_binding->get_name());
	ui->editor_binding_mode->setCurrentIndex(
		(int)current_binding->get_mode() - 1);
	on_binding_mode_select(current_binding->get_mode());
}

void MidiMGWindow::set_message_view()
{
	// Because current_message is modified in these function calls (LCDData::reset),
	// this uses a const version of it to get its values
	const MMGMessage temp = *current_message;

	ui->text_message_name->setText(temp.get_name());

	lcd_channel.reset(temp.get_channel());
	lcd_note.reset(temp.get_note());
	lcd_value.reset(temp.get_value() < 0 ? 0.0 : temp.get_value());

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
	on_action_cat_change(
		ui->editor_cat->itemText((int)temp.get_category()));
	// Set subcategory
	ui->editor_sub->setCurrentIndex(temp.get_sub());
	on_action_sub_change(temp.get_sub());
	// Set strings (even if they are invalid)
	ui->editor_str1->setCurrentText(temp.get_str(0));
	ui->editor_str2->setCurrentText(temp.get_str(1));
	ui->editor_str3->setCurrentText(temp.get_str(2));
	// Set doubles (extra jargon for using the message value)
	SET_LCD_STATUS(double1, Disabled, temp.get_num(0) == -1);
	SET_LCD_STATUS(double2, Disabled, temp.get_num(1) == -1);
	SET_LCD_STATUS(double3, Disabled, temp.get_num(2) == -1);
	SET_LCD_STATUS(double4, Disabled, temp.get_num(3) == -1);
	lcd_double1.reset(temp.get_num(0) == -1 ? 0 : temp.get_num(0));
	lcd_double2.reset(temp.get_num(1) == -1 ? 0 : temp.get_num(1));
	lcd_double3.reset(temp.get_num(2) == -1 ? 0 : temp.get_num(2));
	lcd_double4.reset(temp.get_num(3) == -1 ? 0 : temp.get_num(3));
	// Re-set current_action to the correct one
	*current_action = temp;
}

void MidiMGWindow::set_sub_visible(bool visible) const
{
	ui->label_sub->setVisible(visible);
	ui->editor_sub->setVisible(visible);
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

void MidiMGWindow::on_message_type_change(const QString &type)
{
	current_message->set_type(type);
	ui->editor_type->setCurrentText(type);

	SET_LCD_STATUS(note, Enabled, current_message->get_note() >= 0);
	SET_LCD_STATUS(value, Visible, true);
	SET_LCD_STATUS(value, Enabled, true);

	if (type == "Note On" || type == "Note Off") {
		ui->label_note->setText("Note #");
		ui->label_value->setText("Velocity");
		SET_LCD_STATUS(value, Enabled,
			       current_message->get_value() >= 0);
		lcd_value.display();
	} else if (type == "Control Change") {
		ui->label_note->setText("Control #");
		ui->label_value->setText("Value");
		SET_LCD_STATUS(value, Enabled,
			       current_message->get_value() >= 0);
		lcd_value.display();
	} else if (type == "Program Change") {
		ui->label_note->setText("Program #");
		SET_LCD_STATUS(value, Visible, false);
	} else if (type == "Pitch Bend") {
		ui->label_note->setText("Pitch Adj.");
		SET_LCD_STATUS(value, Visible, false);
	}
}

void MidiMGWindow::on_action_cat_change(const QString &cat)
{
	current_action->set_category(MMGAction::categoryFromString(cat));

	set_sub_visible();
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

void MidiMGWindow::set_sub_options(std::initializer_list<QString> list) const
{
	set_sub_visible(true);
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

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
	case MMGAction::Category::MMGACTION_STREAM:
	case MMGAction::Category::MMGACTION_RECORD:
	case MMGAction::Category::MMGACTION_VIRCAM:
		break;
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		if (index == 3) {
			set_strs_visible(true);
			ui->label_str1->setText("Scene");
			MMGAction::do_obs_scene_enum(ui->editor_str1);
			ui->editor_str1->addItem("Use Message Value");
		}
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		set_strs_visible(true);
		ui->label_str1->setText("Scene");
		MMGAction::do_obs_scene_enum(ui->editor_str1);
		ui->editor_str1->addItem("Use Message Value");
		break;
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		set_strs_visible(true);
		ui->label_str1->setText("Scene");
		MMGAction::do_obs_scene_enum(ui->editor_str1);
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		set_strs_visible(true);
		ui->label_str1->setText("Source");
		MMGAction::do_obs_source_enum(
			ui->editor_str1,
			MMGAction::Category::MMGACTION_SOURCE_PROPS,
			QString::number(index));
		break;
	case MMGAction::Category::MMGACTION_MEDIA:
		set_strs_visible(true);
		ui->label_str1->setText("Source");
		MMGAction::do_obs_media_enum(ui->editor_str1);
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		set_strs_visible(true);
		ui->label_str1->setText("Transition");
		MMGAction::do_obs_transition_enum(ui->editor_str1);
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		set_strs_visible(true);
		ui->label_str1->setText("Source");
		MMGAction::do_obs_source_enum(ui->editor_str1);
		break;
	case MMGAction::Category::MMGACTION_HOTKEY:
		set_strs_visible(true);
		ui->label_str1->setText("Hotkey");
		MMGAction::do_obs_hotkey_enum(ui->editor_str1);
		break;
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		set_strs_visible(true);
		ui->label_str1->setText("Device");
		ui->editor_str1->addItems(MMGDevice::get_output_device_names());
		break;
	case MMGAction::Category::MMGACTION_WAIT:
		set_doubles_visible(true);
		ui->label_double1->setText("Time");
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
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		set_strs_visible(true, true);
		ui->label_str2->setText("Source");
		MMGAction::do_obs_source_enum(
			ui->editor_str2, MMGAction::Category::MMGACTION_SCENE,
			value);
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		switch ((MMGAction::SourceProperties)current_action->get_sub()) {
		case MMGAction::SourceProperties::PROPERTY_VOLUME_CHANGETO:
			set_doubles_visible(true);
			ui->label_double1->setText("Volume");
			lcd_double1.set_range(0.0, 100.0);
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			break;
		case MMGAction::SourceProperties::PROPERTY_VOLUME_CHANGEBY:
			set_doubles_visible(true);
			ui->label_double1->setText("Volume Adj.");
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
			ui->label_double1->setText("Sync Offset");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
			break;
		case MMGAction::SourceProperties::PROPERTY_AUDIO_MONITOR:
			set_strs_visible(true, true);
			ui->label_str2->setText("Monitor");
			ui->editor_str2->addItems(
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
			ui->label_double1->setText("Time");
			lcd_double1.set_range(
				0.0, get_obs_media_length(
					     current_action->get_str(0)));
			lcd_double1.set_step(1.0, 10.0);
			lcd_double1.reset();
			break;
		case MMGAction::Media::MEDIA_SKIP_FORWARD_TIME:
		case MMGAction::Media::MEDIA_SKIP_BACKWARD_TIME:
			set_doubles_visible(true);
			ui->label_double1->setText("Time Adj.");
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
			ui->label_double1->setText("Duration");
			lcd_double1.set_range(0.0, 20000.0);
			lcd_double1.set_step(25.0, 250.0);
			lcd_double1.reset();
			break;
		/*case MMGAction::Transitions::TRANSITION_TBAR:
			set_doubles_visible(true);
			ui->label_double1->setText("Position (%)");
			lcd_double1.set_range(0.0, 100.0);
			lcd_double1.set_step(0.5, 5.0);
			set_lcd_value(ui->lcd_double1, LCDButtons::MIDIMGWINDOW_NEUTRAL_RESET);
			break;*/
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_strs_visible(true, true);
			ui->label_str2->setText("Scene");
			MMGAction::do_obs_scene_enum(ui->editor_str2);
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
			MMGAction::Category::MMGACTION_SOURCE_PROPS, value);
		break;
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		set_strs_visible(true, true);
		ui->label_str2->setText("Type");
		for (int i = 0; i < ui->editor_type->count(); ++i) {
			ui->editor_str2->addItem(ui->editor_type->itemText(i));
		}
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

	QString str1;

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		switch ((MMGAction::SourceTransform)current_action->get_sub()) {
		case MMGAction::SourceTransform::TRANSFORM_POSITION:
			set_doubles_visible(true, true);
			ui->label_double1->setText("Pos X");
			ui->label_double2->setText("Pos Y");
			lcd_double1.set_range(0.0, get_obs_dimensions().first);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(0.0, get_obs_dimensions().second);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			break;
		case MMGAction::SourceTransform::TRANSFORM_DISPLAY:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Action");
			ui->editor_str3->addItems({"Show", "Hide", "Toggle"});
			break;
		case MMGAction::SourceTransform::TRANSFORM_LOCKED:
			set_strs_visible(true, true, true);
			ui->label_str3->setText("Action");
			ui->editor_str3->addItems(
				{"Locked", "Unlocked", "Toggle"});
			break;
		case MMGAction::SourceTransform::TRANSFORM_CROP:
			set_doubles_visible(true, true, true, true);
			ui->label_double1->setText("Top");
			ui->label_double2->setText("Right");
			ui->label_double3->setText("Bottom");
			ui->label_double4->setText("Left");
			lcd_double1.set_range(
				0.0, get_obs_source_dimensions(
					     ui->editor_str1->currentText())
						     .second >>
					     1);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			lcd_double2.set_range(
				0.0, get_obs_source_dimensions(
					     ui->editor_str1->currentText())
						     .first >>
					     1);
			lcd_double2.set_step(0.5, 5.0);
			lcd_double2.reset();
			lcd_double3.set_range(
				0.0, get_obs_source_dimensions(
					     ui->editor_str1->currentText())
						     .second >>
					     1);
			lcd_double3.set_step(0.5, 5.0);
			lcd_double3.reset();
			lcd_double4.set_range(
				0.0, get_obs_source_dimensions(
					     ui->editor_str1->currentText())
						     .first >>
					     1);
			lcd_double4.set_step(0.5, 5.0);
			lcd_double4.reset();
			break;
		case MMGAction::SourceTransform::TRANSFORM_SCALE:
			set_doubles_visible(true, true);
			ui->label_double1->setText("Scale X");
			ui->label_double2->setText("Scale Y");
			lcd_double1.set_range(0.2, 5.0);
			lcd_double1.set_step(0.05, 0.5);
			lcd_double1.reset(1.0);
			lcd_double2.set_range(0.2, 5.0);
			lcd_double2.set_step(0.05, 0.5);
			lcd_double2.reset(1.0);
			break;
		case MMGAction::SourceTransform::TRANSFORM_ROTATION:
			set_doubles_visible(true);
			ui->label_double1->setText("Rotation (°)");
			lcd_double1.set_range(0.0, 360.0);
			lcd_double1.set_step(0.5, 5.0);
			lcd_double1.reset();
			break;
		case MMGAction::SourceTransform::TRANSFORM_BOUNDINGBOX:
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
			break;
		case MMGAction::SourceTransform::TRANSFORM_RESET:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		if (current_action->get_sub() == 7) {
			current_action->set_num(
				0, ui->editor_str2->currentIndex());
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
			lcd_double1.reset();
			break;
		default:
			break;
		}
		break;
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		ui->label_double1->setText("Channel");
		lcd_double1.set_range(1.0, 16.0);
		lcd_double1.set_step(1.0, 5.0);
		lcd_double1.reset();
		lcd_double2.set_range(0.0, 127.0);
		lcd_double2.set_step(1.0, 10.0);
		lcd_double2.reset();
		lcd_double3.set_range(0.0, 127.0);
		lcd_double3.set_step(1.0, 10.0);
		lcd_double3.reset();
		str1 = ui->editor_str2->currentText();
		if (str1 == "Note On" || str1 == "Note Off") {
			set_doubles_visible(true, true, true);
			ui->label_double2->setText("Note #");
			ui->label_double3->setText("Velocity");
		} else if (str1 == "Control Change") {
			set_doubles_visible(true, true, true);
			ui->label_double2->setText("Control #");
			ui->label_double3->setText("Value");
		} else if (str1 == "Program Change") {
			set_doubles_visible(true, true);
			ui->label_double2->setText("Program #");
		} else if (str1 == "Pitch Bend") {
			set_doubles_visible(true, true);
			ui->label_double2->setText("Pitch Adj.");
		}
		break;
	default:
		break;
	}
}

void MidiMGWindow::set_str3(const QString &value)
{
	current_action->set_str(2, value);

	switch (current_action->get_category()) {
	case MMGAction::Category::MMGACTION_TRANSITION:
		switch ((MMGAction::Transitions)current_action->get_sub()) {
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
			set_doubles_visible(true);
			ui->label_double1->setText("Duration");
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
	switch (ui->editor_structure->property("mode").value<MMGModes>()) {
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

void MidiMGWindow::on_binding_mode_select(enum MMGBinding::Mode mode)
{
	current_binding->set_mode(mode);
	ui->text_binding_mode->setText(
		tr(binding_mode_description(mode).qtocs()));
}

QString MidiMGWindow::binding_mode_description(enum MMGBinding::Mode mode) const
{
	switch (mode) {
	case MMGBinding::Mode::MMGBINDING_CONSECUTIVE:
		return "All actions will execute in order after the final message is received, and will receive that message as a parameter (if applicable).\n\nThis setting is the default.";
	case MMGBinding::Mode::MMGBINDING_CORRESPONDENCE:
		return "When multiple messages are used, all action(s) will receive their corresponding message as a parameter (if applicable).\n\nExample: There are three messages and three actions. When all three messages have been heard, the first message received will be sent as a parameter to the first action, the second message to the second action, and so on.";
	case MMGBinding::Mode::MMGBINDING_MULTIPLY:
		return "When multiple messages are used, all action(s) will each receive ALL of the messages as parameters (if applicable). Do not use this mode unless it is absolutely necessary!\n\nExample: There are three messages before three actions. When all three messages have been heard, the first action receives all three messages as parameters (in order), then the second action receives all three messages, and so on.";
	default:
		return "Error: Invalid description. Report this as a bug from the Preferences page.";
	}
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
	ui->button_add->setEnabled(true);
	ui->button_remove->setEnabled(false);
	ui->button_return->setEnabled(true);
	switch (mode) {
	case MMGModes::MMGMODE_PREFERENCES:
		ui->button_add->setEnabled(false);
		ui->label_structure->setText(tr("Preferences"));
		ui->pages->setCurrentIndex(5);
		return;
	case MMGModes::MMGMODE_DEVICE:
		ui->button_add->setEnabled(false);
		ui->button_return->setEnabled(false);
		for (const QString &name : global()->get_device_names()) {
			add_widget_item(mode, name);
		}
		ui->label_structure->setText(tr("Devices"));
		break;
	case MMGModes::MMGMODE_BINDING:
		for (const MMGBinding *const binding_el :
		     current_device->get_bindings()) {
			add_widget_item(mode, binding_el->get_name());
		}
		ui->label_structure->setText(tr("Bindings"));
		break;
	case MMGModes::MMGMODE_MESSAGE:
		for (const MMGMessage *const message_el :
		     current_binding->get_messages()) {
			add_widget_item(mode, message_el->get_name());
		}
		ui->label_structure->setText(tr("Messages"));
		break;
	case MMGModes::MMGMODE_ACTION:
		for (const MMGAction *const action_el :
		     current_binding->get_actions()) {
			add_widget_item(mode, action_el->get_name());
		}
		ui->label_structure->setText(tr("Actions"));
		break;
	default:
		ui->button_add->setEnabled(false);
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

#undef SET_LCD_STATUS
#undef CONNECT_LCD
#undef INIT_LCD
#undef SET_TOOLTIP
#undef SET_LCD_TOOLTIP
