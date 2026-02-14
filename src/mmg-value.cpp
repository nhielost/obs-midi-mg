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

#include "mmg-value.h"

template <typename T> MMGValue<T>::MMGValue()
{
	_data.reset(new MMGStates::Fixed<T>);
}

template <typename T> MMGValue<T>::MMGValue(const QJsonObject &json_obj, const QString &prefix)
{
	init(json_obj[prefix].toObject());
}

template <typename T> void MMGValue<T>::init(const QJsonObject &json_obj)
{
	if constexpr (MMGStates::DefaultFeatures<T>) {
		switch (MMGJson::getValue<ValueState>(json_obj, "state")) {
			case STATE_MIDI:
				_data.reset(new MMGStates::MIDIMap<T>);
				break;

			case STATE_TOGGLE:
				_data.reset(new MMGStates::Toggle<T>);
				break;

			case STATE_IGNORE:
				_data.reset(new MMGStates::Ignore<T>);
				break;

			case STATE_RANGE:
				if constexpr (MMGStates::ExtraFeatures<T>) {
					_data.reset(new MMGStates::MIDIRange<T>);
					break;
				}
				[[fallthrough]];

			case STATE_INCREMENT:
				if constexpr (MMGStates::ExtraFeatures<T>) {
					_data.reset(new MMGStates::Increment<T>);
					break;
				}
				[[fallthrough]];

			default:
				_data.reset(new MMGStates::Fixed<T>);
				break;
		}

	} else {
		_data.reset(new MMGStates::Fixed<T>);
	}

	_data->init(json_obj);
}

template <typename T> void MMGValue<T>::copy(MMGValue<T> &other) const
{
	QJsonObject json_obj;
	_data->json(json_obj, "copy");
	other.init(json_obj["copy"].toObject());
}
