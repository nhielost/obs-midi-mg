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

#ifndef MMG_VALUE_H
#define MMG_VALUE_H

#include "mmg-states.h"

template <typename T> class MMGValue {

public:
	template <ValueState State> using StateType = MMGStates::Map<T, State>::Type;

	MMGValue();
	MMGValue(const QJsonObject &json_obj, const QString &prefix);

	const MMGStates::MMGState<T> *operator->() const { return _data.get(); };
	MMGStates::MMGState<T> *operator->() { return _data.get(); };

	template <typename U> const U *as() const { return dynamic_cast<const U *>(_data.get()); };
	template <typename U> U *as() { return dynamic_cast<U *>(_data.get()); };

	template <ValueState State> const StateType<State> *as() const
	{
		if (_data->state() != State) throw;
		return as<StateType<State>>();
	};
	template <ValueState State> StateType<State> *as()
	{
		if (_data->state() != State) throw;
		return as<StateType<State>>();
	};

	template <ValueState State> StateType<State> *changeTo()
	{
		if (_data->state() == State) return as<StateType<State>>();

		StateType<State> *new_data = new StateType<State>;
		_data.reset(new_data);
		return new_data;
	};

	void copy(MMGValue<T> &other) const;

	bool converts() const { return _data->state() == STATE_FIXED || _data->state() == STATE_TOGGLE; };
	bool usesReference() const { return _data->state() == STATE_MIDI || _data->state() == STATE_RANGE; };

	operator const T &() const { return _data->operator const T &(); };
	bool operator==(const T &other) const { return _data->operator const T &() == other; };

	MMGValue<T> &operator=(const T &value)
	{
		changeTo<STATE_FIXED>()->setValue(value);
		return *this;
	};

	MMGValue<T> &operator=(const MMGValue<T> &other)
	{
		other.copy(*this);
		return *this;
	};

private:
	void init(const QJsonObject &json_obj);

private:
	std::unique_ptr<MMGStates::MMGState<T>> _data;
};

using MMGInteger = MMGValue<int32_t>;
using MMGFloat = MMGValue<float>;
using MMGStringID = MMGValue<MMGString>;
using MMGBoolean = MMGValue<bool>;

using MMG8Bit = MMGValue<uint8_t>;
using MMG16Bit = MMGValue<uint16_t>;
using MMG32Bit = MMGValue<uint32_t>;

namespace MMGCompatibility {

template <typename T>
requires MMGIsNumeric<T> inline void initOldNumberData(MMGValue<T> &value_obj, const QJsonObject &json_obj,
						       const QString &preferred, int fallback_num)
{
	QJsonObject init_obj;
	T fallback = MMGJson::getValue<T>(json_obj, qUtf8Printable(QString("num%1").arg(fallback_num)));

	if (json_obj[preferred].isObject()) {
		// v2.3.0 - v3.0.3 (with bounds)
		QJsonObject number_obj = json_obj[preferred].toObject();
		init_obj["state"] = number_obj["state"];
		init_obj["value"] = number_obj["number"];
		init_obj["min"] = number_obj["lower"];
		init_obj["max"] = number_obj["higher"];
	} else if (json_obj[preferred].isDouble()) {
		// v2.2.0 - v3.0.3 (no bounds)
		init_obj["state"] = json_obj[preferred + "_state"].toInt();
		init_obj["value"] = json_obj[preferred];
	} else if (json_obj["nums_state"].isDouble()) {
		// v2.1.0 - v2.1.1
		int nums_state = (json_obj["nums_state"].toInt() & (3 << (fallback_num * 2))) >> (fallback_num * 2);
		init_obj["state"] = nums_state == 2 ? 3 : nums_state;
		MMGJson::setValue(init_obj, "value", fallback);
	} else {
		// pre v2.1.0
		init_obj["state"] = (int)(fallback == T(-1));
		MMGJson::setValue(init_obj, "value", fallback == T(-1) ? T(0) : fallback);
	}

	switch (init_obj["state"].toInt()) {
		case STATE_MIDI:
			init_obj["state"] = STATE_RANGE;
			[[fallthrough]];

		case STATE_RANGE:
			MMGJson::setValue(init_obj, "index", MMGStates::MMGReferenceIndexHandler::oldReferenceIndex());
			break;

		case STATE_TOGGLE: {
			QJsonArray toggle_arr;
			toggle_arr += init_obj["min"];
			toggle_arr += init_obj["max"];
			init_obj["range"] = toggle_arr;
			MMGJson::setValue<int>(init_obj, "index", init_obj["value"] == init_obj["max"]);
		}

		default:
			break;
	}

	QJsonObject final_obj;
	final_obj["old"] = init_obj;
	value_obj = MMGValue<T>(final_obj, "old");
};

template <typename T>
inline void initOldStringData(MMGValue<T> &value_obj, const QJsonObject &json_obj, const QString &preferred,
			      int fallback_num, const MMGTranslationMap<T> &range)
{
	if (!json_obj[preferred].isObject()) {
		// Before v2.3.0, strings did not have any state at all
		// Thus, they have lost any state compatibility from those versions
		QString fallback_str = QString("str%1").arg(fallback_num);
		QString fixed = json_obj[json_obj[preferred].isString() ? preferred : fallback_str].toString();
		value_obj.template changeTo<STATE_FIXED>();
		for (const auto &[val, _tr] : range) {
			if (_tr != fixed) continue;
			value_obj.template as<STATE_FIXED>()->setValue(val);
			return;
		}
	}

	// v2.3.0 - v3.0.3
	QJsonObject init_obj;
	QJsonArray init_values_arr;
	QJsonArray toggle_arr;

	QJsonObject string_obj = json_obj[preferred].toObject();
	init_obj["state"] = string_obj["state"];

	QString fixed_str = string_obj["string"].toString();
	QString midi_l_str = string_obj["lower"].toString();
	QString midi_h_str = string_obj["higher"].toString();

	for (const auto &[val, _tr] : range) {
		QJsonValue converted_val = MMGJson::convertFrom(val);
		init_values_arr += converted_val;

		if (_tr == fixed_str) MMGJson::setValue<T>(init_obj, "value", val);
		if (_tr == midi_l_str || _tr == midi_h_str) toggle_arr += converted_val;
	}

	switch (init_obj["state"].toInt()) {
		case STATE_RANGE:
			init_obj["state"] = STATE_MIDI;
			[[fallthrough]];

		case STATE_MIDI:
			init_obj["values"] = init_values_arr;
			MMGJson::setValue(init_obj, "index", MMGStates::MMGReferenceIndexHandler::oldReferenceIndex());
			break;

		case STATE_TOGGLE:
			init_obj["range"] = toggle_arr;
			if (toggle_arr.size() >= 2)
				MMGJson::setValue<int>(init_obj, "index", init_obj["value"] == toggle_arr[1]);
			break;

		default:
			break;
	}

	QJsonObject final_obj;
	final_obj["old"] = init_obj;
	value_obj = MMGValue<T>(final_obj, "old");
};

inline void initOldBooleanData(MMGValue<bool> &boolean, int mode)
{
	if (mode == 2) { // Boolean should toggle
		boolean.changeTo<STATE_TOGGLE>();
	} else { // Boolean should be fixed (0 == true, 1 == false)
		boolean = !mode;
	}
};

}; // namespace MMGCompatibility

#endif // MMG_VALUE_H
