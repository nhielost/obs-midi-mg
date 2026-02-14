/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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

#ifndef MMG_JSON_H
#define MMG_JSON_H

#include "mmg-string.h"
#include "obs-midi-mg.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace MMGJson {

template <typename T> concept MMGJsonEligibleInteger = MMGIsInteger<T> || std::is_enum_v<T>;
template <typename T> concept MMGJsonEligible = !std::is_pointer_v<T> && !MMGJsonEligibleInteger<T>;
template <typename T> concept MMGJsonIneligible = std::is_pointer_v<T>;

// Conversions
template <typename T> requires MMGJsonEligible<T> T convertTo(const QJsonValue &value);
template <typename T> requires MMGJsonEligible<T> inline QJsonValue convertFrom(const T &value)
{
	return value;
};

template <typename T> requires MMGJsonEligibleInteger<T> inline QJsonValue convertFrom(const T &value)
{
	return QJsonValue(qint64(value));
};

template <typename T> requires MMGJsonEligibleInteger<T> inline T convertTo(const QJsonValue &value)
{
	return (T)(value.toInteger());
};

template <> float convertTo(const QJsonValue &value);
template <> bool convertTo(const QJsonValue &value);
template <> QString convertTo(const QJsonValue &value);

template <> MMGString convertTo(const QJsonValue &value);
template <> QJsonValue convertFrom(const MMGString &value);

template <> QColor convertTo(const QJsonValue &value);
template <> QJsonValue convertFrom(const QColor &value);

template <> QFont convertTo(const QJsonValue &value);
template <> QJsonValue convertFrom(const QFont &value);

template <> QDir convertTo(const QJsonValue &value);
template <> QJsonValue convertFrom(const QDir &value);
// End Conversions

QByteArray toString(const QJsonObject &json_obj);
QJsonObject toObject(const QByteArray &str);
QJsonObject toObject(const char *str);

template <typename T> inline T getValue(const QJsonObject &json_obj, const char *key)
{
	return convertTo<T>(json_obj[key]);
};

template <typename T> inline QList<T> getList(const QJsonObject &json_obj, const char *key)
{
	QList<T> list;

	for (QJsonValue json_val : json_obj[key].toArray())
		list += convertTo<T>(json_val);

	return list;
};

template <typename T> inline QMap<int64_t, T> getMap(const QJsonObject &json_obj, const char *key)
{
	QMap<int64_t, T> map;

	QJsonObject map_obj = json_obj[key].toObject();
	for (const QString &key : map_obj.keys())
		map.insert(key.toLongLong(), convertTo<T>(map_obj[key]));

	return map;
};

template <typename T> inline void setValue(QJsonObject &json_obj, const char *key, const T &data)
{
	json_obj[key] = convertFrom(data);
};

template <typename T> inline void setList(QJsonObject &json_obj, const char *key, const QList<T> &data)
{
	QJsonArray json_arr;
	for (const T &t : data)
		json_arr += convertFrom(t);
	json_obj[key] = json_arr;
};

template <typename T> inline void setMap(QJsonObject &json_obj, const char *key, const QMap<int64_t, T> &data)
{
	QJsonObject map_obj;
	for (const auto &[key, t] : data.asKeyValueRange())
		map_obj[QString::number(key)] = convertFrom(t);
	json_obj[key] = map_obj;
};

} // namespace MMGJson

#endif // MMG_JSON_H
