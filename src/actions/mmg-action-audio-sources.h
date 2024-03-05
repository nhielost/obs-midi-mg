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

class MMGActionAudioSources : public MMGAction {
	Q_OBJECT

public:
	MMGActionAudioSources(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions {
		SOURCE_AUDIO_VOLUME_CHANGETO,
		SOURCE_AUDIO_VOLUME_CHANGEBY,
		SOURCE_AUDIO_VOLUME_MUTE_ON,
		SOURCE_AUDIO_VOLUME_MUTE_OFF,
		SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF,
		SOURCE_AUDIO_OFFSET,
		SOURCE_AUDIO_MONITOR
	};
	enum Events {
		SOURCE_AUDIO_VOLUME_CHANGED,
		SOURCE_AUDIO_VOLUME_UNUSED,
		SOURCE_AUDIO_VOLUME_MUTED,
		SOURCE_AUDIO_VOLUME_UNMUTED,
		SOURCE_AUDIO_VOLUME_TOGGLE_MUTED,
		SOURCE_AUDIO_OFFSET_CHANGED,
		SOURCE_AUDIO_MONITOR_CHANGED
	};

	Category category() const override { return MMGACTION_SOURCE_AUDIO; };
	const QString trName() const override { return "AudioSources"; };
	const QStringList subNames() const override;

	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;
	void setEditable(bool edit) override;
	void toggle() override;

	void createDisplay(QWidget *parent) override;
	void setComboOptions(QComboBox *sub) override;
	void setActionParams() override;

	void execute(const MMGMessage *midi) const override;
	void connectSignals(bool connect) override;

	static const QStringList enumerate();
	static const QStringList audioMonitorOptions();
	static const QStringList volumeFormatOptions();
	static double convertDecibels(double value, bool convert_to);

private:
	MMGUtils::MMGString source;
	MMGUtils::MMGString action;
	MMGUtils::MMGNumber num;

private slots:
	void onList1Change();
	void onList2Change();

	void sourceEventReceived(MMGSourceSignal::Event, QVariant) override;
};
