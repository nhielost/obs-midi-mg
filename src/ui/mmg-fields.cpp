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

#include "mmg-fields.h"

#include <obs-frontend-api.h>

#include <QColorDialog>
#include <QFontDialog>
#include <QLineEdit>
#include <QScrollBar>
#include <QDesktopServices>

using namespace MMGUtils;

// MMGOBSNumberField
MMGOBSNumberField::MMGOBSNumberField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSField(init.fields, p)
{
  name = obs_property_name(init.prop);

  num_display = new MMGNumberDisplay(this);
  num_display->setStorage(&number);

  update(init.prop);
  if (init.current_json.contains(name)) {
    number = init.current_json[name].toDouble();
  } else {
    number = init.default_json[name].toDouble();
  }
  num_display->display();

  connect(num_display, &MMGNumberDisplay::numberChanged, parent, &MMGOBSFields::mmg_json);
}

void MMGOBSNumberField::apply(const QJsonObject &json_obj)
{
  num_display->blockSignals(true);
  update(parent->property(name));
  number = MMGNumber(json_obj, name, 0);
  num_display->display();
  num_display->blockSignals(false);
}

void MMGOBSNumberField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  num_display->setEnabled(enabled);

  if (obs_property_get_type(prop) != OBS_PROPERTY_FLOAT &&
      obs_property_get_type(prop) != OBS_PROPERTY_INT)
    return;

  vec3 bounds = get_obs_property_bounds(prop);
  num_display->setBounds(bounds.x, bounds.y);
  num_display->setStep(bounds.z);
  num_display->setDescription(obs_property_description(prop));
}
// End MMGOBSNumber Field

// MMGOBSStringField
MMGOBSStringField::MMGOBSStringField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSField(init.fields, p)
{
  name = obs_property_name(init.prop);

  label = new QLabel(this);
  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setWordWrap(true);
  label->setGeometry(0, 0, 270, 30);

  combo = new QComboBox(this);
  combo->setVisible(true);
  combo->setGeometry(0, 30, 270, 40);
  combo->setEditable(obs_property_list_type(init.prop) == OBS_COMBO_TYPE_EDITABLE);

  if (init.current_json.contains(name)) {
    value = init.current_json[name];
  } else {
    value = init.default_json[name];
  }
  update(init.prop);

  combo->connect(combo, &QComboBox::currentIndexChanged, combo,
		 [&](int index) { callback(combo->itemData(index)); });
}

void MMGOBSStringField::callback(const QVariant &val)
{
  value = val.value<QJsonValue>();
  state = val == "msg_val" ? MMGString::STRINGSTATE_MIDI : MMGString::STRINGSTATE_FIXED;
  parent->obs_json();
  parent->mmg_json();
}

void MMGOBSStringField::apply(const QJsonObject &json_obj)
{
  value = json_obj[name]["value"];
  state = (MMGString::State)json_obj[name]["state"].toInt();
  update(parent->property(name));
}

void MMGOBSStringField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  combo->setEnabled(enabled);

  if (obs_property_get_type(prop) != OBS_PROPERTY_LIST) return;

  QList<MMGPair> prop_options = get_obs_property_options(prop);
  if (options != prop_options) {
    options = prop_options;
    combo->clear();
    for (const MMGPair &key : prop_options) {
      combo->addItem(key.key, key.val);
    }
    if (!prop_options.isEmpty()) combo->addItem(mmgtr("Fields.UseMessageValue"), "msg_val");
    int index = combo->findData(value.toVariant());
    combo->setCurrentIndex(index == -1 ? 0 : index);
  }
}
// End MMGOBSStringField

// MMGOBSBooleanField
MMGOBSBooleanField::MMGOBSBooleanField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSStringField(p, init)
{
  label->setWordWrap(false);

  combo->blockSignals(true);
  combo->clear();
  combo->addItems(mmgtr_all("Fields", {"True", "False", "Toggle", "Ignore"}));
  combo->setItemData(0, true);
  combo->setItemData(1, false);
  combo->blockSignals(false);

  combo->blockSignals(true);
  if (init.current_json.contains(name)) {
    value = init.current_json[name];
    combo->setCurrentIndex(state != 0 ? state : !value.toBool());
  } else {
    value = init.default_json[name];
    combo->setCurrentIndex(state != 0 ? state : !value.toBool());
  }
  combo->blockSignals(false);
}

