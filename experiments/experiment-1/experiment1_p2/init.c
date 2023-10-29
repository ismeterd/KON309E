

// The processor initialization for 'C' language for the LPC824 processor.

// Performs:
// - Writes the vectors (Stack pointer, Reset vector and all ISRs
//   (ISR: Interrupt Service Routines)
//
// - Configures the processor clock and sets the speed.
//
// - Initializes the variables with set values. Sets all other variables to '0'.


// Heavily modified from:
// http://eleceng.dit.ie/frank and follow the links
// Author: Frank Duignan

#include "lpc824.h"

void init(void);
void clock_init();
void Default_Handler(void);
void SysTick_Handler(void);
void SysTick(void);

extern int main(void);

// The following are 'declared' in the linker script
extern unsigned char  INIT_DATA_VALUES;
extern unsigned char  INIT_DATA_START;
extern unsigned char  INIT_DATA_END;
extern unsigned char  BSS_START;
extern unsigned char  BSS_END;

// the section "vectors" is placed at the beginning of flash 
// by the linker script
const void * Vectors[] __attribute__((section(".vectors"))) ={
	(void *)(RAM_START+RAM_SIZE), 	/* Top of stack */ 
	init,   			/* Reset Handler */
	Default_Handler,		/* NMI */
	Default_Handler,		/* Hard Fault */
	0,	                	/* Reserved */
	0,            			/* Reserved */
	0,                  		/* Reserved */
	0,                  		/* Reserved */
	0,                  		/* Reserved */
	0,                  		/* Reserved */
	0,                  		/* Reserved */
	Default_Handler,		/* SVC */
	0,                 		/* Reserved */
	0,                 		/* Reserved */
	Default_Handler,   		/* PendSV */
	//Default_Handler, 		/* SysTick */		
	SysTick_Handler, 		/* SysTick */
/* External interrupt handlers follow */
	Default_Handler,		/* 0 SPI0_IRQ */
	Default_Handler,		/* 1 SPI1_IRQ */
	Default_Handler,		/* 2 RESERVED */
	Default_Handler,		/* 3 UART0_IRQ */
	Default_Handler,		/* 4 UART1_IRQ */
	Default_Handler,		/* 5 UART2_IRQ */
	Default_Handler,		/* 6 RESERVED */
	Default_Handler,		/* 7 I2C1_IRQ */
	Default_Handler,		/* 8 I2C0_IRQ */
	Default_Handler,		/* 9 SCT_IRQ */
	Default_Handler,		/* 10 MRT_IRQ */
	Default_Handler,		/* 11 CMP_IRQ */
	Default_Handler,		/* 12 WDT_IRQ */
	Default_Handler,		/* 13 BOD_IRQ */
	Default_Handler,		/* 14 FLASH_IRQ */
	Default_Handler,		/* 15 WKT_IRQ */
	Default_Handler,		/* 16 ADC_SEQA_IRQ */
	Default_Handler,		/* 17 ADC_SEQB_IRQ */
	Default_Handler,		/* 18 ADC_THCMP_IRQ */
	Default_Handler,		/* 19 ADC_OVR_IRQ */
	Default_Handler,		/* 20 DMA_IRQ */
	Default_Handler,		/* 21 I2C2_IRQ */
	Default_Handler,		/* 22 I2C3_IRQ */
	Default_Handler,		/* 23 RESERVED */
	Default_Handler,		/* 24 PININT0_IRQ */
	Default_Handler,		/* 25 PININT1_IRQ */
	Default_Handler,		/* 26 PININT2_IRQ */
	Default_Handler,		/* 27 PININT3_IRQ */
	Default_Handler,		/* 28 PININT4_IRQ */
	Default_Handler,		/* 29 PININT5_IRQ */
	Default_Handler,		/* 30 PININT6_IRQ */
	Default_Handler,		/* 31 PININT7_IRQ */
};
void clock_init() {
  // This function sets the main clock to the PLL
  // The PLL input is the built in 12MHz RC oscillator
  // This is multiplied up to 60MHz for the main clock
  // MSEL=4 (i.e.M=5), PSEL = 1 (P=2) and divisor AHBCLKDIV=2
  // See Secs 5.7.4 and 5.3.1 of Ref Manual.

  SYSCON_PDRUNCFG |= (1 << 7);  // Power down the PLL before changing.
  SYSCON_SYSPLLCLKSEL = 0;      // Set internal RC oscillator as PLL input.
  SYSCON_SYSPLLCTRL = 0x24;     // This line sets the clock frequency.
  SYSCON_SYSPLLCLKUEN = 0;      // Latch the changes into the PLL 
  SYSCON_SYSPLLCLKUEN = 1; 
  SYSCON_PDRUNCFG &= ~(1 << 7); // Power up the PLL.
  while ((SYSCON_SYSPLLSTAT & (0x01U))==0);  // Wait until PLL locks.
  SYSCON_MAINCLKSEL = 3;        // Use PLL as main clock of the system
  SYSCON_MAINCLKUEN = 0;        // Latch the changes into the main clock input.
  SYSCON_MAINCLKUEN = 1; 
  SYSCON_SYSAHBCLKDIV = 2;      // PLL=60 MHz, /2 => System Clock= 30MHz 
}


void init() {
  // 1.Initialize the processor clock,
  // 2. Perform global/static data initialization
  // 3. Call "main()"
  
  unsigned char *src;
  unsigned char *dest;
  unsigned len;

  clock_init(); // boost speed to 30MHz

  // Initialize variable values:
  src= &INIT_DATA_VALUES;
  dest= &INIT_DATA_START;
  len= &INIT_DATA_END-&INIT_DATA_START;
  while (len--)
    *dest++ = *src++;
  
  // zero out the uninitialized global/static variables
  dest = &BSS_START;
  len = &BSS_END - &BSS_START;
  while (len--)
    *dest++=0;

  // Processor initialization is finished. Call the main() function:
  main();
}


// If an INT occurs but with no handler defined, then this function is called:
void Default_Handler() {  
  while(1);
}


