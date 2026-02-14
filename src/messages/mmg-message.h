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

#ifndef MMG_MESSAGE_H
#define MMG_MESSAGE_H

#include "../mmg-mapping.h"
#include "../mmg-params.h"
#include "mmg-message-data.h"

class MMGMIDIPort;

class MMGMessage;
template <class T> class MMGManager;
using MMGMessageManager = MMGManager<MMGMessage>;

namespace MMGWidgets {
class MMGMessageDisplay;
} // namespace MMGWidgets

namespace MMGMessages {
enum Id : uint16_t;
}

class MMGMessage : public QObject, public MMGMessageReceiver {
	Q_OBJECT

protected:
	using MessageFulfillment = MMGMappingFulfillment<MMGMessage>;

public:
	MMGMessage(MMGMessageManager *parent, const QJsonObject &json_obj);
	virtual ~MMGMessage() = default;

	MMGMIDIPort *device() const { return _device; };
	void setDevice(MMGMIDIPort *device);

	virtual MMGMessages::Id id() const = 0;
	virtual const char *typeName() const = 0;
	virtual const char *trMessageName() const = 0;

	void blog(int log_status, const QString &message) const;
	virtual void initOldData(const QJsonObject &) {};
	virtual void json(QJsonObject &message_obj) const;
	virtual void copy(MMGMessage *dest) const;

	virtual void createDisplay(MMGWidgets::MMGMessageDisplay *) {};

	void send(const MMGMappingTest &test) const;
	void connectDevice(bool connect);

	virtual void replaceString(QString &str) const;
	virtual void copyFromMessageData(const MMGMessageData &data) = 0;

	static MMGMessage *generate(MMGMessageManager *parent, const QJsonObject &json_obj);

protected:
	virtual void copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const = 0;

signals:
	void refreshRequested() const;
	void fulfilled(MMGMappingTest) const;

private:
	MMGMIDIPort *_device = nullptr;
};
MMG_DECLARE_STREAM_OPERATORS(MMGMessage);

namespace MMGMessages {

enum Id : uint16_t {};

template <typename T>
concept IsMMGMessage =
	std::derived_from<T, MMGMessage> && std::constructible_from<T, MMGMessageManager *, const QJsonObject &>;

struct ConstructBase {
	ConstructBase(MMGMessage *message);

	virtual MMGMessage *operator()(MMGMessageManager *parent, const QJsonObject &json_obj) = 0;
};

template <typename T> requires IsMMGMessage<T> struct Construct : public ConstructBase {
	Construct() : ConstructBase(std::make_unique<T>(nullptr, QJsonObject()).get()) {};

	MMGMessage *operator()(MMGMessageManager *parent, const QJsonObject &json_obj) override
	{
		return new T(parent, json_obj);
	};
};

bool usingMIDI2();

const MMGTranslationMap<Id> availableMessageTypes();
const MMGTranslationMap<Id> availableMessages(Id message_type);

MMGMessage *generateMessage(MMGMessageManager *parent, const QJsonObject &json_obj);
bool changeMessage(MMGMessageManager *parent, MMGMessage *&message, Id new_id);

template <typename T>
inline void createMessageField(MMGWidgets::MMGMessageDisplay *display, MMGValue<T> *storage, const MMGParams<T> *params,
			       const MMGCallback &cb = 0)
{
	MMGParameters::createField<T>((MMGWidgets::MMGValueManager *)(display), storage, params, cb);
};

#define MMG_DECLARE_MESSAGE(T) static Construct<T> _registration_##T {};

} // namespace MMGMessages

#endif // MMG_MESSAGE_H
