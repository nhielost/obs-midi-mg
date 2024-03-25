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

#include "mmg-settings.h"
#include "mmg-config.h"

#include <QAbstractItemView>

using namespace MMGUtils;

MMGSettings::MMGSettings(QObject *parent) : QObject(parent) {}

void MMGSettings::json(const QString &key, QJsonObject &json_obj) const {}

void MMGSettings::load(const QJsonObject &json_obj) {}