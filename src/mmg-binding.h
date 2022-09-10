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
	explicit MMGBinding();
	explicit MMGBinding(const QJsonObject &obj);
	~MMGBinding()
	{
		qDeleteAll(messages);
		qDeleteAll(actions);
		saved_messages.clear();
	};

	void json(QJsonObject &binding_obj) const;
	void blog(int log_status, const QString &message) const;

	enum class Mode {
		MMGBINDING_INVALID,
		MMGBINDING_CONSECUTIVE,
		MMGBINDING_CORRESPONDENCE,
		MMGBINDING_MULTIPLY
	};

	const QString get_name() const { return name; };
	Mode get_mode() const { return (Mode)mode; };
	bool get_note_toggling() const { return note_toggling; };

	void set_name(const QString &val) { name = val; };
	void set_mode(Mode val) { mode = (short)val; };
	void set_note_toggling(bool val);

	MMGMessage *add_message(MMGMessage *const el = new MMGMessage);
	void insert_message(size_t index, MMGMessage *const el);
	void remove(MMGMessage *const el);
	MMGAction *add_action(MMGAction *const el = new MMGAction);
	void insert_action(size_t index, MMGAction *const el);
	void remove(MMGAction *const el);

	size_t index_of(MMGMessage *const el) const;
	size_t index_of(MMGAction *const el) const;
	size_t message_size() const;
	size_t action_size() const;

	const MMGMessageList &get_messages() const { return messages; };
	const MMGActionList &get_actions() const { return actions; };
	MMGMessage *find_message(const QString &name);
	MMGAction *find_action(const QString &name);

	bool is_valid();
	void do_actions(const MMGSharedMessage &el);

	void move_elements(MMGModes mode, size_t from, size_t to);

	static qulonglong get_next_default() { return next_default; };
	static void set_next_default(qulonglong num) { next_default = num; };
	static QString get_next_default_name();

private:
	QString name;
	short mode;
	bool note_toggling;
	QList<MMGMessage *> messages;
	QList<MMGAction *> actions;
	QList<MMGSharedMessage> saved_messages;

	size_t current_message_index = 0;

	static qulonglong next_default;

	void check_message_default_names();
	void check_action_default_names();
};

using MMGBindingList = QList<MMGBinding *>;
