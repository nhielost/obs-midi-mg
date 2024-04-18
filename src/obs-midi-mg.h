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

#include <QObject>

#include <obs.hpp>
#include <obs-module.h>
#include <obs-frontend-api.h>

#include <libremidi/libremidi.hpp>

class MMGConfig;
MMGConfig *config();

#define OBS_MIDIMG_VERSION "v3.0.3"

#define qtocs() toUtf8().constData()
#define mmgtocs() value().qtocs()
#define mmgtr(str) obs_module_text(str)
#define obstr(str) obs_frontend_get_locale_string(str)
