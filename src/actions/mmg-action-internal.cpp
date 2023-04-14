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

#include "mmg-action-internal.h"
#include "../mmg-config.h"

#include <thread>
#include <QThread>

using namespace MMGUtils;

short MMGInternalThread::thread_count = 0;

// MMGActionManager
MMGActionManager::MMGActionManager()
{
  _time.set_num(1);
  _time.set_min(1);
  _time.set_max(999);
}

MMGActionManager::MMGActionManager(const QJsonObject &json_obj) : MMGActionManager()
{
  binding_name = json_obj["action"].toString();
  before_action = json_obj["pre"].toInt();
  _time = json_obj["time"].toInt();
}

void MMGActionManager::json(QJsonObject &json_obj)
{
  json_obj["action"] = binding_name;
  json_obj["pre"] = before_action;
  json_obj["time"] = (int)_time;
};

void MMGActionManager::copy(MMGActionManager *other) const
{
  other->before_action = before_action;
  other->binding_name = binding_name;
  other->_time = _time.copy();
}
// End MMGActionManager

// MMGInternalThread
MMGInternalThread::MMGInternalThread(const QJsonObject &json_obj)
{
  if (json_obj.contains("str1")) {
    for (int i = 0; i < 3; ++i) {
      QString name = json_obj[num_to_str(i + 1, "str")].toString();
      if (name.isEmpty()) continue;
      actions.append(new MMGActionManager);
      actions.last()->setBinding(name);
    }
  } else if (json_obj.contains("action1")) {
    for (int i = 0; i < 3; ++i) {
      QString name = json_obj[num_to_str(i + 1, "action")].toString();
      if (name.isEmpty()) continue;
      actions.append(new MMGActionManager);
      actions.last()->setBinding(name);
    }
  } else if (json_obj["actions"].isArray()) {
    for (const QJsonValue &manager : json_obj["actions"].toArray())
      actions.append(new MMGActionManager(manager.toObject()));
  }
}

void MMGInternalThread::json(QJsonObject &json_obj) const
{
  QJsonArray json_array;
  for (MMGActionManager *manager : actions) {
    QJsonObject json_obj;
    manager->json(json_obj);
    json_array += json_obj;
  }
  json_obj["actions"] = json_array;
}

void MMGInternalThread::copy(MMGInternalThread *other) const
{
  other->actions.clear();
  for (MMGActionManager *manager : actions) {
    other->actions.append(new MMGActionManager);
    manager->copy(other->actions.last());
  }
}

void MMGInternalThread::setEditable(bool edit)
{
  for (MMGActionManager *manager : actions)
    manager->setEditable(edit);
}

void MMGInternalThread::run()
{
  thread_count++;
  locked = true;
  mutex.lock();

  MMGDevice *device = global()->currentDevice();
  if (!device) goto cleanup;

  for (MMGActionManager *manager : actions) {
    const MMGBinding *binding = device->find(manager->binding());
    if (!binding) continue;

    if (binding->action()->category() == MMGAction::MMGACTION_INTERNAL) {
      binding->action()->blog(LOG_INFO, "FAILED: Cannot execute another [Internal] action.");
      continue;
    }

    switch (manager->timing()) {
      case 1: // Milliseconds
	if (mutex.try_lock_for((std::chrono::milliseconds)(int)*manager->time())) goto cleanup;
	break;
      case 2: // Seconds
	if (mutex.try_lock_for((std::chrono::seconds)(int)*manager->time())) goto cleanup;
	break;
      case 0: // As soon as possible
	break;
      default:
	continue;
    }

    binding->action()->execute(&message);
  }

cleanup:
  mutex.unlock();
  locked = false;
  thread_count--;
}

void MMGInternalThread::restart(const MMGMessage *msg)
{
  if (locked) {
    mutex.unlock();
    wait();
  }
  message = *msg;
  start();
}

