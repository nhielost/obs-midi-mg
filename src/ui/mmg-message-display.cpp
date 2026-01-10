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

#include "mmg-message-display.h"

#include "../messages/mmg-message.h"
#include "../mmg-config.h"

namespace MMGWidgets {

MMGParams<MMGMIDIPort *> MMGMessageDisplay::device_params {
	.desc = mmgtr("Device.Name"),
	.options = OPTION_NONE,
	.default_value = nullptr,
	.bounds = {},
	.placeholder = mmgtr("Device.NoConnection"),
};

MMGParams<MMGMessages::Id> MMGMessageDisplay::id_params {
	.desc = mmgtr("Message.CV.Status"),
	.options = OPTION_NONE,
	.default_value = MMGMessages::Id(0x0181),
	.bounds = {},
};

MMGMessageDisplay::MMGMessageDisplay(QWidget *parent, MMGStateDisplay *state_display)
	: MMGValueManager(parent, state_display)
{
	if (!!state_display) state_display->setMessageReferences(state_infos);

	addFixed(&_device, &device_params, std::bind(&MMGMessageDisplay::setDevice, this));
	addFixed(&_id, &id_params, std::bind(&MMGMessageDisplay::setType, this));

	id_params.bounds = MMGMessages::availableMessages(MMGMessages::Id(0x4000));

	listen_button = new QPushButton(this);
	listen_button->setFixedHeight(30);
	listen_button->setCursor(Qt::PointingHandCursor);
	listen_button->setCheckable(true);
	listen_button->setText(mmgtr("UI.Listen.Execution"));
	connect(listen_button, &QPushButton::clicked, this, &MMGMessageDisplay::onListenClick);
	connect(this, &MMGValueManager::modifyRequested, listen_button, &QPushButton::setVisible);
	bottom_layout->addWidget(listen_button, 0, Qt::AlignBottom);
}

void MMGMessageDisplay::setStorage(DeviceType message_type, MMGMessageManager *parent, MMGMessage *storage)
{
	MMGTranslationMap<MMGMIDIPort *> typed_device_names;

	for (const auto &[device, tr] : manager(device)->names())
		if (device->isCapable(message_type)) typed_device_names.insert(device, tr);

	device_params.bounds = typed_device_names;

	if (_parent == parent && _storage == storage) {
		refreshAll();
		return;
	}

	if (listening_mode > 0) resetListening();

	if (!!_storage) disconnect(_storage, &QObject::destroyed, this, nullptr);
	_parent = parent;
	_storage = storage;
	if (!parent || !storage) {
		clear();
		return;
	}
	connect(_storage, &QObject::destroyed, this, [&]() { _storage = nullptr; });

	_device = storage->device();
	_id = storage->id();

	resetMessage();
}

void MMGMessageDisplay::setDevice()
{
	if (!_storage) return;
	connectDevice(false);

	_storage->setDevice(_device);
	emit messageChanged();

	listening_mode--;
	onListenClick();
}

void MMGMessageDisplay::setType()
{
	if (MMGMessages::changeMessage(_parent, _storage, _id)) {
		emit messageChanged();
		resetMessage();
	}
}

void MMGMessageDisplay::resetMessage()
{
	clear();

	_storage->createDisplay(this);
	refresh_sender = nullptr;
	refreshAll();
}

void MMGMessageDisplay::resetListening()
{
	listening_mode = -1;
	onListenClick();
}

void MMGMessageDisplay::connectDevice(bool connected)
{
	if (!_storage || !_storage->device()) return;
	_storage->device()->blockReceiver(this, connected);
}

void MMGMessageDisplay::onListenClick()
{
	listening_mode++;
	connectDevice(false);

	switch (listening_mode) {
		default:
			listening_mode = 0;
			[[fallthrough]];

		case 0: // Listen for execution
			listen_button->setChecked(false);
			listen_button->setText(mmgtr("Message.Listen.Execution"));
			mmgblog(LOG_DEBUG, "Listening deactivated.");
			break;

		case 1: // Listen once
			listen_button->setChecked(true);
			listen_button->setText(mmgtr("Message.Listen.Once"));
			connectDevice(true);
			mmgblog(LOG_DEBUG, "Single listen activated.");
			break;

		case 2: // Listen continuously
			listen_button->setChecked(true);
			listen_button->setText(mmgtr("Message.Listen.Continuous"));
			connectDevice(true);
			mmgblog(LOG_DEBUG, "Continuous listen activated.");
			break;
	}
}

void MMGMessageDisplay::processMessage(const MMGMessageData &incoming)
{
	if (listening_mode < 1) {
		connectDevice(false);
		return;
	}

	MMGMessages::Id old_id = _id;
	MMGMessages::Id new_id = MMGMessages::Id(0x4000 | incoming.status());
	if (!id_params.bounds.contains(new_id)) return;

	if (listening_mode == 1) resetListening();
	_id = new_id;

	if (old_id == _id) {
		_storage->copyFromMessageData(incoming);
		refreshAll();
	} else {
		runInMainThread([this, incoming]() {
			MMGMessages::changeMessage(_parent, _storage, _id);
			_storage->copyFromMessageData(incoming);
			resetMessage();
		});
	}
}

} // namespace MMGWidgets
