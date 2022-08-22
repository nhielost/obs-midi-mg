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

#include "mmg-action.h"
#include "mmg-utils.h"
#include "mmg-config.h"

#include <obs-frontend-api.h>

#include <thread>

using namespace MMGUtils;

struct R {
	QComboBox *c;
	MMGAction::Category r_t;
	QString r_v;
};

qulonglong MMGAction::next_default = 0;

MMGAction::MMGAction()
{
	name = get_next_default_name();
	category = 0;
	subcategory = 0;
	set_str(0, "");
	set_str(1, "");
	set_str(2, "");
	set_num(0, 0.0);
	set_num(1, 0.0);
	set_num(2, 0.0);
	set_num(3, 0.0);
}

MMGAction::MMGAction(const QJsonObject &obj)
{
	name = obj["name"].toString();
	if (name.isEmpty())
		name = get_next_default_name();
	category = obj["category"].toInt(0);
	subcategory = obj["sub"].toInt(0);
	set_str(0, obj["str1"].toString());
	set_str(1, obj["str2"].toString());
	set_str(2, obj["str3"].toString());
	set_num(0, obj["num1"].toDouble());
	set_num(1, obj["num2"].toDouble());
	set_num(2, obj["num3"].toDouble());
	set_num(3, obj["num4"].toDouble());
}

void MMGAction::json(QJsonObject &action_obj) const
{
	action_obj["name"] = name;
	action_obj["category"] = (int)category;
	action_obj["sub"] = subcategory;
	// Really does not like QJsonArray here for some reason
	action_obj["str1"] = get_str(0);
	action_obj["str2"] = get_str(1);
	action_obj["str3"] = get_str(2);
	action_obj["num1"] = get_num(0);
	action_obj["num2"] = get_num(1);
	action_obj["num3"] = get_num(2);
	action_obj["num4"] = get_num(3);
}

QString MMGAction::get_next_default_name()
{
	return QVariant(++MMGAction::next_default)
		.toString()
		.prepend("Untitled Action ");
}

MMGAction::Category MMGAction::categoryFromString(const QString &str)
{
	if (str == "None") {
		return MMGAction::Category::MMGACTION_NONE;
	} else if (str == "Streaming") {
		return MMGAction::Category::MMGACTION_STREAM;
	} else if (str == "Recording") {
		return MMGAction::Category::MMGACTION_RECORD;
	} else if (str == "Virtual Camera") {
		return MMGAction::Category::MMGACTION_VIRCAM;
	} else if (str == "Studio Mode") {
		return MMGAction::Category::MMGACTION_STUDIOMODE;
	} else if (str == "Scene Switching") {
		return MMGAction::Category::MMGACTION_SCENE;
	} else if (str == "Video Sources") {
		return MMGAction::Category::MMGACTION_SOURCE_VIDEO;
	} else if (str == "Audio Sources") {
		return MMGAction::Category::MMGACTION_SOURCE_AUDIO;
	} else if (str == "Media Sources") {
		return MMGAction::Category::MMGACTION_SOURCE_MEDIA;
	} else if (str == "Transitions") {
		return MMGAction::Category::MMGACTION_TRANSITION;
	} else if (str == "Filters") {
		return MMGAction::Category::MMGACTION_FILTER;
	} else if (str == "Hotkeys") {
		return MMGAction::Category::MMGACTION_HOTKEY;
	} else if (str == "Profiles") {
		return MMGAction::Category::MMGACTION_PROFILE;
	} else if (str == "Scene Collections") {
		return MMGAction::Category::MMGACTION_COLLECTION;
	} else if (str == "OBS Studio UI") {
		return MMGAction::Category::MMGACTION_UI;
	} else if (str == "MIDI") {
		return MMGAction::Category::MMGACTION_MIDI;
	} else if (str == "Timeout") {
		return MMGAction::Category::MMGACTION_TIMEOUT;
	}
	return MMGAction::Category::MMGACTION_NONE;
}

void MMGAction::do_obs_scene_enum(QComboBox *list)
{
	char **scene_names = obs_frontend_get_scene_names();

	for (int i = 0; scene_names[i] != 0; ++i) {
		list->addItem(scene_names[i]);
	}

	bfree(scene_names);
}

