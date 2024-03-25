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

#ifndef MMG_ACTION_H
#define MMG_ACTION_H

#include "../mmg-message.h"
#include "../mmg-signal.h"
#include "../ui/mmg-action-display.h"

class MMGLink;
class MMGActionManager;

class MMGAction : public QObject {
	Q_OBJECT

public:
	MMGAction(MMGActionManager *parent, const QJsonObject &json_obj);
	virtual ~MMGAction() = default;

	enum Category {
		MMGACTION_NONE,
		MMGACTION_STREAM,
		MMGACTION_RECORD,
		MMGACTION_VIRCAM,
		MMGACTION_REPBUF,
		MMGACTION_STUDIOMODE,
		MMGACTION_SCENE,
		MMGACTION_SOURCE_VIDEO,
		MMGACTION_SOURCE_AUDIO,
		MMGACTION_SOURCE_MEDIA,
		MMGACTION_TRANSITION,
		MMGACTION_FILTER,
		MMGACTION_HOTKEY,
		MMGACTION_PROFILE,
		MMGACTION_COLLECTION,
		MMGACTION_MIDI
	};

	virtual Category category() const = 0;
	virtual const QString trName() const = 0;
	virtual const QStringList subNames() const = 0;

	MMGUtils::DeviceType type() const { return _type; };
	short sub() const { return subcategory; };
	void setSub(short val) { subcategory = val; };

	void blog(int log_status, const QString &message) const;
	virtual void json(QJsonObject &action_obj) const;
	virtual void copy(MMGAction *dest) const;
	virtual void setEditable(bool){};
	virtual void toggle(){};

	virtual void createDisplay(QWidget *parent) { _display = new MMGActionDisplay(parent); };
	MMGActionDisplay *display() const { return _display; };

	virtual void setComboOptions(QComboBox *sub) { sub->addItems(subNames()); };
	virtual void setActionParams(){};

	const QString subModuleText(const QString &footer) const;
	const QStringList subModuleTextList(const QStringList &footer_list) const;

	virtual void execute(const MMGMessage *midi) const = 0;
	virtual void connectSignals(bool connect);

protected:
	bool _connected = false;

	void connectSourceSignal(const MMGSourceSignal *signal);
	void triggerEvent(const MMGUtils::MMGNumberList &values = {MMGUtils::MMGNumber()});

signals:
	void fulfilled(const MMGUtils::MMGNumberList &);

protected slots:
	virtual void frontendEventReceived(obs_frontend_event){};
	virtual void sourceEventReceived(MMGSourceSignal::Event, QVariant){};

private:
	const MMGUtils::DeviceType _type;
	short subcategory = 0;
	MMGActionDisplay *_display = nullptr;
};

using MMGActionList = QList<MMGAction *>;
QDataStream &operator<<(QDataStream &out, const MMGAction *&obj);
QDataStream &operator>>(QDataStream &in, MMGAction *&obj);

class MMGActionManager : public MMGManager<MMGAction> {

public:
	MMGActionManager(QObject *parent) : MMGManager(parent){};

	MMGAction *add(const QJsonObject &json_obj = QJsonObject()) override;
	MMGAction *copy(MMGAction *action) override;

	void setType(MMGUtils::DeviceType type) { _type = type; };
	void changeActionCategory(MMGAction *&action, const QJsonObject &json_obj = QJsonObject());

private:
	MMGUtils::DeviceType _type = MMGUtils::TYPE_INPUT;
};

#define ACTION_ASSERT(cond, str)                \
	if (!(cond)) {                          \
		blog(LOG_INFO, "FAILED: " str); \
		return;                         \
	}

#endif // MMG_ACTION_H
