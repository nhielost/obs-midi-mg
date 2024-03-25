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

#pragma once
#include <QDialog>

#include "ui_mmg-echo-window.h"
#include "mmg-message-display.h"
#include "mmg-manager-control.h"
#include "../mmg-device.h"

class MMGEchoWindow : public QDialog {
	Q_OBJECT

public:
	explicit MMGEchoWindow(QWidget *parent);
	~MMGEchoWindow() override;

	enum MultiMode { MODE_COLLECTION, MODE_DEVICE, MODE_BINDING, MODE_MESSAGE, MODE_ACTION };

private:
	Ui::MMGEchoWindow *ui;

	QMenu *menu_binding_groups;

	MMGMessageDisplay *message_display;

	MMGManagerControl<MMGBindingManager> *collection_control;
	MMGManagerControl<MMGDevice> *device_control;
	MMGManagerControl<MMGBinding> *binding_control;
	MMGManagerControl<MMGMessage> *message_control;
	MMGManagerControl<MMGAction> *action_control;

	MMGBindingManager *current_collection = nullptr;
	MMGDevice *current_device = nullptr;
	MMGBinding *current_binding = nullptr;
	MMGMessage *current_message = nullptr;
	MMGAction *current_action = nullptr;

	void reject() override;
	void connectUISignals();
	void translate();

	void changeView(MultiMode mode);
	int multiIndex(MMGUtils::DeviceType) const;

public slots:
	void displayWindow();

private slots:
	void collectionShow();

	void deviceShow();
	void onDeviceInputActiveChange(bool toggled);
	void onDeviceOutputActiveChange(bool toggled);
	void onDeviceThruStateChange(bool toggled);
	void onDeviceThruChange(const QString &device);
	void onDeviceCheck();
	void onDeviceRemove();

	void multiShow();
	void onMultiRename(QListWidgetItem *item);
	void onMultiReset(int = 0);
	void onMultiClick();
	void onConfirmClick();

	void bindingShow();
	void onBindingActiveChange(bool toggled);
	void onBindingSwitch(bool toggled);
	void onBindingResetChange(int index);
	void onBindingMoveClick();
	void onBindingMoveSelect();

	void messageShow();

	void actionShow();
	void onActionCategoryChange(int index);
	void onActionSubUpdate();
	void onActionSubChange(int index);

	void preferenceShow();
	void exportBindings();
	void importBindings();
	void openHelp() const;
	void reportBug() const;
	void checkForUpdates() const;
};
