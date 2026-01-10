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

#include "mmg-json.h"

#include <QColor>
#include <QDir>
#include <QFont>

namespace MMGJson {

// Conversions
template <> float convertTo(const QJsonValue &value)
{
	return value.toDouble();
};

template <> bool convertTo(const QJsonValue &value)
{
	return value.toBool();
};

template <> QString convertTo(const QJsonValue &value)
{
	return value.toString();
};

template <> MMGString convertTo(const QJsonValue &value)
{
	return qUtf8Printable(value.toString());
};

template <> QJsonValue convertFrom(const MMGString &value)
{
	return value.value();
};

qint64 convertColor(qint64 rgb)
{
	return (rgb & 0xFF000000) | ((rgb & 0x00FF0000) >> 16) | (rgb & 0x0000FF00) | ((rgb & 0x000000FF) << 16);
}

template <> QColor convertTo(const QJsonValue &value)
{
	return QColor::fromRgba(convertColor(value.toInteger()));
}

template <> QJsonValue convertFrom(const QColor &value)
{
	return convertColor(value.rgba());
}

template <> QFont convertTo(const QJsonValue &value)
{
	QFont json_font;
	QJsonObject json_obj = value.toObject();

	json_font.setFamily(json_obj["face"].toString());
	int flags = json_obj["flags"].toInt();
	json_font.setBold(flags & 0b0001);
	json_font.setItalic(flags & 0b0010);
	json_font.setUnderline(flags & 0b0100);
	json_font.setStrikeOut(flags & 0b1000);
	json_font.setPointSize(json_obj["size"].toInt());
	json_font.setStyleName(json_obj["style"].toString());

	return json_font;
}

template <> QJsonValue convertFrom(const QFont &value)
{
	QJsonObject json_obj;

	json_obj["face"] = value.family();
	json_obj["flags"] = (value.strikeOut() << 3) | (value.underline() << 2) | (value.italic() << 1) |
			    (value.bold() << 0);
	json_obj["size"] = value.pointSize();
	json_obj["style"] = value.styleName();

	return json_obj;
}

template <> QDir convertTo(const QJsonValue &value)
{
	return value.toString();
}

template <> QJsonValue convertFrom(const QDir &value)
{
	return value.absolutePath();
}
// End Conversions

QByteArray toString(const QJsonObject &json_obj)
{
	return QJsonDocument(json_obj).toJson(QJsonDocument::Compact);
}

QJsonObject toObject(const QByteArray &str)
{
	return QJsonDocument::fromJson(str).object();
}

QJsonObject toObject(const char *str)
{
	return toObject(QByteArray(str));
}

} // namespace MMGJson
