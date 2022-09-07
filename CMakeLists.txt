cmake_minimum_required(VERSION 3.16...3.21)

# Change obs-plugintemplate to your plugin's name in a machine-readable format
# (e.g.: obs-myawesomeplugin) and set
project(obs-midi-mg VERSION 1.3.0)
add_library(obs-midi-mg MODULE)

# Replace `Your Name Here` with the name (yours or your organization's) you want
# to see as the author of the plugin (in the plugin's metadata itself and in the
# installers)
set(PLUGIN_AUTHOR "nhielost <nhielost@gmail.com>")

# Replace `com.example.obs-plugin-template` with a unique Bundle ID for macOS
# releases (used both in the installer and when submitting the installer for
# notarization)
set(MACOS_BUNDLEID "com.nhielost.obs-midi-mg")

# Replace `me@contoso.com` with the maintainer email address you want to put in
# Linux packages
set(LINUX_MAINTAINER_EMAIL "nhielost@gmail.com")

# Include libremidi library
add_subdirectory(libremidi EXCLUDE_FROM_ALL)
include_directories(libremidi/include)
target_link_libraries(obs-midi-mg PRIVATE libremidi)

# Add your custom source files here - header files are optional and only
# required for visibility e.g. in Xcode or Visual Studio
target_sources(
  obs-midi-mg
  PRIVATE ./src/mmg-action.cpp
          ./src/mmg-binding.cpp
          ./src/mmg-config.cpp
          ./src/mmg-device.cpp
          ./src/mmg-message.cpp
          ./src/mmg-utils.cpp
          ./src/obs-midi-mg.cpp
          ./src/forms/midimg-window.cpp)

target_sources(
  obs-midi-mg
  PRIVATE ./src/mmg-action.h
          ./src/mmg-binding.h
          ./src/mmg-config.h
          ./src/mmg-device.h
          ./src/mmg-message.h
          ./src/mmg-utils.h
          ./src/obs-midi-mg.h
          ./src/forms/midimg-window.h)

# Import libobs as main plugin dependency
find_package(libobs REQUIRED)
include(cmake/ObsPluginHelpers.cmake)

# Uncomment these lines if you want to use the OBS Frontend API in your plugin

find_package(obs-frontend-api REQUIRED)
target_link_libraries(obs-midi-mg PRIVATE OBS::obs-frontend-api)

# Uncomment those lines if you want to use Qt in your plugin

find_qt(COMPONENTS Widgets Core)
target_link_libraries(obs-midi-mg PRIVATE Qt::Core Qt::Widgets)
set_target_properties(
  obs-midi-mg
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

configure_file(src/plugin-macros.h.in
               ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(obs-midi-mg PRIVATE src/plugin-macros.generated.h)

# /!\ TAKE NOTE: No need to edit things past this point /!\

# --- Platform-independent build settings ---

target_include_directories(obs-midi-mg PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(obs-midi-mg PRIVATE OBS::libobs)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  if(MSVC)
    target_compile_options(obs-midi-mg PRIVATE /W3)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macos.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macos.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1.3.0")

  target_compile_options(obs-midi-mg PRIVATE -Wall)
  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  target_compile_options(obs-midi-mg PRIVATE -Wall)
endif()
# --- End of section ---

setup_plugin_target(obs-midi-mg)