void MMGInternalThread::createNew(const MMGMessage *msg)
{
  if (thread_count >= 64) {
    global_blog(LOG_INFO, "Thread count exceeded - the provided function will not execute.");
    return;
  }

  auto *custom_thread = new MMGInternalThread;
  copy(custom_thread);
  custom_thread->connect(custom_thread, &QThread::finished, &QObject::deleteLater);
  custom_thread->restart(msg);
}
// End MMGInternalThread

MMGActionInternal::MMGActionInternal()
{
  thread = new MMGInternalThread;
  blog(LOG_DEBUG, "Empty action created.");
};

MMGActionInternal::MMGActionInternal(const QJsonObject &json_obj)
{
  subcategory = 0;
  thread = new MMGInternalThread(json_obj);

  blog(LOG_DEBUG, "Action created.");
}

void MMGActionInternal::blog(int log_status, const QString &message) const
{
  MMGAction::blog(log_status, "[Internal] " + message);
}

void MMGActionInternal::json(QJsonObject &json_obj) const
{
  MMGAction::json(json_obj);

  thread->json(json_obj);
}

void MMGActionInternal::execute(const MMGMessage *midi) const
{
  switch (sub()) {
    case INTERNAL_DOACTIONS:
      switch (global()->preferences()->internalBehavior()) {
	case 0: // RESET
	  thread->restart(midi);
	  break;
	case 1: // DON'T RESET
	  thread->createNew(midi);
	  break;
	default:
	  break;
      }
      break;

    default:
      blog(LOG_DEBUG, "Successfully executed.");
      break;
  }
}

void MMGActionInternal::copy(MMGAction *dest) const
{
  MMGAction::copy(dest);

  auto casted = dynamic_cast<MMGActionInternal *>(dest);
  if (!casted) return;

  thread->copy(casted->thread);
}

void MMGActionInternal::setEditable(bool edit)
{
  thread->setEditable(edit);
}

const QStringList MMGActionInternal::enumerateActions()
{
  QStringList list;
  for (MMGBinding *const binding : global()->currentDevice()->bindings()) {
    if (binding->action()->category() != MMGACTION_INTERNAL) list.append(binding->name());
  }
  return list;
}

void MMGActionInternal::createDisplay(QWidget *parent)
{
  MMGAction::createDisplay(parent);

  internal_display = new MMGActionInternalDisplay(_display, thread);
  internal_display->resize(290, 350);
  _display->setScrollWidget(internal_display);
}

void MMGActionInternal::setSubOptions(QComboBox *sub)
{
  sub->addItem(mmgtr("Actions.Internal.Sub.ExecuteOther"));
}

void MMGActionInternal::setSubConfig()
{
  internal_display->setVisible(sub() == 0);
  internal_display->setOptions(enumerateActions());
}

