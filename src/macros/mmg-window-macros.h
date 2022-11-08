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

#define SET_LCD_STATUS(lcd, kind, status)        \
	ui->down_major_##lcd->set##kind(status); \
	ui->down_minor_##lcd->set##kind(status); \
	ui->up_minor_##lcd->set##kind(status);   \
	ui->up_major_##lcd->set##kind(status);   \
	ui->label_##lcd->set##kind(status);      \
	ui->lcd_##lcd->set##kind(status)

#define CONNECT_LCD(kind)                                                     \
	connect(ui->down_major_##kind, &QAbstractButton::clicked, this,       \
		[&]() {                                                       \
			lcd_##kind.down_major();                              \
			set_##kind(lcd_##kind.get_value());                   \
		});                                                           \
	connect(ui->down_minor_##kind, &QAbstractButton::clicked, this,       \
		[&]() {                                                       \
			lcd_##kind.down_minor();                              \
			set_##kind(lcd_##kind.get_value());                   \
		});                                                           \
	connect(ui->up_minor_##kind, &QAbstractButton::clicked, this, [&]() { \
		lcd_##kind.up_minor();                                        \
		set_##kind(lcd_##kind.get_value());                           \
	});                                                                   \
	connect(ui->up_major_##kind, &QAbstractButton::clicked, this, [&]() { \
		lcd_##kind.up_major();                                        \
		set_##kind(lcd_##kind.get_value());                           \
	})

#define INIT_LCD(kind) lcd_##kind = LCDData(ui->lcd_##kind);

#define COMBOBOX_ITEM_STATE(list, index, state)               \
	qobject_cast<QStandardItemModel *>(ui->list->model()) \
		->item(index)                                 \
		->setEnabled(state);

#define INSERT_SUB_OPTIONS()                                                                           \
	current_action->set_category((MMGAction::Category)index);                                      \
	set_strs_visible();                                                                            \
	current_action->set_sub(0);                                                                    \
	set_str1("");                                                                                  \
	set_str2("");                                                                                  \
	set_str3("");                                                                                  \
	set_double1(0.0);                                                                              \
	set_double2(0.0);                                                                              \
	set_double3(0.0);                                                                              \
	set_double4(0.0);                                                                              \
	switch (current_action->get_category()) {                                                      \
	case MMGAction::Category::MMGACTION_NONE:                                                      \
		set_sub_options({"None"});                                                             \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_STREAM:                                                    \
		set_sub_options({"Start Streaming", "Stop Streaming",                                  \
				 "Toggle Streaming"});                                                 \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_RECORD:                                                    \
		set_sub_options({"Start Recording", "Stop Recording",                                  \
				 "Toggle Recording", "Pause Recording",                                \
				 "Resume Recording",                                                   \
				 "Toggle Pause Recording"});                                           \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_VIRCAM:                                                    \
		set_sub_options({"Start Virtual Camera",                                               \
				 "Stop Virtual Camera",                                                \
				 "Toggle Virtual Camera"});                                            \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_REPBUF:                                                    \
		set_sub_options({"Start Replay Buffer", "Stop Replay Buffer",                          \
				 "Toggle Replay Buffer",                                               \
				 "Save Replay Buffer"});                                               \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_STUDIOMODE:                                                \
		set_sub_options({"Turn On Studio Mode",                                                \
				 "Turn Off Studio Mode", "Toggle Studio Mode",                         \
				 "Change Preview Scene",                                               \
				 "Preview to Program"});                                               \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_SCENE:                                                     \
		set_sub_options({"Scene Switching"});                                                  \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:                                              \
		set_sub_options(                                                                       \
			{"Move Source", "Display Source", "Source Locking",                            \
			 "Source Crop", "Align Source", "Source Scale",                                \
			 "Source Scale Filtering", "Rotate Source",                                    \
			 "Source Bounding Box Type",                                                   \
			 "Resize Source Bounding Box",                                                 \
			 "Align Source Bounding Box", "Source Blending Mode",                          \
			 "Take Source Screenshot"});                                                   \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:                                              \
		set_sub_options(                                                                       \
			{"Change Source Volume To", "Change Source Volume By",                         \
			 "Mute Source", "Unmute Source", "Toggle Source Mute",                         \
			 "Source Audio Offset", "Source Audio Monitor"});                              \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:                                              \
		set_sub_options({"Play or Pause", "Restart", "Stop",                                   \
				 "Set Track Time", "Next Track",                                       \
				 "Previous Track", "Skip Forward Time",                                \
				 "Skip Backward Time"});                                               \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_TRANSITION:                                                \
		set_sub_options(                                                                       \
			{"Change Current Transition", /*"Set Transition Bar Position (Studio Mode)",*/ \
			 "Set Source Show Transition",                                                 \
			 "Set Source Hide Transition"});                                               \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_FILTER:                                                    \
		set_sub_options({"Show Filter", "Hide Filter",                                         \
				 "Toggle Filter Display",                                              \
				 "Reorder Filter Appearance"});                                        \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_HOTKEY:                                                    \
		set_sub_options({"Activate Hotkey"});                                                  \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_PROFILE:                                                   \
		set_sub_options({"Switch Profiles"});                                                  \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_COLLECTION:                                                \
		set_sub_options({"Switch Scene Collections"});                                         \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_MIDI:                                                      \
		set_sub_options({"Send a MIDI Message"});                                              \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_INTERNAL:                                                  \
		set_sub_options(                                                                       \
			{"Do 1 Action", "Do 2 Actions", "Do 3 Actions"});                              \
		break;                                                                                 \
	case MMGAction::Category::MMGACTION_TIMEOUT:                                                   \
		set_sub_options({"Wait in Milliseconds", "Wait in Seconds"});                          \
		break;                                                                                 \
	}

