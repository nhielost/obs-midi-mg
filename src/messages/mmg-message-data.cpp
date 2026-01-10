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

#include "mmg-message-data.h"

#include <libremidi/detail/conversion.hpp>
#include <libremidi/libremidi.hpp>

// MMGMessageData
MMGMessageData::MMGMessageData() noexcept : msg(0) {}

MMGMessageData::MMGMessageData(uint64_t message) noexcept : msg(message) {}

MMGMessageData::MMGMessageData(const libremidi::message &midi) noexcept
{
	set<0, 4>(MMGMessages::MIDI1_CV);
	set<4, 4>(0);
	set<8, 8>(midi[0]);
	set<16, 8>(midi[1]);
	if (midi.size() > 2) set<24, 8>(midi[2]);
}

MMGMessageData::MMGMessageData(const libremidi::ump &midi) noexcept
{
	set<0, 32>(midi[0]);
	set<32, 32>(midi[1]);
}

MMGMessages::ChannelStatusCode MMGMessageData::status() const noexcept
{
	if (!isCV()) return MMGMessages::RESERVED;
	return MMGMessages::ChannelStatusCode(get<8, 4>() << 4);
}

void MMGMessageData::setStatus(MMGMessages::ChannelStatusCode status) noexcept
{
	if (!isCV()) return;
	set<8, 4>(uint32_t(status) >> 4);
}

MMGMessageData::operator libremidi::ump() const noexcept
{
	if (type() == MMGMessages::MIDI1_CV)
		return libremidi::ump_from_midi1(*this);
	else
		return libremidi::ump(get<0, 32>(), get<32, 32>());
}

MMGMessageData::operator libremidi::message() const noexcept
{
	if (type() == MMGMessages::MIDI2_CV)
		return libremidi::midi1_from_ump(*this);
	else
		return libremidi::message({uint8_t(get<8, 8>()), uint8_t(get<16, 8>()), uint8_t(get<24, 8>())});
}
