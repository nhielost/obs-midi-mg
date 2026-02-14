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

#ifndef MMG_DIALOG_WIDGETS_H
#define MMG_DIALOG_WIDGETS_H

#include "mmg-value-widget.h"

#include <QDir>
#include <QLabel>

namespace MMGWidgets {

class MMGDialogWidget : public MMGValueWidget {
	Q_OBJECT

public:
	MMGDialogWidget(QWidget *parent = nullptr) : MMGValueWidget(parent) {};

public slots:
	virtual void display() const override = 0;

protected:
	bool focusAcquired() override
	{
		if (openDialog()) update();
		return true;
	};
	bool focusLost() override { return true; };
	virtual bool openDialog() = 0;
};

class MMGColorDialogWidget : public MMGDialogWidget {
	Q_OBJECT

public:
	MMGColorDialogWidget(QWidget *parent = nullptr);

	const QColor &value() const { return _color; };
	void setValue(const QColor &color) { _color = color; };

	void setDefaultColor(const QColor &color) { default_color = color; };
	void reset() override;

	void setParams(const MMGParams<QColor> &params)
	{
		setDefaultColor(params.default_value);
		alpha = params.alpha;
	};

public slots:
	void display() const override;

private slots:
	bool openDialog() override;

private:
	QFrame *color_frame;

	QColor _color;
	QColor default_color;
	bool alpha;

	static QString style_sheet;
};

class MMGFontDialogWidget : public MMGDialogWidget {
	Q_OBJECT

public:
	MMGFontDialogWidget(QWidget *parent = nullptr);

	const QFont &value() const { return _font; };
	void setValue(const QFont &font) { _font = font; };

	void setDefaultFont(const QFont &font) { default_font = font; };
	void reset() override;

	void setParams(const MMGParams<QFont> &params) { setDefaultFont(params.default_value); };

public slots:
	void display() const override;

private slots:
	bool openDialog() override;

private:
	QFont _font;
	QFont default_font;

	QLabel *font_label;
};

class MMGPathDialogWidget : public MMGDialogWidget {
	Q_OBJECT

public:
	MMGPathDialogWidget(QWidget *parent = nullptr);

	const QDir &value() const { return _dir; };
	void setValue(const QDir &dir) { _dir = dir; };

	void setDefaultAskPath(const QString &path) { default_ask_path = path; };
	void setFilters(const QString &filters) { path_filters = filters; };
	void setDialogType(obs_path_type type) { dialog_type = type; };

	void setDefaultPath(const QDir &dir) { default_dir = dir; };
	void reset() override;

	void setParams(const MMGParams<QDir> &params);

public slots:
	void display() const override;

private slots:
	bool openDialog() override;

private:
	QDir _dir;
	QDir default_dir;

	QString default_ask_path;
	QString path_filters;
	obs_path_type dialog_type = OBS_PATH_FILE;

	QLabel *path_label;
};

} // namespace MMGWidgets

#endif // MMG_DIALOG_WIDGETS_H
