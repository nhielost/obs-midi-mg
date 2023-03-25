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

#include "mmg-number-display.h"

#include <QStandardItemModel>

using namespace MMGUtils;

// MMGNumberDisplay
MMGNumberDisplay::MMGNumberDisplay(QWidget *parent) : QWidget(parent)
{
  lcd_number = new MMGLCDNumber(this);
  lcd_min = new MMGLCDNumber(this);
  lcd_max = new MMGLCDNumber(this);

  setVisible(true);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

  lcd_number->setVisible(true);
  lcd_min->setVisible(false);
  lcd_max->setVisible(false);

  connect(lcd_number, &MMGLCDNumber::numberChanged, this, [&](double val) {
    number->set_num(val);
    emit numberChanged();
  });
  connect(lcd_min, &MMGLCDNumber::numberChanged, this, [&](double val) {
    number->set_min(val);
    emit numberChanged();
  });
  connect(lcd_max, &MMGLCDNumber::numberChanged, this, [&](double val) {
    number->set_max(val);
    emit numberChanged();
  });

  label = new QLabel(this);
  label->setVisible(true);
  label->setWordWrap(true);
  label->setGeometry(0, 0, 95, 30);

  lcd_min->setToolTip("Lower Bound (corresponds to MIDI value of 0)");
  lcd_min->setGeometry(95, 0, 175, 30);
  lcd_max->setToolTip("Upper Bound (corresponds to MIDI value of 127)");
  lcd_max->setGeometry(95, 40, 175, 30);

  combo = new QComboBox(this);
  combo->setGeometry(0, 30, 95, 40);
  combo->addItems({"Fixed", "0-127", "Custom", "Ignore"});
  combo->connect(combo, &QComboBox::currentIndexChanged, this, &MMGNumberDisplay::setLCDMode);

  setDisplayMode(MODE_DEFAULT);
}

void MMGNumberDisplay::setStorage(MMGNumber *storage, bool force_values)
{
  number = storage;
  if (force_values) {
    setBounds(number->min(), number->max());
  } else {
    number->set_min(lcd_min->value());
    number->set_max(lcd_max->value());
  }
  display();
}

void MMGNumberDisplay::setDescription(const QString &desc)
{
  label->setText(desc);
}

void MMGNumberDisplay::setBounds(double lower, double upper, bool extend_bounds)
{
  double range = upper - lower;
  if (extend_bounds) {
    lcd_number->setBounds(lower - range, upper + range);
    lcd_min->setBounds(lower - range, upper + range);
    lcd_max->setBounds(lower - range, upper + range);
  } else {
    lcd_number->setBounds(lower, upper);
    lcd_min->setBounds(lower, upper);
    lcd_max->setBounds(lower, upper);
  }
  if (number) {
    number->set_min(lower);
    number->set_max(upper);
    display();
  }
  default_bounds_min = lower;
  default_bounds_max = upper;
}

void MMGNumberDisplay::setOptions(ComboOptions options)
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>(combo->model());
  model->item(1)->setEnabled(options != 0 && options != 3);
  model->item(2)->setEnabled(options == 2 || options == 4);
  model->item(3)->setEnabled(options > 2);
  if (options == 0 || options == 3) setLCDMode(0);
}

void MMGNumberDisplay::setStep(double inc)
{
  lcd_number->setStep(inc < 0.01 ? 0.01 : inc);
  lcd_min->setStep(inc < 0.01 ? 0.01 : inc);
  lcd_max->setStep(inc < 0.01 ? 0.01 : inc);
}

void MMGNumberDisplay::setTimeFormat(bool format)
{
  lcd_number->setTimeFormat(format);
  lcd_min->setTimeFormat(format);
  lcd_max->setTimeFormat(format);
}

void MMGNumberDisplay::setLCDMode(int mode)
{
  if (mode > 3) return;
  if (combo->currentIndex() != mode) combo->setCurrentIndex(mode);

  lcd_number->setVisible(mode != 2);
  lcd_number->setEnabled(mode == 0);
  lcd_number->display((MMGLCDNumber::State)(mode == 3 ? 2 : mode));
  lcd_min->setVisible(mode == 2);
  lcd_max->setVisible(mode == 2);
  if (number) number->set_state((MMGNumber::State)mode);
  if (mode == 1) {
    // numberChanged will be called twice here
    lcd_min->blockSignals(true);
    lcd_max->blockSignals(true);
    lcd_min->setValue(default_bounds_min);
    lcd_max->setValue(default_bounds_max);
    lcd_min->blockSignals(false);
    lcd_max->blockSignals(false);
  }
  emit numberChanged();
}

void MMGNumberDisplay::setDisplayMode(Mode mode)
{
  switch (mode) {
    case MODE_DEFAULT:
      setFixedSize(270, 70);
      lcd_number->setGeometry(95, 20, 175, 30);
      combo->setVisible(true);
      break;
    case MODE_THIN:
      setFixedSize(270, 30);
      lcd_number->setGeometry(95, 0, 175, 30);
      setLCDMode(0);
      combo->setVisible(false);
      break;
  }
}

void MMGNumberDisplay::display()
{
  lcd_number->setValue(number->num());
  lcd_min->setValue(number->min());
  lcd_max->setValue(number->max());
  combo->setCurrentIndex(number->state());
  setLCDMode(number->state());
}

void MMGNumberDisplay::reset()
{
  if (number) *number = default_val;
  display();
}
// End MMGNumberDisplay

// MMGNumberDisplayFields
MMGNumberDisplayFields::MMGNumberDisplayFields(QWidget *parent) : QWidget(parent)
{
  setGeometry(0, 0, 290, 350);
  QVBoxLayout *default_field_layout = new QVBoxLayout(this);
  default_field_layout->setSpacing(10);
  default_field_layout->setSizeConstraint(QLayout::SetFixedSize);
  default_field_layout->setContentsMargins(10, 10, 10, 10);
  setLayout(default_field_layout);
}

void MMGNumberDisplayFields::add(MMGNumberDisplay *field)
{
  fields.append(field);
  layout()->addWidget(field);
}
// End MMGNumberDisplayFields
