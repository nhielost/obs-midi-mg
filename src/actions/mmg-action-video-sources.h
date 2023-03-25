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
  void execute(const MMGMessage *midi) const override;
  void json(QJsonObject &json_obj) const override;
  void copy(MMGAction *dest) const override;
  void setEditable(bool edit) override;
  void createDisplay(QWidget *parent) override;
  void setSubOptions(QComboBox *sub) override;

  Category category() const override { return Category::MMGACTION_SOURCE_VIDEO; }

  static const QStringList enumerate();
  static const vec2 obsResolution();
  const vec2 sourceResolution() const;

  private:
  MMGUtils::MMGString parent_scene;
  MMGUtils::MMGString source;
  MMGUtils::MMGString action;
  MMGUtils::MMGString json_str;
  MMGUtils::MMGNumber nums[4];

  void setSubConfig() override;
  void setList1Config() override;
  void setList2Config() override;

  const MMGUtils::MMGNumber &num1() const { return nums[0]; };
  const MMGUtils::MMGNumber &num2() const { return nums[1]; };
  const MMGUtils::MMGNumber &num3() const { return nums[2]; };
  const MMGUtils::MMGNumber &num4() const { return nums[3]; };

  static const QStringList alignment_options;
  static const QStringList boundingbox_options;
  static const QStringList scalefilter_options;
  static const QStringList blendmode_options;
};
