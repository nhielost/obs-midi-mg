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

#ifndef MMG_LINK_H
#define MMG_LINK_H

#include "messages/mmg-message-data.h"
#include "mmg-mapping.h"

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
	void execute(const MMGMappingTest &);

	void run() override;
	void executeInput();
	void executeOutput();

	void changeName(const QString &name);

private:
	MMGBinding *binding;
	MMGMappingTest _test;

	std::timed_mutex mutex;
	bool locked = false;

	static uint16_t thread_count;
};
#endif // MMG_LINK_H
