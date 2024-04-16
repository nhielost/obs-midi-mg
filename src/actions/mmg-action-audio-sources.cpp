/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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

#include "mmg-action-audio-sources.h"

using namespace MMGUtils;

MMGActionAudioSources::MMGActionAudioSources(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  source(json_obj, "source", 1),
	  action(json_obj, "action", 2),
	  num(json_obj, "num", 1)
{
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionAudioSources::subNames() const
{
	return subModuleTextList(
		{"ChangeVolume", "IncrementVolume", "Mute", "Unmute", "ToggleMute", "AudioOffset", "AudioMonitor"});
}

void MMGActionAudioSources::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	source.json(json_obj, "source", false);
	action.json(json_obj, "action");
	num.json(json_obj, "num");
}

void MMGActionAudioSources::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionAudioSources *>(dest);
	if (!casted) return;

	casted->source = source.copy();
	casted->action = action.copy();
	casted->num = num.copy();
}

void MMGActionAudioSources::setEditable(bool edit)
{
	source.setEditable(edit);
	action.setEditable(edit);
	num.setEditable(edit);
}

void MMGActionAudioSources::toggle()
{
	action.toggle();
}

void MMGActionAudioSources::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	connect(display()->addNew(&source), &MMGStringDisplay::stringChanged, this,
		&MMGActionAudioSources::onList1Change);
	connect(display()->addNew(&action), &MMGStringDisplay::stringChanged, this,
		&MMGActionAudioSources::onList2Change);

	display()->addNew(&num);
}

void MMGActionAudioSources::setComboOptions(QComboBox *sub)
{
	MMGAction::setComboOptions(sub);
	if (type() == TYPE_OUTPUT) MMGInterface::setComboBoxItemEnabled(sub, 1, false);
}

void MMGActionAudioSources::setActionParams()
{
	display()->hideAll();

	MMGStringDisplay *source_display = display()->stringDisplay(0);
	source_display->setVisible(true);
	source_display->setDescription(mmgtr("Actions.AudioSources.AudioSource"));
	source_display->setBounds(enumerate());
}

void MMGActionAudioSources::onList1Change()
{
	connectSignals(true);

	MMGStringDisplay *action_display = display()->stringDisplay(1);
	MMGNumberDisplay *num_display = display()->numberDisplay(0);

	switch (sub()) {
		case SOURCE_AUDIO_VOLUME_CHANGETO:
		case SOURCE_AUDIO_VOLUME_CHANGEBY:
			action_display->setDisplayMode(MMGStringDisplay::MODE_THIN);
			action_display->setVisible(!source.value().isEmpty());
			action_display->setDescription(mmgtr("Actions.AudioSources.Format"));
			action_display->setBounds(volumeFormatOptions());
			return;

		case SOURCE_AUDIO_OFFSET:
			num_display->setVisible(!source.value().isEmpty());
			num_display->setDescription(obstr("Basic.AdvAudio.SyncOffset"));
			num_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM);
			num_display->setBounds(0.0, 20000.0);
			num_display->setStep(25.0);
			num_display->setDefaultValue(0.0);
			break;

		case SOURCE_AUDIO_MONITOR:
			action_display->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
			action_display->setVisible(true);
			action_display->setDescription(obstr("Basic.AdvAudio.Monitoring"));
			action_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
			action_display->setBounds(audioMonitorOptions());
			return;

		default:
			return;
	}

	num_display->reset();
}

void MMGActionAudioSources::onList2Change()
{
	MMGNumberDisplay *num_display = display()->numberDisplay(0);
	bool is_percent = action == volumeFormatOptions()[0];

	switch (sub()) {
		case SOURCE_AUDIO_VOLUME_CHANGETO:
			num_display->setVisible(true);
			num_display->setDescription(mmgtr("Actions.AudioSources.Volume"));
			num_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM);
			num_display->setBounds(is_percent ? 0.0 : -100.0, is_percent ? 100.0 : 27.0);
			num_display->setStep(0.1);
			num_display->setDefaultValue(0.0);
			break;

		case SOURCE_AUDIO_VOLUME_CHANGEBY:
			num_display->setVisible(true);
			num_display->setDescription(mmgtr("Actions.AudioSources.Volume"));
			num_display->setOptions(MIDIBUTTON_FIXED);
			num_display->setBounds(is_percent ? -25.0 : -20.0, is_percent ? 25.0 : 20.0);
			num_display->setStep(0.1);
			num_display->setDefaultValue(0.0);
			break;

		default:
			return;
	}

	num_display->reset();
}

