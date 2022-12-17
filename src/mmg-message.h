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

  MMGUtils::MMGString &type() { return _type; };
  const MMGUtils::MMGString &type() const { return _type; };
  MMGUtils::MMGNumber &channel() { return _channel; };
  const MMGUtils::MMGNumber &channel() const { return _channel; };
  MMGUtils::MMGNumber &note() { return _note; };
  const MMGUtils::MMGNumber &note() const { return _note; };
  MMGUtils::MMGNumber &value() { return _value; };
  const MMGUtils::MMGNumber &value() const { return _value; };

  void toggle();
  bool is_acceptable(const MMGMessage *test) const;
  void deep_copy(MMGMessage *dest);

  static QString get_midi_type(const libremidi::message &mess);
  static int get_midi_note(const libremidi::message &mess);
  static int get_midi_value(const libremidi::message &mess);

  private:
  MMGUtils::MMGNumber _channel;
  MMGUtils::MMGString _type;
  MMGUtils::MMGNumber _note;
  MMGUtils::MMGNumber _value;
};

using MMGSharedMessage = QSharedPointer<MMGMessage>;
