/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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

#include "mmg-action-profiles.h"

namespace MMGActions {

const MMGStringTranslationMap enumerateProfiles()
{
	MMGStringTranslationMap map;

	char **profile_names = obs_frontend_get_profiles();
	for (int i = 0; profile_names[i] != 0; ++i) {
		map.insert(profile_names[i], nontr(profile_names[i]));
	}
	bfree(profile_names);

	return map;
}

const MMGString currentProfile()
{
	char *char_profile = obs_frontend_get_current_profile();
	MMGString profile_text(char_profile);
	bfree(char_profile);
	return profile_text;
}

MMGParams<MMGString> MMGActionProfiles::profile_params {
	.desc = obstr("TitleBar.Profile"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = "",
	.bounds = {},
};

MMGActionProfiles::MMGActionProfiles(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  profile(json_obj, "profile")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionProfiles::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(profile, json_obj, "profile", 1, enumerateProfiles());
}

void MMGActionProfiles::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	profile->json(json_obj, "profile");
}

void MMGActionProfiles::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionProfiles *>(dest);
	if (!casted) return;

	profile.copy(casted->profile);
}

void MMGActionProfiles::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	profile_params.bounds = enumerateProfiles();
	profile_params.default_value = currentProfile();

	MMGActions::createActionField(display, &profile, &profile_params);
}

void MMGActionProfiles::execute(const MMGMappingTest &test) const
{
	ACTION_ASSERT(!obs_frontend_streaming_active(), "Profiles cannot be changed when streaming.");
	ACTION_ASSERT(!obs_frontend_recording_active(), "Profiles cannot be changed when recording.");
	ACTION_ASSERT(!obs_frontend_virtualcam_active(), "Profiles cannot be changed when using the virtual camera.");

	MMGString name = currentProfile();
	ACTION_ASSERT(test.applicable(profile, name), "A profile could not be selected. Check the Profile field and "
						      "try again.");
	ACTION_ASSERT(name != currentProfile(), "The profile is already active.");

	runInMainThread([name]() { obs_frontend_set_current_profile(name); });

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionProfiles::processEvent(obs_frontend_event event) const
{
	if (event != OBS_FRONTEND_EVENT_PROFILE_CHANGED) return;

	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(profile, currentProfile());
}

} // namespace MMGActions
