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

#include "mmg-fields.h"

#include <obs-frontend-api.h>

#include <QColorDialog>
#include <QScrollBar>

using namespace MMGUtils;

// MMGNumberField
MMGNumberField::MMGNumberField(const MMGFieldInit &init)
  : lcd(new QLCDNumber(init.parent)), lcd_data(lcd)
{
  name = obs_property_name(init.prop);
  parent = init.group;

  lcd_data.set_storage(&number);
  vec3 bounds = get_obs_filter_property_bounds(init.prop);
  lcd_data.set_range(bounds.x, bounds.y);
  lcd_data.set_step(bounds.z < 0.01 ? 0.01 : bounds.z, bounds.z < 0.01 ? 0.1 : bounds.z * 10);
  lcd_data.set_default_value(init.default_json[name].toDouble());
  if (json_is_valid(init.action_json[name], QJsonValue::Double)) {
    number = init.action_json[name].toDouble();
  } else if (json_is_valid(init.current_json[name], QJsonValue::Double)) {
    number = init.current_json[name].toDouble();
  } else {
    lcd_data.reset();
  }
  lcd_data.display();
  number.set_state((MMGNumber::State)init.action_json[name + "_state"].toInt());

  label = new QLabel(init.parent);
  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setGeometry(10, 10, 110, 30);

  lcd->setDigitCount(8);
  lcd->setSmallDecimalPoint(true);
  QPalette p = init.parent->palette();
  p.setColor(QPalette::Dark, QColor("#ffffff"));
  lcd->setPalette(p);
  lcd->setSegmentStyle(QLCDNumber::Filled);
  lcd->setFrameShape(QFrame::NoFrame);
  lcd->setFrameShadow(QFrame::Raised);
  lcd->setLineWidth(0);
  lcd->setVisible(true);
  lcd->setGeometry(120, 10, 160, 30);

  down_major_button = new QToolButton(init.parent);
  down_major_button->setVisible(true);
  down_major_button->setText("◂◂");
  down_major_button->setAutoRepeat(true);
  down_major_button->setAutoRepeatDelay(500);
  down_major_button->setAutoRepeatInterval(50);
  down_major_button->setGeometry(10, 50, 40, 30);
  down_major_button->connect(down_major_button, &QAbstractButton::clicked, down_major_button,
			     [&]() {
			       lcd_data.down_major();
			       parent->update(this);
			     });

  down_minor_button = new QToolButton(init.parent);
  down_minor_button->setVisible(true);
  down_minor_button->setText("◂");
  down_minor_button->setAutoRepeat(true);
  down_minor_button->setAutoRepeatDelay(500);
  down_minor_button->setAutoRepeatInterval(50);
  down_minor_button->setGeometry(50, 50, 40, 30);
  down_minor_button->connect(down_minor_button, &QAbstractButton::clicked, down_minor_button,
			     [&]() {
			       lcd_data.down_minor();
			       parent->update(this);
			     });

  up_minor_button = new QToolButton(init.parent);
  up_minor_button->setVisible(true);
  up_minor_button->setText("▸");
  up_minor_button->setAutoRepeat(true);
  up_minor_button->setAutoRepeatDelay(500);
  up_minor_button->setAutoRepeatInterval(50);
  up_minor_button->setGeometry(200, 50, 40, 30);
  up_minor_button->connect(up_minor_button, &QAbstractButton::clicked, up_minor_button, [&]() {
    lcd_data.up_minor();
    parent->update(this);
  });

  up_major_button = new QToolButton(init.parent);
  up_major_button->setVisible(true);
  up_major_button->setText("▸▸");
  up_major_button->setAutoRepeat(true);
  up_major_button->setAutoRepeatDelay(500);
  up_major_button->setAutoRepeatInterval(50);
  up_major_button->setGeometry(240, 50, 40, 30);
  up_major_button->connect(up_major_button, &QAbstractButton::clicked, up_major_button, [&]() {
    lcd_data.up_major();
    parent->update(this);
  });

  combo = new QComboBox(init.parent);
  combo->setVisible(true);
  combo->addItems({"Fixed", "0-127", "127-0", "Ignore"});
  combo->setCurrentIndex(number.state());
  combo->setGeometry(92, 47, 106, 36);
  combo->connect(combo, &QComboBox::currentIndexChanged, combo, [&](int index) {
    lcd->setEnabled(index == 0);
    number.set_state((MMGNumber::State)index);
    parent->update(this);
  });
}

void MMGNumberField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop) && number.state() == 0;
  label->setEnabled(enabled);
  lcd->setEnabled(enabled);
  lcd_data.display();
  down_major_button->setEnabled(enabled);
  down_minor_button->setEnabled(enabled);
  up_minor_button->setEnabled(enabled);
  up_major_button->setEnabled(enabled);
  combo->setEnabled(obs_property_visible(prop) && obs_property_enabled(prop));
}
// End MMGNumber Field

