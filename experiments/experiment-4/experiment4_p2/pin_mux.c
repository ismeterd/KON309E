
#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_swm.h"
#include "pin_mux.h"

void BOARD_InitPins(void) {

  uint32_t IOCON_config;
  
  CLOCK_EnableClock(kCLOCK_Iocon);  // Enable IOCON clock
  
  CLOCK_EnableClock(kCLOCK_Swm);  // Enable SWM clock

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
  // PIO0 PIN0 (pin no 24) is configured as USART0, RXD.
  IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_0, IOCON_config);
  
  // Connect USART0_RXD to P0_0
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
  // PIO0_4 (pin no 4) is configured with these properties.
  IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_4, IOCON_config);
  
  // Connect USART0_TXD to PIO0_4 
  SWM_SetMovablePinSelect(SWM0, kSWM_USART0_TXD, kSWM_PortPin_P0_4);
  
  
  IOCON_config = (//Deactivate pull-up
		  IOCON_PIO_HYS_DI |      // Disable hysteresis
		  IOCON_PIO_INV_DI |      // Do not invert if input
		  IOCON_PIO_OD_DI |       // Disable open-drain
		  IOCON_PIO_SMODE_BYPASS |// Bypass input filter
		  IOCON_PIO_CLKDIV0);     // IOCONCLKDIV0
  // Configure PIO0_7 (pin no 23)  with these properties.
  IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_7, IOCON_config);
  
  // Connect ADC_CHN0 to PIO0_7:
  SWM_SetFixedPinSelect(SWM0, kSWM_ADC_CHN0, true);


    IOCON_config = (//Deactivate pull-up
            IOCON_PIO_HYS_DI |      // Disable hysteresis
            IOCON_PIO_INV_DI |      // Do not invert if input
            IOCON_PIO_OD_DI |       // Disable open-drain
            IOCON_PIO_SMODE_BYPASS |// Bypass input filter
            IOCON_PIO_CLKDIV0);     // IOCONCLKDIV0
    // Configure PIO0_6 (pin no 23)  with these properties.
    IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_6, IOCON_config);

    // Connect ADC_CHN1 to PIO0_6:
    SWM_SetFixedPinSelect(SWM0, kSWM_ADC_CHN1, true);
  
  // Disable SWM clock since configuration is complete
  CLOCK_DisableClock(kCLOCK_Swm);
}
