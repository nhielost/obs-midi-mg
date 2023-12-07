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

#ifndef MMG_MANAGER_CPP
#define MMG_MANAGER_CPP

#include "mmg-manager.h"

template<class T> T *MMGManager<T>::add(T *new_t)
{
	if (!new_t) return nullptr;
	_list.append(new_t);
	if (find(new_t->name()) != new_t) setUniqueName(new_t);
	return new_t;
}

template<class T> T *MMGManager<T>::copy(T *source)
{
	return copy(source, add());
}

template<class T> T *MMGManager<T>::copy(T *source, T *dest)
{
	source->copy(dest);
	if (find(dest->name()) != dest) setUniqueName(dest);
	return dest;
}

template<class T> T *MMGManager<T>::find(const QString &name) const
{
	for (T *value : _list) {
		if (value->name() == name) return value;
	}
	return nullptr;
}

template<class T> void MMGManager<T>::move(int from, int to)
{
	if (from >= _list.size()) return;
	to >= _list.size() ? _list.append(_list.takeAt(from)) : _list.move(from, to);
}

template<class T> void MMGManager<T>::setUniqueName(T *source, qulonglong count)
{
	QString new_name = QString("%1 (%2)").arg(source->name()).arg(count);
	if (find(new_name)) {
		setUniqueName(source, ++count);
		return;
	}
	source->setName(new_name);
}

template<class T> void MMGManager<T>::remove(T *source)
{
	_list.removeOne(source);
	delete source;
}

template<class T> void MMGManager<T>::clear()
{
	qDeleteAll(_list);
	_list.clear();
}

template<class T> void MMGManager<T>::load(const QJsonArray &json_arr)
{
	for (const QJsonValue &value : json_arr) {
		add(value.toObject());
	}
}

template<class T> void MMGManager<T>::json(const QString &key, QJsonObject &json_obj) const
{
	QJsonArray json_arr;
	for (T *value : _list) {
		QJsonObject value_json;
		value->json(value_json);
		json_arr += value_json;
	}
	json_obj[key] = json_arr;
}

#endif // MMG_MANAGER_CPP
