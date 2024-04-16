/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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
#include "mmg-action-display.h"
#include "../mmg-message.h"

#include <mutex>

#include <QColorDialog>
#include <QFontDialog>
#include <QLineEdit>
#include <QScrollBar>
#include <QDesktopServices>

using namespace MMGUtils;

static std::mutex custom_update;

// MMGOBSField
const QString MMGOBSField::frame_style = "border: 1px solid #ff0000; border-radius: 8px;";

MMGOBSField::MMGOBSField(QWidget *widget) : QWidget(widget)
{
	setFixedSize(330, 90);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	label = new QLabel(this);
	label->setVisible(false);
}

const QSignalBlocker MMGOBSField::updateAndBlock(obs_property_t *prop)
{
	setEnabled(obs_property_visible(prop) && obs_property_enabled(prop));
	return QSignalBlocker(this);
}
// End MMGOBSField

// MMGOBSNumberField
MMGOBSNumberField::MMGOBSNumberField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);
	number.setState(STATE_IGNORE);

	num_display = new MMGNumberDisplay(this);
	num_display->move(0, 10);
	num_display->setOptions(MIDIBUTTON_FIXED | MIDIBUTTON_CUSTOM | MIDIBUTTON_IGNORE | MIDIBUTTON_TOGGLE);
	num_display->setStorage(&number);

	refresh(init);

	connect(num_display, &MMGNumberDisplay::numberChanged, this, &MMGOBSField::saveData);
}

void MMGOBSNumberField::jsonData(QJsonObject &json_obj) const
{
	json_obj["number"] = number.value();
	json_obj["lower"] = number.min();
	json_obj["higher"] = number.max();
	json_obj["state"] = number.state();
}

void MMGOBSNumberField::apply(const QJsonObject &json_obj)
{
	number.setValue(json_obj["number"].toDouble());
	number.setMin(json_obj["lower"].toDouble());
	number.setMax(json_obj["higher"].toDouble());
	number.setState(json_obj["state"].toInt());
	emit triggerUpdate();
}

void MMGOBSNumberField::refresh(const MMGOBSFieldInit &init)
{
	if (init.current_json.contains(_name)) {
		number = init.current_json[_name].toDouble();
	} else {
		number = init.default_json[_name].toDouble();
	}
	update(init.prop);
}

void MMGOBSNumberField::update(obs_property_t *prop)
{
	QSignalBlocker blocker_this(updateAndBlock(prop));
	num_display->setDescription(obs_property_description(prop));

	if (obs_property_get_type(prop) != OBS_PROPERTY_FLOAT && obs_property_get_type(prop) != OBS_PROPERTY_INT)
		return;

	MMGNumber bounds = propertyBounds(prop);
	num_display->setBounds(bounds.min(), bounds.max());
	num_display->setStep(bounds.value());

	MMGNoEdit no_edit_number(&number);
	num_display->display();
}

void MMGOBSNumberField::execute(QJsonObject &data, const MMGMessage *message) const
{
	if (number.state() == STATE_IGNORE) return;
	data[_name] = number.chooseFrom(message);
}

const MMGNumber MMGOBSNumberField::propertyBounds(obs_property_t *prop)
{
	MMGNumber bounds;
	if (!prop) return bounds;

	if (obs_property_get_type(prop) == OBS_PROPERTY_INT) {
		bounds.setValue(obs_property_int_step(prop));
		bounds.setMin(obs_property_int_min(prop));
		bounds.setMax(obs_property_int_max(prop));
	} else if (obs_property_get_type(prop) == OBS_PROPERTY_FLOAT) {
		bounds.setValue(obs_property_float_step(prop));
		bounds.setMin(obs_property_float_min(prop));
		bounds.setMax(obs_property_float_max(prop));
	}

	return bounds;
}

const MMGNumber MMGOBSNumberField::propertyBounds(obs_source_t *source, const QString &field)
{
	MMGNumber bounds;
	if (!source) return bounds;

	obs_properties_t *props = obs_source_properties(source);
	if (!props) goto finish;
	bounds = propertyBounds(obs_properties_get(props, field.qtocs()));

finish:
	obs_properties_destroy(props);
	return bounds;
}
// End MMGOBSNumber Field

