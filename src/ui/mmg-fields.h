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
#include "../mmg-utils.h"
#include "../actions/mmg-action.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QScrollArea>
#include <QLayout>
#include <QToolButton>

class MMGFields;

struct MMGFieldInit {
  MMGFields *group;
  QWidget *parent;
  obs_property_t *prop;
  QJsonObject current_json;
  QJsonObject default_json;
  QJsonObject action_json;
};

struct MMGField {
  const QString &get_name() const { return name; };
  void set_name(const QString &val) { name = val; };
  virtual void json(QJsonObject &json_obj) const = 0;
  virtual void update(obs_property_t *prop) = 0;

  protected:
  QString name;

  QLabel *label;

  MMGFields *parent;
};

struct MMGNumberField : public MMGField {
  MMGNumberField(const MMGFieldInit &init);

  void json(QJsonObject &json_obj) const override { number.json(json_obj, name, true); };
  void update(obs_property_t *prop) override;

  MMGUtils::MMGNumber number;

  QLCDNumber *lcd;
  QToolButton *down_major_button;
  QToolButton *down_minor_button;
  QToolButton *up_minor_button;
  QToolButton *up_major_button;
  QComboBox *combo;
  MMGUtils::LCDData lcd_data;
};

struct MMGStringField : public MMGField {
  MMGStringField(const MMGFieldInit &init);

  virtual void callback(const QString &val);
  void json(QJsonObject &json_obj) const override { value.json(json_obj, name, true); };
  void update(obs_property_t *prop) override;

  MMGUtils::MMGString value;

  QComboBox *combo;
};

struct MMGBooleanField : public MMGStringField {
  MMGBooleanField(const MMGFieldInit &init);

  void callback(const QString &val) override;
  void json(QJsonObject &json_obj) const override
  {
    json_obj[name] = bool_value;
    json_obj[name + "_state"] = value.state();
  };
  void update(obs_property_t *prop) override;

  bool bool_value;
};

struct MMGButtonField : public MMGField {
  MMGButtonField(const MMGFieldInit &init);

  virtual void callback();
  void json(QJsonObject &json_obj) const override{};
  void update(obs_property_t *prop) override;

  QPushButton *button;
};

struct MMGColorField : public MMGButtonField {
  MMGColorField(const MMGFieldInit &init, bool use_alpha);

  void callback() override;
  void json(QJsonObject &json_obj) const override { json_obj[name] = (double)(uint)color.rgba(); };
  void update(obs_property_t *prop) override;

  QColor color;
  bool alpha;

  QFrame *frame;

  private:
  static QString style_sheet;
};

class MMGFields {
  public:
  enum Kind { MMGFIELDS_FILTER, MMGFIELDS_TRANSITION, MMGFIELDS_SOURCE };

  MMGFields(Kind kind, QStackedWidget *parent, MMGAction *action);

  int get_index() const { return index; };
  obs_property_t *get_property(const QString &name)
  {
    return obs_properties_get(props, name.qtocs());
  };
  MMGAction *get_action() const { return action; };
  bool ensure_validity(MMGAction *test) const
  {
    return action == test && action->str1() == source && action->str2() == filter;
  };

  const QString json();
  void update(MMGField *from_field);

  ~MMGFields()
  {
    qDeleteAll(fields);
    obs_properties_destroy(props);
  }

  private:
  QString source;
  QString filter;

  QWidget *widget;
  int index;
  MMGAction *action;
  obs_properties_t *props = nullptr;
  QList<MMGField *> fields;

  void create_filter_fields();
};
