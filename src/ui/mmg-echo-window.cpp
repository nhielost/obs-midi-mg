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
#include "mmg-manager-control.h"

#include <QButtonGroup>
#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopServices>

using namespace MMGUtils;

MMGEchoWindow::MMGEchoWindow(QWidget *parent) : QDialog(parent, Qt::Dialog), ui(new Ui::MMGEchoWindow)
{
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	ui->setupUi(this);
	ui->label_version->setText(OBS_MIDIMG_VERSION);

	menu_binding_groups = new QMenu(this);
	menu_binding_groups->setTitle(mmgtr("UI.Buttons.Move"));
	menu_binding_groups->setIcon(MMGInterface::icon("move"));

	message_display = new MMGMessageDisplay(ui->message_frame);
	message_display->move(10, 10);

	collection_control = new MMGManagerControl<MMGBindingManager>(ui->structure_collections);
	collection_control->setStorage(manager(collection));
	collection_control->connectButtons(ui->button_add_collection, ui->button_remove_collection);

	device_control = new MMGManagerControl<MMGDevice>(ui->structure_devices);
	device_control->setStorage(manager(device));

	binding_control = new MMGManagerControl<MMGBinding>(ui->structure_multi);
	binding_control->connectButtons(ui->button_add, ui->button_remove, ui->button_duplicate);

	message_control = new MMGManagerControl<MMGMessage>(ui->structure_multi);
	message_control->connectButtons(ui->button_add, ui->button_remove, ui->button_duplicate);

	action_control = new MMGManagerControl<MMGAction>(ui->structure_multi);
	action_control->connectButtons(ui->button_add, ui->button_remove, ui->button_duplicate);

	translate();
	connectUISignals();
}

void MMGEchoWindow::displayWindow()
{
	ui->pages->setCurrentIndex(0);

	ui->editor_device->setVisible(false);

	device_control->refresh();
	collection_control->refresh();
	changeView(MODE_COLLECTION);

	setVisible(true);
}

void MMGEchoWindow::reject()
{
	for (MMGDevice *device : *manager(device))
		emit device->messageListened(nullptr);

	config()->save();

	QDialog::reject();
}

void MMGEchoWindow::connectUISignals()
{
	// Collection Display Connections
	connect(ui->structure_collections, &QListWidget::itemSelectionChanged, this, &MMGEchoWindow::collectionShow);
	connect(ui->button_edit_collection, &QPushButton::clicked, this, &MMGEchoWindow::onConfirmClick);

	// Device Display Connections
	connect(midi(), &MMGMIDI::deviceCapableChange, device_control, &MMGManagerControl<MMGDevice>::refresh);
	connect(ui->structure_devices, &QListWidget::itemSelectionChanged, this, &MMGEchoWindow::deviceShow);
	connect(ui->button_in_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceInputActiveChange);
	connect(ui->button_thru, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceThruStateChange);
	connect(ui->editor_thru, &QComboBox::currentTextChanged, this, &MMGEchoWindow::onDeviceThruChange);
	connect(ui->button_out_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onDeviceOutputActiveChange);
	connect(ui->button_device_check, &QPushButton::clicked, this, &MMGEchoWindow::onDeviceCheck);
	connect(ui->button_remove_device, &QPushButton::clicked, this, &MMGEchoWindow::onDeviceRemove);

	// Multipurpose Display Connections
	connect(ui->structure_multi, &QListWidget::itemSelectionChanged, this, &MMGEchoWindow::multiShow);
	connect(ui->structure_multi, &QListWidget::itemChanged, this, &MMGEchoWindow::onMultiRename);
	connect(ui->editor_multi_select, &QComboBox::currentIndexChanged, this, &MMGEchoWindow::onMultiReset);
	connect(ui->button_multi_edit, &QPushButton::clicked, this, &MMGEchoWindow::onMultiClick);
	connect(ui->button_confirm, &QPushButton::clicked, this, &MMGEchoWindow::onConfirmClick);

	// Binding Display Connections
	connect(ui->button_move, &QPushButton::clicked, this, &MMGEchoWindow::onBindingMoveClick);
	connect(ui->button_enable, &QAbstractButton::toggled, this, &MMGEchoWindow::onBindingActiveChange);
	connect(ui->button_switch, &QAbstractButton::toggled, this, &MMGEchoWindow::onBindingSwitch);
	connect(ui->editor_binding_reset, &QComboBox::currentIndexChanged, this, &MMGEchoWindow::onBindingResetChange);

	// Action Display Connections
	connect(ui->editor_cat, &QComboBox::currentIndexChanged, this, &MMGEchoWindow::onActionCategoryChange);
	connect(ui->editor_sub, &QComboBox::currentIndexChanged, this, &MMGEchoWindow::onActionSubChange);

	// Preferences Connections
	connect(ui->button_export, &QPushButton::clicked, this, &MMGEchoWindow::exportBindings);
	connect(ui->button_import, &QPushButton::clicked, this, &MMGEchoWindow::importBindings);
	connect(ui->button_help_advanced, &QPushButton::clicked, this, &MMGEchoWindow::openHelp);
	connect(ui->button_bug_report, &QPushButton::clicked, this, &MMGEchoWindow::reportBug);
	connect(ui->button_update_check, &QPushButton::clicked, this, &MMGEchoWindow::checkForUpdates);
}