// MMGOBSStringField
MMGOBSStringField::MMGOBSStringField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);
	string.setState(STATE_IGNORE);

	str_display = new MMGStringDisplay(this);
	str_display->setOptions(MIDIBUTTON_FIXED | MIDIBUTTON_MIDI | MIDIBUTTON_IGNORE | MIDIBUTTON_TOGGLE);
	str_display->setDisplayMode(
		(MMGStringDisplay::Mode)(obs_property_list_type(init.prop) == OBS_COMBO_TYPE_EDITABLE ? 2 : 0));
	str_display->setStorage(&string);

	refresh(init);

	connect(str_display, &MMGStringDisplay::stringChanged, this, &MMGOBSField::saveUpdates);
	connect(str_display, &MMGStringDisplay::stringChanged, this, &MMGOBSField::saveData);
}

void MMGOBSStringField::jsonUpdate(QJsonObject &json_obj) const
{
	json_obj[_name] = values.value(options.indexOf(string.value())).toJsonValue();
}

void MMGOBSStringField::jsonData(QJsonObject &json_obj) const
{
	json_obj["value"] = values.value(options.indexOf(string.value())).toJsonValue();
	json_obj["lower"] = values.value(options.indexOf(string.min())).toJsonValue();
	json_obj["higher"] = values.value(options.indexOf(string.max())).toJsonValue();
	json_obj["state"] = string.state();
}

void MMGOBSStringField::apply(const QJsonObject &json_obj)
{
	string.setValue(options.value(values.indexOf(json_obj["value"].toVariant())));
	string.setMin(options.value(values.indexOf(json_obj["lower"].toVariant())));
	string.setMax(options.value(values.indexOf(json_obj["higher"].toVariant())));
	string.setState(json_obj["state"].toInt());
	emit triggerUpdate();
}

void MMGOBSStringField::refresh(const MMGOBSFieldInit &init)
{
	options = propertyDescriptions(init.prop);
	values = propertyValues(init.prop);
	if (init.current_json.contains(_name)) {
		string = options.value(values.indexOf(init.current_json[_name].toVariant()));
	} else {
		string = options.value(values.indexOf(init.default_json[_name].toVariant()));
	}
	update(init.prop);
}

void MMGOBSStringField::update(obs_property_t *prop)
{
	QSignalBlocker blocker_this(updateAndBlock(prop));
	if (obs_property_get_type(prop) != OBS_PROPERTY_LIST) return;

	str_display->setDescription(obs_property_description(prop));
	options = propertyDescriptions(prop);
	values = propertyValues(prop);
	if (str_display->bounds() != options) str_display->setBounds(options);

	MMGNoEdit no_edit_string(&string);
	str_display->display();
}

void MMGOBSStringField::execute(QJsonObject &data, const MMGMessage *message) const
{
	if (string.state() == STATE_IGNORE) return;
	data[_name] = QJsonValue::fromVariant(values.value(options.indexOf(string.chooseFrom(message, options))));
}

const QStringList MMGOBSStringField::propertyDescriptions(obs_property_t *prop)
{
	QStringList names;

	for (size_t i = 0; i < obs_property_list_item_count(prop); ++i)
		names += obs_property_list_item_name(prop, i);

	return names;
}

const QVariantList MMGOBSStringField::propertyValues(obs_property_t *prop)
{
	QVariantList values;

	for (size_t i = 0; i < obs_property_list_item_count(prop); ++i) {
		QVariant value;

		switch (obs_property_list_format(prop)) {
			case OBS_COMBO_FORMAT_FLOAT:
				value = obs_property_list_item_float(prop, i);
				break;
			case OBS_COMBO_FORMAT_INT:
				value = obs_property_list_item_int(prop, i);
				break;
			case OBS_COMBO_FORMAT_STRING:
				value = QString(obs_property_list_item_string(prop, i));
				break;
			default:
				break;
		}

		values += value;
	}

	return values;
}

const QVariantList MMGOBSStringField::propertyValues(obs_source_t *source, const QString &field)
{
	QVariantList values;
	if (!source) return values;

	obs_properties_t *props = obs_source_properties(source);
	if (!props) goto finish;
	values = propertyValues(obs_properties_get(props, field.qtocs()));

finish:
	obs_properties_destroy(props);
	return values;
}
// End MMGOBSStringField

