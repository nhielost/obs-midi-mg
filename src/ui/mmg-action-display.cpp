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

#include "mmg-action-display.h"
#include "mmg-fields.h"

using namespace MMGUtils;

MMGActionDisplay::MMGActionDisplay(QWidget *parent) : QWidget(parent)
{
  label_str1 = new QLabel(this);
  label_str2 = new QLabel(this);
  label_str3 = new QLabel(this);
  editor_str1 = new QComboBox(this);
  editor_str2 = new QComboBox(this);
  editor_str3 = new QComboBox(this);
  scroll_area = new QScrollArea(this);

  label_str1->setGeometry(10, 10, 220, 20);
  label_str2->setGeometry(10, 80, 220, 20);
  label_str3->setGeometry(10, 150, 220, 20);
  editor_str1->setGeometry(10, 30, 220, 40);
  editor_str2->setGeometry(10, 100, 220, 40);
  editor_str3->setGeometry(10, 170, 220, 40);
  scroll_area->setGeometry(240, 0, 290, 350);

  scroll_area->setWidgetResizable(true);
  scroll_area->setFrameShape(QFrame::NoFrame);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  default_scroll_widget = new MMGNumberDisplayFields(scroll_area);
  resetScrollWidget();

  connect(editor_str1, &QComboBox::currentTextChanged, this, &MMGActionDisplay::setStr1);
  connect(editor_str2, &QComboBox::currentTextChanged, this, &MMGActionDisplay::setStr2);
  connect(editor_str3, &QComboBox::currentTextChanged, this, &MMGActionDisplay::setStr3);

  //setVisible(true);
  setStr1Visible(false);
  setStr2Visible(false);
  setStr3Visible(false);
}

void MMGActionDisplay::setStr1(const QString &str)
{
  if (str1) {
    *str1 = str;
    if (str == mmgtr("Fields.Toggle"))
      str1->set_state(MMGString::STRINGSTATE_TOGGLE);
    else if (str == mmgtr("Fields.UseMessageValue"))
      str1->set_state(MMGString::STRINGSTATE_MIDI);
    else
      str1->set_state(MMGString::STRINGSTATE_FIXED);
  }
  emit str1Changed();
}

void MMGActionDisplay::setStr2(const QString &str)
{
  if (str2) {
    *str2 = str;
    if (str == mmgtr("Fields.Toggle"))
      str2->set_state(MMGString::STRINGSTATE_TOGGLE);
    else if (str == mmgtr("Fields.UseMessageValue"))
      str2->set_state(MMGString::STRINGSTATE_MIDI);
    else
      str2->set_state(MMGString::STRINGSTATE_FIXED);
  }
  emit str2Changed();
}

void MMGActionDisplay::setStr3(const QString &str)
{
  if (str3) {
    *str3 = str;
    if (str == mmgtr("Fields.Toggle"))
      str3->set_state(MMGString::STRINGSTATE_TOGGLE);
    else if (str == mmgtr("Fields.UseMessageValue"))
      str3->set_state(MMGString::STRINGSTATE_MIDI);
    else
      str3->set_state(MMGString::STRINGSTATE_FIXED);
  }
  emit str3Changed();
}

void MMGActionDisplay::setStr1Visible(bool visible)
{
  label_str1->setVisible(visible);
  editor_str1->setVisible(visible);
}

void MMGActionDisplay::setStr2Visible(bool visible)
{
  label_str2->setVisible(visible);
  editor_str2->setVisible(visible);
}

void MMGActionDisplay::setStr3Visible(bool visible)
{
  label_str3->setVisible(visible);
  editor_str3->setVisible(visible);
}

void MMGActionDisplay::setStr1Options(const QStringList &options)
{
  editor_str1->blockSignals(true);
  editor_str1->clear();
  editor_str1->addItems(options);
  if (options.contains(*str1)) editor_str1->setCurrentText(*str1);
  setStr1(editor_str1->currentText());
  editor_str1->blockSignals(false);
}

void MMGActionDisplay::setStr2Options(const QStringList &options)
{
  editor_str2->blockSignals(true);
  editor_str2->clear();
  editor_str2->addItems(options);
  if (options.contains(*str2)) editor_str2->setCurrentText(*str2);
  setStr2(editor_str2->currentText());
  editor_str2->blockSignals(false);
}

void MMGActionDisplay::setStr3Options(const QStringList &options)
{
  editor_str3->blockSignals(true);
  editor_str3->clear();
  editor_str3->addItems(options);
  if (options.contains(*str3)) editor_str3->setCurrentText(*str3);
  setStr3(editor_str3->currentText());
  editor_str3->blockSignals(false);
}

void MMGActionDisplay::setScrollWidget(QWidget *widget)
{
  scroll_area->takeWidget();
  current_scroll_widget = widget;
  scroll_area->setWidget(widget);
}
