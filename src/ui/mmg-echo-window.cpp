/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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
#include "../mmg-preference-defs.h"

#include <QButtonGroup>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>

using namespace MMGWidgets;

MMGParams<MMGMIDIPort *> MMGEchoWindow::thru_params {
	.desc = mmgtr("Device.Thru"),
	.options = OPTION_NONE,
	.default_value = nullptr,
	.bounds = {},
};

MMGParams<MMGBinding::ResetMode> MMGEchoWindow::reset_mode_params {
	.desc = mmgtr("Binding.Label.ResetMode"),
	.options = OPTION_NONE,
	.default_value = MMGBinding::BINDING_TRIGGERED,
	.bounds =
		{
			{MMGBinding::BINDING_TRIGGERED, mmgtr("Binding.Label.ResetMode.Triggered")},
			{MMGBinding::BINDING_CONTINUOUS, mmgtr("Binding.Label.ResetMode.Concurrent")},
		},
};

MMGEchoWindow::MMGEchoWindow(QWidget *parent) : QDialog(parent, Qt::Dialog), ui(new Ui::MMGEchoWindow)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->setupUi(this);

	menu_binding_groups = new QMenu(this);
	menu_binding_groups->setTitle(mmgtr("UI.Buttons.Move"));
	menu_binding_groups->setIcon(mmgicon("move"));

	thru_display = new MMGValueFixedDisplay<MMGMIDIPort *>(ui->editor_device, &thru_params);
	thru_display->setGeometry(0, 100, 330, 40);

	reset_mode_display = new MMGValueFixedDisplay<MMGBinding::ResetMode>(this, &reset_mode_params);
	reset_mode_display->setFixedHeight(40);
	ui->layout_binding_info->insertWidget(3, reset_mode_display);

	state_display = new MMGStateDisplay(ui->value_editor_frame);
	ui->layout_value_editor_frame->addWidget(state_display);

	message_object_display = new MMGMessageDisplay(ui->message_frame, state_display);
	ui->layout_message_frame->addWidget(message_object_display);

	action_object_display = new MMGActionDisplay(ui->action_frame, state_display);
	ui->layout_action_frame->addWidget(action_object_display);

	collection_display =
		new MMGManagerDisplay<MMGBindingManager>(ui->overview_frame, ui->editor_collection_buttons);
	collection_display->setStorage(manager(collection));

	device_display = new MMGManagerDisplay<MMGDevice>(ui->device_frame_main, ui->editor_device, false);
	device_display->setStorage(manager(device));

	preference_display = new MMGManagerDisplay<MMGPreference>(ui->preferences_frame, ui->editor_preferences, false);
	preference_display->setStorage(manager(preference));

	binding_display = new MMGManagerDisplay<MMGBinding>(ui->binding_frame);

	message_display = new MMGManagerDisplay<MMGMessage>(ui->binding_frame);

	action_display = new MMGManagerDisplay<MMGAction>(ui->binding_frame);

	translate();
	connectUISignals();
}

void MMGEchoWindow::displayWindow()
{
	ui->pages->setCurrentIndex(0);

	ui->editor_device->setVisible(false);

	collection_display->refresh();
	device_display->refresh();
	changeView(MODE_COLLECTION);

	setVisible(true);
}

void MMGEchoWindow::reject()
{
	message_object_display->resetListening();

	config()->save();

	QDialog::reject();
}

