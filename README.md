# OpenGloves Driver

OpenGloves is an OpenVR Driver for Virtual Reality (DIY) hardware, specifically designed to be used with VR Gloves.

OpenGloves has an [open interface](https://github.com/LucidVR/opengloves-driver/wiki/Driver-Input) for plugging in your own hardware to SteamVR, without worrying about writing any specific OpenVR driver code.

## Features
* Full Finger Tracking
  * Splay support
  * Individual joint support
* Force Feedback Haptics
* Positioning from Controllers/Trackers
   * Automatic calibration for positioning the glove relative to the controller
* Button and Joystick Inputs
   * Trigger, A and B Buttons
   * Joystick X, Y + Button
* Multiple Communication Protocols
   * Serial USB
   * Bluetooth Serial
   * Named Pipes

[If downloaded on Steam](https://store.steampowered.com/app/1574050/OpenGloves), OpenGloves comes with a UI, which includes the following features:
* Edit Driver Settings
* Automatic Calibration for positioning the glove relative to the controller
* Triggering Force Feedback for testing

## Limitations
* Many VR titles do not support finger tracking from **custom** controllers, requiring the need to emulate controller types
   * We emulate the index controller to achieve this compatibility, which means that we are limited to the inputs that the index controller exposes. It is possible to emulate an index controller while providing your own input profiles and bindings, but we have chosen not to include that by default in the driver, as to preserve compatbility with default index controller bindings
   * If you would like to make a custom controller, implement a child class of `DeviceDriver` within the driver and specify your properties and settings there.  
[LucidGloveDriver](https://github.com/LucidVR/opengloves-driver/blob/develop/src/DeviceDriver/LucidGloveDriver.cpp) is an example of a fully custom controller and you are welcome to adapt it to your needs
* Due to how OpenVR works, inputs are not able to be set dynamically 
   * The inputs that we specify for the index controller emulated device are fixed
   * You are welcome to define your own, custom inputs in a custom device, with a different input profile

## Installation and Usage

We strongly recommend downloading the driver from Steam, to recieve automatic updates and UI settings.  

### Steam
[![Steam Release](https://cdn.discordapp.com/attachments/790676300552994826/845412304219537439/openglovessteam.png)](https://store.steampowered.com/app/1574050/OpenGloves)  

### GitHub
While **not recommended**, you can download the [lastest release](https://github.com/LucidVR/opengloves-driver/releases) and install OpenGloves manually.

### Building
If you plan on making changes to the driver, use the instructions in [Building.md](https://github.com/LucidVR/opengloves-driver/blob/develop/BUILDING.md) to build and install the Debug version of the driver to SteamVR.

## Compatibility
### Officially Compatible Hardware
While it is possible for any hardware to be compatible with OpenGloves, the driver is known to be compatible with these projects:

* [LucidGloves](https://github.com/LucidVR/lucidgloves) - [Lucas VRTech](https://github.com/lucas-vrtech)
* [Fngrs](https://github.com/danwillm/Fngrs/) - [danwillm](https://github.com/danwillm)
* Have your own hardware you want to feature here? Let us know!

### Integrating Your Own Hardware
OpenGloves is capable of handling inputs from a range of VR controller hardware, but is designed to be used with VR Gloves, especially with the abstractions OpenGloves provides over the OpenVR Skeletal System to make it easy to plug into with your own glove hardware.

If you'd like to plug in your own hardware to OpenGloves, refer to [Driver Input](https://github.com/LucidVR/opengloves-driver/wiki/Driver-Input) for the encoding scheme, and communication methods available.

LucidVR also has firmware which is [avaiable here](https://github.com/LucidVR/lucidgloves/tree/main/firmware/lucidgloves-firmware) which can be used on Arduino or ESP32 devices, and is compatible with OpenGloves.

### Compatible Games
By emulating index controllers, OpenGloves is compatible with at least finger curling in games that support the index controller.  

For Force Feedback, the selection is more limited. Refer to [Game Compatibility List](https://github.com/LucidVR/opengloves-driver/wiki/Game-Compatibility-List)  

OpenGloves is strictly compatible with games that take input from the OpenVR API.

## Contributing
Pull requests are very welcome. For major changes, please open an issue or contact us first to discuss what you would like to change.

## Authors

* Danwillm (`danwillm#8254`)
* Lucas VRTech (`LucidVR#0001`)

## Discord
https://discord.gg/lucidvr
