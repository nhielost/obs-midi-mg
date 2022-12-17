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

#include <QJsonDocument>
#include <QHash>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QLCDNumber>
#include <QComboBox>
#include <QDateTime>
#include <QLabel>

void global_blog(int log_status, const QString &message);

class MMGMessage;
class MMGAction;

namespace MMGUtils {

struct MMGNumber {
  MMGNumber(){};
  MMGNumber(const QJsonObject &json_obj, const QString &preferred, int fallback_num);

  enum State { NUMBERSTATE_FIXED, NUMBERSTATE_MIDI, NUMBERSTATE_MIDI_INVERT, NUMBERSTATE_IGNORE };

  double num() const { return number; };
  State state() const { return num_state; };
  void set_num(double val) { number = val; };
  void set_state(State state) { num_state = state; }

  void json(QJsonObject &json_obj, const QString &prefix, bool use_state) const;
  void copy(MMGNumber *dest) const;

  double choose(const MMGMessage *midi, double default_val = 0.0, double mult = 128.0,
		bool add_one = false);

  operator double() const { return number; };
  double operator=(double val) { return number = val; };

  private:
  double number = 0.0;
  State num_state{NUMBERSTATE_FIXED};
};
struct MMGString {
  MMGString(){};
  MMGString(const QJsonObject &json_obj, const QString &preferred, int fallback_num);

  enum State { STRINGSTATE_FIXED, STRINGSTATE_MIDI, STRINGSTATE_TOGGLE };

  const QString &str() const { return string; };
  State state() const { return str_state; };
  void set_str(const QString &val) { string = val; };
  void set_state(State state) { str_state = state; }

  void json(QJsonObject &json_obj, const QString &prefix, bool use_state) const;
  void copy(MMGString *dest) const;

  operator const QString &() const { return string; };
  QString &operator=(const QString &val) { return string = val; };

  bool operator==(const char *ch) const { return string == ch; };
  bool operator!=(const char *ch) const { return string != ch; };

  private:
  QString string;
  State str_state{STRINGSTATE_FIXED};
};

class LCDData {
  public:
  LCDData(){};
  explicit LCDData(QLCDNumber *lcd_ptr);

  void set_storage(MMGNumber *number);
  void can_set(bool can_set) { editable = can_set; };

  double get_minor_step() const { return minor_step; }
  double get_major_step() const { return major_step; }
  void set_range(double min, double max);
  void set_step(double minor, double major);

  void set_value(double val);
  void set_default_value(double val) { default_val = val; };
  void reset();

  void set_state(int index);

  void set_use_time(bool time) { use_time = time; }

  void down_major();
  void down_minor();
  void up_minor();
  void up_major();

  void display();

  private:
  QLCDNumber *lcd = nullptr;
  MMGNumber *storage = nullptr;
  bool editable = true;

  double maximum = 100.0;
  double minimum = 0.0;
  double minor_step = 1.0;
  double major_step = 10.0;

  double default_val = 0.0;

  bool use_time = false;
};

struct MMGActionDisplayParams {
  enum FieldDisplay : int {
    DISPLAY_NONE = 0,
    DISPLAY_STR1 = 1,   // STR1
    DISPLAY_STR2 = 3,   // STR1 + STR2
    DISPLAY_STR3 = 7,   // STR1 + STR2 + STR3
    DISPLAY_NUM1 = 8,   // NUM1
    DISPLAY_NUM2 = 24,  // NUM1 + NUM2
    DISPLAY_NUM3 = 56,  // NUM1 + NUM2 + NUM3
    DISPLAY_NUM4 = 120, // NUM1 + NUM2 + NUM3 + NUM4
    DISPLAY_SEC = 128
  };
  using FieldDisplayFlags = int;

  enum LCDComboDisplay : short {
    COMBODISPLAY_FIXED = 0,
    COMBODISPLAY_MIDI = 1,
    COMBODISPLAY_MIDI_INVERT = 2,
    COMBODISPLAY_IGNORE = 4,
    COMBODISPLAY_ALL = 7
  };
  using LCDComboDisplayFlags = short;

  void clear()
  {
    display = DISPLAY_NONE;

    label_text = "";
    list.clear();

    combo_display[0] = COMBODISPLAY_MIDI;
    combo_display[1] = COMBODISPLAY_MIDI;
    combo_display[2] = COMBODISPLAY_MIDI;
    combo_display[3] = COMBODISPLAY_MIDI;
    label_lcds[0] = "";
    label_lcds[1] = "";
    label_lcds[2] = "";
    label_lcds[3] = "";
  };

  // Window will fill in the LCD
  // MMGAction will respond with these values full

  bool initializing = false;
  FieldDisplayFlags display = DISPLAY_NONE;

  QString label_text;
  QStringList list;

  LCDComboDisplayFlags combo_display[4]{COMBODISPLAY_MIDI, COMBODISPLAY_MIDI, COMBODISPLAY_MIDI,
					COMBODISPLAY_MIDI};
  QString label_lcds[4];
  LCDData *lcds[4];

  QString extra_data;
};

void call_midi_callback(const libremidi::message &message);

bool json_key_exists(const QJsonObject &obj, const QString &key, QJsonValue::Type value_type);
bool json_is_valid(const QJsonValue &value, QJsonValue::Type value_type);
const QByteArray json_to_str(const QJsonObject &json_obj);
const QJsonObject json_from_str(const QByteArray &str);

bool bool_from_str(const QString &str);
QString num_to_str(int num, const QString &prefix);

void open_message_box(const QString &title, const QString &text);

void transfer_bindings(short mode, const QString &source, const QString &dest);

vec2 get_obs_dimensions();
vec2 get_obs_source_dimensions(const QString &name);

obs_source_t *get_obs_transition_by_name(const QString &name);
bool get_obs_transition_fixed_length(const QString &name);

double get_obs_media_length(const QString &name);

const vec3 get_obs_filter_property_bounds(obs_property_t *prop);
const vec3 get_obs_filter_property_bounds(obs_source_t *filter, const QString &field);
const QHash<QString, QString> get_obs_filter_property_options(obs_property_t *prop);
const QHash<QString, QString> get_obs_filter_property_options(obs_source_t *filter,
							      const QString &field);
}
Q_DECLARE_METATYPE(MMGUtils::LCDData);