void MMGOBSBooleanField::callback(const QVariant &val)
{
  value = (combo->currentIndex() > 1 ? QVariant(true) : val).toJsonValue();
  state = (MMGString::State)(combo->currentIndex() > 1 ? combo->currentIndex() : 0);
  parent->obs_json();
  parent->mmg_json();
}

void MMGOBSBooleanField::apply(const QJsonObject &json_obj)
{
  value = json_obj[name]["value"];
  state = (MMGString::State)json_obj[name]["state"].toInt();
  combo->setCurrentIndex(state != 0 ? state : !value.toBool());
}

void MMGOBSBooleanField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  combo->setEnabled(enabled);
}
// End MMGOBSBooleanField

// MMGOBSButtonField
const QString MMGOBSButtonField::style_sheet = "border: 1px solid #ff0000; border-radius: 10px;";

MMGOBSButtonField::MMGOBSButtonField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSField(init.fields, p)
{
  name = obs_property_name(init.prop);

  label = new QLabel(this);
  label->setVisible(false);

  button = new QPushButton(this);
  button->setVisible(true);
  button->setGeometry(15, 15, 240, 40);
  button->setCursor(QCursor(Qt::PointingHandCursor));
  button->setText(obs_property_description(init.prop));
  button->connect(button, &QPushButton::clicked, button, [&]() { callback(); });
}

void MMGOBSButtonField::update(obs_property_t *prop)
{
  button->setEnabled(obs_property_visible(prop) && obs_property_enabled(prop));
}

void MMGOBSButtonField::callback()
{
  obs_property_t *prop = parent->property(name);
  if (obs_property_button_type(prop) == OBS_BUTTON_URL) {
    QUrl url(QString(obs_property_button_url(prop)), QUrl::StrictMode);
    if (!url.isValid() || !url.scheme().contains("http")) return;
    QDesktopServices::openUrl(url);
  } else {
    obs_property_button_clicked(prop, nullptr);
  }
  parent->obs_json();
  parent->mmg_json();
}
// End MMGOBSButtonField

// MMGOBSColorField
MMGOBSColorField::MMGOBSColorField(QWidget *p, const MMGOBSFieldInit &init, bool use_alpha)
  : MMGOBSButtonField(p, init)
{
  alpha = use_alpha;

  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setWordWrap(true);
  label->setGeometry(0, 0, 130, 20);

  button->setGeometry(0, 30, 130, 40);
  button->setText(mmgtr("Fields.ColorSelect"));

  frame = new QFrame(this);
  frame->setVisible(true);
  frame->setGeometry(140, 0, 130, 70);

  if (init.current_json.contains(name)) {
    color = QColor::fromRgba(argb_abgr(init.current_json[name].toInteger(4278190080u)));
  } else {
    color = QColor::fromRgba(argb_abgr(init.default_json[name].toInteger(4278190080u)));
  }
  frame->setStyleSheet(style_sheet + " background-color: " + color.name((QColor::NameFormat)alpha));
}

void MMGOBSColorField::obs_json(QJsonObject &json_obj) const
{
  json_obj[name] = (double)argb_abgr(color.rgba());
}

void MMGOBSColorField::mmg_json(QJsonObject &json_obj) const
{
  QJsonObject color_obj;
  color_obj["color"] = (double)argb_abgr(color.rgba());
  json_obj[name] = color_obj;
}

void MMGOBSColorField::apply(const QJsonObject &json_obj)
{
  color = QColor::fromRgba(argb_abgr(json_obj[name]["color"].toInteger(4278190080u)));
  frame->setStyleSheet(style_sheet + " background-color: " + color.name((QColor::NameFormat)alpha));
}

void MMGOBSColorField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  button->setEnabled(enabled);
  frame->setEnabled(enabled);
}

void MMGOBSColorField::callback()
{
  QColor _color = QColorDialog::getColor(color, nullptr, QString(),
					 alpha ? QColorDialog::ShowAlphaChannel
					       : (QColorDialog::ColorDialogOption)0);
  color = _color.isValid() ? _color : color;
  frame->setStyleSheet(style_sheet + " background-color: " + color.name((QColor::NameFormat)alpha));
  parent->mmg_json();
}
// End MMGOBSColorField

