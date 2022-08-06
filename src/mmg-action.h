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
#include "mmg-message.h"

#include <QComboBox>

class MMGAction {
public:
	MMGAction() = default;
	explicit MMGAction(const QJsonObject &obj);

	void json(QJsonObject &action_obj) const;

	void do_action(const MMGMessage *const prevMessage);

	enum class Category {
		MMGACTION_NONE,
		MMGACTION_STREAM,
		MMGACTION_RECORD,
		MMGACTION_VIRCAM,
		MMGACTION_STUDIOMODE,
		MMGACTION_SCENE,
		MMGACTION_SOURCE_TRANS,
		MMGACTION_SOURCE_PROPS,
		MMGACTION_MEDIA,
		MMGACTION_TRANSITION,
		MMGACTION_FILTER,
		MMGACTION_HOTKEY,
		MMGACTION_MIDIMESSAGE,
		MMGACTION_WAIT
	};

	enum class None { NONE_NONE };
	enum class Stream { STREAM_ON, STREAM_OFF, STREAM_TOGGLE_ONOFF };
	enum class Record {
		RECORD_ON,
		RECORD_OFF,
		RECORD_TOGGLE_ONOFF,
		RECORD_PAUSE,
		RECORD_RESUME,
		RECORD_TOGGLE_PAUSE
	};
	enum class VirtualCam { VIRCAM_ON, VIRCAM_OFF, VIRCAM_TOGGLE_ONOFF };
	enum class StudioMode {
		STUDIOMODE_ON,
		STUDIOMODE_OFF,
		STUDIOMODE_TOGGLE_ONOFF,
		STUDIOMODE_CHANGEPREVIEW,
		STUDIOMODE_TRANSITION
	};
	enum class Scenes { SCENE_SCENE };
	enum class SourceTransform {
		TRANSFORM_POSITION,
		TRANSFORM_DISPLAY,
		TRANSFORM_LOCKED,
		TRANSFORM_CROP,
		TRANSFORM_SCALE,
		TRANSFORM_ROTATION,
		TRANSFORM_BOUNDINGBOX,
		TRANSFORM_RESET
	};
	enum class SourceProperties {
		PROPERTY_VOLUME_CHANGETO,
		PROPERTY_VOLUME_CHANGEBY,
		PROPERTY_VOLUME_MUTE_ON,
		PROPERTY_VOLUME_MUTE_OFF,
		PROPERTY_VOLUME_MUTE_TOGGLE_ONOFF,
		PROPERTY_SCREENSHOT,
		PROPERTY_AUDIO_OFFSET,
		PROPERTY_AUDIO_MONITOR,
		PROPERTY_CUSTOM
	};
	enum class Media {
		MEDIA_TOGGLE_PLAYPAUSE,
		MEDIA_RESTART,
		MEDIA_STOP,
		MEDIA_TIME,
		MEDIA_SKIP_FORWARD_TRACK,
		MEDIA_SKIP_BACKWARD_TRACK,
		MEDIA_SKIP_FORWARD_TIME,
		MEDIA_SKIP_BACKWARD_TIME
	};
	enum class Transitions {
		TRANSITION_CURRENT,
		/*TRANSITION_TBAR,*/ TRANSITION_SOURCE_SHOW,
		TRANSITION_SOURCE_HIDE
	};
	enum class Filters {
		FILTER_SHOW,
		FILTER_HIDE,
		FILTER_TOGGLE_SHOWHIDE,
		FILTER_REORDER,
		FILTER_CUSTOM
	};
	enum class Hotkeys { HOTKEY_HOTKEY };
	enum class SendMidi { SENDMIDI_SENDMIDI };
	enum class Sleep { WAIT_MS, WAIT_S };

	const QString get_name() const { return name; };
	Category get_category() const { return (Category)category; };
	int get_sub() const { return subcategory; };
	const QString get_str(int index) const
	{
		if (index >= 0 && index < 4)
			return strs[index];
		throw 1;
	};
	const QString *get_strs() const { return strs; };
	double get_num(int index) const
	{
		if (index >= 0 && index < 4)
			return nums[index];
		throw 1;
	};
	const double *get_nums() const { return nums; };

	void set_name(const QString &val) { name = val; };
	void set_category(Category val) { category = (int)val; };
	void set_sub(int val) { subcategory = val; };
	void set_str(int index, const QString &val)
	{
		if (index >= 0 && index < 3)
			strs[index] = val;
	};
	void set_num(int index, double val)
	{
		if (index >= 0 && index < 4)
			nums[index] = val;
	};

	static Category categoryFromString(const QString &str);

	static void do_obs_scene_enum(QComboBox *list);
	static void
	do_obs_source_enum(QComboBox *list,
			   MMGAction::Category restriction_t =
				   MMGAction::Category::MMGACTION_NONE,
			   const QString &rest = "");
	static void
	do_obs_media_enum(QComboBox *list,
			  MMGAction::Category restriction_t =
				  MMGAction::Category::MMGACTION_NONE,
			  const QString &rest = "");
	static void
	do_obs_transition_enum(QComboBox *list,
			       MMGAction::Category restriction_t =
				       MMGAction::Category::MMGACTION_NONE,
			       const QString &rest = "");
	static void
	do_obs_filter_enum(QComboBox *list,
			   MMGAction::Category restriction_t =
				   MMGAction::Category::MMGACTION_NONE,
			   const QString &rest = "");
	static void do_obs_hotkey_enum(QComboBox *list);

private:
	QString name = MMGUtils::next_default_name(MMGModes::MMGMODE_ACTION);
	int category = 0;
	int subcategory = 0;
	QString strs[4]{"", "", "", ""};
	double nums[4]{0.0, 0.0, 0.0, 0.0};

	static void do_action_none(None kind);
	static void do_action_stream(Stream kind);
	static void do_action_record(Record kind);
	static void do_action_virtual_cam(VirtualCam kind);
	static void do_action_studio_mode(StudioMode kind,
					  const QString &scene = "",
					  uint value = 0U);
	static void do_action_scenes(Scenes kind, const QString &scene = "",
				     uint value = 0U);
	static void do_action_source_transform(SourceTransform kind,
					       const QString strs[4],
					       const double nums[4] = nullptr,
					       uint value = 0U);
	static void do_action_source_properties(SourceProperties kind,
						const QString &source,
						double num = 0.0,
						uint value = 0U);
	static void do_action_media(Media kind, const QString &source,
				    double time = 0.0, uint value = 0U);
	static void do_action_transitions(Transitions kind,
					  const QString names[4] = nullptr,
					  double time = 0, uint value = 0U);
	static void do_action_filters(Filters kind,
				      const QString names[4] = nullptr,
				      int index = 0, uint value = 0U);
	static void do_action_hotkeys(Hotkeys kind, const QString &name = "",
				      uint value = 0U);
	static void do_action_send_midi(SendMidi kind,
					const MMGMessage *const sender);
	static void do_action_sleep(Sleep kind, ulong duration = 0UL,
				    uint value = 0U);

	static double num_or_value(double original, double value,
				   double multiplier)
	{
		return original == -1 ? qRound((value / 128.0) * multiplier)
				      : original;
	}
};

using MMGActionList = QList<MMGAction *>;