void MMGAction::do_obs_source_enum(QComboBox *list,
				   MMGAction::Category restriction_t,
				   const QString &rest)
{
	R r;
	r.c = list;
	r.r_t = restriction_t;
	r.r_v = rest;

	obs_enum_all_sources(
		[](void *param, obs_source_t *source) {
			auto r = reinterpret_cast<R *>(param);

			switch (r->r_t) {
			case MMGAction::Category::MMGACTION_SCENE:
				if (!OBSSceneItemAutoRelease(
					    obs_scene_sceneitem_from_source(
						    OBSSceneAutoRelease(
							    obs_get_scene_by_name(
								    r->r_v.qtocs())),
						    source)))
					return true;
				break;
			case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
				if (obs_source_get_type(source) !=
				    OBS_SOURCE_TYPE_INPUT)
					return true;
				if (!(obs_source_get_output_flags(source) &
				      OBS_SOURCE_VIDEO))
					return true;
				break;
			case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
				if (obs_source_get_type(source) !=
				    OBS_SOURCE_TYPE_INPUT)
					return true;
				if (!(obs_source_get_output_flags(source) &
				      OBS_SOURCE_AUDIO))
					return true;
				break;
			default:
				if (obs_source_get_type(source) !=
				    OBS_SOURCE_TYPE_INPUT)
					return true;
				break;
			}

			r->c->addItem(obs_source_get_name(source));
			return true;
		},
		&r);
}

void MMGAction::do_obs_media_enum(QComboBox *list,
				  MMGAction::Category restriction_t,
				  const QString &rest)
{
	R r;
	r.c = list;
	r.r_t = restriction_t;
	r.r_v = rest;

	obs_enum_sources(
		[](void *param, obs_source_t *source) {
			auto r = reinterpret_cast<R *>(param);
			if (!(obs_source_get_output_flags(source) &
			      OBS_SOURCE_CONTROLLABLE_MEDIA))
				return true;

			r->c->addItem(obs_source_get_name(source));
			return true;
		},
		&r);
}

void MMGAction::do_obs_transition_enum(QComboBox *list,
				       MMGAction::Category restriction_t,
				       const QString &rest)
{
	R r;
	r.c = list;
	r.r_t = restriction_t;
	r.r_v = rest;

	obs_enum_all_sources(
		[](void *param, obs_source_t *source) {
			auto r = reinterpret_cast<R *>(param);
			if (obs_source_get_type(source) !=
			    OBS_SOURCE_TYPE_TRANSITION)
				return true;

			r->c->addItem(obs_source_get_name(source));
			return true;
		},
		&r);
}

void MMGAction::do_obs_filter_enum(QComboBox *list,
				   MMGAction::Category restriction_t,
				   const QString &rest)
{
	if (restriction_t == MMGAction::Category::MMGACTION_SOURCE_VIDEO) {
		OBSSourceAutoRelease source =
			obs_get_source_by_name(rest.qtocs());
		obs_source_enum_filters(
			source,
			[](obs_source_t *source, obs_source_t *filter,
			   void *param) {
				Q_UNUSED(source);
				auto r = reinterpret_cast<QComboBox *>(param);
				r->addItem(obs_source_get_name(filter));
			},
			list);
		return;
	}

	R r;
	r.c = list;
	r.r_t = restriction_t;
	r.r_v = rest;

	obs_enum_all_sources(
		[](void *param, obs_source_t *source) {
			auto r = reinterpret_cast<R *>(param);
			if (obs_source_get_type(source) !=
			    OBS_SOURCE_TYPE_FILTER)
				return true;

			r->c->addItem(obs_source_get_name(source));
			return true;
		},
		&r);
}

void MMGAction::do_obs_hotkey_enum(QComboBox *list)
{
	obs_enum_hotkeys(
		[](void *param, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			Q_UNUSED(id);
			auto helper = reinterpret_cast<QComboBox *>(param);
			if (obs_hotkey_get_registerer_type(hotkey) !=
			    OBS_HOTKEY_REGISTERER_FRONTEND)
				return true;
			helper->addItem(obs_hotkey_get_description(hotkey));
			return true;
		},
		list);
}

