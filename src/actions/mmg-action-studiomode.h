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

class MMGActionStudioMode : public MMGAction {
  public:
  explicit MMGActionStudioMode() { blog(LOG_DEBUG, "Empty action created."); };
  explicit MMGActionStudioMode(const QJsonObject &json_obj);
  enum Actions {
    STUDIOMODE_ON,
    STUDIOMODE_OFF,
    STUDIOMODE_TOGGLE_ONOFF,
    STUDIOMODE_CHANGEPREVIEW,
    STUDIOMODE_TRANSITION
  };

  void blog(int log_status, const QString &message) const override;
  void do_action(const MMGMessage *midi) override;
  void json(QJsonObject &json_obj) const override;
  void deep_copy(MMGAction *dest) const override;

  Category get_category() const override { return Category::MMGACTION_STUDIOMODE; }

  MMGUtils::MMGString &str1() override { return scene; };
  const MMGUtils::MMGString &str1() const override { return scene; };

  void change_options_sub(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str1(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str2(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str3(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_final(MMGUtils::MMGActionDisplayParams &val) override;

  private:
  MMGUtils::MMGString scene;
};
