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
#include "mmg-action.h"

class MMGActionInternal : public MMGAction {
  public:
  explicit MMGActionInternal() { blog(LOG_DEBUG, "Empty action created."); };
  explicit MMGActionInternal(const QJsonObject &json_obj);
  enum Actions { INTERNAL_1, INTERNAL_2, INTERNAL_3 };

  void blog(int log_status, const QString &message) const override;
  void do_action(const MMGMessage *midi) override;
  void json(QJsonObject &json_obj) const override;
  void deep_copy(MMGAction *dest) const override;

  Category get_category() const override { return Category::MMGACTION_INTERNAL; }

  MMGUtils::MMGString &str1() override { return actions[0]; };
  const MMGUtils::MMGString &str1() const override { return actions[0]; };
  MMGUtils::MMGString &str2() override { return actions[1]; };
  const MMGUtils::MMGString &str2() const override { return actions[1]; };
  MMGUtils::MMGString &str3() override { return actions[2]; };
  const MMGUtils::MMGString &str3() const override { return actions[2]; };

  void change_options_sub(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str1(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str2(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str3(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_final(MMGUtils::MMGActionDisplayParams &val) override;

  static const QStringList enumerate(const QString &current_binding);

  private:
  MMGUtils::MMGString actions[3];
};
