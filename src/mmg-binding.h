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

#ifndef MMG_BINDING_H
#define MMG_BINDING_H

#include "mmg-message.h"
#include "actions/mmg-action.h"
#include "mmg-midiin.h"
#include "mmg-midiout.h"

class MMGBinding : public QObject {
  Q_OBJECT

  public:
  explicit MMGBinding();
  explicit MMGBinding(const QJsonObject &obj);
  ~MMGBinding()
  {
    delete _message;
    delete _action;
  };

  void json(QJsonObject &binding_obj) const;
  void blog(int log_status, const QString &message) const;

  const QString &name() const { return _name; };
  bool enabled() const { return _enabled; };

  void setName(const QString &val) { _name = val; };
  void setEnabled(bool val);

  MMGMessage *message() { return _message; };
  const MMGMessage *message() const { return _message; };
  MMGAction *action() { return _action; };
  const MMGAction *action() const { return _action; };

  void setCategory(int index);
  void setCategory(const QJsonObject &json_obj);
  void copy(MMGBinding *dest);

  static qulonglong get_next_default() { return next_default; };
  static void set_next_default(qulonglong num) { next_default = num; };
  static QString get_next_default_name();

  public slots:
  void execute(const MMGSharedMessage &);

  private:
  QString _name;
  bool _enabled;
  MMGMessage *_message;
  MMGAction *_action;

  static qulonglong next_default;
};

using MMGBindingList = QList<MMGBinding *>;

#endif // MMG_BINDING_H
