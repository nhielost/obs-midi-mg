/*
obs-midi-mg
Copyright (C) 2022-2026 nhielost <nhielost@gmail.com>

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
#include "../mmg-obs-object.h"
#include "mmg-action.h"

namespace MMGActions {

const MMGParams<MMGString> &sourceParams(uint64_t type = 0);

const MMGStringTranslationMap enumerateAllSources();
const MMGStringTranslationMap enumerateAudioSources();
const MMGStringTranslationMap enumerateMediaSources();

class MMGActionSources : public MMGAction, public MMGSignal::MMGSourceReceiver {
	Q_OBJECT

public:
	MMGActionSources(MMGActionManager *parent, const QJsonObject &json_obj);
	virtual ~MMGActionSources() = default;

	virtual const char *categoryName() const final override { return "Sources"; };
	virtual const char *trActionName() const override = 0;

	MMGString sourceId() const final override { return source; };
	virtual const char *sourceSignalName() const override = 0;
	virtual uint64_t sourceBounds() const = 0;

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceChanged() const;
	virtual void onSourceFixedChanged(const obs_source_t *obs_source) const = 0;

private:
	void execute(const MMGMappingTest &test) const final override;
	void connectSignal(bool connect) final override { MMGSignal::connectMMGSignal(this, connect); };
	void processEvent(const calldata_t *cd) const final override;

protected:
	virtual void execute(const MMGMappingTest &test, obs_source_t *obs_source) const = 0;
	virtual void processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const = 0;

private:
	MMGStringID source;

	mutable const char *current_uuid;
};

class MMGActionSourcesAudioVolume : public MMGActionSources {
	Q_OBJECT

public:
	MMGActionSourcesAudioVolume(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1301); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Volume"; };

	const char *sourceSignalName() const override { return "volume"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_AUDIO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_source_t *obs_source) const override;
	void onFormatChanged() const;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const override;

private:
	float convertIfToDecibels(float pct) const { return format == "dB" ? 20.0 * std::log10(pct) : pct * 100.0; };
	float convertIfFromDecibels(float db) const { return format == "dB" ? std::pow(10, db / 20.0) : db / 100.0; };

private:
	MMGStringID format;
	MMGFloat volume;

	static MMGParams<MMGString> format_params;
	static MMGParams<float> volume_params;
};
MMG_DECLARE_ACTION(MMGActionSourcesAudioVolume);

class MMGActionSourcesAudioMute : public MMGActionSources {
public:
	MMGActionSourcesAudioMute(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1302); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "MuteStatus"; };

	const char *sourceSignalName() const override { return "mute"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_AUDIO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_source_t *obs_source) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const override;

private:
	MMGBoolean mute;

	static MMGParams<bool> mute_params;
};
MMG_DECLARE_ACTION(MMGActionSourcesAudioMute);

class MMGActionSourcesAudioSyncOffset : public MMGActionSources {
public:
	MMGActionSourcesAudioSyncOffset(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1351); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "AudioOffset"; };

	const char *sourceSignalName() const override { return "audio_sync"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_AUDIO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_source_t *obs_source) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const override;

private:
	static int64_t getMillisecondOffset(const obs_source_t *obs_source)
	{
		return obs_source_get_sync_offset(obs_source) / 1000000LL;
	};

private:
	MMGValue<int64_t> offset;

	static MMGParams<int64_t> offset_params;
};
MMG_DECLARE_ACTION(MMGActionSourcesAudioSyncOffset);

class MMGActionSourcesAudioMonitor : public MMGActionSources {
public:
	MMGActionSourcesAudioMonitor(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x1352); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "AudioMonitor"; };

	const char *sourceSignalName() const override { return "audio_monitoring"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_AUDIO; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_source_t *obs_source) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const override;

private:
	MMGValue<obs_monitoring_type> monitor;

	static MMGParams<obs_monitoring_type> monitor_params;
};
MMG_DECLARE_ACTION(MMGActionSourcesAudioMonitor);

class MMGActionSourcesMediaState : public MMGActionSources {
public:
	MMGActionSourcesMediaState(MMGActionManager *parent, const QJsonObject &json_obj);

	enum Actions {
		PLAY,
		PAUSE,
		RESTART,
		STOP,
		TRACK_FORWARD,
		TRACK_BACKWARD,
	};

	static constexpr Id actionId() { return Id(0x13a1); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "MediaState"; };

	const char *sourceSignalName() const override;
	uint64_t sourceBounds() const override { return OBS_SOURCE_CONTROLLABLE_MEDIA; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_source_t *obs_source) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(MMGMappingTest &, const obs_source_t *) const override {};

private:
	MMGValue<Actions> media_state;

	static MMGParams<Actions> media_state_params;
};
MMG_DECLARE_ACTION(MMGActionSourcesMediaState);

class MMGActionSourcesMediaTime : public MMGActionSources {
public:
	MMGActionSourcesMediaTime(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x13a2); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Input.MediaTime"; };

	const char *sourceSignalName() const override { return "n/a"; };
	uint64_t sourceBounds() const override { return OBS_SOURCE_CONTROLLABLE_MEDIA; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_source_t *obs_source) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(MMGMappingTest &, const obs_source_t *) const override {};

private:
	MMGValue<int64_t> time;

	static MMGParams<int64_t> time_params;
};
MMG_DECLARE_ACTION(MMGActionSourcesMediaTime);

class MMGActionSourcesCustom : public MMGActionSources {
public:
	MMGActionSourcesCustom(MMGActionManager *parent, const QJsonObject &json_obj);

	static constexpr Id actionId() { return Id(0x13ff); };
	constexpr Id id() const final override { return actionId(); };
	const char *trActionName() const override { return "Custom"; };

	const char *sourceSignalName() const override { return "update"; };
	uint64_t sourceBounds() const override { return -1; };

	void initOldData(const QJsonObject &json_obj) override;
	void json(QJsonObject &json_obj) const override;
	void copy(MMGAction *dest) const override;

	void createDisplay(MMGWidgets::MMGActionDisplay *display) override;
	void onSourceFixedChanged(const obs_source_t *obs_source) const override;

private:
	void execute(const MMGMappingTest &test, obs_source_t *obs_source) const override;
	void processEvent(MMGMappingTest &test, const obs_source_t *obs_source) const override;

private:
	MMGOBSFields::MMGOBSObject *custom_data;
};
MMG_DECLARE_ACTION(MMGActionSourcesCustom);

} // namespace MMGActions
