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

#include "mmg-manager.h"
#include "mmg-config.h"

template <class T> T *MMGManager<T>::add(T *new_t)
{
	if (!new_t) return nullptr;
	_list.append(new_t);
	new_t->setParent(this);
	if (!new_t->objectName().isEmpty() && find(new_t->objectName()) != new_t) setUniqueName(new_t);
	return new_t;
}

template <class T> T *MMGManager<T>::copy(T *source)
{
	QJsonObject json_obj;
	source->json(json_obj);
	return copy(source, add(json_obj));
}

template <class T> T *MMGManager<T>::copy(T *source, T *dest)
{
	source->copy(dest);
	if (!dest->objectName().isEmpty() && find(dest->objectName()) != dest) setUniqueName(dest);
	return dest;
}

template <class T> T *MMGManager<T>::find(const QString &name) const
{
	for (T *value : _list) {
		if (value->objectName() == name) return value;
	}
	return nullptr;
}

template <class T> void MMGManager<T>::move(int from, int to)
{
	if (from >= _list.size()) return;
	to >= _list.size() ? _list.append(_list.takeAt(from)) : _list.move(from, to);
}

template <class T> void MMGManager<T>::setUniqueName(T *source, qulonglong count)
{
	QString new_name = QString("%1 (%2)").arg(source->objectName()).arg(count);
	if (find(new_name)) {
		setUniqueName(source, ++count);
	} else {
		source->setObjectName(new_name);
	}
}

template <class T> const MMGTranslationMap<T *> MMGManager<T>::names() const
{
	MMGTranslationMap<T *> names;
	for (T *val : _list)
		names.insert(val, nontr(qUtf8Printable(val->objectName())));
	return names;
}

template <class T> void MMGManager<T>::remove(T *source)
{
	_list.removeOne(source);
	delete source;
}

template <class T> void MMGManager<T>::clear(bool full)
{
	qDeleteAll(_list);
	_list.clear();
	if (!full) add();
}

template <class T> void MMGManager<T>::load(const QJsonObject &json_obj)
{
	for (const QJsonValue &value : json_obj[key].toArray()) {
		add(value.toObject());
	}
	if (size() < 1) add();
}

template <class T> void MMGManager<T>::json(QJsonObject &json_obj) const
{
	QJsonArray json_arr;
	for (T *value : _list) {
		QJsonObject value_json;
		value_json["name"] = value->objectName();
		value->json(value_json);
		json_arr += value_json;
	}
	json_obj[key] = json_arr;
}

template <class T> void MMGManager<T>::copy(MMGManager<T> *dest) const
{
	dest->setObjectName(objectName());

	dest->clear();
	for (T *value : _list)
		dest->copy(value);
}

template class MMGManager<MMGMessage>;
template class MMGManager<MMGAction>;
template class MMGManager<MMGBinding>;
template class MMGManager<MMGBindingManager>;
template class MMGManager<MMGDevice>;
template class MMGManager<MMGPreference>;
