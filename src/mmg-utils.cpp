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

#include "mmg-utils.h"
#include "mmg-message.h"

#include <QMessageBox>
#include <QStandardItemModel>

void global_blog(int log_status, const QString &message)
{
	blog(log_status, "[obs-midi-mg] %s", message.qtocs());
}

QDataStream &operator<<(QDataStream &out, const QObject *&obj)
{
	return out << *(quint64 *)(&obj);
}

QDataStream &operator>>(QDataStream &in, QObject *&obj)
{
	quint64 data;
	in >> data;
	obj = *(QObject **)(&data);
	return in;
}

namespace MMGUtils {

// MMGNumber
MMGNumber::MMGNumber()
{
	val = 0.0;
	lower = 0.0;
	higher = 100.0;
}

MMGNumber::MMGNumber(const QJsonObject &json_obj, const QString &preferred, int fallback_num)
{
	double fallback = json_obj[num_to_str(fallback_num, "num")].toDouble();

	if (json_obj[preferred].isObject()) {
		// v2.3.0+ (with bounds)
		QJsonObject number_obj = json_obj[preferred].toObject();
		setValue(number_obj["number"].toDouble());
		setMin(number_obj["lower"].toDouble());
		setMax(number_obj["higher"].toDouble());
		setState(number_obj["state"].toInt());
	} else if (json_obj[preferred].isDouble()) {
		// v2.2.0+ (no bounds)
		setValue(json_obj[preferred].toDouble());
		short nums_state = json_obj[preferred + "_state"].toInt();
		setState(nums_state == 2 ? 1 : nums_state);
		setMin(val < lower ? val : lower);
		setMax(val > higher ? val : higher);
	} else if (json_obj["nums_state"].isDouble()) {
		// v2.1.0 - v2.1.1
		setValue(fallback);
		int nums_state = (json_obj["nums_state"].toInt() & (3 << (fallback_num * 2))) >> (fallback_num * 2);
		setState(nums_state == 2 ? 3 : nums_state);
		setMin(val < lower ? val : lower);
		setMax(val > higher ? val : higher);
	} else {
		// pre v2.1.0
		setValue(fallback == -1 ? 0 : fallback);
		setState(fallback == -1);
		setMin(val < lower ? val : lower);
		setMax(val > higher ? val : higher);
	}
	if ((uint)value_state > 4) value_state = STATE_FIXED;
}

void MMGNumber::json(QJsonObject &json_obj, const QString &prefix, bool use_bounds) const
{
	if (!use_bounds) {
		json_obj[prefix] = val;
		return;
	}
	QJsonObject number_json_obj;
	number_json_obj["number"] = val;
	number_json_obj["lower"] = lower;
	number_json_obj["higher"] = higher;
	number_json_obj["state"] = value_state;
	json_obj[prefix] = number_json_obj;
}

double MMGNumber::chooseFrom(const MMGMessage *midi, bool round, double default_value) const
{
	switch (value_state) {
		case STATE_MIDI:
		case STATE_CUSTOM:
			return midi->value().map(*this, round);

		case STATE_IGNORE:
			return default_value;

		default:
			return val;
	}
}

void MMGNumber::chooseOther(const MMGNumber &applied)
{
	switch (value_state) {
		case STATE_MIDI:
		case STATE_CUSTOM:
			val = applied.map(*this);
			break;

		case STATE_IGNORE:
			val = applied.value();
			break;

		default:
			break;
	}
}

double MMGNumber::map(const MMGNumber &other, bool round) const
{
	if (higher == lower) return other.lower;
	double mapped = (val - lower) / (higher - lower) * (other.higher - other.lower) + other.lower;
	return round ? qRound64(mapped) : mapped;
}

bool MMGNumber::acceptable(double value) const
{
	switch (value_state) {
		case STATE_FIXED:
		case STATE_TOGGLE:
			return val == value;

		case STATE_MIDI:
		case STATE_CUSTOM:
			if (lower <= higher) {
				return num_between(value, lower, higher);
			} else {
				return num_between(value, higher, lower, false);
			}

		case STATE_IGNORE:
			return true;

		default:
			return false;
	}
}
// End MMGNumber

// MMGString
MMGString::MMGString(const QJsonObject &json_obj, const QString &preferred, int fallback_num)
{
	if (json_obj[preferred].isObject()) {
		// v2.3.0+
		QJsonObject string_obj = json_obj[preferred].toObject();
		setValue(string_obj["string"].toString());
		setMin(string_obj["lower"].toString());
		setMax(string_obj["higher"].toString());
		setState(string_obj["state"].toInt());
	} else if (json_obj[preferred].isString()) {
		// v2.1.0 - v2.2.0
		setValue(json_obj[preferred].toString());
		setState(json_obj[preferred + "_state"].toInt());
	} else {
		// pre v2.1.0
		setValue(json_obj[num_to_str(fallback_num, "str")].toString());
		if (val == mmgtr("Fields.UseMessageValue")) {
			value_state = STATE_MIDI;
		} else if (val == mmgtr("Fields.Toggle")) {
			value_state = STATE_TOGGLE;
		}
	}
	if (value_state > STATE_TOGGLE) value_state = STATE_FIXED;
}

void MMGString::json(QJsonObject &json_obj, const QString &prefix, bool use_state) const
{
	if (use_state) {
		QJsonObject string_obj;
		string_obj["string"] = val;
		string_obj["lower"] = lower;
		string_obj["higher"] = higher;
		string_obj["state"] = (int)value_state;
		json_obj[prefix] = string_obj;
	} else {
		json_obj[prefix] = val;
	}
}

QString MMGString::chooseFrom(const MMGMessage *midi, const QStringList &midi_range) const
{
	if (value_state != STATE_MIDI) return val;
	if (midi_range.isEmpty()) return "";

	MMGNumber number;
	number.setMax(midi_range.size() - 1);
	return midi_range.value(midi->value().map(number));
}

bool MMGString::chooseTo(MMGNumber &values, const QStringList &range) const
{
	if (!range.isEmpty()) values.setMax(range.size() - 1);

	switch (value_state) {
		case STATE_MIDI:
		case STATE_CUSTOM:
			values.setState(STATE_CUSTOM);
			return false;

		default:
			return values != range.indexOf(val);
	}
}
// End MMGString

// MMGJsonObject
MMGJsonObject::MMGJsonObject(QObject *parent, const QJsonObject &head) : QObject(parent)
{
	if (head["json"].isString()) {
		json_obj = MMGJsonObject::toObject(head["json"].toString().toUtf8());
	} else if (head["json_str"].isString()) {
		json_obj = MMGJsonObject::toObject(head["json_str"].toString().toUtf8());
	} else {
		json_obj = head["json"].toObject();
	}
}

QByteArray MMGJsonObject::toString(const QJsonObject &json_obj)
{
	return QJsonDocument(json_obj).toJson(QJsonDocument::Compact);
}

QJsonObject MMGJsonObject::toObject(const QByteArray &str)
{
	return QJsonDocument::fromJson(str).object();
}
// End MMGJsonObject

// MMGTimer
MMGTimer::MMGTimer(QObject *parent) : QTimer(parent)
{
	connect(this, &QTimer::timeout, this, &MMGTimer::stopTimer);
	connect(this, &MMGTimer::stopping, this, &QTimer::stop);
	connect(this, &MMGTimer::resetting, this, QOverload<int>::of(&QTimer::start));
}

void MMGTimer::reset(int time)
{
	emit resetting(time);
}
// End MMGTimer

QString mmgtr_join(const QString &header, const QString &joiner)
{
	return mmgtr(QString("%1.%2").arg(header).arg(joiner).qtocs());
}

QString mmgtr_two(const char *header, const char *opt1, const char *opt2, bool decider)
{
	return mmgtr_join(header, decider ? opt1 : opt2);
}

QStringList mmgtr_all(const QString &header, const QStringList &list)
{
	QStringList tr_list;
	for (const QString &str : list) {
		tr_list += mmgtr_join(header, str);
	}
	return tr_list;
}

QStringList obstr_all(const QString &header, const QStringList &list)
{
	QStringList tr_list;
	for (const QString &str : list) {
		tr_list += obstr((header + "." + str).qtocs());
	}
	return tr_list;
}

bool num_between(double num, double lower, double higher, bool inclusive)
{
	return inclusive ? ((num >= lower) && (num <= higher)) : ((num > lower) && (num < higher));
}

QString num_to_str(int num, const QString &prefix)
{
	return QString("%1%2").arg(prefix).arg(num);
}

bool open_message_box(const QString &message, bool information)
{
	QString title = mmgtr_join("UI.MessageBox.Title", message);
	QString text = mmgtr_join("UI.MessageBox.Text", message);

	if (information) {
		QMessageBox::information(nullptr, title, text);
		return true;
	} else {
		return QMessageBox::question(nullptr, title, text, QMessageBox::Ok | QMessageBox::Cancel,
					     QMessageBox::Ok) == QMessageBox::Ok;
	}
}

void enable_combo_option(QComboBox *combo, int index, bool enable)
{
	qobject_cast<QStandardItemModel *>(combo->model())->item(index)->setEnabled(enable);
}

QIcon mmg_icon(const QString &icon_name)
{
	return QIcon(QString(":/icons/%1.svg").arg(icon_name));
}

} // namespace MMGUtils
