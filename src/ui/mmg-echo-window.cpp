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

#include "mmg-echo-window.h"
#include "../mmg-config.h"

#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

using namespace MMGUtils;

#define MANAGER_SWITCH(_enum, macro)       \
	switch (_enum) {                   \
		case 0:                    \
			macro(binding);    \
			break;             \
		case 1:                    \
			macro(device);     \
			break;             \
		case 2:                    \
			macro(message);    \
			break;             \
		case 3:                    \
			macro(action);     \
			break;             \
		case 4:                    \
			macro(preference); \
			break;             \
		default:                   \
			break;             \
	}

MMGEchoWindow::MMGEchoWindow(QWidget *parent) : QDialog(parent, Qt::Dialog), ui(new Ui::MMGEchoWindow)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->setupUi(this);
	ui->label_version->setText(OBS_MIDIMG_VERSION);

	type_display = new MMGStringDisplay(ui->message_frame);
	type_display->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
	type_display->move(10, 10);
	type_display->setDescription(mmgtr("Message.Type.Text"));
	type_display->setBounds(MMGMessage::acceptedTypes());
	type_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);

	channel_display = new MMGNumberDisplay(ui->message_frame);
	channel_display->move(10, 120);
	channel_display->setDescription(mmgtr("Message.Channel"));
	channel_display->setBounds(1, 16);
	channel_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_TOGGLE);

	note_display = new MMGNumberDisplay(ui->message_frame);
	note_display->move(10, 210);
	note_display->setDescription(mmgtr("Message.Note"));
	note_display->setBounds(0, 127);
	note_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_TOGGLE);

	value_display = new MMGNumberDisplay(ui->message_frame);
	value_display->move(10, 300);
	value_display->setDescription(mmgtr("Message.Velocity"));
	value_display->setBounds(0, 127);
	value_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_TOGGLE);
	value_display->lower();

	main_binding_display = new MMGBindingDisplay(ui->binding_frame, true);
	main_binding_display->setFixedSize(350, 320);

	devices_display = new MMGManagerDisplay<MMGDevice>(ui->device_page, manager(device));
	devices_display->setGeometry(10, 10, 341, 461);
	devices_display->setItemsEditable(false);

	bindings_display = new MMGManagerDisplay<MMGBinding>(ui->binding_page, manager(binding));
	bindings_display->setGeometry(10, 10, 341, 461);

	messages_display = new MMGManagerDisplay<MMGMessage>(ui->message_page, manager(message));
	messages_display->setGeometry(10, 10, 341, 461);

	actions_display = new MMGManagerDisplay<MMGAction>(ui->action_page, manager(action));
	actions_display->setGeometry(10, 10, 341, 461);

	preferences_display = new MMGManagerDisplay<MMGSettings>(ui->preferences_page, manager(setting));
	preferences_display->setGeometry(10, 10, 341, 461);
	preferences_display->setItemsEditable(false);

	QButtonGroup *pages_button_group = new QButtonGroup(this);
	pages_button_group->addButton(ui->button_binding_display, 0);
	pages_button_group->addButton(ui->button_device_display, 1);
	pages_button_group->addButton(ui->button_message_display, 2);
	pages_button_group->addButton(ui->button_action_display, 3);
	pages_button_group->addButton(ui->button_preferences, 4);
	connect(pages_button_group, &QButtonGroup::idClicked, this, &MMGEchoWindow::onScreenChange);

	ui->button_confirm->setVisible(false);
	ui->label_binding_selection_notice->setVisible(false);
	ui->scroll_binding_contents->setWidget(main_binding_display);
	ui->editor_binding_contents->setVisible(false);

	connect(midi(), &MMGMIDI::messageListened, this, &MMGEchoWindow::updateMessage);

	translate();
	connectUISignals();
}

