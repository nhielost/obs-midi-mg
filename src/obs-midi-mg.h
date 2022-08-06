/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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
#include <string>

#include <QObject>

#include <obs.hpp>

#include "plugin-macros.generated.h"

class MMGConfig;
class MidiMGWindow;

using Configuration = QSharedPointer<MMGConfig>;
Configuration global();
static MidiMGWindow *plugin_window;

enum class MMGModes {
	MMGMODE_NONE,
	MMGMODE_PREFERENCES,
	MMGMODE_DEVICE,
	MMGMODE_BINDING,
	MMGMODE_MESSAGE,
	MMGMODE_ACTION
};
Q_DECLARE_METATYPE(MMGModes);

#define OBS_MIDIMG_VERSION "v" PLUGIN_VERSION
#define qtocs() toStdString().c_str()
