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

#ifndef MMG_ACTION_H
#define MMG_ACTION_H

#include "../mmg-message.h"
#include "../ui/mmg-action-display.h"

#include <obs-frontend-api.h>

class MMGAction {
  public:
  virtual ~MMGAction() = default;

  enum Category {
    MMGACTION_NONE,
    MMGACTION_STREAM,
    MMGACTION_RECORD,
    MMGACTION_VIRCAM,
    MMGACTION_REPBUF,
    MMGACTION_STUDIOMODE,
    MMGACTION_SCENE,
    MMGACTION_SOURCE_VIDEO,
    MMGACTION_SOURCE_AUDIO,
    MMGACTION_SOURCE_MEDIA,
    MMGACTION_TRANSITION,
    MMGACTION_FILTER,
    MMGACTION_HOTKEY,
    MMGACTION_PROFILE,
    MMGACTION_COLLECTION,
    MMGACTION_MIDI,
    MMGACTION_INTERNAL
  };

  virtual void json(QJsonObject &action_obj) const;
  virtual void execute(const MMGMessage *midi) const = 0;
  virtual void copy(MMGAction *dest) const;
  virtual Category category() const = 0;

  virtual void createDisplay(QWidget *parent) { _display = new MMGActionDisplay(parent); };
  MMGActionDisplay *display() { return _display; };

  short sub() const { return subcategory; };
  void setSub(short val) { subcategory = val; };
  virtual void setSubOptions(QComboBox *sub) = 0;

  virtual void blog(int log_status, const QString &message) const;

  virtual void setEditable(bool edit) { Q_UNUSED(edit); };

  virtual void setSubConfig(){};

  protected:
  int subcategory = 0;
  MMGActionDisplay *_display = nullptr;

  virtual void setList1Config(){};
  virtual void setList2Config(){};
  virtual void setList3Config(){};
};

// Macros
#define MIDI_NUMBER_IS_NOT_IN_RANGE(mmgnumber, range) \
  mmgnumber.state() == MMGNumber::NUMBERSTATE_MIDI && (uint)(midi->value()) >= range

#define MIDI_STRING_IS_NOT_IN_RANGE(mmgstring, range) \
  mmgstring.state() == MMGString::STRINGSTATE_MIDI && (uint)(midi->value()) >= range
// End Macros

struct R {
  QStringList list;
  QString str;
};

#endif // MMG_ACTION_H