// MMGOBSFontField
MMGOBSFontField::MMGOBSFontField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSButtonField(p, init)
{
  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setWordWrap(true);
  label->setGeometry(0, 0, 130, 20);

  button->setGeometry(0, 30, 130, 40);
  button->setText(mmgtr("Fields.FontSelect"));

  QFrame *frame = new QFrame(this);
  frame->setVisible(true);
  frame->setGeometry(140, 0, 130, 70);
  frame->setStyleSheet(style_sheet);

  inner_label = new QLabel(frame);
  inner_label->setVisible(true);
  inner_label->setGeometry(0, 0, 130, 70);
  inner_label->setAlignment(Qt::AlignCenter);
  inner_label->setText(font.family() + "\n" + QString::number(font.pointSize()) + "pt");
  inner_label->setWordWrap(true);

  if (init.current_json.contains(name)) {
    apply(init.current_json);
  } else {
    apply(init.default_json);
  }
}

void MMGOBSFontField::obs_json(QJsonObject &json_obj) const
{
  QJsonObject font_json;
  font_json["face"] = font.family();
  font_json["flags"] = (font.strikeOut() << 3) | (font.underline() << 2) | (font.italic() << 1) |
		       (font.bold() << 0);
  font_json["size"] = font.pointSize();
  font_json["style"] = font.styleName();
  json_obj[name] = font_json;
}

void MMGOBSFontField::apply(const QJsonObject &json_obj)
{
  QJsonObject font_obj = json_obj[name].toObject();
  font.setFamily(font_obj["face"].toString());
  int flags = font_obj["flags"].toInt();
  font.setBold(flags & 0b0001);
  font.setItalic(flags & 0b0010);
  font.setUnderline(flags & 0b0100);
  font.setStrikeOut(flags & 0b1000);
  font.setPointSize(font_obj["size"].toInt());
  font.setStyleName(font_obj["style"].toString());
  inner_label->setText(font.family() + "\n" + QString::number(font.pointSize()) + "pt");
}

void MMGOBSFontField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  button->setEnabled(enabled);
  inner_label->setEnabled(enabled);
}

void MMGOBSFontField::callback()
{
  bool accept;
  QFont _font = QFontDialog::getFont(&accept, font, nullptr, mmgtr("Fields.FontSelect"),
				     QFontDialog::DontUseNativeDialog);
  if (accept) font = _font;
  inner_label->setText(font.family() + "\n" + QString::number(font.pointSize()) + "pt");
  parent->mmg_json();
}
// End MMGOBSFontField

// MMGOBSGroupField
MMGOBSGroupField::MMGOBSGroupField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSField(init.fields, p)
{
  name = obs_property_name(init.prop);

  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(15);
  layout->setSizeConstraint(QLayout::SetFixedSize);
  layout->setContentsMargins(0, 0, 0, 25);
  setLayout(layout);

  if (obs_property_group_type(init.prop) == OBS_GROUP_CHECKABLE) {
    boolean_field = new MMGOBSBooleanField(p, init);
    boolean_field->setStyleSheet("QLabel {font-weight: bold;}");
    parent->layout()->addWidget(boolean_field);
  } else if (obs_property_group_type(init.prop) == OBS_GROUP_NORMAL) {
    label = new QLabel(this);
    label->setVisible(true);
    label->setStyleSheet("QLabel {font-weight: bold;}");
    label->setText(obs_property_description(init.prop));
    layout->addWidget(label);
  }

  parent->create(this, obs_property_group_content(init.prop), init);
}

void MMGOBSGroupField::obs_json(QJsonObject &json_obj) const
{
  if (boolean_field) boolean_field->obs_json(json_obj);
}

void MMGOBSGroupField::mmg_json(QJsonObject &json_obj) const
{
  if (boolean_field) boolean_field->mmg_json(json_obj);
}

void MMGOBSGroupField::update(obs_property_t *prop)
{
  if (boolean_field) {
    boolean_field->update(prop);
    boolean_field_state = boolean_field->value.toBool(true);
    setEnabled(boolean_field_state);
  }
}
// End MMGOBSGroupField

