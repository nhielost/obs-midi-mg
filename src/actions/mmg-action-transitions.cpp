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

#include "mmg-action-transitions.h"
#include "mmg-action-scenes.h"

namespace MMGActions {

static MMGParams<MMGString> transition_params {
	.desc = obstr("Transition"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE,
	.default_value = "",
	.bounds = {},
};

const MMGStringTranslationMap enumerateTransitions()
{
	MMGStringTranslationMap list;
	obs_frontend_source_list transition_list = {0};

	obs_frontend_get_transitions(&transition_list);
	for (size_t i = 0; i < transition_list.sources.num; ++i) {
		obs_source_t *transition = transition_list.sources.array[i];
		list.insert(obs_source_get_name(transition), nontr(obs_source_get_name(transition)));
	}

	obs_frontend_source_list_free(&transition_list);
	return list;
}

const MMGString findTransitionId(const MMGString &name)
{
	if (name.isEmpty()) return "";

	MMGString uuid;
	obs_frontend_source_list transition_list = {0};

	obs_frontend_get_transitions(&transition_list);
	for (size_t i = 0; i < transition_list.sources.num; ++i) {
		obs_source_t *transition = transition_list.sources.array[i];
		if (obs_source_get_name(transition) != name) continue;

		uuid = obs_source_get_uuid(transition);
		break;
	}

	obs_frontend_source_list_free(&transition_list);
	return uuid;
}

MMGString currentTransition()
{
	return obs_source_get_name(OBSSourceAutoRelease(obs_frontend_get_current_transition()));
}

// MMGActionTransitionsCurrent
MMGParams<int32_t> MMGActionTransitionsCurrent::duration_params {
	.desc = obstr("Basic.TransitionDuration"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE,
	.default_value = 0,
	.lower_bound = 25.0,
	.upper_bound = 20000.0,
	.step = 5.0,
};

MMGActionTransitionsCurrent::MMGActionTransitionsCurrent(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  transition(json_obj, "transition"),
	  duration(json_obj, "duration")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionTransitionsCurrent::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(transition, json_obj, "transition", 1, enumerateTransitions());
	MMGCompatibility::initOldNumberData(duration, json_obj, "num", 1);
}

void MMGActionTransitionsCurrent::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	transition->json(json_obj, "transition");
	duration->json(json_obj, "duration");
}

void MMGActionTransitionsCurrent::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionTransitionsCurrent *>(dest);
	if (!casted) return;

	transition.copy(casted->transition);
	duration.copy(casted->duration);
}

void MMGActionTransitionsCurrent::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	transition_params.options.setFlag(OPTION_ALLOW_MIDI, true);
	transition_params.options.setFlag(OPTION_ALLOW_TOGGLE, true);
	transition_params.options.setFlag(OPTION_ALLOW_IGNORE, type() == TYPE_OUTPUT);

	transition_params.bounds = enumerateTransitions();
	transition_params.default_value = currentTransition();

	MMGActions::createActionField(display, &transition, &transition_params,
				      std::bind(&MMGActionTransitionsCurrent::onTransitionChanged, this));
	MMGActions::createActionField(display, &duration, &duration_params);
}

void MMGActionTransitionsCurrent::onTransitionChanged() const
{
	if (transition->state() == STATE_FIXED)
		duration_params.options.setFlag(OPTION_HIDDEN,
						obs_transition_fixed(OBSSourceAutoRelease(
							obs_get_source_by_uuid(findTransitionId(transition)))));

	duration_params.default_value = obs_frontend_get_transition_duration();
}

void MMGActionTransitionsCurrent::execute(const MMGMappingTest &test) const
{
	MMGString transition_name;
	ACTION_ASSERT(test.applicable(transition, transition_name),
		      "A transition could not be selected. Check the Transition "
		      "field and try again.");

	OBSSourceAutoRelease obs_transition = obs_get_source_by_uuid(findTransitionId(transition_name));
	ACTION_ASSERT(obs_transition, "This transition does not exist.");

	if (!obs_transition_fixed(obs_transition)) {
		int32_t current_duration = obs_frontend_get_transition_duration();
		ACTION_ASSERT(test.applicable(duration, current_duration),
			      "A transition duration could not be selected. Check the "
			      "Duration field and try again.");
		obs_frontend_set_transition_duration(current_duration);
	}
	obs_frontend_set_current_transition(obs_transition);
}

void MMGActionTransitionsCurrent::processEvent(obs_frontend_event event) const
{
	if (event == OBS_FRONTEND_EVENT_TRANSITION_CHANGED) {
		EventFulfillment fulfiller(this);
		fulfiller->addCondition(transition->state() != STATE_IGNORE);
		fulfiller->addAcceptable(transition, currentTransition());
		fulfiller->addAcceptable(duration, obs_frontend_get_transition_duration());
	} else if (event == OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED) {
		EventFulfillment fulfiller(this);
		fulfiller->addCondition(duration->state() != STATE_IGNORE);
		fulfiller->addAcceptable(transition, currentTransition());
		fulfiller->addAcceptable(duration, obs_frontend_get_transition_duration());
	}
}
// End MMGActionTransitionsCurrent

