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

#ifndef MMG_MAPPING_H
#define MMG_MAPPING_H

#include "mmg-json.h"
#include "mmg-value.h"

namespace MMGMapping {

class Tester {
public:
	Tester() {};
	Tester(const Tester &other) { *this = other; };
	Tester &operator=(const Tester &other);

	bool valid() const { return _valid; };

	template <typename T, typename U = T>
	requires std::convertible_to<U, T> void addAcceptable(const MMGValue<T> &value, const U &test,
							      bool use_if = true)
	{
		if (!use_if) {
			addEmptyAcceptable();
			return;
		}
		if (!_valid) return;

		int64_t ref_index = -1;
		if (value->acceptable(ref_index, test)) {
			results += ref_index;
		} else {
			addUnacceptable();
		}
	};

	template <typename T> void addAcceptable(const MMGBoolean &value, T test, T truthy, T falsy)
	{
		switch (value->state()) {
			case STATE_FIXED:
				addConditionalAcceptable((((bool)(value)) && test == truthy) ||
							 (!((bool)(value)) && test == falsy));
				break;

			case STATE_MIDI:
				if (test == truthy) {
					addAcceptable(value, true);
				} else if (test == falsy) {
					addAcceptable(value, false);
				} else {
					addUnacceptable();
				}
				break;

			default:
				addConditionalAcceptable(test == truthy || test == falsy);
				break;
		}
	};

	void addEmptyAcceptable() { results += -1; };
	void addUnacceptable() { _valid = false; };

	void addCondition(bool condition) { _valid &= condition; };
	void addConditionalAcceptable(bool condition) { condition ? addEmptyAcceptable() : addUnacceptable(); };

	template <typename T> bool applicable(const MMGValue<T> &value, T &result) const
	{
		if (!_valid) return false;

		int64_t ref_index =
			value.usesReference()
				? int64_t(value.template as<MMGStates::MMGReferenceIndexHandler>()->referenceIndex())
				: -1;
		return value->apply(results.value(ref_index, -1), result);
	};

private:
	QList<int64_t> results;
	bool _valid = true;
};

template <typename T> struct Fulfiller {
	Fulfiller(const T *self) : self(self) {};
	~Fulfiller()
	{
		if (test.valid()) emit self->fulfilled(test);
	};

	Tester *operator->() { return &test; };
	Tester &operator*() { return test; };

private:
	const T *self;
	Tester test;
};

}; // namespace MMGMapping

using MMGMappingTest = MMGMapping::Tester;
template <class T> using MMGMappingFulfillment = MMGMapping::Fulfiller<T>;

#endif // MMG_MAPPING_H