void MMGEchoWindow::displayWindow()
{
	bindings_display->displayAll();
	devices_display->displayAll();
	messages_display->displayAll();
	actions_display->displayAll();
	preferences_display->displayAll();
	ui->button_binding_display->setChecked(true);
	onScreenChange(0);

	setVisible(true);
}

void MMGEchoWindow::reject()
{
	if (ui->button_listen_once->isChecked() || ui->button_listen_continuous->isChecked()) {
		ui->button_listen_once->setChecked(false);
		ui->button_listen_continuous->setChecked(false);
	}

	config()->save();

	QDialog::reject();
}

void MMGEchoWindow::connectUISignals()
{
	// Device Display Connections
	connect(devices_display, &MMGListWidget::selected, this, &MMGEchoWindow::deviceShow);
	connect(ui->button_in_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceInputActiveChange);
	connect(ui->button_thru, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceThruStateChange);
	connect(ui->editor_thru, &QComboBox::currentTextChanged, this, &MMGEchoWindow::onDeviceThruChange);
	connect(ui->button_out_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceOutputActiveChange);

	// Message Display Connections
	connect(messages_display, &MMGListWidget::selected, this, &MMGEchoWindow::messageShow);
	connect(type_display, &MMGStringDisplay::stringChanged, this, &MMGEchoWindow::onMessageTypeChange);
	connect(ui->button_listen_continuous, &QAbstractButton::toggled, this, &MMGEchoWindow::onListenContinuousClick);
	connect(ui->button_listen_once, &QAbstractButton::toggled, this, &MMGEchoWindow::onListenOnceClick);

	// Action Display Connections
	connect(actions_display, &MMGListWidget::selected, this, &MMGEchoWindow::actionShow);
	connect(ui->editor_cat, &QComboBox::currentIndexChanged, this, &MMGEchoWindow::onActionCategoryChange);
	connect(ui->button_action_switch, &QAbstractButton::toggled, this, &MMGEchoWindow::onActionSwitch);
	connect(ui->editor_sub, &QComboBox::currentIndexChanged, this, &MMGEchoWindow::onActionSubChange);

	// Binding Display Connections
	connect(bindings_display, &MMGListWidget::selected, this, &MMGEchoWindow::bindingShow);
	connect(bindings_display, &MMGListWidget::itemMoved, manager(binding), &MMGBindingManager::resetConnections);
	connect(ui->button_binding_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onBindingActiveChange);
	connect(ui->button_binding_switch, &QAbstractButton::toggled, this, &MMGEchoWindow::onBindingSwitch);
	connect(main_binding_display, &MMGBindingDisplay::edited, this, &MMGEchoWindow::onBindingFieldEdit);

	// Preferences Connections
	connect(preferences_display, &MMGListWidget::selected, this, &MMGEchoWindow::preferenceShow);

	connect(ui->button_export, &QPushButton::clicked, this, &MMGEchoWindow::exportBindings);
	connect(ui->button_import, &QPushButton::clicked, this, &MMGEchoWindow::importBindings);
	connect(ui->button_help_advanced, &QPushButton::clicked, this, &MMGEchoWindow::openHelp);
	connect(ui->button_bug_report, &QPushButton::clicked, this, &MMGEchoWindow::reportBug);
	connect(ui->button_update_check, &QPushButton::clicked, this, &MMGEchoWindow::checkForUpdates);

	// Upper Button Connections
	connect(ui->button_add, &QPushButton::clicked, this, &MMGEchoWindow::onAddClick);
	connect(ui->button_duplicate, &QPushButton::clicked, this, &MMGEchoWindow::onCopyClick);
	connect(ui->button_remove, &QPushButton::clicked, this, &MMGEchoWindow::onRemoveClick);
	connect(ui->button_confirm, &QPushButton::clicked, this, &MMGEchoWindow::onConfirmClick);
}

