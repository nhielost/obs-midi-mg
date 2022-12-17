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
#include "actions/mmg-action.h"

class MMGBinding {
  public:
  explicit MMGBinding();
  explicit MMGBinding(const QJsonObject &obj);
  ~MMGBinding()
  {
    delete message;
    delete action;
  };

  void json(QJsonObject &binding_obj) const;
  void blog(int log_status, const QString &message) const;

  const QString &get_name() const { return name; };
  bool get_enabled() const { return enabled; };

  void set_name(const QString &val) { name = val; };
  void set_enabled(bool val) { enabled = val; };

  MMGMessage *const get_message() const { return message; };
  MMGAction *const get_action() const { return action; };

  void set_action_type(int index);
  void set_action_type(const QJsonObject &json_obj);
  void deep_copy(MMGBinding *dest);
  void do_action(const MMGSharedMessage &el);
  void reset_execution() { action->reset_execution(); };

  static qulonglong get_next_default() { return next_default; };
  static void set_next_default(qulonglong num) { next_default = num; };
  static QString get_next_default_name();

  private:
  QString name;
  bool enabled;
  MMGMessage *message;
  MMGAction *action;

  static qulonglong next_default;
};

using MMGBindingList = QList<MMGBinding *>;
