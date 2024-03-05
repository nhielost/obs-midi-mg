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

#include "mmg-action-profiles.h"

using namespace MMGUtils;

MMGActionProfiles::MMGActionProfiles(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj), profile(json_obj, "profile", 1)
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionProfiles::subNames() const
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
		default:
			opts << subModuleText("Switch");
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"Changing", "Changed", "Toggle"});
			break;
	}

	return opts;
}

void MMGActionProfiles::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	profile.json(json_obj, "profile");
}

void MMGActionProfiles::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionProfiles *>(dest);
	if (!casted) return;

	casted->profile = profile.copy();
}

void MMGActionProfiles::setEditable(bool edit)
{
	profile.setEditable(edit);
}

void MMGActionProfiles::toggle()
{
	profile.toggle();
}

void MMGActionProfiles::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	display()->addNew(&profile)->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
}

void MMGActionProfiles::setActionParams()
{
	MMGStringDisplay *profile_display = display()->stringDisplay(0);
	profile_display->setVisible(true);
	profile_display->setDescription(obstr("TitleBar.Profile"));
	profile_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
	profile_display->setBounds(enumerate());
}

const QStringList MMGActionProfiles::enumerate()
{
	QStringList list;

	char **profile_names = obs_frontend_get_profiles();
	for (int i = 0; profile_names[i] != 0; ++i) {
		list.append(profile_names[i]);
	}
	bfree(profile_names);

	return list;
}

const QString MMGActionProfiles::currentProfile()
{
	char *char_profile = obs_frontend_get_current_profile();
	QString q_profile = char_profile;
	bfree(char_profile);
	return q_profile;
}

void MMGActionProfiles::execute(const MMGMessage *midi) const
{
	ACTION_ASSERT(sub() == PROFILE_PROFILE, "Invalid action.");

	ACTION_ASSERT(!obs_frontend_streaming_active(), "Profiles cannot be changed when streaming.");
	ACTION_ASSERT(!obs_frontend_recording_active(), "Profiles cannot be changed when recording.");
	ACTION_ASSERT(!obs_frontend_virtualcam_active(), "Profiles cannot be changed when using the virtual camera.");

	QString name = profile.chooseFrom(midi, enumerate());
	ACTION_ASSERT(name != currentProfile(), "The profile is already active.");

	obs_queue_task(
		OBS_TASK_UI,
		[](void *param) {
			auto profile_name = (QString *)param;
			obs_frontend_set_current_profile(profile_name->qtocs());
		},
		&name, true);

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionProfiles::frontendEventReceived(obs_frontend_event event)
{
	MMGNumber values;
	const QStringList profiles = enumerate();

	switch (sub()) {
		case PROFILE_CHANGING:
			if (event != OBS_FRONTEND_EVENT_PROFILE_CHANGING) return;
			values = profiles.indexOf(currentProfile());
			break;

		case PROFILE_CHANGED:
			if (event != OBS_FRONTEND_EVENT_PROFILE_CHANGED) return;
			values = profiles.indexOf(currentProfile());
			break;

		case PROFILE_TOGGLE_CHANGING:
			if (event != OBS_FRONTEND_EVENT_PROFILE_CHANGING && event != OBS_FRONTEND_EVENT_PROFILE_CHANGED)
				return;
			values = profiles.indexOf(currentProfile());
			break;

		default:
			return;
	}

	if (profile.chooseTo(values, profiles)) return;
	triggerEvent({values});
}