// MMGOBSTextField
MMGOBSTextField::MMGOBSTextField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSField(init.fields, p)
{
  name = obs_property_name(init.prop);

  obs_text_type prop_type = obs_property_text_type(init.prop);

  label = new QLabel(this);
  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setGeometry(0, 0, 270, 20);

  text_edit = new QTextEdit(this);
  text_edit->setGeometry(0, 20, 270, 50);
  text_edit->setVisible(prop_type == OBS_TEXT_MULTILINE);

  line_edit = new QLineEdit(this);
  line_edit->setGeometry(0, 20, 270, 50);
  line_edit->setVisible(prop_type != OBS_TEXT_MULTILINE);
  line_edit->setEchoMode((QLineEdit::EchoMode)(prop_type == OBS_TEXT_PASSWORD ? 3 : 0));

  if (init.current_json.contains(name)) {
    text = init.current_json[name].toString();
  } else {
    text = init.default_json[name].toString();
  }
  text.set_state((MMGString::State)checkMIDI());
  update(parent->property(name));

  text_edit->connect(text_edit, &QTextEdit::textChanged, text_edit,
		     [&]() { callback(text_edit->toPlainText()); });
  line_edit->connect(line_edit, &QLineEdit::textChanged, line_edit,
		     [&]() { callback(line_edit->text()); });
}

void MMGOBSTextField::callback(const QString &str)
{
  text = str;
  text.set_state((MMGString::State)checkMIDI());
  parent->mmg_json();
}

void MMGOBSTextField::apply(const QJsonObject &json_obj)
{
  text = json_obj[name]["string"].toString();
  text.set_state((MMGString::State)checkMIDI());

  update(parent->property(name));
}

void MMGOBSTextField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  text_edit->setEnabled(enabled);
  line_edit->setEnabled(enabled);

  text_edit->setPlainText(text);
  line_edit->setText(text);
}

bool MMGOBSTextField::checkMIDI() const
{
  return text.str().contains("${type}") || text.str().contains("${channel}") ||
	 text.str().contains("${note}") || text.str().contains("${value}") ||
	 text.str().contains("${control}");
}
// End MMGOBSTextField

// MMGOBSPathField
MMGOBSPathField::MMGOBSPathField(QWidget *p, const MMGOBSFieldInit &init)
  : MMGOBSButtonField(p, init)
{
  label->setVisible(true);
  label->setText(obs_property_description(init.prop));
  label->setGeometry(0, 0, 130, 20);

  button->setGeometry(0, 30, 130, 40);

  QFrame *frame = new QFrame(this);
  frame->setVisible(true);
  frame->setGeometry(140, 0, 130, 70);
  frame->setStyleSheet(style_sheet);

  path_display = new QLabel(frame);
  path_display->setVisible(true);
  path_display->setGeometry(0, 0, 130, 70);
  path_display->setAlignment(Qt::AlignBottom);
  path_display->setWordWrap(true);

  if (init.current_json.contains(name)) {
    path = init.current_json[name].toString();
  } else {
    path = init.default_json[name].toString();
  }
  update(parent->property(name));
}

void MMGOBSPathField::apply(const QJsonObject &json_obj)
{
  path = json_obj[name]["string"].toString();
  update(parent->property(name));
}

void MMGOBSPathField::update(obs_property_t *prop)
{
  bool enabled = obs_property_visible(prop) && obs_property_enabled(prop);
  label->setEnabled(enabled);
  button->setEnabled(enabled);
  path_display->setEnabled(enabled);

  if (obs_property_get_type(prop) != OBS_PROPERTY_PATH) return;

  default_path = obs_property_path_default_path(prop);
  dialog_type = obs_property_path_type(prop);
  filters = obs_property_path_filter(prop);

  path_display->setText(path);
  button->setText(dialog_type < 2 ? mmgtr("Fields.FileSelect") : mmgtr("Fields.FolderSelect"));
}

void MMGOBSPathField::callback()
{
  QString new_path;
  switch (dialog_type) {
    case 0: // FILE READING
      new_path = QFileDialog::getOpenFileName(nullptr, mmgtr("Fields.FileSelect"), default_path,
					      filters, nullptr,
					      QFileDialog::Option::HideNameFilterDetails);
      break;
    case 1: // FILE WRITING
      new_path =
	QFileDialog::getSaveFileName(nullptr, mmgtr("Fields.FileSelect"), default_path, filters);
      break;
    case 2: // DIRECTORY
      new_path =
	QFileDialog::getExistingDirectory(nullptr, mmgtr("Fields.FolderSelect"), default_path);
      break;
    default:
      break;
  }
  if (!new_path.isNull()) {
    path = new_path;
    update(parent->property(name));
    parent->mmg_json();
  }
}
// End MMGOBSPathField

