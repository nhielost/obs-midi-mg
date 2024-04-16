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

QDataStream &operator<<(QDataStream &out, const QObject *&obj);
QDataStream &operator>>(QDataStream &in, QObject *&obj);

class MMGMessage;

namespace MMGUtils {

enum DeviceType { TYPE_NONE = -1, TYPE_INPUT, TYPE_OUTPUT };
enum ValueState { STATE_FIXED, STATE_MIDI, STATE_CUSTOM, STATE_IGNORE, STATE_TOGGLE };
enum Translator { TEXT_MMG, TEXT_OBS };

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
using MMGNumberList = QList<MMGNumber>;

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

class MMGText {
public:
	static void mmgblog(int log_status, const QString &message);

	static QString asString(int num, const QString &prefix = "");

	static QString join(Translator translator, const QString &header, const QString &joiner);
	static QString choose(const char *header, const char *opt1, const char *opt2, bool decider);
	static QStringList batch(Translator translator, const QString &header, const QStringList &list);
};

class MMGJsonObject : public QObject {

public:
	MMGJsonObject(QObject *parent, const QJsonObject &json);

	void setEditable(bool edit) { editable = edit; };

	bool isEmpty() const { return json_obj.isEmpty(); };
	QStringList keys() const { return json_obj.keys(); };
	QJsonValue value(const QString &key) const { return json_obj[key]; };

	void json(QJsonObject &json, const QString &key) const { json[key] = json_obj; };
	void setJson(const QJsonObject &json) { MMG_ENABLED json_obj = json; };
	void copy(MMGJsonObject *dest) const { dest->json_obj = json_obj; };
	void clear() { json_obj = {}; };

	static QByteArray toString(const QJsonObject &json_obj);
	static QJsonObject toObject(const QByteArray &str);
	static QJsonObject toObject(const char *str) { return toObject(QByteArray(str)); };

private:
	QJsonObject json_obj;
	bool editable = true;
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

class MMGInterface {

public:
	static void setComboBoxItemEnabled(QComboBox *combo, int index, bool enable);
	static QIcon icon(const QString &icon_name);
	static bool promptUser(const QString &message_name, bool question = true);
};

} // namespace MMGUtils

#undef MMG_ENABLED