// MMGOBSBooleanField
MMGOBSBooleanField::MMGOBSBooleanField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);

	label->setVisible(true);
	label->setGeometry(0, 10, 150, 30);
	label->setText(obs_property_description(init.prop));
	label->setWordWrap(true);

	button = new QPushButton(this);
	button->setCheckable(true);
	button->setGeometry(220, 20, 50, 50);
	button->setIcon(MMGInterface::icon("confirm"));
	button->setIconSize(QSize(28, 28));
	button->setCursor(Qt::PointingHandCursor);

	midi_buttons = new MMGMIDIButtons(this);
	midi_buttons->move(0, 47);
	midi_buttons->setOptions(MIDIBUTTON_FIXED | MIDIBUTTON_IGNORE | MIDIBUTTON_TOGGLE);

	refresh(init);

	connect(button, &QAbstractButton::toggled, this, &MMGOBSBooleanField::changeBoolean);
	connect(midi_buttons, &MMGMIDIButtons::stateChanged, this, &MMGOBSBooleanField::changeState);
}

void MMGOBSBooleanField::jsonData(QJsonObject &json_obj) const
{
	json_obj["value"] = boolean;
	json_obj["state"] = state;
}

void MMGOBSBooleanField::changeBoolean(bool value)
{
	boolean = value;
	emit saveUpdates();
	emit saveData();
}

void MMGOBSBooleanField::changeState(int _state)
{
	state = (ValueState)_state;
	button->setEnabled(state == 0);
	emit saveUpdates();
	emit saveData();
}

void MMGOBSBooleanField::apply(const QJsonObject &json_obj)
{
	boolean = json_obj["value"].toBool();
	state = (ValueState)json_obj["state"].toInt();
	if (state > 0 && state < 3) state = STATE_IGNORE;
	button->setChecked(boolean);
	midi_buttons->setState(state);
}

void MMGOBSBooleanField::refresh(const MMGOBSFieldInit &init)
{
	QSignalBlocker blocker_this(updateAndBlock(init.prop));

	if (init.current_json.contains(_name)) {
		boolean = init.current_json[_name].toBool();
	} else {
		boolean = init.default_json[_name].toBool();
	}
	state = STATE_FIXED;

	button->setChecked(boolean);
	midi_buttons->setState(state);
}

void MMGOBSBooleanField::execute(QJsonObject &data, const MMGMessage *) const
{
	if (state == STATE_IGNORE) return;
	data[_name] = state == STATE_TOGGLE ? !data[_name].toBool() : boolean;
}
// End MMGOBSBooleanField

// MMGOBSButtonField
MMGOBSButtonField::MMGOBSButtonField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);

	button = new QPushButton(this);
	button->setVisible(true);
	button->setGeometry(10, 25, 310, 40);
	button->setCursor(Qt::PointingHandCursor);
	connect(button, &QPushButton::clicked, this, &MMGOBSButtonField::clicked);

	refresh(init);
}

void MMGOBSButtonField::refresh(const MMGOBSFieldInit &init)
{
	update(init.prop);
}

void MMGOBSButtonField::update(obs_property_t *prop)
{
	QSignalBlocker blocker_this(updateAndBlock(prop));
	button->setText(obs_property_description(prop));
}

void MMGOBSButtonField::callback(obs_property_t *prop, void *source)
{
	if (obs_property_button_type(prop) == OBS_BUTTON_URL) {
		QUrl url(obs_property_button_url(prop), QUrl::StrictMode);
		if (!url.isValid() || !url.scheme().contains("http")) return;
		QDesktopServices::openUrl(url);
	} else {
		obs_property_button_clicked(prop, source);
		update(prop);
	}
}
// End MMGOBSButtonField