void MMGEchoWindow::translate()
{
	ui->button_edit_collection->setToolTip(mmgtr("UI.Buttons.Edit"));

	ui->label_overview_header->setText(mmgtr("UI.Buttons.Collections"));
	ui->label_device_header->setToolTip(mmgtr("UI.Buttons.Devices"));
	ui->label_preferences_header->setToolTip(mmgtr("UI.Buttons.Preferences"));

	ui->button_confirm->setToolTip(mmgtr("UI.Buttons.Confirm"));
	ui->button_add->setToolTip(mmgtr("UI.Buttons.New"));
	ui->button_duplicate->setToolTip(mmgtr("UI.Buttons.Copy"));
	ui->button_remove->setToolTip(mmgtr("UI.Buttons.Delete"));

	ui->button_enable->setToolTip(mmgtr("UI.Buttons.Enable"));
	ui->button_switch->setToolTip(mmgtr("UI.Buttons.Switch"));
	ui->button_move->setToolTip(mmgtr("UI.Buttons.Move"));
	ui->label_binding_reset->setText(mmgtr("Binding.Reset"));
	ui->editor_binding_reset->addItems(MMGText::batch(TEXT_MMG, "Binding.Reset", {"Triggered", "Continuous"}));

	ui->label_thru->setText(mmgtr("Device.Thru.Label"));
	ui->button_device_check->setText(mmgtr("Device.Check.Label"));

	ui->label_cat->setText(mmgtr("Actions.Category"));
	ui->label_sub->setText(mmgtr("Actions.Name"));
	ui->editor_cat->clear();
	ui->editor_cat->addItems(
		MMGText::batch(TEXT_MMG, "Actions.Titles",
			       {"None", "Streaming", "Recording", "VirtualCamera", "ReplayBuffer", "StudioMode",
				"Scenes", "VideoSources", "AudioSources", "MediaSources", "Transitions", "Filters",
				"Hotkeys", "Profiles", "Collections", "MIDI"}));

	ui->button_export->setToolTip(mmgtr("UI.Buttons.Export"));
	ui->button_import->setToolTip(mmgtr("UI.Buttons.Import"));
	ui->button_help_advanced->setToolTip(mmgtr("UI.Buttons.Help"));
	ui->button_bug_report->setToolTip(mmgtr("UI.Buttons.BugReport"));
	ui->label_author->setText(QString(mmgtr("Preferences.Creator")).arg(mmgtr("Plugin.Author")));
	ui->button_update_check->setText(mmgtr("Preferences.Updates"));
}

void MMGEchoWindow::changeView(MultiMode mode)
{
	ui->structure_multi->setProperty("kind", mode);

	MMGBinding *binding = current_binding;
	current_binding = nullptr;
	onMultiReset();
	current_binding = binding;

	ui->editor_secondary_buttons->setVisible(false);
	ui->label_binding_name->setVisible(false);

	switch (mode) {
		case MODE_COLLECTION:
		case MODE_DEVICE:
			ui->pages->setCurrentIndex(0);
			setWindowTitle(mmgtr("Plugin.TitleBar"));
			ui->structure_collections->setCurrentItem(nullptr);
			ui->structure_devices->setCurrentItem(nullptr);
			collectionShow();
			deviceShow();
			return;

		case MODE_BINDING:
			ui->pages->setCurrentIndex(1);
			setWindowTitle(
				QString("%1 - %2").arg(current_collection->objectName()).arg(mmgtr("Plugin.TitleBar")));
			binding_control->setStorage(current_collection);
			break;

		case MODE_MESSAGE:
			setWindowTitle(
				QString("%1 - %2").arg(current_binding->objectName()).arg(mmgtr("Plugin.TitleBar")));
			message_control->setStorage(current_binding->messages());
			break;

		case MODE_ACTION:
			setWindowTitle(
				QString("%1 - %2").arg(current_binding->objectName()).arg(mmgtr("Plugin.TitleBar")));
			action_control->setStorage(current_binding->actions());
			break;
	}

	multiShow();
}

int MMGEchoWindow::multiIndex(DeviceType type) const
{
	if (ui->editor_multi_select->signalsBlocked()) return 0;
	if (current_binding->type() == type) return 0;
	return ui->editor_multi_select->currentIndex();
}

