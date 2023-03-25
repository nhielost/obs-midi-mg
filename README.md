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

Go to the [Releases page](https://github.com/nhielost/obs-midi-mg/releases) and download and install the latest release for the proper operating system. When the install process has completed, open OBS Studio and open the setup window under *Tools > obs-midi-mg Setup* to begin creating bindings.

## Help

If help is needed, click the *Help* button in the plugin, or click [here](HELP.md). 

Some users may find that they want to have multiple programs monitor the same MIDI device (for example, OBS Studio and Ableton). To do this, install a MIDI routing software such as [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html) or [BOME MIDI Translator](https://www.bome.com/products/mtclassic). This will allow for virtual devices to be created, and, when configured, multiple programs can connect to these seamlessly.

I would love to hear honestly from you about this plugin. Feel free to share some ideas and don't be afraid to [report an issue](https://github.com/nhielost/obs-midi-mg/issues) or [post on the OBS forum discussion](https://obsproject.com/forum/threads/obs-midi-mg.158407/)!

## Credits

I have to give a shoutout to [@cpyarger](https://github.com/cpyarger) and [@Alzy](https://github.com/alzy) for the inspiration of this plugin. Many ideas found in obs-midi-mg can be found in their plugin. Without [obs-midi](https://github.com/cpyarger/obs-midi/), obs-midi-mg would have never been possible. 