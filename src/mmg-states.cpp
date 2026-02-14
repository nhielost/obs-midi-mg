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

#include "mmg-states.h"
#include "mmg-mapping.h"

namespace MMGStates {

static int64_t normalize(double value, double range)
{
	return std::round<int64_t>(value / range * double(0x100000000));
};

static double denormalize(int64_t normalized_value, double range)
{
	return std::clamp<double>(double(normalized_value) / double(0xffffffff), 0.0, 1.0) * range;
};

// Fixed<T>
template <typename T> inline void Fixed<T>::init(const QJsonObject &json_obj)
{
	if constexpr (DefaultFeatures<T>) fixed_value = MMGJson::getValue<T>(json_obj, "value");
}

template <typename T> inline void Fixed<T>::json(QJsonObject &json_obj) const
{
	if constexpr (DefaultFeatures<T>) MMGJson::setValue(json_obj, "value", fixed_value);
}

template <typename T> inline bool Fixed<T>::apply(int64_t, T &result) const
{
	result = fixed_value;
	return true;
}
// End Fixed<T>

// MIDIMap<T>
template <typename T> requires DefaultFeatures<T> inline void MIDIMap<T>::init(const QJsonObject &json_obj)
{
	mappings = MMGJson::getList<T>(json_obj, "values");
	setReferenceIndex(MMGJson::getValue<ReferenceIndex>(json_obj, "index"));
}

template <typename T> requires DefaultFeatures<T> inline void MIDIMap<T>::json(QJsonObject &json_obj) const
{
	MMGJson::setList(json_obj, "values", mappings);
	MMGJson::setValue(json_obj, "index", referenceIndex());
}

template <typename T>
requires DefaultFeatures<T> inline bool MIDIMap<T>::acceptable(int64_t &ref_index, const T &value) const
{
	int64_t value_index = indexOf(value);
	ref_index = normalize(value_index, referenceSize() - 1);
	return value_index >= 0;
}

template <typename T> requires DefaultFeatures<T> bool MIDIMap<T>::apply(int64_t ref_index, T &result) const
{
	if (ref_index < 0) return false;
	result = mappings[int64_t(denormalize(ref_index, referenceSize() - 1) + 0.5)];
	return true;
};
// End MIDIMap<T>

// MIDIRange<T>
template <typename T> requires ExtraFeatures<T> inline void MIDIRange<T>::init(const QJsonObject &json_obj)
{
	min_value = MMGJson::getValue<T>(json_obj, "min");
	max_value = MMGJson::getValue<T>(json_obj, "max");
	setReferenceIndex(MMGJson::getValue<ReferenceIndex>(json_obj, "index"));
}

template <typename T> requires ExtraFeatures<T> inline void MIDIRange<T>::json(QJsonObject &json_obj) const
{
	MMGJson::setValue(json_obj, "min", min_value);
	MMGJson::setValue(json_obj, "max", max_value);
	MMGJson::setValue(json_obj, "index", referenceIndex());
}

template <typename T>
requires ExtraFeatures<T> inline bool MIDIRange<T>::acceptable(int64_t &ref_index, const T &value) const
{
	ref_index = normalize(value - min_value, max_value - min_value);
	return min() <= max() ? min() <= value && value <= max() : max() <= value && value <= min();
}

template <typename T> requires ExtraFeatures<T> inline bool MIDIRange<T>::apply(int64_t ref_index, T &result) const
{
	result = T(denormalize(ref_index, max_value - min_value)) + min_value;
	return ref_index >= 0;
}
// End MIDIRange<T>

// Toggle<T>
template <typename T> requires DefaultFeatures<T> inline void Toggle<T>::init(const QJsonObject &json_obj)
{
	_range = MMGJson::getList<T>(json_obj, "range");
	index = MMGJson::getValue<int64_t>(json_obj, "current");
	if (index >= size()) index = 0;
}

template <typename T> requires DefaultFeatures<T> inline void Toggle<T>::json(QJsonObject &json_obj) const
{
	MMGJson::setList(json_obj, "range", _range);
	MMGJson::setValue(json_obj, "current", index);
}

template <typename T> requires DefaultFeatures<T> inline void Toggle<T>::toggle() const
{
	if (++index >= size()) index = 0;
}

template <typename T> requires DefaultFeatures<T> inline bool Toggle<T>::acceptable(int64_t &, const T &value) const
{
	bool result = value == _range[index];
	if (result) toggle();
	return result;
}

template <typename T> requires DefaultFeatures<T> inline bool Toggle<T>::apply(int64_t, T &result) const
{
	result = _range[index];
	toggle();
	return true;
}
// End Toggle<T>

// Increment<T>
template <typename T> requires ExtraFeatures<T> inline void Increment<T>::init(const QJsonObject &json_obj)
{
	_increment = MMGJson::getValue<T>(json_obj, "value");
}

template <typename T> requires ExtraFeatures<T> inline void Increment<T>::json(QJsonObject &json_obj) const
{
	MMGJson::setValue(json_obj, "value", _increment);
}

template <typename T> requires ExtraFeatures<T> inline bool Increment<T>::apply(int64_t, T &result) const
{
	result += _increment;
	return true;
}
// End Increment<T>

} // namespace MMGStates