void MMGEchoWindow::translate()
{
	ui->button_add->setToolTip(mmgtr("UI.Buttons.Create"));
	ui->button_duplicate->setToolTip(mmgtr("UI.Buttons.Copy"));
	ui->button_remove->setToolTip(mmgtr("UI.Buttons.Delete"));

	ui->button_binding_display->setToolTip(mmgtr("UI.Buttons.Bindings"));
	ui->button_device_display->setToolTip(mmgtr("UI.Buttons.Devices"));
	ui->button_message_display->setToolTip(mmgtr("UI.Buttons.Messages"));
	ui->button_action_display->setToolTip(mmgtr("UI.Buttons.Actions"));
	ui->button_preferences->setToolTip(mmgtr("UI.Buttons.Preferences"));

	ui->label_binding_selection_notice->setText(mmgtr("UI.SelectionNotice"));

	ui->label_thru->setText(mmgtr("Device.Thru.Label"));

	ui->button_listen_once->setText(mmgtr("UI.Listen.Once"));
	ui->button_listen_continuous->setText(mmgtr("UI.Listen.Continuous"));

	ui->label_cat->setText(mmgtr("Actions.Category"));
	ui->label_sub->setText(mmgtr("Actions.Name"));
	ui->editor_cat->clear();
	ui->editor_cat->addItems(
		mmgtr_all("Actions.Titles", {"None", "Streaming", "Recording", "VirtualCamera", "ReplayBuffer",
					     "StudioMode", "Scenes", "VideoSources", "AudioSources", "MediaSources",
					     "Transitions", "Filters", "Hotkeys", "Profiles", "Collections", "MIDI"}));

	ui->label_device_edit->setText(mmgtr("UI.Buttons.Devices"));

	ui->button_export->setToolTip(mmgtr("UI.Buttons.Export"));
	ui->button_import->setToolTip(mmgtr("UI.Buttons.Import"));
	ui->button_help_advanced->setToolTip(mmgtr("UI.Buttons.Help"));
	ui->button_bug_report->setToolTip(mmgtr("UI.Buttons.BugReport"));
	ui->label_author->setText(QString(mmgtr("Preferences.Creator")).arg(mmgtr("Plugin.Author")));
	ui->button_update_check->setText(mmgtr("Preferences.Updates"));
}

void MMGEchoWindow::onAddClick()
{
#define ON_ADD(which) which##s_display->addNew()
	MANAGER_SWITCH(ui->pages->currentIndex(), ON_ADD);
#undef ON_ADD
}

void MMGEchoWindow::onCopyClick()
{
#define ON_COPY(which) which##s_display->copy()
	MANAGER_SWITCH(ui->pages->currentIndex(), ON_COPY);
#undef ON_COPY
}

void MMGEchoWindow::onRemoveClick()
{
	if (!open_message_box("PermanentRemove", false)) return;

#define ON_REMOVE(which) which##s_display->remove()
	MANAGER_SWITCH(ui->pages->currentIndex(), ON_REMOVE);
#undef ON_REMOVE
}

void MMGEchoWindow::onConfirmClick()
{
	binding_edit = false;
	ui->button_confirm->setVisible(false);
	ui->label_binding_selection_notice->setVisible(false);

	ui->button_add->setEnabled(true);
	ui->button_binding_display->setEnabled(true);
	ui->button_device_display->setEnabled(true);
	ui->button_message_display->setEnabled(true);
	ui->button_action_display->setEnabled(true);
	ui->button_preferences->setEnabled(true);

	switch (ui->pages->currentIndex()) {
		case 1:
			current_binding->setUsedDevices(devices_display->selectedValues());
			devices_display->setSelectionMode(QAbstractItemView::SingleSelection);
			devices_display->setFilterType(TYPE_NONE);
			devices_display->displayAll();
			break;

		case 2:
			current_binding->setUsedMessages(messages_display->selectedValues());
			messages_display->setSelectionMode(QAbstractItemView::SingleSelection);
			messages_display->setFilterType(TYPE_NONE);
			messages_display->setItemsEditable(true);
			break;

		case 3:
			current_binding->setUsedActions(actions_display->selectedValues());
			actions_display->setSelectionMode(QAbstractItemView::SingleSelection);
			actions_display->setFilterType(TYPE_NONE);
			actions_display->setItemsEditable(true);
			break;

		case 4:
			preferences_display->setFilterType(TYPE_NONE);
			preferences_display->displayAll();
			break;

		default:
			break;
	}

	if (!manager(binding)->list().contains(current_binding)) {
		// Action Request Binding (those are not registered in the manager)
		ui->button_action_display->click();
		current_binding = nullptr;
	} else {
		// Normal Binding Access
		ui->button_binding_display->click();
	}
}

