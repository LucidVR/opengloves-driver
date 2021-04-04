# OpenGloves Driver - LucidVR X Fngrs
## 🚨 Please follow instructions in the <a href="https://github.com/LucidVR/opengloves-driver/wiki">wiki</a> for necessary configuration of the driver
The driver won't work unless you configure the SteamVR input bindings and the driver settings.

Developed by:
* Danwillm
* Lucas_VRTech (LucidVR)

## What is this?
This repository contains the OpenVR(SteamVR) driver for a DIY VR Haptic Glove.

__This Repository is a *very early* work in progress. Many things are likely to change.__
Things in this repo are subject to rapid changes and currently is not in a stable release state.

Pre-Built binaries are available in the releases section.
Instructions for building from source code are in the Wiki.

Releases will be available on Steam once the repo hits a stable release.

That said, we are more than happy to help with issues that may arise, skip to the bottom for how to contact us.

## Officically Compatible Hardware:
* LucidVR Gloves - Prototype 3+
* Fngrs by danwillm
* If you've made you're own, we would love to know and incorporate into this project!

## Currently supported:
* Finger flexion tracking
* Positioning from a controller
* Button and joystick inputs
* Communication Protocols:
  - Serial over USB

## Features that are almost certainly going to be supported:
* Positioning from a tracker (If you have some available and don't mind running test driver builds, please do contact us!)
* Communication Protocols:
  - Bluetooth LE
  - Single port serial

### Considered additions:
* Finger splay tracking
* Force feedback haptics
* Communication Protocols:
  - 2.4ghz wifi
  - Hex compression for serial

## Games tested:
* Half Life Alyx (compatible)
* SteamVR Home (compatible) (finger tracking compatible **with** knuckles emulation)
* The Lab (compatible)
* Aperture Hand Lab (compatible)
* Pavlov (compatible)
* Boneworks (compatible **with** knuckles emulation)
* Blade and Sorcery (known issues)
* VRChat (compatible **with** knuckles emulation)
* Have a game you've tested that does/doesn't work? Please let us know!

### What's knuckles emulation?
Some applications are picky with the controllers they support, we've found this to be the case with Boneworks and VRChat. To get around this issue, we "emulate" a knuckle controller and pass in the values expected for the skeleton, buttons, joysticks etc.
This should make the gloves compatible with any game that is able to use an index controller, with finger tracking as an added bonus.
You are welcome to emulate knuckle controllers for all games, but we recommend switching back to using the lucidglove controllers in the games compatible with it listed above.

#### How do I enable knuckles emulation?
To enable knuckle emulation go to `resources/settings/default.vrsettings` and set `device_driver` to `1`. When launching SteamVR you should see the knuckle icons appear, as well as the controllers appear in game.  
To switch back to using the lucidglove controllers, set `device_driver` to `0`.

## Contributions
We are actively welcoming contributions, feel free to open a pull request.
## Issues
If you run into issues, you are more than welcome to open a GitHub issue/discussion, or contact us directly on Discord: 
`danwillm#8254`  
`LucidVR#0001`
