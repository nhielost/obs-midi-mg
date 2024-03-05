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

#ifndef MMG_BINDING_H
#define MMG_BINDING_H

#include "mmg-message.h"
#include "actions/mmg-action.h"
#include "mmg-link.h"

class MMGBindingManager;

class MMGBinding : public QObject {
	Q_OBJECT

public:
	MMGBinding(MMGBindingManager *parent, const QJsonObject &json_obj = QJsonObject());

	enum ResetMode { MMGBINDING_TRIGGERED, MMGBINDING_CONTINUOUS };

	MMGUtils::DeviceType type() const { return _type; };
	void setType(MMGUtils::DeviceType type);

	bool enabled() const { return _enabled; };
	void setEnabled(bool val);

	void setConnected(bool connected);
	void refresh();

	ResetMode resetMode() const { return (ResetMode)reset_mode; };
	void setResetMode(short mode) { reset_mode = mode; }

	void blog(int log_status, const QString &message) const;
	void json(QJsonObject &binding_obj) const;
	void copy(MMGBinding *dest);
	void toggle();

	MMGMessageManager *messages() const { return _messages; };
	MMGActionManager *actions() const { return _actions; };

	MMGMessage *messages(int index) const { return _messages->at(index); };
	MMGAction *actions(int index) const { return _actions->at(index); };

private:
	MMGUtils::DeviceType _type;
	bool _enabled;
	bool connected = false;
	short reset_mode = 0;

	MMGMessageManager *_messages;
	MMGActionManager *_actions;

	MMGLink *link;
};
QDataStream &operator<<(QDataStream &out, const MMGBinding *&obj);
QDataStream &operator>>(QDataStream &in, MMGBinding *&obj);

class MMGBindingManager : public MMGManager<MMGBinding> {

public:
	MMGBindingManager(QObject *parent, const QJsonObject &json_obj);

	MMGBinding *add(const QJsonObject &json_obj = QJsonObject()) override;
	MMGBinding *copy(MMGBinding *binding) override { return MMGManager::copy(binding); };

	void json(QJsonObject &json_obj) const;
	void copy(MMGBindingManager *dest);
	void refreshAll();
};
QDataStream &operator<<(QDataStream &out, const MMGBindingManager *&obj);
QDataStream &operator>>(QDataStream &in, MMGBindingManager *&obj);

class MMGCollections : public MMGManager<MMGBindingManager> {

public:
	MMGCollections(QObject *parent) : MMGManager(parent){};

	MMGBindingManager *add(const QJsonObject &json_obj = QJsonObject()) override;
};

#endif // MMG_BINDING_H
