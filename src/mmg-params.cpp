/*
obs-midi-mg
Copyright (C) 2022-2024 nhielost <nhielost@gmail.com>

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

#include "mmg-params.h"
#include "ui/mmg-value-manager.h"

namespace MMGParameters {

template <typename T>
void Construct<T>::createField(MMGWidgets::MMGValueManager *display, MMGValue<T> *storage,
			       const MMGParamsMap<T> *params, const MMGCallback &cb)
{
	display->addNew(storage, params, cb);
}

template <typename T>
void Construct<T>::createWarning(MMGWidgets::MMGValueManager *display,
				 const MMGString &header) requires std::same_as<T, bool>
{
	QLabel *warning_label = new QLabel(display);
	warning_label->setContentsMargins(10, 10, 10, 10);
	warning_label->setWordWrap(true);
	warning_label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
	warning_label->setTextFormat(Qt::MarkdownText);
	warning_label->setOpenExternalLinks(true);
	warning_label->setText(mmgtr(MMGText::join(header, "Warning")));
	display->addCustom(warning_label);
}

} // namespace MMGParameters
