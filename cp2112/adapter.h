#ifndef ADAPTER_H_INCLUDED
#define ADAPTER_H_INCLUDED
#include <windows.h>
#include "SLABCP2112.h"  // CP2112 HID-to-SMBus header

// Define function pointer types for HID-to-SMBus DLL
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_GetNumDevices)(DWORD * lpdwNumDevices, const WORD vid,	const WORD 	pid );
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_Open) 	( 	HID_SMBUS_DEVICE *pdevice,const DWORD  deviceNum,const WORD	vid,const WORD	pid	);
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_Close)(HID_SMBUS_DEVICE);
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_WriteRequest)	(	const HID_SMBUS_DEVICE 	device,
		const BYTE  	slaveAddress,
		BYTE *  	buffer,
		const BYTE  	numBytesToWrite
	);
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_SetTimeouts)(const HID_SMBUS_DEVICE device,const DWORD  responseTimeout );
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_SetSmbusConfig)( 	const HID_SMBUS_DEVICE  device,
                                                                const DWORD  	bitRate,
                                                                const BYTE  	address,
                                                                const BOOL  	autoReadRespond,
                                                                const WORD  	writeTimeout,
                                                                const WORD  	readTimeout,
                                                                const BOOL  	sclLowTimeout,
                                                                const WORD  	transferRetries
                                                            );
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_GetOpenedString)( 	const HID_SMBUS_DEVICE 	device,
                                                                char *  	deviceString,
                                                                const HID_SMBUS_GETSTRING  	options
                                                            );
typedef HID_SMBUS_STATUS (WINAPI *pfHidSmbus_GetString)( const DWORD deviceNum,
                                                            const WORD  	vid,
                                                            const WORD  	pid,
                                                            char *  	deviceString,
                                                            const HID_SMBUS_GETSTRING  	options
                                                        );

 typedef struct adapterStruct {
        WORD vendorID;
        WORD productID;
        DWORD numDevices;
        HMODULE hHidSmbus;
        HID_SMBUS_DEVICE device;
        pfHidSmbus_GetNumDevices HidSmbus_GetNumDevicesX;
        pfHidSmbus_Open HidSmbus_OpenX;
        pfHidSmbus_Close HidSmbus_CloseX;
        pfHidSmbus_WriteRequest HidSmbus_WriteRequestX;
        pfHidSmbus_SetTimeouts HidSmbus_SetTimeoutsX;
        pfHidSmbus_SetSmbusConfig HidSmbus_SetSmbusConfigX;
        pfHidSmbus_GetString HidSmbus_GetStringX;
    } adapterStruct;


 int initAdapter(adapterStruct* hAdapter);

 void deInitAdapter(adapterStruct * hAdapter);

#endif // ADAPTER_H_INCLUDED
