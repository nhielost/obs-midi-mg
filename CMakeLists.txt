cmake_minimum_required(VERSION 3.16...3.26)

# Make linking on Windows static
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)

project(${_name} VERSION ${_version})

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" OFF)
option(ENABLE_QT "Use Qt functionality" OFF)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${CMAKE_PROJECT_NAME} MODULE)

find_package(libobs REQUIRED)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)

if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets Core)
  target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Widgets)
  target_compile_options(
    ${CMAKE_PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header
                                  -Wno-comma>)
  set_target_properties(
    ${CMAKE_PROJECT_NAME}
    PROPERTIES AUTOMOC ON
               AUTOUIC ON
               AUTORCC ON)
endif()

# Include libremidi library
set(CMAKE_CXX_STANDARD 20)
add_subdirectory(libremidi EXCLUDE_FROM_ALL)
target_include_directories(${CMAKE_PROJECT_NAME} SYSTEM PRIVATE libremidi/include)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE libremidi)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ./src/actions/mmg-action.cpp
          ./src/actions/mmg-action-none.cpp
          ./src/actions/mmg-action-stream.cpp
          ./src/actions/mmg-action-record.cpp
          ./src/actions/mmg-action-virtualcam.cpp
          ./src/actions/mmg-action-replaybuffer.cpp
          ./src/actions/mmg-action-studiomode.cpp
          ./src/actions/mmg-action-scenes.cpp
          ./src/actions/mmg-action-video-sources.cpp
          ./src/actions/mmg-action-audio-sources.cpp
          ./src/actions/mmg-action-media-sources.cpp
          ./src/actions/mmg-action-transitions.cpp
          ./src/actions/mmg-action-filters.cpp
          ./src/actions/mmg-action-hotkeys.cpp
          ./src/actions/mmg-action-profiles.cpp
          ./src/actions/mmg-action-collections.cpp
          ./src/actions/mmg-action-midi.cpp
          ./src/mmg-binding.cpp
          ./src/mmg-config.cpp
          ./src/mmg-device.cpp
          ./src/mmg-link.cpp
          ./src/mmg-message.cpp
          ./src/mmg-manager.cpp
          ./src/mmg-midi.cpp
          ./src/mmg-settings.cpp
          ./src/mmg-signal.cpp
          ./src/mmg-utils.cpp
          ./src/obs-midi-mg.cpp
          ./src/ui/mmg-fields.cpp
          ./src/ui/mmg-lcd-number.cpp
          ./src/ui/mmg-midi-buttons.cpp
          ./src/ui/mmg-number-display.cpp
          ./src/ui/mmg-string-display.cpp
          ./src/ui/mmg-message-display.cpp
          ./src/ui/mmg-action-display.cpp
          ./src/ui/mmg-manager-control.cpp
          ./src/ui/mmg-echo-window.cpp)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ./src/actions/mmg-action.h
          ./src/actions/mmg-action-none.h
          ./src/actions/mmg-action-stream.h
          ./src/actions/mmg-action-record.h
          ./src/actions/mmg-action-virtualcam.h
          ./src/actions/mmg-action-replaybuffer.h
          ./src/actions/mmg-action-studiomode.h
          ./src/actions/mmg-action-scenes.h
          ./src/actions/mmg-action-video-sources.h
          ./src/actions/mmg-action-audio-sources.h
          ./src/actions/mmg-action-media-sources.h
          ./src/actions/mmg-action-transitions.h
          ./src/actions/mmg-action-filters.h
          ./src/actions/mmg-action-hotkeys.h
          ./src/actions/mmg-action-profiles.h
          ./src/actions/mmg-action-collections.h
          ./src/actions/mmg-action-midi.h
          ./src/mmg-binding.h
          ./src/mmg-config.h
          ./src/mmg-device.h
          ./src/mmg-link.h
          ./src/mmg-manager.h
          ./src/mmg-message.h
          ./src/mmg-midi.h
          ./src/mmg-settings.h
          ./src/mmg-signal.h
          ./src/mmg-utils.h
          ./src/obs-midi-mg.h
          ./src/ui/mmg-fields.h
          ./src/ui/mmg-lcd-number.h
          ./src/ui/mmg-midi-buttons.h
          ./src/ui/mmg-number-display.h
          ./src/ui/mmg-string-display.h
          ./src/ui/mmg-message-display.h
          ./src/ui/mmg-action-display.h
          ./src/ui/mmg-manager-control.h
          ./src/ui/mmg-echo-window.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ./src/ui/resources.qrc)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
