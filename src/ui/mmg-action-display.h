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
#include "../mmg-utils.h"
#include "mmg-number-display.h"

class MMGBinding;
class MMGOBSFields;

class MMGActionDisplay : public QWidget {
  Q_OBJECT

  public:
  MMGActionDisplay(QWidget *parent);

  void setStr1Visible(bool visible);
  void setStr2Visible(bool visible);
  void setStr3Visible(bool visible);
  void setStr1Description(const QString &desc) { label_str1->setText(desc); };
  void setStr2Description(const QString &desc) { label_str2->setText(desc); };
  void setStr3Description(const QString &desc) { label_str3->setText(desc); };
  void setStr1Options(const QStringList &options);
  void setStr2Options(const QStringList &options);
  void setStr3Options(const QStringList &options);
  void setStr1Storage(MMGUtils::MMGString *storage) { str1 = storage; };
  void setStr2Storage(MMGUtils::MMGString *storage) { str2 = storage; };
  void setStr3Storage(MMGUtils::MMGString *storage) { str3 = storage; };

  QWidget *scrollWidget() const { return current_scroll_widget; };
  MMGNumberDisplayFields *numberDisplays() const { return default_scroll_widget; };
  void setScrollWidget(QWidget *widget);
  void resetScrollWidget() { setScrollWidget(default_scroll_widget); };

  const MMGBinding *parentBinding() const { return parent_binding; };
  void setParentBinding(MMGBinding *binding) { parent_binding = binding; };

  signals:
  void str1Changed();
  void str2Changed();
  void str3Changed();

  void customFieldRequest(void *, MMGUtils::MMGString *);

  private slots:
  void setStr1(const QString &);
  void setStr2(const QString &);
  void setStr3(const QString &);

  protected:
  MMGUtils::MMGString *str1 = nullptr;
  MMGUtils::MMGString *str2 = nullptr;
  MMGUtils::MMGString *str3 = nullptr;

  QLabel *label_str1;
  QLabel *label_str2;
  QLabel *label_str3;
  QComboBox *editor_str1;
  QComboBox *editor_str2;
  QComboBox *editor_str3;

  QScrollArea *scroll_area;
  MMGNumberDisplayFields *default_scroll_widget;
  QWidget *current_scroll_widget;

  MMGBinding *parent_binding;
};
