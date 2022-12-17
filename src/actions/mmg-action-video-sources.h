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

class MMGActionVideoSources : public MMGAction {
  public:
  explicit MMGActionVideoSources() { blog(LOG_DEBUG, "Empty action created."); };
  explicit MMGActionVideoSources(const QJsonObject &json_obj);
  enum Actions {
    SOURCE_VIDEO_POSITION,
    SOURCE_VIDEO_DISPLAY,
    SOURCE_VIDEO_LOCKED,
    SOURCE_VIDEO_CROP,
    SOURCE_VIDEO_ALIGNMENT,
    SOURCE_VIDEO_SCALE,
    SOURCE_VIDEO_SCALEFILTER,
    SOURCE_VIDEO_ROTATION,
    SOURCE_VIDEO_BOUNDING_BOX_TYPE,
    SOURCE_VIDEO_BOUNDING_BOX_SIZE,
    SOURCE_VIDEO_BOUNDING_BOX_ALIGN,
    SOURCE_VIDEO_BLEND_MODE,
    SOURCE_VIDEO_SCREENSHOT,
    SOURCE_VIDEO_CUSTOM
  };

  void blog(int log_status, const QString &message) const override;
  void do_action(const MMGMessage *midi) override;
  void json(QJsonObject &json_obj) const override;
  void deep_copy(MMGAction *dest) const override;

  Category get_category() const override { return Category::MMGACTION_SOURCE_VIDEO; }

  MMGUtils::MMGString &str1() override { return parent_scene; };
  const MMGUtils::MMGString &str1() const override { return parent_scene; };
  MMGUtils::MMGString &str2() override { return source; };
  const MMGUtils::MMGString &str2() const override { return source; };
  MMGUtils::MMGString &str3() override { return action; };
  const MMGUtils::MMGString &str3() const override { return action; };
  MMGUtils::MMGNumber &num1() override { return nums[0]; };
  const MMGUtils::MMGNumber &num1() const override { return nums[0]; };
  MMGUtils::MMGNumber &num2() override { return nums[1]; };
  const MMGUtils::MMGNumber &num2() const override { return nums[1]; };
  MMGUtils::MMGNumber &num3() override { return nums[2]; };
  const MMGUtils::MMGNumber &num3() const override { return nums[2]; };
  MMGUtils::MMGNumber &num4() override { return nums[3]; };
  const MMGUtils::MMGNumber &num4() const override { return nums[3]; };

  void change_options_sub(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str1(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str2(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_str3(MMGUtils::MMGActionDisplayParams &val) override;
  void change_options_final(MMGUtils::MMGActionDisplayParams &val) override;

  static const QStringList enumerate(const QString &restriction = "");

  private:
  MMGUtils::MMGString parent_scene;
  MMGUtils::MMGString source;
  MMGUtils::MMGString action;
  MMGUtils::MMGNumber nums[4];
};
