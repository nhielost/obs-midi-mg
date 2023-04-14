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
} tbar_timer;

MMGActionTransitions::MMGActionTransitions()
{
  blog(LOG_DEBUG, "Empty action created.");
};

MMGActionTransitions::MMGActionTransitions(const QJsonObject &json_obj)
  : transition(json_obj, "transition", 1),
    parent_scene(json_obj, "scene", 2),
    source(json_obj, "source", 3),
    json_str(json_obj, "json_str", 0),
    num(json_obj, "num", 1)
{
  subcategory = json_obj["sub"].toInt();

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionTransitions::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Transitions] " + message);
}

void MMGActionTransitions::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  transition.json(json_obj, "transition", false);
  parent_scene.json(json_obj, "scene", false);
  source.json(json_obj, "source", false);
  json_str.json(json_obj, "json_str", false);
  num.json(json_obj, "num");
}

void MMGActionTransitions::execute(const MMGMessage *midi) const
{
  OBSSourceAutoRelease obs_transition = sourceByName();
  if (!obs_transition && !(sub() == 3 || sub() == 4)) {
    blog(LOG_INFO, "FAILED: Transition does not exist.");
    return;
  }

  int obs_transition_time = num.choose(midi, obs_frontend_get_transition_duration());
  bool fixed = obs_transition_fixed(obs_transition);

  OBSSourceAutoRelease obs_source = obs_get_source_by_name(source.mmgtocs());
  OBSSceneAutoRelease obs_scene = obs_get_scene_by_name(parent_scene.mmgtocs());
  OBSSceneItemAutoRelease obs_sceneitem = obs_scene_sceneitem_from_source(obs_scene, obs_source);

  switch (sub()) {
    case MMGActionTransitions::TRANSITION_CURRENT:
      obs_frontend_set_current_transition(obs_transition);
      if (!fixed) obs_frontend_set_transition_duration(obs_transition_time);
      break;
    case MMGActionTransitions::TRANSITION_SOURCE_SHOW:
      if (!obs_sceneitem) {
	blog(LOG_INFO, "FAILED: Source in scene does not exist.");
	return;
      }
      obs_sceneitem_set_transition(obs_sceneitem, true, obs_transition);
      if (!fixed) obs_sceneitem_set_transition_duration(obs_sceneitem, true, obs_transition_time);
      break;
    case MMGActionTransitions::TRANSITION_SOURCE_HIDE:
      if (!obs_sceneitem) {
	blog(LOG_INFO, "FAILED: Source in scene does not exist.");
	return;
      }
      obs_sceneitem_set_transition(obs_sceneitem, false, obs_transition);
      if (!fixed) obs_sceneitem_set_transition_duration(obs_sceneitem, false, obs_transition_time);
      break;
    case MMGActionTransitions::TRANSITION_TBAR_ACTIVATE:
      if (!obs_frontend_preview_program_mode_active()) {
	blog(LOG_INFO, "FAILED: Studio mode is inactive.");
	return;
      }
      obs_frontend_set_tbar_position((int)num.choose(midi));
      tbar_timer.timer->reset(1000);
      break;
    case MMGActionTransitions::TRANSITION_TBAR_RELEASE:
      if (!obs_frontend_preview_program_mode_active()) {
	blog(LOG_INFO, "FAILED: Studio mode is inactive.");
	return;
      }
      tbar_timer.timer->stopTimer();
      break;
    case MMGActionTransitions::TRANSITION_CUSTOM:
      obs_source_custom_update(obs_transition, json_from_str(json_str.mmgtocs()), midi);
      break;
    default:
      break;
  }
  blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionTransitions::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionTransitions *>(dest);
  if (!casted) return;

  casted->transition = transition.copy();
  casted->parent_scene = parent_scene.copy();
  casted->source = source.copy();
  casted->json_str = json_str.copy();
  casted->num = num.copy();
}

void MMGActionTransitions::setEditable(bool edit)
{
  transition.set_edit(edit);
  parent_scene.set_edit(edit);
  source.set_edit(edit);
  json_str.set_edit(edit);
  num.set_edit(edit);
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

void MMGActionTransitions::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  _display->setStr1Storage(&transition);
  _display->setStr2Storage(&parent_scene);
  _display->setStr3Storage(&source);

  MMGNumberDisplay *num_display = new MMGNumberDisplay(_display->numberDisplays());
  num_display->setStorage(&num, true);
  _display->numberDisplays()->add(num_display);

  _display->connect(_display, &MMGActionDisplay::str1Changed, [&]() { setList1Config(); });
  _display->connect(_display, &MMGActionDisplay::str2Changed, [&]() { setList2Config(); });
  _display->connect(_display, &MMGActionDisplay::str3Changed, [&]() { setList3Config(); });
}