void MMGEchoWindow::connectUISignals()
{
	// Collection Display Connections
	connect(collection_display, &MMGManagerDisplayQWidget::currentValueChanged, this,
		&MMGEchoWindow::collectionShow);
	connect(ui->button_edit_collection, &QPushButton::clicked, this, &MMGEchoWindow::onConfirmClick);

	// Device Display Connections
	connect(config(), &MMGConfig::midiStateChanged, device_display, &MMGManagerDisplay<MMGDevice>::refresh);
	connect(device_display, &MMGManagerDisplayQWidget::currentValueChanged, this, &MMGEchoWindow::deviceShow);
	connect(ui->button_in_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceInputActiveChange);
	connect(ui->button_out_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceOutputActiveChange);
	connect(ui->button_device_refresh, &QPushButton::clicked, this, &MMGEchoWindow::onDeviceRefresh);
	connect(ui->button_remove_device, &QPushButton::clicked, this, &MMGEchoWindow::onDeviceRemove);
	connect(thru_display, &MMGValueQWidget::valueChanged, this, &MMGEchoWindow::onDeviceThruChange);

	// Multipurpose Display Connections
	connect(ui->button_message_edit, &QPushButton::clicked, this, &MMGEchoWindow::onMessageEditClick);
	connect(ui->button_action_edit, &QPushButton::clicked, this, &MMGEchoWindow::onActionEditClick);
	connect(ui->button_confirm, &QPushButton::clicked, this, &MMGEchoWindow::onConfirmClick);
	connect(state_display, &MMGStateDisplay::visibilityChanged, ui->label_value_editor_info, &QLabel::setHidden);

	// Binding Display Connections
	connect(binding_display, &MMGManagerDisplayQWidget::currentValueChanged, this, &MMGEchoWindow::bindingShow);
	connect(binding_display, &MMGManagerDisplayQWidget::itemRenamed, ui->label_binding_name, &QLabel::setText);
	connect(ui->button_move, &QPushButton::clicked, this, &MMGEchoWindow::onBindingMoveClick);
	connect(ui->button_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onBindingActiveChange);
	connect(ui->button_switch, &QAbstractButton::toggled, this, &MMGEchoWindow::onBindingSwitch);
	connect(reset_mode_display, &MMGValueQWidget::valueChanged, this, &MMGEchoWindow::onBindingResetChange);

	// Message Display Connections
	connect(message_display, &MMGManagerDisplayQWidget::currentValueChanged, this, &MMGEchoWindow::messageShow);
	connect(message_display, &MMGManagerDisplayQWidget::itemRenamed, ui->label_binding_name, &QLabel::setText);
	connect(message_object_display, &MMGMessageDisplay::messageChanged, this,
		&MMGEchoWindow::onBindingConditionChange);

	// Action Display Connections
	connect(action_display, &MMGManagerDisplayQWidget::currentValueChanged, this, &MMGEchoWindow::actionShow);
	connect(action_display, &MMGManagerDisplayQWidget::itemRenamed, ui->label_binding_name, &QLabel::setText);
	connect(action_object_display, &MMGActionDisplay::actionChanged, this,
		&MMGEchoWindow::onBindingConditionChange);

	// Preferences Connections
	connect(preference_display, &MMGManagerDisplayQWidget::currentValueChanged, this,
		&MMGEchoWindow::preferenceShow);
}

void MMGEchoWindow::translate()
{
	ui->button_edit_collection->setToolTip(mmgtr("UI.Buttons.Edit"));

	ui->label_overview_header->setText(mmgtr("UI.Buttons.Collections"));
	ui->label_device_header->setToolTip(mmgtr("UI.Buttons.Devices"));
	ui->label_preferences_header->setToolTip(mmgtr("UI.Buttons.Preferences"));

	ui->button_confirm->setToolTip(mmgtr("UI.Buttons.Confirm"));
	ui->button_device_refresh->setText(mmgtr("Device.Refresh"));
	ui->label_value_editor_info->setText(mmgtr("UI.Text.ValueEditorInfo"));

	ui->label_binding_enable->setText(mmgtr("Binding.Label.Status"));
	ui->label_binding_switch->setText(mmgtr("Binding.Label.Switch"));
	ui->label_binding_move->setText(mmgtr("Binding.Label.Move"));

	ui->label_value_editor_info->setContentsMargins(5, 5, 5, 5);
}

void MMGEchoWindow::changeView(View view)
{
	current_view = view;

	binding_display->hide();
	action_display->hide();
	message_display->hide();

	message_object_display->clearEditRequest();
	action_object_display->clearEditRequest();
	state_display->clearStorage();

	ui->message_frame->setProperty("disabled", int(view == MODE_BINDING));
	refreshStylesheet(ui->message_frame);
	ui->action_frame->setProperty("disabled", int(view == MODE_BINDING));
	refreshStylesheet(ui->action_frame);

	switch (view) {
		case MODE_COLLECTION:
			ui->pages->setCurrentIndex(0);
			setTitle();
			message_object_display->setStorage(TYPE_INPUT, nullptr, nullptr);
			action_object_display->setStorage(TYPE_INPUT, nullptr, nullptr);
			collection_display->refresh();
			device_display->refresh();
			return;

		case MODE_BINDING:
			ui->pages->setCurrentIndex(1);
			binding_display->show();
			setTitle(current_collection->objectName());
			ui->value_pages->setCurrentIndex(1);
			if (!!current_binding) binding_display->setCurrentValue(current_binding);
			bindingShow();
			return;

		case MODE_MESSAGE:
			ui->binding_editors_pages->setCurrentIndex(0);
			message_display->setEditable(current_binding->type() == TYPE_OUTPUT);
			message_display->show();
			message_display->setCurrentValue(current_binding->messages(0));
			messageShow();
			break;

		case MODE_ACTION:
			ui->binding_editors_pages->setCurrentIndex(1);
			action_display->setEditable(current_binding->type() == TYPE_INPUT);
			action_display->show();
			action_display->setCurrentValue(current_binding->actions(0));
			actionShow();
			break;
	}

	setTitle(current_binding->objectName());
	ui->value_pages->setCurrentIndex(0);
}

