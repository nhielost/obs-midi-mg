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
#include <memory>

#include <QDir>
#include <QLayout>
#include <QObject>
#include <QWidget>

#include <obs-frontend-api.h>
#include <obs-module.h>
#include <obs.hpp>

#define OBS_MIDIMG_VERSION "v3.1.0"

class MMGConfig;
MMGConfig *config();

enum DeviceType { TYPE_NONE = -1, TYPE_INPUT, TYPE_OUTPUT };

template <typename T> concept MMGIsInteger = std::is_integral_v<T> && !std::is_same_v<T, bool>;
template <typename T> concept MMGIsNumeric = MMGIsInteger<T> || std::is_floating_point_v<T>;
using MMGCallback = std::function<void()>;

void mmgblog(int log_status, const QString &message);
void runInMainThread(const MMGCallback &func);

QDataStream &operator<<(QDataStream &out, const QObject *&obj);
QDataStream &operator>>(QDataStream &in, QObject *&obj);

#define MMG_DECLARE_STREAM_OPERATORS(T)                                 \
	inline QDataStream &operator<<(QDataStream &out, const T *&obj) \
	{                                                               \
		return out << (const QObject *&)obj;                    \
	}                                                               \
	inline QDataStream &operator>>(QDataStream &in, T *&obj)        \
	{                                                               \
		return in >> (QObject *&)obj;                           \
	}
