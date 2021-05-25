Binaries are (or will) be provided in the Releases in this Repository, but if you would prefer to build the driver yourself instructions are documented below.

# Setting Up
* Clone the repo **(including submodules)**
  * `git clone --recursive https://github.com/LucidVR/opengloves-driver.git` 
    *If this doesn't clone the submodules correctly, try `git submodule update --init --recursive`   

## Generate Project Files
* Ensure that you have cmake installed (along with the path variable set)
  * https://cmake.org/download/
* Ensure that you have `C++ CMake tools for Windows` installed
  * Modify Visual Studio in Visual Studio installer
* Navigate into the project folder
  * `cd opengloves-driver`
* Make a build directory and enter it
    * `mkdir build`
    * `cd build`
* Run CMake
  * `cmake ..`

This should generate Visual Studio project files in the `build/` folder, which you can then compile.

# Building with Visual Studio Build Tools
* run a cmake build in the `build/` folder
  * `cmake --build . --config Release`
  * The artifacts of the build will be outputted to `build/Debug/`, or `build/Release/` depending on build configuration

# Building with Visual Studio IDE
* Open the Visual Studio project (.sln file) in the `build/` folder
* You should already have the ability to build the driver by pressing `Ctrl + Shift + B`
  * The artifacts of the build will be outputted to `build/Debug/`, or `build/Release/` depending on build configuration

# Adding driver to Steam
**Note:** For a more streamlined debugging environment, refer to [Debugging with Visual Studio](https://github.com/LucidVR/opengloves-driver/blob/develop/BUILDING.md#debugging-with-visual-studio).  
This step is for people who may not necessarily want to setup a debugging environment, or are testing release builds.  
* Copy the `openglove` folder into the steamvr drivers folder
  * Usually located `C:\Program Files (x86)\Steam\steamapps\common\SteamVR\drivers`
* Running SteamVR you should see the driver activate two new controllers

# Debugging with Visual Studio  
If you want to make changes to the code and would like to use a debugger/not have to copy builds by hand, you are able to do so with the following steps:

* Navigate to `C:\Users\<user>\AppData\Local\openvr`
  * Open `openvrpaths.vrpath`.
* Append the build output path of the project to `external_drivers`. For example:

```
{
	"config" : 
	[
		"C:\\Program Files (x86)\\Steam\\config"
	],
	"external_drivers" : 
	[
		"E:\\opengloves-driver\\build\\Debug\\openglove" <============== (or equivalent driver path & name)
	],
	"jsonid" : "vrpathreg",
	"log" : 
	[
		"C:\\Program Files (x86)\\Steam\\logs"
	],
	"runtime" : 
	[
		"C:\\Program Files (x86)\\Steam\\steamapps\\common\\SteamVR"
	],
	"version" : 1
}
```

## Setup the Debugger
* Install the Microsoft Process Debugging Tool
  * https://marketplace.visualstudio.com/items?itemName=vsdbgplat.MicrosoftChildProcessDebuggingPowerTool
* Navigate to `Debug>Other Debug Targets>Child Process Debug Settings`
  * Check `Enable child process debugging`
  * On the first row (with the process name `<All other processes>`, make sure that the `Action` is set to `Do not debug`.  
  * Add a new row (double click on the empty `Process name` underneath `<All other processes>`.  
  * Add `vrserver.exe` as the process name 
  * Ensure that `Action` is set to `Attach Debugger`.  

## Launch SteamVR when building through Visual Studio
It's usually quite useful to build then automatically launch SteamVR for debugging purposes.  
To launch SteamVR for debugging:  
* Click on the arrow next to `Local Windows Debugger`
* Select `ALL_BUILD Debug Properties`
* Navigate to the `Debugger` Property (under Configuration Properties)
* Set `Command` to the location of `vrstartup.exe`
    * This is usually located `C:\Program Files (x86)\Steam\steamapps\common\SteamVR\bin\win64\vrstartup.exe`

![Debug Properties](https://cdn.discordapp.com/attachments/790676300552994826/840985376679002172/unknown.png)
![Debugging Configuration Properties](https://cdn.discordapp.com/attachments/790676300552994826/840985404202549318/unknown.png)