#define INSERT_FIRST_OPTION()                                               \
	current_action->set_sub(index);                                     \
	ui->editor_str1->clear();                                           \
	ui->editor_str2->clear();                                           \
	ui->editor_str3->clear();                                           \
	ui->lcd_double1->display(0);                                        \
	ui->lcd_double2->display(0);                                        \
	ui->lcd_double3->display(0);                                        \
	ui->lcd_double4->display(0);                                        \
	ui->editor_double1->setCurrentIndex(0);                             \
	ui->editor_double2->setCurrentIndex(0);                             \
	ui->editor_double3->setCurrentIndex(0);                             \
	ui->editor_double4->setCurrentIndex(0);                             \
	set_strs_visible();                                                 \
	set_doubles_visible();                                              \
	switch (current_action->get_category()) {                           \
	case MMGAction::Category::MMGACTION_NONE:                           \
	case MMGAction::Category::MMGACTION_STREAM:                         \
	case MMGAction::Category::MMGACTION_RECORD:                         \
	case MMGAction::Category::MMGACTION_VIRCAM:                         \
	case MMGAction::Category::MMGACTION_REPBUF:                         \
		break;                                                      \
	case MMGAction::Category::MMGACTION_STUDIOMODE:                     \
		if (index == 3) {                                           \
			set_strs_visible(true);                             \
			ui->label_str1->setText("Scene");                   \
			MMGAction::do_obs_scene_enum(ui->editor_str1);      \
			ui->editor_str1->addItem("Use Message Value");      \
		}                                                           \
		break;                                                      \
	case MMGAction::Category::MMGACTION_SCENE:                          \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Scene");                           \
		MMGAction::do_obs_scene_enum(ui->editor_str1);              \
		ui->editor_str1->addItem("Use Message Value");              \
		break;                                                      \
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:                   \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Scene");                           \
		MMGAction::do_obs_scene_enum(ui->editor_str1);              \
		break;                                                      \
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:                   \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Source");                          \
		MMGAction::do_obs_source_enum(                              \
			ui->editor_str1,                                    \
			MMGAction::Category::MMGACTION_SOURCE_AUDIO);       \
		break;                                                      \
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:                   \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Source");                          \
		MMGAction::do_obs_media_enum(ui->editor_str1);              \
		break;                                                      \
	case MMGAction::Category::MMGACTION_TRANSITION:                     \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Transition");                      \
		MMGAction::do_obs_transition_enum(ui->editor_str1);         \
		break;                                                      \
	case MMGAction::Category::MMGACTION_FILTER:                         \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Source");                          \
		MMGAction::do_obs_source_enum(                              \
			ui->editor_str1,                                    \
			MMGAction::Category::MMGACTION_FILTER);             \
		break;                                                      \
	case MMGAction::Category::MMGACTION_HOTKEY:                         \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Hotkey");                          \
		MMGAction::do_obs_hotkey_enum(ui->editor_str1);             \
		break;                                                      \
	case MMGAction::Category::MMGACTION_PROFILE:                        \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Profile");                         \
		MMGAction::do_obs_profile_enum(ui->editor_str1);            \
		ui->editor_str1->addItem("Use Message Value");              \
		break;                                                      \
	case MMGAction::Category::MMGACTION_COLLECTION:                     \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Collection");                      \
		MMGAction::do_obs_collection_enum(ui->editor_str1);         \
		ui->editor_str1->addItem("Use Message Value");              \
		break;                                                      \
	case MMGAction::Category::MMGACTION_MIDI:                           \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Device");                          \
		ui->editor_str1->addItems(                                  \
			MMGDevice::get_output_device_names());              \
		break;                                                      \
	case MMGAction::Category::MMGACTION_INTERNAL:                       \
		set_strs_visible(true);                                     \
		ui->label_str1->setText("Action 1");                        \
		MMGAction::do_mmg_binding_enum(ui->editor_str1,             \
					       current_binding->get_name(), \
					       current_action->get_str(0)); \
		break;                                                      \
	case MMGAction::Category::MMGACTION_TIMEOUT:                        \
		set_doubles_visible(true);                                  \
		COMBOBOX_ITEM_STATE(editor_double1, 2, false);              \
		ui->label_double1->setText("Time");                         \
		lcd_double1.set_range(0.0, 1000.0);                         \
		lcd_double1.set_step(1.0, 10.0);                            \
		lcd_double1.reset();                                        \
		break;                                                      \
	default:                                                            \
		break;                                                      \
	}