void MMGEchoWindow::onScreenChange(int page)
{
	ui->pages->setCurrentIndex(page);
	ui->button_add->setEnabled(true);
	ui->button_duplicate->setEnabled(false);
	ui->button_remove->setEnabled(false);
	QString name;

#define ON_TYPE_SET(which) \
	which##Show();     \
	name = #which
	MANAGER_SWITCH(page, ON_TYPE_SET);
#undef ON_TYPE_SET

	if (binding_edit) return;
	name[0] = name[0].toUpper();
	name = QString("UI.Buttons.%1s").arg(name);
	ui->label_header->setText(mmgtr(name.qtocs()));
}

void MMGEchoWindow::deviceShow()
{
	ui->button_add->setEnabled(false);
	ui->button_duplicate->setEnabled(false);
	ui->button_remove->setEnabled(false);

	current_device = devices_display->currentValue();
	ui->device_frame->setVisible(!!current_device && !binding_edit);
	if (!current_device || binding_edit) return;

	MMGNoEdit no_edit_device(current_device);
	QSignalBlocker blocker_in_enable(ui->button_in_enable);
	QSignalBlocker blocker_thru(ui->button_thru);
	QSignalBlocker blocker_thru_val(ui->editor_thru);
	QSignalBlocker blocker_out_enable(ui->button_out_enable);

	ui->button_in_enable->setEnabled(current_device->isCapable(TYPE_INPUT));
	ui->button_in_enable->setChecked(current_device->isActive(TYPE_INPUT));
	onDeviceInputActiveChange(current_device->isActive(TYPE_INPUT));

	ui->editor_thru->clear();
	for (MMGDevice *device : *manager(device))
		if (device->isCapable(TYPE_OUTPUT)) ui->editor_thru->addItem(device->name());

	bool thru_enabled = current_device->isActive(TYPE_INPUT) && !current_device->thru().isEmpty();
	ui->button_thru->setChecked(thru_enabled);
	onDeviceThruStateChange(thru_enabled);
	ui->editor_thru->setCurrentText(current_device->thru());

	ui->button_out_enable->setEnabled(current_device->isCapable(TYPE_OUTPUT));
	ui->button_out_enable->setChecked(current_device->isActive(TYPE_OUTPUT));
	onDeviceOutputActiveChange(current_device->isActive(TYPE_OUTPUT));
}

void MMGEchoWindow::deviceBindingEdit()
{
	ui->label_header->setText(QString(mmgtr("UI.Select")).arg(mmgtr("Device.Name")));

	devices_display->setSelectionMode(QAbstractItemView::ExtendedSelection);
	devices_display->setFilterType(current_binding->type());
	devices_display->displayAll();

	devices_display->selectValues(current_binding->usedDevices());
}

void MMGEchoWindow::onDeviceInputActiveChange(bool toggled)
{
	current_device->setActive(TYPE_INPUT, toggled);
	toggled = current_device->isPortOpen(TYPE_INPUT);
	ui->button_in_enable->setChecked(toggled);
	ui->label_in_connect->setText(current_device->status(TYPE_INPUT));

	ui->button_thru->setVisible(toggled);
	ui->label_thru->setVisible(toggled);
	ui->editor_thru->setVisible(toggled && ui->button_thru->isChecked());
}

