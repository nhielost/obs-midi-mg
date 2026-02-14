/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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
#include <QProxyStyle>

#include "../mmg-device.h"
#include "mmg-action-display.h"
#include "mmg-manager-display.h"
#include "mmg-message-display.h"
#include "mmg-state-display.h"
#include "ui_mmg-echo-window.h"

class MMGEchoWindow : public QDialog {
	Q_OBJECT

public:
	explicit MMGEchoWindow(QWidget *parent);
	~MMGEchoWindow() override;

	enum View { MODE_COLLECTION, MODE_BINDING, MODE_MESSAGE, MODE_ACTION };

private:
	Ui::MMGEchoWindow *ui;

	QMenu *menu_binding_groups;

	MMGWidgets::MMGMessageDisplay *message_object_display;
	MMGWidgets::MMGActionDisplay *action_object_display;
	MMGWidgets::MMGStateDisplay *state_display;

	MMGWidgets::MMGManagerDisplay<MMGBindingManager> *collection_display;
	MMGBindingManager *current_collection = nullptr;

	MMGWidgets::MMGManagerDisplay<MMGDevice> *device_display;
	MMGDevice *current_device = nullptr;

	MMGWidgets::MMGValueFixedDisplay<MMGMIDIPort *> *thru_display;
	static MMGParams<MMGMIDIPort *> thru_params;

	MMGWidgets::MMGValueFixedDisplay<MMGBinding::ResetMode> *reset_mode_display;
	static MMGParams<MMGBinding::ResetMode> reset_mode_params;

	MMGWidgets::MMGManagerDisplay<MMGPreference> *preference_display;
	MMGPreference *current_preference = nullptr;

	MMGWidgets::MMGManagerDisplay<MMGBinding> *binding_display;
	MMGBinding *current_binding = nullptr;

	MMGWidgets::MMGManagerDisplay<MMGMessage> *message_display;
	MMGMessage *current_message = nullptr;

	MMGWidgets::MMGManagerDisplay<MMGAction> *action_display;
	MMGAction *current_action = nullptr;

	View current_view = MODE_COLLECTION;

	void reject() override;
	void connectUISignals();
	void translate();

	void changeView(View view);
	void setTitle(const QString &name = "");
	void refreshStylesheet(QWidget *widget);

public slots:
	void displayWindow();

private slots:
	void collectionShow();

	void deviceShow();
	void onDeviceInputActiveChange(bool toggled);
	void onDeviceOutputActiveChange(bool toggled);
	void onDeviceThruChange();
	void onDeviceRefresh();
	void onDeviceRemove();

	void onMessageEditClick();
	void onActionEditClick();
	void onConfirmClick();

	void bindingShow();
	void onBindingActiveChange(bool toggled);
	void onBindingSwitch(bool toggled);
	void onBindingResetChange();
	void onBindingMoveClick();
	void onBindingMoveSelect();
	void onBindingConditionChange();

	void messageShow();
	void actionShow();
	void preferenceShow();
};
