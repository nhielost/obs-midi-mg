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
#include "../mmg-message.h"

#include <obs-frontend-api.h>

class MMGAction {
  public:
  virtual ~MMGAction() = default;

  enum class Category {
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
    MMGACTION_INTERNAL,
    MMGACTION_TIMEOUT
  };

  virtual void json(QJsonObject &action_obj) const = 0;
  virtual void do_action(const MMGMessage *midi) = 0;
  virtual void deep_copy(MMGAction *dest) const = 0;
  virtual Category get_category() const = 0;

  short get_sub() const { return subcategory; };
  void set_sub(short val) { subcategory = val; };

  virtual MMGUtils::MMGString &str1() { return empty_str; };
  virtual const MMGUtils::MMGString &str1() const { return empty_str; };
  virtual MMGUtils::MMGString &str2() { return empty_str; };
  virtual const MMGUtils::MMGString &str2() const { return empty_str; };
  virtual MMGUtils::MMGString &str3() { return empty_str; };
  virtual const MMGUtils::MMGString &str3() const { return empty_str; };
  virtual MMGUtils::MMGNumber &num1() { return empty_num; };
  virtual const MMGUtils::MMGNumber &num1() const { return empty_num; };
  virtual MMGUtils::MMGNumber &num2() { return empty_num; };
  virtual const MMGUtils::MMGNumber &num2() const { return empty_num; };
  virtual MMGUtils::MMGNumber &num3() { return empty_num; };
  virtual const MMGUtils::MMGNumber &num3() const { return empty_num; };
  virtual MMGUtils::MMGNumber &num4() { return empty_num; };
  virtual const MMGUtils::MMGNumber &num4() const { return empty_num; };

  virtual void change_options_sub(MMGUtils::MMGActionDisplayParams &val) = 0;
  virtual void change_options_str1(MMGUtils::MMGActionDisplayParams &val) = 0;
  virtual void change_options_str2(MMGUtils::MMGActionDisplayParams &val) = 0;
  virtual void change_options_str3(MMGUtils::MMGActionDisplayParams &val) = 0;
  virtual void change_options_final(MMGUtils::MMGActionDisplayParams &val) = 0;

  virtual void blog(int log_status, const QString &message) const;
  void reset_execution() { executed = false; };

  protected:
  int subcategory = 0;
  bool executed = false;

  private:
  static MMGUtils::MMGString empty_str;
  static MMGUtils::MMGNumber empty_num;
};

// Macros
#define MIDI_NUMBER_IS_NOT_IN_RANGE(mmgnumber, range) \
  mmgnumber.state() == MMGNumber::NUMBERSTATE_MIDI && (uint)midi->value() >= range

#define MIDI_STRING_IS_NOT_IN_RANGE(mmgstring, range) \
  mmgstring.state() == MMGString::STRINGSTATE_MIDI && (uint)midi->value() >= range
// End Macros

struct R {
  QStringList list;
  QString str;
};