#define INSERT_SECOND_OPTION()                                                  \
	set_strs_visible(true);                                                 \
	set_doubles_visible();                                                  \
	if (value.isEmpty())                                                    \
		return;                                                         \
	current_action->set_str(0, value);                                      \
	ui->editor_str2->clear();                                               \
	lcd_double1.set_use_time(false);                                        \
	COMBOBOX_ITEM_STATE(editor_double1, 1, true);                           \
	COMBOBOX_ITEM_STATE(editor_double1, 2, false);                          \
	switch (current_action->get_category()) {                               \
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:                       \
		set_strs_visible(true, true);                                   \
		ui->label_str2->setText("Source");                              \
		MMGAction::do_obs_source_enum(                                  \
			ui->editor_str2, MMGAction::Category::MMGACTION_SCENE,  \
			value);                                                 \
		break;                                                          \
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:                       \
		switch ((MMGAction::AudioSources)current_action->get_sub()) {   \
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGETO:     \
			set_doubles_visible(true);                              \
			ui->label_double1->setText("Volume");                   \
			lcd_double1.set_range(0.0, 100.0);                      \
			lcd_double1.set_step(1.0, 10.0);                        \
			lcd_double1.reset();                                    \
			break;                                                  \
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_CHANGEBY:     \
			set_doubles_visible(true);                              \
			COMBOBOX_ITEM_STATE(editor_double1, 1, false);          \
			ui->label_double1->setText("Volume Adj.");              \
			lcd_double1.set_range(-50.0, 50.0);                     \
			lcd_double1.set_step(1.0, 10.0);                        \
			lcd_double1.reset();                                    \
			break;                                                  \
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_MUTE_ON:      \
		case MMGAction::AudioSources::SOURCE_AUDIO_VOLUME_MUTE_OFF:     \
		case MMGAction::AudioSources::                                  \
			SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF:                  \
			break;                                                  \
		case MMGAction::AudioSources::SOURCE_AUDIO_OFFSET:              \
			set_doubles_visible(true);                              \
			ui->label_double1->setText("Offset");                   \
			lcd_double1.set_range(0.0, 20000.0);                    \
			lcd_double1.set_step(25.0, 250.0);                      \
			lcd_double1.reset();                                    \
			break;                                                  \
		case MMGAction::AudioSources::SOURCE_AUDIO_MONITOR:             \
			set_strs_visible(true, true);                           \
			ui->label_str2->setText("Monitor");                     \
			ui->editor_str2->addItems({"Off", "Monitor Only",       \
						   "Monitor & Output",          \
						   "Use Message Value"});       \
			break;                                                  \
		default:                                                        \
			break;                                                  \
		}                                                               \
		break;                                                          \
	case MMGAction::Category::MMGACTION_SOURCE_MEDIA:                       \
		lcd_double1.set_use_time(true);                                 \
		switch ((MMGAction::MediaSources)current_action->get_sub()) {   \
		case MMGAction::MediaSources::SOURCE_MEDIA_TOGGLE_PLAYPAUSE:    \
		case MMGAction::MediaSources::SOURCE_MEDIA_RESTART:             \
		case MMGAction::MediaSources::SOURCE_MEDIA_STOP:                \
			break;                                                  \
		case MMGAction::MediaSources::SOURCE_MEDIA_TIME:                \
			set_doubles_visible(true);                              \
			ui->label_double1->setText("Time");                     \
			lcd_double1.set_range(                                  \
				0.0, get_obs_media_length(                      \
					     current_action->get_str(0)));      \
			lcd_double1.set_step(1.0, 10.0);                        \
			lcd_double1.reset();                                    \
			break;                                                  \
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_FORWARD_TRACK:  \
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TRACK: \
			break;                                                  \
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_FORWARD_TIME:   \
		case MMGAction::MediaSources::SOURCE_MEDIA_SKIP_BACKWARD_TIME:  \
			set_doubles_visible(true);                              \
			COMBOBOX_ITEM_STATE(editor_double1, 1, false);          \
			ui->label_double1->setText("Time Adj.");                \
			lcd_double1.set_range(                                  \
				0.0, get_obs_media_length(                      \
					     current_action->get_str(0)));      \
			lcd_double1.set_step(1.0, 10.0);                        \
			lcd_double1.reset();                                    \
			break;                                                  \
		default:                                                        \
			break;                                                  \
		}                                                               \
		break;                                                          \
	case MMGAction::Category::MMGACTION_TRANSITION:                         \
		switch ((MMGAction::Transitions)current_action->get_sub()) {    \
		case MMGAction::Transitions::TRANSITION_CURRENT:                \
			set_doubles_visible(true);                              \
			COMBOBOX_ITEM_STATE(editor_double1, 2, true);           \
			ui->label_double1->setText("Duration");                 \
			lcd_double1.set_range(25.0, 20000.0);                   \
			lcd_double1.set_step(25.0, 250.0);                      \
			lcd_double1.reset(                                      \
				obs_frontend_get_transition_duration());        \
			break;                                                  \
		/*case MMGAction::Transitions::TRANSITION_TBAR:\
			set_doubles_visible(true);\
			ui->label_double1->setText("Position (%)");\
			lcd_double1.set_range(0.0, 100.0);\
			lcd_double1.set_step(0.5, 5.0);\
			lcd_double1.reset();\
			break;*/               \
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:            \
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:            \
			set_strs_visible(true, true);                           \
			ui->label_str2->setText("Scene");                       \
			MMGAction::do_obs_scene_enum(ui->editor_str2);          \
			break;                                                  \
		default:                                                        \
			break;                                                  \
		}                                                               \
		break;                                                          \
	case MMGAction::Category::MMGACTION_FILTER:                             \
		set_strs_visible(true, true);                                   \
		ui->label_str2->setText("Filter");                              \
		MMGAction::do_obs_filter_enum(                                  \
			ui->editor_str2,                                        \
			MMGAction::Category::MMGACTION_SOURCE_VIDEO, value);    \
		break;                                                          \
	case MMGAction::Category::MMGACTION_MIDI:                               \
		set_strs_visible(true, true);                                   \
		ui->label_str2->setText("Type");                                \
		for (int i = 0; i < ui->editor_type->count(); ++i) {            \
			if (ui->editor_type->itemText(i) ==                     \
			    "Note On / Note Off")                               \
				continue;                                       \
			ui->editor_str2->addItem(                               \
				ui->editor_type->itemText(i));                  \
		}                                                               \
		break;                                                          \
	case MMGAction::Category::MMGACTION_INTERNAL:                           \
		if (current_action->get_sub() > 0) {                            \
			set_strs_visible(true, true);                           \
			ui->label_str2->setText("Action 2");                    \
			MMGAction::do_mmg_binding_enum(                         \
				ui->editor_str2, current_binding->get_name(),   \
				current_action->get_str(1));                    \
		}                                                               \
		break;                                                          \
	default:                                                                \
		break;                                                          \
	}

