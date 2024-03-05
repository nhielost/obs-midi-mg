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

#pragma once
#include "mmg-action.h"
#include "../ui/mmg-message-display.h"

#include <QThread>
#include <QQueue>
#include <mutex>

class MMGConnectionQueue;

class MMGActionMIDI : public MMGAction {
	Q_OBJECT

public:
	MMGActionMIDI(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions { MIDI_SEND_MESSAGES };
	enum Events { MIDI_MESSAGES_SENT };

	Category category() const override { return MMGACTION_MIDI; };
	const QString trName() const override { return "MIDI"; };
	const QStringList subNames() const override { return {subModuleText("Message")}; };

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(QWidget *parent) override;
	void setActionParams() override;

	void execute(const MMGMessage *midi) const override;
	void connectSignals(bool connect) override;

private:
	MMGMessageManager *messages;
	MMGConnectionQueue *_queue;

	MMGMessageDisplay *message_display = nullptr;

	friend class MMGConnectionQueue;
};

class MMGConnectionQueue : public QObject {
	Q_OBJECT

public:
	MMGConnectionQueue(MMGActionMIDI *parent);

	void blog(int log_status, const QString &message) const;

	void disconnectQueue();
	void connectQueue();
	void resetQueue();
	void resetConnections();

private slots:
	void messageFound(const MMGSharedMessage &);

private:
	QQueue<MMGMessage *> queue;

	MMGActionMIDI *action;
};
