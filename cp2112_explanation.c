/**
DRIVER AND SDK installation

Load page for drivers (MacOS, Linux, Windows)
https://www.silabs.com/development-tools/interface/cp2112ek-development-kit?tab=software-tools
for the Windows is here:
https://www.silabs.com/documents/public/software/USBXpressHostSDK-Win.zip
Run the installer USBXpressHostSDK-6.7.7-Win.msi
After installation, there will be main folder "SiliconLabs" on the computer.
1)The dynamic link library  is here :   D:\SiliconLabs\USBXpressHostSDK\CP2112\Release\x64\SLABHIDtoSMBus.dll,
upload all the DLLs in your C /C++ project because there may be dependencies;
  Moreover there are also statical and Python libraries.
2)The header  file is here:  D:\SiliconLabs\USBXpressHostSDK\CP2112\include\SLABHIDDevice.h
*/
