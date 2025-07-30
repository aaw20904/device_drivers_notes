#include <windows.h>
#include "SLABCP2112.h"
#include "adapter.h"
#include <stdio.h>




 void initDlls (adapterStruct* hdl) {
      hdl->HidSmbus_GetNumDevicesX = (pfHidSmbus_GetNumDevices) GetProcAddress (hdl->hHidSmbus, "HidSmbus_GetNumDevices");
      hdl->HidSmbus_OpenX = (pfHidSmbus_Open)GetProcAddress (hdl->hHidSmbus, "HidSmbus_Open");
      hdl->HidSmbus_CloseX = (pfHidSmbus_Close)GetProcAddress (hdl->hHidSmbus, "HidSmbus_Close");
      hdl->HidSmbus_WriteRequestX = (pfHidSmbus_WriteRequest) GetProcAddress (hdl->hHidSmbus, "HidSmbus_WriteRequest");
     hdl->HidSmbus_SetTimeoutsX = (pfHidSmbus_SetTimeouts) GetProcAddress (hdl->hHidSmbus, "HidSmbus_SetTimeouts");
      hdl->HidSmbus_SetSmbusConfigX = (pfHidSmbus_SetSmbusConfig) GetProcAddress ( hdl->hHidSmbus, "HidSmbus_SetSmbusConfig");
      hdl->HidSmbus_GetStringX = (pfHidSmbus_GetString) GetProcAddress ( hdl->hHidSmbus, "HidSmbus_GetString");
      return;
 }

  int initAdapter (adapterStruct *hAdapter) {

        HID_SMBUS_STATUS cpStatus;  //status of an operatoin

        hAdapter->vendorID = 0x10c4;
        hAdapter->productID= 0xea90;

     //1) load a libnrary
         hAdapter->hHidSmbus = LoadLibrary("SLABHIDtoSMBus.dll");

    if (!hAdapter->hHidSmbus) {
        printf("Error loading SLABHIDtoSMBus.dll\n");
        return -1;
    }
    //2) Load functions from DLL

    initDlls(hAdapter);

    //3)amount of devices inside cp2112
     cpStatus = hAdapter->HidSmbus_GetNumDevicesX(&hAdapter->numDevices, hAdapter->vendorID, hAdapter->productID);

     if ( cpStatus != SILABS_STATUS_SUCCESS ) {
        printf("Error CP2112 device opening or not found.\n");
        FreeLibrary(hAdapter->hHidSmbus);
        return 1;
    }
    //4)When found - show result
    printf("Found %lu CP2112 device(s).\n", hAdapter->numDevices);

    //5)Try to open the device:
    cpStatus = hAdapter->HidSmbus_OpenX(&hAdapter->device, 0, hAdapter->vendorID, hAdapter->productID);

        if ( cpStatus != HID_SMBUS_SUCCESS) {
        printf("Failed to open device.\n");
        FreeLibrary(hAdapter->hHidSmbus);
        return 1;
    }

    //6) When the IC has opened:
     printf("Device opened.\n");

    // 7)Set I2C timeouts (read 1000ms, write 1000ms)
    cpStatus = hAdapter->HidSmbus_SetTimeoutsX(hAdapter->device, 1000);

    if (cpStatus != HID_SMBUS_SUCCESS) {
       printf("Error of timeout initialization!");
       FreeLibrary(hAdapter->hHidSmbus);
       return 1;
    }
    // 8)Configure SMBus: clockSpeed=100kHz, retries=3, timeout=0, enable autoRead etc.
    // 	device, bitRate, address, autoReadRespond,writeTimeout,	readTimeout, BOOL  sclLowTimeout, transferRetries
    cpStatus = hAdapter->HidSmbus_SetSmbusConfigX(hAdapter->device, 100000, 32, 0, 0, 10, FALSE, 2);

     if (cpStatus != HID_SMBUS_SUCCESS) {
       printf("Error of clock configuration!");
       FreeLibrary(hAdapter->hHidSmbus);
       return 1;
    }
    return 0;

  }


  void deInitAdapter(adapterStruct * hAdapter) {
      // Close the device
    hAdapter->HidSmbus_CloseX(hAdapter->device);
    FreeLibrary(hAdapter->hHidSmbus);
  }