// MMGOBSFields
MMGOBSFields::MMGOBSFields(QWidget *parent, obs_source_t *source) : QWidget(parent)
{
  // Qt Setup
  setGeometry(0, 0, 290, 350);
  QVBoxLayout *custom_field_layout = new QVBoxLayout(this);
  custom_field_layout->setSpacing(10);
  custom_field_layout->setSizeConstraint(QLayout::SetFixedSize);
  custom_field_layout->setContentsMargins(10, 10, 10, 10);
  setLayout(custom_field_layout);

  // The Good Stuff
  if (!source) {
    open_message_box(mmgtr("UI.MessageBox.FieldsError.Title"),
		     mmgtr("UI.MessageBox.FieldsError.BadSource"));
    return;
  };

  _source = obs_source_get_ref(source);

  OBSDataAutoRelease source_data = obs_source_get_settings(source);
  OBSDataAutoRelease source_defaults = obs_data_get_defaults(source_data);
  QJsonObject source_json = json_from_str(obs_data_get_json(source_data));
  QJsonObject defaults_json = json_from_str(obs_data_get_json(source_defaults));

  props = obs_source_properties(source);

  MMGOBSFieldInit init{this, nullptr, source_json, defaults_json};
  create(this, props, init);

  // Finish
  obs_json();
};

void MMGOBSFields::add(QWidget *parent, MMGOBSField *field)
{
  fields.append(field);
  parent->layout()->addWidget(field);
}

void MMGOBSFields::create(QWidget *parent, obs_properties_t *props,
			  const MMGOBSFieldInit &json_init)
{
  if (!props) return;

  obs_property_t *prop = obs_properties_first(props);

  MMGOBSFieldInit init{this, prop, json_init.current_json, json_init.default_json};

  do {
    init.prop = prop;

    switch (obs_property_get_type(prop)) {
      case OBS_PROPERTY_FLOAT:
      case OBS_PROPERTY_INT:
	add(parent, new MMGOBSNumberField(parent, init));
	break;
      case OBS_PROPERTY_LIST:
	add(parent, new MMGOBSStringField(parent, init));
	break;
      case OBS_PROPERTY_BOOL:
	add(parent, new MMGOBSBooleanField(parent, init));
	break;
      case OBS_PROPERTY_BUTTON:
	add(parent, new MMGOBSButtonField(parent, init));
	break;
      case OBS_PROPERTY_COLOR:
	add(parent, new MMGOBSColorField(parent, init, false));
	break;
      case OBS_PROPERTY_COLOR_ALPHA:
	add(parent, new MMGOBSColorField(parent, init, true));
	break;
      case OBS_PROPERTY_FONT:
	add(parent, new MMGOBSFontField(parent, init));
	break;
      case OBS_PROPERTY_GROUP:
	add(parent, new MMGOBSGroupField(parent, init));
	continue;
      case OBS_PROPERTY_PATH:
	add(parent, new MMGOBSPathField(parent, init));
	break;
      case OBS_PROPERTY_TEXT:
	if (obs_property_text_type(prop) != OBS_TEXT_INFO) {
	  add(parent, new MMGOBSTextField(parent, init));
	  break;
	}
	[[fallthrough]];
      case OBS_PROPERTY_EDITABLE_LIST:
      case OBS_PROPERTY_FRAME_RATE:
      default:
	continue;
    }
  } while (obs_property_next(&prop) != 0);
}

void MMGOBSFields::obs_json()
{
  QJsonObject json_obj;
  for (MMGOBSField *field : fields) {
    field->obs_json(json_obj);
  }
  OBSDataAutoRelease data = obs_data_create_from_json(json_to_str(json_obj));
  obs_properties_apply_settings(props, data);
  for (MMGOBSField *field : fields) {
    field->update(property(field->get_name()));
  }
}

void MMGOBSFields::mmg_json()
{
  if (!json_des) return;

  QJsonObject json_obj;
  for (MMGOBSField *field : fields) {
    field->mmg_json(json_obj);
  }
  json_des->set_str(json_to_str(json_obj));
}

void MMGOBSFields::setJsonDestination(MMGString *json, bool force)
{
  if (json_des == json) return;
  json_des = json;

  if (force) {
    if (!json->str().isEmpty()) apply();
  } else {
    mmg_json();
  }
};

void MMGOBSFields::apply()
{
  QJsonObject json_obj = json_from_str(json_des->mmgtocs());
  for (MMGOBSField *field : fields) {
    field->apply(json_obj);
  }
}
// End MMGOBSFields
