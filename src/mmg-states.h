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

#ifndef MMG_STATES_H
#define MMG_STATES_H

#include "mmg-json.h"

template <typename T> class MMGValue;
namespace MMGMapping {
class Tester;
}

enum ValueState : uint16_t { STATE_FIXED, STATE_MIDI, STATE_RANGE, STATE_IGNORE, STATE_TOGGLE, STATE_INCREMENT };

namespace MMGStates {

template <typename T> concept BasicFeatures = std::is_pointer_v<T>;
template <typename T> concept DefaultFeatures = !BasicFeatures<T>;
template <typename T> concept ExtraFeatures = MMGIsNumeric<T> && !std::is_enum_v<T>;

enum ReferenceIndex : int64_t {};

class MMGReferenceIndexHandler {

public:
	virtual ~MMGReferenceIndexHandler() = default;

	const ReferenceIndex &referenceIndex() const { return ref_index; };
	void setReferenceIndex(const ReferenceIndex &index) { ref_index = index; };

	virtual int64_t referenceSize() const = 0;
	virtual bool hasReferenceValue(int64_t) const = 0;

	static MMGStates::ReferenceIndex oldReferenceIndex() { return old_ref_index; };
	static void setOldReferenceIndex(MMGStates::ReferenceIndex ref_index) { old_ref_index = ref_index; };

private:
	ReferenceIndex ref_index = ReferenceIndex(0);

	static ReferenceIndex old_ref_index;
};
inline ReferenceIndex MMGReferenceIndexHandler::old_ref_index = ReferenceIndex(0);

template <typename T> class MMGState {

public:
	virtual ~MMGState() = default;

	virtual ValueState state() const = 0;

	void json(QJsonObject &json_obj, const QString &prefix) const
	{
		QJsonObject value_obj;
		value_obj["state"] = (int)state();
		json(value_obj);
		json_obj[prefix] = value_obj;
	};

	virtual operator const T &() const { throw; };

protected:
	virtual void init(const QJsonObject &json_obj) = 0;
	virtual void json(QJsonObject &json_obj) const = 0;

	virtual bool acceptable(int64_t &ref_index, const T &value) const = 0;
	virtual bool apply(int64_t ref_index, T &result) const = 0;

	friend class MMGMapping::Tester;
	friend class MMGValue<T>;
};

template <typename T> class Fixed : public MMGState<T> {
public:
	ValueState state() const override { return STATE_FIXED; };

	const T &value() const { return fixed_value; };
	void setValue(const T &value) { fixed_value = value; };

protected:
	void init(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;

	bool acceptable(int64_t &, const T &value) const override { return value == fixed_value; };
	bool apply(int64_t, T &result) const override;

	operator const T &() const override { return fixed_value; };

private:
	T fixed_value = T();
};

template <typename T> requires DefaultFeatures<T> class MIDIMap : public MMGState<T>, public MMGReferenceIndexHandler {
	using DataList = QList<T>;

public:
	ValueState state() const override { return STATE_MIDI; };

	const DataList &values() const { return mappings; };
	void setSize(int64_t size) { mappings.resize(size); };
	void clearAll() { mappings.clear(); };

	const T &getValue(int64_t index) { return mappings[index]; };
	void setValue(int64_t index, const T &value) { mappings[index] = value; };
	void clearValue(int64_t index) { mappings.remove(index); };

	bool isEmpty() const { return mappings.isEmpty(); };
	int64_t indexOf(const T &value) const { return mappings.indexOf(value); };

	bool hasReferenceValue(int64_t index) const override { return index >= 0 && index < referenceSize(); };
	int64_t referenceSize() const override { return mappings.size(); };

protected:
	void init(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;

	bool acceptable(int64_t &ref_index, const T &value) const override;
	bool apply(int64_t ref_index, T &result) const override;

private:
	DataList mappings;
};

template <typename T> requires ExtraFeatures<T> class MIDIRange : public MMGState<T>, public MMGReferenceIndexHandler {

public:
	ValueState state() const override { return STATE_RANGE; };

	const T &min() const { return min_value; };
	void setMin(const T &min) { min_value = min; };
	const T &max() const { return max_value; };
	void setMax(const T &max) { max_value = max; };

	bool hasReferenceValue(int64_t) const override { return true; };
	int64_t referenceSize() const override { return std::abs(int64_t(max_value) - int64_t(min_value)) + 1; };

protected:
	void init(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;

	bool acceptable(int64_t &ref_index, const T &value) const override;
	bool apply(int64_t ref_index, T &result) const override;

private:
	T min_value = T();
	T max_value = T();
};

template <typename T> requires DefaultFeatures<T> class Toggle : public MMGState<T> {
	using DataList = QList<T>;

public:
	ValueState state() const override { return STATE_TOGGLE; };

	const T &get(int64_t index) { return _range[index]; };
	void set(int64_t index, const T &value) { _range[index] = value; };

	int64_t size() const { return _range.size(); };
	void setSize(int64_t size) { _range.resize(size); };

	const int64_t &currentIndex() const { return index; };
	void setCurrentIndex(int64_t current_index) { index = current_index; };

protected:
	void init(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;

	bool acceptable(int64_t &, const T &value) const override;
	bool apply(int64_t, T &result) const override;
	void toggle() const;

	operator const T &() const override { return _range[index]; };

private:
	DataList _range;
	mutable int64_t index = 0;
};

template <typename T> requires ExtraFeatures<T> class Increment : public MMGState<T> {
public:
	ValueState state() const override { return STATE_INCREMENT; };

	const T &increment() const { return _increment; };
	void setIncrement(const T &increment) { _increment = increment; };

protected:
	void init(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;

	bool acceptable(int64_t &, const T &) const override { return true; };
	bool apply(int64_t, T &current) const override;

	operator const T &() const override { return _increment; };

private:
	T _increment = T();
};

template <typename T> class Ignore : public MMGState<T> {
public:
	ValueState state() const override { return STATE_IGNORE; };

protected:
	void init(const QJsonObject &) override {};
	void json(QJsonObject &) const override {};

	bool acceptable(int64_t &, const T &) const override { return true; };
	bool apply(int64_t, T &) const override { return true; };
};

// Specializations
template <> class Toggle<bool> : public MMGState<bool> {
public:
	ValueState state() const override { return STATE_TOGGLE; };

protected:
	void init(const QJsonObject &) override {};
	void json(QJsonObject &) const override {};

	bool acceptable(int64_t &, const bool &) const override { return true; };
	bool apply(int64_t, bool &result) const override
	{
		result = !result;
		return true;
	};
};
// End Specializations

template <typename T, ValueState State> struct Map {
	using Type = Fixed<T>;
};
template <typename T> requires DefaultFeatures<T> struct Map<T, STATE_MIDI> {
	using Type = MIDIMap<T>;
};
template <typename T> requires ExtraFeatures<T> struct Map<T, STATE_RANGE> {
	using Type = MIDIRange<T>;
};
template <typename T> requires DefaultFeatures<T> struct Map<T, STATE_TOGGLE> {
	using Type = Toggle<T>;
};
template <typename T> requires ExtraFeatures<T> struct Map<T, STATE_INCREMENT> {
	using Type = Increment<T>;
};
template <typename T> struct Map<T, STATE_IGNORE> {
	using Type = Ignore<T>;
};

} // namespace MMGStates

#undef MMG_ENABLED

#endif // MMG_STATES_H
