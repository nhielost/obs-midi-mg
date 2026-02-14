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

#ifndef MMG_MANAGER_H
#define MMG_MANAGER_H

#include "mmg-json.h"
#include "mmg-string.h"

template <class T> class MMGManager : public QObject {

public:
	MMGManager(QObject *parent, const char *load_key) : QObject(parent), key(load_key) {};
	~MMGManager() { clear(); };

	T *add(const QJsonObject &json_obj = QJsonObject()) { return add(T::generate(this, json_obj)); };

	T *copy(T *source);
	T *find(const QString &name) const;
	void move(int from, int to);
	void remove(T *source);
	void clear(bool full = true);

	void load(const QJsonObject &json_obj);
	void json(QJsonObject &json_obj) const;
	void setUniqueName(T *source, qulonglong count = 2);
	const MMGTranslationMap<T *> names() const;

	T *at(qsizetype i) const { return _list.value(i); };
	qsizetype indexOf(T *source) const { return _list.indexOf(source); };
	qsizetype size() const { return _list.size(); };

	auto begin() const { return _list.begin(); };
	auto end() const { return _list.end(); };

protected:
	T *add(T *new_t);
	T *copy(T *source, T *dest);

	void copy(MMGManager<T> *dest) const;

	static MMGManager<T> *generate(MMGManager<MMGManager<T>> *, const QJsonObject &) { return nullptr; };

private:
	QList<T *> _list;
	const char *key;

	friend class MMGManager<MMGManager<T>>;
};

#endif // MMG_MANAGER_H
