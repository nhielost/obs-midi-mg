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
#include <util/config-file.h>

#include <thread>

using namespace MMGUtils;

double num_or_value(const MMGAction *params, const MMGMessage *midi, int num,
		    double mult = 128.0, double value_oper = 0.0)
{
	return (params->get_num(num) == -1
			? qRound(((midi->get_value() + value_oper) / 128.0) *
				 mult)
			: params->get_num(num));
}

struct R {
	QComboBox *c;
	MMGAction::Category r_t;
	QString r_v;
};

MMGAction::MMGAction()
{
	category = 0;
	subcategory = 0;
	set_str(0, "");
	set_str(1, "");
	set_str(2, "");
	set_num(0, 0.0);
	set_num(1, 0.0);
	set_num(2, 0.0);
	set_num(3, 0.0);
	blog(LOG_DEBUG, "Empty action created.");
}

MMGAction::MMGAction(const QJsonObject &obj)
{
	category = obj["category"].toInt(0);
	subcategory = obj["sub"].toInt(0);
	set_str(0, obj["str1"].toString());
	set_str(1, obj["str2"].toString());
	set_str(2, obj["str3"].toString());
	set_num(0, obj["num1"].toDouble());
	set_num(1, obj["num2"].toDouble());
	set_num(2, obj["num3"].toDouble());
	set_num(3, obj["num4"].toDouble());
	blog(LOG_DEBUG, "Action created.");
}

void MMGAction::json(QJsonObject &action_obj) const
{
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

void MMGAction::blog(int log_status, const QString &message) const
{
	global_blog(log_status, "Actions -> " + message);
}

void MMGAction::deep_copy(MMGAction *dest)
{
	dest->set_category(get_category());
	dest->set_sub(subcategory);
	dest->set_str(0, strs[0]);
	dest->set_str(1, strs[1]);
	dest->set_str(2, strs[2]);
	dest->set_str(3, strs[3]);
	dest->set_num(0, nums[0]);
	dest->set_num(1, nums[1]);
	dest->set_num(2, nums[2]);
	dest->set_num(3, nums[3]);
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
			case MMGAction::Category::MMGACTION_FILTER:
				if (obs_source_get_type(source) !=
					    OBS_SOURCE_TYPE_INPUT &&
				    obs_source_get_type(source) !=
					    OBS_SOURCE_TYPE_SCENE)
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
			QString name = obs_hotkey_get_name(hotkey);
			// From obs-midi
			if (name.contains("libobs") ||
			    name.contains("MediaSource") ||
			    name.contains("OBSBasic"))
				return true;
			helper->addItem(obs_hotkey_get_description(hotkey));
			return true;
		},
		list);
}

void MMGAction::do_obs_profile_enum(QComboBox *list)
{
	char **profile_names = obs_frontend_get_profiles();

	for (int i = 0; profile_names[i] != 0; ++i) {
		list->addItem(profile_names[i]);
	}

	bfree(profile_names);
}

void MMGAction::do_obs_collection_enum(QComboBox *list)
{
	char **collection_names = obs_frontend_get_scene_collections();

	for (int i = 0; collection_names[i] != 0; ++i) {
		list->addItem(collection_names[i]);
	}

	bfree(collection_names);
}

void MMGAction::do_mmg_binding_enum(QComboBox *list,
				    const QString &current_binding,
				    const QString current_select)
{
	for (MMGBinding *const binding :
	     global()->find_current_device()->get_bindings()) {
		if (binding->get_name() != current_binding)
			list->addItem(binding->get_name());
	}
	if (list->findText(current_select) != -1)
		list->setCurrentText(current_select);
}