// MMGStringField
MMGStringField::MMGStringField(const MMGFieldInit &init)
{
  name = obs_property_name(init.prop);
  if (json_is_valid(init.action_json[name], QJsonValue::String)) {
    value = init.action_json[name].toString();
  } else if (json_is_valid(init.current_json[name], QJsonValue::String)) {
    value = init.current_json[name].toString();
  } else {
    value = init.default_json[name].toString();
  }
  parent = init.group;

  label = new QLabel(init.parent);
  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setGeometry(10, 10, 270, 20);

  combo = new QComboBox(init.parent);
  combo->setVisible(true);
  QHash<QString, QString> options = get_obs_filter_property_options(init.prop);
  for (const QString &key : options.keys()) {
    combo->addItem(key, options[key]);
  }
  combo->addItem("Use Message Value", "msg_val");
  combo->setGeometry(10, 30, 270, 40);
  combo->setCurrentIndex(combo->findData(value.str()));
  combo->connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), combo,
		 [&](int index) { callback(combo->itemData(index).value<QString>()); });
}

void MMGStringField::callback(const QString &val)
{
  value = val;
  value.set_state(val == "msg_val" ? MMGString::STRINGSTATE_MIDI : MMGString::STRINGSTATE_FIXED);
  parent->update(this);
}

void MMGStringField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  combo->setEnabled(enabled);
}
// End MMGStringField

// MMGBooleanField
MMGBooleanField::MMGBooleanField(const MMGFieldInit &init) : MMGStringField(init)
{
  combo->clear();
  combo->addItems({"True", "False", "Toggle", "Ignore"});
  combo->setItemData(0, "True");

  if (json_is_valid(init.action_json[name], QJsonValue::Bool)) {
    bool_value = init.action_json[name].toBool();
  } else if (json_is_valid(init.current_json[name], QJsonValue::Bool)) {
    bool_value = init.current_json[name].toBool();
  } else {
    bool_value = init.default_json[name].toBool();
  }
  value.set_state((MMGString::State)init.action_json[name + "_state"].toInt());

  combo->setCurrentIndex(value.state() != 0 ? value.state() : !bool_value);
}

void MMGBooleanField::callback(const QString &val)
{
  bool_value = combo->currentIndex() > 1 ? true : bool_from_str(val);
  value.set_state((MMGString::State)(combo->currentIndex() > 1 ? combo->currentIndex() : 0));
  parent->update(this);
}

void MMGBooleanField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  combo->setEnabled(enabled);
}
// End MMGBooleanField

// MMGButtonField
MMGButtonField::MMGButtonField(const MMGFieldInit &init)
{
  name = obs_property_name(init.prop);
  parent = init.group;

  label = new QLabel(init.parent);
  label->setVisible(false);

  button = new QPushButton(init.parent);
  button->setVisible(true);
  button->setGeometry(25, 25, 240, 40);
  button->setCursor(QCursor(Qt::PointingHandCursor));
  button->setText(obs_property_description(init.prop));
  button->connect(button, &QPushButton::clicked, button, [&]() { callback(); });
}

void MMGButtonField::update(obs_property_t *prop)
{
  button->setEnabled(obs_property_visible(prop) && obs_property_enabled(prop));
}

void MMGButtonField::callback()
{
  obs_property_t *prop = parent->get_property(name);
  obs_property_button_clicked(prop, nullptr);
  parent->update(this);
}
// End MMGButtonField

// MMGColorField
QString MMGColorField::style_sheet =
  "border: 1px solid #ff0000; border-radius: 10px; background-color: ";

MMGColorField::MMGColorField(const MMGFieldInit &init, bool use_alpha) : MMGButtonField(init)
{
  alpha = use_alpha;

  if (json_is_valid(init.action_json[name], QJsonValue::Double)) {
    color = QColor::fromRgba((quint32)init.action_json[name].toInteger());
  } else if (json_is_valid(init.current_json[name], QJsonValue::Double)) {
    qDebug("%d", init.current_json[name].toInteger());
    color = QColor::fromRgba((quint32)init.current_json[name].toInteger());
  } else {
    color = QColor::fromRgba((quint32)init.default_json[name].toInteger(4278190080u));
  }

  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setGeometry(10, 10, 130, 20);

  button->setGeometry(10, 40, 130, 40);
  button->setText("Select Color...");

  frame = new QFrame(init.parent);
  frame->setVisible(true);
  frame->setGeometry(150, 10, 130, 70);
  frame->setStyleSheet(style_sheet + color.name(alpha ? QColor::HexArgb : QColor::HexRgb));
}

void MMGColorField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  button->setEnabled(enabled);
  frame->setEnabled(enabled);
}