void MMGEchoWindow::onDeviceOutputActiveChange(bool toggled)
{
	current_device->setActive(TYPE_OUTPUT, toggled);
	toggled = current_device->isPortOpen(TYPE_OUTPUT);
	ui->button_out_enable->setChecked(toggled);
	ui->label_out_connect->setText(current_device->status(TYPE_OUTPUT));
}

void MMGEchoWindow::onDeviceThruStateChange(bool toggled)
{
	ui->button_thru->setIcon(QIcon(QString(":/icons/toggle-%1.svg").arg(toggled ? "on" : "off")));
	ui->editor_thru->setVisible(toggled);

	if (ui->editor_thru->signalsBlocked()) return;
	current_device->setThru(toggled ? ui->editor_thru->currentText() : "");
}

void MMGEchoWindow::onDeviceThruChange(const QString &device)
{
	current_device->setThru(device);
}

void MMGEchoWindow::messageShow()
{
	current_message = messages_display->currentValue();
	bool enable_frame = !!current_message && !binding_edit;
	ui->message_frame->setVisible(enable_frame);
	ui->button_duplicate->setEnabled(enable_frame);
	ui->button_remove->setEnabled(enable_frame);
	if (!enable_frame) return;

	MMGNoEdit no_edit_message(current_message);

	type_display->setStorage(&current_message->type());
	channel_display->setStorage(&current_message->channel());
	note_display->setStorage(&current_message->note());
	value_display->setStorage(&current_message->value());
	onMessageTypeChange();
}

void MMGEchoWindow::messageBindingEdit()
{
	ui->label_header->setText(QString(mmgtr("UI.Select")).arg(mmgtr("Message.Name")));

	messages_display->setSelectionMode(current_binding->type() == TYPE_OUTPUT ? QAbstractItemView::ExtendedSelection
										  : QAbstractItemView::SingleSelection);
	messages_display->setFilterType(current_binding->type());
	messages_display->displayAll();

	messages_display->selectValues(current_binding->usedMessages());
}

void MMGEchoWindow::onMessageTypeChange()
{
	set_message_labels(current_message->type(), note_display, value_display);
}

void MMGEchoWindow::onListenOnceClick(bool toggled)
{
	ui->button_listen_once->setText(mmgtr_two("UI.Listen", "Cancel", "Once", toggled));
	if (listening_mode == 2) {
		QSignalBlocker blocker_continuous(ui->button_listen_continuous);
		ui->button_listen_continuous->setChecked(false);
		ui->button_listen_continuous->setText(mmgtr("UI.Listen.Continuous"));
	}
	midi()->setListening(toggled);
	listening_mode = toggled;
	global_blog(LOG_DEBUG, toggled ? "Single listen activated." : "Listening deactivated.");
}

void MMGEchoWindow::onListenContinuousClick(bool toggled)
{
	ui->button_listen_continuous->setText(mmgtr_two("UI.Listen", "Cancel", "Continuous", toggled));
	if (listening_mode == 1) {
		QSignalBlocker blocker_once(ui->button_listen_once);
		ui->button_listen_once->setChecked(false);
		ui->button_listen_once->setText(mmgtr("UI.Listen.Once"));
	}
	midi()->setListening(toggled);
	listening_mode = toggled ? 2 : 0;
	global_blog(LOG_DEBUG, toggled ? "Continuous listen activated." : "Listening deactivated.");
}

void MMGEchoWindow::updateMessage(const MMGSharedMessage &incoming)
{
	if (!incoming) return;
	if (listening_mode < 1) return;
	// Check the validity of the message type (whether it is one of the five
	// supported types)
	if (MMGMessage::acceptedTypes().indexOf(incoming->type()) == -1) return;
	if (listening_mode == 1) {
		ui->button_listen_once->setText(mmgtr("UI.Listen.Once"));
		ui->button_listen_once->setChecked(false);
		midi()->setListening(false);
	}
	current_message = messages_display->currentValue();
	QString name = current_message->name();

	incoming->copy(current_message);
	current_message->setName(name);
	current_message->value().setMax(127);

	messageShow();
}

