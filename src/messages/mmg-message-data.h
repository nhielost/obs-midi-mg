/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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

#ifndef MMG_MESSAGE_DATA_H
#define MMG_MESSAGE_DATA_H

#include "../mmg-string.h"

namespace libremidi {
struct message;
struct ump;
} // namespace libremidi

namespace MMGMessages {

// MIDI 2.0 Specification section 2.1.4
enum Type {
	UTILITY,
	SYSTEM_RT_COMMON,
	MIDI1_CV,
	MIDI1_SYSEX_7,
	MIDI2_CV,
	MIDI2_SYSEX_8,
};

// Since the MIDI 2.0 status codes are not defined by libremidi,
// (they are defined by cmidi2)
// These are provided for obs-midi-mg's API
enum ChannelStatusCode : uint8_t {
	PER_NOTE_RCC = 0x00,
	PER_NOTE_ACC = 0x10,
	RPN = 0x20,
	NRPN = 0x30,
	REL_RPN = 0x40,
	REL_NRPN = 0x50,
	PER_NOTE_PITCH_BEND = 0x60,

	RESERVED = 0x70,

	NOTE_OFF = 0x80,
	NOTE_ON = 0x90,
	POLY_PRESSURE = 0xa0,
	CONTROL_CHANGE = 0xb0,
	PROGRAM_CHANGE = 0xc0,
	CHANNEL_PRESSURE = 0xd0,
	PITCH_BEND = 0xe0,
	PER_NOTE_MGMT = 0xf0,
};

} // namespace MMGMessages

struct MMGMessageData {
private:
	template <uint8_t Size> static constexpr uint64_t flag() { return (1ull << Size) - 1ull; };
	template <uint8_t Offset, uint8_t Size> static constexpr uint8_t offsetClamp()
	{
		return 64u - Size - std::clamp<uint8_t>(Offset % 64u, 0u, 64u - Size);
	};

public:
	MMGMessageData() noexcept;
	MMGMessageData(uint64_t message) noexcept;
	MMGMessageData(const libremidi::message &midi) noexcept;
	MMGMessageData(const libremidi::ump &midi) noexcept;
	~MMGMessageData() = default;

	MMGMessages::Type type() const noexcept { return MMGMessages::Type(get<0, 4>()); };
	bool isCV() const noexcept { return type() == MMGMessages::MIDI1_CV || type() == MMGMessages::MIDI2_CV; };

	MMGMessages::ChannelStatusCode status() const noexcept;
	void setStatus(MMGMessages::ChannelStatusCode status) noexcept;

	template <uint8_t Offset, uint8_t Size> uint32_t get() const noexcept
	{
		return (msg >> offsetClamp<Offset, Size>()) & flag<Size>();
	}

	template <uint8_t Offset, uint8_t Size> void set(uint32_t value) noexcept
	{
		msg &= ~flag<Size>() << offsetClamp<Offset, Size>();
		msg |= (value & flag<Size>()) << offsetClamp<Offset, Size>();
	};

	operator libremidi::ump() const noexcept;
	operator libremidi::message() const noexcept;

private:
	uint64_t msg;
};

class MMGMessageReceiver {
public:
	virtual ~MMGMessageReceiver() = default;

	virtual void processMessage(const MMGMessageData &data) = 0;
};

#endif // MMG_MESSAGE_DATA_H