void MMGEchoWindow::collectionShow()
{
	current_collection = collection_control->currentValue();

	ui->editor_secondary_collection_buttons->setVisible(!!current_collection);
	if (!!current_collection) {
		QString size_str = " ";
		qsizetype size = current_collection->size();
		size_str.prepend(QString::number(size));
		size_str.append(MMGText::choose("Binding", "Name", "Names", size < 2));
		ui->label_collection_size->setText(size_str);
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
	current_device = device_control->currentValue();
	ui->editor_device->setVisible(!!current_device);
	if (!current_device) return;

	MMGNoEdit no_edit_device(current_device);
	QSignalBlocker blocker_in_enable(ui->button_in_enable);
	QSignalBlocker blocker_thru(ui->button_thru);
	QSignalBlocker blocker_thru_val(ui->editor_thru);
	QSignalBlocker blocker_out_enable(ui->button_out_enable);

	ui->button_in_enable->setEnabled(current_device->isCapable(TYPE_INPUT));
	ui->button_in_enable->setChecked(current_device->isActive(TYPE_INPUT));
	onDeviceInputActiveChange(current_device->isActive(TYPE_INPUT));

	ui->button_out_enable->setEnabled(current_device->isCapable(TYPE_OUTPUT));
	ui->button_out_enable->setChecked(current_device->isActive(TYPE_OUTPUT));
	onDeviceOutputActiveChange(current_device->isActive(TYPE_OUTPUT));

	ui->editor_thru->clear();
	for (MMGDevice *device : *manager(device))
		if (device->isCapable(TYPE_OUTPUT)) ui->editor_thru->addItem(device->objectName());

	bool thru_enabled = current_device->isActive(TYPE_INPUT) && !current_device->thru().isEmpty();
	ui->button_thru->setChecked(thru_enabled);
	onDeviceThruStateChange(thru_enabled);
	ui->editor_thru->setCurrentText(current_device->thru());

	ui->button_remove_device->setVisible(!current_device->isCapable(TYPE_NONE));
}

void MMGEchoWindow::onDeviceInputActiveChange(bool toggled)
{
	current_device->setActive(TYPE_INPUT, toggled);
	toggled = current_device->isPortOpen(TYPE_INPUT);
	ui->button_in_enable->setChecked(toggled);
	ui->label_in_connect->setText(current_device->status(TYPE_INPUT));

	ui->editor_device_thru->setVisible(toggled);
	ui->editor_thru->setVisible(ui->button_thru->isChecked());
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
	ui->editor_thru->setVisible(toggled);

	if (ui->editor_thru->signalsBlocked()) return;
	current_device->setThru(toggled ? ui->editor_thru->currentText() : "");
}

void MMGEchoWindow::onDeviceThruChange(const QString &device)
{
	current_device->setThru(device);
}

void MMGEchoWindow::onDeviceCheck()
{
	current_device->checkCapable();
	deviceShow();
	MMGInterface::promptUser("DeviceCheck", false);
}

void MMGEchoWindow::onDeviceRemove()
{
	if (!MMGInterface::promptUser("DeviceRemove")) return;

	manager(device)->remove(current_device);
	device_control->refresh();
	deviceShow();
}

void MMGEchoWindow::multiShow()
{
	QListWidgetItem *item = ui->structure_multi->currentItem();
	ui->editor_secondary_buttons->setVisible(!!item);
	ui->label_binding_name->setVisible(!!item);

	switch (ui->structure_multi->property("kind").toInt()) {
		case MODE_BINDING:
		default:
			bindingShow();
			ui->button_move->setVisible(!menu_binding_groups->isEmpty());
			ui->button_remove->setVisible(current_collection->size() > 1);
			break;

		case MODE_ACTION:
			actionShow();
			ui->button_move->setVisible(false);
			ui->button_remove->setVisible(current_binding->actions()->size() > 1);
			break;

		case MODE_MESSAGE:
			messageShow();
			ui->button_move->setVisible(false);
			ui->button_remove->setVisible(current_binding->messages()->size() > 1);
			break;
	}
}

void MMGEchoWindow::onMultiRename(QListWidgetItem *item)
{
	if (!item) return;
	ui->label_binding_name->setText(item->text());
}

void MMGEchoWindow::onMultiReset(int)
{
	ui->editor_binding->setVisible(!!current_binding);
	ui->editor_multi_access->setVisible(!!current_binding);

	messageShow();
	actionShow();
}

void MMGEchoWindow::onMultiClick()
{
	changeView(current_binding->type() == TYPE_OUTPUT ? MODE_MESSAGE : MODE_ACTION);
}

void MMGEchoWindow::onConfirmClick()
{
	switch (ui->structure_multi->property("kind").toInt()) {
		case MODE_BINDING:
			changeView(MODE_COLLECTION);
			break;

		default:
			changeView(MODE_BINDING);
			break;
	}
}

void MMGEchoWindow::bindingShow()
{
	current_binding = binding_control->currentValue();

	onMultiReset();

	if (!current_binding) return;

	QSignalBlocker blocker_enable(ui->button_enable);
	QSignalBlocker blocker_switch(ui->button_switch);
	QSignalBlocker blocker_binding_reset(ui->editor_binding_reset);
	QSignalBlocker blocker_multi_select(ui->editor_multi_select);

	ui->label_binding_name->setText(current_binding->objectName());
	bool is_output = current_binding->type() == TYPE_OUTPUT;
	ui->message_frame->move(is_output ? 720 : 360, 60);
	ui->action_frame->move(is_output ? 360 : 720, 60);

	ui->button_enable->setChecked(current_binding->enabled());
	onBindingActiveChange(current_binding->enabled());

	ui->button_switch->setChecked((bool)current_binding->type());
	onBindingSwitch((bool)current_binding->type());

	ui->editor_binding_reset->setCurrentIndex((int)current_binding->resetMode());
	onBindingResetChange((int)current_binding->resetMode());

	ui->editor_multi_select->clear();
	ui->editor_multi_select->addItems(is_output ? current_binding->messages()->names()
						    : current_binding->actions()->names());
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
		if (!MMGInterface::promptUser("BindingSwitch")) return;
		ui->button_switch->setChecked(toggled);
	}

	if (ui->button_switch->signalsBlocked()) return;
	current_binding->setType((DeviceType)toggled);
	bindingShow();
}