void MMGEchoWindow::setTitle(const QString &name)
{
	QString title = mmgtr("Plugin.TitleBar");
	setWindowTitle(name.isEmpty() ? title : QString("%1 - %2").arg(name).arg(title));
}

void MMGEchoWindow::refreshStylesheet(QWidget *widget)
{
	QString ss = widget->styleSheet();
	widget->setStyleSheet("");
	widget->setStyleSheet(ss);
}

void MMGEchoWindow::collectionShow()
{
	current_collection = collection_display->currentValue();

	if (!!current_collection) {
		QString size_str = "%1 %2";
		size_str = size_str.arg(current_collection->size());
		size_str = size_str.arg(mmgtr(choosetr("Binding", "Name", "Names", current_collection->size() < 2)));
		ui->label_collection_size->setText(size_str);

		binding_display->setStorage(current_collection);
	}

	menu_binding_groups->clear();
	for (MMGBindingManager *manager : *manager(collection)) {
		if (manager == current_collection) continue;

		QAction *new_action = new QAction(this);
		new_action->setText(manager->objectName());
		new_action->setData(QVariant::fromValue(manager));
		connect(new_action, &QAction::triggered, this, &MMGEchoWindow::onBindingMoveSelect);
		menu_binding_groups->addAction(new_action);
	}
}

void MMGEchoWindow::deviceShow()
{
	current_device = device_display->currentValue();
	ui->editor_device->setVisible(!!current_device);
	ui->button_remove_device->setVisible(!!current_device && !current_device->isCapable(TYPE_NONE));
	if (!current_device) return;

	current_device->setEditable(false);
	QSignalBlocker blocker_in_enable(ui->button_in_enable);
	QSignalBlocker blocker_out_enable(ui->button_out_enable);

	thru_params.bounds.clear();
	thru_params.bounds.insert(nullptr, mmgtr("Plugin.Disabled"));
	for (MMGDevice *device : *manager(device))
		if (device->isCapable(TYPE_OUTPUT))
			thru_params.bounds.insert(device, nontr(qUtf8Printable(device->objectName())));

	ui->button_in_enable->setEnabled(current_device->isCapable(TYPE_INPUT));
	ui->button_in_enable->setChecked(current_device->isActive(TYPE_INPUT));
	onDeviceInputActiveChange(current_device->isActive(TYPE_INPUT));

	ui->button_out_enable->setEnabled(current_device->isCapable(TYPE_OUTPUT));
	ui->button_out_enable->setChecked(current_device->isActive(TYPE_OUTPUT));
	onDeviceOutputActiveChange(current_device->isActive(TYPE_OUTPUT));

	current_device->setEditable(true);
}

void MMGEchoWindow::onDeviceInputActiveChange(bool toggled)
{
	current_device->setActive(TYPE_INPUT, toggled);
	toggled = current_device->isPortOpen(TYPE_INPUT);
	ui->button_in_enable->setChecked(toggled);
	ui->label_in_connect->setText(current_device->status(TYPE_INPUT));

	thru_params.options.setFlag(OPTION_DISABLED, !toggled);
	thru_display->setValue(current_device->isActive(TYPE_INPUT) ? current_device->thru() : nullptr);
	thru_display->refresh();
}

void MMGEchoWindow::onDeviceOutputActiveChange(bool toggled)
{
	current_device->setActive(TYPE_OUTPUT, toggled);
	toggled = current_device->isPortOpen(TYPE_OUTPUT);
	ui->button_out_enable->setChecked(toggled);
	ui->label_out_connect->setText(current_device->status(TYPE_OUTPUT));
}

void MMGEchoWindow::onDeviceThruChange()
{
	current_device->setThru(thru_display->value());
}

void MMGEchoWindow::onDeviceRefresh()
{
	resetMIDIAPI(libremidi_api(MMGPreferences::MMGPreferenceMIDI::currentAPI()));
}

void MMGEchoWindow::onDeviceRemove()
{
	if (!prompt_question("DeviceRemove")) return;

	manager(device)->remove(current_device);
	device_display->refresh();
	deviceShow();
}

void MMGEchoWindow::onMessageEditClick()
{
	changeView(MODE_MESSAGE);
}

void MMGEchoWindow::onActionEditClick()
{
	changeView(MODE_ACTION);
}

void MMGEchoWindow::onConfirmClick()
{
	switch (current_view) {
		case MODE_COLLECTION:
		case MODE_MESSAGE:
		case MODE_ACTION:
			changeView(MODE_BINDING);
			break;

		default:
			changeView(MODE_COLLECTION);
			break;
	}
}

