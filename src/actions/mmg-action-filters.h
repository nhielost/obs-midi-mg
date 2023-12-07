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

#pragma once
#include "mmg-action.h"

class MMGActionFilters : public MMGAction {
	Q_OBJECT

public:
	MMGActionFilters(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { FILTER_SHOW, FILTER_HIDE, FILTER_TOGGLE_SHOWHIDE, FILTER_REORDER, FILTER_CUSTOM };
	enum Events { FILTER_SHOWN, FILTER_HIDDEN, FILTER_TOGGLE_SHOWN, FILTER_REORDERED, FILTER_CUSTOM_CHANGED };

	Category category() const override { return MMGACTION_FILTER; };
	const QString trName() const override { return "Filters"; };

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;
	void setEditable(bool edit) override;
	void toggle() override;

	void createDisplay(QWidget *parent) override;
	void setComboOptions(QComboBox *sub) override;
	void setActionParams() override;

	void execute(const MMGMessage *midi) const override;
	void connectOBSSignals() override;
	void disconnectOBSSignals() override;

	static const QStringList enumerate(const QString &source);
	static const QStringList enumerateEligible();

private:
	MMGUtils::MMGString source;
	MMGUtils::MMGString filter;
	MMGUtils::MMGNumber num;
	MMGUtils::MMGJsonObject *_json;

	const MMGSourceSignal *active_source_signal = nullptr;

private slots:
	void onList1Change();
	void onList2Change();

	void filterEnableCallback(bool enable);
	void filterReorderCallback();
	void sourceDataCallback(void *_source);
};