void MMGActionTransitions::setSubOptions(QComboBox *sub)
{
  sub->addItems(mmgtr_all("Actions.Transitions.Sub", {"ChangeCurrent", "SourceShow", "SourceHide",
						      "SetTBar", "ReleaseTBar", "Custom"}));
}

void MMGActionTransitions::setSubConfig()
{
  _display->setStr1Visible(false);
  _display->setStr2Visible(false);
  _display->setStr3Visible(false);

  MMGNumberDisplay *num_display = _display->numberDisplays()->fieldAt(0);

  switch (subcategory) {
    case TRANSITION_TBAR_ACTIVATE:
      _display->resetScrollWidget();

      num_display->setVisible(true);
      num_display->setDescription(obstr("Basic.TransformWindow.Position"));
      num_display->setOptions(MMGNumberDisplay::OPTIONS_MIDI_CUSTOM);
      num_display->setBounds(0.0, 1024.0);
      num_display->setStep(1.0);
      num_display->setDefaultValue(0.0);
      num_display->reset();
      return;

    case TRANSITION_TBAR_RELEASE:
      _display->resetScrollWidget();
      num_display->setVisible(false);
      return;

    default:
      break;
  }

  _display->setStr1Visible(true);
  _display->setStr1Description(obstr("Transition"));
  _display->setStr1Options(enumerate());
}

void MMGActionTransitions::setList1Config()
{
  _display->resetScrollWidget();

  MMGNumberDisplay *num_display = _display->numberDisplays()->fieldAt(0);
  num_display->setVisible(false);

  OBSSourceAutoRelease source;

  switch ((Actions)subcategory) {
    case TRANSITION_CURRENT:
      num_display->setVisible(!transitionFixed());
      num_display->setDescription(obstr("Basic.TransitionDuration"));
      num_display->setOptions(MMGNumberDisplay::OPTIONS_IGNORE);
      num_display->setBounds(25.0, 20000.0);
      num_display->setStep(25.0);
      num_display->setDefaultValue(obs_frontend_get_transition_duration());
      num_display->reset();
      break;

    case TRANSITION_SOURCE_SHOW:
    case TRANSITION_SOURCE_HIDE:
      _display->setStr2Visible(true);
      _display->setStr2Description(obstr("Basic.Scene"));
      _display->setStr2Options(MMGActionScenes::enumerate());
      break;

    case TRANSITION_CUSTOM:
      source = sourceByName();
      if (!obs_source_configurable(source)) break;
      emit _display->customFieldRequest(source, &json_str);
      break;

    default:
      return;
  }
}

void MMGActionTransitions::setList2Config()
{
  switch ((Actions)subcategory) {
    case TRANSITION_SOURCE_SHOW:
    case TRANSITION_SOURCE_HIDE:
      _display->setStr3Visible(true);
      _display->setStr3Description(obstr("Basic.Main.Source"));
      _display->setStr3Options(MMGActionScenes::enumerate_items(parent_scene));
      break;

    default:
      return;
  }
}

void MMGActionTransitions::setList3Config()
{
  MMGNumberDisplay *num_display = _display->numberDisplays()->fieldAt(0);

  switch ((Actions)subcategory) {
    case TRANSITION_SOURCE_SHOW:
    case TRANSITION_SOURCE_HIDE:
      num_display->setVisible(!transitionFixed() && !source.str().isEmpty());
      num_display->setDescription(obstr("Basic.TransitionDuration"));
      num_display->setOptions(MMGNumberDisplay::OPTIONS_IGNORE);
      num_display->setBounds(25.0, 20000.0);
      num_display->setStep(25.0);
      num_display->setDefaultValue(obs_frontend_get_transition_duration());
      num_display->reset();
      return;

    default:
      return;
  }
}

obs_source_t *MMGActionTransitions::sourceByName() const
{
  obs_frontend_source_list transition_list = {0};
  obs_frontend_get_transitions(&transition_list);
  for (size_t i = 0; i < transition_list.sources.num; ++i) {
    obs_source_t *ptr = transition_list.sources.array[i];
    if (obs_source_get_name(ptr) == transition.str()) {
      obs_frontend_source_list_free(&transition_list);
      return obs_source_get_ref(ptr);
    }
  }
  obs_frontend_source_list_free(&transition_list);
  return nullptr;
}

bool MMGActionTransitions::transitionFixed() const
{
  return obs_transition_fixed(OBSSourceAutoRelease(sourceByName()));
}
