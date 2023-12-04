/*
 * Copyright  2018 ,2021 NXP
 * All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef _PIN_MUX_H_
#define _PIN_MUX_H_


#define IOCON_PIO_CLKDIV0 0x00u      // IOCONCLKDIV0 
#define IOCON_PIO_HYS_EN 0x20u       // Enable hysteresis 
#define IOCON_PIO_INV_DI 0x00u       // Input not invert 
#define IOCON_PIO_MODE_PULLUP 0x10u  // Selects pull-up function 
#define IOCON_PIO_OD_DI 0x00u        // Disables Open-drain function 
#define IOCON_PIO_SMODE_BYPASS 0x00u // Bypass input filter 

void InitPins(void); // Function assigned for the Cortex-M0P 

#endif // _PIN_MUX_H_ 


