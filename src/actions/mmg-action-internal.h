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

#pragma once
#include "mmg-action.h"

#include <QStackedWidget>
#include <QPushButton>
#include <QThread>
#include <mutex>

#define MMG_ENABLED if (editable)

struct MMGActionManager {
  public:
  MMGActionManager();
  MMGActionManager(const QJsonObject &json_obj);

  const QString &binding() const { return binding_name; };
  short timing() const { return before_action; };
  MMGUtils::MMGNumber *time() { return &_time; };

  void setBinding(const QString &name) { MMG_ENABLED binding_name = name; };
  void setTiming(short timing) { MMG_ENABLED before_action = timing; };

  void json(QJsonObject &json_obj);
  void copy(MMGActionManager *other) const;
  void setEditable(bool edit)
  {
    editable = edit;
    _time.set_edit(edit);
  };

  private:
  QString binding_name;
  short before_action = 0;
  MMGUtils::MMGNumber _time;

  bool editable = true;
};

class MMGInternalThread : public QThread {
  Q_OBJECT

  public:
  MMGInternalThread(){};
  MMGInternalThread(const QJsonObject &);
  ~MMGInternalThread()
  {
    if (locked) mutex.unlock();
    qDeleteAll(actions);
  }

  void json(QJsonObject &) const;
  void copy(MMGInternalThread *) const;
  void setEditable(bool edit);

  void run() override;
  void restart(const MMGMessage *msg);
  void createNew(const MMGMessage *msg);

  static short thread_count;

  private:
  std::timed_mutex mutex;
  bool locked = false;

  QList<MMGActionManager *> actions;
  MMGMessage message;

  friend class MMGActionInternalDisplay;
};

class MMGActionInternalDisplay : public QWidget {
  Q_OBJECT

  public:
  MMGActionInternalDisplay(QWidget *parent, MMGInternalThread *storage);

  void setOptions(const QStringList &options);

  public slots:
  void addPage();

  private slots:
  void setPageIndex();
  void setMode(int);
  void setString(const QString &str);
  void deletePage();

  private:
  MMGInternalThread *thread;
  MMGActionManager *currentManager() const { return thread->actions[num_display_storage - 1]; };

  QWidget *action_options_widget;
  QComboBox *action_options;
  MMGNumberDisplay *time;
  QWidget *binding_options_widget;
  QComboBox *binding_options;

  MMGNumberDisplay *num_display;
  MMGUtils::MMGNumber num_display_storage;
  QPushButton *add_action;
  QPushButton *remove_action;
};

class MMGActionInternal : public MMGAction {
  public:
  explicit MMGActionInternal();
  explicit MMGActionInternal(const QJsonObject &json_obj);
  ~MMGActionInternal() { delete thread; };
  enum Actions { INTERNAL_DOACTIONS };

  void blog(int log_status, const QString &message) const override;
  void execute(const MMGMessage *midi) const override;
  void json(QJsonObject &json_obj) const override;
  void copy(MMGAction *dest) const override;
  void setEditable(bool edit) override;
  void createDisplay(QWidget *parent) override;
  void setSubOptions(QComboBox *sub) override;

  Category category() const override { return Category::MMGACTION_INTERNAL; }

  static const QStringList enumerateActions();

  private:
  MMGInternalThread *thread;
  MMGActionInternalDisplay *internal_display;

  void setSubConfig() override;
};

#undef MMG_ENABLED
