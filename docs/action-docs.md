# obs-midi-mg Actions Page

## Contents
1. [None](#none)
2. [Streaming](#streaming)
3. [Recording](#recording)
4. [Virtual Camera](#virtual-camera)
5. [Replay Buffer](#replay-buffer)
6. [Studio Mode](#studio-mode)
7. [Scene Switching](#scene-switching)
8. [Video Sources](#video-sources)
9. [Audio Sources](#audio-sources)
10. [Media Sources](#media-sources)
11. [Transitions](#transitions)
12. [Filters](#filters)
13. [Hotkeys](#hotkeys)
14. [Profiles](#profiles)
15. [Scene Collections](#scene-collections)
16. [MIDI](#midi)


## None

| Input Action | Description                     | Fields |
|--------------|---------------------------------|--------|
| None         | Does exactly what you'd expect. | *N/A*  |

| Output Action | Description       | Fields |
|---------------|-------------------|--------|
| None          | Will never occur. | *N/A*  |

## Streaming

| Input Action     | Description                                                                   | Fields |
|------------------|-------------------------------------------------------------------------------|--------|
| Start Streaming  | Starts the stream if it is not running.                                       | *N/A*  |
| Stop Streaming   | Stops the stream if it is running.                                            | *N/A*  |
| Toggle Streaming | Starts the stream if it is not running, or stops the stream if it is running. | *N/A*  |

| Output Action          | Description                                                                       | Fields |
|------------------------|-----------------------------------------------------------------------------------|--------|
| Stream Starting        | Occurs when a stream is starting (i.e. the *Start Streaming* button was clicked). | *N/A*  |
| Stream Started         | Occurs when a stream is started (i.e. the stream connection was successful).      | *N/A*  |
| Stream Stopping        | Occurs when a stream is stopping (i.e. the *Stop Streaming* button was clicked).  | *N/A*  |
| Stream Stopped         | Occurs when a stream is stopped (i.e. the stream is no longer active in any way). | *N/A*  |
| Toggle Stream Starting | Occurs if either **Stream Starting** or **Stream Stopping** occurs.               | *N/A*  |
| Toggle Stream Started  | Occurs if either **Stream Started** or **Stream Stopped** occurs.                 | *N/A*  |
                       
## Recording

| Input Action           | Description                                                                 | Fields |
|------------------------|-----------------------------------------------------------------------------|--------|
| Start Recording        | Starts recording if it is not running.                                      | *N/A*  |
| Stop Recording         | Stops recording if it is running.                                           | *N/A*  |
| Toggle Recording       | Starts recording if it is not running, or stops recording if it is running. | *N/A*  |
| Pause Recording        | Pauses recording if it is running.                                          | *N/A*  |
| Resume Recording       | Resumes recording if it is paused.                                          | *N/A*  |
| Toggle Pause Recording | Pauses recording if it is running, or resumes recording if it is paused.    | *N/A*  |

| Output Action             | Description                                                                             | Fields |
|---------------------------|-----------------------------------------------------------------------------------------|--------|
| Recording Starting        | Occurs when a recording is starting (i.e. the *Start Recording* button was clicked).    | *N/A*  |
| Recording Started         | Occurs when a recording is started (i.e. the recording start was successful).           | *N/A*  |
| Recording Stopping        | Occurs when a recording is stopping (i.e. the *Stop Recording* button was clicked).     | *N/A*  |
| Recording Stopped         | Occurs when a recording is stopped (i.e. the recording is no longer active in any way). | *N/A*  |
| Toggle Recording Starting | Occurs if either **Recording Starting** or **Recording Stopping** occurs.               | *N/A*  |
| Toggle Recording Started  | Occurs if either **Recording Started** or **Recording Stopped** occurs.                 | *N/A*  |
| Recording Paused          | Occurs when a recording is paused (i.e. the pause button was clicked).                  | *N/A*  |
| Recording Resumed         | Occurs when a recording is started (i.e. the play button was clicked).                  | *N/A*  |
| Toggle Recording Paused   | Occurs if either **Recording Paused** or **Recording Resumed** occurs.                  | *N/A*  |

## Virtual Camera

| Input Action          | Description                                                                                   | Fields |
|-----------------------|-----------------------------------------------------------------------------------------------|--------|
| Start Virtual Camera  | Starts the virtual camera if it is not running.                                               | *N/A*  |
| Stop Virtual Camera   | Stops the virtual camera if it is running.                                                    | *N/A*  |
| Toggle Virtual Camera | Starts the virtual camera if it is not running, or stops the virtual camera if it is running. | *N/A*  |

| Output Action                 | Description                                                                         | Fields |
|-------------------------------|-------------------------------------------------------------------------------------|--------|
| Virtual Camera Started        | Occurs when the virtual camera is started (i.e. the connection was successful).     | *N/A*  |
| Virtual Camera Stopped        | Occurs when the virtual camera is stopped (i.e. it is no longer active in any way). | *N/A*  |
| Toggle Virtual Camera Started | Occurs if either **Virtual Camera Started** or **Virtual Camera Stopped** occurs.   | *N/A*  |

## Replay Buffer

| Input Action         | Description                                                                      | Fields |
|----------------------|----------------------------------------------------------------------------------|--------|
| Start Replay Buffer  | Starts a Replay Buffer.                                                          | *N/A*  |
| Stop Replay Buffer   | Stops a Replay Buffer if one has started.                                        | *N/A*  |
| Toggle Replay Buffer | Starts a Replay Buffer if there is not one active, or stops it if it is running. | *N/A*  |
| Save Replay Buffer   | Saves the Replay Buffer to a file.                                               | *N/A*  |

| Output Action                 | Description                                                                       | Fields |
|-------------------------------|-----------------------------------------------------------------------------------|--------|
| Replay Buffer Starting        | Occurs when a replay buffer is starting.                                          | *N/A*  |
| Replay Buffer Started         | Occurs when a replay buffer is started.                                           | *N/A*  |
| Replay Buffer Stopping        | Occurs when a replay buffer is stopping.                                          | *N/A*  |
| Replay Buffer Stopped         | Occurs when a replay buffer is stopped.                                           | *N/A*  |
| Toggle Replay Buffer Starting | Occurs if either **Replay Buffer Starting** or **Replay Buffer Stopping** occurs. | *N/A*  |
| Toggle Replay Buffer Started  | Occurs if either **Replay Buffer Started** or **Replay Buffer Stopped** occurs.   | *N/A*  |
| Replay Buffer Saved           | Occurs when a replay buffer is saved to a file.                                   | *N/A*  |

## Studio Mode

| Input Action           | Description                                                                                                           | Fields                                                     |
|------------------------|-----------------------------------------------------------------------------------------------------------------------|------------------------------------------------------------|
| Activate Studio Mode   | Turns on Studio Mode.                                                                                                 | *N/A*                                                      |
| Deactivate Studio Mode | Turns off Studio Mode.                                                                                                | *N/A*                                                      |
| Toggle Studio Mode     | Turns on Studio Mode if it is off, or turns it off if it is on.                                                       | *N/A*                                                      |
| Change Preview Scene   | Changes the scene displayed in the preview in Studio Mode.<br>Does nothing if Studio Mode is disabled.                | **SCENE**: The name of the scene to switch the preview to. |
| Preview to Program     | Switches the preview and the program scenes using the current transition.<br>Does nothing if Studio Mode is disabled. | *N/A*                                                      |

| Output Action                | Description                                                                                                   | Fields                                                |
|------------------------------|---------------------------------------------------------------------------------------------------------------|-------------------------------------------------------|
| Studio Mode Activated        | Occurs when Studio Mode is activated (i.e. the *Studio Mode* button was clicked).                             | *N/A*                                                 |
| Studio Mode Deactivated      | Occurs when Studio Mode is deactivated.                                                                       | *N/A*                                                 |
| Toggle Studio Mode Activated | Occurs if either **Studio Mode Activated** or **Studio Mode Deactivated** occurs.                             | *N/A*                                                 |
| Preview Scene Changed        | Occurs if the Studio Mode preview scene is switched to **SCENE**.<br>Does nothing if Studio Mode is disabled. | **SCENE**: The name of the preview scene to look for. |

## Scene Switching

| Input Action    | Description                                   | Fields                                         |
|-----------------|-----------------------------------------------|------------------------------------------------|
| Scene Switching | Switches scenes using the current transition. | **SCENE**: The name of the scene to switch to. |

| Output Action  | Description                                           | Fields                                        |
|----------------|-------------------------------------------------------|-----------------------------------------------|
| Scene Switched | Occurs if the current scene is switched to **SCENE**. | **SCENE**: The name of the scene to look for. |

## Video Sources

All Video Sources actions contain these fields in addition to the fields listed in the table below:

**SCENE**: The name of the scene containing the source.

**SOURCE**: The name of the source. This can be any source, even if it does not display video.

| Input Action               | Description                                                                                                                     | Fields                                                                                                                                                                                                                                                                                                 |
|----------------------------|---------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Move Source                | Moves the source to the specified coordinates on the current scene.<br>NOTE: Behavior for sources in groups is not predictable. | **POSITION X**: The x-coordinate of the new position of the source.<br>**POSITION Y**: The y-coordinate of the new position of the source.                                                                                                                                                             |
| Display Source             | Shows or hides the source on the current scene.                                                                                 | **STATE**: The display state of the source.                                                                                                                                                                                                                                                            |
| Source Locking             | Locks or unlocks the source on the current scene.                                                                               | **STATE**: The lock state of the source.                                                                                                                                                                                                                                                               |
| Source Crop                | Crops the source on the current scene.                                                                                          | **TOP**: The number of pixels to crop from the top of the source.<br>**RIGHT**: The number of pixels to crop from the right side of the source.<br>**BOTTOM**: The number of pixels to crop from the bottom of the source.<br>**LEFT**: The number of pixels to crop from the left side of the source. |
| Align Source               | Changes the alignment of the position of the source on the current scene.                                                       | **ALIGNMENT**: The alignment state of the source.                                                                                                                                                                                                                                                      |
| Source Scale               | Scales the source by a specified amount and magnitude on the current scene.                                                     | **SCALE X**: The percent scale of the x-axis of the source.<br>**SCALE Y**: The percent scale of the y-axis of the source.<br>**MAGNITUDE**: The amount of scaling to apply. A magnitude of 1 is the default, and is appropriate for most cases.                                                       |
| Source Scale Filtering     | Changes the scale filtering of the source on the current scene.                                                                 | **FILTERING**: The scale filtering state of the source.                                                                                                                                                                                                                                                |
| Rotate Source              | Rotates the source by a specified number of degrees on the current scene.                                                       | **ROTATION**: The rotation of the source in degrees.                                                                                                                                                                                                                                                   |
| Source Bounding Box Type   | Changes the type of bounding box of the source on the current scene.                                                            | **TYPE**: The bounding box state of the source.                                                                                                                                                                                                                                                        |
| Resize Source Bounding Box | Resizes the bounding box of the source on the current scene.                                                                    | **SIZE X**: The new length of the bounding box of the source.<br>**SIZE Y**: The new height of the bounding box of the source.                                                                                                                                                                         |
| Align Source Bounding Box  | Changes the alignment of the bounding box of the source on the current scene.                                                   | **ALIGNMENT**: The alignment state of the bounding box of the source.                                                                                                                                                                                                                                  |
| Source Blending Mode       | Changes the blending mode of the source on the current scene.                                                                   | **BLEND MODE**: The blend mode state of the source.                                                                                                                                                                                                                                                    |
| Take Source Screenshot     | Takes a screenshot of the source.                                                                                               | *N/A*                                                                                                                                                                                                                                                                                                  |
| Custom Source Settings     | Changes custom properties of a source.                                                                                          | These fields are defined by the source.                                                                                                                                                                                                                                                                |

| Output Action                    | Description                                                                                                       | Fields                                                                                                                                                                                                                                                                                                 |
|----------------------------------|-------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Source Moved                     | Occurs when the source is moved on the current scene.<br>NOTE: Behavior for sources in groups is not predictable. | **POSITION X**: The x-coordinate of the position of the source.<br>**POSITION Y**: The y-coordinate of the position of the source.                                                                                                                                                                     |
| Source Display Changed           | Occurs when the source on the current scene is **STATE**.                                                         | **STATE**: The display state of the source.                                                                                                                                                                                                                                                            |
| Source Locked                    | Occurs when the source on the current scene is **STATE**.                                                         | **STATE**: The lock state of the source.                                                                                                                                                                                                                                                               |
| Source Cropped                   | Occurs when the source on the current scene is cropped.                                                           | **TOP**: The number of pixels cropped from the top of the source.<br>**RIGHT**: The number of pixels cropped from the right side of the source.<br>**BOTTOM**: The number of pixels cropped from the bottom of the source.<br>**LEFT**: The number of pixels cropped from the left side of the source. |
| Source Aligned                   | Occurs when the alignment of the source on the current scene is **ALIGNMENT**.                                    | **ALIGNMENT**: The alignment state of the source.                                                                                                                                                                                                                                                      |
| Source Scaled                    | Occurs when the scale of the source on the current scene is changed.                                              | **SCALE X**: The percent scale of the x-axis of the source.<br>**SCALE Y**: The percent scale of the y-axis of the source.<br>**MAGNITUDE**: The amount of scaling to apply. A magnitude of 1 is the default, and is appropriate for most cases.                                                       |
| Source Scale Filtering Changed   | Occurs when the scale filtering of the source on the current scene is **FILTERING**.                              | **FILTERING**: The scale filtering state of the source.                                                                                                                                                                                                                                                |
| Source Rotated                   | Occurs when the rotation of the source on the current scene is **ROTATION**.                                      | **ROTATION**: The rotation of the source in degrees.                                                                                                                                                                                                                                                   |
| Source Bounding Box Type Changed | Occurs when the type of bounding box of the source on the current scene ia **TYPE**.                              | **TYPE**: The bounding box state of the source.                                                                                                                                                                                                                                                        |
| Source Bounding Box Resized      | Occurs when the bounding box of the source on the current scene is changed.                                       | **SIZE X**: The length of the bounding box of the source.<br>**SIZE Y**: The height of the bounding box of the source.                                                                                                                                                                                 |
| Source Bounding Box Aligned      | Occurs when the alignment of the bounding box of the source on the current scene is **ALIGNMENT**.                | **ALIGNMENT**: The alignment state of the bounding box of the source.                                                                                                                                                                                                                                  |
| Source Blending Mode Changed     | Occurs when the blending mode of the source on the current scene is **BLEND MODE**.                               | **BLEND MODE**: The blend mode state of the source.                                                                                                                                                                                                                                                    |
| Screenshot Taken                 | Occurs when a screenshot is taken within OBS. *This action is not source-specific*.                               | *N/A*                                                                                                                                                                                                                                                                                                  |
| Custom Source Settings Changed   | Occurs when custom properties of a source change.                                                                 | There are currently no fields for this action.                                                                                                                                                                                                                                                         |

## Audio Sources

All Audio Sources actions contain these fields in addition to the fields listed in the table below:

**SOURCE**: The name of the audio source.

| Input Action             | Description                                                                                           | Fields                                              |
|--------------------------|-------------------------------------------------------------------------------------------------------|-----------------------------------------------------|
| Change Source Volume     | Sets the volume of the audio source to the specified decibel level (no longer a percentage value).    | **VOLUME**: The volume to set in decibels.          |
| Increment Source Volume  | Changes the volume of the audio source by the specified decibel level (no longer a percentage value). | **VOLUME**: The volume increment in decibels.       |
| Mute Source              | Mutes the audio source.                                                                               | *N/A*                                               |
| Unmute Source            | Unmutes the audio source.                                                                             | *N/A*                                               |
| Toggle Source Mute       | Mutes the audio source if unmuted, unmutes otherwise.                                                 | *N/A*                                               |
| Source Audio Sync Offset | Changes the audio sync offset of a source.                                                            | **OFFSET**: The audio sync offset in milliseconds.  |
| Source Audio Monitor     | Changes the audio monitor of a source.                                                                | **MONITOR**: The audio monitor state of the source. |

| Output Action                    | Description                                                     | Fields                                           |
|----------------------------------|-----------------------------------------------------------------|--------------------------------------------------|
| Source Volume Changed            | Occurs when the volume of the current source changes.           | **VOLUME**: The volume to look for.              |
| Source Muted                     | Occurs when the current source is muted.                        | *N/A*                                            |
| Source Unmuted                   | Occurs when the current source is unmuted.                      | *N/A*                                            |
| Toggle Source Muted              | Occurs if either **Source Muted** or **Source Unmuted** occurs. | *N/A*                                            |
| Source Audio Sync Offset Changed | Occurs when the sync offset of the current source changes.      | **OFFSET**: The sync offset to look for, if any. |
| Source Audio Monitor Changed     | Occurs when the monitor of the current source changes.          | **MONITOR**: The monitor to look for, if any.    |

### Media Sources

All Media Sources actions contain these fields in addition to the fields listed in the table below:

**SOURCE**: The name of the media source.

| Input Action        | Description                                                                                                     | Fields                                             |
|---------------------|-----------------------------------------------------------------------------------------------------------------|----------------------------------------------------|
| Play or Pause Media | Pauses the media source if playing, starts playing otherwise.                                                   | *N/A*                                              |
| Restart Media       | Restarts the media source if playing.                                                                           | *N/A*                                              |
| Stop Media          | Stops the media source.                                                                                         | *N/A*                                              |
| Set Track Time      | Sets the time of the media source.<br>(In MIDI mode, this will adjust to the time values of the current track.) | **TIME**: The time to set to the media source.     |
| Next Track          | If there are multiple tracks in the media source, advance to the next one.                                      | *N/A*                                              |
| Previous Track      | If there are multiple tracks in the media source, go back to the previous one.                                  | *N/A*                                              |
| Skip Forward Time   | Advances a specified amount of time in the media source.                                                        | **TIME**: The time to advance in the media source. |
| Skip Backward Time  | Rewinds a specified amount of time in the media source.                                                         | **TIME**: The time to rewind in the media source.  |

| Output Action        | Description                                                       | Fields |
|----------------------|-------------------------------------------------------------------|--------|
| Media Played         | Occurs when the media source starts playing.                      | *N/A*  |
| Media Paused         | Occurs when the media source is paused.                           | *N/A*  |
| Media Restarted      | Occurs when the media source is restarted.                        | *N/A*  |
| Media Stopped        | Occurs when the media source is stopped.                          | *N/A*  |
| Media Track Next     | Occurs when the media source track is advanced, if applicable.    | *N/A*  |
| Media Track Previous | Occurs when the media source track is backtracked, if applicable. | *N/A*  |

### Transitions
          
| Input Action                              | Description                                                                                                                                              | Fields                                                                                                                                                                                                                                        |
|-------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Change Current Transition                 | Sets the current transition.                                                                                                                             | **TRANSITION**: The name of the transition to use.<br>**DURATION**: The duration of the transition selected. If ignored, the current transition duration is used.                                                                             |
| Set Source Show Transition                | Sets the transition that will be used when the specified source becomes shown.                                                                           | **SCENE**: The name of the scene containing the video source.<br>**SOURCE**: The name of the source to set the transition to.<br>**TRANSITION**: The name of the transition to use.<br>**DURATION**: The duration of the transition selected. |
| Set Source Hide Transition                | Sets the transition that will be used when the specified source becomes hidden.                                                                          | **SCENE**: The name of the scene containing the video source.<br>**SOURCE**: The name of the source to set the transition to.<br>**TRANSITION**: The name of the transition to use.<br>**DURATION**: The duration of the transition selected. |
| Set Transition Bar Position (Studio Mode) | Sets the transition bar position. This will automatically release the transition bar after 1 second of inactivity.<br>Fails if not in Studio Mode.       | **POSITION**: The amount transitioned by the transition bar.                                                                                                                                                                                  |
| Toggle Transition Bar (Studio Mode)       | Toggles the availability of the transition bar. Causes all *Set Transition Bar Position* actions to fail if toggled off.<br>Fails if not in Studio Mode. | *N/A*                                                                                                                                                                                                                                         |
| Custom Transition Settings                | Changes custom properties of a transition.                                                                                                               | These fields are defined by the source.                                                                                                                                                                                                       |

| Output Action                       | Description                                                               | Fields                                                                                                                               |
|-------------------------------------|---------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------|
| Current Transition Changed          | Occurs when the current transition changes.                               | **TRANSITION**: The name of the transition to listen for. If ignored, any transition change will be listened for.                    |
| Current Transition Duration Changed | Occurs when the current transition duration changes.                      | **DURATION**: The duration of the current transition to listen for. If ignored, any transition duration change will be listened for. |
| Transition Started                  | Occurs when the specified transition begins.                              | **TRANSITION**: The name of the transition to listen for. If ignored, any transition change will be listened for.                    |
| Transition Stopped                  | Occurs when the specified transition ends.                                | **TRANSITION**: The name of the transition to listen for. If ignored, any transition change will be listened for.                    |
| Toggle Transition Started           | Occurs if either **Transition Started** or **Transition Stopped** occurs. | **TRANSITION**: The name of the transition to listen for. If ignored, any transition change will be listened for.                    |
| Transition Bar Moved (Studio Mode)  | Occurs when the transition bar value changes.                             | **POSITION**: The amount transitioned by the transition bar to listen for.                                                           |
| Custom Transition Settings Changed  | Occurs when custom properties of a transition change.                     | There are currently no fields for this action.                                                                                       |

### Filters

All Filters actions contain these fields in addition to the fields listed in the table below:

**SOURCE**: The name of the source containing the filter.

**FILTER**: The name of the filter.

| Input Action              | Description                                                               | Fields                                                    |
|---------------------------|---------------------------------------------------------------------------|-----------------------------------------------------------|
| Show Filter               | Shows a filter on a specified source.                                     | *N/A*                                                     |
| Hide Filter               | Hides a filter on a specified source.                                     | *N/A*                                                     |
| Toggle Filter Display     | Shows a filter on a specified source if it is hidden, hides it otherwise. | *N/A*                                                     |
| Reorder Filter Appearance | Moves filter up and down the list on a specified source.                  | **POSITION**: The new position of the filter in the list. |
| Custom Filter Settings    | Changes custom properties of a filter.                                    | These fields are defined by the source.                   |

| Output Action                  | Description                                                       | Fields                                         |
|--------------------------------|-------------------------------------------------------------------|------------------------------------------------|
| Filter Shown                   | Occurs when the specified filter on a specified source is shown.  | *N/A*                                          |
| Filter Hidden                  | Occurs when the specified filter on a specified source is hidden. | *N/A*                                          |
| Toggle Filter Shown            | Occurs if either **Filter Shown** or **Filter Hidden** occurs.    | *N/A*                                          |
| Filters Reordered              | Occurs if any filters are reordered on a specified source.        | *N/A*                                          |
| Custom Filter Settings Changed | Occurs when custom properties of a filter change.                 | There are currently no fields for this action. |

### Hotkeys

| Input Action               | Description                          | Fields                                                                                                                       |
|----------------------------|--------------------------------------|------------------------------------------------------------------------------------------------------------------------------|
| Activate Predefined Hotkey | Activates a hotkey via a MIDI event. | **GROUP**: The category of hotkeys as found in the Hotkeys settings menu.<br>**HOTKEY**: The name of the hotkey to activate. |

| Output Action                   | Description                                                                                                 | Fields                                                                                                           |
|---------------------------------|-------------------------------------------------------------------------------------------------------------|------------------------------------------------------------------------------------------------------------------|
| Hotkey Activated (Experimental) | Occurs when the specified hotkey is activated.<br>*This action is not guaranteed to work 100% of the time.* | **GROUP**: The category of hotkeys as found in the Hotkeys settings menu.<br>**HOTKEY**: The name of the hotkey. |

### Profiles

All Profiles actions contain this field:

**PROFILE**: The name of the profile being switched.

| Input Action    | Description                                                                     | Fields |
|-----------------|---------------------------------------------------------------------------------|--------|
| Switch Profiles | Switches to another Profile.<br>Fails if a stream / recording / etc. is active. | *N/A*  |

| Output Action           | Description                                                                                | Fields |
|-------------------------|--------------------------------------------------------------------------------------------|--------|
| Profile Changing        | Occurs if the current Profile begins to change (i.e. the user clicks a different profile). | *N/A*  |
| Profile Changed         | Occurs if the current Profile has changed completely.                                      | *N/A*  |
| Toggle Profile Changing | Occurs if either **Profile Changing** or **Profile Changed** occurs.                       | *N/A*  |

### Scene Collections

All Scene Collections actions contain this field:

**SCENE COLLECTION**: The name of the scene collection being switched.

| Input Action             | Description                           | Fields |
|--------------------------|---------------------------------------|--------|
| Switch Scene Collections | Switches to another scene collection. | *N/A*  |

| Output Action                    | Description                                                                                                  | Fields |
|----------------------------------|--------------------------------------------------------------------------------------------------------------|--------|
| Scene Collection Changing        | Occurs if the current scene collection begins to change (i.e. the user clicks a different scene collection). | *N/A*  |
| Scene Collection Changed         | Occurs if the current scene collection has changed completely.                                               | *N/A*  |
| Toggle Scene Collection Changing | Occurs if either **Scene Collection Changing** or **Scene Collection Changed** occurs.                       | *N/A*  |

### MIDI

| Input Action             | Description                                            | Fields                                  |
|--------------------------|--------------------------------------------------------|-----------------------------------------|
| Send Messages to Devices | Sends MIDI messages to the the specified MIDI devices. | These fields are the same as a message. |

| Output Action            | Description                                                                                                            | Fields                                  |
|--------------------------|------------------------------------------------------------------------------------------------------------------------|-----------------------------------------|
| Messages Sent to Devices | Occurs if ALL messages have been sent to ANY of the devices. *This will toggle the messages if they are set to do so.* | These fields are the same as a message. |