// MMGOBSColorField
MMGOBSColorField::MMGOBSColorField(QWidget *p, const MMGOBSFieldInit &init, bool use_alpha) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);
	alpha = use_alpha;

	label->setVisible(true);
	label->setGeometry(0, 0, 150, 40);
	label->setWordWrap(true);

	button = new QPushButton(this);
	button->setVisible(true);
	button->setGeometry(160, 0, 170, 40);
	button->setCursor(Qt::PointingHandCursor);
	button->setText(mmgtr("Fields.ColorSelect"));
	connect(button, &QPushButton::clicked, this, &MMGOBSColorField::callback);

	frame = new QFrame(this);
	frame->setVisible(true);
	frame->setGeometry(0, 45, 330, 45);

	refresh(init);
	frame->setStyleSheet(frame_style + " background-color: " + color.name((QColor::NameFormat)alpha));
}

void MMGOBSColorField::jsonUpdate(QJsonObject &json_obj) const
{
	json_obj[_name] = convertColor(color.rgba());
}

void MMGOBSColorField::jsonData(QJsonObject &json_obj) const
{
	json_obj["color"] = convertColor(color.rgba());
}

void MMGOBSColorField::apply(const QJsonObject &json_obj)
{
	color = QColor::fromRgba(convertColor(json_obj["color"].toInteger(0xFF000000)));
	frame->setStyleSheet(frame_style + " background-color: " + color.name((QColor::NameFormat)alpha));
}

void MMGOBSColorField::refresh(const MMGOBSFieldInit &init)
{
	if (init.current_json.contains(_name)) {
		color = QColor::fromRgba(convertColor(init.current_json[_name].toInteger(0xFF000000)));
	} else {
		color = QColor::fromRgba(convertColor(init.default_json[_name].toInteger(0xFF000000)));
	}
	update(init.prop);
}

void MMGOBSColorField::update(obs_property_t *prop)
{
	QSignalBlocker blocker_this(updateAndBlock(prop));
	label->setText(obs_property_description(prop));
}

void MMGOBSColorField::callback()
{
	QColor _color = QColorDialog::getColor(color, nullptr, QString(), (QColorDialog::ColorDialogOption)alpha);
	color = _color.isValid() ? _color : color;
	frame->setStyleSheet(frame_style + " background-color: " + color.name((QColor::NameFormat)alpha));
	emit saveData();
}

qint64 MMGOBSColorField::convertColor(qint64 rgb)
{
	return (rgb & 0xFF000000) | ((rgb & 0xFF0000) >> 16) | (rgb & 0xFF00) | ((rgb & 0xFF) << 16);
}
// End MMGOBSColorField

// MMGOBSFontField
MMGOBSFontField::MMGOBSFontField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);

	label->setVisible(true);
	label->setGeometry(0, 0, 150, 40);
	label->setWordWrap(true);

	button = new QPushButton(this);
	button->setVisible(true);
	button->setGeometry(160, 0, 170, 40);
	button->setText(mmgtr("Fields.FontSelect"));
	button->setCursor(Qt::PointingHandCursor);
	connect(button, &QPushButton::clicked, this, &MMGOBSFontField::callback);

	frame = new QFrame(this);
	frame->setVisible(true);
	frame->setGeometry(0, 45, 330, 45);
	frame->setStyleSheet(frame_style);

	inner_label = new QLabel(frame);
	inner_label->setVisible(true);
	inner_label->setGeometry(0, 0, 330, 45);
	inner_label->setAlignment(Qt::AlignCenter);

	refresh(init);
}

void MMGOBSFontField::jsonUpdate(QJsonObject &json_obj) const
{
	QJsonObject font_obj;
	jsonData(font_obj);
	json_obj[_name] = font_obj;
}

void MMGOBSFontField::jsonData(QJsonObject &json_obj) const
{
	json_obj["face"] = font.family();
	json_obj["flags"] = (font.strikeOut() << 3) | (font.underline() << 2) | (font.italic() << 1) |
			    (font.bold() << 0);
	json_obj["size"] = font.pointSize();
	json_obj["style"] = font.styleName();
}

void MMGOBSFontField::apply(const QJsonObject &json_obj)
{
	font.setFamily(json_obj["face"].toString());
	int flags = json_obj["flags"].toInt();
	font.setBold(flags & 0b0001);
	font.setItalic(flags & 0b0010);
	font.setUnderline(flags & 0b0100);
	font.setStrikeOut(flags & 0b1000);
	font.setPointSize(json_obj["size"].toInt());
	font.setStyleName(json_obj["style"].toString());
	inner_label->setText(font.family() + ", " + QString::number(font.pointSize()) + "pt");
}

