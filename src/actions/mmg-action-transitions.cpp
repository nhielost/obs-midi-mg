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

#include "mmg-action-transitions.h"
#include "mmg-action-scenes.h"

using namespace MMGUtils;

static struct MMGTBarTimer {
	MMGTBarTimer()
	{
		timer->connect(timer, &MMGTimer::stopping, [&]() { obs_frontend_release_tbar(); });
	}
	~MMGTBarTimer() { delete timer; };

	MMGTimer *timer = new MMGTimer;
	bool available = true;
} tbar_timer;

// MMGActionTransitions
MMGActionTransitions::MMGActionTransitions(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  transition(json_obj, "transition", 1),
	  parent_scene(json_obj, "scene", 2),
	  source(json_obj, "source", 3),
	  num(json_obj, "num", 1),
	  _json(new MMGJsonObject(this))
{
	if (json_obj["json_str"].isString()) {
		_json->setJson(json_from_str(json_obj["json_str"].toString().qtocs()));
	} else {
		_json->setJson(json_obj["json"].toObject());
	}
	blog(LOG_DEBUG, "Action created.");
}

const QStringList MMGActionTransitions::subNames() const
{
	QStringList opts;

	switch (type()) {
		case TYPE_INPUT:
		default:
			opts << subModuleTextList(
				{"CurrentChange", "SourceShow", "SourceHide", "TBarChange", "TBarToggle"});
			break;

		case TYPE_OUTPUT:
			opts << subModuleTextList({"CurrentChange", "DurationChange", "Started", "Stopped",
						   "ToggleStarted", "TBarChange"});
			break;
	}

	opts << subModuleText("Custom");
	return opts;
}

void MMGActionTransitions::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	transition.json(json_obj, "transition", false);
	parent_scene.json(json_obj, "scene", false);
	source.json(json_obj, "source", false);
	num.json(json_obj, "num");
	json_obj["json"] = _json->json();
}

void MMGActionTransitions::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionTransitions *>(dest);
	if (!casted) return;

	casted->transition = transition.copy();
	casted->parent_scene = parent_scene.copy();
	casted->source = source.copy();
	casted->num = num.copy();
	casted->_json->setJson(_json->json());
}

void MMGActionTransitions::setEditable(bool edit)
{
	transition.setEditable(edit);
	parent_scene.setEditable(edit);
	source.setEditable(edit);
	num.setEditable(edit);
}

void MMGActionTransitions::toggle()
{
	num.toggle();
}

void MMGActionTransitions::createDisplay(QWidget *parent)
{
	MMGAction::createDisplay(parent);

	connect(display()->addNew(&transition), &MMGStringDisplay::stringChanged, this,
		&MMGActionTransitions::onList1Change);
	connect(display()->addNew(&parent_scene), &MMGStringDisplay::stringChanged, this,
		&MMGActionTransitions::onList2Change);
	connect(display()->addNew(&source), &MMGStringDisplay::stringChanged, this,
		&MMGActionTransitions::onList3Change);

	display()->addNew(&num);
}

void MMGActionTransitions::setActionParams()
{
	display()->hideAll();

	MMGNumberDisplay *num_display = display()->numberDisplay(0);
	num_display->setVisible(false);

	if (type() != TYPE_OUTPUT) {
		switch (sub()) {
			case TRANSITION_TBAR_ACTIVATE:
				display()->reset();

				num_display->setVisible(true);
				num_display->setDescription(obstr("Basic.TransformWindow.Position"));
				num_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM);
				num_display->setBounds(0.0, 1024.0);
				num_display->setStep(1.0);
				num_display->setDefaultValue(0.0);
				num_display->reset();
				return;

			case TRANSITION_TBAR_TOGGLE:
				display()->reset();
				return;

			default:
				break;
		}
	} else {
		switch (sub()) {
			case TRANSITION_CURRENT_DURATION_CHANGED:
				num_display->setVisible(!transitionFixed());
				num_display->setDescription(obstr("Basic.TransitionDuration"));
				num_display->setOptions(MIDIBUTTON_IGNORE | MIDIBUTTON_TOGGLE);
				num_display->setBounds(25.0, 20000.0);
				num_display->setStep(25.0);
				num_display->setDefaultValue(obs_frontend_get_transition_duration());
				num_display->reset();
				return;

			case TRANSITION_TBAR_CHANGED:
				num_display->setVisible(true);
				num_display->setDescription(obstr("Basic.TransformWindow.Position"));
				num_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM);
				num_display->setBounds(0.0, 1024.0);
				num_display->setStep(1.0);
				num_display->setDefaultValue(0.0);
				num_display->reset();
				return;

			default:
				break;
		}
	}

	MMGStringDisplay *transition_display = display()->stringDisplay(0);
	transition_display->setVisible(true);
	transition_display->setDescription(obstr("Transition"));
	transition_display->setBounds(enumerate());
}