void MMGEchoWindow::actionShow()
{
	current_action = actions_display->currentValue();
	bool enable_frame = !!current_action && !binding_edit;
	ui->action_frame->setVisible(enable_frame);
	ui->button_duplicate->setEnabled(enable_frame);
	ui->button_remove->setEnabled(enable_frame);
	if (!enable_frame) return;

	MMGNoEdit no_edit_action(current_action);
	QSignalBlocker blocker_switch(ui->button_action_switch);
	QSignalBlocker blocker_cat(ui->editor_cat);
	QSignalBlocker blocker_sub(ui->editor_sub);

	ui->editor_cat->setCurrentIndex((int)current_action->category());

	ui->button_action_switch->setChecked((bool)current_action->type());
	onActionSwitch((bool)current_action->type());

	ui->editor_sub->setCurrentIndex(current_action->sub());
	onActionSubChange(current_action->sub());
}

void MMGEchoWindow::actionBindingEdit()
{
	ui->label_header->setText(QString(mmgtr("UI.Select")).arg(mmgtr("Actions.Name")));

	actions_display->setSelectionMode(current_binding->type() == TYPE_INPUT ? QAbstractItemView::ExtendedSelection
										: QAbstractItemView::SingleSelection);
	actions_display->setFilterType(current_binding->type());
	actions_display->displayAll();

	actions_display->selectValues(current_binding->usedActions());
}

void MMGEchoWindow::onActionCategoryChange(int index)
{
	QJsonObject json_obj;
	json_obj["category"] = index;
	manager(action)->changeActionCategory(current_action, json_obj);
	actions_display->updateData();

	onActionSubUpdate();
}

void MMGEchoWindow::onActionSwitch(bool toggled)
{
	if (!ui->button_action_switch->signalsBlocked()) {
		QSignalBlocker blocker_action_switch(ui->button_action_switch);
		ui->button_action_switch->setChecked(!toggled);
		if (!open_message_box("ActionSwitch", false)) return;
		ui->button_action_switch->setChecked(toggled);
	}

	current_action->setType((DeviceType)toggled);
	ui->button_action_switch->setIcon(QIcon(toggled ? ":/icons/switch-out.svg" : ":/icons/switch-in.svg"));

	onActionSubUpdate();
}

void MMGEchoWindow::onActionSubUpdate()
{
	ui->editor_sub->clear();
	current_action->setComboOptions(ui->editor_sub);
}

void MMGEchoWindow::onActionSubChange(int index)
{
	if (index < 0) return;

	current_action->setSub(index);

	if (!current_action->display()) {
		current_action->createDisplay(ui->action_display_editor);
		connect(current_action->display(), &MMGActionDisplay::editRequest, this,
			&MMGEchoWindow::onActionEditRequest);
		ui->action_display_editor->addWidget(current_action->display());
	}

	current_action->display()->setParent(ui->action_display_editor);
	current_action->setActionParams();
	ui->action_display_editor->setCurrentWidget(current_action->display());
}

void MMGEchoWindow::onActionEditRequest(MMGBinding *binding, int page)
{
	if (!sender() || !binding) return;

	current_binding = binding;
	onBindingFieldEdit(page);
}

void MMGEchoWindow::bindingShow()
{
	current_binding = bindings_display->currentValue();
	ui->binding_frame->setVisible(!!current_binding);
	ui->button_duplicate->setEnabled(!!current_binding);
	ui->button_remove->setEnabled(!!current_binding);
	if (!current_binding) return;

	QSignalBlocker blocker_binding_enable(ui->button_binding_enable);
	QSignalBlocker blocker_binding_switch(ui->button_binding_switch);
	main_binding_display->setStorage(current_binding);

	ui->button_binding_enable->setChecked(current_binding->enabled());
	onBindingActiveChange(current_binding->enabled());

	ui->button_binding_switch->setChecked((bool)current_binding->type());
	onBindingSwitch((bool)current_binding->type());
}

