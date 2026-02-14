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

#ifndef MMG_VALUE_MANAGER_H
#define MMG_VALUE_MANAGER_H

#include "mmg-state-display.h"
#include "mmg-value-display.h"

namespace MMGWidgets {

class MMGValueManager : public QWidget {
	Q_OBJECT

public:
	MMGValueManager(QWidget *parent, MMGStateDisplay *state_display = nullptr);

	int32_t count() const;
	int32_t scrollCount() const { return scroll_layout->count(); };

	template <typename T> void addNew(MMGValue<T> *storage, const MMGParams<T> *params, const MMGCallback &cb = 0)
	{
		if (params->options != OPTION_NONE) {
			auto *state_display = new MMGValueStateDisplay<T>(this, params, storage);
			state_infos += state_display;

			connect(state_display, &MMGValueQWidget::editRequested, this,
				&MMGValueManager::sendEditRequest<T>);
			connectNew(state_display, cb);
			scroll_layout->addWidget(state_display);
		} else {
			storage->template changeTo<STATE_FIXED>();

			auto *fixed_display = new MMGValueFixedDisplay<T>(this, params, storage);
			connectNew(fixed_display, cb);
			scroll_layout->addWidget(fixed_display);
		}
	};

	void addManager(MMGValueManager *manager);
	void addCustom(QWidget *other);

	void removeExcept(int32_t remaining);
	void removeAll() { removeExcept(0); };

	void setModifiable(bool modify) { emit modifyRequested(modify); };

protected:
	template <typename T> void addFixed(MMGValue<T> *storage, const MMGParams<T> *params, const MMGCallback &cb = 0)
	{
		auto *value_display = new MMGValueFixedDisplay<T>(this, params, storage);
		connectNew(value_display, cb);
		fixed_layout->addWidget(value_display);
	};

	void clear();

signals:
	void refreshRequested();
	void editRequested();
	void modifyRequested(bool);

protected slots:
	void refreshAll();

private:
	void connectNew(MMGValueQWidget *value_display, const MMGCallback &cb);
	void sendRefresh(MMGValueQWidget *value_display);

	template <typename T> void sendEditRequest()
	{
		if (!state_display) return;

		if (!!current_value_display_editor) {
			disconnect(current_value_display_editor, &QObject::destroyed, this, nullptr);
			current_value_display_editor->setEditingProperty(-1);
		}

		auto *value_display = dynamic_cast<MMGValueStateDisplay<T> *>(sender());
		if (!value_display->isModifiable()) return;
		current_value_display_editor = current_value_display_editor != value_display ? value_display : nullptr;

		if (!!current_value_display_editor) {
			connect(current_value_display_editor, &QObject::destroyed, this,
				[&]() { current_value_display_editor = nullptr; });
			current_value_display_editor->setEditingProperty(1);
		}

		state_display->setStorage(!!current_value_display_editor ? value_display : nullptr);
	};

public:
	void clearEditRequest();

protected:
	QVBoxLayout *fixed_layout;
	QVBoxLayout *scroll_layout;
	QVBoxLayout *bottom_layout;
	QObject *refresh_sender = nullptr;

	MMGValueStateInfoList state_infos;

private:
	MMGValueQWidget *current_value_display_editor = nullptr;
	MMGStateDisplay *state_display;
};

} // namespace MMGWidgets

#endif // MMG_VALUE_MANAGER_H