void MMGActionTransitions::onList1Change()
{
	connectSignals(true);

	display()->reset();

	MMGStringDisplay *scene_display = display()->stringDisplay(1);
	MMGNumberDisplay *num_display = display()->numberDisplay(0);
	num_display->setVisible(false);

	if (type() != TYPE_INPUT) return;

	MMGActionFieldRequest req;

	switch (sub()) {
		case TRANSITION_CURRENT:
			num_display->setVisible(!transitionFixed());
			num_display->setDescription(obstr("Basic.TransitionDuration"));
			num_display->setOptions(MIDIBUTTON_IGNORE | MIDIBUTTON_TOGGLE);
			num_display->setBounds(25.0, 20000.0);
			num_display->setStep(25.0);
			num_display->setDefaultValue(obs_frontend_get_transition_duration());
			num_display->reset();
			break;

		case TRANSITION_SOURCE_SHOW:
		case TRANSITION_SOURCE_HIDE:
			scene_display->setVisible(true);
			scene_display->setDescription(obstr("Basic.Scene"));
			scene_display->setBounds(MMGActionScenes::enumerate());
			break;

		case TRANSITION_CUSTOM:
			req.source = sourceByName(transition);
			if (!obs_source_configurable(req.source)) break;
			req.json = _json;
			display()->setCustomOBSFields(req);
			break;

		default:
			return;
	}
}

void MMGActionTransitions::onList2Change()
{
	MMGStringDisplay *source_display = display()->stringDisplay(2);

	switch (sub()) {
		case TRANSITION_SOURCE_SHOW:
		case TRANSITION_SOURCE_HIDE:
			source_display->setVisible(true);
			source_display->setDescription(obstr("Basic.Main.Source"));
			source_display->setBounds(MMGActionScenes::enumerateItems(parent_scene));
			break;

		default:
			return;
	}
}

void MMGActionTransitions::onList3Change()
{
	MMGNumberDisplay *num_display = display()->numberDisplay(0);

	switch (sub()) {
		case TRANSITION_SOURCE_SHOW:
		case TRANSITION_SOURCE_HIDE:
			num_display->setVisible(!transitionFixed() && !source.value().isEmpty());
			num_display->setDescription(obstr("Basic.TransitionDuration"));
			num_display->setOptions(MIDIBUTTON_IGNORE);
			num_display->setBounds(25.0, 20000.0);
			num_display->setStep(25.0);
			num_display->setDefaultValue(obs_frontend_get_transition_duration());
			num_display->reset();
			return;

		default:
			return;
	}
}

const QStringList MMGActionTransitions::enumerate()
{
	QStringList list;
	obs_frontend_source_list transition_list = {0};
	obs_frontend_get_transitions(&transition_list);
	for (size_t i = 0; i < transition_list.sources.num; ++i) {
		list.append(obs_source_get_name(transition_list.sources.array[i]));
	}
	obs_frontend_source_list_free(&transition_list);
	return list;
}

const QString MMGActionTransitions::currentTransition()
{
	return obs_source_get_name(OBSSourceAutoRelease(obs_frontend_get_current_transition()));
}

obs_source_t *MMGActionTransitions::sourceByName(const QString &name)
{
	obs_frontend_source_list transition_list = {0};
	obs_frontend_get_transitions(&transition_list);
	obs_source_t *source = nullptr;

	for (size_t i = 0; i < transition_list.sources.num; ++i) {
		if (obs_source_get_name(transition_list.sources.array[i]) == name) {
			source = obs_source_get_ref(transition_list.sources.array[i]);
			break;
		}
	}
	obs_frontend_source_list_free(&transition_list);
	return source;
}

bool MMGActionTransitions::transitionFixed() const
{
	return obs_transition_fixed(OBSSourceAutoRelease(sourceByName(transition)));
}

