cmake_minimum_required(VERSION 3.16...3.21)

# Change obs-plugintemplate to your plugin's name in a machine-readable format
# (e.g.: obs-myawesomeplugin) and set
project(obs-midi-mg VERSION 1.0.0)
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

set(LIBREMIDI_HEADER_ONLY
    ON
    CACHE BOOL "Header-only mode")
add_subdirectory(libremidi EXCLUDE_FROM_ALL)
include_directories(libremidi/include)

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

# /!\ TAKE NOTE: No need to edit things past this point /!\

find_package(libobs REQUIRED)
find_package(obs-frontend-api REQUIRED)
include(cmake/ObsPluginHelpers.cmake)
find_qt(VERSION ${QT_VERSION} COMPONENTS Widgets Core)

configure_file(src/plugin-macros.h.in
               ${CMAKE_SOURCE_DIR}/src/plugin-macros.generated.h)

target_sources(obs-midi-mg PRIVATE ./src/plugin-macros.generated.h)

# --- Platform-independent build settings ---

target_include_directories(obs-midi-mg PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(obs-midi-mg PRIVATE OBS::libobs OBS::obs-frontend-api
                                          Qt::Core Qt::Widgets libremidi)

set_target_properties(
  obs-midi-mg
  PROPERTIES AUTOMOC ON
             AUTOUIC ON
             AUTORCC ON)

target_compile_features(obs-midi-mg PRIVATE cxx_std_17)

# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  if(MSVC)
    target_compile_options(obs-midi-mg PRIVATE /MP /d2FH4-)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
elseif(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macOS.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macOS.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")

  target_compile_options(
    obs-midi-mg PRIVATE -Wall -Wextra -Werror-implicit-function-declaration
                        -stdlib=libc++ -fvisibility=default)

  set_target_properties(obs-midi-mg PROPERTIES PREFIX "")
  # --- End of section ---

  # --- Linux-specific build settings and tasks ---
else()
  target_compile_options(obs-midi-mg PRIVATE -Wall -Wextra)

  set_target_properties(obs-midi-mg PROPERTIES PREFIX "")
endif()
# --- End of section ---

setup_plugin_target(obs-midi-mg)
