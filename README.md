# obs-midi-mg

Allows MIDI devices to interact with OBS Studio.

### This plugin will not work in OBS Studio versions below 28.0.0 due to its usage of Qt 6, meaning that any users using an OS below Windows 10 or MacOS 10.15 will not be able to use this plugin.
(This may be addressed at a later date.)

[![CodeFactor](https://www.codefactor.io/repository/github/nhielost/obs-midi-mg/badge)](https://www.codefactor.io/repository/github/nhielost/obs-midi-mg)

## Description

Connect MIDI devices and seamlessly integrate with OBS Studio! This plugin offers:
- Easy setup
- Easy navigation
- Many customization options
- Cross-platform

## Installing

Go to the [Releases page](https://github.com/nhielost/obs-midi-mg/releases) and download and install the latest release for the proper operating system. That's it!

## Usage

To set up bindings, open OBS Studio and open the setup window under *Tools > obs-midi-mg Setup*. The setup window should appear.

If any MIDI devices are connected, they should appear in the ***Devices*** display on the left.

A device must be chosen as an active device in order to receive messages. To do this, select the **Set As Active Device** option in the device's status tab. Only one device can be active at a time.

To access the bindings display, select the device to use for interacting with OBS Studio, and click *View Bindings* in the bottom right.

### Bindings

To add a binding, click the *Add* button in the bottom left. A new binding will appear in the ***Bindings*** display for the selected device.

To rename the binding, double-click the binding and type to change the name. To remove the binding (if needed), select the binding and click *Remove* in the bottom left.

To use a binding, messages and actions must be added. To add messages and actions, click *View Messages* and *View Actions* respectively.

### Messages

To add a message, click the *Add* button in the bottom left. A new message will appear in the ***Messages*** display for the selected binding.

To rename the message, double-click the message and type to change the name. To remove the message (if needed), select the message and click *Remove* in the bottom left.

To edit the message, select the message in the ***Messages*** display, and the editor will appear on the right.

Adjust the values as necessary for the message. By default, the value/velocity option is Off for the use of its value in executing actions. If you wish to make the value required for executing an action, click the *Require Value* button in the bottom right, and it will switch to a number. Clicking the button again will revert it to the Off state.

A message can be provided by the device. This can be done by selecting the *Listen to Message* button and sending the message to the plugin. It will automatically fill in the message values for you. Make sure to click *Cancel* when finished.

*Note: You can use multiple messages in a binding by adding more messages. If this is desired, the behavior of the binding may not be appropriate. See [Reception Methods](#reception-methods) for more details*.

### Actions

To add an action, click the *Add* button in the bottom left. A new message will appear in the ***Actions*** display for the selected binding.

To rename the action, double-click the action and type to change the name. To remove the action (if needed), select the action and click *Remove* in the bottom left.

To edit the action, select the action in the ***Actions*** display, and the editor will appear on the right.

Adjust the values as necessary for the action(s) in this binding. Settings will appear as the form is filled out. To use a value from a message, either click the label for the number fields, or select *Use Message Value* in the selection fields.

*Note: You can use multiple actions in a binding by adding more actions. If this is desired, the behavior of the binding may not be appropriate. See [Reception Methods](#reception-methods) for more details*.

### Reception Methods

When selecting a binding, a selection field will appear displaying the reception method. Each method changes how actions are executed, and which messages will be sent as values to each action. In all methods, messages will be evaluated before actions, and all actions will execute after the final message is received.

- **Consecutive**: In this mode, actions can only use the final message as a value. *This is the default method, and should be used in most cases*.
- **Correspondence**: In this mode, actions can use the corresponding message as their value.
   - Example: There are five messages and five actions in a binding. When all five messages have been heard, the first message received will be sent as a parameter to the first action, the second message to the second action, and so on.
- **Multiply**: In this mode, actions receive all previous messages as values. Only use this mode for multiple messages and singular actions as this may take a long time.
   - Example: There are five messages before five actions. When all five messages have been heard, the first action receives all five messages as parameters (in order), then the second action receives all five messages, and so on.

### Message Toggling

A binding can now be set up so that it is easily able to toggle an action, without the need for extra bindings. Message toggling changes how messages are received by changing them before the next message.

- **Off**: Message toggling is disabled, and messages will not change. An unlimited amount of messages can be used.
- **Note Type**: Messages will change based on type. Note On and Note Off messages will switch to the other. Only one message can be used in this mode, and nothing will happen if the message is not a Note On or Note Off message.
- **Velocity**: Messages will change based on velocity. Velocities will switch between 127 and 0. Only one message can be used in this mode, and nothing will happen if the initial velocity is not set to 127 or 0.
- **Note Type and Velocity**: Messages will change based on both message type and velocity. Note On and Note Off messages will switch to the other, and velocities will switch between 127 and 0. The same restrictions apply as above.

### Returning to a Previous Menu

To return to the menu before the current menu, click the *Return* button in the bottom left. The *Messages* and *Actions* menus will return to the *Bindings* menu, and the *Bindings* menu will return to the *Devices* menu.

### Help

If something needs explanation in the plugin, click the *Help* button located to the right of the *Preferences* button. If more help is needed, feel free to look at this page by clicking the *Help* button in the *Preferences* menu, or post a thought on the OBS forum.

### Preferences

To adjust other settings, click the *Preferences* button in the bottom left. Bindings can be exported and imported here, and the whole plugin can be turned off here as well.

## Future Content

The coming updates will hopefully introduce these new features:

- A transfer bindings between devices feature (for when a device has disconnected or is unavailable)
- MIDI Output device support - send a message when an event in OBS Studio occurs (well underway!)

## Feedback

I would love to hear honestly from you about this plugin. Feel free to share some ideas and don't be afraid to report an issue!

[Click here to go to the Issues Page](https://github.com/nhielost/obs-midi-mg/issues).

## Credits

I have to give a shoutout to [@cpyarger](https://github.com/cpyarger) and [@Alzy](https://github.com/alzy) for the inspiration of this plugin. Many ideas found in obs-midi-mg can be found in their plugin. Without [obs-midi](https://github.com/cpyarger/obs-midi/), obs-midi-mg would have never been possible. 