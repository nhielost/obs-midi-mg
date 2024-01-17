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
#include "mmg-message-display.h"
#include "../mmg-device.h"

class MMGEchoWindow : public QDialog {
	Q_OBJECT

public:
	explicit MMGEchoWindow(QWidget *parent);
	~MMGEchoWindow() override;

	enum MultiMode { MODE_BINDING, MODE_ACTION, MODE_MESSAGE };

private:
	Ui::MMGEchoWindow *ui;

	QMenu *menu_binding_groups;

	MMGMessageDisplay *message_display;

	MMGBindingManager *current_manager = nullptr;
	MMGDevice *current_device = nullptr;

	MMGBinding *current_binding = nullptr;
	MMGMessage *current_message = nullptr;
	MMGAction *current_action = nullptr;
	MultiMode multi_mode = MODE_BINDING;

	void reject() override;
	void connectUISignals();
	void translate();

	int multiIndex(MMGUtils::DeviceType) const;

public slots:
	void displayWindow();

private slots:
	void collectionShow(QListWidgetItem *item = nullptr);
	void onCollectionRename(QListWidgetItem *item);
	void onCollectionMove(const QModelIndex &, int from, int, const QModelIndex &, int to);
	void onCollectionAdd();
	void onCollectionEdit();
	void onCollectionConfirm();
	void onCollectionRemove();

	void deviceShow(QListWidgetItem *item = nullptr);
	void onDeviceInputActiveChange(bool toggled);
	void onDeviceOutputActiveChange(bool toggled);
	void onDeviceThruStateChange(bool toggled);
	void onDeviceThruChange(const QString &device);
	void onDeviceCheck();
	void onDeviceRemove();

	void multiShow(QListWidgetItem *item = nullptr);
	void onMultiRename(QListWidgetItem *item);
	void onMultiMove(const QModelIndex &, int from, int, const QModelIndex &, int to);
	void onMultiSelect(int);
	void onMultiClick();
	void onMultiChange();
	void onAddClick();
	void onCopyClick();
	void onMoveClick();
	void onMoveSelect();
	void onRemoveClick();

	void bindingShow(QListWidgetItem *item = nullptr);
	void onBindingActiveChange(bool toggled);
	void onBindingSwitch(bool toggled);
	void onBindingResetChange(int index);

	void messageShow(QListWidgetItem *item = nullptr);

	void actionShow(QListWidgetItem *item = nullptr);
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