#define INSERT_THIRD_OPTION()                                                  \
	if (value.isEmpty())                                                   \
		return;                                                        \
	current_action->set_str(1, value);                                     \
	ui->editor_str3->clear();                                              \
	COMBOBOX_ITEM_STATE(editor_double1, 1, true);                          \
	COMBOBOX_ITEM_STATE(editor_double3, 1, true);                          \
	COMBOBOX_ITEM_STATE(editor_double1, 2, false);                         \
	COMBOBOX_ITEM_STATE(editor_double2, 2, false);                         \
	COMBOBOX_ITEM_STATE(editor_double3, 2, false);                         \
	COMBOBOX_ITEM_STATE(editor_double4, 2, false);                         \
	switch (current_action->get_category()) {                              \
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:                      \
		switch ((MMGAction::VideoSources)current_action->get_sub()) {  \
		case MMGAction::VideoSources::SOURCE_VIDEO_POSITION:           \
			set_doubles_visible(true, true);                       \
			COMBOBOX_ITEM_STATE(editor_double1, 2, true);          \
			COMBOBOX_ITEM_STATE(editor_double2, 2, true);          \
			ui->label_double1->setText("Pos X");                   \
			ui->label_double2->setText("Pos Y");                   \
			lcd_double1.set_range(0.0,                             \
					      get_obs_dimensions().first);     \
			lcd_double1.set_step(0.5, 5.0);                        \
			lcd_double1.reset();                                   \
			lcd_double2.set_range(0.0,                             \
					      get_obs_dimensions().second);    \
			lcd_double2.set_step(0.5, 5.0);                        \
			lcd_double2.reset();                                   \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_DISPLAY:            \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("State");                      \
			ui->editor_str3->addItems({"Show", "Hide", "Toggle"}); \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_LOCKED:             \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("State");                      \
			ui->editor_str3->addItems(                             \
				{"Locked", "Unlocked", "Toggle"});             \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_CROP:               \
			set_doubles_visible(true, true, true, true);           \
			COMBOBOX_ITEM_STATE(editor_double1, 2, true);          \
			COMBOBOX_ITEM_STATE(editor_double2, 2, true);          \
			COMBOBOX_ITEM_STATE(editor_double3, 2, true);          \
			COMBOBOX_ITEM_STATE(editor_double4, 2, true);          \
			ui->label_double1->setText("Top");                     \
			ui->label_double2->setText("Right");                   \
			ui->label_double3->setText("Bottom");                  \
			ui->label_double4->setText("Left");                    \
			lcd_double1.set_range(                                 \
				0.0,                                           \
				get_obs_source_dimensions(value).second >> 1); \
			lcd_double1.set_step(0.5, 5.0);                        \
			lcd_double1.reset();                                   \
			lcd_double2.set_range(                                 \
				0.0,                                           \
				get_obs_source_dimensions(value).first >> 1);  \
			lcd_double2.set_step(0.5, 5.0);                        \
			lcd_double2.reset();                                   \
			lcd_double3.set_range(                                 \
				0.0,                                           \
				get_obs_source_dimensions(value).second >> 1); \
			lcd_double3.set_step(0.5, 5.0);                        \
			lcd_double3.reset();                                   \
			lcd_double4.set_range(                                 \
				0.0,                                           \
				get_obs_source_dimensions(value).first >> 1);  \
			lcd_double4.set_step(0.5, 5.0);                        \
			lcd_double4.reset();                                   \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_ALIGNMENT:          \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("Alignment");                  \
			ui->editor_str3->addItems(                             \
				{"Top Left", "Top Center", "Top Right",        \
				 "Middle Left", "Middle Center",               \
				 "Middle Right", "Bottom Left",                \
				 "Bottom Center", "Bottom Right",              \
				 "Use Message Value"});                        \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_SCALE:              \
			set_doubles_visible(true, true, true);                 \
			COMBOBOX_ITEM_STATE(editor_double1, 2, true);          \
			COMBOBOX_ITEM_STATE(editor_double2, 2, true);          \
			COMBOBOX_ITEM_STATE(editor_double3, 1, false);         \
			ui->label_double1->setText("Scale X");                 \
			ui->label_double2->setText("Scale Y");                 \
			ui->label_double3->setText("Magnitude");               \
			lcd_double1.set_range(0.0, 100.0);                     \
			lcd_double1.set_step(1.0, 10.0);                       \
			lcd_double1.reset(0.0);                                \
			lcd_double2.set_range(0.0, 100.0);                     \
			lcd_double2.set_step(1.0, 10.0);                       \
			lcd_double2.reset(0.0);                                \
			lcd_double3.set_range(0.5, 100.0);                     \
			lcd_double3.set_step(0.5, 5.0);                        \
			lcd_double3.reset(1.0);                                \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_SCALEFILTER:        \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("Filtering");                  \
			ui->editor_str3->addItems(                             \
				{"Disable", "Point", "Bilinear", "Bicubic",    \
				 "Lanczos", "Area", "Use Message Value"});     \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_ROTATION:           \
			set_doubles_visible(true);                             \
			ui->label_double1->setText("Rotation");                \
			lcd_double1.set_range(0.0, 360.0);                     \
			lcd_double1.set_step(0.5, 5.0);                        \
			lcd_double1.reset();                                   \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_TYPE:  \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("Type");                       \
			ui->editor_str3->addItems(                             \
				{"No Bounds", "Stretch to Bounds",             \
				 "Scale to Inner Bounds",                      \
				 "Scale to Outer Bounds",                      \
				 "Scale to Width of Bounds",                   \
				 "Scale to Height of Bounds", "Maximum Size",  \
				 "Use Message Value"});                        \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_SIZE:  \
			set_doubles_visible(true, true);                       \
			COMBOBOX_ITEM_STATE(editor_double1, 2, true);          \
			COMBOBOX_ITEM_STATE(editor_double2, 2, true);          \
			ui->label_double1->setText("Size X");                  \
			ui->label_double2->setText("Size Y");                  \
			lcd_double1.set_range(                                 \
				0.0, get_obs_source_dimensions(                \
					     ui->editor_str1->currentText())   \
					     .first);                          \
			lcd_double1.set_step(0.5, 5.0);                        \
			lcd_double1.reset();                                   \
			lcd_double2.set_range(                                 \
				0.0, get_obs_source_dimensions(                \
					     ui->editor_str1->currentText())   \
					     .second);                         \
			lcd_double2.set_step(0.5, 5.0);                        \
			lcd_double2.reset();                                   \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_ALIGN: \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("Alignment");                  \
			ui->editor_str3->addItems(                             \
				{"Top Left", "Top Center", "Top Right",        \
				 "Middle Left", "Middle Center",               \
				 "Middle Right", "Bottom Left",                \
				 "Bottom Center", "Bottom Right",              \
				 "Use Message Value"});                        \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_BLEND_MODE:         \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("Blend Mode");                 \
			ui->editor_str3->addItems(                             \
				{"Normal", "Additive", "Subtract", "Screen",   \
				 "Multiply", "Lighten", "Darken",              \
				 "Use Message Value"});                        \
			break;                                                 \
		case MMGAction::VideoSources::SOURCE_VIDEO_SCREENSHOT:         \
		case MMGAction::VideoSources::SOURCE_VIDEO_CUSTOM:             \
			break;                                                 \
		}                                                              \
		break;                                                         \
	case MMGAction::Category::MMGACTION_SOURCE_AUDIO:                      \
		if (current_action->get_sub() == 6) {                          \
			if (value == "Use Message Value") {                    \
				current_action->set_num_state(0, 1);           \
			} else {                                               \
				set_double1(ui->editor_str2->currentIndex());  \
				current_action->set_num_state(0, 0);           \
			}                                                      \
		}                                                              \
		break;                                                         \
	case MMGAction::Category::MMGACTION_TRANSITION:                        \
		switch ((MMGAction::Transitions)current_action->get_sub()) {   \
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:           \
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:           \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("Source");                     \
			MMGAction::do_obs_source_enum(                         \
				ui->editor_str3,                               \
				MMGAction::Category::MMGACTION_SCENE, value);  \
			break;                                                 \
		default:                                                       \
			break;                                                 \
		}                                                              \
		break;                                                         \
	case MMGAction::Category::MMGACTION_FILTER:                            \
		switch ((MMGAction::Filters)current_action->get_sub()) {       \
		case MMGAction::Filters::FILTER_REORDER:                       \
			set_doubles_visible(true);                             \
			ui->label_double1->setText("Position");                \
			lcd_double1.set_range(                                 \
				1.0, get_obs_source_filter_count(              \
					     ui->editor_str1->currentText())); \
			lcd_double1.set_step(1.0, 5.0);                        \
			lcd_double1.reset(1.0);                                \
			break;                                                 \
		default:                                                       \
			break;                                                 \
		}                                                              \
		break;                                                         \
	case MMGAction::Category::MMGACTION_MIDI:                              \
		COMBOBOX_ITEM_STATE(editor_double1, 1, false);                 \
		ui->label_double1->setText("Channel");                         \
		lcd_double1.set_range(1.0, 16.0);                              \
		lcd_double1.set_step(1.0, 5.0);                                \
		lcd_double1.reset(1.0);                                        \
		lcd_double2.set_range(0.0, 127.0);                             \
		lcd_double2.set_step(1.0, 10.0);                               \
		lcd_double2.reset();                                           \
		lcd_double3.set_range(0.0, 127.0);                             \
		lcd_double3.set_step(1.0, 10.0);                               \
		lcd_double3.reset();                                           \
		if (value == "Note On" || value == "Note Off") {               \
			set_doubles_visible(true, true, true);                 \
			ui->label_double2->setText("Note #");                  \
			ui->label_double3->setText("Velocity");                \
		} else if (value == "Control Change") {                        \
			set_doubles_visible(true, true, true);                 \
			ui->label_double2->setText("Control");                 \
			ui->label_double3->setText("Value");                   \
		} else if (value == "Program Change") {                        \
			set_doubles_visible(true, true);                       \
			ui->label_double2->setText("Program");                 \
		} else if (value == "Pitch Bend") {                            \
			set_doubles_visible(true, true);                       \
			ui->label_double2->setText("Pitch Adj");               \
		}                                                              \
		break;                                                         \
	case MMGAction::Category::MMGACTION_INTERNAL:                          \
		if (current_action->get_sub() == 2) {                          \
			set_strs_visible(true, true, true);                    \
			ui->label_str3->setText("Action 3");                   \
			MMGAction::do_mmg_binding_enum(                        \
				ui->editor_str3, current_binding->get_name(),  \
				current_action->get_str(2));                   \
		}                                                              \
		break;                                                         \
	default:                                                               \
		break;                                                         \
	}

