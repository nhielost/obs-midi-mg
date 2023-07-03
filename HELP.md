# obs-midi-mg Help Page

Tips in using the plugin as well as how it works can be found here.

## Contents
1. [Navigation](#navigation)
   - [Devices](#devices)
   - [Bindings](#bindings)
   - [Preferences](#preferences)
2. [Organization](#organization)
   - [Bindings](#bindings-1)
   - [Messages](#messages)
   - [Actions](#actions)
3. [Actions Guide](#actions-guide)
   - [None](#none)
   - [Streaming](#streaming)
   - [Recording](#recording)
   - [Virtual Camera](#virtual-camera)
   - [Replay Buffer](#replay-buffer)
   - [Studio Mode](#studio-mode)
   - [Scene Switching](#scene-switching)
   - [Video Sources](#video-sources)
   - [Audio Sources](#audio-sources)
   - [Media Sources](#media-sources)
   - [Transitions](#transitions)
   - [Filters](#filters)
   - [Hotkeys](#hotkeys)
   - [Profiles](#profiles)
   - [Scene Collections](#scene-collections)
   - [MIDI](#midi)
   - [Internal](#internal)
   - [Timeout](#timeout)
4. [Using the Message Value](#using-the-message-value)
5. [Preferences Guide](#preferences-guide)
   - [Device Interaction](#device-interaction)
   - [Binding Transfer](#binding-transfer)
6. [Additional Help](#additional-help)

---------------------------------------------------

## Navigation

### Devices

The device that is selected in the *Active Device* field is the device that is currently being used for incoming messages as well as editing bindings. Changing this field will change the device and the bindings view.

### Bindings

Bindings associated with the current active device are listed in the top left corner. To edit them, select the binding name in the list. The message and the action will appear on the right. To change the binding's name, double click it in the list and change the name.

To duplicate bindings, click a binding, then click *Copy Binding*. A new binding should appear in the list with the same values as the one selected.

### Preferences

To access the preferences menu, click the *Preferences* button in the bottom left. Click the *Return to Bindings* button in the bottom left to go back to the binding editor. 

----------------------------------------------------

## Organization

### Bindings

Bindings are composed of one message and one action. If the binding is enabled, the message will be listened to on the current device, and whenever the message is received with all the correct parameters, the action will execute. These parameters can be set in the binding editor when a binding is selected. 

If a binding is disabled, the message will not be listened to. This can be changed by clicking the checkmark next to the binding's name. The action will never be executed whenever any message is received. *See the [Internal](#internal) action for an exception to this rule.*

### Messages

Messages are composed of a type, channel, and one or two other values. These values can be inputted manually or, by using the *Listen to Message* feature, can be inputted by the input device automatically. Only one message can activate any action (no longer can one use multiple messages to do this).

If the *Toggle Note On / Off Messages* option is checked, the message will toggle to the other type every time that it is received (i.e. if the message is currently listening for a Note On type, it will switch to Note Off type when it is received, and viceversa).

If the *Toggle Velocity* option is checked, the message will toggle the velocity between 127 and 0 every time that it is received.

### Actions

Actions are composed of a category, subcategory and other fields. These fields are changed dynamically when the categories and subcategories are changed. 

In addition, some of the fields can use the message value field as their value. The list fields use this by an option called *Use Message Value*, while the number fields can be changed to *0-127*. In *0-127* mode, the plugin will use the message value according to the action specification (see below).

When a number field is in *Fixed* mode, the plugin will use the value that is displayed in the number field for that action. 

For some actions, a *Custom* mode and an *Ignore* mode are present in number fields.

In *Custom* mode, the plugin will use the message value according to the boundary specified in the action. The top number corresponds to a MIDI value of 0, and the bottom number corresponds to a MIDI value of 127. This allows for more control over the range of those actions that support it.

In *Ignore* mode, the plugin will ignore the value in that field while setting other values in other fields. This is the most useful when having to set two values (e.g. Move Source or Source Scale), but only one needs to change.

----------------------------------------------------

## Actions Guide

### None

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| None | Does exactly what you'd expect. | *N/A* | *N/A* |

### Streaming

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Start Streaming | Starts the stream if it is not running. | *N/A* | *N/A* |
| Stop Streaming | Stops the stream if it is running. | *N/A* | *N/A* |
| Toggle Streaming | Starts the stream if it is not running, or stops the stream if it is running. | *N/A* | *N/A* |
                       
### Recording

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Start Recording | Starts recording if it is not running. | *N/A* | *N/A* |
| Stop Recording | Stops recording if it is running. | *N/A* | *N/A* |
| Toggle Recording | Starts recording if it is not running, or stops recording if it is running. | *N/A* | *N/A* |
| Pause Recording | Pauses recording if it is running. | *N/A* | *N/A* |
| Resume Recording | Resumes recording if it is paused. | *N/A* | *N/A* |
| Toggle Pause Recording | Pauses recording if it is running, or resumes recording if it is paused. | *N/A* | *N/A* |

### Virtual Camera

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Start Virtual Camera | Starts the virtual camera if it is not running. | *N/A* | *N/A* |
| Stop Virtual Camera | Stops the virtual camera if it is running. | *N/A* | *N/A* |
| Toggle Virtual Camera | Starts the virtual camera if it is not running, or stops the virtual camera if it is running. | *N/A* | *N/A* |

### Replay Buffer

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Start Replay Buffer | Starts a Replay Buffer. | *N/A* | *N/A* |
| Stop Replay Buffer | Stops a Replay Buffer if one has started. | *N/A* | *N/A* |
| Toggle Replay Buffer | Starts a Replay Buffer if there is not one active, or stops it if it is running. | *N/A* | *N/A* |
| Save Replay Buffer | Saves the Replay Buffer to a file. | *N/A* | *N/A* |

### Studio Mode

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Turn On Studio Mode | Turns on Studio Mode. | *N/A* | *N/A* |
| Turn Off Studio Mode | Turns off Studio Mode. | *N/A* | *N/A* |
| Toggle Studio Mode | Turns on Studio Mode if it is off, or turns it off if it is on. | *N/A* | *N/A* |
| Change Preview Scene | Changes the scene displayed in the preview in Studio Mode.<br>Does nothing if Studio Mode is disabled. | **SCENE** <sub>[1](#using-the-message-value)</sub>: The name of the scene to switch the preview to. | *N/A* |
| Preview to Program | Switches the preview and the program scenes using the current transition.<br>Does nothing if Studio Mode is disabled. | *N/A* | *N/A* |

### Scene Switching

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Scene Switching | Switches scenes using the current transition. | **SCENE** <sub>[1](#using-the-message-value)</sub>: The name of the scene to switch to. | *N/A* |

### Video Sources

All Video Sources actions contain these list fields in addition to the fields listed in the table below:

**SCENE**: The name of the scene containing the video source.

**SOURCE**: The name of the video source.


| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Move Source | Moves the source to the specified coordinates on the current scene. | *N/A* | **POSITION X <sub>[3](#using-the-message-value)</sub>**: The x-coordinate of the new position of the source.<br>**POSITION Y <sub>[4](#using-the-message-value)</sub>**: The y-coordinate of the new position of the source. |
| Display Source | Shows or hides the source on the current scene. | **STATE**: The display state of the source. | *N/A* |
| Source Locking | Locks or unlocks the source on the current scene. | **STATE**: The lock state of the source. | *N/A* |
| Source Crop | Crops the source on the current scene. | *N/A* | **TOP <sub>[4](#using-the-message-value)</sub>**: The number of pixels to crop from the top of the source.<br>**RIGHT <sub>[3](#using-the-message-value)</sub>**: The number of pixels to crop from the right side of the source.<br>**BOTTOM <sub>[4](#using-the-message-value)</sub>**: The number of pixels to crop from the bottom of the source.<br>**LEFT <sub>[3](#using-the-message-value)</sub>**: The number of pixels to crop from the left side of the source. |
| Align Source | Changes the alignment of the position of the source on the current scene. | **ALIGNMENT <sub>[2](#using-the-message-value)</sub>**: The alignment state of the source. | *N/A* |
| Source Scale | Scales the source by a specified amount and magnitude on the current scene. | *N/A* | **SCALE X <sub>[5](#using-the-message-value)</sub>**: The percent scale of the x-axis of the source.<br>**SCALE Y <sub>[5](#using-the-message-value)</sub>**: The percent scale of the y-axis of the source.<br>**MAGNITUDE**: The amount of scaling to apply. A magnitude of 1 is the default, and is appropriate for most cases. |
| Source Scale Filtering | Changes the scale filtering of the source on the current scene. | **FILTERING <sub>[2](#using-the-message-value)</sub>**: The scale filtering state of the source. | *N/A* |
| Rotate Source | Rotates the source by a specified number of degrees on the current scene. | *N/A* | **ROTATION <sub>[5](#using-the-message-value)</sub>**: The rotation of the source in degrees. |
| Source Bounding Box Type | Changes the type of bounding box of the source on the current scene. | **TYPE <sub>[2](#using-the-message-value)</sub>**: The bounding box state of the source. | *N/A* |
| Resize Source Bounding Box | Resizes the bounding box of the source on the current scene. | *N/A* | **SIZE X <sub>[3](#using-the-message-value)</sub>**: The new length of the bounding box of the source.<br>**SIZE Y <sub>[4](#using-the-message-value)</sub>**: The new height of the bounding box of the source. |
| Align Source Bounding Box | Changes the alignment of the bounding box of the source on the current scene. | **ALIGNMENT <sub>[2](#using-the-message-value)</sub>**: The alignment state of the bounding box of the source. | *N/A* |
| Source Blending Mode | Changes the blending mode of the source on the current scene. | **BLEND MODE <sub>[2](#using-the-message-value)</sub>**: The blend mode state of the source. | *N/A* |
| Take Source Screenshot | Takes a screenshot of the source. | *N/A* | *N/A* |
| Custom Source Settings | Changes custom properties of a source. | *N/A* | *N/A* |

### Audio Sources

All Audio Sources actions contain these list fields in addition to the fields listed in the table below:

**SOURCE**: The name of the audio source.


| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Change Source Volume To | Sets the volume of the audio source to the specified percentage. | *N/A* | **VOLUME <sub>[5](#using-the-message-value)</sub>**: The volume to set. |
| Change Source Volume By | Changes the volume of the audio source by the specified percentage. | *N/A* | **VOLUME**: The volume increment. |
| Mute Source | Mutes the audio source. | *N/A* | *N/A* |
| Unmute Source | Unmutes the audio source. | *N/A* | *N/A* |
| Toggle Source Mute | Mutes the audio source if unmuted, unmutes otherwise. | *N/A* | *N/A* |
| Source Audio Offset | Changes the audio offset of a source. | *N/A* | **OFFSET <sub>[6](#using-the-message-value)</sub>**: The audio offset to set. |
| Source Audio Monitor | Changes the audio monitor of a source. | **MONITOR <sub>[2](#using-the-message-value)</sub>**: The audio monitor state of the source. | *N/A* |

### Media Sources

All Media Sources actions contain these list fields in addition to the fields listed in the table below:

**SOURCE**: The name of the media source.


| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Play or Pause | Pauses the media source if playing, plays otherwise. | *N/A* | *N/A* |
| Restart | Restarts the media source if playing. | *N/A* | *N/A* |
| Stop | Stops the media source. | *N/A* | *N/A* |
| Set Track Time | Sets the time of the media source. | *N/A* | **TIME <sub>[5](#using-the-message-value)</sub>**: The time to set to the media source. |
| Next Track | If there are multiple tracks in the media source, advance to the next one. | *N/A* | *N/A* |
| Previous Track | If there are multiple tracks in the media source, go back to the previous one. | *N/A* | *N/A* |
| Skip Forward Time | Advances a specified amount of time in the media source. | *N/A* | **TIME**: The time to advance in the media source. |
| Skip Backward Time | Rewinds a specified amount of time in the media source. | *N/A* | **TIME**: The time to rewind in the media source. |

### Transitions
          
| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Change Current Transition | Sets the current transition. | **TRANSITION**: The name of the transition to use. | **DURATION <sub>[7](#using-the-message-value)</sub>**: The duration of the transition selected. If a value of 0 is used, the plugin will use the current transition duration. |
| Set Source Show Transition | Sets the transition that will be used when a source becomes shown. | **SCENE**: The name of the scene containing the video source.<br>**SOURCE**: The name of the source to set the transition to.<br>**TRANSITION**: The name of the transition to use. | **DURATION <sub>[6](#using-the-message-value)</sub>**: The duration of the transition selected. |
| Set Source Hide Transition | Sets the transition that will be used when a source becomes hidden. | **SCENE**: The name of the scene containing the video source.<br>**SOURCE**: The name of the source to set the transition to.<br>**TRANSITION**: The name of the transition to use. | **DURATION <sub>[6](#using-the-message-value)</sub>**: The duration of the transition selected. |
| Set Transition Bar Position (Studio Mode) | Sets the transition bar position. This will automatically release the transition bar after 1 second of inactivity.<br>Fails if not in Studio Mode. | *N/A* | **POSITION <sub>[5](#using-the-message-value)</sub>**: The amount transitioned by the transition bar. |
| Release Transition Bar (Studio Mode) | Releases the transition bar manually.<br>Fails if not in Studio Mode. | *N/A* | *N/A* |
| Custom Transition Settings | Changes custom properties of a transition. | *N/A* | *N/A* |

### Filters

All Filters actions contain these list fields in addition to the fields listed in the table below:

**SOURCE**: The name of the source containing the filter.

**FILTER**: The name of the filter.


| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Show Filter | Shows a filter on a specified source. | *N/A* | *N/A* |
| Hide Filter | Hides a filter on a specified source. | *N/A* | *N/A* |
| Toggle Filter Display | Shows a filter on a specified source if it is hidden, hides it otherwise. | *N/A* | *N/A* |
| Reorder Filter Appearance | Moves filter up and down the list on a specified source. | *N/A* | **POSITION <sub>[1,7](#using-the-message-value)</sub>**: The new position of the filter in the list. |
| Custom Filter Settings | Changes custom properties of a filter. | *N/A* | *N/A* |

### Hotkeys

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Activate Predefined Hotkey | Activates a hotkey. | **GROUP**: The category of hotkeys as found in the Hotkeys settings menu.<br>**HOTKEY**: The name of the hotkey to activate. | *N/A* |

### Profiles

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Switch Profiles | Switches to another Profile.<br>Fails if a stream / recording / etc. is active. | **PROFILE** <sub>[1](#using-the-message-value)</sub>: The name of the profile to switch to. | *N/A* |

### Scene Collections

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Switch Scene Collections | Switches to another Scene Collection. | **COLLECTION** <sub>[1](#using-the-message-value)</sub>: The name of the scene collection to switch to. | *N/A* |

### MIDI

| Subcategory | Description | List Fields | Number Fields |
|---|---|---|---|
| Send a MIDI Message | Sends the specified MIDI message to the specified MIDI device. <sub>[8](#using-the-message-value)</sub> | **DEVICE**: The name of the device to send  the message to.<br>**TYPE**: The message type. | **CHANNEL**: The message channel.<br>**DATA 1**: The message note / control / program / pitch.<br>**DATA 2**: The message value / velocity. |

### Internal

An unlimited number of actions can now be executed using the *Internal* action. Add actions using the *Add Action* button, and switch between actions by incrementing the *Action #* indicator. Actions cannot be moved around once they are placed, so ensure that the actions are placed in the correct order - Action #1 being first. To remove any action, use the *Remove Action* button.

Actions can be configured to execute certain time intervals after a previous action. The *Timing* option has three choices:

- **Execute as soon as possible** -> The action will execute as soon as either the *Internal* action begins executing or when the previous action finishes executing if it is not Action #1.
- **Wait (ms) before executing** -> The action will execute after some amount of milliseconds have passed. This can be chained, meaning Action #5 may take a while to execute if all the actions have this choice selected.
- **Wait (s) before executing** -> The action will execute after some amount of seconds have passed. This can be chained, meaning Action #5 may take a *long* while to execute if all the actions have this choice selected.

The timing range is set between 1 and 999 of the specified units.

**Note 1**: Actions can now be executed without limits. This changes the policy from when an action could only be executed once.<br>
**Note 2**: Actions executed in this manner bypass disabled bindings, meaning that an action in a disabled binding can be executed.<br>
**Note 3**: If a selected action is using the message for any of its values, that action will use the message sent to the *Internal* action.<br>
**Note 4**: Other *Internal* actions cannot be executed in an *Internal* action. (Just make another binding if this behavior is desired.)<br>
**Note 5**: Only up to 64 *Internal* actions can be running at any one time. Further attempts to run an *Internal* action will fail until one of the 64 running actions finishes.<br>
**Note 6**: Try not to have 64 *Internal* actions running at once, if possible. Your computer will thank you later.

### Timeout

This category is discontinued as it has moved inside of the *Internal* action.

----------------------------------------------------

## Using the Message Value

If any fields are marked with a subscript listed below, they are eligible to be used by the incoming message value. See below for specific details.

<sub>**[1](#using-the-message-value)**</sub> When using this field, a value called *Use Message Value* will appear. If this value is selected, the plugin will use the value/velocity section of the incoming message instead of text.<br>Example: You have 15 objects (scenes, filters, etc.) in a list, and you send a message with a value of 6. The 7th object in the list is the one that will be used in the action.<br>Note 1: The action will fail if the value provided is equal to or larger than the number of objects in the list (e.g. if there are 4 objects, a value of 37 will fail).<br>Note 2: If you have more than 128 objects, those additional objects (e.g. 129, 130, ...) cannot be accessed using this method.

<sub>**[2](#using-the-message-value)**</sub> When using this field, a value called *Use Message Value* will appear. If this value is selected, the plugin will use the value/velocity section of the incoming message instead of the options listed.<br>Example: You have 4 options in a list for an action, and you send a message to that action with a value of 2. The 3rd option in the list is the one that will be used in the action.<br>The action will fail if the value provided is equal to or larger than the number of options in the list (e.g. if there are 4 options, a value of 37 will fail).

<sub>**[3](#using-the-message-value)**</sub> When using this field in *0-127* mode, an incoming message value/velocity will correspond to 1/128 the length of the scene.<br>Example: You have a 1920x1080 scene size, and you send a message to a Move Source action with a value of 29 for the Position X field. The x-coordinate of the source would change to 435.

*This field can also be used in **Ignore** Mode*.

<sub>**[4](#using-the-message-value)**</sub> When using this field in *0-127* mode, an incoming message value/velocity will correspond to 1/128 the height of the scene.<br>Example: You have a 1920x1080 scene size, and you send a message to a Move Source action with a value of 29 for the Position Y field. The y-coordinate of the source would change to about 245.

*This field can also be used in **Ignore** Mode*.

<sub>**[5](#using-the-message-value)**</sub> When using this field in *0-127* mode, an incoming message value/velocity will correspond to a percentage of the largest reasonable value.<br>Example: You have a Set Volume action, and you send a message to it with a value of 102. The volume of the source mentioned in the action will be set to about 80% of full volume.

<sub>**[6](#using-the-message-value)**</sub> When using this field in *0-127* mode, an incoming message value/velocity will correspond to 25ms increments. A value of 127 only reaches 3175ms, which is a *lot* smaller than the *Fixed* mode limit of 20,000ms. If using this field for a transition, a value of 0 will still allow for usage of the current transition duration.<br>Example: You have a Set Transition action, and you send a message to it with a value of 56. The duration of the transition will be set to 1400ms.

<sub>**[7](#using-the-message-value)**</sub> When using this field in *0-127* mode, an incoming message value/velocity will correspond exactly to the value provided. A value of 0 corresponds to 0, and a value of 127 corresponds to 127.

<sub>**[8](#using-the-message-value)**</sub> When using the MIDI action, all Use Message Value options will correspond directly to the sent MIDI message values, including the message type.

----------------------------------------------------

## Preferences Guide

### Device Interaction

If this option is toggled off, it will turn off all communication between the plugin and the active device.

### MIDI Throughput

When this option is enabled, all messages received by the active device will automatically be routed to the output device specified.

This may cause a feedback loop if the output device is configured to send messages to the *Active Device*. Feedback loops lead to undesired behavior, and occasionally cause crashes, so be careful when using this feature.

### Binding Transfer

Transferring bindings is useful when a lot of bindings cannot be used since they are on another device that is disconnected or unavailable. All information is provided in the plugin itself.

----------------------------------------------------

## Additional Help

If you need additional help, please file an issue [here](https://github.com/nhielost/obs-midi-mg/issues), or post on the OBS forum discussion [here](https://obsproject.com/forum/threads/obs-midi-mg.158407/).