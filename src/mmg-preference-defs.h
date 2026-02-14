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

#pragma once

#include "mmg-preference.h"

namespace MMGPreferences {

class MMGPreferenceGeneral : public MMGPreference {
	Q_OBJECT

public:
	MMGPreferenceGeneral(MMGPreferenceManager *parent, const QJsonObject &json_obj)
		: MMGPreference(parent, json_obj)
	{
		self = this;
	};

	Id id() const override { return preferenceId(); };
	static Id preferenceId() { return Id(0x0001); };
	const char *trPreferenceName() const override { return "General"; };

	void createDisplay(QWidget *widget) override;

private:
	void exportBindings() const;
	void importBindings() const;
	void openHelp() const;
	void reportBug() const;

	static MMGPreferenceGeneral *self;
};
MMG_DECLARE_PREFERENCE(MMGPreferenceGeneral);

class MMGPreferenceMIDI : public MMGPreference {
	Q_OBJECT

public:
	MMGPreferenceMIDI(MMGPreferenceManager *parent, const QJsonObject &json_obj) : MMGPreference(parent, json_obj)
	{
		self = this;
	};

	enum MessageMode : uint8_t {
		MIDI_ALWAYS_1,
		MIDI_ALWAYS_2,
	};

	Id id() const override { return preferenceId(); };
	static Id preferenceId() { return Id(0x0101); };
	const char *trPreferenceName() const override { return "MIDI"; };

	void load(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;

	void createDisplay(QWidget *widget) override;

	static uint32_t currentAPI() { return self->midi_api; };
	static MessageMode currentMessageMode() { return self->message_mode; };

private:
	void setMessageMode(const MessageMode &);
	void setMIDIAPI(const uint32_t &);

	void initMIDI();
	static void refreshAPIs();

private:
	uint32_t midi_api = 0xffffffff;
	MessageMode message_mode = MIDI_ALWAYS_1;

	static MMGPreferenceMIDI *self;
};
MMG_DECLARE_PREFERENCE(MMGPreferenceMIDI);

class MMGPreferenceAbout : public MMGPreference {
	Q_OBJECT

public:
	MMGPreferenceAbout(MMGPreferenceManager *parent, const QJsonObject &json_obj) : MMGPreference(parent, json_obj)
	{
		self = this;
	};

	Id id() const override { return preferenceId(); };
	static Id preferenceId() { return Id(0xffff); };
	const char *trPreferenceName() const override { return "About"; };

	void createDisplay(QWidget *widget) override;

private:
	void openAuthor() const;
	void checkForUpdates() const;

	static MMGPreferenceAbout *self;
};
MMG_DECLARE_PREFERENCE(MMGPreferenceAbout);

} // namespace MMGPreferences
