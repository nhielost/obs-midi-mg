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

#include "obs-midi-mg.h"

#include "ui/mmg-echo-window.h"
#include "mmg-config.h"
#include "mmg-midiin.h"
#include "mmg-midiout.h"

#include <QAction>
#include <QMainWindow>
#include <QDir>

#include <obs-module.h>
#include <obs-frontend-api.h>

using namespace std;

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("obs-midi-mg", "en-US")

static MMGEchoWindow *echo_window;
Configuration global_config;
MMGMIDIInDevice input;
MMGMIDIOutDevice output;

void _blog(int log_status, const QString &message)
{
  QString temp_msg = "Main -> ";
  temp_msg.append(message);
  global_blog(log_status, temp_msg);
}

bool obs_module_load(void)
{
  _blog(LOG_INFO, "Loading plugin...");

  // Create the obs-midi-mg dir in plugin_config if it doesn't exist
  auto *config_path = obs_module_config_path("");
  if (!QDir(config_path).exists()) QDir().mkdir(config_path);
  bfree(config_path);

  // Load the libremidi device objects
  input.reset(new MMGMIDIIn);
  output.reset(new MMGMIDIOut);

  // Load the configuration
  global_config.reset(new MMGConfig());

  // Load any new devices and open the input port
  global_config->refresh();

  // Load the UI Window and the menu button (Tools -> obs-midi-mg Setup)
  const char *menu_action_text = obs_module_text("obs-midi-mg Setup");
  auto *menu_action = (QAction *)obs_frontend_add_tools_menu_qaction(menu_action_text);
  auto *mainWindow = (QMainWindow *)obs_frontend_get_main_window();
  _blog(LOG_INFO, "Loading plugin using the Echo style...");
  echo_window = new MMGEchoWindow(mainWindow);
  QObject::connect(menu_action, &QAction::triggered, echo_window, &MMGEchoWindow::show_window);

  // Done
  _blog(LOG_INFO, "Plugin loaded.");
  return true;
}

void obs_module_unload()
{
  global_config.reset();
  input.reset();
  output.reset();
  _blog(LOG_INFO, "Plugin unloaded.");
}

Configuration global()
{
  return global_config;
}

MMGMIDIInDevice input_device()
{
  return input;
}

MMGMIDIOutDevice output_device()
{
  return output;
}
