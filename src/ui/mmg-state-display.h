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

#ifndef MMG_STATE_DISPLAY_H
#define MMG_STATE_DISPLAY_H

#include "mmg-state-widget.h"

namespace MMGWidgets {

class MMGStateDisplay : public QWidget {
	Q_OBJECT

public:
	MMGStateDisplay(QWidget *parent);

	template <typename T> void setStorage(MMGValueStateDisplay<T> *storage)
	{
		setVisible(!!storage);
		if (!storage) return;
		connect(storage, &QObject::destroyed, this, &MMGStateDisplay::clearStorage);

		MMGValueOptions options = storage->params()->options;
		bool has_refs = !used_refs.isEmpty();
		bool refs_used = !used_refs.contains(nullptr);

		constructor.reset(new MMGStateWidgets::Construct<T>(this, storage, used_refs));
		states_group->button(STATE_MIDI)->setEnabled(has_refs && (options & OPTION_ALLOW_MIDI));
		states_group->button(STATE_RANGE)->setEnabled(has_refs && (options & OPTION_ALLOW_RANGE));
		states_group->button(STATE_TOGGLE)->setEnabled(options & OPTION_ALLOW_TOGGLE);
		states_group->button(STATE_INCREMENT)->setEnabled(refs_used && (options & OPTION_ALLOW_INCREMENT));
		states_group->button(STATE_IGNORE)->setEnabled(options & OPTION_ALLOW_IGNORE);
		setButtonState(storage->storage()->operator->()->state());
	};
	void refreshStorage();
	void clearStorage();

	void setMessageReferences(const MMGValueStateInfoList &refs) { message_refs = &refs; };
	void setActionReferences(const MMGValueStateInfoList &refs) { action_refs = &refs; };
	void applyReferences(DeviceType desired_type, DeviceType current_type);

signals:
	void visibilityChanged(bool);

protected:
	void showEvent(QShowEvent *) override { emit visibilityChanged(true); };
	void hideEvent(QHideEvent *) override { emit visibilityChanged(false); };

private:
	void setButtonState(uint16_t state);

private:
	QVBoxLayout *main_layout;
	QScrollArea *scroll_area;
	QFrame *sep_lower;
	QButtonGroup *states_group;
	QLabel *states_desc;

	MMGStateWidgets::MMGStateQWidget *state_widget = nullptr;
	std::unique_ptr<MMGStateWidgets::ConstructBase> constructor;

	MMGValueStateInfoList used_refs;
	const MMGValueStateInfoList *message_refs;
	const MMGValueStateInfoList *action_refs;
};

} // namespace MMGWidgets

#endif // MMG_STATE_DISPLAY_H