void MMGColorField::callback()
{
  QColor _color =
    alpha ? QColorDialog::getColor(color, nullptr, QString(), QColorDialog::ShowAlphaChannel)
	  : QColorDialog::getColor(color);
  color = _color.isValid() ? _color : color;
  frame->setStyleSheet(style_sheet + color.name(alpha ? QColor::HexArgb : QColor::HexRgb));
  parent->update(this);
}
// End MMGColorField

// MMGFields
MMGFields::MMGFields(Kind kind, QStackedWidget *parent, MMGAction *fields_action)
{
  index = parent->count();
  action = fields_action;
  source = action->str1();
  filter = action->str2();

  QScrollBar *scrollbar = new QScrollBar(parent->window());

  QScrollArea *scroll_area = new QScrollArea(parent);
  scroll_area->setGeometry(QRect(0, 0, 291, 360));
  scroll_area->setFrameShape(QFrame::NoFrame);
  scroll_area->setVerticalScrollBar(scrollbar);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setWidgetResizable(true);

  widget = new QWidget(scroll_area);
  widget->setGeometry(QRect(0, 0, 290, 360));
  QVBoxLayout *custom_field_layout = new QVBoxLayout(widget);
  custom_field_layout->setSpacing(0);
  custom_field_layout->setSizeConstraint(QLayout::SetFixedSize);
  custom_field_layout->setContentsMargins(0, 0, 0, 0);
  widget->setLayout(custom_field_layout);
  scroll_area->setWidget(widget);

  parent->addWidget(scroll_area);

  switch (kind) {
    case Kind::MMGFIELDS_FILTER:
      create_filter_fields();
      break;
    default:
      break;
  }

  update(nullptr);
};

const QString MMGFields::json()
{
  QJsonObject json_obj;
  for (MMGField *field : fields) {
    QString name = field->get_name();
    bool exists = !json_obj[name].isNull();

    if (exists) field->set_name(name + "_");
    field->json(json_obj);
    if (exists) field->set_name(name);
  }
  action->str3() = json_to_str(json_obj);
  return action->str3();
}

void MMGFields::update(MMGField *from_field)
{
  QJsonObject json_obj = json_from_str(json().qtocs());
  for (const QString &key : json_obj.keys()) {
    if (key.endsWith("_state_")) json_obj[key.chopped(1)] = json_obj[key];
  }
  OBSDataAutoRelease data = obs_data_create_from_json(json_to_str(json_obj));
  for (MMGField *field : fields) {
    obs_property_t *prop = get_property(field->get_name());
    if (field == from_field) obs_property_modified(prop, data);
    field->update(prop);
  }
}

void MMGFields::create_filter_fields()
{
  OBSSourceAutoRelease obs_source = obs_get_source_by_name(action->str1().mmgtocs());
  OBSSourceAutoRelease obs_filter =
    obs_source_get_filter_by_name(obs_source, action->str2().mmgtocs());
  OBSDataAutoRelease filter_data = obs_source_get_settings(obs_filter);
  OBSDataAutoRelease filter_defaults = obs_data_get_defaults(filter_data);
  QJsonObject filter_json = json_from_str(obs_data_get_json(filter_data));
  QJsonObject action_json = json_from_str(action->str3().mmgtocs());
  QJsonObject defaults_json = json_from_str(obs_data_get_json(filter_defaults));

  props = obs_source_properties(obs_filter);

  obs_property_t *prop = obs_properties_first(props);
  do {
    QWidget *middle_widget = new QWidget;
    middle_widget->setFixedSize(290, 90);
    middle_widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    MMGFieldInit init{this, middle_widget, prop, filter_json, defaults_json, action_json};
    switch (obs_property_get_type(prop)) {
      case OBS_PROPERTY_FLOAT:
      case OBS_PROPERTY_INT:
	fields.append(new MMGNumberField(init));
	break;
      case OBS_PROPERTY_LIST:
	fields.append(new MMGStringField(init));
	break;
      case OBS_PROPERTY_BOOL:
	fields.append(new MMGBooleanField(init));
	break;
      case OBS_PROPERTY_BUTTON:
	fields.append(new MMGButtonField(init));
	break;
      case OBS_PROPERTY_COLOR:
	fields.append(new MMGColorField(init, false));
	break;
      case OBS_PROPERTY_COLOR_ALPHA:
	fields.append(new MMGColorField(init, true));
	break;
      case OBS_PROPERTY_EDITABLE_LIST:
      case OBS_PROPERTY_FONT:
      case OBS_PROPERTY_GROUP:
      case OBS_PROPERTY_PATH:
      case OBS_PROPERTY_TEXT:
      default:
	delete middle_widget;
	continue;
    }
    widget->layout()->addWidget(middle_widget);
  } while (obs_property_next(&prop) != 0);
}
// End MMGFields