#define INSERT_FOURTH_OPTION()                                                 \
	current_action->set_str(2, value);                                     \
	COMBOBOX_ITEM_STATE(editor_double1, 2, true);                          \
	switch (current_action->get_category()) {                              \
	case MMGAction::Category::MMGACTION_SOURCE_VIDEO:                      \
		switch ((MMGAction::VideoSources)current_action->get_sub()) {  \
		case MMGAction::VideoSources::SOURCE_VIDEO_ALIGNMENT:          \
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_TYPE:  \
		case MMGAction::VideoSources::SOURCE_VIDEO_BOUNDING_BOX_ALIGN: \
		case MMGAction::VideoSources::SOURCE_VIDEO_SCALEFILTER:        \
		case MMGAction::VideoSources::SOURCE_VIDEO_BLEND_MODE:         \
			if (value == "Use Message Value") {                    \
				current_action->set_num_state(0, 1);           \
			} else {                                               \
				set_double1(ui->editor_str3->currentIndex());  \
				current_action->set_num_state(0, 0);           \
			}                                                      \
			break;                                                 \
		default:                                                       \
			break;                                                 \
		}                                                              \
		break;                                                         \
	case MMGAction::Category::MMGACTION_TRANSITION:                        \
		switch ((MMGAction::Transitions)current_action->get_sub()) {   \
		case MMGAction::Transitions::TRANSITION_SOURCE_SHOW:           \
		case MMGAction::Transitions::TRANSITION_SOURCE_HIDE:           \
			set_doubles_visible(true);                             \
			COMBOBOX_ITEM_STATE(editor_double1, 2, false);         \
			ui->label_double1->setText("Duration");                \
			lcd_double1.set_range(0.0, 20000.0);                   \
			lcd_double1.set_step(25.0, 250.0);                     \
			lcd_double1.reset();                                   \
			break;                                                 \
		default:                                                       \
			break;                                                 \
		}                                                              \
		break;                                                         \
	default:                                                               \
		break;                                                         \
	}

