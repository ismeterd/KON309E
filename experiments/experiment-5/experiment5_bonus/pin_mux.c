/*
 * Copyright  2018 ,2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_swm.h"
#include "pin_mux.h"

void InitPins(void)
{
    uint32_t IOCON_PIO_config; // This is the struct for keeping the GPIO pin conf
    // See: User Manual Section 8.4.1 Pin Configuration & Fig.10

    CLOCK_EnableClock(kCLOCK_Iocon);   // Enable clock for IOCON block.
    CLOCK_EnableClock(kCLOCK_Swm);     // Enables clock for switch matrix.

    // In this processor, outputs of some peripheral devices
    //  can be connected to any GPIO pin by configuring a "Pin multiplexer".
    // (This is only possible for peripherals with "moveable pins"
    //
    //This code does the configuration.

    /////////////// I2C Configuration /////////////////////////////////

    // Configure PIO_10, (Alakart label: P10)  as : I2C SCL
    // IC package physical pin is: 9,
    // GPIO pin name: PIO0_10
    // Peripheral: I2C, signal: SCL.

    // First configure the GPIO pin characteristics:
    IOCON_PIO_config = (IOCON_PIO_INV_DI |       // Do not invert input
                        IOCON_PIO_I2CMODE_FAST |
                        IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                        IOCON_PIO_CLKDIV0);      // IOCONCLKDIV0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_10, IOCON_PIO_config);

    // Second, connect I2C SCL to PIO0_10 using the multiplexer:
    // I2C SCL is a FIXED pin.
    // It is at Port 0, Pin 10: PIO0_10
    // PIO0_10 is pin #9  on the 32 pin IC package
    SWM_SetFixedPinSelect(SWM0, kSWM_I2C0_SCL, true);


    // Configure PIO_11, (Alakart label: P11)  as : I2C SDA
    // IC package physical pin is: 8,
    // GPIO pin name: PIO0_11
    // Peripheral: I2C, signal: SDA.

    // First configure the GPIO pin characteristics:
    IOCON_PIO_config = (IOCON_PIO_INV_DI |       // Do not invert input
                        IOCON_PIO_I2CMODE_FAST |
                        IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                        IOCON_PIO_CLKDIV0);      // IOCONCLKDIV0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_11, IOCON_PIO_config);

    // Second, connect I2C SDA to PIO0_11 using the multiplexer:
    // I2C SDA is a FIXED pin.
    // It is at Port 0, Pin 11: PIO0_11
    // PIO0_11 is pin #8  on the 32 pin IC package
    SWM_SetFixedPinSelect(SWM0, kSWM_I2C0_SDA, true);




    /////////////// USART Configuration ///////////////////////////////

    // Configure PIO_0, (Alakart label: P0)  as : USART0 RX
    // IC package physical pin is: 24,
    // GPIO pin name: PIO0_0
    // Peripheral: USART0, signal: RX.

    // First configure the GPIO pin characteristics:
    IOCON_PIO_config = (IOCON_PIO_MODE_PULLUP |  // Select pull-up function
                        IOCON_PIO_HYS_EN |       // Enable hysteresis
                        IOCON_PIO_INV_DI |       // Do not invert input
                        IOCON_PIO_OD_DI |        // Disable open-drain function
                        IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                        IOCON_PIO_CLKDIV0);      // IOCONCLKDIV = 0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_0, IOCON_PIO_config);

    // Second, connect USART0 RX to PIO0_0 using the multiplexer:
    // USART0 RX is a moveable pin.
    // It will be connected to Port 0, Pin 0: PIO0_0
    // PIO0_0 is pin #24  on the 32 pin IC package
    SWM_SetMovablePinSelect(SWM0, kSWM_USART0_RXD, kSWM_PortPin_P0_0);


    // Configure PIO_4, (Alakart label: Po)  as : USART0 TX
    // IC package physical pin is: 4,
    // GPIO pin name: PIO0_4
    // Peripheral: USART0, signal: TX.

    // First configure the GPIO pin characteristics:
    IOCON_PIO_config = (IOCON_PIO_MODE_PULLUP |  // Select pull-up function
                        IOCON_PIO_HYS_EN |       // Enable hysteresis
                        IOCON_PIO_INV_DI |       // Do not invert input
                        IOCON_PIO_OD_DI |        // Disable open-drain function
                        IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                        IOCON_PIO_CLKDIV0);      // IOCONCLKDIV = 0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_4, IOCON_PIO_config);

    // Second, connect USART0 TX to PIO0_4 using the multiplexer:
    // USART0 TX is a moveable pin.
    // It will be connected to Port 0, Pin 4: PIO0_4
    // PIO0_4 is pin #4  on the 32 pin IC package
    SWM_SetMovablePinSelect(SWM0, kSWM_USART0_TXD, kSWM_PortPin_P0_4);


    CLOCK_DisableClock(kCLOCK_Swm); // After configuration, SWM can be stopped.
}