void MMGEchoWindow::bindingShow()
{
	current_binding = binding_display->currentValue();

	ui->label_binding_name->hide();
	ui->value_pages->setVisible(!!current_binding);
	ui->binding_editors_pages->setVisible(!!current_binding);

	if (!current_binding) return;

	QSignalBlocker blocker_messages(message_display);
	QSignalBlocker blocker_actions(action_display);
	QSignalBlocker blocker_enable(ui->button_enable);
	QSignalBlocker blocker_switch(ui->button_switch);

	message_display->setStorage(current_binding->messages());
	message_display->setCurrentValue(current_binding->messages()->at(0));
	action_display->setStorage(current_binding->actions());
	action_display->setCurrentValue(current_binding->actions()->at(0));
	messageShow();
	actionShow();

	ui->binding_editors_pages->setProperty("disabled", true);
	ui->label_binding_name->show();
	ui->editor_binding_move->setVisible(!menu_binding_groups->isEmpty());

	ui->button_enable->setChecked(current_binding->enabled());
	onBindingActiveChange(current_binding->enabled());

	bool binding_type = current_binding->type() == TYPE_OUTPUT;
	ui->button_switch->setChecked(binding_type);
	ui->binding_editors_pages->setCurrentIndex(current_binding->type());
	ui->layout_binding_editor_buttons->setDirection(QBoxLayout::Direction(binding_type + 2));
	ui->label_message_button->setText(mmgtr(choosetr("Binding.Label.Message", "Output", "Input", binding_type)));
	ui->label_action_button->setText(mmgtr(choosetr("Binding.Label.Action", "Output", "Input", binding_type)));
	onBindingSwitch(binding_type);

	reset_mode_display->refresh();
	reset_mode_display->setValue(current_binding->resetMode());
}

void MMGEchoWindow::onBindingActiveChange(bool toggled)
{
	current_binding->setEnabled(toggled);
}

void MMGEchoWindow::onBindingSwitch(bool toggled)
{
	if (!ui->button_switch->signalsBlocked()) {
		QSignalBlocker blocker_switch(ui->button_switch);
		ui->button_switch->setChecked(!toggled);
		if (!prompt_question("BindingSwitch")) return;
		ui->button_switch->setChecked(toggled);
	}

	if (ui->button_switch->signalsBlocked()) return;
	current_binding->setType((DeviceType)toggled);
	bindingShow();
}

void MMGEchoWindow::onBindingResetChange()
{
	current_binding->setResetMode(reset_mode_display->value());
}

void MMGEchoWindow::onBindingMoveClick()
{
	if (menu_binding_groups->isEmpty()) return;
	menu_binding_groups->popup(QCursor::pos());
}

void MMGEchoWindow::onBindingMoveSelect()
{
	if (!sender()) return;

	auto action = qobject_cast<QAction *>(sender());
	if (!action) return;

	MMGBindingManager *manager = action->data().value<MMGBindingManager *>();
	MMGBinding *moved_binding = manager->add();
	current_binding->copy(moved_binding);
	current_collection->remove(current_binding);

	binding_display->refresh();
}

void MMGEchoWindow::onBindingConditionChange()
{
	current_binding->refresh();
}

void MMGEchoWindow::messageShow()
{
	current_message = message_display->currentValue();
	ui->message_frame->setVisible(!!current_message);
	ui->value_editor_frame->setVisible(!!current_message);
	ui->label_binding_name->setVisible(current_view == MODE_MESSAGE && !!current_message);
	if (!current_message) return;

	state_display->applyReferences(current_binding->type(), TYPE_OUTPUT);
	message_object_display->setStorage(current_binding->type(), current_binding->messages(), current_message);
	message_object_display->setModifiable(current_view == MODE_MESSAGE);
}

void MMGEchoWindow::actionShow()
{
	current_action = action_display->currentValue();
	ui->action_frame->setVisible(!!current_action);
	ui->value_editor_frame->setVisible(!!current_action);
	ui->label_binding_name->setVisible(current_view == MODE_ACTION && !!current_action);
	if (!current_action) return;

	state_display->applyReferences(current_binding->type(), TYPE_INPUT);
	action_object_display->setStorage(current_binding->type(), current_binding->actions(), current_action);
	action_object_display->setModifiable(current_view == MODE_ACTION);
}

void MMGEchoWindow::preferenceShow()
{
	current_preference = preference_display->currentValue();
	if (!current_preference) return;

	clearLayout(ui->editor_preferences->layout());
	current_preference->createDisplay(ui->editor_preferences);
}

MMGEchoWindow::~MMGEchoWindow()
{
	delete ui;
}
