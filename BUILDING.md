Binaries are (or will) be provided in the Releases in this Repository, but if you would prefer to build the driver yourself instructions are documented below.

To build the driver, you will need:
* Visual Studio 2019 (https://visualstudio.microsoft.com/)

This repo contains everything required to build the driver. Either clone/download zip and extract to your desired location

Once downloaded, open the driver from the Visual Studio dashboard by clicking Open Project/Solution. Navigate to where you downloaded the repo, go through to `LucidGloves` and open the `.sln` file.

Once opened, choose the `Release` configuration (debugging is covered on `Debugging the Driver` page). Press `Ctrl+Shift+B` to build without running. You should see the build successfully complete.

The binaries are built to the top level of the project, navigate into it and copy the `.dll`.

Inside the project folder `LucidGloves`, there is a folder named `lucidgloves`, which is the folder containing all files needed for the driver to operate.

The `.dll` needs to be copied there, at the location: `lucidgloves/bin/win64/`.

Make sure that the filename of the dll is `driver_lucidgloves.dll`.

Copy this whole folder (`lucidgloves`) to the location you installed SteamVR to, inside the `driver` folder. This is usually located at `C:\Program Files (x86)\Steam\steamapps\common\SteamVR\drivers`.

Carry out the steps located in `Configuring the Driver`.
