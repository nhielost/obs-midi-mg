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

#include "mmg-string.h"

// MMGString
MMGString::MMGString()
{
	_text = new char[1];
}

MMGString::MMGString(const char *text)
{
	_text = new char[std::strlen(text) + 1];
	std::strcpy(_text, text);
}

MMGString &MMGString::operator=(const MMGString &other)
{
	if (!!_text) delete[] _text;

	_text = new char[std::strlen(other.value()) + 1];
	std::strcpy(_text, other.value());
	return *this;
}
// End MMGString

// MMGText
MMGString MMGText::join(const char *header, const char *footer)
{
	return std::format("{}.{}", header, footer).c_str();
}

MMGString MMGText::choose(const char *header, const char *truthy, const char *falsy, bool decider)
{
	return join(header, decider ? truthy : falsy);
}
// End MMGText
