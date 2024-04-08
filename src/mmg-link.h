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

#ifndef MMG_LINK_H
#define MMG_LINK_H

#include "mmg-utils.h"
#include "mmg-message.h"

#include <QThread>
#include <mutex>

class MMGBinding;

class MMGLink : public QThread {
	Q_OBJECT

public:
	MMGLink(MMGBinding *parent);
	~MMGLink()
	{
		if (locked) mutex.unlock();
	}

	void blog(int log_status, const QString &message) const;
	void establish(bool connect);

private:
	void execute();

	void run() override;
	void executeInput();
	void executeOutput();

public slots:
	void messageReceived(const MMGSharedMessage &);
	void actionFulfilled(const MMGUtils::MMGNumberList &);

private:
	MMGBinding *binding;
	MMGMessage *incoming_message;
	MMGUtils::MMGNumberList incoming_nums;

	std::timed_mutex mutex;
	bool locked = false;

	static short thread_count;
};
#endif // MMG_LINK_H
