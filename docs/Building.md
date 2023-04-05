# Building OpenGloves
If you'd like to make changes to OpenGloves, follow the instructions below to set up a local copy of the repository.

If you're looking to use OpenGloves to use your hardware in SteamVR applications, it is heavily recommended to install the driver on Steam.

## Setting Up
Pre-built binaries are be provided in the Releases in this Repository, but if you would prefer to build the driver yourself instructions are documented below. Be sure to uninstall the driver from Steam if you do this.

* Clone the repo **(including submodules)**
  * `git clone https://github.com/LucidVR/opengloves-driver.git` 

## Generate Project Files
* Ensure that you have cmake installed (along with the path variable set)
  * https://cmake.org/download/
* Ensure that you have `C++ CMake tools for Windows` installed
  * Modify Visual Studio in Visual Studio installer
* Navigate into the project folder
  * `cd opengloves-driver`
* Ensure `vcpkg` is installed
	* If you need to, get [vcpkg](https://vcpkg.io/en/getting-started.html) installed in the base of this repository

```
git clone https://github.com/Microsoft/vcpkg.git
.\vcpkg\bootstrap-vcpkg.bat -disableMetrics
```

* Make a build directory and enter it
    * `mkdir build`
    * `cd build`
* Run CMake
  * `cmake .. -DCMAKE_TOOLCHAIN_FILE="..\vcpkg\scripts\buildsystems\vcpkg.cmake"`

This should generate Visual Studio project files in the `build/` folder, which you can then compile.

## Building with Visual Studio Build Tools
* run a cmake build in the `build/` folder
  * `cmake --build . --config Release`
  * The artifacts of the build will be outputted to `build/Debug/`, or `build/Release/` depending on build configuration

## Building with Visual Studio IDE
* Open the Visual Studio project (.sln file) in the `build/` folder
* You should already have the ability to build the driver by pressing `Ctrl + Shift + B`
  * The artifacts of the build will be outputted to `build/Debug/`, or `build/Release/` depending on build configuration

## Adding driver to Steam
**Note:** For a more streamlined debugging environment, refer to [Debugging with Visual Studio](https://github.com/LucidVR/opengloves-driver/blob/develop/BUILDING.md#debugging-with-visual-studio).  
This step is for people who may not necessarily want to setup a debugging environment, or are testing release builds.  
* Copy the `openglove` folder into the steamvr drivers folder
  * Usually located `C:\Program Files (x86)\Steam\steamapps\common\SteamVR\drivers`
* Run SteamVR

# Debugging with Visual Studio  
If you want to make changes to the code and would like to use a debugger/not have to copy builds by hand, you are able to do so with the following steps:

* Navigate to `C:\Users\<user>\AppData\Local\openvr`
  * Open `openvrpaths.vrpath`.
* Append the `build/<build_type>/openglove` path of the project repository to `external_drivers`. For example:

```
{
	"config" : 
	[
		"C:\\Program Files (x86)\\Steam\\config"
	],
	"external_drivers" : 
	[
		"E:\\opengloves-driver\\build\\driver\\opengloves" <============== (or equivalent driver path & name)
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

## Debugger Setup
SteamVR launches vrserver.exe under a child process of vrstartup.exe, which means that you must have the ability to be able to debug child processes in order to debug your driver. Microsoft provides the Child Process Debugging Power Tool for this.
* Install the Microsoft Process Debugging Tool
  * VS 2015, 2017, 2019: https://marketplace.visualstudio.com/items?itemName=vsdbgplat.MicrosoftChildProcessDebuggingPowerTool
  * VS 2022: https://marketplace.visualstudio.com/items?itemName=vsdbgplat.MicrosoftChildProcessDebuggingPowerTool2022

* Navigate to `Debug>Other Debug Targets>Child Process Debug Settings` in the top bar of Visual Studio<br>
<img src="https://user-images.githubusercontent.com/39023874/154146932-2a8b08e8-b2a8-43e6-b043-55202c1e2fbe.png" width="500" height="200" >
<br>

1. Tick `Enable child process debugging`
2. On the first row (with the process name `<All other processes>`, make sure that the `Action` is set to `Do not debug`.  
3. Add a new row (double click on the empty `Process name` underneath `<All other processes>`
4. Add `vrserver.exe` as the process name 
5. Ensure that `Action` is set to `Attach Debugger` <br>
![image](https://user-images.githubusercontent.com/39023874/154147019-390ee21d-cce4-406c-987a-0b824ec32146.png)

## Launch SteamVR when building through Visual Studio
It's usually quite useful to build then automatically launch SteamVR/ for debugging purposes.  

To launch SteamVR for debugging:  
* Click on the arrow next to `Local Windows Debugger`
* Select `ALL_BUILD Debug Properties`
* Navigate to the `Debugger` Property (under Configuration Properties)
* Set `Command` to the location of `vrstartup.exe` (to start just SteamVR) or the Overlay exe (to start the Ovlerlay and SteamVR).
    * SteamVR's entry point (`vrstartup.exe`) is usually located<br> `C:\Program Files (x86)\Steam\steamapps\common\SteamVR\bin\win64\vrstartup.exe`

![image](https://user-images.githubusercontent.com/39023874/154147592-4e55fc13-73cb-4814-ad43-4abecb4fc3f6.png)
