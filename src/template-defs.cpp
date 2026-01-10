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

#include <libremidi/libremidi.hpp>

#include "actions/mmg-action-scene-items.h"
#include "actions/mmg-action-sources.h"
#include "mmg-binding.h"
#include "mmg-midi.h"
#include "mmg-preference-defs.h"

#include "mmg-params.cpp"
#include "mmg-states.cpp"
#include "mmg-value.cpp"
#include "ui/mmg-state-widget.cpp"
#include "ui/mmg-value-display.cpp"

#define MMG_DECLARE_TEMPLATE_CLASS_BASIC(T_NAME)      \
	template class T_NAME<MMGMIDIPort *>;         \
	template class T_NAME<libremidi_api>;         \
	template class T_NAME<MMGBinding::ResetMode>; \
	template class T_NAME<MMGPreferences::MMGPreferenceMIDI::MessageMode>

#define MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(T_NAME) \
	template class T_NAME<uint8_t>;            \
	template class T_NAME<uint16_t>;           \
	template class T_NAME<uint32_t>;           \
	template class T_NAME<uint64_t>;           \
	template class T_NAME<int32_t>;            \
	template class T_NAME<int64_t>;            \
	template class T_NAME<float>

#define MMG_DECLARE_TEMPLATE_CLASS_BOOL(T_NAME) template class T_NAME<bool>

#define MMG_DECLARE_TEMPLATE_CLASS_SPECIALIZED(T_NAME) \
	template class T_NAME<MMGString>;              \
	template class T_NAME<QColor>;                 \
	template class T_NAME<QFont>;                  \
	template class T_NAME<QDir>;                   \
	template class T_NAME<QString>

#define MMG_DECLARE_TEMPLATE_CLASS_ACTION_FIELDS(T_NAME)  \
	template class T_NAME<MMGStates::ReferenceIndex>; \
	template class T_NAME<MMGMessages::Id>;           \
	template class T_NAME<MMGActions::Id>;            \
	template class T_NAME<MMGActions::Alignment>;     \
	template class T_NAME<obs_scale_type>;            \
	template class T_NAME<obs_blending_type>;         \
	template class T_NAME<obs_bounds_type>;           \
	template class T_NAME<obs_monitoring_type>;       \
	template class T_NAME<MMGActions::MMGActionSourcesMediaState::Actions>

#define MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(T_NAME)     \
	MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(T_NAME);     \
	MMG_DECLARE_TEMPLATE_CLASS_BOOL(T_NAME);        \
	MMG_DECLARE_TEMPLATE_CLASS_SPECIALIZED(T_NAME); \
	MMG_DECLARE_TEMPLATE_CLASS_ACTION_FIELDS(T_NAME)

#define MMG_DECLARE_TEMPLATE_CLASS_NO_BOOL(T_NAME)      \
	MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(T_NAME);     \
	MMG_DECLARE_TEMPLATE_CLASS_SPECIALIZED(T_NAME); \
	MMG_DECLARE_TEMPLATE_CLASS_ACTION_FIELDS(T_NAME)

#define MMG_DECLARE_TEMPLATE_CLASS_NO_SPECIALIZED(T_NAME) \
	MMG_DECLARE_TEMPLATE_CLASS_BASIC(T_NAME);         \
	MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(T_NAME);       \
	MMG_DECLARE_TEMPLATE_CLASS_ACTION_FIELDS(T_NAME)

#define MMG_DECLARE_TEMPLATE_CLASS_ALL(T_NAME)    \
	MMG_DECLARE_TEMPLATE_CLASS_BASIC(T_NAME); \
	MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(T_NAME)

MMG_DECLARE_TEMPLATE_CLASS_ALL(MMGValue);
MMG_DECLARE_TEMPLATE_CLASS_NO_SPECIALIZED(MMGParameters::MMGParamsMap);
MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(MMGParameters::Construct);

namespace MMGStates {

MMG_DECLARE_TEMPLATE_CLASS_ALL(Fixed);
MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(MIDIMap);
MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(MIDIRange);
MMG_DECLARE_TEMPLATE_CLASS_NO_BOOL(Toggle);
MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(Increment);
MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(Ignore);

} // namespace MMGStates

namespace MMGWidgets {

MMG_DECLARE_TEMPLATE_CLASS_ALL(MMGValueDisplay);
MMG_DECLARE_TEMPLATE_CLASS_ALL(MMGValueFixedDisplay);
MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(MMGValueStateDisplay);

namespace MMGStateWidgets {

MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(Construct);
MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(Fixed);
MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(MIDIMap);
MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(MIDIRange);
MMG_DECLARE_TEMPLATE_CLASS_NO_BOOL(Toggle);
MMG_DECLARE_TEMPLATE_CLASS_NUMERIC(Increment);
MMG_DECLARE_TEMPLATE_CLASS_NO_BASIC(Ignore);

} // namespace MMGStateWidgets

} // namespace MMGWidgets
