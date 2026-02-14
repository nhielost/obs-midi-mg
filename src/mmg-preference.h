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

#ifndef MMG_PREFERENCE_H
#define MMG_PREFERENCE_H

#include "mmg-json.h"

class MMGPreference;
template <class T> class MMGManager;
using MMGPreferenceManager = MMGManager<MMGPreference>;

namespace MMGPreferences {
enum Id : uint16_t;
}

class MMGPreference : public QObject {
	Q_OBJECT

public:
	MMGPreference(MMGPreferenceManager *parent, const QJsonObject &);
	virtual ~MMGPreference() = default;

	virtual MMGPreferences::Id id() const = 0;
	virtual const char *trPreferenceName() const = 0;

	void blog(int log_status, const QString &message) const;
	virtual void load(const QJsonObject &) {};
	virtual void json(QJsonObject &) const {};
	void copy(MMGPreference *) {};

	virtual void createDisplay(QWidget *) {};

	static MMGPreference *generate(MMGPreferenceManager *parent, const QJsonObject &json_obj);
};

namespace MMGPreferences {

enum Id : uint16_t {};

template <typename T>
concept IsMMGPreference =
	std::derived_from<T, MMGPreference> && std::constructible_from<T, MMGPreferenceManager *, const QJsonObject &>;

struct ConstructBase {
	ConstructBase(MMGPreference *action);

	virtual MMGPreference *operator()(MMGPreferenceManager *parent, const QJsonObject &json_obj) = 0;
};

template <typename T> requires IsMMGPreference<T> struct Construct : public ConstructBase {
	Construct() : ConstructBase(std::make_unique<T>(nullptr, QJsonObject()).get()) {};

	MMGPreference *operator()(MMGPreferenceManager *parent, const QJsonObject &json_obj) override
	{
		return new T(parent, json_obj);
	};
};

const MMGTranslationMap<Id> availablePreferences();

#define MMG_DECLARE_PREFERENCE(T) static Construct<T> _registration_##T {};

} // namespace MMGPreferences

#endif // MMG_PREFERENCE_H