void MMGOBSFontField::refresh(const MMGOBSFieldInit &init)
{
	if (init.current_json.contains(_name)) {
		apply(init.current_json[_name].toObject());
	} else {
		apply(init.default_json[_name].toObject());
	}
	update(init.prop);
}

void MMGOBSFontField::update(obs_property_t *prop)
{
	QSignalBlocker blocker_this(updateAndBlock(prop));
	label->setText(obs_property_description(prop));

	inner_label->setFont(font);
	inner_label->setText(QString("%1, %2pt").arg(font.family()).arg(font.pointSize()));
}

void MMGOBSFontField::callback()
{
	bool accept;
	QFont _font = QFontDialog::getFont(&accept, font, nullptr, mmgtr("Fields.FontSelect"),
					   QFontDialog::DontUseNativeDialog);
	if (accept) font = _font;
	emit triggerUpdate();
	emit saveData();
}
// End MMGOBSFontField

// MMGOBSGroupField
MMGOBSGroupField::MMGOBSGroupField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);

	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(10);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	layout->setContentsMargins(0, 10, 0, 25);
	setLayout(layout);

	if (obs_property_group_type(init.prop) == OBS_GROUP_CHECKABLE) {
		boolean_field = new MMGOBSBooleanField(p, init);
		boolean_field->setStyleSheet("QLabel {font-weight: bold;}");
	} else if (obs_property_group_type(init.prop) == OBS_GROUP_NORMAL) {
		label->setVisible(true);
		label->setWordWrap(true);
		label->setStyleSheet("QLabel {font-weight: bold;}");
		layout->addWidget(label);
	}
}

void MMGOBSGroupField::jsonUpdate(QJsonObject &json_obj) const
{
	HAS_BOOLEAN_FIELD boolean_field->jsonUpdate(json_obj);
}

void MMGOBSGroupField::jsonData(QJsonObject &json_obj) const
{
	HAS_BOOLEAN_FIELD boolean_field->jsonData(json_obj);
}

void MMGOBSGroupField::refresh(const MMGOBSFieldInit &init)
{
	HAS_BOOLEAN_FIELD boolean_field->refresh(init);
	update(init.prop);
}

void MMGOBSGroupField::update(obs_property_t *prop)
{
	if (boolean_field) {
		boolean_field->update(prop);
		setEnabled(boolean_field->boolean);
	} else {
		label->setText(obs_property_description(prop));
	}
}

void MMGOBSGroupField::execute(QJsonObject &data, const MMGMessage *message) const
{
	HAS_BOOLEAN_FIELD boolean_field->execute(data, message);
}
// End MMGOBSGroupField

// MMGOBSPathField
MMGOBSPathField::MMGOBSPathField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);

	label->setVisible(true);
	label->setGeometry(0, 0, 150, 40);
	label->setWordWrap(true);

	button = new QPushButton(this);
	button->setVisible(true);
	button->setGeometry(160, 0, 170, 40);
	button->setCursor(Qt::PointingHandCursor);
	connect(button, &QPushButton::clicked, this, &MMGOBSPathField::callback);

	frame = new QFrame(this);
	frame->setVisible(true);
	frame->setGeometry(0, 45, 330, 45);
	frame->setStyleSheet(frame_style);

	path_display = new QLabel(frame);
	path_display->setVisible(true);
	path_display->setGeometry(0, 0, 330, 45);
	path_display->setAlignment(Qt::AlignVCenter | Qt::AlignBottom);
	path_display->setWordWrap(true);

	refresh(init);
}

void MMGOBSPathField::apply(const QJsonObject &json_obj)
{
	path = json_obj["string"].toString();
	emit triggerUpdate();
}

void MMGOBSPathField::refresh(const MMGOBSFieldInit &init)
{
	if (init.current_json.contains(_name)) {
		path = init.current_json[_name].toString();
	} else {
		path = init.default_json[_name].toString();
	}
	update(init.prop);
}

