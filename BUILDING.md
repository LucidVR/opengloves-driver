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
* Run CMake
  * `cmake .`

This should generate Visual Studio project files, which you can then open.

# Building with Visual Studio  
* You should alread have the ability to build the driver by pressing `Ctrl + Shift + B`
  * The artifacts of the build will be outputted to `Debug/`, or `Release/` depending on build configuration
* You can copy the `openglove` fodler into the steamvr drivers folder
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
		"E:\\opengloves-driver\\Debug" <=================
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
* Right click on the project `driver_openglove`
  * Select `properties`. 
  * Navigate to the `Debugger` Property (under Configuration Properties)
  * Set `Command` to the location of `vrstartup.exe`
    * This is usually located `C:\Program Files (x86)\Steam\steamapps\common\SteamVR\bin\win64\vrstartup.exe`
