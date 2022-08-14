/*
obs-midi-mg
Copyright (C) 2022 nhielost <nhielost@gmail.com>

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
#include "obs-midi-mg.h"
#include <libremidi/libremidi.hpp>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QLCDNumber>
#include <QDateTime>

namespace MMGUtils {

class LCDData {
public:
	LCDData() = default;
	explicit LCDData(QLCDNumber *lcd_ptr, std::function<void(double)> func);

	void set_range(double min, double max);
	void set_step(double minor, double major);

	void set_use_time(bool time) { use_time = time; }

	void down_major();
	void down_minor();
	void up_minor();
	void up_major();
	void reset(double value = 0.0);

private:
	QLCDNumber *lcd;
	std::function<void(double)> value_func;

	double maximum = 100.0;
	double minimum = 0.0;
	double minor_step = 1.0;
	double major_step = 10.0;
	double internal_val = 0.0;

	bool use_time = false;

	void display();
};
Q_DECLARE_METATYPE(LCDData);

void call_midi_callback(const libremidi::message &message);

bool json_key_exists(const QJsonObject &obj, QString key,
		     QJsonValue::Type value_type);
bool json_is_valid(const QJsonValue &value, QJsonValue::Type value_type);

bool bool_from_str(const QString &str);

std::pair<uint, uint> get_obs_dimensions();
std::pair<uint, uint> get_obs_source_dimensions(const QString &name);
uint get_obs_scene_count();
double get_obs_media_length(const QString &name);
qulonglong get_obs_source_filter_count(const QString &name);

QString next_default_name(MMGModes mode);
}
