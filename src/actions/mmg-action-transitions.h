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

class MMGActionTransitions : public MMGAction {
  public:
  explicit MMGActionTransitions();
  explicit MMGActionTransitions(const QJsonObject &json_obj);

  enum Actions {
    TRANSITION_CURRENT,
    TRANSITION_SOURCE_SHOW,
    TRANSITION_SOURCE_HIDE,
    TRANSITION_TBAR_ACTIVATE,
    TRANSITION_TBAR_RELEASE,
    TRANSITION_CUSTOM
  };

  void blog(int log_status, const QString &message) const override;
  void execute(const MMGMessage *midi) const override;
  void json(QJsonObject &json_obj) const override;
  void copy(MMGAction *dest) const override;
  void setEditable(bool edit) override;
  void createDisplay(QWidget *parent) override;
  void setSubOptions(QComboBox *sub) override;

  Category category() const override { return Category::MMGACTION_TRANSITION; }

  static const QStringList enumerate();
  obs_source_t *sourceByName() const;
  bool transitionFixed() const;

  private:
  MMGUtils::MMGString transition;
  MMGUtils::MMGString parent_scene;
  MMGUtils::MMGString source;
  MMGUtils::MMGString json_str;
  MMGUtils::MMGNumber num;

  void setSubConfig() override;
  void setList1Config() override;
  void setList2Config() override;
  void setList3Config() override;
};