void MMGEchoWindow::onBindingResetChange(int index)
{
	current_binding->setResetMode(index);
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

	binding_control->refresh();
	multiShow();
}

void MMGEchoWindow::messageShow()
{
	current_message = nullptr;

	switch (ui->structure_multi->property("kind").toInt()) {
		case MODE_BINDING:
		default:
			if (current_binding) current_message = current_binding->messages(multiIndex(TYPE_INPUT));
			break;

		case MODE_MESSAGE:
			current_message = message_control->currentValue();
			if (!!current_message) ui->label_binding_name->setText(current_message->objectName());
			break;
	}

	ui->message_frame->setVisible(!!current_message);
	if (!current_message) return;

	MMGNoEdit no_edit_message(current_message);
	message_display->setStorage(current_message);
}

void MMGEchoWindow::actionShow()
{
	current_action = nullptr;

	switch (ui->structure_multi->property("kind").toInt()) {
		case MODE_BINDING:
		default:
			if (current_binding) current_action = current_binding->actions(multiIndex(TYPE_OUTPUT));
			break;

		case MODE_ACTION:
			current_action = action_control->currentValue();
			if (!!current_action) ui->label_binding_name->setText(current_action->objectName());
			break;
	}

	ui->action_frame->setVisible(!!current_action);
	if (!current_action) return;

	MMGNoEdit no_edit_action(current_action);
	QSignalBlocker blocker_cat(ui->editor_cat);
	QSignalBlocker blocker_sub(ui->editor_sub);

	ui->editor_cat->setCurrentIndex((int)current_action->category());
	onActionSubUpdate();

	ui->editor_sub->setCurrentIndex(current_action->sub());
	onActionSubChange(current_action->sub());
}

void MMGEchoWindow::onActionCategoryChange(int index)
{
	QJsonObject json_obj;
	json_obj["name"] = current_action->objectName();
	json_obj["category"] = index;
	current_binding->actions()->changeActionCategory(current_action, json_obj);
	current_binding->refresh();

	QSignalBlocker blocker_multi(ui->structure_multi);
	action_control->refresh();

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
		current_action->createDisplay(ui->editor_action_display);
		ui->editor_action_display->addWidget(current_action->display());
	}

	ui->editor_action_display->setCurrentWidget(current_action->display());
	current_action->setActionParams();
}

void MMGEchoWindow::preferenceShow()
{
	/*MMGSettings *current_settings = binding_edit ? current_binding->settings()
						     : preferences_display->currentValue();
	ui->preferences_frame->setVisible(!!current_settings);
	if (!current_settings) return;

	MMGNoEdit no_edit_settings(manager(setting));

	if (!current_settings->display()) current_settings->createDisplay(this);
	ui->scroll_preferences->takeWidget();
	ui->scroll_preferences->setWidget(current_settings->display());*/
}

void MMGEchoWindow::exportBindings()
{
	QString filepath = QFileDialog::getSaveFileName(this, mmgtr("UI.Filesystem.ExportTitle"), MMGConfig::filename(),
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