void MMGAction::do_action(const MMGSharedMessage &params)
{
	switch (get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
		do_action_none(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_STREAM:
		do_action_stream(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_RECORD:
		do_action_record(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_VIRCAM:
		do_action_virtual_cam(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		do_action_studio_mode(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		do_action_scenes(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
		do_action_video_source(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
		do_action_audio_source(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:
		do_action_media_source(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		do_action_transitions(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		do_action_filters(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_HOTKEY:
		do_action_hotkeys(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_PROFILE:
		do_action_profiles(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_COLLECTION:
		do_action_collections(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_UI:
		do_action_ui(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_MIDI:
		do_action_midi(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_TIMEOUT:
		do_action_pause(this, params.get());
		break;
	default:
		break;
	}
}

void MMGAction::do_action_none(const MMGAction *params, const MMGMessage *midi)
{
	Q_UNUSED(params);
	Q_UNUSED(midi);
}

void MMGAction::do_action_stream(const MMGAction *params,
				 const MMGMessage *midi)
{
	Q_UNUSED(midi);
	switch ((MMGAction::Stream)params->get_sub()) {
	case MMGAction::Stream::STREAM_ON:
		obs_frontend_streaming_start();
		break;
	case MMGAction::Stream::STREAM_OFF:
		obs_frontend_streaming_stop();
		break;
	case MMGAction::Stream::STREAM_TOGGLE_ONOFF:
		if (obs_frontend_streaming_active()) {
			obs_frontend_streaming_stop();
		} else {
			obs_frontend_streaming_start();
		}
		break;
	default:
		break;
	}
}

void MMGAction::do_action_record(const MMGAction *params,
				 const MMGMessage *midi)
{
	Q_UNUSED(midi);
	switch ((MMGAction::Record)params->get_sub()) {
	case MMGAction::Record::RECORD_ON:
		obs_frontend_recording_start();
		break;
	case MMGAction::Record::RECORD_OFF:
		obs_frontend_recording_stop();
		break;
	case MMGAction::Record::RECORD_TOGGLE_ONOFF:
		if (obs_frontend_recording_active()) {
			obs_frontend_recording_stop();
		} else {
			obs_frontend_recording_start();
		}
		break;
	case MMGAction::Record::RECORD_PAUSE:
		obs_frontend_recording_pause(true);
		break;
	case MMGAction::Record::RECORD_RESUME:
		obs_frontend_recording_pause(false);
		break;
	case MMGAction::Record::RECORD_TOGGLE_PAUSE:
		obs_frontend_recording_pause(!obs_frontend_recording_paused());
		break;
	default:
		break;
	}
}

void MMGAction::do_action_virtual_cam(const MMGAction *params,
				      const MMGMessage *midi)
{
	Q_UNUSED(midi);
	switch ((MMGAction::VirtualCam)params->get_sub()) {
	case MMGAction::VirtualCam::VIRCAM_ON:
		obs_frontend_start_virtualcam();
		break;
	case MMGAction::VirtualCam::VIRCAM_OFF:
		obs_frontend_stop_virtualcam();
		break;
	case MMGAction::VirtualCam::VIRCAM_TOGGLE_ONOFF:
		if (obs_frontend_virtualcam_active()) {
			obs_frontend_stop_virtualcam();
		} else {
			obs_frontend_start_virtualcam();
		}
		break;
	default:
		break;
	}
}

void MMGAction::do_action_studio_mode(const MMGAction *params,
				      const MMGMessage *midi)
{
	if (params->get_str(0) == "Use Message Value" &&
	    (uint)midi->get_value() >= get_obs_scene_count())
		return;
	// For new Studio Mode activation (pre 28.0.0 method encounters threading errors)
	auto set_studio_mode = [](bool on) {
		if (obs_frontend_preview_program_mode_active() == on)
			return;
		obs_queue_task(
			OBS_TASK_UI,
			[](void *param) {
				auto enabled = (bool *)param;
				obs_frontend_set_preview_program_mode(*enabled);
			},
			&on, true);
	};
	char **scene_names = obs_frontend_get_scene_names();
	OBSSourceAutoRelease source_obs_scene =
		obs_get_source_by_name(params->get_str(0) == "Use Message Value"
					       ? scene_names[midi->get_value()]
					       : params->get_str(0).qtocs());
	switch ((MMGAction::StudioMode)params->get_sub()) {
	case MMGAction::StudioMode::STUDIOMODE_ON:
		set_studio_mode(true);
		break;
	case MMGAction::StudioMode::STUDIOMODE_OFF:
		set_studio_mode(false);
		break;
	case MMGAction::StudioMode::STUDIOMODE_TOGGLE_ONOFF:
		set_studio_mode(!obs_frontend_preview_program_mode_active());
		break;
	case MMGAction::StudioMode::STUDIOMODE_CHANGEPREVIEW:
		if (!source_obs_scene)
			break;
		obs_frontend_set_current_preview_scene(source_obs_scene);
		break;
	case MMGAction::StudioMode::STUDIOMODE_TRANSITION:
		obs_frontend_preview_program_trigger_transition();
		break;
	default:
		break;
	}
	bfree(scene_names);
}

void MMGAction::do_action_scenes(const MMGAction *params,
				 const MMGMessage *midi)
{
	char **scene_names = obs_frontend_get_scene_names();
	if (params->get_str(0) == "Use Message Value" &&
	    (uint)midi->get_value() >= get_obs_scene_count())
		return;
	OBSSourceAutoRelease source_obs_scene =
		obs_get_source_by_name(params->get_str(0) == "Use Message Value"
					       ? scene_names[midi->get_value()]
					       : params->get_str(0).qtocs());
	switch ((MMGAction::Scenes)params->get_sub()) {
	case MMGAction::Scenes::SCENE_SCENE:
		if (!source_obs_scene)
			break;
		obs_frontend_set_current_scene(source_obs_scene);
		break;
	default:
		break;
	}
	bfree(scene_names);
}

void MMGAction::do_action_video_source(const MMGAction *params,
				       const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_scene_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(1).qtocs());
	if (!obs_source || !obs_scene_source)
		return;
	OBSSceneItemAutoRelease obs_sceneitem = obs_scene_sceneitem_from_source(
		obs_scene_from_source(obs_scene_source), obs_source);
	if (!obs_sceneitem)
		return;
	vec2 coordinates;
	obs_sceneitem_crop crop;

	switch ((MMGAction::VideoSources)params->get_sub()) {
	case MMGAction::VideoSources::SOURCE_VIDEO_POSITION:
		vec2_set(&coordinates,
			 num_or_value(params->get_num(0), midi->get_value(),
				      get_obs_dimensions().first),
			 num_or_value(params->get_num(1), midi->get_value(),
				      get_obs_dimensions().second));
		obs_sceneitem_set_pos(obs_sceneitem, &coordinates);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_DISPLAY:
		if (params->get_str(2) == "Toggle") {
			obs_sceneitem_set_visible(
				obs_sceneitem,
				!obs_sceneitem_visible(obs_sceneitem));
		} else {
			obs_sceneitem_set_visible(
				obs_sceneitem,
				bool_from_str(params->get_str(2)));
		}
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_LOCKED:
		if (params->get_str(2) == "Toggle") {
			obs_sceneitem_set_locked(
				obs_sceneitem,
				!obs_sceneitem_locked(obs_sceneitem));
		} else {
			obs_sceneitem_set_locked(
				obs_sceneitem,
				bool_from_str(params->get_str(2)));
		}
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_CROP:
		crop.top = num_or_value(
			params->get_num(0), midi->get_value(),
			get_obs_source_dimensions(params->get_str(1)).second >>
				1);
		crop.right = num_or_value(
			params->get_num(1), midi->get_value(),
			get_obs_source_dimensions(params->get_str(1)).first >>
				1);
		crop.bottom = num_or_value(
			params->get_num(2), midi->get_value(),
			get_obs_source_dimensions(params->get_str(1)).second >>
				1);
		crop.left = num_or_value(
			params->get_num(3), midi->get_value(),
			get_obs_source_dimensions(params->get_str(1)).first >>
				1);
		obs_sceneitem_set_crop(obs_sceneitem, &crop);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_SCALE:
		// This is incomplete
		vec2_set(&coordinates, params->get_num(0), params->get_num(1));
		obs_sceneitem_set_scale(obs_sceneitem, &coordinates);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_SCALEFILTER:
		if (params->get_num(0) == -1 && midi->get_value() > 5)
			return;
		obs_sceneitem_set_scale_filter(
			obs_sceneitem,
			(obs_scale_type)num_or_value(params->get_num(0),
						     midi->get_value(), 128.0));
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_ROTATION:
		obs_sceneitem_set_rot(obs_sceneitem,
				      num_or_value(params->get_num(0),
						   midi->get_value(), 360.0));
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDINGBOX:
		vec2_set(&coordinates,
			 num_or_value(params->get_num(0), midi->get_value(),
				      get_obs_dimensions().first),
			 num_or_value(params->get_num(1), midi->get_value(),
				      get_obs_dimensions().second));
		obs_sceneitem_set_bounds(obs_sceneitem, &coordinates);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_BLEND_MODE:
		if (params->get_num(0) == -1 && midi->get_value() > 6)
			return;
		obs_sceneitem_set_blending_mode(
			obs_sceneitem,
			(obs_blending_type)num_or_value(
				params->get_num(0), midi->get_value(), 128.0));
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_SCREENSHOT:
		obs_frontend_take_source_screenshot(obs_source);
		break;
	default:
		break;
	}
}

void MMGAction::do_action_audio_source(const MMGAction *params,
				       const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	if (!obs_source)
		return;
	if (!(obs_source_get_output_flags(obs_source) & OBS_SOURCE_AUDIO))
		return;

	switch ((MMGAction::AudioSources)params->get_sub()) {
	case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGETO:
		// Value only has range 0-127, meaning full volume cannot be achieved (as it is divided by 128).
		// Adding one allows the capability of full volume - at the cost of removing mute capability.
		obs_source_set_volume(
			obs_source, num_or_value(params->get_num(0),
						 midi->get_value() + 1, 100.0) /
					    100.0);
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGEBY:
		if (obs_source_get_volume(obs_source) * 100.0 +
			    num_or_value(params->get_num(0),
					 midi->get_value() - 64, 100.0) >=
		    100.0) {
			obs_source_set_volume(obs_source, 1);
		} else if (obs_source_get_volume(obs_source) * 100.0 +
				   num_or_value(params->get_num(0),
						midi->get_value() - 64,
						100.0) <=
			   0.0) {
			obs_source_set_volume(obs_source, 0);
		} else {
			obs_source_set_volume(
				obs_source,
				obs_source_get_volume(obs_source) +
					(num_or_value(params->get_num(0),
						      midi->get_value() - 64,
						      100.0) /
					 100.0));
		}
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_MUTE_ON:
		obs_source_set_muted(obs_source, true);
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_MUTE_OFF:
		obs_source_set_muted(obs_source, false);
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF:
		obs_source_set_muted(obs_source, !obs_source_muted(obs_source));
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_OFFSET:
		// Multiplier is 3200 here to make it so that the sync offset is incremented by 25.
		// Hard limit is set at 3175 ms
		obs_source_set_sync_offset(
			obs_source, num_or_value(params->get_num(0),
						 midi->get_value(), 3200.0) *
					    1000000);
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_MONITOR:
		obs_source_set_monitoring_type(
			obs_source,
			(obs_monitoring_type)(num_or_value(
				params->get_num(0),
				midi->get_value() >= 3 ? 0 : midi->get_value(),
				128.0)));
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_CUSTOM:
		break;
	default:
		break;
	}
}

void MMGAction::do_action_media_source(const MMGAction *params,
				       const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	if (!obs_source)
		return;
	if (!(obs_source_get_output_flags(obs_source) &
	      OBS_SOURCE_CONTROLLABLE_MEDIA))
		return;
	obs_media_state state = obs_source_media_get_state(obs_source);
	auto new_time =
		(int64_t)(num_or_value(
				  params->get_num(0), midi->get_value(),
				  get_obs_media_length(params->get_str(0))) *
			  1000);
	switch ((MMGAction::MediaSources)params->get_sub()) {
	case MMGAction::MediaSources::SOURCE_MEDIA_TOGGLE_PLAYPAUSE:
		switch (state) {
		case OBS_MEDIA_STATE_PLAYING:
			obs_source_media_play_pause(obs_source, true);
			break;
		case OBS_MEDIA_STATE_PAUSED:
			obs_source_media_play_pause(obs_source, false);
			break;
		case OBS_MEDIA_STATE_STOPPED:
		case OBS_MEDIA_STATE_ENDED:
			obs_source_media_restart(obs_source);
			break;
		default:
			break;
		}
		break;
	case MMGAction::MediaSources::SOURCE_MEDIA_RESTART:
		obs_source_media_restart(obs_source);
		break;
	case MMGAction::MediaSources::SOURCE_MEDIA_STOP:
		obs_source_media_stop(obs_source);
		break;
	case MMGAction::MediaSources::SOURCE_MEDIA_TIME:
		obs_source_media_set_time(obs_source, new_time);
		break;
	case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_FORWARD_TRACK:
		obs_source_media_next(obs_source);
		break;
	case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TRACK:
		obs_source_media_previous(obs_source);
		break;
	case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_FORWARD_TIME:
		obs_source_media_set_time(
			obs_source,
			obs_source_media_get_time(obs_source) + new_time);
		break;
	case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TIME:
		obs_source_media_set_time(
			obs_source,
			obs_source_media_get_time(obs_source) - new_time);
		break;
	default:
		break;
	}
}

void MMGAction::do_action_transitions(const MMGAction *params,
				      const MMGMessage *midi)
{
	Q_UNUSED(midi);
	OBSSourceAutoRelease obs_transition =
		obs_get_transition_by_name(params->get_str(0).qtocs());
	if (!obs_transition)
		obs_transition = obs_frontend_get_current_transition();
	int obs_transition_time =
		params->get_num(0) == 0 ? obs_frontend_get_transition_duration()
					: params->get_num(0);
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(2).qtocs());
	OBSSceneAutoRelease obs_scene =
		obs_get_scene_by_name(params->get_str(1).qtocs());
	OBSSceneItemAutoRelease obs_sceneitem =
		obs_scene_sceneitem_from_source(obs_scene, obs_source);
	switch ((MMGAction::Transitions)params->get_sub()) {
	case MMGAction::Transitions::TRANSITION_CURRENT:
		obs_frontend_set_current_transition(obs_transition);
		obs_frontend_set_transition_duration(obs_transition_time);
		break;
	/*case MMGAction::Transitions::TRANSITION_TBAR:
		if (!obs_frontend_preview_program_mode_active())
			break;
		obs_frontend_set_tbar_position((int)(num_or_value(time, midi->get_value(), 100.0) * 10.24));
		obs_frontend_release_tbar();
		break;*/
	case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		if (!obs_sceneitem)
			break;
		obs_sceneitem_set_transition(obs_sceneitem, true,
					     obs_transition);
		obs_sceneitem_set_transition_duration(obs_sceneitem, true,
						      obs_transition_time);
		break;
	case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
		if (!obs_sceneitem)
			break;
		obs_sceneitem_set_transition(obs_sceneitem, false,
					     obs_transition);
		obs_sceneitem_set_transition_duration(obs_sceneitem, false,
						      obs_transition_time);
		break;
	default:
		break;
	}
}

void MMGAction::do_action_filters(const MMGAction *params,
				  const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	OBSSourceAutoRelease obs_filter = obs_source_get_filter_by_name(
		obs_source, params->get_str(1).qtocs());
	if (!obs_filter)
		return;
	switch ((MMGAction::Filters)params->get_sub()) {
	case MMGAction::Filters::FILTER_SHOW:
		obs_source_set_enabled(obs_filter, true);
		break;
	case MMGAction::Filters::FILTER_HIDE:
		obs_source_set_enabled(obs_filter, false);
		break;
	case MMGAction::Filters::FILTER_TOGGLE_SHOWHIDE:
		obs_source_set_enabled(obs_filter,
				       !obs_source_enabled(obs_filter));
		break;
	case MMGAction::Filters::FILTER_REORDER:
		if (num_or_value(params->get_num(0) - 1, midi->get_value(),
				 128.0) >= obs_source_filter_count(obs_source))
			break;
		obs_source_filter_set_order(obs_source, obs_filter,
					    OBS_ORDER_MOVE_TOP);
		for (int i = 0; i < num_or_value(params->get_num(0) - 1,
						 midi->get_value(), 128.0);
		     ++i) {
			obs_source_filter_set_order(obs_source, obs_filter,
						    OBS_ORDER_MOVE_DOWN);
		}
		break;
	case MMGAction::Filters::FILTER_CUSTOM:
		break;
	default:
		break;
	}
}

void MMGAction::do_action_hotkeys(const MMGAction *params,
				  const MMGMessage *midi)
{
	Q_UNUSED(midi);
	struct HotkeyRequestBody {
		QString hotkey_name;
		obs_hotkey_id hotkey_id;
	};
	HotkeyRequestBody req;
	req.hotkey_name = params->get_str(0);
	req.hotkey_id = -1;

	obs_enum_hotkeys(
		[](void *data, obs_hotkey_id id, obs_hotkey_t *hotkey) {
			auto *req = reinterpret_cast<HotkeyRequestBody *>(data);

			const char *current_hotkey_name =
				obs_hotkey_get_description(hotkey);
			if (current_hotkey_name == req->hotkey_name) {
				req->hotkey_id = id;
				return false;
			}

			return true;
		},
		&req);

	if (req.hotkey_id == (size_t)(-1))
		return;

	switch ((MMGAction::Hotkeys)params->get_sub()) {
	case MMGAction::Hotkeys::HOTKEY_HOTKEY:
		obs_hotkey_trigger_routed_callback(req.hotkey_id, true);
		break;
	default:
		break;
	}
}

void MMGAction::do_action_profiles(const MMGAction *params,
				   const MMGMessage *midi)
{
	Q_UNUSED(params);
	Q_UNUSED(midi);
}

void MMGAction::do_action_collections(const MMGAction *params,
				      const MMGMessage *midi)
{
	Q_UNUSED(params);
	Q_UNUSED(midi);
}

void MMGAction::do_action_ui(const MMGAction *params, const MMGMessage *midi)
{
	Q_UNUSED(params);
	Q_UNUSED(midi);
}

void MMGAction::do_action_midi(const MMGAction *params, const MMGMessage *midi)
{
	libremidi::message message;
	int channel = params->get_num(0) < 1 ? 1 : params->get_num(0);
	if (params->get_str(1) == "Note On") {
		message = libremidi::message::note_on(
			channel,
			num_or_value(params->get_num(1), midi->get_note(),
				     128.0),
			num_or_value(params->get_num(2), midi->get_value(),
				     128.0));
	} else if (params->get_str(1) == "Note Off") {
		message = libremidi::message::note_off(
			channel,
			num_or_value(params->get_num(1), midi->get_note(),
				     128.0),
			num_or_value(params->get_num(2), midi->get_value(),
				     128.0));
	} else if (params->get_str(1) == "Control Change") {
		message = libremidi::message::control_change(
			channel,
			num_or_value(params->get_num(1), midi->get_note(),
				     128.0),
			num_or_value(params->get_num(2), midi->get_value(),
				     128.0));
	} else if (params->get_str(1) == "Program Change") {
		message = libremidi::message::program_change(
			channel, num_or_value(params->get_num(1),
					      midi->get_value(), 128.0));
	} else if (params->get_str(1) == "Pitch Bend") {
		int num1 = num_or_value(params->get_num(1), midi->get_value(),
					128.0);
		message = libremidi::message::pitch_bend(
			channel, num1 >= 64 ? ((num1 - 64) << 1) : 0, num1);
	} else {
		// Message type is invalid
		return;
	}
	if (params->get_sub() == 0) {
		MMGDevice *const output =
			global()->find_device(params->get_str(0));
		if (!output)
			return;
		if (!output->output_port_open())
			output->open_output_port();
		output->output_send(message);
	}
}

void MMGAction::do_action_pause(const MMGAction *params, const MMGMessage *midi)
{
	switch ((MMGAction::Timeout)params->get_sub()) {
	case MMGAction::Timeout::TIMEOUT_MS:
		std::this_thread::sleep_for(std::chrono::milliseconds(
			(int)num_or_value(params->get_num(0), midi->get_value(),
					  128.0)));
		break;
	case MMGAction::Timeout::TIMEOUT_S:
		std::this_thread::sleep_for(std::chrono::seconds(
			(int)num_or_value(params->get_num(0), midi->get_value(),
					  128.0)));
		break;
	default:
		break;
	}
}
