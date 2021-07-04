# OpenGloves Driver

OpenGloves is an OpenVR driver for DIY Virtual Reality Gloves. Using OpenVR's driver interface we are able to provide support for many SteamVR/OpenVR games.

## Installation and Usage


### Download on Steam:
[![Steam Release](https://cdn.discordapp.com/attachments/790676300552994826/845412304219537439/openglovessteam.png)](https://store.steampowered.com/app/1574050/OpenGloves)
 * We strongly recommend downloading the driver from Steam, to recieve automatic updates and UI settings.

*Or download the latest on GitHub:*
 * https://github.com/LucidVR/opengloves-driver/releases

**Follow the wiki guide for configuring the driver**
* https://github.com/LucidVR/opengloves-driver/wiki/Configuring-the-Driver
* The driver will not work correctly unless you configure it properly.  

**Problems?**
* Check [Troubleshooting](https://github.com/LucidVR/opengloves-driver/wiki/Troubleshooting)
  * Didn't help? Contact us on the [Community Discord Server](https://discord.com/invite/Y6XTvnHDUC)
## Building
If you want to use the driver as-is, refer to [Installation and Usage](#Installation-and-Usage).  
If you are planning on modifying source files, refer to [BUILDING.md](https://github.com/LucidVR/opengloves-driver/blob/develop/BUILDING.md).

## Compatibility
### Compatible Hardware
* [LucidVR Gloves](https://github.com/LucidVR/lucidgloves-hardware) - Lucas VRTech
* [Fngrs](https://github.com/danwillm/Fngrs/) - danwillm
* Have your own hardware you want to feature here? Let us know!

### Compatible Games
* Refer to [Game Compatibility List](https://github.com/LucidVR/opengloves-driver/wiki/Game-Compatibility-List)
* As this is an OpenVR driver, it is strictly compatible with games that take input from the OpenVR API. Only the games in the list above have been tested to work properly.

### Current features included in the driver
* Finger flexion tracking
* Positioning from controllers + trackers
* Button/Joystick inputs
* Communication Protocols:
  * Serial USB
  * Serial over Bluetooth

### Planned features
* BLE Communication
* Finger splay tracking
* Force feedback haptics
* Vibration haptics


## Contributing
Pull requests are very welcome. For major changes, please open an issue first to discuss what you would like to change.

## Authors

* Danwillm (`danwillm#8254`)
* Lucas VRTech (`LucidVR#0001`)

## Discord
https://discord.gg/RjV9T8jN2G