const QStringList MMGActionAudioSources::enumerate()
{
	QStringList list;
	obs_enum_all_sources(
		[](void *param, obs_source_t *source) {
			auto _list = reinterpret_cast<QStringList *>(param);

			if (obs_source_get_type(source) != OBS_SOURCE_TYPE_INPUT) return true;
			if (!(obs_source_get_output_flags(source) & OBS_SOURCE_AUDIO)) return true;

			_list->append(obs_source_get_name(source));
			return true;
		},
		&list);
	return list;
}

const QStringList MMGActionAudioSources::audioMonitorOptions()
{
	return MMGText::batch(TEXT_OBS, "Basic.AdvAudio.Monitoring", {"None", "MonitorOnly", "Both"});
}

const QStringList MMGActionAudioSources::volumeFormatOptions()
{
	return MMGText::batch(TEXT_MMG, "Actions.AudioSources.Format", {"Percent", "Decibels"});
}

double MMGActionAudioSources::convertDecibels(double value, bool convert_to)
{
	// true = convert to decibels, false = convert to percentage
	return convert_to ? (20 * std::log10(value)) : std::pow(10, value / 20);
}

void MMGActionAudioSources::execute(const MMGMessage *midi) const
{
	OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
	ACTION_ASSERT(obs_source, "Source does not exist.");

	ACTION_ASSERT(obs_source_get_output_flags(obs_source) & OBS_SOURCE_AUDIO, "Source is not an audio source.");

	bool is_percent = action == volumeFormatOptions()[0];
	double util_value = num.chooseFrom(midi);
	QStringList util_list;

	switch (sub()) {
		case SOURCE_AUDIO_VOLUME_CHANGETO:
			obs_source_set_volume(obs_source,
					      is_percent ? util_value / 100.0 : convertDecibels(util_value, false));
			break;

		case SOURCE_AUDIO_VOLUME_CHANGEBY:
			if (is_percent) {
				util_value /= 100.0;
				util_value += obs_source_get_volume(obs_source);
			} else {
				util_value = convertDecibels(
					convertDecibels(obs_source_get_volume(obs_source), true) + util_value, false);
			}
			obs_source_set_volume(obs_source, util_value);
			break;

		case SOURCE_AUDIO_VOLUME_MUTE_ON:
			obs_source_set_muted(obs_source, true);
			break;

		case SOURCE_AUDIO_VOLUME_MUTE_OFF:
			obs_source_set_muted(obs_source, false);
			break;

		case SOURCE_AUDIO_VOLUME_MUTE_TOGGLE_ONOFF:
			obs_source_set_muted(obs_source, !obs_source_muted(obs_source));
			break;

		case SOURCE_AUDIO_OFFSET:
			obs_source_set_sync_offset(obs_source, (num.chooseFrom(midi) * 1000000));
			break;

		case SOURCE_AUDIO_MONITOR:
			util_list = audioMonitorOptions();
			util_value = util_list.indexOf(action.chooseFrom(midi, util_list));
			obs_source_set_monitoring_type(obs_source, (obs_monitoring_type)(util_value));
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionAudioSources::connectSignals(bool _connect)
{
	MMGAction::connectSignals(_connect);
	if (!_connected) return;

	connectSourceSignal(mmgsignals()->requestSourceSignalByName(source));
}

void MMGActionAudioSources::sourceEventReceived(MMGSourceSignal::Event event, QVariant data)
{
	MMGNumber value;
	double util_value;

	switch (sub()) {
		case SOURCE_AUDIO_VOLUME_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_VOLUME) return;

			value = num;
			util_value = data.toDouble();
			util_value = action == volumeFormatOptions()[0] ? util_value * 100.0
									: convertDecibels(util_value, true);
			if (!num.acceptable(util_value)) return;

			value = util_value;
			break;

		case SOURCE_AUDIO_VOLUME_MUTED:
			if (event != MMGSourceSignal::SIGNAL_MUTE) return;
			if (!data.toBool()) return;
			break;

		case SOURCE_AUDIO_VOLUME_UNMUTED:
			if (event != MMGSourceSignal::SIGNAL_MUTE) return;
			if (data.toBool()) return;
			break;

		case SOURCE_AUDIO_VOLUME_TOGGLE_MUTED:
			if (event != MMGSourceSignal::SIGNAL_MUTE) return;
			break;

		case SOURCE_AUDIO_OFFSET_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_SYNC_OFFSET) return;

			value = num;
			util_value = data.toDouble() / 1000000.0;
			if (!num.acceptable(util_value)) return;

			value = util_value;
			break;

		case SOURCE_AUDIO_MONITOR_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_MONITOR) return;

			value = data.toInt();
			if (action.chooseTo(value, audioMonitorOptions())) return;
			break;

		default:
			return;
	}

	triggerEvent({value});
}
