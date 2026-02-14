cmake_minimum_required(VERSION 3.28...3.30)

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

include(libremidi)

configure_file(src/template-defs.h.in "${CMAKE_CURRENT_SOURCE_DIR}/src/template-defs.h")

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ./src/template-defs.cpp
          ./src/actions/mmg-action.cpp
          ./src/actions/mmg-action-none.cpp
          ./src/actions/mmg-action-stream.cpp
          ./src/actions/mmg-action-record.cpp
          ./src/actions/mmg-action-virtualcam.cpp
          ./src/actions/mmg-action-replaybuffer.cpp
          ./src/actions/mmg-action-studiomode.cpp
          ./src/actions/mmg-action-scenes.cpp
          ./src/actions/mmg-action-scene-items.cpp
          ./src/actions/mmg-action-sources.cpp
          ./src/actions/mmg-action-transitions.cpp
          ./src/actions/mmg-action-filters.cpp
          ./src/actions/mmg-action-hotkeys.cpp
          ./src/actions/mmg-action-profiles.cpp
          ./src/actions/mmg-action-collections.cpp
          ./src/actions/mmg-action-midi.cpp
          ./src/messages/mmg-message.cpp
          ./src/messages/mmg-message-data.cpp
          ./src/messages/mmg-message-channel-voice.cpp
          ./src/mmg-binding.cpp
          ./src/mmg-config.cpp
          ./src/mmg-device.cpp
          ./src/mmg-json.cpp
          ./src/mmg-link.cpp
          ./src/mmg-manager.cpp
          ./src/mmg-mapping.cpp
          ./src/mmg-midi.cpp
          ./src/mmg-obs-object.cpp
          ./src/mmg-params.cpp
          ./src/mmg-preference.cpp
          ./src/mmg-preference-defs.cpp
          ./src/mmg-signal.cpp
          ./src/mmg-states.cpp
          ./src/mmg-string.cpp
          ./src/mmg-value.cpp
          ./src/obs-midi-mg.cpp
          ./src/ui/mmg-value-widget.cpp
          ./src/ui/mmg-lcd-number.cpp
          ./src/ui/mmg-combo-box.cpp
          ./src/ui/mmg-toggle-widget.cpp
          ./src/ui/mmg-dialog-widgets.cpp
          ./src/ui/mmg-string-widget.cpp
          ./src/ui/mmg-value-display.cpp
          ./src/ui/mmg-value-manager.cpp
          ./src/ui/mmg-state-widget.cpp
          ./src/ui/mmg-state-display.cpp
          ./src/ui/mmg-message-display.cpp
          ./src/ui/mmg-action-display.cpp
          ./src/ui/mmg-manager-display.cpp
          ./src/ui/mmg-echo-window.cpp)

target_sources(
  ${CMAKE_PROJECT_NAME}
  PRIVATE ./src/template-defs.h
          ./src/actions/mmg-action.h
          ./src/actions/mmg-action-none.h
          ./src/actions/mmg-action-stream.h
          ./src/actions/mmg-action-record.h
          ./src/actions/mmg-action-virtualcam.h
          ./src/actions/mmg-action-replaybuffer.h
          ./src/actions/mmg-action-studiomode.h
          ./src/actions/mmg-action-scenes.h
          ./src/actions/mmg-action-scene-items.h
          ./src/actions/mmg-action-sources.h
          ./src/actions/mmg-action-transitions.h
          ./src/actions/mmg-action-filters.h
          ./src/actions/mmg-action-hotkeys.h
          ./src/actions/mmg-action-profiles.h
          ./src/actions/mmg-action-collections.h
          ./src/actions/mmg-action-midi.h
          ./src/messages/mmg-message.h
          ./src/messages/mmg-message-data.h
          ./src/messages/mmg-message-channel-voice.h
          ./src/mmg-binding.h
          ./src/mmg-config.h
          ./src/mmg-device.h
          ./src/mmg-json.h
          ./src/mmg-link.h
          ./src/mmg-manager.h
          ./src/mmg-mapping.h
          ./src/mmg-midi.h
          ./src/mmg-obs-object.h
          ./src/mmg-params.h
          ./src/mmg-preference.h
          ./src/mmg-preference-defs.h
          ./src/mmg-signal.h
          ./src/mmg-states.h
          ./src/mmg-string.h
          ./src/mmg-value.h
          ./src/obs-midi-mg.h
          ./src/ui/mmg-value-widget.h
          ./src/ui/mmg-lcd-number.h
          ./src/ui/mmg-combo-box.h
          ./src/ui/mmg-toggle-widget.h
          ./src/ui/mmg-dialog-widgets.h
          ./src/ui/mmg-string-widget.h
          ./src/ui/mmg-value-display.h
          ./src/ui/mmg-value-manager.h
          ./src/ui/mmg-state-widget.h
          ./src/ui/mmg-state-display.h
          ./src/ui/mmg-message-display.h
          ./src/ui/mmg-action-display.h
          ./src/ui/mmg-manager-display.h
          ./src/ui/mmg-echo-window.h)

target_sources(${CMAKE_PROJECT_NAME} PRIVATE ./src/ui/resources.qrc)

set_target_properties_plugin(${CMAKE_PROJECT_NAME} PROPERTIES OUTPUT_NAME ${_name})