void MMGOBSPathField::update(obs_property_t *prop)
{
	QSignalBlocker blocker_this(updateAndBlock(prop));
	label->setText(obs_property_description(prop));

	if (obs_property_get_type(prop) != OBS_PROPERTY_PATH) return;

	default_path = obs_property_path_default_path(prop);
	dialog_type = obs_property_path_type(prop);
	filters = obs_property_path_filter(prop);

	path_display->setText(path);
	button->setText(MMGText::choose("Fields", "FileSelect", "FolderSelect", dialog_type < 2));
}

void MMGOBSPathField::execute(QJsonObject &data, const MMGMessage *message) const
{
	QString str = path;
	str.replace("${type}", message->type());
	str.replace("${channel}", MMGText::asString(message->channel()));
	str.replace("${note}", MMGText::asString(message->note()));
	str.replace("${control}", MMGText::asString(message->note()));
	str.replace("${value}", MMGText::asString(message->value()));
	str.replace("${velocity}", MMGText::asString(message->value()));
	data[_name] = str;
}

void MMGOBSPathField::callback()
{
	QString new_path;
	switch (dialog_type) {
		case 0: // FILE READING
			new_path = QFileDialog::getOpenFileName(nullptr, button->text(), default_path, filters, nullptr,
								QFileDialog::Option::HideNameFilterDetails);
			break;
		case 1: // FILE WRITING
			new_path = QFileDialog::getSaveFileName(nullptr, button->text(), default_path, filters);
			break;
		case 2: // DIRECTORY
			new_path = QFileDialog::getExistingDirectory(nullptr, button->text(), default_path);
			break;
		default:
			break;
	}

	if (new_path.isNull()) return;
	path = new_path;
	emit triggerUpdate();
	emit saveData();
}
// End MMGOBSPathField

// MMGOBSTextField
MMGOBSTextField::MMGOBSTextField(QWidget *p, const MMGOBSFieldInit &init) : MMGOBSField(p)
{
	_name = obs_property_name(init.prop);

	obs_text_type prop_type = obs_property_text_type(init.prop);

	label->setVisible(true);
	label->setGeometry(0, 0, 330, 30);

	text_edit = new QTextEdit(this);
	text_edit->setGeometry(0, 30, 330, 60);
	text_edit->setVisible(prop_type == OBS_TEXT_MULTILINE);

	line_edit = new QLineEdit(this);
	line_edit->setGeometry(0, 30, 330, 60);
	line_edit->setVisible(prop_type != OBS_TEXT_MULTILINE);
	line_edit->setEchoMode((QLineEdit::EchoMode)(prop_type == OBS_TEXT_PASSWORD ? 3 : 0));

	refresh(init);

	text_edit->connect(text_edit, &QTextEdit::textChanged, text_edit,
			   [&]() { callback(text_edit->toPlainText()); });
	line_edit->connect(line_edit, &QLineEdit::textChanged, this, &MMGOBSTextField::callback);
}

void MMGOBSTextField::callback(const QString &str)
{
	text = str;
	emit saveData();
}

void MMGOBSTextField::apply(const QJsonObject &json_obj)
{
	text = json_obj["string"].toString();
	emit triggerUpdate();
}

void MMGOBSTextField::refresh(const MMGOBSFieldInit &init)
{
	if (init.current_json.contains(_name)) {
		text = init.current_json[_name].toString();
	} else {
		text = init.default_json[_name].toString();
	}
	update(init.prop);
}

void MMGOBSTextField::update(obs_property_t *prop)
{
	QSignalBlocker blocker_this(updateAndBlock(prop));
	label->setText(obs_property_description(prop));

	text_edit->setPlainText(text);
	line_edit->setText(text);
}

void MMGOBSTextField::execute(QJsonObject &data, const MMGMessage *message) const
{
	QString str = text;
	str.replace("${type}", message->type());
	str.replace("${channel}", MMGText::asString(message->channel()));
	str.replace("${note}", MMGText::asString(message->note()));
	str.replace("${control}", MMGText::asString(message->note()));
	str.replace("${value}", MMGText::asString(message->value()));
	str.replace("${velocity}", MMGText::asString(message->value()));
	data[_name] = str;
}
// End MMGOBSTextField

