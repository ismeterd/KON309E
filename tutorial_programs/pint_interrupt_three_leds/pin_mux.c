#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_swm.h"
#include "pin_mux.h"
//#include "fsl_gpio.h"


void InitPins(void) {

  uint32_t IOCON_PIO_config;
  
  CLOCK_EnableClock(kCLOCK_Iocon);    // Enable clock for IOCON block.
  CLOCK_EnableClock(kCLOCK_Swm);      // Enable clock for switch matrix.

  // In this processor, outputs of some peripheral devices
  //  can be connected to any GPIO pin by configuring a "Pin multiplexer".
  // (This is only possible for peripherals with "moveable pins"
  //
  //This code does the configuration.

  
  // Configure PIO0, PIN0 (Physical pin no: 24) as USART0, RXD:
  // IC package physical pin: 24,
  // GPIO pin name: PIO0_0/ACMP_I1/TDO,
  // Mode: pullUp, invert: dis, hysteresis: en,
  //       opendrain: dis, smode: bypass, clkdiv: div0
  // Peripheral: USART0, signal: RXD,

  // First configure the GPIO pin characteristics:
  IOCON_PIO_config = (IOCON_PIO_MODE_PULLUP | // Activate pull-up function
		      IOCON_PIO_HYS_EN |      // Enable hysteresis
		      IOCON_PIO_INV_DI |      // Do not invert input
		      IOCON_PIO_OD_DI |       // Disable Open-drain function 
		      IOCON_PIO_SMODE_BYPASS | // Do not use the input filter 
		      IOCON_PIO_CLKDIV0);     // IOCONCLKDIV0
  IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_0, IOCON_PIO_config);
  
  // Second, connect USART0_RXD to P0_0 using the multiplexer:
  // USART RXD is a moveable pin.
  // Connect it to Port 0, Pin 0: PIO0_0
  // PIO0_0 is pin #24  on the 32 pin IC package 
  SWM_SetMovablePinSelect(SWM0, kSWM_USART0_RXD, kSWM_PortPin_P0_0);

    
  // Configure PIO0, PIN4 (Physical pin no: 4) as USART0, TXD.
  // IC package physical pin: '4',
  // GPIO pin name: PIO0_4/ADC11/!TRST
  // Mode: pullUp, invert: dis, hysteresis: en,
  //       opendrain: dis, smode: bypass, clkdiv: div0
  // Peripheral: USART0, signal: TXD,
  
  // First configure the GPIO pin characteristics:
  IOCON_PIO_config = (IOCON_PIO_MODE_PULLUP | // Activate pull-up function
		      IOCON_PIO_HYS_EN |      // Enable hysteresis
		      IOCON_PIO_INV_DI |      // Do not invert input
		      IOCON_PIO_OD_DI |       // Disable Open-drain function 
		      IOCON_PIO_SMODE_BYPASS | // Do not use the input filter 
		      IOCON_PIO_CLKDIV0);     // IOCONCLKDIV0
  IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_4, IOCON_PIO_config);
  
  // Secondly, connect USART0_TXD to P0_4:
  // USART TXD is a moveable pin.
  // Connect it to Port 0, Pin4: PIO0_4
  // Which is pin #4  on the 32 pin IC package 
  SWM_SetMovablePinSelect(SWM0, kSWM_USART0_TXD, kSWM_PortPin_P0_4);


//  // Configure PIO0 PIN09 (Package pin #2)? as GPIO (as default is an input.):
//
//  // Configure the GPIO pin characteristics:
//  IOCON_PIO_config = (IOCON_PIO_MODE_PULLUP | // Activate pull-up function
//		      IOCON_PIO_HYS_EN |      // Enable hysteresis
//		      IOCON_PIO_INV_DI |      // Do not invert input
//		      IOCON_PIO_OD_DI |       // Disable Open-drain function
//		      IOCON_PIO_SMODE_BYPASS | // Do not use the input filter
//		      IOCON_PIO_CLKDIV0);     // IOCONCLKDIV0
//
//  IOCON_PinMuxSet(IOCON, IOCON_INDEX_PIO0_9, IOCON_PIO_config);

  
  CLOCK_DisableClock(kCLOCK_Swm); // Disable clock for switch matrix.
}
