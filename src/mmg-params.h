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

#ifndef MMG_PARAMS_H
#define MMG_PARAMS_H

#include "mmg-string.h"

enum ValueOption : uint32_t {
	OPTION_NONE = 0x0000, // No special values necessary

	OPTION_HIDDEN = 0x0001,   // Widget is not visible to user
	OPTION_DISABLED = 0x0002, // Widget will appear grayed out and cannot be interacted with
	OPTION_EMPHASIS = 0x0004, // Widget descriptions are bolded

	OPTION_SPECIAL_1 = 0x0010, // Widget-specific parameter 1 (MMGLCDNumber: setTimeFormat())
	OPTION_SPECIAL_2 = 0x0020, // Widget-specific parameter 2 (unused)
	OPTION_SPECIAL_3 = 0x0040, // Widget-specific parameter 3 (unused)
	OPTION_SPECIAL_4 = 0x0080, // Widget-specific parameter 4 (unused)

	OPTION_ALLOW_MIDI = 0x0100,      // MIDI button enabled
	OPTION_ALLOW_RANGE = 0x0200,     // Range button enabled
	OPTION_ALLOW_TOGGLE = 0x0400,    // Toggle button enabled
	OPTION_ALLOW_INCREMENT = 0x0800, // Increment button enabled
	OPTION_ALLOW_IGNORE = 0x1000,    // Ignore button enabled
};
Q_DECLARE_FLAGS(MMGValueOptions, ValueOption);
Q_DECLARE_OPERATORS_FOR_FLAGS(MMGValueOptions);

template <typename T> class MMGValue;
namespace MMGWidgets {
class MMGValueManager;
}

namespace MMGParameters {

template <typename T> class MMGParamsMap {
public:
	MMGText desc;
	MMGValueOptions options;
	T default_value;

	MMGTranslationMap<T> bounds;
	MMGText placeholder;
	bool text_editable = false;

	int64_t boundsSize() const { return bounds.size(); };
	T boundsValue(int64_t index) const { return bounds.keyAtIndex(index % boundsSize()); };
};

template <typename T> requires MMGIsNumeric<T> class MMGParamsMap<T> {
public:
	MMGText desc;
	MMGValueOptions options;
	T default_value = 0;

	double lower_bound = 0.0;
	double upper_bound = 100.0;
	double step = 1.0;
	double incremental_bound = 10.0;

	int64_t boundsSize() const { return std::clamp<int64_t>((upper_bound - lower_bound + 1.0) / step, 1, 128); };
	T boundsValue(int64_t index) const
	{
		return std::clamp<T>(default_value + (index * step), lower_bound, upper_bound);
	};
};

template <> class MMGParamsMap<bool> {
public:
	MMGText desc;
	MMGValueOptions options;
	bool default_value;

	int64_t boundsSize() const { return 2; };
	bool boundsValue(int64_t index) const { return index == 0; };
};

template <> class MMGParamsMap<QColor> {
public:
	MMGText desc;
	MMGValueOptions options;
	QColor default_value;

	bool alpha = false;

	int64_t boundsSize() const { return 128; };
	QColor boundsValue(int64_t index) const { return QColor::fromHsv((index & 0b111) << 5, 255, 255); };
};

template <> class MMGParamsMap<QFont> {
public:
	MMGText desc;
	MMGValueOptions options;
	QFont default_value;

	int64_t boundsSize() const { return 128; };
	QFont boundsValue(int64_t index) const
	{
		QFont default_font = default_value;
		default_font.setPointSize(8 + index);
		return default_font;
	};
};

template <> class MMGParamsMap<QDir> {
public:
	MMGText desc;
	MMGValueOptions options;
	QDir default_value;

	QString default_ask_path;
	QString path_filters;
	obs_path_type dialog_type;

	int64_t boundsSize() const { return 128; };
	QDir boundsValue(int64_t index) const
	{
		QDir default_dir = default_value;

		QFileInfo dir_info(default_dir.absolutePath());
		default_dir = dir_info.absoluteDir(); // Only have the path, not the file
		QString dir_filename = dir_info.fileName();

		default_dir.setPath(QString("%1/%2").arg(default_dir.absolutePath()).arg(index));
		if (!dir_filename.isEmpty())
			default_dir.setPath(QString("%1/%2").arg(default_dir.absolutePath()).arg(dir_filename));

		dir_filename = default_dir.absolutePath();
		return default_dir;
	};
};

template <> class MMGParamsMap<QString> {
public:
	MMGText desc;
	MMGValueOptions options;
	obs_text_type text_type = OBS_TEXT_DEFAULT;
	QString default_value;

	int64_t boundsSize() const { return 128; };
	QString boundsValue(int64_t index) const
	{
		return mmgtr("MIDIButtons.MIDI.DisplayLabel").arg(mmgtr("Fields.StringPlaceholder")).arg(index + 1);
	};
};

template <typename T> class Construct {
public:
	static void createField(MMGWidgets::MMGValueManager *display, class MMGValue<T> *storage,
				const MMGParamsMap<T> *params, const MMGCallback &cb = 0);

	static void createWarning(MMGWidgets::MMGValueManager *display,
				  const MMGString &header) requires std::same_as<T, bool>;
};

template <typename T>
void createField(MMGWidgets::MMGValueManager *display, class MMGValue<T> *storage, const MMGParamsMap<T> *params,
		 const MMGCallback &cb = 0)
{
	Construct<T>::createField(display, storage, params, cb);
};

inline void createWarning(MMGWidgets::MMGValueManager *display, const MMGString &header)
{
	Construct<bool>::createWarning(display, header);
};

} // namespace MMGParameters

template <typename T> using MMGParams = MMGParameters::MMGParamsMap<T>;

#endif // MMG_PARAMS_H
