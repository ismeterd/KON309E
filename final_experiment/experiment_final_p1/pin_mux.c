#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_swm.h"
#include "pin_mux.h"

void BOARD_InitPins(void) {

    uint32_t IOCON_config;

//    Enable IOCON clock
    CLOCK_EnableClock(kCLOCK_Iocon);
//    Enable SWM clock
    CLOCK_EnableClock(kCLOCK_Swm);

//   ---------------------------------------------------------------------------------------
//  -------------------------------- UART PIN CONFIGURATION --------------------------------
//   ---------------------------------------------------------------------------------------

    IOCON_config = (/* Selects pull-up function */
            IOCON_PIO_MODE_PULLUP |
            /* Disable hysteresis */
            IOCON_PIO_HYS_DI |
            /* Input not invert */
            IOCON_PIO_INV_DI |
            /* Disables Open-drain function */
            IOCON_PIO_OD_DI |
            /* Bypass input filter */
            IOCON_PIO_SMODE_BYPASS |
            /* IOCONCLKDIV0 */
            IOCON_PIO_CLKDIV0);
//    PIO0 PIN0 (pin no 24) is configured as USART0, RXD.
    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_0, IOCON_config);

//    Connect USART0_RXD to P0_0
    SWM_SetMovablePinSelect(SWM0, kSWM_USART0_RXD, kSWM_PortPin_P0_0);


    IOCON_config = (/* Selects pull-up function */
            IOCON_PIO_MODE_PULLUP |
            /* Disable hysteresis */
            IOCON_PIO_HYS_DI |
            /* Input not invert */
            IOCON_PIO_INV_DI |
            /* Disables Open-drain function */
            IOCON_PIO_OD_DI |
            /* Bypass input filter */
            IOCON_PIO_SMODE_BYPASS |
            /* IOCONCLKDIV0 */
            IOCON_PIO_CLKDIV0);
//    PIO0_4 (pin no 4) is configured with these properties.
    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_4, IOCON_config);

//    Connect USART0_TXD to PIO0_4
    SWM_SetMovablePinSelect(SWM0, kSWM_USART0_TXD, kSWM_PortPin_P0_4);

//   ---------------------------------------------------------------------------------------
//   ---------------------------------------------------------------------------------------


//   ---------------------------------------------------------------------------------------
//  -------------------------------- ADC PIN CONFIGURATION ---------------------------------
//   ---------------------------------------------------------------------------------------

    IOCON_config = (//Deactivate pull-up
            IOCON_PIO_HYS_DI |      // Disable hysteresis
            IOCON_PIO_INV_DI |      // Do not invert if input
            IOCON_PIO_OD_DI |       // Disable open-drain
            IOCON_PIO_SMODE_BYPASS |// Bypass input filter
            IOCON_PIO_CLKDIV0);     // IOCONCLKDIV0
//    Configure PIO0_7 (pin no ?)  with these properties.
    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_7, IOCON_config);

//    Connect ADC_CHN0 to PIO0_7:
    SWM_SetFixedPinSelect(SWM0, kSWM_ADC_CHN0, true);

//   ---------------------------------------------------------------------------------------
//   ---------------------------------------------------------------------------------------


//   ---------------------------------------------------------------------------------------
//  ------------------------------- SCT OUT 2 CONFIGURATION --------------------------------
//   ---------------------------------------------------------------------------------------

    // Configure PIO_27, (Alakart label: P27) This is the Green
    // as : SCT OUT2
    // IC package physical pin is: 11,??
    // GPIO pin name: PIO27_0
    // Peripheral: SCT, signal: OUT2

    // First configure the GPIO pin characteristics:
    IOCON_config =(IOCON_PIO_MODE_PULLUP |  // Select pull-up function
                       IOCON_PIO_HYS_DI |       // Disable hysteresis
                       IOCON_PIO_INV_DI |       // Do not invert input
                       IOCON_PIO_OD_DI |        // Disable open-drain function
                       IOCON_PIO_SMODE_BYPASS | // Bypass the input filter
                       IOCON_PIO_CLKDIV0);      // IOCONCLKDIV = 0

    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_27, IOCON_config);

    // Second, connect SCT_OUT2 to P0_27 using the multiplexer:
    // SCT_OUT3 is a moveable pin.
    // Connect it to Port 0, Pin 27: PIO0_27
    // PIO0_19 is pin #11 ??? on the 32 pin IC package
    SWM_SetMovablePinSelect(SWM0, kSWM_SCT_OUT2, kSWM_PortPin_P0_27);

//   ---------------------------------------------------------------------------------------
//   ---------------------------------------------------------------------------------------

    // Disable SWM clock since configuration is complete
    CLOCK_DisableClock(kCLOCK_Swm);
}
