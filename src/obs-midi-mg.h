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
#include <memory>

#include <QObject>

#include <obs.hpp>

#include <libremidi/libremidi.hpp>

#include "plugin-macros.generated.h"

class MMGMIDIIn;
class MMGMIDIOut;
class MMGConfig;
class MMGEchoWindow;

using Configuration = QSharedPointer<MMGConfig>;
Configuration global();

using MMGMIDIInDevice = QSharedPointer<MMGMIDIIn>;
using MMGMIDIOutDevice = QSharedPointer<MMGMIDIOut>;

MMGMIDIInDevice input_device();
MMGMIDIOutDevice output_device();

#define OBS_MIDIMG_VERSION "v" PLUGIN_VERSION
#define qtocs() toUtf8().constData()
#define mmgtocs() str().qtocs()
