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
#include "mmg-action.h"

class MMGActionHotkeys : public MMGAction {
	Q_OBJECT

public:
	MMGActionHotkeys(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { HOTKEY_PREDEF };
	enum Events { HOTKEY_ACTIVATED };

	Category category() const override { return MMGACTION_HOTKEY; };
	const QString trName() const override { return "Hotkeys"; };
	const QStringList subNames() const override { return {subModuleText("Activate")}; };

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;
	void setEditable(bool edit) override;
	void toggle() override;

	void createDisplay(QWidget *parent) override;
	void setActionParams() override;

	void execute(const MMGMessage *) const override;
	void connectSignals(bool connect) override;

	static const QMap<QString, QString> enumerate(const QString &category);
	static const QStringList enumerateCategories();
	static const QString registerer(obs_hotkey_t *hotkey);

private:
	MMGUtils::MMGString group;
	MMGUtils::MMGString hotkey;

private slots:
	void onList1Change();

	void hotkeyEventReceived(obs_hotkey_id id);
};
