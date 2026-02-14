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

#include "obs-midi-mg.h"

#ifndef MMG_STRING_H
#define MMG_STRING_H

class MMGString {

public:
	MMGString();
	MMGString(const char *text);
	MMGString(const MMGString &other) { *this = other; };
	~MMGString() { delete[] _text; };

	const char *value() const { return _text; };
	bool isEmpty() const { return std::strlen(value()) == 0; };

	operator const char *() const { return value(); };

	bool operator==(const MMGString &other) const { return std::strcmp(value(), other.value()) == 0; };
	bool operator==(const QString &other) const { return QString(value()) == other; };
	bool operator==(const char *other) const { return std::strcmp(value(), other) == 0; };

	MMGString &operator=(const MMGString &other);

private:
	char *_text = nullptr;
};
Q_DECLARE_METATYPE(MMGString);

class MMGText : public MMGString {

public:
	using TranslationFunc = const char *(const char *);

	MMGText() : MMGString() {};
	MMGText(const char *str, TranslationFunc *tr_f) : MMGString(str), trf(tr_f) {};

	const char *translate() const { return !!trf ? (*trf)(value()) : value(); };
	QString arg(const MMGText &arg) const { return QString(translate()).arg(arg.translate()); };

	operator QString() const { return translate(); };
	operator const char *() const { return translate(); };

	bool operator==(const MMGText &other) const { return std::strcmp(translate(), other.translate()) == 0; };
	bool operator==(const QString &other) const { return QString(translate()) == other; };
	bool operator==(const char *other) const { return std::strcmp(translate(), other) == 0; };

	MMGText &operator=(const MMGText &other)
	{
		MMGString::operator=(other);
		trf = other.trf;
		return *this;
	};

	static MMGString join(const char *header, const char *joiner);
	static MMGString choose(const char *header, const char *truthy, const char *falsy, bool decider);

private:
	TranslationFunc *trf = nullptr;
};
Q_DECLARE_METATYPE(MMGText);

template <typename T> class MMGTranslationMap {
	using PairType = std::pair<T, MMGText>;
	using MapType = QList<PairType>;
	using ConstIterator = typename MapType::ConstIterator;

public:
	MMGTranslationMap() {};
	MMGTranslationMap(std::initializer_list<PairType> map) : m(map) {};

	const MMGText &operator[](int32_t index) const { return m[index].second; };
	const T &keyAtIndex(int32_t index) const { return m[index].first; };

	int32_t indexOf(const T &key) const
	{
		for (int i = 0; i < m.size(); ++i)
			if (m[i].first == key) return i;
		return -1;
	};
	const MMGText &find(const T &value) const
	{
		for (const PairType &p : m)
			if (p.first == value) return p.second;
		throw;
	}

	void insert(const T &key, const MMGText &value) { m.append({key, value}); };
	void clear() { m.clear(); };

	bool isEmpty() const { return m.isEmpty(); };
	size_t size() const { return m.size(); };
	const T &firstKey() const noexcept { return m.first().first; };
	bool contains(const T &key) const { return indexOf(key) >= 0; };

	QList<T> keys() const
	{
		QList<T> keys;
		for (const PairType &p : m)
			keys += p.first;
		return keys;
	};

	QList<MMGText> values() const
	{
		QList<MMGText> values;
		for (const PairType &p : m)
			values += p.second;
		return values;
	};

	ConstIterator begin() const { return m.begin(); };
	ConstIterator end() const { return m.end(); };

private:
	MapType m;
};

#define nontr(string) MMGText(string, nullptr)
#define obstr(string) MMGText(string, obs_frontend_get_locale_string)
#define mmgtr(string) MMGText(string, obs_module_text)

#define jointr(header, footer) header "." footer
#define choosetr(header, truthy, falsy, decider) decider ? jointr(header, truthy) : jointr(header, falsy)

using MMGStringTranslationMap = MMGTranslationMap<MMGString>;

#endif // MMG_STRING_H
