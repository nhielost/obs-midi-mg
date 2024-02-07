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

#ifndef MMG_MESSAGE_H
#define MMG_MESSAGE_H

#include "mmg-utils.h"
#include "mmg-manager.h"

class MMGMIDIPort;
class MMGLink;

class MMGMessage : public QObject {
	Q_OBJECT

public:
	MMGMessage(QObject *parent = nullptr);
	MMGMessage(const QJsonObject &json_obj);
	MMGMessage(MMGMIDIPort *device, const libremidi::message &message);

	MMGMIDIPort *device() const { return _device; };
	void setDevice(MMGMIDIPort *device);

	void setLink(MMGLink *link);

	MMGUtils::MMGNumber &channel() { return _channel; };
	MMGUtils::MMGString &type() { return _type; };
	MMGUtils::MMGNumber &note() { return _note; };
	MMGUtils::MMGNumber &value() { return _value; };
	const MMGUtils::MMGNumber &channel() const { return _channel; };
	const MMGUtils::MMGString &type() const { return _type; };
	const MMGUtils::MMGNumber &note() const { return _note; };
	const MMGUtils::MMGNumber &value() const { return _value; };

	void blog(int log_status, const QString &message) const;
	void json(QJsonObject &message_obj) const;
	void copy(MMGMessage *dest) const;
	void setEditable(bool edit);
	void toggle();

	bool acceptable(const MMGMessage *test) const;
	void applyValues(const MMGUtils::MMGNumber &applied);
	void send();
	bool valueOnlyType() const;

	static QString getType(const libremidi::message &mess);
	static int getNote(const libremidi::message &mess);
	static int getValue(const libremidi::message &mess);
	static const QStringList acceptedTypes();

	private slots:
	void acceptMessage(const std::shared_ptr<MMGMessage> &incoming);

private:
	MMGUtils::MMGNumber _channel;
	MMGUtils::MMGString _type;
	MMGUtils::MMGNumber _note;
	MMGUtils::MMGNumber _value;

	MMGMIDIPort *_device = nullptr;
	MMGLink *binding_link = nullptr;

	void setRanges();
};

using MMGMessageList = QList<MMGMessage *>;
using MMGSharedMessage = std::shared_ptr<MMGMessage>;
Q_DECLARE_METATYPE(MMGSharedMessage);
QDataStream &operator<<(QDataStream &out, const MMGMessage *&obj);
QDataStream &operator>>(QDataStream &in, MMGMessage *&obj);

class MMGMessageManager : public MMGManager<MMGMessage> {

public:
	MMGMessageManager(QObject *parent) : MMGManager(parent){};

	MMGMessage *add(const QJsonObject &json_obj = QJsonObject()) override;
};

#endif // MMG_MESSAGE_H
