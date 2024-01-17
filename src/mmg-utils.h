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
#include "obs-midi-mg.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QLCDNumber>
#include <QTimer>
#include <QScrollArea>
#include <QComboBox>
#include <QDateTime>
#include <QLabel>
#include <QPushButton>
#include <QLayout>

#define MMG_ENABLED if (editable)

void global_blog(int log_status, const QString &message);

QDataStream &operator<<(QDataStream &out, const QObject *&obj);
QDataStream &operator>>(QDataStream &in, QObject *&obj);

class MMGMessage;

namespace MMGUtils {

enum DeviceType { TYPE_NONE = -1, TYPE_INPUT, TYPE_OUTPUT };
enum ValueState { STATE_FIXED, STATE_MIDI, STATE_CUSTOM, STATE_IGNORE, STATE_TOGGLE };

template<typename T> struct MMGValue {

public:
	virtual ~MMGValue() = default;

	const T &value() const { return val; };
	const T &min() const { return lower; };
	const T &max() const { return higher; };
	ValueState state() const { return value_state; };
	void setValue(const T &value) { MMG_ENABLED val = value; };
	void setMin(const T &min) { MMG_ENABLED lower = min; };
	void setMax(const T &max) { MMG_ENABLED higher = max; };
	void setState(ValueState state) { MMG_ENABLED value_state = state; };
	void setState(short state) { MMG_ENABLED value_state = (ValueState)state; };

	MMGValue<T> copy() const { return *this; };
	bool isEditable() const { return editable; };
	void setEditable(bool edit) { editable = edit; };
	void toggle()
	{
		if (value_state != STATE_TOGGLE) return;
		val = val == lower ? higher : lower;
	};

	operator const T &() const { return val; };
	MMGValue<T> &operator=(const T &value)
	{
		setValue(value);
		return *this;
	};

protected:
	T val;
	T lower;
	T higher;
	ValueState value_state = STATE_FIXED;

	bool editable = true;
};

struct MMGNumber : public MMGValue<double> {

public:
	using MMGValue<double>::operator=;

	MMGNumber();
	MMGNumber(const QJsonObject &json_obj, const QString &preferred, int fallback_num = 0);

	void json(QJsonObject &json_obj, const QString &prefix, bool use_bounds = true) const;

	double chooseFrom(const MMGMessage *midi, bool round = false, double default_val = 0.0) const;
	void chooseOther(const MMGNumber &applied);

	double map(const MMGNumber &other, bool round = true) const;
	bool acceptable(double val) const;
};

struct MMGString : public MMGValue<QString> {

public:
	using MMGValue<QString>::operator=;

	MMGString(){};
	MMGString(const QJsonObject &json_obj, const QString &preferred, int fallback_num = 0);

	void json(QJsonObject &json_obj, const QString &prefix, bool use_state = true) const;

	QString chooseFrom(const MMGMessage *midi, const QStringList &midi_range) const;
	bool chooseTo(MMGNumber &values, const QStringList &range) const;

	bool operator==(const char *ch) const { return val == ch; };
	bool operator!=(const char *ch) const { return val != ch; };
};

class MMGJsonObject : public QObject {

public:
	MMGJsonObject(QObject *parent = nullptr) : QObject(parent){};

	bool isEmpty() const { return json_obj.isEmpty(); };
	QJsonValue value(const QString &key) const { return json_obj[key]; };
	void insert(const QString &key, const QJsonValue &value) { json_obj[key] = value; };

	const QJsonObject &json() const { return json_obj; };
	void setJson(const QJsonObject &json) { json_obj = json; };
	void clear() { json_obj = {}; };

private:
	QJsonObject json_obj;
};

template<class T> struct MMGNoEdit {
	MMGNoEdit(T *val)
	{
		no_edit = val;
		val->setEditable(false);
	};
	~MMGNoEdit() { no_edit->setEditable(true); };

private:
	T *no_edit;
};

class MMGTimer : public QTimer {
	Q_OBJECT

public:
	MMGTimer(QObject *parent = nullptr);

signals:
	void resetting(int);
	void stopping();

public slots:
	void stopTimer() { emit stopping(); };
	void reset(int time);
};

QString mmgtr_join(const QString &header, const QString &joiner);
QString mmgtr_two(const char *header, const char *opt1, const char *opt2, bool decider);
QStringList mmgtr_all(const QString &header, const QStringList &list);
QStringList obstr_all(const QString &header, const QStringList &list);

const QByteArray json_to_str(const QJsonObject &json_obj);
const QJsonObject json_from_str(const QByteArray &str);

bool num_between(double num, double lower, double higher, bool inclusive = true);
QString num_to_str(int num, const QString &prefix = "");

void enable_combo_option(QComboBox *combo, int index, bool enable);
QIcon mmg_icon(const QString &icon_name);

bool open_message_box(const QString &message, bool information = true);

void obs_source_custom_update(obs_source_t *source, const QJsonObject &action_json, const MMGMessage *midi_value);
QList<MMGNumber> obs_source_custom_updated(obs_source_t *source, const QJsonObject &action_json);
}

#undef MMG_ENABLED
