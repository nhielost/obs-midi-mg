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

#include "mmg-action-midi.h"
#include "../mmg-config.h"
#include "../ui/mmg-action-display.h"
#include "../ui/mmg-message-display.h"

namespace MMGActions {

// MMGActionMIDISend
MMGActionMIDISend::MMGActionMIDISend(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  messages(new MMGMessageManager(this, "messages"))
{
	if (!parent) return; // Guard for initial population of MMGActions::all_action_types

	if (json_obj.contains("device")) {
		messages->add(json_obj);
	} else {
		messages->load(json_obj);
	}

	blog(LOG_DEBUG, "Action created.");
}

void MMGActionMIDISend::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	messages->json(json_obj);
}

void MMGActionMIDISend::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionMIDISend *>(dest);
	if (!casted) return;

	casted->messages->clear();
	for (MMGMessage *message : *messages)
		message->copy(casted->messages->add());
}

void MMGActionMIDISend::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	MMGWidgets::MMGMessageDisplay *message_display = new MMGWidgets::MMGMessageDisplay(display, nullptr);
	message_display->setStorage(TYPE_OUTPUT, messages, messages->at(0));

	display->addManager(message_display);
}

void MMGActionMIDISend::execute(const MMGMappingTest &test) const
{
	messages->at(0)->send(test);
	blog(LOG_DEBUG, "Successfully executed.");
}
// End MMGActionMIDISend

// MMGActionMIDIConnection
static MMGParams<MMGString> device_params {
	.desc = mmgtr("Device.Name"),
	.options = OPTION_NONE,
	.default_value = "",
	.bounds = {},
	.placeholder = mmgtr("Device.NoConnection"),
};

static MMGParams<bool> in_status_params {
	.desc = mmgtr("Device.Status.Input"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE,
	.default_value = true,
};

static MMGParams<bool> out_status_params {
	.desc = mmgtr("Device.Status.Output"),
	.options = OPTION_ALLOW_MIDI | OPTION_ALLOW_TOGGLE | OPTION_ALLOW_IGNORE,
	.default_value = true,
};

MMGActionMIDIConnection::MMGActionMIDIConnection(MMGActionManager *parent, const QJsonObject &json_obj)
	: MMGAction(parent, json_obj),
	  _device(json_obj, "device"),
	  in_status(json_obj, "in_status"),
	  out_status(json_obj, "out_status")
{
	blog(LOG_DEBUG, "Action created.");
}

MMGDevice *MMGActionMIDIConnection::device() const
{
	return manager(device)->find(MMGString(_device).value());
}

void MMGActionMIDIConnection::json(QJsonObject &json_obj) const
{
	MMGAction::json(json_obj);

	_device->json(json_obj, "device");
	in_status->json(json_obj, "in_status");
	out_status->json(json_obj, "out_status");
}

void MMGActionMIDIConnection::copy(MMGAction *dest) const
{
	MMGAction::copy(dest);

	auto casted = dynamic_cast<MMGActionMIDIConnection *>(dest);
	if (!casted) return;

	_device.copy(casted->_device);
	in_status.copy(casted->in_status);
	out_status.copy(casted->out_status);
}

void MMGActionMIDIConnection::createDisplay(MMGWidgets::MMGActionDisplay *display)
{
	device_params.bounds.clear();
	for (MMGDevice *device : *manager(device))
		device_params.bounds.insert(qUtf8Printable(device->objectName()),
					    nontr(qUtf8Printable(device->objectName())));

	MMGActions::createActionField(display, &_device, &device_params,
				      std::bind(&MMGActionMIDIConnection::onDeviceChange, this));
	MMGActions::createActionField(display, &in_status, &in_status_params);
	MMGActions::createActionField(display, &out_status, &out_status_params);
}

void MMGActionMIDIConnection::onDeviceChange()
{
	MMGDevice *selected_device = device();
	emit refreshRequested();

	in_status_params.options.setFlag(OPTION_HIDDEN, !selected_device);
	out_status_params.options.setFlag(OPTION_HIDDEN, !selected_device);
	if (!selected_device) return;

	in_status_params.options.setFlag(OPTION_DISABLED, !selected_device->isCapable(TYPE_INPUT));
	out_status_params.options.setFlag(OPTION_DISABLED, !selected_device->isCapable(TYPE_OUTPUT));
}

void MMGActionMIDIConnection::execute(const MMGMappingTest &test) const
{
	MMGDevice *selected_device = device();
	ACTION_ASSERT(selected_device, "The selected device does not exist.");

	bool input_status = selected_device->isActive(TYPE_INPUT);
	bool output_status = selected_device->isActive(TYPE_OUTPUT);

	ACTION_ASSERT(test.applicable(in_status, input_status),
		      "An input status could not be selected. Check the Input Status "
		      "field and try again.");
	ACTION_ASSERT(test.applicable(out_status, output_status),
		      "An output status could not be selected. Check the Output "
		      "Status field and try again.");

	selected_device->setActive(TYPE_INPUT, input_status);
	selected_device->setActive(TYPE_OUTPUT, output_status);

	blog(LOG_DEBUG, "Successfully executed.");
}

void MMGActionMIDIConnection::connectSignal(bool _connect)
{
	MMGDevice *selected_device = device();

	if (_connect && !!selected_device) {
		device_connection = connect(selected_device, &MMGDevice::activeStateChanged, this,
					    &MMGActionMIDIConnection::processMIDIState);
	} else {
		disconnect(device_connection);
	}
}

void MMGActionMIDIConnection::processMIDIState()
{
	MMGDevice *selected_device = device();
	if (!selected_device) return;

	EventFulfillment fulfiller(this);
	fulfiller->addAcceptable(in_status, selected_device->isActive(TYPE_INPUT));
	fulfiller->addAcceptable(out_status, selected_device->isActive(TYPE_OUTPUT));
}
// End MMGActionMIDIConnection

} // namespace MMGActions
