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

#ifndef MMG_BINDING_H
#define MMG_BINDING_H

#include "actions/mmg-action.h"
#include "messages/mmg-message.h"
#include "mmg-link.h"
#include "mmg-manager.h"

class MMGBinding;
using MMGBindingManager = MMGManager<MMGBinding>;

class MMGBinding : public QObject {
	Q_OBJECT

public:
	MMGBinding(MMGBindingManager *parent, const QJsonObject &json_obj = QJsonObject());

	enum ResetMode { BINDING_TRIGGERED, BINDING_CONTINUOUS };

	DeviceType type() const { return _type; };
	void setType(DeviceType type);

	bool enabled() const { return _enabled; };
	void setEnabled(bool val);

	void setConnected(bool connected);
	void refresh();

	ResetMode resetMode() const { return (ResetMode)reset_mode; };
	void setResetMode(short mode) { reset_mode = mode; }

	void blog(int log_status, const QString &message) const;
	void json(QJsonObject &binding_obj) const;
	void copy(MMGBinding *dest);

	MMGMessageManager *messages() const { return _messages; };
	MMGActionManager *actions() const { return _actions; };

	MMGMessage *messages(int index) const { return _messages->at(index); };
	MMGAction *actions(int index) const { return _actions->at(index); };

	static MMGBinding *generate(MMGBindingManager *parent, const QJsonObject &json_obj)
	{
		return new MMGBinding(parent, json_obj);
	};

private:
	DeviceType _type;
	bool _enabled;
	bool connected = false;
	short reset_mode = 0;

	MMGMessageManager *_messages;
	MMGActionManager *_actions;

	MMGLink *link;
};
MMG_DECLARE_STREAM_OPERATORS(MMGBinding);

using MMGCollections = MMGManager<MMGBindingManager>;
MMG_DECLARE_STREAM_OPERATORS(MMGBindingManager);
template <> MMGBindingManager *MMGBindingManager::generate(MMGCollections *parent, const QJsonObject &json_obj);

#endif // MMG_BINDING_H