void MMGEchoWindow::onBindingActiveChange(bool toggled)
{
	current_binding->setEnabled(toggled);
	ui->label_binding_enable->setText(mmgtr_two("Plugin", "Enabled", "Disabled", toggled));
}

void MMGEchoWindow::onBindingSwitch(bool toggled)
{
	if (!ui->button_binding_switch->signalsBlocked()) {
		QSignalBlocker blocker_binding_switch(ui->button_binding_switch);
		ui->button_binding_switch->setChecked(!toggled);
		if (!open_message_box("BindingSwitch", false)) return;
	}

	ui->label_binding_switch->setText(mmgtr_two("Binding", "Output", "Input", toggled));
	ui->button_binding_switch->setIcon(QIcon(toggled ? ":/icons/switch-out.svg" : ":/icons/switch-in.svg"));

	if (ui->button_binding_switch->signalsBlocked()) return;
	current_binding->setType((DeviceType)toggled);
	bindings_display->updateData();
	bindingShow();
}

void MMGEchoWindow::onBindingFieldEdit(int page)
{
	if (page < 1) return;

	ui->button_confirm->setVisible(true);
	ui->label_binding_selection_notice->setVisible(true);

	ui->button_binding_display->setEnabled(false);
	ui->button_device_display->setEnabled(false);
	ui->button_message_display->setEnabled(false);
	ui->button_action_display->setEnabled(false);
	ui->button_preferences->setEnabled(false);

	binding_edit = true;

#define ON_BINDING_EDIT(which)                     \
	which##s_display->setItemsEditable(false); \
	which##BindingEdit()
	MANAGER_SWITCH(page, ON_BINDING_EDIT);
#undef ON_BINDING_EDIT

	onScreenChange(page);

	ui->button_add->setEnabled(false);
	ui->button_duplicate->setEnabled(false);
	ui->button_remove->setEnabled(false);
}

void MMGEchoWindow::preferenceShow()
{
	ui->button_add->setEnabled(false);

	MMGSettings *current_settings = binding_edit ? current_binding->settings()
						     : preferences_display->currentValue();
	ui->preferences_frame->setVisible(!!current_settings);
	if (!current_settings) return;

	MMGNoEdit no_edit_settings(manager(setting));

	if (!current_settings->display()) current_settings->createDisplay(this);
	ui->scroll_preferences->takeWidget();
	ui->scroll_preferences->setWidget(current_settings->display());
}

void MMGEchoWindow::preferenceBindingEdit()
{
	ui->label_binding_selection_notice->setVisible(false);

	ui->label_header->setText(mmgtr("Preferences.Binding.Header"));

	preferences_display->setFilterType(current_binding->type());
	preferences_display->displayAll();

	preferenceShow();
}

void MMGEchoWindow::exportBindings()
{
	QString filepath = QFileDialog::getSaveFileName(this, mmgtr("UI.Filesystem.ExportTitle"), MMGConfig::filepath(),
							mmgtr("UI.Filesystem.FileType"));
	if (!filepath.isNull()) config()->save(filepath);
}

void MMGEchoWindow::importBindings()
{
	QString filepath = QFileDialog::getOpenFileName(this, mmgtr("UI.Filesystem.ImportTitle"), "",
							mmgtr("UI.Filesystem.FileType"));
	if (filepath.isNull()) return;
	config()->load(filepath);
	displayWindow();
}

void MMGEchoWindow::openHelp() const
{
	QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/blob/master/docs/README.md"));
}

void MMGEchoWindow::reportBug() const
{
	QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/issues"));
}

void MMGEchoWindow::checkForUpdates() const
{
	QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/releases"));
}

MMGEchoWindow::~MMGEchoWindow()
{
	delete ui;
}
