
#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_swm.h"
#include "pin_mux.h"

//     EXPERIMENT 2
// --------------------
// # Connections
// - Button1    -> P09
// - Button2    -> P08
// - Button3    -> P07
// - Red LED    -> P17
// - Green LED  -> P18
// - Yellow LED -> P19

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


    /////////////// SCT OUT4 Configuration /////////////////////////////////

    // Configure PIO_17, (Alakart label: P17)  as : SCT OUT4
    // This is the RED LED
    // IC package physical pin is: 10, ?
    // GPIO pin name: PIO16_0
    // Peripheral: SCT, signal: OUT4,

    // First configure the GPIO pin characteristics:
    IOCON_PIO_config = (IOCON_PIO_MODE_PULLUP |  // Select pull-up function
                        IOCON_PIO_HYS_EN |       // Enable hysteresis
                        IOCON_PIO_INV_DI |       // Do not invert input
                        IOCON_PIO_OD_DI |        // Disable open-drain function
                        IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                        IOCON_PIO_CLKDIV0);      // IOCONCLKDIV = 0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_17, IOCON_PIO_config);

    // Second, connect SCT_OUT4 to P0_16 using the multiplexer:
    // SCT_OUT4 is a moveable pin.
    // Connect it to Port 0, Pin 16: PIO0_16
    // PIO0_16 is pin #10  on the 32 pin IC package
    SWM_SetMovablePinSelect(SWM0, kSWM_SCT_OUT4, kSWM_PortPin_P0_17);


    ////////////// SCT OUT5 Configuration /////////////////////////////////

    // Configure PIO_18, (Alakart label: P18) This is the Green LED
    // as : SCT OUT5
    // IC package physical pin is: 11,??
    // GPIO pin name: PIO27_0
    // Peripheral: SCT, signal: OUT5

    // First configure the GPIO pin characteristics:
    IOCON_PIO_config =(IOCON_PIO_MODE_PULLUP |  // Select pull-up function
                       IOCON_PIO_HYS_EN |       // Enable hysteresis
                       IOCON_PIO_INV_DI |       // Do not invert input
                       IOCON_PIO_OD_DI |        // Disable open-drain function
                       IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                       IOCON_PIO_CLKDIV0);      // IOCONCLKDIV = 0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_18, IOCON_PIO_config);

    // Second, connect SCT_OUT5 to P0_18 using the multiplexer:
    // SCT_OUT2 is a moveable pin.
    // Connect it to Port 0, Pin 18: PIO0_18
    // PIO0_18 is pin #11  on the 32 pin IC package ???
    SWM_SetMovablePinSelect(SWM0, kSWM_SCT_OUT5, kSWM_PortPin_P0_18);


    ////////////// SCT OUT3 Configuration /////////////////////////////////

    // Configure PIO_19, (Alakart label: P19) This is the Yellow LED
    // as : SCT OUT3
    // IC package physical pin is: 11,??
    // GPIO pin name: PIO19_0
    // Peripheral: SCT, signal: OUT6

    // First configure the GPIO pin characteristics:
    IOCON_PIO_config =(IOCON_PIO_MODE_PULLUP |  // Select pull-up function
                       IOCON_PIO_HYS_EN |       // Enable hysteresis
                       IOCON_PIO_INV_DI |       // Do not invert input
                       IOCON_PIO_OD_DI |        // Disable open-drain function
                       IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                       IOCON_PIO_CLKDIV0);      // IOCONCLKDIV = 0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_19, IOCON_PIO_config);

    // Second, connect SCT_OUT3 to P0_19 using the multiplexer:
    // SCT_OUT3 is a moveable pin.
    // Connect it to Port 0, Pin 27: PIO0_27
    // PIO0_19 is pin #11 ??? on the 32 pin IC package
    SWM_SetMovablePinSelect(SWM0, kSWM_SCT_OUT3, kSWM_PortPin_P0_19);

    CLOCK_DisableClock(kCLOCK_Swm); // Disable clock for switch matrix.
}

