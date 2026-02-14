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

#ifndef MMG_ACTION_H
#define MMG_ACTION_H

#include "../mmg-mapping.h"
#include "../mmg-params.h"
#include "../mmg-signal.h"

class MMGAction;
template <class T> class MMGManager;
using MMGActionManager = MMGManager<MMGAction>;

namespace MMGWidgets {
class MMGActionDisplay;
} // namespace MMGWidgets

namespace MMGActions {
enum Id : uint16_t;
}

class MMGAction : public QObject {
	Q_OBJECT

protected:
	using EventFulfillment = MMGMappingFulfillment<MMGAction>;

public:
	MMGAction(MMGActionManager *parent, const QJsonObject &json_obj);
	virtual ~MMGAction() = default;

	virtual constexpr MMGActions::Id id() const = 0;
	virtual const char *categoryName() const = 0;
	virtual const char *trActionName() const = 0;

	virtual void initOldData(const QJsonObject &) {};

	void blog(int log_status, const QString &message) const;
	virtual void json(QJsonObject &action_obj) const;
	virtual void copy(MMGAction *dest) const;

	virtual void createDisplay(MMGWidgets::MMGActionDisplay *) {};

	virtual void execute(const MMGMappingTest &test) const = 0;
	virtual void connectSignal(bool connect) = 0;

	static MMGAction *generate(MMGActionManager *parent, const QJsonObject &json_obj);

protected:
	DeviceType type() const;

signals:
	void refreshRequested() const;
	void fulfilled(MMGMappingTest) const;
};
MMG_DECLARE_STREAM_OPERATORS(MMGAction);

namespace MMGActions {

enum Id : uint16_t {};

template <typename T>
concept IsMMGAction =
	std::derived_from<T, MMGAction> && std::constructible_from<T, MMGActionManager *, const QJsonObject &>;

struct ConstructBase {
	ConstructBase(MMGAction *action);

	virtual MMGAction *operator()(MMGActionManager *parent, const QJsonObject &json_obj) = 0;
};

template <typename T> requires IsMMGAction<T> struct Construct : public ConstructBase {
	Construct() : ConstructBase(std::make_unique<T>(nullptr, QJsonObject()).get()) {};

	MMGAction *operator()(MMGActionManager *parent, const QJsonObject &json_obj) override
	{
		return new T(parent, json_obj);
	};
};

const MMGTranslationMap<Id> availableActionCategories();
const MMGTranslationMap<Id> availableActions(Id category_id, DeviceType type);

MMGAction *generateAction(MMGActionManager *parent, const QJsonObject &json_obj);
bool changeAction(MMGActionManager *parent, MMGAction *&action, Id new_id);

template <typename T>
inline void createActionField(MMGWidgets::MMGActionDisplay *display, MMGValue<T> *storage, const MMGParams<T> *params,
			      const MMGCallback &cb = 0)
{
	MMGParameters::createField<T>((MMGWidgets::MMGValueManager *)(display), storage, params, cb);
};

#define MMG_DECLARE_ACTION(T) static Construct<T> _registration_##T {};

#define ACTION_ASSERT(cond, str)                \
	if (!(cond)) {                          \
		blog(LOG_INFO, "FAILED: " str); \
		return;                         \
	}

} // namespace MMGActions

#endif // MMG_ACTION_H
