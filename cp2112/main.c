#include <stdio.h>
#include <windows.h>
#include "SLABCP2112.h"  // CP2112 HID-to-SMBus header
#include "adapter.h"

      adapterStruct adapterHandle;

int main() {

    if( initAdapter(&adapterHandle) != 0) {
        getchar();
        return 0;
    }

    // Write a single byte (0xAA) to slave at address 0x50, to register 0x10
    BYTE writeBuffer[2];
    writeBuffer[0] = 0x10;  // Register address
    writeBuffer[1] = 0xAA;  // Data byte

    if (adapterHandle.HidSmbus_WriteRequestX(adapterHandle.device, 0x50, writeBuffer, 2) == HID_SMBUS_SUCCESS) {
        printf("I2C Write success: slave=0x50 reg=0x10 val=0xAA\n");
    } else {
        printf("I2C Write failed.\n");
    }

    deInitAdapter(&adapterHandle);
    return 0;
}