// MMGOBSFields
QList<MMGOBSFields *> MMGOBSFields::all_fields;

MMGOBSFields::MMGOBSFields(obs_source_t *source)
{
	if (!source) {
		MMGInterface::promptUser("FieldsError", false);
		return;
	}

	identifier = obs_source_get_id(source);

	resize(330, 350);
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(10);
	layout->setSizeConstraint(QLayout::SetFixedSize);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	addFields(this, props, gatherSourceData(source));
	jsonUpdate();
}

void MMGOBSFields::changeSource(obs_source_t *source)
{
	if (!match(source) || obs_source_get_name(source) == source_name) return;

	MMGOBSFieldInit init = gatherSourceData(source);
	property_json = {};
	data_json = {};

	for (MMGOBSField *field : fields) {
		init.prop = obs_properties_get(props, field->name().qtocs());
		field->refresh(init);
	}
}

MMGOBSFieldInit MMGOBSFields::gatherSourceData(obs_source_t *source)
{
	source_name = obs_source_get_name(source);

	if (props) obs_properties_destroy(props);
	props = obs_source_properties(source);

	return {nullptr, sourceDataJson(source), sourceDefaultJson(source)};
}

QJsonObject MMGOBSFields::sourceDataJson(obs_source_t *source)
{
	OBSDataAutoRelease source_data = obs_source_get_settings(source);
	OBSDataAutoRelease source_defaults = obs_data_get_defaults(source_data);
	return MMGJsonObject::toObject(obs_data_get_json(source_data));
}

QJsonObject MMGOBSFields::sourceDefaultJson(obs_source_t *source)
{
	OBSDataAutoRelease source_data = obs_source_get_settings(source);
	OBSDataAutoRelease source_defaults = obs_data_get_defaults(source_data);
	return MMGJsonObject::toObject(obs_data_get_json(source_defaults));
}

void MMGOBSFields::addFields(QWidget *parent, obs_properties_t *_props, const MMGOBSFieldInit &json_init)
{
	if (!_props) return;

	obs_property_t *prop = obs_properties_first(_props);

	MMGOBSFieldInit init{prop, json_init.current_json, json_init.default_json};

	do {
		init.prop = prop;
		MMGOBSField *field;

		switch (obs_property_get_type(prop)) {
			case OBS_PROPERTY_FLOAT:
			case OBS_PROPERTY_INT:
				field = new MMGOBSNumberField(parent, init);
				break;

			case OBS_PROPERTY_LIST:
				field = new MMGOBSStringField(parent, init);
				break;

			case OBS_PROPERTY_BOOL:
				field = new MMGOBSBooleanField(parent, init);
				break;

			case OBS_PROPERTY_BUTTON:
				field = new MMGOBSButtonField(parent, init);
				connect(qobject_cast<MMGOBSButtonField *>(field), &MMGOBSButtonField::clicked, this,
					&MMGOBSFields::buttonFieldClicked);
				break;

			case OBS_PROPERTY_COLOR:
				field = new MMGOBSColorField(parent, init, false);
				break;

			case OBS_PROPERTY_COLOR_ALPHA:
				field = new MMGOBSColorField(parent, init, true);
				break;

			case OBS_PROPERTY_FONT:
				field = new MMGOBSFontField(parent, init);
				break;

			case OBS_PROPERTY_GROUP:
				field = new MMGOBSGroupField(parent, init);
				addGroupContent(qobject_cast<MMGOBSGroupField *>(field), init);
				break;

			case OBS_PROPERTY_PATH:
				field = new MMGOBSPathField(parent, init);
				break;

			case OBS_PROPERTY_TEXT:
				if (obs_property_text_type(prop) != OBS_TEXT_INFO) {
					field = new MMGOBSTextField(parent, init);
					break;
				}
				[[fallthrough]];

			case OBS_PROPERTY_EDITABLE_LIST:
			case OBS_PROPERTY_FRAME_RATE:
			default:
				continue;
		}

		connect(field, &MMGOBSField::triggerUpdate, this, &MMGOBSFields::updateField);
		connect(field, &MMGOBSField::saveUpdates, this, &MMGOBSFields::jsonUpdate);
		connect(field, &MMGOBSField::saveData, this, &MMGOBSFields::jsonData);

		fields.append(field);
		parent->layout()->addWidget(field);

	} while (obs_property_next(&prop) != 0);
}