#define PREFERENCE_FUNCTION_DECL(style)                                                                     \
	void style::on_active_change(bool toggle)                                                           \
	{                                                                                                   \
		global()->preferences().set_active(toggle);                                                 \
	}                                                                                                   \
                                                                                                            \
	void style::export_bindings()                                                                       \
	{                                                                                                   \
		QString filepath = QFileDialog::getSaveFileName(                                            \
			this, tr("Save Bindings..."),                                                       \
			MMGConfig::get_filepath(), "JSON Files (*.json)");                                  \
		if (!filepath.isNull())                                                                     \
			global()->save(filepath);                                                           \
	}                                                                                                   \
                                                                                                            \
	void style::import_bindings()                                                                       \
	{                                                                                                   \
		QString filepath = QFileDialog::getOpenFileName(                                            \
			this, tr("Open Bindings File..."), "",                                              \
			"JSON Files (*.json)");                                                             \
		if (!filepath.isNull())                                                                     \
			global()->load(filepath);                                                           \
	}                                                                                                   \
                                                                                                            \
	void style::i_need_help() const                                                                     \
	{                                                                                                   \
		QDesktopServices::openUrl(QUrl(                                                             \
			"https://github.com/nhielost/obs-midi-mg/blob/master/HELP.md"));                    \
	}                                                                                                   \
                                                                                                            \
	void style::report_a_bug() const                                                                    \
	{                                                                                                   \
		QDesktopServices::openUrl(QUrl(                                                             \
			"https://github.com/nhielost/obs-midi-mg/issues"));                                 \
	}                                                                                                   \
                                                                                                            \
	void style::on_transfer_mode_change(short index)                                                    \
	{                                                                                                   \
		switch (index) {                                                                            \
		case 0:                                                                                     \
			ui->text_transfer_mode->setText(                                                    \
				"In this mode, bindings will be copied from the source device "             \
				"to the destination device. "                                               \
				"The destination device will then contain both device's bindings.");        \
			break;                                                                              \
		case 1:                                                                                     \
			ui->text_transfer_mode->setText(                                                    \
				"In this mode, bindings will be removed from the source device "            \
				"and added to the destination device. "                                     \
				"The destination device will then contain both device's bindings.");        \
			break;                                                                              \
		case 2:                                                                                     \
			ui->text_transfer_mode->setText(                                                    \
				"In this mode, bindings will be removed from the source device, "           \
				"and they will also replace the bindings in the destination device. "       \
				"NOTE: This will remove all existing bindings in the destination device."); \
			break;                                                                              \
		}                                                                                           \
	}                                                                                                   \
                                                                                                            \
	void style::on_transfer_bindings_click()                                                            \
	{                                                                                                   \
		transfer_bindings(ui->editor_transfer_mode->currentIndex(),                                 \
				  ui->editor_transfer_source->currentText(),                                \
				  ui->editor_transfer_dest->currentText());                                 \
	}                                                                                                   \
                                                                                                            \
	void style::on_interface_style_change(short index)                                                  \
	{                                                                                                   \
		/* global()->preferences().set_ui_style(index); */                                          \
	}                                                                                                   \
                                                                                                            \
	void style::on_update_check()                                                                       \
	{                                                                                                   \
		QDesktopServices::openUrl(QUrl(                                                             \
			"https://github.com/nhielost/obs-midi-mg/releases"));                               \
	}
