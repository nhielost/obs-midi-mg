/*
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
#include "mmg-number-display.h"
#include "mmg-string-display.h"
#include "mmg-binding-display.h"
#include "mmg-manager-display.h"
#include "../mmg-device.h"

class MMGEchoWindow : public QDialog {
	Q_OBJECT

public:
	explicit MMGEchoWindow(QWidget *parent);
	~MMGEchoWindow() override;

private:
	Ui::MMGEchoWindow *ui;

	MMGBindingDisplay *main_binding_display;

	MMGStringDisplay *type_display;
	MMGNumberDisplay *channel_display;
	MMGNumberDisplay *note_display;
	MMGNumberDisplay *value_display;

	MMGManagerDisplay<MMGDevice> *devices_display;
	MMGManagerDisplay<MMGBinding> *bindings_display;
	MMGManagerDisplay<MMGMessage> *messages_display;
	MMGManagerDisplay<MMGAction> *actions_display;
	MMGManagerDisplay<MMGSettings> *preferences_display;

	MMGDevice *current_device = nullptr;
	MMGBinding *current_binding = nullptr;
	MMGMessage *current_message = nullptr;
	MMGAction *current_action = nullptr;

	short listening_mode = 0;
	bool binding_edit = false;

	void reject() override;
	void connectUISignals();
	void translate();

public slots:
	void displayWindow();
	void updateMessage(const MMGSharedMessage &);

private slots:
	void onAddClick();
	void onCopyClick();
	void onRemoveClick();
	void onConfirmClick();
	void onScreenChange(int page);

	void deviceShow();
	void deviceBindingEdit();
	void onDeviceInputActiveChange(bool toggled);
	void onDeviceOutputActiveChange(bool toggled);
	void onDeviceThruStateChange(bool toggled);
	void onDeviceThruChange(const QString &device);

	void messageShow();
	void messageBindingEdit();
	void onMessageTypeChange();
	void onListenOnceClick(bool toggled);
	void onListenContinuousClick(bool toggled);

	void actionShow();
	void actionBindingEdit();
	void onActionCategoryChange(int index);
	void onActionSwitch(bool toggled);
	void onActionSubUpdate();
	void onActionSubChange(int index);
	void onActionEditRequest(MMGBinding *binding, int page);

	void bindingShow();
	void bindingBindingEdit(){};
	void onBindingActiveChange(bool toggled);
	void onBindingSwitch(bool toggled);
	void onBindingFieldEdit(int page);

	void preferenceShow();
	void preferenceBindingEdit();
	void exportBindings();
	void importBindings();
	void openHelp() const;
	void reportBug() const;
	void checkForUpdates() const;
};
