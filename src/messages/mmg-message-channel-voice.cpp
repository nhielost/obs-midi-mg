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

#include "mmg-message-channel-voice.h"

namespace MMGMessages {

#define BYTE_2 16, 8
#define BYTE_3 24, 8

template <uint8_t MIDI2_Offset, uint8_t MIDI2_Size, uint8_t MIDI1_Offset, uint8_t MIDI1_Size>
static constexpr uint32_t getMessageDataValue(const MMGMessageData &message)
{
	return usingMIDI2() ? message.get<MIDI2_Offset, MIDI2_Size>() : message.get<MIDI1_Offset, MIDI1_Size>();
}

template <uint8_t MIDI2_Offset, uint8_t MIDI2_Size, uint8_t MIDI1_Offset, uint8_t MIDI1_Size>
static constexpr void setMessageDataValue(MMGMessageData &message, uint32_t value)
{
	usingMIDI2() ? message.set<MIDI2_Offset, MIDI2_Size>(value) : message.set<MIDI1_Offset, MIDI1_Size>(value);
}

#define GROUP 4, 4
#define CHANNEL 12, 4

// MMGMessageChannelVoice
static MMGParams<uint8_t> group_params {
	.desc = mmgtr("Message.CV.Group"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 1,

	.lower_bound = 1.0,
	.upper_bound = 16.0,
	.step = 1.0,
};

static MMGParams<uint8_t> channel_params {
	.desc = mmgtr("Message.CV.Channel"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 1,

	.lower_bound = 1.0,
	.upper_bound = 16.0,
	.step = 1.0,
};

MMGMessageChannelVoice::MMGMessageChannelVoice(MMGMessageManager *parent, const QJsonObject &json_obj)
	: MMGMessage(parent, json_obj),
	  _group(json_obj, "group"),
	  _channel(json_obj, "channel")
{
	if (_group->state() == STATE_FIXED && _group == uint8_t(0)) _group = 1;
}

void MMGMessageChannelVoice::initOldData(const QJsonObject &json_obj)
{
	// Old references don't know which field to use, so we assume the last field
	// is used because that's how it was in pre v3.1 versions
	MMGStates::MMGReferenceIndexHandler::setOldReferenceIndex(MMGStates::ReferenceIndex(3));

	MMGCompatibility::initOldNumberData(_channel, json_obj, "channel", 0);
	if (_channel->state() == STATE_FIXED && _channel == uint8_t(0)) _channel = 1;
}

void MMGMessageChannelVoice::json(QJsonObject &message_obj) const
{
	MMGMessage::json(message_obj);

	_group->json(message_obj, "group");
	_channel->json(message_obj, "channel");
}

void MMGMessageChannelVoice::copy(MMGMessage *dest) const
{
	MMGMessage::copy(dest);

	auto *casted = qobject_cast<MMGMessageChannelVoice *>(dest);
	if (!casted) return;

	_group.copy(casted->_group);
	_channel.copy(casted->_channel);
}

void MMGMessageChannelVoice::createDisplay(MMGWidgets::MMGMessageDisplay *display)
{
	group_params.options.setFlag(OPTION_HIDDEN, !usingMIDI2());
	MMGMessages::createMessageField(display, &_group, &group_params);
	MMGMessages::createMessageField(display, &_channel, &channel_params);
}

void MMGMessageChannelVoice::processMessage(MMGMappingTest &test, const MMGMessageData &data) const
{
	test.addCondition(status() == data.status());
	test.addAcceptable(_group, data.get<GROUP>() + 1, usingMIDI2());
	test.addAcceptable(_channel, data.get<CHANNEL>() + 1);
}

void MMGMessageChannelVoice::replaceString(QString &str) const
{
	MMGMessage::replaceString(str);

	str.replace("${group}", QString::number(_group));
	str.replace("${status}", mmgtr(MMGText::join("Message.CV.Status", trMessageName())));
	str.replace("${channel}", QString::number(_channel));
}

void MMGMessageChannelVoice::copyFromMessageData(const MMGMessageData &data)
{
	_group = data.get<GROUP>() + 1;
	_channel = data.get<CHANNEL>() + 1;
}

void MMGMessageChannelVoice::copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const
{
	uint8_t group = 1, channel = 1;
	if (!test.applicable(_group, group)) blog(LOG_INFO, "A group could not be selected. Defaulted to group 1.");
	if (!test.applicable(_channel, channel))
		blog(LOG_INFO, "A channel could not be selected. Defaulted to channel 1.");

	message.set<GROUP>(group - 1);
	message.setStatus(status());
	message.set<CHANNEL>(channel - 1);
}
// End MMGMessageChannelVoice

// MMGMessageNote
#define NOTE 16, 8
#define VELOCITY 32, 16

static const MMGParams<bool> note_type_params {
	.desc = mmgtr("Message.CV.Note.Type"),
	.options = OPTION_ALLOW_TOGGLE,
	.default_value = true,
};

static const MMGParams<uint8_t> note_params {
	.desc = mmgtr("Message.CV.Note"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 60,
	.lower_bound = 0.0,
	.upper_bound = 127.0,
};

static MMGParams<uint16_t> velocity_params {
	.desc = mmgtr("Message.CV.Velocity"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 0,
	.lower_bound = 0.0,
	.upper_bound = 65535.0,
};

MMGMessageNote::MMGMessageNote(MMGMessageManager *parent, const QJsonObject &json_obj)
	: MMGMessageChannelVoice(parent, json_obj),
	  _note(json_obj, "note"),
	  _velocity(json_obj, "velocity")
{
}

void MMGMessageNote::initOldData(const QJsonObject &json_obj)
{
	MMGMessageChannelVoice::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(_note, json_obj, "note", 0);
	MMGCompatibility::initOldNumberData(_velocity, json_obj, "value", 0);

	if ((json_obj["value_require"].isBool() && !(json_obj["value_require"].toBool())) ||
	    (_velocity->state() == STATE_FIXED && _velocity == uint16_t(0xffff))) {
		auto velocity_midi = _velocity.changeTo<STATE_RANGE>();
		velocity_midi->setMin(0);
		velocity_midi->setMax(127);
	}
}

void MMGMessageNote::json(QJsonObject &message_obj) const
{
	MMGMessageChannelVoice::json(message_obj);

	_note->json(message_obj, "note");
	_velocity->json(message_obj, "velocity");
}

void MMGMessageNote::copy(MMGMessage *dest) const
{
	MMGMessageChannelVoice::copy(dest);

	auto *casted = qobject_cast<MMGMessageNote *>(dest);
	if (!casted) return;

	_note.copy(casted->_note);
	_velocity.copy(casted->_velocity);
}

void MMGMessageNote::createDisplay(MMGWidgets::MMGMessageDisplay *display)
{
	MMGMessageChannelVoice::createDisplay(display);

	velocity_params.upper_bound = usingMIDI2() ? 65535.0 : 127.0;
	MMGMessages::createMessageField(display, &_note, &note_params);
	MMGMessages::createMessageField(display, &_velocity, &velocity_params);
}

void MMGMessageNote::processMessage(const MMGMessageData &data)
{
	MessageFulfillment fulfiller(this);
	MMGMessageChannelVoice::processMessage(*fulfiller, data);

	fulfiller->addAcceptable(_note, getMessageDataValue<NOTE, BYTE_2>(data));
	fulfiller->addAcceptable(_velocity, getMessageDataValue<VELOCITY, BYTE_3>(data));
}

void MMGMessageNote::replaceString(QString &str) const
{
	MMGMessageChannelVoice::replaceString(str);

	str.replace("${note}", QString::number(_note));
	str.replace("${velocity}", QString::number(_velocity));
}

void MMGMessageNote::copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const
{
	MMGMessageChannelVoice::copyToMessageData(message, test);

	uint8_t note = 0;
	uint16_t velocity = 0;

	if (!test.applicable(_note, note))
		blog(LOG_INFO, "A note number could not be selected. Defaulted to note "
			       "number 0 (C-2).");
	if (!test.applicable(_velocity, velocity))
		blog(LOG_INFO, "A velocity could not be selected. Defaulted to a velocity of 0.");

	setMessageDataValue<NOTE, BYTE_2>(message, note);
	setMessageDataValue<VELOCITY, BYTE_3>(message, velocity);
}

void MMGMessageNote::copyFromMessageData(const MMGMessageData &data)
{
	MMGMessageChannelVoice::copyFromMessageData(data);

	_note = getMessageDataValue<NOTE, BYTE_2>(data);
	_velocity = getMessageDataValue<VELOCITY, BYTE_3>(data);
}
// End MMGMessageNote

// MMGMessageNoteToggle
void MMGMessageNoteToggle::processMessage(const MMGMessageData &data)
{
	MMGMessageNote::processMessage(data);
	note_type = !note_type;
}

void MMGMessageNoteToggle::copyFromMessageData(const MMGMessageData &data)
{
	MMGMessageNote::copyFromMessageData(data);
	note_type = data.status() == NOTE_ON;
}

void MMGMessageNoteToggle::copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const
{
	MMGMessageNote::copyToMessageData(message, test);
	note_type = !note_type;
}
// End MMGMessageNoteToggle

// MMGMessageControlChange
#define CONTROL 16, 8
#define VALUE 32, 32

static const MMGParams<uint8_t> control_params {
	.desc = mmgtr("Message.CV.Control"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 0,
	.lower_bound = 0.0,
	.upper_bound = 127.0,
};

static MMGParams<uint32_t> value_params {
	.desc = mmgtr("Message.CV.Value"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 0,
	.lower_bound = 0.0,
	.upper_bound = 127.0,
};

MMGMessageControlChange::MMGMessageControlChange(MMGMessageManager *parent, const QJsonObject &json_obj)
	: MMGMessageChannelVoice(parent, json_obj),
	  _control(json_obj, "control"),
	  _value(json_obj, "value")
{
}

void MMGMessageControlChange::initOldData(const QJsonObject &json_obj)
{
	MMGMessageChannelVoice::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(_control, json_obj, "note", 0);
	MMGCompatibility::initOldNumberData(_value, json_obj, "value", 0);

	if ((json_obj["value_require"].isBool() && !(json_obj["value_require"].toBool())) ||
	    (_value->state() == STATE_FIXED && _value == uint32_t(0xffffffff))) {
		auto value_midi = _value.changeTo<STATE_RANGE>();
		value_midi->setMin(0);
		value_midi->setMax(127);
	}
}

void MMGMessageControlChange::json(QJsonObject &message_obj) const
{
	MMGMessageChannelVoice::json(message_obj);

	_control->json(message_obj, "control");
	_value->json(message_obj, "value");
}

void MMGMessageControlChange::copy(MMGMessage *dest) const
{
	MMGMessageChannelVoice::copy(dest);

	auto *casted = qobject_cast<MMGMessageControlChange *>(dest);
	if (!casted) return;

	_control.copy(casted->_control);
	_value.copy(casted->_value);
}

void MMGMessageControlChange::createDisplay(MMGWidgets::MMGMessageDisplay *display)
{
	MMGMessageChannelVoice::createDisplay(display);

	value_params.upper_bound = usingMIDI2() ? 4294967295.0 : 127.0;
	MMGMessages::createMessageField(display, &_control, &control_params);
	MMGMessages::createMessageField(display, &_value, &value_params);

	if (usingMIDI2()) MMGParameters::createWarning((MMGWidgets::MMGValueManager *)(display), "Message.CV.Control");
}

void MMGMessageControlChange::processMessage(const MMGMessageData &data)
{
	MessageFulfillment fulfiller(this);
	MMGMessageChannelVoice::processMessage(*fulfiller, data);

	fulfiller->addAcceptable(_control, getMessageDataValue<CONTROL, BYTE_2>(data));
	fulfiller->addAcceptable(_value, getMessageDataValue<VALUE, BYTE_3>(data));
}

void MMGMessageControlChange::replaceString(QString &str) const
{
	MMGMessageChannelVoice::replaceString(str);

	str.replace("${control}", QString::number(_control));
	str.replace("${value}", QString::number(_value));
}

void MMGMessageControlChange::copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const
{
	MMGMessageChannelVoice::copyToMessageData(message, test);

	uint8_t control = 0;
	uint32_t value = 0;
	if (!test.applicable(_control, control))
		blog(LOG_INFO, "A control number could not be selected. Defaulted to "
			       "control number 0.");
	if (!test.applicable(_value, value))
		blog(LOG_INFO, "A value could not be selected. Defaulted to a value of 0.");

	setMessageDataValue<CONTROL, BYTE_2>(message, control);
	setMessageDataValue<VALUE, BYTE_3>(message, value);
}

void MMGMessageControlChange::copyFromMessageData(const MMGMessageData &data)
{
	MMGMessageChannelVoice::copyFromMessageData(data);

	_control = getMessageDataValue<CONTROL, BYTE_2>(data);
	_value = getMessageDataValue<VALUE, BYTE_3>(data);
}
// End MMGMessageControlChange

// MMGMessageProgramChange
#define PROGRAM 32, 8
#define BANK 48, 16
#define BANK_MSB 48, 8
#define BANK_LSB 56, 8

#define GET_BANK(midi) midi.get<31, 1>() ? ((uint16_t(midi.get<BANK_MSB>()) << 7) | midi.get<BANK_LSB>()) + 1 : 0
#define SET_BANK(midi, value)                           \
	midi.set<31, 1>((value) > 0);                   \
	if ((value) > 0) {                              \
		midi.set<BANK_MSB>((value - 1) >> 7);   \
		midi.set<BANK_LSB>((value - 1) & 0x7f); \
	}

static const MMGParams<uint8_t> program_params {
	.desc = mmgtr("Message.CV.Program"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 0,
	.lower_bound = 1.0,
	.upper_bound = 128.0,
};

static MMGParams<uint16_t> bank_params {
	.desc = mmgtr("Message.CV.ProgramBank"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 0,
	.lower_bound = 0.0,
	.upper_bound = 16384.0,
};

MMGMessageProgramChange::MMGMessageProgramChange(MMGMessageManager *parent, const QJsonObject &json_obj)
	: MMGMessageChannelVoice(parent, json_obj),
	  _program(json_obj, "program"),
	  _bank(json_obj, "bank")
{
}

void MMGMessageProgramChange::initOldData(const QJsonObject &json_obj)
{
	MMGMessageChannelVoice::initOldData(json_obj);

	MMGCompatibility::initOldNumberData(_program, json_obj, "value", 0);
	_bank = 0;

	if ((json_obj["value_require"].isBool() && !(json_obj["value_require"].toBool())) ||
	    (_program->state() == STATE_FIXED && _program == uint8_t(0xff))) {
		auto value_midi = _bank.changeTo<STATE_RANGE>();
		value_midi->setMin(1);
		value_midi->setMax(128);
	}

	if (_program->state() == STATE_FIXED) _program.as<STATE_FIXED>()->setValue(_program + 1);
}

void MMGMessageProgramChange::json(QJsonObject &message_obj) const
{
	MMGMessageChannelVoice::json(message_obj);

	_program->json(message_obj, "program");
	_bank->json(message_obj, "bank");
}

void MMGMessageProgramChange::copy(MMGMessage *dest) const
{
	MMGMessageChannelVoice::copy(dest);

	auto *casted = qobject_cast<MMGMessageProgramChange *>(dest);
	if (!casted) return;

	_program.copy(casted->_program);
	_bank.copy(casted->_bank);
}

void MMGMessageProgramChange::createDisplay(MMGWidgets::MMGMessageDisplay *display)
{
	MMGMessageChannelVoice::createDisplay(display);

	bank_params.options.setFlag(OPTION_HIDDEN, !usingMIDI2());
	MMGMessages::createMessageField(display, &_program, &program_params);
	MMGMessages::createMessageField(display, &_bank, &bank_params);
}

void MMGMessageProgramChange::processMessage(const MMGMessageData &data)
{
	MessageFulfillment fulfiller(this);
	MMGMessageChannelVoice::processMessage(*fulfiller, data);

	fulfiller->addAcceptable(_program, getMessageDataValue<PROGRAM, BYTE_2>(data) + 1);
	if (usingMIDI2()) fulfiller->addAcceptable(_bank, GET_BANK(data));
}

void MMGMessageProgramChange::replaceString(QString &str) const
{
	MMGMessageChannelVoice::replaceString(str);

	str.replace("${program}", QString::number(_program));
	str.replace("${programBank}", QString::number(_bank));
}

void MMGMessageProgramChange::copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const
{
	MMGMessageChannelVoice::copyToMessageData(message, test);

	uint8_t program = 1;
	uint16_t bank = 0;
	if (!test.applicable(_program, program))
		blog(LOG_INFO, "A program number could not be selected. Defaulted to "
			       "program number 1.");
	if (!test.applicable(_bank, bank)) blog(LOG_INFO, "A bank could not be selected. Defaulted to no bank change.");

	setMessageDataValue<PROGRAM, BYTE_2>(message, program - 1);
	if (usingMIDI2()) { SET_BANK(message, bank); }
}

void MMGMessageProgramChange::copyFromMessageData(const MMGMessageData &data)
{
	MMGMessageChannelVoice::copyFromMessageData(data);

	_program = getMessageDataValue<PROGRAM, BYTE_2>(data) + 1;
	_bank = usingMIDI2() && GET_BANK(data);
}
// End MMGMessageProgramChange

// MMGMessagePitchBend
#define PITCH 32, 32

static MMGParams<int32_t> pitch_params {
	.desc = mmgtr("Message.CV.PitchAdjust"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 0,
	.lower_bound = -8192.0,
	.upper_bound = +8191.0,
};

MMGMessagePitchBend::MMGMessagePitchBend(MMGMessageManager *parent, const QJsonObject &json_obj)
	: MMGMessageChannelVoice(parent, json_obj),
	  _pitch(json_obj, "pitch")
{
}

int32_t MMGMessagePitchBend::getPitch(const MMGMessageData &message)
{
	if (usingMIDI2()) {
		return message.get<PITCH>() - 0x80000000u;
	} else {
		return (message.get<BYTE_3>() << 7) + message.get<BYTE_2>() - 0x2000;
	}
}

void MMGMessagePitchBend::setPitch(MMGMessageData &message, int32_t pitch)
{
	if (usingMIDI2()) {
		message.set<PITCH>(pitch + 0x80000000u);
	} else {
		pitch += 0x2000;
		message.set<BYTE_2>(pitch & 0x007f);
		message.set<BYTE_3>(pitch & 0x3f80);
	}
}

void MMGMessagePitchBend::initOldData(const QJsonObject &json_obj)
{
	MMGMessageChannelVoice::initOldData(json_obj);
	MMGStates::MMGReferenceIndexHandler::setOldReferenceIndex(MMGStates::ReferenceIndex(2));

	MMGCompatibility::initOldNumberData(_pitch, json_obj, "value", 0);

	if ((json_obj["value_require"].isBool() && !(json_obj["value_require"].toBool())) ||
	    (_pitch->state() == STATE_FIXED && _pitch == -1)) {
		auto pitch_midi = _pitch.changeTo<STATE_RANGE>();
		pitch_midi->setMin(-2147483648);
		pitch_midi->setMax(2147483647);
	}
}

void MMGMessagePitchBend::json(QJsonObject &message_obj) const
{
	MMGMessageChannelVoice::json(message_obj);

	_pitch->json(message_obj, "pitch");
}

void MMGMessagePitchBend::copy(MMGMessage *dest) const
{
	MMGMessageChannelVoice::copy(dest);

	auto *casted = qobject_cast<MMGMessagePitchBend *>(dest);
	if (!casted) return;

	_pitch.copy(casted->_pitch);
}

void MMGMessagePitchBend::createDisplay(MMGWidgets::MMGMessageDisplay *display)
{
	MMGMessageChannelVoice::createDisplay(display);

	pitch_params.lower_bound = usingMIDI2() ? -2147483648.0 : -8192.0;
	pitch_params.upper_bound = usingMIDI2() ? +2147483647.0 : +8191.0;
	MMGMessages::createMessageField(display, &_pitch, &pitch_params);
}

void MMGMessagePitchBend::processMessage(const MMGMessageData &data)
{
	MessageFulfillment fulfiller(this);
	MMGMessageChannelVoice::processMessage(*fulfiller, data);

	fulfiller->addAcceptable(_pitch, getPitch(data));
}

void MMGMessagePitchBend::replaceString(QString &str) const
{
	MMGMessageChannelVoice::replaceString(str);

	str.replace("${pitch}", QString::number(_pitch));
}

void MMGMessagePitchBend::copyToMessageData(MMGMessageData &message, const MMGMappingTest &test) const
{
	MMGMessageChannelVoice::copyToMessageData(message, test);

	int32_t pitch = 0;
	if (!test.applicable(_pitch, pitch))
		blog(LOG_INFO, "A pitch adjustment could not be selected. Defaulted to no "
			       "pitch adjustment.");

	setPitch(message, pitch);
}

void MMGMessagePitchBend::copyFromMessageData(const MMGMessageData &data)
{
	MMGMessageChannelVoice::copyFromMessageData(data);

	_pitch = getPitch(data);
}
// End MMGMessagePitchBend

} // namespace MMGMessages