// MMGActionInternalDisplay
MMGActionInternalDisplay::MMGActionInternalDisplay(QWidget *parent, MMGInternalThread *storage)
  : QWidget(parent)
{
  thread = storage;
  num_display_storage = 1;

  if (thread->actions.size() < 1) thread->actions.append(new MMGActionManager);

  QWidget *widget = new QWidget(this);
  widget->setGeometry(0, 0, 290, 250);
  widget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  QVBoxLayout *custom_layout = new QVBoxLayout(widget);
  custom_layout->setSpacing(10);
  custom_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  custom_layout->setContentsMargins(10, 10, 10, 10);
  widget->setLayout(custom_layout);

  binding_options_widget = new QWidget(widget);
  binding_options_widget->setFixedSize(270, 70);

  QLabel *binding_options_label = new QLabel(binding_options_widget);
  binding_options_label->setGeometry(0, 0, 270, 25);
  binding_options_label->setText(mmgtr("Actions.Internal.Binding"));

  binding_options = new QComboBox(binding_options_widget);
  binding_options->setGeometry(0, 25, 270, 40);
  connect(binding_options, &QComboBox::currentTextChanged, this,
	  &MMGActionInternalDisplay::setString);

  widget->layout()->addWidget(binding_options_widget);

  action_options_widget = new QWidget(widget);
  action_options_widget->setFixedSize(270, 70);

  QLabel *action_options_label = new QLabel(action_options_widget);
  action_options_label->setGeometry(0, 0, 270, 25);
  action_options_label->setText(mmgtr("Actions.Internal.Timing.Text"));

  action_options = new QComboBox(action_options_widget);
  action_options->setGeometry(0, 25, 270, 40);
  action_options->addItems(mmgtr_all("Actions.Internal.Timing", {"ASAP", "WaitMS", "WaitS"}));
  connect(action_options, &QComboBox::currentIndexChanged, this,
	  &MMGActionInternalDisplay::setMode);

  widget->layout()->addWidget(action_options_widget);

  time = new MMGNumberDisplay(widget);
  time->setVisible(false);
  time->resize(270, 70);
  time->setStorage(currentManager()->time(), true);
  time->setOptions(MMGNumberDisplay::OPTIONS_FIXED_ONLY);
  time->setBounds(1.0, 999.0);
  time->setStep(1.0);
  time->setDefaultValue(1.0);
  widget->layout()->addWidget(time);

  QFrame *separator = new QFrame(this);
  separator->setGeometry(10, 250, 270, 1);
  separator->setFrameShape(QFrame::HLine);
  separator->setLineWidth(1);

  num_display = new MMGNumberDisplay(this);
  num_display->setDisplayMode(MMGNumberDisplay::MODE_THIN);
  num_display->move(10, 260);
  num_display->setDescription(mmgtr("Actions.Internal.Number"));
  num_display->setStorage(&num_display_storage);
  num_display->setBounds(1, thread->actions.size());
  connect(num_display, &MMGNumberDisplay::numberChanged, this,
	  &MMGActionInternalDisplay::setPageIndex);

  add_action = new QPushButton(this);
  add_action->setText(mmgtr("Actions.Internal.AddAction"));
  add_action->setGeometry(10, 300, 130, 40);
  connect(add_action, &QPushButton::clicked, this, &MMGActionInternalDisplay::addPage);

  remove_action = new QPushButton(this);
  remove_action->setText(mmgtr("Actions.Internal.RemoveAction"));
  remove_action->setGeometry(150, 300, 130, 40);
  connect(remove_action, &QPushButton::clicked, this, &MMGActionInternalDisplay::deletePage);

  setPageIndex();
}

void MMGActionInternalDisplay::setPageIndex()
{
  if (thread->actions.size() < num_display_storage) return;
  MMGActionManager *current = currentManager();
  action_options->setCurrentIndex(current->timing());
  time->setStorage(current->time(), true);
  binding_options->setCurrentText(current->binding());
  remove_action->setEnabled(thread->actions.size() != 1);
}

void MMGActionInternalDisplay::setMode(int index)
{
  if (index > 2) return;
  currentManager()->setTiming(index);
  time->setVisible(index != 0);
  QString desc = "Actions.Internal.Timing.";
  desc += (index < 2 ? "Milliseconds" : "Seconds");
  time->setDescription(mmgtr(desc.qtocs()));
}

void MMGActionInternalDisplay::setOptions(const QStringList &options)
{
  QString name = currentManager()->binding();
  QSignalBlocker blocker(binding_options);

  binding_options->clear();
  binding_options->addItems(options);
  if (options.contains(name)) binding_options->setCurrentText(name);
  currentManager()->setBinding(binding_options->currentText());
}

void MMGActionInternalDisplay::addPage()
{
  MMGActionManager *manager = new MMGActionManager;
  manager->setBinding(binding_options->itemText(0));
  thread->actions.insert(num_display_storage, manager);

  num_display_storage = num_display_storage + 1;
  num_display->setBounds(1, thread->actions.size());
  emit num_display->numberChanged();
}

void MMGActionInternalDisplay::setString(const QString &str)
{
  currentManager()->setBinding(str);
}

void MMGActionInternalDisplay::deletePage()
{
  MMGActionManager *manager = currentManager();
  thread->actions.removeAt(num_display_storage - 1);
  delete manager;

  num_display->setBounds(1, thread->actions.size());
  emit num_display->numberChanged();
}
// End MMGActionInternalDisplay
