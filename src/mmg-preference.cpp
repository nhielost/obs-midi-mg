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

#include "mmg-preference.h"
#include "mmg-config.h"

namespace MMGPreferences {

struct PreferenceTypeInfo {
	const char *preference_name;
	ConstructBase *init;
};

static QMap<Id, PreferenceTypeInfo> all_preference_types;

ConstructBase::ConstructBase(MMGPreference *preference)
{
	all_preference_types.insert(preference->id(), {preference->trPreferenceName(), this});
};

const MMGTranslationMap<Id> availablePreferences()
{
	MMGTranslationMap<Id> preferences;

	for (auto [id, info] : all_preference_types.asKeyValueRange()) {
		preferences.insert(id, mmgtr(MMGText::join("Preferences", info.preference_name)));
	}

	return preferences;
};

} // namespace MMGPreferences

// MMGPreference
MMGPreference::MMGPreference(MMGPreferenceManager *parent, const QJsonObject &) : QObject(parent) {};

void MMGPreference::blog(int log_status, const QString &message) const
{
	mmgblog(log_status,
		QString("[Preferences] <%1> %2").arg(MMGPreferences::availablePreferences()[id()]).arg(message));
}

MMGPreference *MMGPreference::generate(MMGPreferenceManager *parent, const QJsonObject &json_obj)
{
	auto init = MMGPreferences::all_preference_types[MMGJson::getValue<MMGPreferences::Id>(json_obj, "id")].init;
	return (*init)(parent, json_obj);
}
// End MMGPreference
