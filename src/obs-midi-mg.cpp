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

#include "mmg-config.h"
#include "ui/mmg-echo-window.h"

#include <QAction>
#include <QDir>
#include <QMainWindow>

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-midi-mg", "en-US")

static MMGEchoWindow *echo_window = nullptr;
static MMGConfig *global_config;

QDataStream &operator<<(QDataStream &out, const QObject *&obj)
{
	return out << *(quint64 *)(&obj);
}

QDataStream &operator>>(QDataStream &in, QObject *&obj)
{
	quint64 data;
	in >> data;
	obj = *(QObject **)(&data);
	return in;
}

void mmgblog(int log_status, const QString &message)
{
	blog(log_status, "[obs-midi-mg] %s", qUtf8Printable(message));
}

void runInMainThread(const MMGCallback &func)
{
	obs_queue_task(
		OBS_TASK_UI,
		[](void *ptr) {
			auto *func = static_cast<MMGCallback *>(ptr);
			if (!func) return;

			(*func)();

			delete func;
		},
		new MMGCallback(func), false);
}

const char *obs_module_name(void)
{
	return "obs-midi-mg";
}

const char *obs_module_description(void)
{
	return mmgtr("Plugin.Description");
}

static void showUI()
{
	if (!echo_window) {
		mmgblog(LOG_INFO, "Loading plugin interface using the Echo style...");
		echo_window = new MMGEchoWindow((QWidget *)obs_frontend_get_main_window());
		mmgblog(LOG_INFO, "Plugin interface loaded.");
	}
	echo_window->displayWindow();
}

bool obs_module_load(void)
{
	mmgblog(LOG_INFO, "Loading plugin (" OBS_MIDIMG_VERSION_DISPLAY ")...");

	// Create the obs-midi-mg directory in plugin_config if it doesn't exist
	auto *config_path = obs_module_config_path("");
	if (!QDir(config_path).exists()) QDir().mkdir(config_path);
	bfree(config_path);

	// Load the configuration
	global_config = new MMGConfig;
	global_config->load();
	QObject::connect(global_config, &MMGConfig::refreshRequested, showUI);

	// Load the UI Window and the menu button (Tools -> obs-midi-mg Setup)
	auto *menu_action = (QAction *)obs_frontend_add_tools_menu_qaction(mmgtr("Plugin.ToolsButton"));
	QObject::connect(menu_action, &QAction::triggered, showUI);

	// Done
	mmgblog(LOG_INFO, "Plugin loaded.");
	return true;
}

void obs_module_unload()
{
	delete global_config;
	mmgblog(LOG_INFO, "Plugin unloaded.");
}

MMGConfig *config()
{
	return global_config;
}