void MMGOBSFields::jsonUpdate()
{
	auto sender_field = qobject_cast<MMGOBSField *>(sender());

	if (sender_field) {
		sender_field->jsonUpdate(property_json);
	} else {
		property_json = {};
		for (MMGOBSField *field : fields)
			field->jsonUpdate(property_json);
	}

	OBSDataAutoRelease obs_data = obs_data_create_from_json(MMGJsonObject::toString(property_json));
	obs_properties_apply_settings(props, obs_data);

	for (MMGOBSField *field : fields)
		field->update(obs_properties_get(props, field->name().qtocs()));
}

void MMGOBSFields::jsonData()
{
	auto field = qobject_cast<MMGOBSField *>(sender());
	if (!field) return;

	QJsonObject json_obj;
	field->jsonData(json_obj);
	data_json[field->name()] = json_obj;

	emit dataChanged(data_json);
}

void MMGOBSFields::apply(MMGJsonObject *json)
{
	disconnect(this, &MMGOBSFields::dataChanged, nullptr, nullptr);
	if (!json) return;
	connect(this, &MMGOBSFields::dataChanged, json, &MMGJsonObject::setJson);

	if (property_json.keys() != json->keys()) json->clear();
	bool do_apply = !json->isEmpty();

	data_json = {};
	for (MMGOBSField *field : fields) {
		if (do_apply) field->apply(json->value(field->name()).toObject());
		emit field->saveData();
	}
}

void MMGOBSFields::updateField()
{
	auto field = qobject_cast<MMGOBSField *>(sender());
	if (!field) return;

	field->update(obs_properties_get(props, field->name().qtocs()));
}

void MMGOBSFields::buttonFieldClicked()
{
	auto field = qobject_cast<MMGOBSButtonField *>(sender());
	if (!field) return;

	obs_property_t *prop = obs_properties_get(props, field->name().qtocs());
	field->callback(prop, OBSSourceAutoRelease(obs_get_source_by_name(source_name.qtocs())));
}

void MMGOBSFields::addGroupContent(MMGOBSGroupField *field, const MMGOBSFieldInit &init)
{
	MMGOBSBooleanField *bool_field = field->booleanField();
	if (bool_field) {
		connect(bool_field, &MMGOBSField::triggerUpdate, this, &MMGOBSFields::updateField);
		connect(bool_field, &MMGOBSField::saveUpdates, this, &MMGOBSFields::jsonUpdate);
		connect(bool_field, &MMGOBSField::saveData, this, &MMGOBSFields::jsonData);
		layout()->addWidget(bool_field);
	}

	addFields(field, obs_property_group_content(init.prop), init);
}

MMGOBSFields *MMGOBSFields::findFields(obs_source_t *source)
{
	if (!source) return nullptr;

	for (MMGOBSFields *obs_fields : all_fields) {
		if (!obs_fields->match(source)) continue;
		obs_fields->changeSource(source);
		return obs_fields;
	}

	return nullptr;
}

MMGOBSFields *MMGOBSFields::registerSource(obs_source_t *source, MMGJsonObject *json)
{
	if (!source) return nullptr;

	MMGOBSFields *fields = findFields(source);
	if (!fields) {
		fields = new MMGOBSFields(source);
		all_fields.append(fields);
	}

	fields->apply(json);
	return fields;
}

void MMGOBSFields::execute(obs_source_t *source, const MMGJsonObject *json, const MMGMessage *message)
{
	MMGOBSFields *fields = findFields(source);
	if (!fields) return;

	std::lock_guard custom_guard(custom_update);
	QJsonObject final_json = sourceDataJson(source);

	for (MMGOBSField *field : fields->fields) {
		field->apply(json->value(field->name()).toObject());
		field->execute(final_json, message);
		field->apply(fields->data_json[field->name()].toObject());
	}

	obs_source_update(source, OBSDataAutoRelease(obs_data_create_from_json(MMGJsonObject::toString(final_json))));
}
