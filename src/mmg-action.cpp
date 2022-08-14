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

MMGAction::MMGAction(const QJsonObject &obj)
{
	name = obj["name"].toString(
		next_default_name(MMGModes::MMGMODE_ACTION));
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
	} else if (str == "Switching Scenes") {
		return MMGAction::Category::MMGACTION_SCENE;
	} else if (str == "Source Transform") {
		return MMGAction::Category::MMGACTION_SOURCE_TRANS;
	} else if (str == "Source Properties") {
		return MMGAction::Category::MMGACTION_SOURCE_PROPS;
	} else if (str == "Media") {
		return MMGAction::Category::MMGACTION_MEDIA;
	} else if (str == "Transitions") {
		return MMGAction::Category::MMGACTION_TRANSITION;
	} else if (str == "Filters") {
		return MMGAction::Category::MMGACTION_FILTER;
	} else if (str == "Hotkeys") {
		return MMGAction::Category::MMGACTION_HOTKEY;
	} else if (str == "Send MIDI Message") {
		return MMGAction::Category::MMGACTION_MIDIMESSAGE;
	} else if (str == "Wait") {
		return MMGAction::Category::MMGACTION_WAIT;
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
			case MMGAction::Category::MMGACTION_SOURCE_PROPS:
				if (obs_source_get_type(source) !=
				    OBS_SOURCE_TYPE_INPUT)
					return true;
				switch (r->r_v.toInt()) {
				case 5:
					if ((obs_source_get_output_flags(
						     source) &
					     OBS_SOURCE_VIDEO) == 0)
						return true;
					break;
				default:
					if ((obs_source_get_output_flags(
						     source) &
					     OBS_SOURCE_AUDIO) == 0)
						return true;
					break;
				}
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
			if ((obs_source_get_output_flags(source) &
			     OBS_SOURCE_CONTROLLABLE_MEDIA) == 0)
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
	if (restriction_t == MMGAction::Category::MMGACTION_SOURCE_PROPS) {
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

void MMGAction::do_action(const MMGMessage *const parameter)
{
	switch (get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
		do_action_none((MMGAction::None)subcategory);
		break;
	case MMGAction::Category::MMGACTION_STREAM:
		do_action_stream((MMGAction::Stream)subcategory);
		break;
	case MMGAction::Category::MMGACTION_RECORD:
		do_action_record((MMGAction::Record)subcategory);
		break;
	case MMGAction::Category::MMGACTION_VIRCAM:
		do_action_virtual_cam((MMGAction::VirtualCam)subcategory);
		break;
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		do_action_studio_mode((MMGAction::StudioMode)subcategory,
				      get_str(0), parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		do_action_scenes((MMGAction::Scenes)subcategory, get_str(0),
				 parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_TRANS:
		do_action_source_transform(
			(MMGAction::SourceTransform)subcategory, strs, nums,
			parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_PROPS:
		do_action_source_properties(
			(MMGAction::SourceProperties)subcategory, get_str(0),
			get_num(0), parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_MEDIA:
		do_action_media((MMGAction::Media)subcategory, get_str(0),
				get_num(0), parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		do_action_transitions((MMGAction::Transitions)subcategory, strs,
				      get_num(0), parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		do_action_filters((MMGAction::Filters)subcategory, strs,
				  (int)get_num(0), parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_HOTKEY:
		do_action_hotkeys((MMGAction::Hotkeys)subcategory, get_str(0),
				  parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_MIDIMESSAGE:
		do_action_send_midi((MMGAction::SendMidi)subcategory, strs,
				    nums, parameter->get_value());
		break;
	case MMGAction::Category::MMGACTION_WAIT:
		do_action_sleep((MMGAction::Sleep)subcategory,
				(ulong)get_num(0));
		break;
	default:
		break;
	}
}

void MMGAction::do_action_none(None kind)
{
	Q_UNUSED(kind);
}

void MMGAction::do_action_stream(Stream kind)
{
	switch (kind) {
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

void MMGAction::do_action_record(Record kind)
{
	switch (kind) {
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

void MMGAction::do_action_virtual_cam(VirtualCam kind)
{
	switch (kind) {
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

void MMGAction::do_action_studio_mode(StudioMode kind, const QString &scene,
				      uint value)
{
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
	OBSSceneAutoRelease obs_scene = obs_get_scene_by_name(
		scene == "Use Message Value" ? scene_names[value]
					     : scene.qtocs());
	OBSSourceAutoRelease source_obs_scene = obs_scene_get_source(obs_scene);
	switch (kind) {
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

void MMGAction::do_action_scenes(Scenes kind, const QString &scene, uint value)
{
	char **scene_names = obs_frontend_get_scene_names();
	if (scene == "Use Message Value" && value >= get_obs_scene_count())
		return;
	OBSSourceAutoRelease source_obs_scene = obs_get_source_by_name(
		scene == "Use Message Value" ? scene_names[value]
					     : scene.qtocs());
	switch (kind) {
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

void MMGAction::do_action_source_transform(SourceTransform kind,
					   const QString strs[4],
					   const double nums[4], uint value)
{
	OBSSourceAutoRelease obs_scene_source =
		obs_get_source_by_name(strs[0].qtocs());
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(strs[1].qtocs());
	if (!obs_source || !obs_scene_source)
		return;
	OBSSceneItemAutoRelease obs_sceneitem = obs_scene_sceneitem_from_source(
		obs_scene_from_source(obs_scene_source), obs_source);
	if (!obs_sceneitem)
		return;
	vec2 coordinates;
	obs_sceneitem_crop crop;
	switch (kind) {
	case MMGAction::SourceTransform::TRANSFORM_POSITION:
		vec2_set(&coordinates, nums[0], nums[1]);
		obs_sceneitem_set_pos(obs_sceneitem, &coordinates);
		break;
	case MMGAction::SourceTransform::TRANSFORM_DISPLAY:
		if (strs[2] == "Toggle") {
			obs_sceneitem_set_visible(
				obs_sceneitem,
				!obs_sceneitem_visible(obs_sceneitem));
		} else {
			obs_sceneitem_set_visible(obs_sceneitem,
						  bool_from_str(strs[2]));
		}
		break;
	case MMGAction::SourceTransform::TRANSFORM_LOCKED:
		if (strs[2] == "Toggle") {
			obs_sceneitem_set_locked(
				obs_sceneitem,
				!obs_sceneitem_locked(obs_sceneitem));
		} else {
			obs_sceneitem_set_locked(obs_sceneitem,
						 bool_from_str(strs[2]));
		}
		break;
	case MMGAction::SourceTransform::TRANSFORM_CROP:
		crop.top = num_or_value(
			nums[0], value,
			get_obs_source_dimensions(strs[1]).second >> 1);
		crop.right = num_or_value(
			nums[1], value,
			get_obs_source_dimensions(strs[1]).first >> 1);
		crop.bottom = num_or_value(
			nums[2], value,
			get_obs_source_dimensions(strs[1]).second >> 1);
		crop.left = num_or_value(
			nums[3], value,
			get_obs_source_dimensions(strs[1]).first >> 1);
		obs_sceneitem_set_crop(obs_sceneitem, &crop);
		break;
	case MMGAction::SourceTransform::TRANSFORM_SCALE:
		vec2_set(&coordinates, nums[0], nums[1]);
		obs_sceneitem_set_scale(obs_sceneitem, &coordinates);
		break;
	case MMGAction::SourceTransform::TRANSFORM_ROTATION:
		obs_sceneitem_set_rot(obs_sceneitem,
				      num_or_value(nums[0], value, 360.0));
		break;
	case MMGAction::SourceTransform::TRANSFORM_BOUNDINGBOX:
		vec2_set(&coordinates, nums[0], nums[1]);
		obs_sceneitem_set_bounds(obs_sceneitem, &coordinates);
		break;
	case MMGAction::SourceTransform::TRANSFORM_RESET:
		break;
	default:
		break;
	}
}

void MMGAction::do_action_source_properties(SourceProperties kind,
					    const QString &source, double num,
					    uint value)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(source.qtocs());
	if (!obs_source)
		return;
	uint32_t flags = obs_source_get_output_flags(obs_source);
	bool flags_contain_audio = (flags & OBS_SOURCE_AUDIO) >> 1;
	bool flags_contain_video = (flags & OBS_SOURCE_VIDEO) >> 0;

	switch (kind) {
	case MMGAction::SourceProperties::PROPERTY_VOLUME_CHANGETO:
		if (!flags_contain_audio)
			break;
		// Value only has range 0-127, meaning full volume cannot be achieved (as it is divided by 128).
		// Adding one allows the capability of full volume - at the cost of removing mute capability.
		obs_source_set_volume(obs_source,
				      num_or_value(num, value + 1, 100.0) /
					      100.0);
		break;
	case MMGAction::SourceProperties::PROPERTY_VOLUME_CHANGEBY:
		if (!flags_contain_audio)
			break;
		if (obs_source_get_volume(obs_source) * 100.0 +
			    num_or_value(num, value - 64, 100.0) >=
		    100.0) {
			obs_source_set_volume(obs_source, 1);
		} else if (obs_source_get_volume(obs_source) * 100.0 +
				   num_or_value(num, value - 64, 100.0) <=
			   0.0) {
			obs_source_set_volume(obs_source, 0);
		} else {
			obs_source_set_volume(
				obs_source,
				obs_source_get_volume(obs_source) +
					(num_or_value(num, value - 64, 100.0) /
					 100.0));
		}
		break;
	case MMGAction::SourceProperties::PROPERTY_VOLUME_MUTE_ON:
		if (!flags_contain_audio)
			break;
		obs_source_set_muted(obs_source, true);
		break;
	case MMGAction::SourceProperties::PROPERTY_VOLUME_MUTE_OFF:
		if (!flags_contain_audio)
			break;
		obs_source_set_muted(obs_source, false);
		break;
	case MMGAction::SourceProperties::PROPERTY_VOLUME_MUTE_TOGGLE_ONOFF:
		if (!flags_contain_audio)
			break;
		obs_source_set_muted(obs_source, !obs_source_muted(obs_source));
		break;
	case MMGAction::SourceProperties::PROPERTY_SCREENSHOT:
		if (!flags_contain_video)
			break;
		obs_frontend_take_source_screenshot(obs_source);
		break;
	case MMGAction::SourceProperties::PROPERTY_AUDIO_OFFSET:
		if (!flags_contain_audio)
			break;
		obs_source_set_sync_offset(obs_source,
					   num_or_value(num, value, 20000.0) *
						   1000000);
		break;
	case MMGAction::SourceProperties::PROPERTY_AUDIO_MONITOR:
		if (!flags_contain_audio)
			break;
		obs_source_set_monitoring_type(
			obs_source,
			(obs_monitoring_type)(num_or_value(
				num, value >= 3 ? 0 : value, 128.0)));
		break;
	case MMGAction::SourceProperties::PROPERTY_CUSTOM:
		break;
	default:
		break;
	}
}

void MMGAction::do_action_media(Media kind, const QString &source, double time,
				uint value)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(source.qtocs());
	if (!obs_source)
		return;
	if ((obs_source_get_output_flags(obs_source) &
	     OBS_SOURCE_CONTROLLABLE_MEDIA) == 0)
		return;
	obs_media_state state = obs_source_media_get_state(obs_source);
	auto new_time = (int64_t)(num_or_value(time, value,
					       get_obs_media_length(source)) *
				  1000);
	switch (kind) {
	case MMGAction::Media::MEDIA_TOGGLE_PLAYPAUSE:
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
	case MMGAction::Media::MEDIA_RESTART:
		obs_source_media_restart(obs_source);
		break;
	case MMGAction::Media::MEDIA_STOP:
		obs_source_media_stop(obs_source);
		break;
	case MMGAction::Media::MEDIA_TIME:
		obs_source_media_set_time(obs_source, new_time);
		break;
	case MMGAction::Media::MEDIA_SKIP_FORWARD_TRACK:
		obs_source_media_next(obs_source);
		break;
	case MMGAction::Media::MEDIA_SKIP_BACKWARD_TRACK:
		obs_source_media_previous(obs_source);
		break;
	case MMGAction::Media::MEDIA_SKIP_FORWARD_TIME:
		obs_source_media_set_time(
			obs_source,
			obs_source_media_get_time(obs_source) + new_time);
		break;
	case MMGAction::Media::MEDIA_SKIP_BACKWARD_TIME:
		obs_source_media_set_time(
			obs_source,
			obs_source_media_get_time(obs_source) - new_time);
		break;
	default:
		break;
	}
}

void MMGAction::do_action_transitions(Transitions kind, const QString names[4],
				      double time, uint value)
{
	Q_UNUSED(value);
	OBSSourceAutoRelease obs_transition =
		obs_get_transition_by_name(names[0].qtocs());
	if (!obs_transition)
		obs_transition = obs_frontend_get_current_transition();
	int obs_transition_time =
		time == 0 ? obs_frontend_get_transition_duration() : time;
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(names[2].qtocs());
	OBSSceneAutoRelease obs_scene = obs_get_scene_by_name(names[1].qtocs());
	OBSSceneItemAutoRelease obs_sceneitem =
		obs_scene_sceneitem_from_source(obs_scene, obs_source);
	switch (kind) {
	case MMGAction::Transitions::TRANSITION_CURRENT:
		obs_frontend_set_current_transition(obs_transition);
		obs_frontend_set_transition_duration(obs_transition_time);
		break;
	/*case MMGAction::Transitions::TRANSITION_TBAR:
		if (!obs_frontend_preview_program_mode_active())
			break;
		obs_frontend_set_tbar_position((int)(num_or_value(time, value, 100.0) * 10.24));
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

void MMGAction::do_action_filters(Filters kind, const QString names[4],
				  int index, uint value)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(names[0].qtocs());
	OBSSourceAutoRelease obs_filter =
		obs_source_get_filter_by_name(obs_source, names[1].qtocs());
	if (!obs_filter)
		return;
	switch (kind) {
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
		if (num_or_value(index - 1, value, 128.0) >=
		    obs_source_filter_count(obs_source))
			break;
		obs_source_filter_set_order(obs_source, obs_filter,
					    OBS_ORDER_MOVE_TOP);
		for (int i = 0; i < num_or_value(index - 1, value, 128.0);
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

void MMGAction::do_action_hotkeys(Hotkeys kind, const QString &name, uint value)
{
	Q_UNUSED(value);
	struct HotkeyRequestBody {
		QString hotkey_name;
		obs_hotkey_id hotkey_id;
	};
	HotkeyRequestBody req;
	req.hotkey_name = name;
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

	if (req.hotkey_id == -1)
		return;

	switch (kind) {
	case MMGAction::Hotkeys::HOTKEY_HOTKEY:
		obs_hotkey_trigger_routed_callback(req.hotkey_id, true);
		break;
	default:
		break;
	}
}

void MMGAction::do_action_send_midi(SendMidi kind, const QString strs[4],
				    const double nums[4], uint value)
{
	libremidi::message message;
	if (strs[1] == "Note On") {
		message = libremidi::message::note_on(
			nums[0], nums[1], num_or_value(nums[2], value, 128.0));
	} else if (strs[1] == "Note Off") {
		message = libremidi::message::note_off(
			nums[0], nums[1], num_or_value(nums[2], value, 128.0));
	} else if (strs[1] == "Control Change") {
		message = libremidi::message::control_change(
			nums[0], nums[1], num_or_value(nums[2], value, 128.0));
	} else if (strs[1] == "Program Change") {
		message = libremidi::message::program_change(
			nums[0], num_or_value(nums[1], value, 128.0));
	} else if (strs[1] == "Pitch Bend") {
		message = libremidi::message::pitch_bend(
			nums[0], nums[1], num_or_value(nums[2], value, 128.0));
	} else {
		// Message type is invalid
		return;
	}
	if (kind == MMGAction::SendMidi::SENDMIDI_SENDMIDI) {
		MMGDevice *const output = global()->find_device(strs[0]);
		if (!output)
			return;
		if (!output->output_port_open())
			output->open_output_port();
		output->output_send(message);
	}
}

void MMGAction::do_action_sleep(Sleep kind, ulong duration, uint value)
{
	switch (kind) {
	case MMGAction::Sleep::WAIT_MS:
		std::this_thread::sleep_for(std::chrono::milliseconds(
			(int)num_or_value(duration, value, 128.0)));
		break;
	case MMGAction::Sleep::WAIT_S:
		std::this_thread::sleep_for(std::chrono::seconds(
			(int)num_or_value(duration, value, 128.0)));
		break;
	default:
		break;
	}
}
