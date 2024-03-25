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

using namespace MMGUtils;

MMGMessageDisplay::MMGMessageDisplay(QWidget *parent) : QWidget(parent)
{
	setFixedSize(330, 440);

	device_display = new MMGStringDisplay(this);
	device_display->move(0, 5);
	device_display->setDescription(mmgtr("Device.Name"));
	connect(device_display, &MMGStringDisplay::stringChanged, this, &MMGMessageDisplay::setDevice);

	type_display = new MMGStringDisplay(this);
	type_display->setDisplayMode(MMGStringDisplay::MODE_NORMAL);
	type_display->move(0, 55);
	type_display->setDescription(mmgtr("Message.Type.Text"));
	type_display->setBounds(MMGMessage::acceptedTypes());
	type_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_TOGGLE);
	connect(type_display, &MMGStringDisplay::stringChanged, this, &MMGMessageDisplay::setLabels);

	channel_display = new MMGNumberDisplay(this);
	channel_display->move(0, 150);
	channel_display->setDescription(mmgtr("Message.Channel"));
	channel_display->setBounds(1, 16);
	channel_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_TOGGLE);

	note_display = new MMGNumberDisplay(this);
	note_display->move(0, 235);
	note_display->setDescription(mmgtr("Message.Note"));
	note_display->setBounds(0, 127);
	note_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_TOGGLE);

	value_display = new MMGNumberDisplay(this);
	value_display->move(0, 320);
	value_display->setDescription(mmgtr("Message.Velocity"));
	value_display->setBounds(0, 127);
	value_display->setOptions(MIDIBUTTON_MIDI | MIDIBUTTON_CUSTOM | MIDIBUTTON_TOGGLE);
	value_display->lower();

	listen_button = new QPushButton(this);
	listen_button->setGeometry(0, 407, 330, 30);
	listen_button->setCursor(Qt::PointingHandCursor);
	listen_button->setCheckable(true);
	listen_button->setText(mmgtr("UI.Listen.Execution"));
	connect(listen_button, &QPushButton::clicked, this, &MMGMessageDisplay::onListenClick);
}

void MMGMessageDisplay::setStorage(MMGMessage *storage)
{
	_storage = storage;

	MMGNoEdit no_edit_storage(storage);
	device_display->setStorage(&_device);
	type_display->setStorage(&storage->type());
	channel_display->setStorage(&storage->channel());
	note_display->setStorage(&storage->note());
	value_display->setStorage(&storage->value());

	device_display->setBounds(manager(device)->names());
	setLabels();
}

void MMGMessageDisplay::connectDevice(bool connected)
{
	MMGMIDIPort *device = _storage->device();
	if (!device) return;

	if (connected) {
		connect(device, &MMGMIDIPort::messageListened, this, &MMGMessageDisplay::updateMessage);
	} else {
		disconnect(device, &MMGMIDIPort::messageListened, this, &MMGMessageDisplay::updateMessage);
	}
}

void MMGMessageDisplay::setLabels()
{
	if (!_storage) return;

	if (_storage->type() == mmgtr("Message.Type.NoteOn") || _storage->type() == mmgtr("Message.Type.NoteOff")) {
		note_display->setVisible(true);
		note_display->setDescription(mmgtr("Message.Note"));
		value_display->setDescription(mmgtr("Message.Velocity"));
	} else if (_storage->type() == mmgtr("Message.Type.ControlChange")) {
		note_display->setVisible(true);
		note_display->setDescription(mmgtr("Message.Control"));
		value_display->setDescription(mmgtr("Message.Value"));
	} else if (_storage->type() == mmgtr("Message.Type.ProgramChange")) {
		note_display->setVisible(false);
		value_display->setDescription(mmgtr("Message.Program"));
	} else if (_storage->type() == mmgtr("Message.Type.PitchBend")) {
		note_display->setVisible(false);
		value_display->setDescription(mmgtr("Message.PitchAdjust"));
	}
}

void MMGMessageDisplay::setDevice()
{
	connectDevice(false);
	_storage->setDevice(manager(device)->find(_device));
}

void MMGMessageDisplay::onListenClick()
{
	if (!_storage) return;

	MMGMIDIPort *device = _storage->device();
	if (!device) return;

	listening_mode++;
	deactivate();

	switch (listening_mode) {
		default:
			listening_mode = 0;
			[[fallthrough]];

		case 0: // Listen for execution
			listen_button->setChecked(false);
			listen_button->setText(mmgtr("UI.Listen.Execution"));
			global_blog(LOG_DEBUG, "Listening deactivated.");
			break;

		case 1: // Listen once
			listen_button->setChecked(true);
			listen_button->setText(mmgtr("UI.Listen.Once"));
			connect(device, &MMGMIDIPort::messageListened, this, &MMGMessageDisplay::updateMessage);
			global_blog(LOG_DEBUG, "Single listen activated.");
			break;

		case 2: // Listen continuously
			listen_button->setChecked(true);
			listen_button->setText(mmgtr("UI.Listen.Continuous"));
			connect(device, &MMGMIDIPort::messageListened, this, &MMGMessageDisplay::updateMessage);
			global_blog(LOG_DEBUG, "Continuous listen activated.");
			break;
	}
}

void MMGMessageDisplay::deactivate()
{
	if (!_storage) return;

	disconnect(_storage->device(), &MMGMIDIPort::messageListened, this, &MMGMessageDisplay::updateMessage);
}

void MMGMessageDisplay::updateMessage(const MMGSharedMessage &incoming)
{
	if (!incoming || listening_mode < 1) {
		deactivate();
		return;
	}

	// Check the validity of the message type (whether it is one of the five supported types)
	if (MMGMessage::acceptedTypes().indexOf(incoming->type()) == -1) return;

	if (listening_mode == 1) {
		listening_mode = -1;
		onListenClick();
	}

	QString name = _storage->objectName();
	incoming->copy(_storage);
	_storage->setObjectName(name);
	_storage->value().setMax(127);

	MMGNoEdit no_edit_message(_storage);
	type_display->display();
	channel_display->display();
	note_display->display();
	value_display->display();
}