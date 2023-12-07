/*
obs-midi-mg
Copyright (C) 2022-2023 nhielost <nhielost@gmail.com>

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

#include "mmg-utils.h"

template<class T> class MMGManager : public QObject {

public:
	virtual T *add(const QJsonObject &json_obj = QJsonObject()) = 0;
	virtual T *copy(T *source);
	T *find(const QString &name) const;
	void move(int from, int to);
	void remove(T *source);
	void clear();

	void load(const QJsonArray &json_arr);
	void json(const QString &key, QJsonObject &json_obj) const;
	void setUniqueName(T *source, qulonglong count = 2);
	virtual bool filter(MMGUtils::DeviceType, T *) const { return true; };

	const QList<T *> &list() const { return _list; };
	T *at(qsizetype i) const { return _list.value(i); };
	auto begin() const { return _list.begin(); };
	auto end() const { return _list.end(); };

protected:
	MMGManager(QObject *parent) : QObject(parent){};
	~MMGManager() { clear(); };

	T *add(T *new_t);
	T *copy(T *source, T *dest);

	QList<T *> _list;
};

#include "mmg-manager.cpp"

#endif // MMG_MANAGER_H
