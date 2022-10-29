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
	explicit MMGAction();
	explicit MMGAction(const QJsonObject &obj);

	void json(QJsonObject &action_obj) const;

	void do_action(const MMGSharedMessage &params);

	enum class Category {
		MMGACTION_NONE,
		MMGACTION_STREAM,
		MMGACTION_RECORD,
		MMGACTION_VIRCAM,
		MMGACTION_REPBUF,
		MMGACTION_STUDIOMODE,
		MMGACTION_SCENE,
		MMGACTION_SOURCE_VIDEO,
		MMGACTION_SOURCE_AUDIO,
		MMGACTION_SOURCE_MEDIA,
		MMGACTION_TRANSITION,
		MMGACTION_FILTER,
		MMGACTION_HOTKEY,
		MMGACTION_PROFILE,
		MMGACTION_COLLECTION,
		MMGACTION_MIDI,
		MMGACTION_INTERNAL,
		MMGACTION_TIMEOUT
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
	enum class ReplayBuffer {
		REPBUF_ON,
		REPBUF_OFF,
		REPBUF_TOGGLE_ONOFF,
		REPBUF_SAVE
	};
	enum class StudioMode {
		STUDIOMODE_ON,
		STUDIOMODE_OFF,
		STUDIOMODE_TOGGLE_ONOFF,
		STUDIOMODE_CHANGEPREVIEW,
		STUDIOMODE_TRANSITION
	};
	enum class Scenes { SCENE_SCENE };
	enum class VideoSources {
		SOURCE_VIDEO_POSITION,
		SOURCE_VIDEO_DISPLAY,
		SOURCE_VIDEO_LOCKED,
		SOURCE_VIDEO_CROP,
		SOURCE_VIDEO_ALIGNMENT,
		SOURCE_VIDEO_SCALE,
		SOURCE_VIDEO_SCALEFILTER,
		SOURCE_VIDEO_ROTATION,
		SOURCE_VIDEO_BOUNDING_BOX_TYPE,
		SOURCE_VIDEO_BOUNDING_BOX_SIZE,
		SOURCE_VIDEO_BOUNDING_BOX_ALIGN,
		SOURCE_VIDEO_BLEND_MODE,
		SOURCE_VIDEO_SCREENSHOT,
		SOURCE_VIDEO_CUSTOM
	};
	enum class AudioSources {
		SOURCE_AUDIO_VOLUME_CHANGETO,
		SOURCE_AUDIO_VOLUME_CHANGEBY,
		SOURCE_AUDIO_VOLUME_MUTE_ON,
		SOURCE_AUDIO_VOLUME_MUTE_OFF,
		SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF,
		SOURCE_AUDIO_OFFSET,
		SOURCE_AUDIO_MONITOR,
		SOURCE_AUDIO_CUSTOM
	};
	enum class MediaSources {
		SOURCE_MEDIA_TOGGLE_PLAYPAUSE,
		SOURCE_MEDIA_RESTART,
		SOURCE_MEDIA_STOP,
		SOURCE_MEDIA_TIME,
		SOURCE_MEDIA_SKIP_FORWARD_TRACK,
		SOURCE_MEDIA_SKIP_BACKWARD_TRACK,
		SOURCE_MEDIA_SKIP_FORWARD_TIME,
		SOURCE_MEDIA_SKIP_BACKWARD_TIME
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
	enum class Profiles { PROFILE_PROFILE };
	enum class Collections { COLLECTION_COLLECTION };
	enum class MidiMessage { MIDI_MIDI };
	enum class Internal { INTERNAL_1, INTERNAL_2, INTERNAL_3 };
	enum class Timeout { TIMEOUT_MS, TIMEOUT_S };

	void blog(int log_status, const QString &message) const;

	Category get_category() const { return (Category)category; };
	int get_sub() const { return subcategory; };
	const QString &get_str(int index) const
	{
		if (index >= 0 && index < 4)
			return strs[index];
		throw 1;
	};
	double get_num(int index) const
	{
		if (index >= 0 && index < 4)
			return nums[index];
		throw 1;
	};

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

	void deep_copy(MMGAction *dest);
	void reset_execution() { executed = false; };

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
	static void do_obs_profile_enum(QComboBox *list);
	static void do_obs_collection_enum(QComboBox *list);
	static void do_mmg_binding_enum(QComboBox *list,
					const QString &current);

private:
	int category;
	int subcategory;
	QString strs[4];
	double nums[4];

	bool executed = false;

	template<typename T>
	static void do_action_specific(const MMGAction *params,
				       const MMGMessage *midi);
};
