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

#include "obs-midi-mg.h"

#include "forms/midimg-window.h"
#include "mmg-config.h"

#include <QAction>
#include <QMainWindow>

#include <obs-module.h>
#include <obs-frontend-api.h>

using namespace std;

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-midi-mg", "en-US")

static MidiMGWindow *plugin_window;
Configuration global_config;

bool obs_module_load(void)
{
	blog(LOG_INFO, "Loading...");
	global_config = Configuration(new MMGConfig());
	auto *mainWindow = (QMainWindow *)obs_frontend_get_main_window();
	plugin_window = new MidiMGWindow(mainWindow);
	const char *menu_action_text = obs_module_text("obs-midi-mg Setup");
	auto *menu_action = (QAction *)obs_frontend_add_tools_menu_qaction(
		menu_action_text);
	QObject::connect(menu_action, &QAction::triggered, plugin_window,
			 &MidiMGWindow::show_window);
	blog(LOG_INFO, "Loading Successful");
	return true;
}

void obs_module_unload()
{
	global_config.reset();
}

Configuration global()
{
	return global_config;
}
