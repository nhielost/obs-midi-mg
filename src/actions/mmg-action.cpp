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

#include "mmg-action.h"
#include "mmg-action-none.h"
#include "mmg-action-stream.h"
#include "mmg-action-record.h"
#include "mmg-action-virtualcam.h"
#include "mmg-action-replaybuffer.h"
#include "mmg-action-studiomode.h"
#include "mmg-action-scenes.h"
#include "mmg-action-video-sources.h"
#include "mmg-action-audio-sources.h"
#include "mmg-action-media-sources.h"
#include "mmg-action-transitions.h"
#include "mmg-action-filters.h"
#include "mmg-action-hotkeys.h"
#include "mmg-action-profiles.h"
#include "mmg-action-collections.h"
#include "mmg-action-midi.h"
#include "../mmg-config.h"

using namespace MMGUtils;

// MMGAction
MMGAction::MMGAction(MMGActionManager *parent, const QJsonObject &json_obj)
	: QObject(parent), _type((DeviceType)json_obj["type"].toInt())
{
	setObjectName(json_obj["name"].toString(mmgtr("Actions.Untitled")));
	subcategory = json_obj["sub"].toInt();
}

void MMGAction::blog(int log_status, const QString &message) const
{
	global_blog(log_status, QString("[Actions] <%1> %2").arg(objectName()).arg(message));
}

void MMGAction::json(QJsonObject &json_obj) const
{
	json_obj["name"] = objectName();
	json_obj["category"] = (int)category();
	json_obj["sub"] = subcategory;
	json_obj["type"] = type();
}

void MMGAction::copy(MMGAction *dest) const
{
	dest->setObjectName(objectName());
	dest->setSub(subcategory);
}

const QString MMGAction::subModuleText(const QString &footer) const
{
	return mmgtr(QString("Actions.%1.Sub.%2.%3")
			     .arg(trName())
			     .arg(type() == TYPE_OUTPUT ? "Output" : "Input")
			     .arg(footer)
			     .qtocs());
}

const QStringList MMGAction::subModuleTextList(const QStringList &footer_list) const
{
	QStringList opts;
	for (const QString &footer : footer_list)
		opts += subModuleText(footer);
	return opts;
}

void MMGAction::connectSignals(bool _connect)
{
	if (_type != TYPE_OUTPUT || _connect == _connected) return;

	mmgsignals()->disconnectAllSignals(this);
	_connected = _connect;
	if (!_connect) return;

	connect(mmgsignals(), &MMGSignals::frontendEvent, this, &MMGAction::frontendEventReceived);
}

void MMGAction::connectSourceSignal(const MMGSourceSignal *signal)
{
	connect(signal, &MMGSourceSignal::sourceChanged, this, &MMGAction::sourceEventReceived);
}

void MMGAction::triggerEvent(const MMGNumberList &values)
{
	emit fulfilled(values);
}

QDataStream &operator<<(QDataStream &out, const MMGAction *&obj)
{
	return out << (const QObject *&)obj;
}

QDataStream &operator>>(QDataStream &in, MMGAction *&obj)
{
	return in >> (QObject *&)obj;
}
// End MMGAction

// MMGActionManager
MMGAction *MMGActionManager::add(const QJsonObject &json_obj)
{
	MMGAction *action = nullptr;
	changeActionCategory(action, json_obj);
	if (find(action->objectName()) != action) setUniqueName(action);
	return action;
}

MMGAction *MMGActionManager::copy(MMGAction *action)
{
	QJsonObject json_obj;
	json_obj["name"] = action->objectName();
	json_obj["category"] = action->category();
	return MMGManager::copy(action, add(json_obj));
}

void MMGActionManager::changeActionCategory(MMGAction *&action, const QJsonObject &json_obj)
{
	MMGAction *new_action = nullptr;

	QJsonObject _json_obj = json_obj;
	_json_obj["type"] = _type;

	switch ((MMGAction::Category)json_obj["category"].toInt()) {
		case MMGAction::MMGACTION_STREAM:
			new_action = new MMGActionStream(this, _json_obj);
			break;

		case MMGAction::MMGACTION_RECORD:
			new_action = new MMGActionRecord(this, _json_obj);
			break;

		case MMGAction::MMGACTION_VIRCAM:
			new_action = new MMGActionVirtualCam(this, _json_obj);
			break;

		case MMGAction::MMGACTION_REPBUF:
			new_action = new MMGActionReplayBuffer(this, _json_obj);
			break;

		case MMGAction::MMGACTION_STUDIOMODE:
			new_action = new MMGActionStudioMode(this, _json_obj);
			break;

		case MMGAction::MMGACTION_SCENE:
			new_action = new MMGActionScenes(this, _json_obj);
			break;

		case MMGAction::MMGACTION_SOURCE_VIDEO:
			new_action = new MMGActionVideoSources(this, _json_obj);
			break;

		case MMGAction::MMGACTION_SOURCE_AUDIO:
			new_action = new MMGActionAudioSources(this, _json_obj);
			break;

		case MMGAction::MMGACTION_SOURCE_MEDIA:
			new_action = new MMGActionMediaSources(this, _json_obj);
			break;

		case MMGAction::MMGACTION_TRANSITION:
			new_action = new MMGActionTransitions(this, _json_obj);
			break;

		case MMGAction::MMGACTION_FILTER:
			new_action = new MMGActionFilters(this, _json_obj);
			break;

		case MMGAction::MMGACTION_HOTKEY:
			new_action = new MMGActionHotkeys(this, _json_obj);
			break;

		case MMGAction::MMGACTION_PROFILE:
			new_action = new MMGActionProfiles(this, _json_obj);
			break;

		case MMGAction::MMGACTION_COLLECTION:
			new_action = new MMGActionCollections(this, _json_obj);
			break;

		case MMGAction::MMGACTION_MIDI:
			new_action = new MMGActionMIDI(this, _json_obj);
			break;

		case 16:
			break;

		default:
			new_action = new MMGActionNone(this, _json_obj);
			break;
	}

	qsizetype index = _list.count();

	if (action) {
		index = _list.indexOf(action);
		remove(action);
	}

	_list.insert(index, new_action);
	action = new_action;
}
// End MMGActionManager
