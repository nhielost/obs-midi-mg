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

#include "mmg-device.h"
#include "mmg-message.h"
#include "actions/mmg-action.h"

#include <QThread>

class MMGBindingSettings;
class MMGBindingThread;
class MMGBindingManager;

class MMGBinding : public QObject {
	Q_OBJECT

public:
	MMGBinding(MMGBindingManager *parent, const QJsonObject &json_obj = QJsonObject());

	const QString &name() const { return _name; };
	void setName(const QString &val) { _name = val; };

	MMGUtils::DeviceType type() const { return _type; };
	void setType(MMGUtils::DeviceType type);

	bool enabled() const { return _enabled; };
	void setEnabled(bool val);
	void setConnected(bool connected);

	void json(QJsonObject &binding_obj) const;
	void blog(int log_status, const QString &message) const;
	void copy(MMGBinding *dest);

	const MMGDeviceList &usedDevices() const { return _devices; };
	void setUsedDevices(const MMGDeviceList &devices);

	const MMGMessageList &usedMessages() const { return _messages; };
	void setUsedMessages(const MMGMessageList &messages);

	const MMGActionList &usedActions() const { return _actions; };
	virtual void setUsedActions(const MMGActionList &actions);

	MMGBindingSettings *settings() const { return _settings; };

public slots:
	void removeDevice(MMGDevice *device) { _devices.removeOne(device); };
	void removeMessage(MMGMessage *message) { _messages.removeOne(message); };
	void removeAction(MMGAction *action) { _actions.removeOne(action); };
	void replaceAction(MMGAction *action);

private slots:
	void executeInput(const MMGSharedMessage &message) const;
	void executeOutput(const QList<MMGUtils::MMGNumber> &values) const;

private:
	QString _name;
	MMGUtils::DeviceType _type;

	bool _enabled;
	bool connected = false;

	MMGDeviceList _devices;
	MMGMessageList _messages;
	MMGActionList _actions;
	MMGBindingSettings *_settings;

	MMGBindingThread *thread;

	friend class MMGBindingThread;
};

class MMGBindingThread : public QThread {
	Q_OBJECT

public:
	MMGBindingThread(MMGBinding *parent);
	~MMGBindingThread()
	{
		if (locked) mutex.unlock();
	}

	void blog(int log_status, const QString &message) const;
	void run() override;

	void sendMessages();
	void doActions();

	void restart(const MMGMessage *msg);
	void restart(const QList<MMGUtils::MMGNumber> &values);
	MMGBindingThread *createNew() const;

private:
	std::timed_mutex mutex;
	bool locked = false;

	MMGBinding *binding;

	MMGMessage *incoming_message;
	QList<MMGUtils::MMGNumber> incoming_values;
	MMGMessage *applied_message;

	static short thread_count;
};

class MMGBindingManager : public MMGManager<MMGBinding> {

public:
	MMGBindingManager(QObject *parent) : MMGManager(parent){};

	MMGBinding *add(const QJsonObject &json_obj = QJsonObject()) override;

	bool filter(MMGUtils::DeviceType type, MMGBinding *check) const override;
	void resetConnections();
};

#endif // MMG_BINDING_H
