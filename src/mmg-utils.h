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
#include "obs-midi-mg.h"

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QLCDNumber>
#include <QTimer>
#include <QScrollArea>
#include <QComboBox>
#include <QDateTime>
#include <QLabel>
#include <QLayout>

#define MMG_ENABLED if (editable)

void global_blog(int log_status, const QString &message);

class MMGMessage;
class MMGNumberDisplay;

namespace MMGUtils {

struct MMGPair {
  QString key;
  QVariant val;

  bool operator==(const MMGPair &other) const { return key == other.key && val == other.val; };
};

struct MMGNumber {
  MMGNumber(){};
  MMGNumber(const QJsonObject &json_obj, const QString &preferred, int fallback_num);

  enum State { NUMBERSTATE_FIXED, NUMBERSTATE_MIDI, NUMBERSTATE_CUSTOM, NUMBERSTATE_IGNORE };

  double num() const { return number; };
  double min() const { return lower; };
  double max() const { return higher; };
  State state() const { return num_state; };
  void set_num(double val) { MMG_ENABLED number = val; };
  void set_min(double val) { MMG_ENABLED lower = val; };
  void set_max(double val) { MMG_ENABLED higher = val; };
  void set_state(State state) { MMG_ENABLED num_state = state; }
  void set_state(short state) { MMG_ENABLED num_state = (State)state; }

  void json(QJsonObject &json_obj, const QString &prefix, bool use_bounds = true) const;
  MMGNumber copy() const { return *this; };
  void set_edit(bool edit) { editable = edit; };

  double choose(const MMGMessage *midi, double default_val = 0.0) const;

  operator double() const { return number; };
  double operator=(double val)
  {
    set_num(val);
    return number;
  };

  private:
  double number = 0.0;
  double lower = 0.0;
  double higher = 100.0;
  bool editable = true;

  State num_state = NUMBERSTATE_FIXED;
};
struct MMGString {
  MMGString(){};
  MMGString(const QJsonObject &json_obj, const QString &preferred, int fallback_num);

  enum State { STRINGSTATE_FIXED, STRINGSTATE_MIDI, STRINGSTATE_TOGGLE };

  const QString &str() const { return string; };
  State state() const { return str_state; };
  void set_str(const QString &val) { MMG_ENABLED string = val; };
  void set_state(State state) { MMG_ENABLED str_state = state; }
  void set_state(short state) { MMG_ENABLED str_state = (State)state; }

  void json(QJsonObject &json_obj, const QString &prefix, bool use_state = true) const;
  MMGString copy() const { return *this; };
  void set_edit(bool edit) { editable = edit; };

  operator const QString &() const { return string; };
  QString &operator=(const QString &val)
  {
    set_str(val);
    return string;
  };

  bool operator==(const char *ch) const { return string == ch; };
  bool operator!=(const char *ch) const { return string != ch; };

  private:
  QString string;
  State str_state = STRINGSTATE_FIXED;
  bool editable = true;
};

class MMGTimer : public QTimer {
  Q_OBJECT

  public:
  MMGTimer(QObject *parent = nullptr);

  signals:
  void resetting(int);
  void stopping();

  public slots:
  void stopTimer() { emit stopping(); };
  void reset(int time);
};

void set_message_labels(const QString &type, MMGNumberDisplay *note_display,
			MMGNumberDisplay *value_display);

const QByteArray json_to_str(const QJsonObject &json_obj);
const QJsonObject json_from_str(const QByteArray &str);
void debug_json(const QJsonValue &val);

bool bool_from_str(const QString &str);
double num_from_str(const QString &str);
QString num_to_str(int num, const QString &prefix = "");

void open_message_box(const QString &title, const QString &text);

void transfer_bindings(short mode, const QString &source, const QString &dest);

const vec3 get_obs_property_bounds(obs_property_t *prop);
const vec3 get_obs_property_bounds(obs_source_t *source, const QString &field);
const QList<MMGPair> get_obs_property_options(obs_property_t *prop);
const QList<MMGPair> get_obs_property_options(obs_source_t *source, const QString &field);
uint argb_abgr(uint rgb);

void obs_source_custom_update(obs_source_t *source, const QJsonObject &action_json,
			      const MMGMessage *midi_value);
}

#undef MMG_ENABLED
