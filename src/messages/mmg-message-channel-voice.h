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

#pragma once

#include "mmg-message.h"

namespace MMGMessages {

class MMGMessageChannelVoice : public MMGMessage {
	Q_OBJECT

public:
	MMGMessageChannelVoice(MMGMessageManager *parent, const QJsonObject &json_obj);
	virtual ~MMGMessageChannelVoice() = default;

	const char *typeName() const final override { return "CV"; };
	virtual ChannelStatusCode status() const = 0;

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &message_obj) const override;
	void copy(MMGMessage *dest) const override;

	void createDisplay(MMGWidgets::MMGMessageDisplay *display) override;

	void replaceString(QString &str) const override;

	void copyFromMessageData(const MMGMessageData &data) override;
	void copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const override;

protected:
	void processMessage(MMGMappingTest &test, const MMGMessageData &data) const;

private:
	MMG8Bit _group;
	MMG8Bit _channel;
};

class MMGMessageNote : public MMGMessageChannelVoice {
	Q_OBJECT

public:
	MMGMessageNote(MMGMessageManager *parent, const QJsonObject &json_obj);

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &message_obj) const override;
	void copy(MMGMessage *dest) const override;

	void createDisplay(MMGWidgets::MMGMessageDisplay *display) override;

	void processMessage(const MMGMessageData &data) override;
	void replaceString(QString &str) const override;

	void copyFromMessageData(const MMGMessageData &data) override;
	void copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const override;

private:
	MMG8Bit _note;
	MMG16Bit _velocity;
};

class MMGMessageNoteOn : public MMGMessageNote {
	Q_OBJECT

public:
	MMGMessageNoteOn(MMGMessageManager *parent, const QJsonObject &json_obj) : MMGMessageNote(parent, json_obj) {};

	Id id() const final override { return Id(0x4090); };
	const char *trMessageName() const final override { return "NoteOn"; };
	ChannelStatusCode status() const final override { return NOTE_ON; };
};
MMG_DECLARE_MESSAGE(MMGMessageNoteOn);

class MMGMessageNoteOff : public MMGMessageNote {
	Q_OBJECT

public:
	MMGMessageNoteOff(MMGMessageManager *parent, const QJsonObject &json_obj) : MMGMessageNote(parent, json_obj) {};

	Id id() const final override { return Id(0x4080); };
	const char *trMessageName() const final override { return "NoteOff"; };
	ChannelStatusCode status() const final override { return NOTE_OFF; };
};
MMG_DECLARE_MESSAGE(MMGMessageNoteOff);

class MMGMessageNoteToggle : public MMGMessageNote {
	Q_OBJECT

public:
	MMGMessageNoteToggle(MMGMessageManager *parent, const QJsonObject &json_obj)
		: MMGMessageNote(parent, json_obj) {};

	Id id() const final override { return Id(0x409f); };
	const char *trMessageName() const final override { return "ToggleNote"; };
	ChannelStatusCode status() const final override { return note_type ? NOTE_ON : NOTE_OFF; };

	void processMessage(const MMGMessageData &data) override;

	void copyFromMessageData(const MMGMessageData &data) override;
	void copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const override;

private:
	mutable bool note_type = true;
};
MMG_DECLARE_MESSAGE(MMGMessageNoteToggle);

class MMGMessageControlChange : public MMGMessageChannelVoice {
	Q_OBJECT

public:
	MMGMessageControlChange(MMGMessageManager *parent, const QJsonObject &json_obj);

	Id id() const final override { return Id(0x40b0); };
	const char *trMessageName() const final override { return "ControlChange"; };
	ChannelStatusCode status() const final override { return CONTROL_CHANGE; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &message_obj) const override;
	void copy(MMGMessage *dest) const override;

	void createDisplay(MMGWidgets::MMGMessageDisplay *display) override;

	void processMessage(const MMGMessageData &data) override;
	void replaceString(QString &str) const override;

	void copyFromMessageData(const MMGMessageData &data) override;
	void copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const override;

private:
	MMG8Bit _control;
	MMG32Bit _value;
};
MMG_DECLARE_MESSAGE(MMGMessageControlChange);

class MMGMessageProgramChange : public MMGMessageChannelVoice {
	Q_OBJECT

public:
	MMGMessageProgramChange(MMGMessageManager *parent, const QJsonObject &json_obj);

	Id id() const final override { return Id(0x40c0); };
	const char *trMessageName() const final override { return "ProgramChange"; };
	ChannelStatusCode status() const final override { return PROGRAM_CHANGE; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &message_obj) const override;
	void copy(MMGMessage *dest) const override;

	void createDisplay(MMGWidgets::MMGMessageDisplay *display) override;

	void processMessage(const MMGMessageData &data) override;
	void replaceString(QString &str) const override;

	void copyFromMessageData(const MMGMessageData &data) override;
	void copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const override;

private:
	MMG8Bit _program;
	MMG16Bit _bank;
};
MMG_DECLARE_MESSAGE(MMGMessageProgramChange);

class MMGMessagePitchBend : public MMGMessageChannelVoice {
	Q_OBJECT

public:
	MMGMessagePitchBend(MMGMessageManager *parent, const QJsonObject &json_obj);

	Id id() const final override { return Id(0x40e0); };
	const char *trMessageName() const final override { return "PitchBend"; };
	ChannelStatusCode status() const final override { return PITCH_BEND; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &message_obj) const override;
	void copy(MMGMessage *dest) const override;

	void createDisplay(MMGWidgets::MMGMessageDisplay *display) override;

	void processMessage(const MMGMessageData &data) override;
	void replaceString(QString &str) const override;

	void copyFromMessageData(const MMGMessageData &data) override;
	void copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const override;

private:
	MMGInteger _pitch;

	static int32_t getPitch(const MMGMessageData &message);
	static void setPitch(MMGMessageData &message, int32_t pitch);
};
MMG_DECLARE_MESSAGE(MMGMessagePitchBend);

} // namespace MMGMessages
