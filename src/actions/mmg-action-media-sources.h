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

#pragma once
#include "mmg-action.h"

class MMGActionMediaSources : public MMGAction {
	Q_OBJECT

public:
	MMGActionMediaSources(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions {
		SOURCE_MEDIA_TOGGLE_PLAYPAUSE,
		SOURCE_MEDIA_RESTART,
		SOURCE_MEDIA_STOP,
		SOURCE_MEDIA_TIME,
		SOURCE_MEDIA_SKIP_FORWARD_TRACK,
		SOURCE_MEDIA_SKIP_BACKWARD_TRACK,
		SOURCE_MEDIA_SKIP_FORWARD_TIME,
		SOURCE_MEDIA_SKIP_BACKWARD_TIME
	};
	enum Events {
		SOURCE_MEDIA_PLAYED,
		SOURCE_MEDIA_PAUSED,
		SOURCE_MEDIA_RESTARTED,
		SOURCE_MEDIA_STOPPED,
		SOURCE_MEDIA_SKIPPED_FORWARD_TRACK,
		SOURCE_MEDIA_SKIPPED_BACKWARD_TRACK
	};

	Category category() const override { return MMGACTION_SOURCE_MEDIA; };
	const QString trName() const override { return "MediaSources"; };
	const QStringList subNames() const override;

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;
	void setEditable(bool edit) override;

	void createDisplay(QWidget *parent) override;
	void setActionParams() override;

	void execute(const MMGMessage *midi) const override;
	void connectSignals(bool connect) override;

	static const QStringList enumerate();
	double sourceDuration() const;

private:
	MMGUtils::MMGString source;
	MMGUtils::MMGNumber num;

private slots:
	void onList1Change();

	void sourceEventReceived(MMGSourceSignal::Event, QVariant) override;
};
