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

#include "mmg-action.h"

void MMGAction::blog(int log_status, const QString &message) const
{
  global_blog(log_status, "Actions -> " + message);
}

void MMGAction::json(QJsonObject &json_obj) const
{
  json_obj["category"] = (int)category();
  json_obj["sub"] = subcategory;
}

void MMGAction::copy(MMGAction *dest) const
{
  dest->setSub(subcategory);
}