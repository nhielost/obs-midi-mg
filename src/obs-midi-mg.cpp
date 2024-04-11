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

#include "obs-midi-mg.h"

#include "ui/mmg-echo-window.h"
#include "mmg-config.h"

#include <QAction>
#include <QMainWindow>
#include <QDir>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-midi-mg", "en-US")

static MMGEchoWindow *echo_window;
static MMGConfig *global_config;
static bool ui_init = false;

const char *obs_module_name(void)
{
	return "obs-midi-mg";
}

const char *obs_module_description(void)
{
	return mmgtr("Plugin.Description");
}

static void show_ui()
{
	if (!ui_init) {
		MMGText::mmgblog(LOG_INFO, "Loading plugin interface using the Echo style...");
		echo_window = new MMGEchoWindow((QWidget *)obs_frontend_get_main_window());
		MMGText::mmgblog(LOG_INFO, "Plugin interface loaded.");

		ui_init = true;
	}
	echo_window->displayWindow();
}

bool obs_module_load(void)
{
	MMGText::mmgblog(LOG_INFO, "Loading plugin (" OBS_MIDIMG_VERSION ")...");

	// Create the obs-midi-mg directory in plugin_config if it doesn't exist
	auto *config_path = obs_module_config_path("");
	if (!QDir(config_path).exists()) QDir().mkdir(config_path);
	bfree(config_path);

	// Load the configuration
	global_config = new MMGConfig;
	global_config->load();

	// Load the UI Window and the menu button (Tools -> obs-midi-mg Setup)
	auto *menu_action = (QAction *)obs_frontend_add_tools_menu_qaction(mmgtr("Plugin.ToolsButton"));
	QObject::connect(menu_action, &QAction::triggered, show_ui);

	// Done
	MMGText::mmgblog(LOG_INFO, "Plugin loaded.");
	return true;
}

void obs_module_unload()
{
	delete global_config;
	MMGText::mmgblog(LOG_INFO, "Plugin unloaded.");
}

MMGConfig *config()
{
	return global_config;
}