void MMGActionTransitions::execute(const MMGMessage *midi) const
{
	OBSSourceAutoRelease obs_transition = sourceByName(transition);
	if (sub() != 3 && sub() != 4) ACTION_ASSERT(obs_transition, "Transition does not exist.");

	int time = num.chooseFrom(midi, false, obs_frontend_get_transition_duration());
	bool fixed = obs_transition_fixed(obs_transition);

	OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
	OBSSceneAutoRelease obs_scene = obs_get_scene_by_name(parent_scene.mmgtocs());
	obs_sceneitem_t *obs_sceneitem = obs_scene_find_source_recursive(obs_scene, obs_source_get_name(obs_source));

	switch (sub()) {
		case TRANSITION_CURRENT:
			obs_frontend_set_current_transition(obs_transition);
			if (!fixed) obs_frontend_set_transition_duration(time);
			break;

		case TRANSITION_SOURCE_SHOW:
			ACTION_ASSERT(obs_sceneitem, "Source in scene does not exist.");

			obs_sceneitem_set_transition(obs_sceneitem, true, obs_transition);
			if (!fixed) obs_sceneitem_set_transition_duration(obs_sceneitem, true, time);
			break;

		case TRANSITION_SOURCE_HIDE:
			ACTION_ASSERT(obs_sceneitem, "Source in scene does not exist.");

			obs_sceneitem_set_transition(obs_sceneitem, false, obs_transition);
			if (!fixed) obs_sceneitem_set_transition_duration(obs_sceneitem, false, time);
			break;

		case TRANSITION_TBAR_ACTIVATE:
			ACTION_ASSERT(obs_frontend_preview_program_mode_active(), "Studio mode is inactive.");

			if (!tbar_timer.available) return;

			obs_frontend_set_tbar_position(time);
			tbar_timer.timer->reset(1000);
			break;

		case TRANSITION_TBAR_TOGGLE:
			ACTION_ASSERT(obs_frontend_preview_program_mode_active(), "Studio mode is inactive.");

			tbar_timer.available = !tbar_timer.available;
			if (!tbar_timer.available) tbar_timer.timer->stopTimer();
			break;

		case TRANSITION_CUSTOM:
			obs_source_custom_update(obs_transition, _json->json(), midi);
			break;

		default:
			break;
	}

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionTransitions::connectSignals(bool _connect)
{
	MMGAction::connectSignals(_connect);
	if (!_connected) return;

	connectSourceSignal(mmgsignals()->requestSourceSignal(OBSSourceAutoRelease(sourceByName(transition))));
}

void MMGActionTransitions::sourceEventReceived(MMGSourceSignal::Event event, QVariant data)
{
	OBSSourceAutoRelease obs_source;

	switch (sub()) {
		case TRANSITION_STARTED:
			if (event != MMGSourceSignal::SIGNAL_TRANSITION_START) return;
			if (transition != currentTransition()) return;
			break;

		case TRANSITION_STOPPED:
			if (event != MMGSourceSignal::SIGNAL_TRANSITION_STOP) return;
			if (transition != currentTransition()) return;
			break;

		case TRANSITION_TOGGLE_STARTED:
			if (event != MMGSourceSignal::SIGNAL_TRANSITION_START &&
			    event != MMGSourceSignal::SIGNAL_TRANSITION_STOP)
				return;
			if (transition != currentTransition()) return;
			break;

		case TRANSITION_CUSTOM_CHANGED:
			if (event != MMGSourceSignal::SIGNAL_UPDATE) return;

			obs_source = sourceByName(transition);
			if (data.value<void *>() != obs_source) return;

			triggerEvent(obs_source_custom_updated(obs_source, _json->json()));
			return;

		default:
			return;
	}

	triggerEvent();
}

void MMGActionTransitions::frontendEventReceived(obs_frontend_event event)
{
	MMGNumber values;

	switch (sub()) {
		case TRANSITION_CURRENT_CHANGED:
			if (event != OBS_FRONTEND_EVENT_TRANSITION_CHANGED) return;
			if (transition != currentTransition()) return;
			break;

		case TRANSITION_CURRENT_DURATION_CHANGED:
			if (event != OBS_FRONTEND_EVENT_TRANSITION_DURATION_CHANGED) return;
			if (num.state() != STATE_IGNORE && num != obs_frontend_get_transition_duration()) return;
			break;

		case TRANSITION_TBAR_CHANGED:
			if (event != OBS_FRONTEND_EVENT_TBAR_VALUE_CHANGED) return;
			if (!num.acceptable(obs_frontend_get_tbar_position())) return;
			values = num;
			values = obs_frontend_get_tbar_position();
			break;

		default:
			return;
	}

	triggerEvent({values});
}