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

class MMGActionFilters : public MMGAction {
  public:
  explicit MMGActionFilters() { blog(LOG_DEBUG, "Empty action created."); };
  explicit MMGActionFilters(const QJsonObject &json_obj);
  enum Actions { FILTER_SHOW, FILTER_HIDE, FILTER_TOGGLE_SHOWHIDE, FILTER_REORDER, FILTER_CUSTOM };

  void blog(int log_status, const QString &message) const override;
  void execute(const MMGMessage *midi) const override;
  void json(QJsonObject &json_obj) const override;
  void copy(MMGAction *dest) const override;
  void setEditable(bool edit) override;
  void createDisplay(QWidget *parent) override;
  void setSubOptions(QComboBox *sub) override;

  Category category() const override { return Category::MMGACTION_FILTER; }

  static const QStringList enumerate(const QString &source = "");
  static const QStringList enumerate_eligible();

  private:
  MMGUtils::MMGString source;
  MMGUtils::MMGString filter;
  MMGUtils::MMGNumber num;
  MMGUtils::MMGString json_str;

  void setSubConfig() override;
  void setList1Config() override;
  void setList2Config() override;
};