template<>
void MMGAction::do_action_specific<MMGAction::None>(const MMGAction *params,
						    const MMGMessage *midi)
{
	Q_UNUSED(midi);
	params->blog(LOG_DEBUG, "<None> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Stream>(const MMGAction *params,
						      const MMGMessage *midi)
{
	Q_UNUSED(midi);
	switch ((MMGAction::Stream)params->get_sub()) {
	case MMGAction::Stream::STREAM_ON:
		if (!obs_frontend_streaming_active())
			obs_frontend_streaming_start();
		break;
	case MMGAction::Stream::STREAM_OFF:
		if (obs_frontend_streaming_active())
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
	params->blog(LOG_DEBUG, "<Stream> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Record>(const MMGAction *params,
						      const MMGMessage *midi)
{
	Q_UNUSED(midi);
	switch ((MMGAction::Record)params->get_sub()) {
	case MMGAction::Record::RECORD_ON:
		if (!obs_frontend_recording_active())
			obs_frontend_recording_start();
		break;
	case MMGAction::Record::RECORD_OFF:
		if (obs_frontend_recording_active())
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
		if (!obs_frontend_recording_paused())
			obs_frontend_recording_pause(true);
		break;
	case MMGAction::Record::RECORD_RESUME:
		if (obs_frontend_recording_paused())
			obs_frontend_recording_pause(false);
		break;
	case MMGAction::Record::RECORD_TOGGLE_PAUSE:
		obs_frontend_recording_pause(!obs_frontend_recording_paused());
		break;
	default:
		break;
	}
	params->blog(LOG_DEBUG, "<Record> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::VirtualCam>(
	const MMGAction *params, const MMGMessage *midi)
{
	Q_UNUSED(midi);
	switch ((MMGAction::VirtualCam)params->get_sub()) {
	case MMGAction::VirtualCam::VIRCAM_ON:
		if (!obs_frontend_virtualcam_active())
			obs_frontend_start_virtualcam();
		break;
	case MMGAction::VirtualCam::VIRCAM_OFF:
		if (obs_frontend_virtualcam_active())
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
	params->blog(LOG_DEBUG, "<Virtual Camera> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::ReplayBuffer>(
	const MMGAction *params, const MMGMessage *midi)
{
	config_t *obs_config = obs_frontend_get_profile_config();
	if ((QString(config_get_string(obs_config, "Output", "Mode")) ==
		     "Simple" &&
	     !config_get_bool(obs_config, "SimpleOutput", "RecRB")) ||
	    (QString(config_get_string(obs_config, "Output", "Mode")) ==
		     "Advanced" &&
	     !config_get_bool(obs_config, "AdvOut", "RecRB"))) {
		params->blog(
			LOG_INFO,
			"<Replay Buffer> action failed - Replay Buffers are not enabled.");
		return;
	}

	Q_UNUSED(midi);
	switch ((MMGAction::ReplayBuffer)params->get_sub()) {
	case MMGAction::ReplayBuffer::REPBUF_ON:
		if (!obs_frontend_replay_buffer_active())
			obs_frontend_replay_buffer_start();
		break;
	case MMGAction::ReplayBuffer::REPBUF_OFF:
		if (obs_frontend_replay_buffer_active())
			obs_frontend_replay_buffer_stop();
		break;
	case MMGAction::ReplayBuffer::REPBUF_TOGGLE_ONOFF:
		if (obs_frontend_replay_buffer_active()) {
			obs_frontend_replay_buffer_stop();
		} else {
			obs_frontend_replay_buffer_start();
		}
		break;
	case MMGAction::ReplayBuffer::REPBUF_SAVE:
		if (obs_frontend_replay_buffer_active())
			obs_frontend_replay_buffer_save();
		break;
	default:
		break;
	}
	params->blog(LOG_DEBUG, "<Replay Buffer> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::StudioMode>(
	const MMGAction *params, const MMGMessage *midi)
{
	if (params->get_str(0) == "Use Message Value" &&
	    (uint)midi->get_value() >= get_obs_scene_count()) {
		params->blog(
			LOG_INFO,
			"<Studio Mode> action failed - MIDI value exceeded scene count.");
		return;
	}
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
	params->blog(LOG_DEBUG, "<Studio Mode> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Scenes>(const MMGAction *params,
						      const MMGMessage *midi)
{
	if (params->get_str(0) == "Use Message Value" &&
	    (uint)midi->get_value() >= get_obs_scene_count()) {
		params->blog(
			LOG_INFO,
			"<Scene Switching> action failed - MIDI value exceeded scene count.");
		return;
	}
	char **scene_names = obs_frontend_get_scene_names();
	OBSSourceAutoRelease source_obs_scene =
		obs_get_source_by_name(params->get_str(0) == "Use Message Value"
					       ? scene_names[midi->get_value()]
					       : params->get_str(0).qtocs());
	switch ((MMGAction::Scenes)params->get_sub()) {
	case MMGAction::Scenes::SCENE_SCENE:
		if (!source_obs_scene) {
			params->blog(
				LOG_INFO,
				"<Scene Switching> action failed - Scene does not exist.");
			bfree(scene_names);
			return;
		}
		obs_frontend_set_current_scene(source_obs_scene);
		break;
	default:
		break;
	}
	bfree(scene_names);
	params->blog(LOG_DEBUG, "<Scene Switching> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::VideoSources>(
	const MMGAction *params, const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_scene_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(1).qtocs());
	if (!obs_source || !obs_scene_source) {
		params->blog(
			LOG_INFO,
			"<Video Sources> action failed - Scene or source does not exist.");
		return;
	}
	OBSSceneItemAutoRelease obs_sceneitem = obs_scene_sceneitem_from_source(
		obs_scene_from_source(obs_scene_source), obs_source);
	if (!obs_sceneitem) {
		params->blog(
			LOG_INFO,
			"<Video Sources> action failed - Source does not exist in scene.");
		return;
	}
	vec2 coordinates;
	obs_sceneitem_crop crop;
	uint32_t align = 0;
	uint32_t length = 0;

	switch ((MMGAction::VideoSources)params->get_sub()) {
	case MMGAction::VideoSources::SOURCE_VIDEO_POSITION:
		vec2_set(&coordinates,
			 num_or_value(params, midi, 0,
				      get_obs_dimensions().first),
			 num_or_value(params, midi, 1,
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
		// align and length used for length and width of sources
		align = get_obs_source_dimensions(params->get_str(1)).first;
		length = get_obs_source_dimensions(params->get_str(1)).second;
		crop.top = num_or_value(params, midi, 0, length);
		crop.right = num_or_value(params, midi, 1, align);
		crop.bottom = num_or_value(params, midi, 2, length);
		crop.left = num_or_value(params, midi, 3, align);
		obs_sceneitem_set_crop(obs_sceneitem, &crop);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_ALIGNMENT:
		if (params->get_num(0) == -1 && midi->get_value() > 8) {
			params->blog(
				LOG_INFO,
				"<Video Sources> action failed - MIDI value exceeded choice options.");
			return;
		}
		if (num_or_value(params, midi, 0) <= 2)
			align |= OBS_ALIGN_TOP;
		if (num_or_value(params, midi, 0) >= 6)
			align |= OBS_ALIGN_BOTTOM;
		if ((uint)num_or_value(params, midi, 0) % 3 == 0)
			align |= OBS_ALIGN_LEFT;
		if ((uint)num_or_value(params, midi, 0) % 3 == 2)
			align |= OBS_ALIGN_RIGHT;
		obs_sceneitem_set_alignment(obs_sceneitem, align);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_SCALE:
		// align variable is used for multiplier, length is used for proportionality
		align = params->get_num(2) == -1 ? 1 : params->get_num(2);
		length = midi->get_value() == -1 ? 127 : 100;
		vec2_set(&coordinates,
			 num_or_value(params, midi, 0) / length * align,
			 num_or_value(params, midi, 1) / length * align);
		obs_sceneitem_set_scale(obs_sceneitem, &coordinates);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_SCALEFILTER:
		if (params->get_num(0) == -1 && midi->get_value() > 5) {
			params->blog(
				LOG_INFO,
				"<Video Sources> action failed - MIDI value exceeded choice options.");
			return;
		}
		obs_sceneitem_set_scale_filter(
			obs_sceneitem,
			(obs_scale_type)num_or_value(params, midi, 0));
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_ROTATION:
		obs_sceneitem_set_rot(obs_sceneitem,
				      num_or_value(params, midi, 0, 360.0));
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_TYPE:
		if (params->get_num(0) == -1 && midi->get_value() > 6) {
			params->blog(
				LOG_INFO,
				"<Video Sources> action failed - MIDI value exceeded choice options.");
			return;
		}
		obs_sceneitem_set_bounds_type(
			obs_sceneitem,
			(obs_bounds_type)num_or_value(params, midi, 0));
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_SIZE:
		vec2_set(&coordinates,
			 num_or_value(params, midi, 0,
				      get_obs_dimensions().first),
			 num_or_value(params, midi, 1,
				      get_obs_dimensions().second));
		obs_sceneitem_set_bounds(obs_sceneitem, &coordinates);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_ALIGN:
		if (params->get_num(0) == -1 && midi->get_value() > 8) {
			params->blog(
				LOG_INFO,
				"<Video Sources> action failed - MIDI value exceeded choice options.");
			return;
		}
		if (num_or_value(params, midi, 0) <= 2)
			align |= OBS_ALIGN_TOP;
		if (num_or_value(params, midi, 0) >= 6)
			align |= OBS_ALIGN_BOTTOM;
		if ((uint)num_or_value(params, midi, 0) % 3 == 0)
			align |= OBS_ALIGN_LEFT;
		if ((uint)num_or_value(params, midi, 0) % 3 == 2)
			align |= OBS_ALIGN_RIGHT;
		obs_sceneitem_set_bounds_alignment(obs_sceneitem, align);
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_BLEND_MODE:
		if (params->get_num(0) == -1 && midi->get_value() > 6) {
			params->blog(
				LOG_INFO,
				"<Video Sources> action failed - MIDI value exceeded choice options.");
			return;
		}
		obs_sceneitem_set_blending_mode(
			obs_sceneitem,
			(obs_blending_type)num_or_value(params, midi, 0));
		break;
	case MMGAction::VideoSources::SOURCE_VIDEO_SCREENSHOT:
		obs_frontend_take_source_screenshot(obs_source);
		break;
	default:
		break;
	}
	params->blog(LOG_DEBUG, "<Video Sources> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::AudioSources>(
	const MMGAction *params, const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	if (!obs_source) {
		params->blog(
			LOG_INFO,
			"<Audio Sources> action failed - Source does not exist.");
		return;
	}
	if (!(obs_source_get_output_flags(obs_source) & OBS_SOURCE_AUDIO)) {
		params->blog(
			LOG_INFO,
			"<Audio Sources> action failed - Source is not an audio source.");
		return;
	}

	switch ((MMGAction::AudioSources)params->get_sub()) {
	case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGETO:
		// Value only has range 0-127, meaning full volume cannot be achieved (as it is divided by 128).
		// Adding one allows the capability of full volume.
		// ADDITION: Removing 1% from the value and changing it to 0% for muting is not as noticable.
		if (params->get_num(0) == -1 && midi->get_value() == 0) {
			obs_source_set_volume(obs_source, 0);
		} else {
			obs_source_set_volume(
				obs_source,
				std::pow((num_or_value(params, midi, 0, 100.0,
						       1) /
					  100.0),
					 3.0));
		}
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGEBY:
		if (std::cbrt(obs_source_get_volume(obs_source)) * 100.0 +
			    num_or_value(params, midi, 0, 100, -64) >=
		    100.0) {
			obs_source_set_volume(obs_source, 1);
		} else if (std::cbrt(obs_source_get_volume(obs_source)) *
					   100.0 +
				   num_or_value(params, midi, 0, 100.0, -64) <=
			   0.0) {
			obs_source_set_volume(obs_source, 0);
		} else {
			obs_source_set_volume(
				obs_source,
				std::pow(std::cbrt(obs_source_get_volume(
						 obs_source)) +
						 (num_or_value(params, midi, 0,
							       100.0, -64) /
						  100.0),
					 3.0));
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
			obs_source,
			(num_or_value(params, midi, 0, 3200.0) * 1000000));
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_MONITOR:
		if (params->get_num(0) && midi->get_value() >= 3) {
			params->blog(
				LOG_INFO,
				"<Audio Sources> action failed - MIDI value exceeds audio monitor option count.");
			return;
		}
		obs_source_set_monitoring_type(
			obs_source,
			(obs_monitoring_type)(num_or_value(params, midi, 0)));
		break;
	case MMGAction::AudioSources::SOURCE_AUDIO_CUSTOM:
		break;
	default:
		break;
	}
	params->blog(LOG_DEBUG, "<Audio Sources> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::MediaSources>(
	const MMGAction *params, const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	if (!obs_source) {
		params->blog(
			LOG_INFO,
			"<Media Sources> action failed - Source does not exist.");
		return;
	}
	if (!(obs_source_get_output_flags(obs_source) &
	      OBS_SOURCE_CONTROLLABLE_MEDIA)) {
		params->blog(
			LOG_INFO,
			"<Media Sources> action failed - Source is not a media source.");
		return;
	}
	obs_media_state state = obs_source_media_get_state(obs_source);
	auto new_time = (int64_t)(num_or_value(params, midi, 0,
					       get_obs_media_length(
						       params->get_str(0))) *
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
	params->blog(LOG_DEBUG, "<Media Sources> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Transitions>(
	const MMGAction *params, const MMGMessage *midi)
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
		obs_frontend_set_tbar_position((int)(num_or_value(time, midi, 100.0) * 10.24));
		obs_frontend_release_tbar();
		break;*/
	case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:
		if (!obs_sceneitem) {
			params->blog(
				LOG_INFO,
				"<Transitions> action failed - Source in scene does not exist.");
			return;
		}
		obs_sceneitem_set_transition(obs_sceneitem, true,
					     obs_transition);
		obs_sceneitem_set_transition_duration(obs_sceneitem, true,
						      obs_transition_time);
		break;
	case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:
		if (!obs_sceneitem) {
			params->blog(
				LOG_INFO,
				"<Transitions> action failed - Source in scene does not exist.");
			return;
		}
		obs_sceneitem_set_transition(obs_sceneitem, false,
					     obs_transition);
		obs_sceneitem_set_transition_duration(obs_sceneitem, false,
						      obs_transition_time);
		break;
	default:
		break;
	}
	params->blog(LOG_DEBUG, "<Transitions> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Filters>(const MMGAction *params,
						       const MMGMessage *midi)
{
	OBSSourceAutoRelease obs_source =
		obs_get_source_by_name(params->get_str(0).qtocs());
	OBSSourceAutoRelease obs_filter = obs_source_get_filter_by_name(
		obs_source, params->get_str(1).qtocs());
	if (!obs_filter) {
		params->blog(
			LOG_INFO,
			"<Filters> action failed - Filter in source does not exist.");
		return;
	}
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
		if (num_or_value(params, midi, 0, 128.0, 1) - 1 >=
		    obs_source_filter_count(obs_source)) {
			params->blog(
				LOG_INFO,
				"<Filters> action failed - MIDI value exceeds filter count in source.");
			return;
		}
		obs_source_filter_set_order(obs_source, obs_filter,
					    OBS_ORDER_MOVE_TOP);
		for (int i = 0; i < num_or_value(params, midi, 0, 128.0, 1) - 1;
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
	params->blog(LOG_DEBUG, "<Filters> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Hotkeys>(const MMGAction *params,
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
			if (req->hotkey_name.contains(current_hotkey_name)) {
				req->hotkey_id = id;
				return false;
			}

			return true;
		},
		&req);

	if (req.hotkey_id == (size_t)(-1)) {
		params->blog(
			LOG_INFO,
			"<Hotkeys> action failed - Hotkey does not exist.");
		return;
	}

	switch ((MMGAction::Hotkeys)params->get_sub()) {
	case MMGAction::Hotkeys::HOTKEY_HOTKEY:
		obs_hotkey_trigger_routed_callback(req.hotkey_id, true);
		break;
	default:
		break;
	}
	params->blog(LOG_DEBUG, "<Hotkeys> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Profiles>(const MMGAction *params,
							const MMGMessage *midi)
{
	char **profile_names = obs_frontend_get_profiles();

	if (params->get_str(0) == "Use Message Value" &&
	    (uint)midi->get_value() >= get_obs_profile_count()) {
		params->blog(
			LOG_INFO,
			"<Profiles> action failed - MIDI value exceeds profile count.");
		return;
	}

	// For new Profile change (pre 28.0.0 method encounters threading errors)
	auto set_profile = [](const char *name) {
		char *current_profile = obs_frontend_get_current_profile();
		if (name == current_profile) {
			bfree(current_profile);
			return;
		}
		bfree(current_profile);
		obs_queue_task(
			OBS_TASK_UI,
			[](void *param) {
				auto profile_name = (char **)param;
				obs_frontend_set_current_profile(*profile_name);
			},
			&name, true);
	};

	if (params->get_sub() == 0) {
		if (!(obs_frontend_streaming_active() ||
		      obs_frontend_recording_active() ||
		      obs_frontend_virtualcam_active())) {
			set_profile(params->get_str(0) == "Use Message Value"
					    ? profile_names[midi->get_value()]
					    : params->get_str(0).qtocs());
		}
	}

	bfree(profile_names);
	params->blog(LOG_DEBUG, "<Profiles> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Collections>(
	const MMGAction *params, const MMGMessage *midi)
{
	char **collection_names = obs_frontend_get_scene_collections();

	if (params->get_str(0) == "Use Message Value" &&
	    (uint)midi->get_value() >= get_obs_collection_count()) {
		params->blog(
			LOG_INFO,
			"<Scene Collections> action failed - MIDI value exceeds scene collection count.");
		return;
	}

	// For new Scene Collection change (pre 28.0.0 method encounters threading errors)
	auto set_collection = [](const char *name) {
		char *current_collection =
			obs_frontend_get_current_scene_collection();
		if (name == current_collection) {
			bfree(current_collection);
			return;
		}
		bfree(current_collection);
		obs_queue_task(
			OBS_TASK_UI,
			[](void *param) {
				auto collection_name = (char **)param;
				obs_frontend_set_current_scene_collection(
					*collection_name);
			},
			&name, true);
	};

	if (params->get_sub() == 0) {
		set_collection(params->get_str(0) == "Use Message Value"
				       ? collection_names[midi->get_value()]
				       : params->get_str(0).qtocs());
	}

	bfree(collection_names);
	params->blog(LOG_DEBUG, "<Scene Collections> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::MidiMessage>(
	const MMGAction *params, const MMGMessage *midi)
{
	libremidi::message message;
	int channel = params->get_num(0) < 1 ? 1 : params->get_num(0);
	if (params->get_str(1) == "Note On") {
		message = libremidi::message::note_on(
			channel, num_or_value(params, midi, 1),
			num_or_value(params, midi, 2));
	} else if (params->get_str(1) == "Note Off") {
		message = libremidi::message::note_off(
			channel, num_or_value(params, midi, 1),
			num_or_value(params, midi, 2));
	} else if (params->get_str(1) == "Control Change") {
		message = libremidi::message::control_change(
			channel, num_or_value(params, midi, 1),
			num_or_value(params, midi, 2));
	} else if (params->get_str(1) == "Program Change") {
		message = libremidi::message::program_change(
			channel, num_or_value(params, midi, 1));
	} else if (params->get_str(1) == "Pitch Bend") {
		int num1 = num_or_value(params, midi, 1);
		message = libremidi::message::pitch_bend(
			channel, num1 >= 64 ? ((num1 - 64) << 1) : 0, num1);
	} else {
		params->blog(
			LOG_INFO,
			"<MIDI> action failed - Set MIDI message is invalid.");
		return;
	}
	if (params->get_sub() == 0) {
		MMGDevice *const output =
			global()->find_device(params->get_str(0));
		if (!output) {
			params->blog(
				LOG_INFO,
				"<MIDI> action failed - Output device is not connected or does not exist.");
			return;
		}
		if (!MMGDevice::output_port_open())
			MMGDevice::open_output_port(output);
		output->output_send(message);
		MMGDevice::close_output_port();
	}
	params->blog(LOG_DEBUG, "<MIDI> executed.");
}

template<>
void MMGAction::do_action_specific<MMGAction::Internal>(const MMGAction *params,
							const MMGMessage *midi)
{
	MMGDevice *device = global()->find_current_device();
	MMGSharedMessage original = *reinterpret_cast<MMGSharedMessage *>(
		(qulonglong)params->get_num(0));
	if (original == 0) {
		open_message_box(
			"Error",
			"Casting Error: Internal memory was corrupted.\n\nReport this as a bug.");
		return;
	}
	if (params->get_sub() > 2 || params->get_sub() < 0)
		return;

	int i = 0;
	while (params->get_sub() >= i) {
		MMGBinding *binding = device->find_binding(params->get_str(i));
		if (!binding)
			return;
		binding->get_action()->do_action(original);
		++i;
	}
}

template<>
void MMGAction::do_action_specific<MMGAction::Timeout>(const MMGAction *params,
						       const MMGMessage *midi)
{
	switch ((MMGAction::Timeout)params->get_sub()) {
	case MMGAction::Timeout::TIMEOUT_MS:
		std::this_thread::sleep_for(std::chrono::milliseconds(
			(int)num_or_value(params, midi, 0)));
		break;
	case MMGAction::Timeout::TIMEOUT_S:
		std::this_thread::sleep_for(std::chrono::seconds(
			(int)num_or_value(params, midi, 0)));
		break;
	default:
		break;
	}
	params->blog(LOG_DEBUG, "<Pause> executed.");
}

void MMGAction::do_action(const MMGSharedMessage &params)
{
	if (executed)
		return;
	switch (get_category()) {
	case MMGAction::Category::MMGACTION_NONE:
		do_action_specific<MMGAction::None>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_STREAM:
		do_action_specific<MMGAction::Stream>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_RECORD:
		do_action_specific<MMGAction::Record>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_VIRCAM:
		do_action_specific<MMGAction::VirtualCam>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_REPBUF:
		do_action_specific<MMGAction::ReplayBuffer>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_STUDIOMODE:
		do_action_specific<MMGAction::StudioMode>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SCENE:
		do_action_specific<MMGAction::Scenes>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:
		do_action_specific<MMGAction::VideoSources>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:
		do_action_specific<MMGAction::AudioSources>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:
		do_action_specific<MMGAction::MediaSources>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_TRANSITION:
		do_action_specific<MMGAction::Transitions>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_FILTER:
		do_action_specific<MMGAction::Filters>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_HOTKEY:
		do_action_specific<MMGAction::Hotkeys>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_PROFILE:
		do_action_specific<MMGAction::Profiles>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_COLLECTION:
		do_action_specific<MMGAction::Collections>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_MIDI:
		do_action_specific<MMGAction::MidiMessage>(this, params.get());
		break;
	case MMGAction::Category::MMGACTION_INTERNAL:
		set_num(0, (qulonglong)&params);
		do_action_specific<MMGAction::Internal>(this, params.get());
		set_num(0, 0.0);
		break;
	case MMGAction::Category::MMGACTION_TIMEOUT:
		do_action_specific<MMGAction::Timeout>(this, params.get());
		break;
	default:
		break;
	}
	executed = true;
}
