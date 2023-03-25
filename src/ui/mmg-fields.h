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

#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QScrollArea>
#include <QLayout>
#include <QToolButton>
#include <QFont>
#include <QTextEdit>
#include <QScopedPointer>
#include <QFileDialog>

class MMGOBSFields;

struct MMGOBSFieldInit {
  MMGOBSFields *fields;
  obs_property_t *prop;
  QJsonObject current_json;
  QJsonObject default_json;
};

struct MMGOBSField : public QWidget {
  Q_OBJECT

  public:
  virtual ~MMGOBSField() = default;

  const QString &get_name() const { return name; };
  void set_name(const QString &val) { name = val; };
  virtual void obs_json(QJsonObject &json_obj) const = 0;
  virtual void mmg_json(QJsonObject &json_obj) const = 0;
  virtual void apply(const QJsonObject &json_obj) = 0;
  virtual void update(obs_property_t *prop) = 0;

  protected:
  QString name;

  QLabel *label;

  MMGOBSFields *parent;

  MMGOBSField(MMGOBSFields *fields, QWidget *widget = nullptr) : QWidget(widget)
  {
    parent = fields;
    setFixedSize(270, 70);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  };
};
using MMGOBSFieldList = QList<MMGOBSField *>;

struct MMGOBSNumberField : public MMGOBSField {
  MMGOBSNumberField(QWidget *parent, const MMGOBSFieldInit &init);

  void obs_json(QJsonObject &json_obj) const override { json_obj[name] = number.num(); };
  void mmg_json(QJsonObject &json_obj) const override { number.json(json_obj, name); };
  void apply(const QJsonObject &json_obj) override;
  void update(obs_property_t *prop) override;

  MMGUtils::MMGNumber number;

  MMGNumberDisplay *num_display;
};

struct MMGOBSStringField : public MMGOBSField {
  MMGOBSStringField(QWidget *parent, const MMGOBSFieldInit &init);

  virtual void callback(const QVariant &val);
  void obs_json(QJsonObject &json_obj) const override { json_obj[name] = value; };
  void mmg_json(QJsonObject &json_obj) const override
  {
    QJsonObject value_obj;
    value_obj["value"] = value;
    value_obj["state"] = state;
    json_obj[name] = value_obj;
  };
  virtual void apply(const QJsonObject &json_obj) override;
  void update(obs_property_t *prop) override;

  QJsonValue value;
  MMGUtils::MMGString::State state = MMGUtils::MMGString::STRINGSTATE_FIXED;

  QList<MMGUtils::MMGPair> options;

  QComboBox *combo;
};

struct MMGOBSBooleanField : public MMGOBSStringField {
  MMGOBSBooleanField(QWidget *parent, const MMGOBSFieldInit &init);

  void callback(const QVariant &val) override;
  void apply(const QJsonObject &json_obj) override;
  void update(obs_property_t *prop) override;
};

struct MMGOBSButtonField : public MMGOBSField {
  MMGOBSButtonField(QWidget *parent, const MMGOBSFieldInit &init);

  virtual void callback();
  void obs_json(QJsonObject &json_obj) const override { Q_UNUSED(json_obj); };
  void mmg_json(QJsonObject &json_obj) const override { Q_UNUSED(json_obj); };
  virtual void apply(const QJsonObject &json_obj) override { Q_UNUSED(json_obj); };
  void update(obs_property_t *prop) override;

  QPushButton *button;

  static const QString style_sheet;
};

struct MMGOBSColorField : public MMGOBSButtonField {
  MMGOBSColorField(QWidget *parent, const MMGOBSFieldInit &init, bool use_alpha);

  void callback() override;
  void obs_json(QJsonObject &json_obj) const override;
  void mmg_json(QJsonObject &json_obj) const override;
  void apply(const QJsonObject &json_obj) override;
  void update(obs_property_t *prop) override;

  QColor color;
  bool alpha;

  QFrame *frame;
};

struct MMGOBSFontField : public MMGOBSButtonField {
  MMGOBSFontField(QWidget *parent, const MMGOBSFieldInit &init);

  void callback() override;
  void obs_json(QJsonObject &json_obj) const override;
  void mmg_json(QJsonObject &json_obj) const override { obs_json(json_obj); };
  void apply(const QJsonObject &json_obj) override;
  void update(obs_property_t *prop) override;

  QFont font;

  QLabel *inner_label;
};

struct MMGOBSGroupField : public MMGOBSField {
  MMGOBSGroupField(QWidget *parent, const MMGOBSFieldInit &init);

  void obs_json(QJsonObject &json_obj) const override;
  void mmg_json(QJsonObject &json_obj) const override;
  void apply(const QJsonObject &json_obj) override { Q_UNUSED(json_obj); };
  void update(obs_property_t *prop) override;

  MMGOBSBooleanField *boolean_field = nullptr;
  bool boolean_field_state;

  ~MMGOBSGroupField()
  {
    if (boolean_field) delete boolean_field;
  }
};

struct MMGOBSPathField : public MMGOBSButtonField {
  MMGOBSPathField(QWidget *parent, const MMGOBSFieldInit &init);

  void callback() override;
  void obs_json(QJsonObject &json_obj) const override { json_obj[name] = path.str(); };
  void mmg_json(QJsonObject &json_obj) const override { path.json(json_obj, name); };
  void apply(const QJsonObject &json_obj) override;
  void update(obs_property_t *prop) override;

  MMGUtils::MMGString path;

  short dialog_type;
  QString filters;
  QString default_path;

  QLabel *path_display;
};

struct MMGOBSTextField : public MMGOBSField {
  MMGOBSTextField(QWidget *parent, const MMGOBSFieldInit &init);

  virtual void callback(const QString &str);
  void obs_json(QJsonObject &json_obj) const override { json_obj[name] = text.str(); };
  void mmg_json(QJsonObject &json_obj) const override { text.json(json_obj, name); };
  void apply(const QJsonObject &json_obj) override;
  void update(obs_property_t *prop) override;

  bool checkMIDI() const;

  MMGUtils::MMGString text;

  QTextEdit *text_edit;
  QLineEdit *line_edit;
};

class MMGOBSFields : public QWidget {
  Q_OBJECT

  public:
  MMGOBSFields(QWidget *parent, obs_source_t *source);

  bool match(obs_source_t *source) const { return _source == source; };
  MMGUtils::MMGString *jsonDestination() const { return json_des; };
  void setJsonDestination(MMGUtils::MMGString *json, bool force);

  obs_property_t *property(const QString &name) { return obs_properties_get(props, name.qtocs()); };
  void create(QWidget *parent, obs_properties_t *props, const MMGOBSFieldInit &json_init);
  void add(QWidget *parent, MMGOBSField *field);

  ~MMGOBSFields()
  {
    obs_source_release(_source);
    obs_properties_destroy(props);
  }

  public slots:
  void obs_json();
  void mmg_json();

  private:
  obs_source_t *_source;
  MMGUtils::MMGString *json_des = nullptr;
  obs_properties_t *props = nullptr;
  MMGOBSFieldList fields;

  void apply();
};
