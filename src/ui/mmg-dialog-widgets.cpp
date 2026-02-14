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

#include "mmg-dialog-widgets.h"

#include <QColorDialog>
#include <QFileDialog>
#include <QFontDialog>

namespace MMGWidgets {

// MMGColorDialogWidget
QString MMGColorDialogWidget::style_sheet = "border-radius: 8px; background-color: %1;";

MMGColorDialogWidget::MMGColorDialogWidget(QWidget *parent) : MMGDialogWidget(parent)
{
	color_frame = new QFrame(this);
	color_frame->setObjectName("sep"); // For border color
	color_frame->setCursor(Qt::PointingHandCursor);
	main_layout->addWidget(color_frame);
}

void MMGColorDialogWidget::reset()
{
	_color = default_color;
	update();
}

bool MMGColorDialogWidget::openDialog()
{
	QColor new_color = QColorDialog::getColor(_color, nullptr, "", (QColorDialog::ColorDialogOption)alpha);
	bool valid = new_color.isValid();

	if (valid) _color = new_color;
	return valid;
}

void MMGColorDialogWidget::display() const
{
	color_frame->setStyleSheet(style_sheet.arg(_color.name((QColor::NameFormat)alpha)));
}
// End MMGColorDialogWidget

// MMGFontDialogWidget
MMGFontDialogWidget::MMGFontDialogWidget(QWidget *parent) : MMGDialogWidget(parent)
{
	font_label = new QLabel(this);
	font_label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	font_label->setWordWrap(true);
	font_label->setCursor(Qt::PointingHandCursor);
	main_layout->addWidget(font_label);
}

void MMGFontDialogWidget::reset()
{
	_font = default_font;
	update();
}

bool MMGFontDialogWidget::openDialog()
{
	bool valid;
	QFont new_font = QFontDialog::getFont(&valid, _font, nullptr, mmgtr("Fields.FontSelect"),
					      QFontDialog::DontUseNativeDialog);

	if (valid) _font = new_font;
	return valid;
}

void MMGFontDialogWidget::display() const
{
	font_label->setFont(_font);
	font_label->setText(QString(mmgtr("Fields.FontStyle").translate()).arg(_font.family()).arg(_font.pointSize()));
}
// End MMGFontDialogWidget

// MMGPathDialogWidget
MMGPathDialogWidget::MMGPathDialogWidget(QWidget *parent) : MMGDialogWidget(parent)
{
	path_label = new QLabel(this);
	path_label->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	path_label->setWordWrap(true);
	path_label->setCursor(Qt::PointingHandCursor);
	main_layout->addWidget(path_label);
}

void MMGPathDialogWidget::reset()
{
	_dir = default_dir;
	update();
}

void MMGPathDialogWidget::setParams(const MMGParams<QDir> &params)
{
	setDefaultPath(params.default_value);
	setDefaultAskPath(params.default_ask_path);
	setFilters(params.path_filters);
	setDialogType(params.dialog_type);
}

bool MMGPathDialogWidget::openDialog()
{
	QString new_path;
	switch (dialog_type) {
		case OBS_PATH_FILE:
		default:
			new_path = QFileDialog::getOpenFileName(nullptr, mmgtr("Fields.FileSelect"), default_ask_path,
								path_filters, nullptr,
								QFileDialog::HideNameFilterDetails);
			break;

		case OBS_PATH_FILE_SAVE:
			new_path = QFileDialog::getSaveFileName(nullptr, mmgtr("Fields.FileSelect"), default_ask_path,
								path_filters);
			break;

		case OBS_PATH_DIRECTORY:
			new_path = QFileDialog::getExistingDirectory(nullptr, mmgtr("Fields.FolderSelect"),
								     default_ask_path);
			break;
	}

	bool valid = !new_path.isNull();

	if (valid) _dir.setPath(new_path);
	return valid;
}

void MMGPathDialogWidget::display() const
{
	path_label->setText(_dir.absolutePath());
}
// End MMGPathDialogWidget

} // namespace MMGWidgets