// MMGActionTransitionsTBar
MMGParams<int32_t> MMGActionTransitionsTBar::tbar_params {
	.desc = obstr("Basic.TransformWindow.Position"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_RANGE | OPTION_ALLOW_TOGGLE,
	.default_value = 0,
	.lower_bound = 0.0,
	.upper_bound = 1024.0,
	.step = 1.0,
	.incremental_bound = 256.0,
};

MMGParams<int32_t> MMGActionTransitionsTBar::held_duration_params {
	.desc = mmgtr("Actions.Transitions.HeldDuration"),
	.options = OPTION_NONE,
	.default_value = 1000,
	.lower_bound = 10.0,
	.upper_bound = 5000.0,
	.step = 10.0,
	.incremental_bound = 0.0,
};

MMGActionTransitionsTBar::Timer MMGActionTransitionsTBar::tbar_timer;

MMGActionTransitionsTBar::MMGActionTransitionsTBar(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  tbar(json_obj, "tbar"),
	  held_duration(json_obj, "held_duration")
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionTransitionsTBar::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldNumberData(tbar, json_obj, "num", 1);
	held_duration = 1000;
}

void MMGActionTransitionsTBar::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	tbar->json(json_obj, "tbar");
	held_duration->json(json_obj, "held_duration");
}

void MMGActionTransitionsTBar::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionTransitionsTBar *>(dest);
	if (!casted) return;

	tbar.copy(casted->tbar);
	held_duration.copy(casted->held_duration);
}

void MMGActionTransitionsTBar::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGActions::createActionField(display, &tbar, &tbar_params);
	if (type() != TYPE_OUTPUT) MMGActions::createActionField(display, &held_duration, &held_duration_params);
}

void MMGActionTransitionsTBar::execute(const MMGMappingTest &test) const
{
	ACTION_ASSERT(obs_frontend_preview_program_mode_active(), "Studio mode is inactive.");

	int32_t tbar_dst = 0;
	ACTION_ASSERT(test.applicable(tbar, tbar_dst), "A transition bar distance could not be selected. Check the "
						       "Position field and try again.");

	runInMainThread([=, this]() {
		tbar_timer.restart(held_duration);
		obs_frontend_set_tbar_position(tbar_dst);
	});
}

void MMGActionTransitionsTBar::processEvent(obs_frontend_event event) const
{
	if (event != OBS_FRONTEND_EVENT_TBAR_VALUE_CHANGED) return;

	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(tbar, obs_frontend_get_tbar_position());
}
// End MMGActionTransitionsTBar

// MMGActionTransitionsCustom
MMGActionTransitionsCustom::MMGActionTransitionsCustom(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  transition(json_obj, "transition"),
	  custom_data(new MMGOBSFields::MMGOBSObject(this, sourceFromName(), json_obj))
{
	blog(LOG_DEBUG, "Action created.");
}

void MMGActionTransitionsCustom::initOldData(const QJsonObject &json_obj)
{
	MMGCompatibility::initOldStringData(transition, json_obj, "transition", 1, enumerateTransitions());
	custom_data->changeSource(sourceFromName(), json_obj);
}

void MMGActionTransitionsCustom::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	transition->json(json_obj, "transition");
	custom_data->json(json_obj);
}

void MMGActionTransitionsCustom::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionTransitionsCustom *>(dest);
	if (!casted) return;

	transition.copy(casted->transition);
	custom_data->copy(casted->custom_data);
}

void MMGActionTransitionsCustom::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	transition_params.options.setFlag(OPTION_ALLOW_MIDI, false);
	transition_params.options.setFlag(OPTION_ALLOW_TOGGLE, false);
	transition_params.options.setFlag(OPTION_ALLOW_IGNORE, false);

	transition_params.bounds = enumerateTransitions();
	transition_params.default_value = currentTransition();

	MMGActions::createActionField(display, &transition, &transition_params,
				      std::bind(&MMGActionTransitionsCustom::onTransitionChanged, this));
	custom_data->createDisplay(display);
}

void MMGActionTransitionsCustom::onTransitionChanged() const
{
	emit refreshRequested();
	custom_data->changeSource(sourceFromName());
}

void MMGActionTransitionsCustom::execute(const MMGMappingTest &test) const
{
	custom_data->execute(test);
}

void MMGActionTransitionsCustom::processEvent(const calldata_t *) const
{
	EventFulfillment fulfiller(this);
	custom_data->processEvent(*fulfiller);
}
// End MMGActionTransitionsCustom

} // namespace MMGActions
