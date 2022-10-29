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
#include "mmg-utils.h"

class MMGMessage {
public:
	explicit MMGMessage();

	explicit MMGMessage(const libremidi::message &message);
	explicit MMGMessage(const QJsonObject &obj);

	void json(QJsonObject &message_obj) const;
	void blog(int log_status, const QString &message) const;

	const QString &get_type() const { return type; };
	int get_channel() const { return channel; };
	int get_note() const { return note; };
	int get_value() const { return value; };
	bool get_value_required() const { return value_require; };

	void set_type(const QString &val) { type = val; };
	void set_channel(int val) { channel = val; };
	void set_note(int val) { note = val; };
	void set_value(int val) { value = val; };
	void set_value_required(bool val) { value_require = val; };

	void toggle();
	bool is_acceptable(const MMGMessage *test) const;
	void deep_copy(MMGMessage *dest);

	static QString get_midi_type(const libremidi::message &mess);
	static int get_midi_note(const libremidi::message &mess);
	static int get_midi_value(const libremidi::message &mess);

private:
	int channel;
	QString type;
	int note;
	int value;
	bool value_require;
};

using MMGSharedMessage = QSharedPointer<MMGMessage>;
