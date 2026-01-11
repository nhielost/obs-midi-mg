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

#include "mmg-preference-defs.h"
#include "mmg-config.h"
#include "mmg-midi.h"

#include "ui/mmg-value-display.h"

#include <libremidi/api.hpp>
#include <libremidi/libremidi.hpp>

#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>

namespace MMGPreferences {

// MMGPreferenceGeneral
MMGPreferenceGeneral *MMGPreferenceGeneral::self = nullptr;

void MMGPreferenceGeneral::createDisplay(QWidget *widget)
{
	QHBoxLayout *main_layout = new QHBoxLayout;
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(10);
	qobject_cast<QVBoxLayout *>(widget->layout())->addLayout(main_layout);

	QPushButton *export_button = new QPushButton(widget);
	export_button->setFixedSize(40, 40);
	export_button->setCursor(QCursor(Qt::PointingHandCursor));
	export_button->setIcon(mmgicon("export"));
	export_button->setIconSize(QSize(21, 21));
	export_button->setToolTip(mmgtr("Preferences.General.Export"));
	connect(export_button, &QPushButton::clicked, this, &MMGPreferenceGeneral::exportBindings);
	main_layout->addWidget(export_button);

	QPushButton *import_button = new QPushButton(widget);
	import_button->setFixedSize(40, 40);
	import_button->setCursor(QCursor(Qt::PointingHandCursor));
	import_button->setIcon(mmgicon("import"));
	import_button->setIconSize(QSize(21, 21));
	import_button->setToolTip(mmgtr("Preferences.General.Import"));
	connect(import_button, &QPushButton::clicked, this, &MMGPreferenceGeneral::importBindings);
	main_layout->addWidget(import_button);

	main_layout->addStretch(1);

	QPushButton *help_button = new QPushButton(widget);
	help_button->setFixedSize(40, 40);
	help_button->setCursor(QCursor(Qt::PointingHandCursor));
	help_button->setIcon(mmgicon("help"));
	help_button->setIconSize(QSize(21, 21));
	help_button->setToolTip(mmgtr("Preferences.General.Help"));
	connect(help_button, &QPushButton::clicked, this, &MMGPreferenceGeneral::openHelp);
	main_layout->addWidget(help_button);

	QPushButton *bug_report_button = new QPushButton(widget);
	bug_report_button->setFixedSize(40, 40);
	bug_report_button->setCursor(QCursor(Qt::PointingHandCursor));
	bug_report_button->setIcon(mmgicon("bugreport"));
	bug_report_button->setIconSize(QSize(21, 21));
	bug_report_button->setToolTip(mmgtr("Preferences.General.BugReport"));
	connect(bug_report_button, &QPushButton::clicked, this, &MMGPreferenceGeneral::reportBug);
	main_layout->addWidget(bug_report_button);
}

void MMGPreferenceGeneral::exportBindings() const
{
	QString filepath = QFileDialog::getSaveFileName(nullptr, mmgtr("Preferences.General.ExportTitle"),
							MMGConfig::filename(), mmgtr("Preferences.General.FileType"));
	if (!filepath.isNull()) config()->save(filepath);
}

void MMGPreferenceGeneral::importBindings() const
{
	QString filepath = QFileDialog::getOpenFileName(nullptr, mmgtr("Preferences.General.ImportTitle"), "",
							mmgtr("Preferences.General.FileType"));
	if (filepath.isNull()) return;

	config()->load(filepath);
	config()->finishLoad();

	emit config() -> refreshRequested();
}

void MMGPreferenceGeneral::openHelp() const
{
	QDesktopServices::openUrl(QUrl("https://nhielost.github.io"));
}

void MMGPreferenceGeneral::reportBug() const
{
	QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/issues"));
}
// End MMGPreferenceGeneral

// MMGPreferenceMIDI
static const QList<libremidi_api> api_blacklist {WINDOWS_UWP, WINDOWS_MIDI_SERVICES, KEYBOARD, KEYBOARD_UMP, NETWORK, NETWORK_UMP, DUMMY};

MMGPreferenceMIDI *MMGPreferenceMIDI::self = nullptr;

static MMGParams<libremidi_api> api_params {
	.desc = mmgtr("Preferences.MIDI.API"),
	.options = OPTION_NONE,
	.default_value = libremidi_api::UNSPECIFIED,
	.bounds = {},
};

static MMGParams<MMGPreferenceMIDI::MessageMode> message_mode_params {
	.desc = mmgtr("Preferences.MIDI.MessageMode"),
	.options = OPTION_NONE,
	.default_value = MMGPreferenceMIDI::MIDI_ALWAYS_1,
	.bounds =
		{
			{MMGPreferenceMIDI::MIDI_ALWAYS_1, mmgtr("Preferences.MIDI.MessageMode.Always1")},
			{MMGPreferenceMIDI::MIDI_ALWAYS_2, mmgtr("Preferences.MIDI.MessageMode.Always2")},
		},
};

void MMGPreferenceMIDI::load(const QJsonObject &json_obj)
{
	message_mode = MMGJson::getValue<MessageMode>(json_obj, "message_mode");

	if (json_obj.contains("api")) midi_api = MMGJson::getValue<uint32_t>(json_obj, "api");
	refreshAPIs();
	// Get the default OS MIDI api if json_obj is from a pre v3.1 version or the
	// selected api is not found
	if (!api_params.bounds.contains(libremidi_api(midi_api))) midi_api = uint32_t(api_params.default_value);

	initMIDI();
}

void MMGPreferenceMIDI::json(QJsonObject &json_obj) const
{
	MMGJson::setValue(json_obj, "message_mode", message_mode);
	MMGJson::setValue(json_obj, "api", midi_api);
}

void MMGPreferenceMIDI::setMessageMode(const MessageMode &mode)
{
	if (message_mode == mode) return;
	message_mode = mode;
	initMIDI();
}

void MMGPreferenceMIDI::setMIDIAPI(const uint32_t &api)
{
	if (midi_api == api) return;
	midi_api = api;
	initMIDI();
}

void MMGPreferenceMIDI::initMIDI()
{
	mmgblog(LOG_INFO, "Initializing MIDI...");
	resetMIDIAPI(libremidi_api(midi_api));
	mmgblog(LOG_INFO, "MIDI initialized.");
}

void MMGPreferenceMIDI::refreshAPIs()
{
	api_params.bounds.clear();

	for (auto api : libremidi::available_apis()) {
		if (api_blacklist.contains(api)) continue;
		api_params.bounds.insert(api, nontr(libremidi::get_api_display_name(api).data()));
	}

	for (auto api : libremidi::available_ump_apis()) {
		if (api_blacklist.contains(api)) continue;
		api_params.bounds.insert(api, nontr(libremidi::get_api_display_name(api).data()));
	}

	if (api_params.bounds.contains(libremidi_api(self->midi_api)))
		api_params.default_value = libremidi_api(self->midi_api);
	else if (api_params.bounds.contains(libremidi::midi2::default_api()))
		api_params.default_value = libremidi::midi2::default_api();
	else if (api_params.bounds.contains(libremidi::midi1::default_api()))
		api_params.default_value = libremidi::midi1::default_api();
	else
		api_params.default_value = libremidi_api::DUMMY;
}

void MMGPreferenceMIDI::createDisplay(QWidget *widget)
{
	refreshAPIs();

	auto *mode_display = new MMGWidgets::MMGValueFixedDisplay<MessageMode>(widget, &message_mode_params);
	mode_display->setContentsMargins(5, 5, 5, 5);
	mode_display->refresh();
	mode_display->setValue(message_mode);
	connect(mode_display, &MMGWidgets::MMGValueQWidget::valueChanged, this,
		[this, mode_display]() { setMessageMode(mode_display->value()); });
	widget->layout()->addWidget(mode_display);

	auto *api_display = new MMGWidgets::MMGValueFixedDisplay<libremidi_api>(widget, &api_params);
	api_display->setContentsMargins(5, 5, 5, 5);
	api_display->setModifiable(false);
	api_display->refresh();
	api_display->setValue(libremidi_api(midi_api));
	connect(api_display, &MMGWidgets::MMGValueQWidget::valueChanged, this,
		[this, api_display]() { setMIDIAPI(uint32_t(api_display->value())); });
	widget->layout()->addWidget(api_display);
}
// End MMGPreferenceMIDI

// MMGPreferenceAbout
MMGPreferenceAbout *MMGPreferenceAbout::self = nullptr;

void MMGPreferenceAbout::createDisplay(QWidget *widget)
{
	QHBoxLayout *main_layout = new QHBoxLayout;
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(10);
	qobject_cast<QVBoxLayout *>(widget->layout())->addLayout(main_layout);

	QPushButton *author_button = new QPushButton(widget);
	author_button->setFixedHeight(40);
	author_button->setCursor(QCursor(Qt::PointingHandCursor));
	author_button->setText(mmgtr("Preferences.About.Creator").arg(mmgtr("Plugin.Author")));
	connect(author_button, &QPushButton::clicked, this, &MMGPreferenceAbout::openAuthor);
	main_layout->addWidget(author_button, 1);

	QPushButton *version_button = new QPushButton(widget);
	version_button->setFixedHeight(40);
	version_button->setCursor(QCursor(Qt::PointingHandCursor));
	version_button->setText(OBS_MIDIMG_VERSION);
	connect(version_button, &QPushButton::clicked, this, &MMGPreferenceAbout::checkForUpdates);
	main_layout->addWidget(version_button, 1);
}

void MMGPreferenceAbout::openAuthor() const
{
	QDesktopServices::openUrl(QUrl("https://nhielost.github.io/docs/about"));
}

void MMGPreferenceAbout::checkForUpdates() const
{
	QDesktopServices::openUrl(QUrl("https://github.com/nhielost/obs-midi-mg/releases"));
}
// End MMGPreferenceAbout

} // namespace MMGPreferences
