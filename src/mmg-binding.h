/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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
#include "mmg-message.h"
#include "mmg-action.h"

class MMGBinding {
public:
	MMGBinding() = default;
	explicit MMGBinding(const QJsonObject &obj);
	~MMGBinding()
	{
		qDeleteAll(messages);
		qDeleteAll(actions);
		saved_messages.clear();
	};

	void json(QJsonObject &binding_obj) const;

	enum class Mode {
		MMGBINDING_INVALID,
		MMGBINDING_CONSECUTIVE,
		MMGBINDING_CORRESPONDENCE,
		MMGBINDING_MULTIPLY
	};

	const QString get_name() const { return name; };
	Mode get_mode() const { return (Mode)mode; };

	void set_name(const QString &val) { name = val; };
	void set_mode(Mode val) { mode = (int)val; };

	MMGMessage *add_message(MMGMessage *const el = new MMGMessage);
	void insert_message(int index, MMGMessage *const el);
	void remove(MMGMessage *const el);
	MMGAction *add_action(MMGAction *const el = new MMGAction);
	void insert_action(int index, MMGAction *const el);
	void remove(MMGAction *const el);

	int index_of(MMGMessage *const el) const;
	int index_of(MMGAction *const el) const;
	int message_size() const;
	int action_size() const;

	const MMGMessageList &get_messages() const { return messages; };
	const MMGActionList &get_actions() const { return actions; };
	MMGMessage *find_message(const QString &name);
	MMGAction *find_action(const QString &name);

	bool is_valid();
	void do_actions(const MMGSharedMessage &el);

	void move_elements(MMGModes mode, int from, int to);

private:
	QString name = MMGUtils::next_default_name(MMGModes::MMGMODE_BINDING);
	int mode = 1;
	QList<MMGMessage *> messages;
	QList<MMGAction *> actions;
	QList<MMGSharedMessage> saved_messages;

	int current_message_index = 0;
};

using MMGBindingList = QList<MMGBinding *>;
