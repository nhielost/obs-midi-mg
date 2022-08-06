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
	MMGMessage() = default;

	explicit MMGMessage(const libremidi::message &message);
	explicit MMGMessage(const QJsonObject &obj);

	void json(QJsonObject &message_obj) const;

	const QString get_name() const { return name; };
	const QString get_type() const { return type; };
	int get_channel() const { return channel; };
	int get_note() const { return note; };
	int get_value() const { return value; };

	void set_name(const QString &val) { name = val; };
	void set_type(const QString &val) { type = val; };
	void set_channel(int val) { channel = val; };
	void set_note(int val) { note = val; };
	void set_value(int val) { value = val; };

	bool is_acceptable(const MMGMessage *const test) const;
	const libremidi::message to_libremidi_message() const;

	static QString get_midi_type(const libremidi::message &mess);
	static int get_midi_note(const libremidi::message &mess);
	static int get_midi_value(const libremidi::message &mess);

private:
	QString name = MMGUtils::next_default_name(MMGModes::MMGMODE_MESSAGE);
	QString type = "Note On";
	int channel = 1;
	int note = 0;
	int value = -1;
};

using MMGMessageList = QList<MMGMessage *>;
