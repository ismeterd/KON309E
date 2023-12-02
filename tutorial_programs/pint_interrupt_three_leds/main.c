// Developed based on Ahmet Onat's PIN Interrupt Example.

// This program is an example program for using the "PIN Interrupt" especially with usage GPIO peripheral.
// * Generates an interrupt when button is released.
// * When interrupt event occurs, state changes.
// * LEDs turn on depending on the state.
// * Also, state variable and some information outputs can be monitored from terminal.

// # State Machine (Output)
// State 0: Red LED -> ON  - Green LED -> OFF  - Yellow LED -> OFF
// State 1: Red LED -> OFF - Green LED -> ON   - Yellow LED -> OFF
// State 2: Red LED -> OFF - Green LED -> OFF  - Yellow LED -> ON

// # Connections
// - Button     -> P09
// - Red LED    -> P17
// - Green LED  -> P18
// - Yellow LED -> P19

#include <stdint.h>

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "fsl_pint.h"
#include "fsl_power.h"
#include "fsl_clock.h"
#include "fsl_syscon.h"
#include "fsl_gpio.h"


#define CORE_CLOCK   30000000U  // Set CPU Core clock frequency (Hz)

#define USART_INSTANCE 0U
#define USART_BAUDRATE 115200

#define GPIO_PORT 0

#define RED_LED 17
#define GREEN_LED 18
#define YELLOW_LED 19


void clock_init(void);
status_t uart_init(void);
void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status);
void SysTick_Handler(void);
void delay_ms(uint32_t ms);

volatile uint32_t g_systickCounter;

uint8_t state = 0;

int main(void)
{
//    Initialize processor clock
    clock_init();

//    Initialize tick period
    SysTick_Config(SystemCoreClock / 1000U);

//    Enable clock of uart0
    CLOCK_EnableClock(kCLOCK_Uart0);

//    Enable clock of GPIO0
    CLOCK_EnableClock(kCLOCK_Gpio0);

//    Initialize UART Peripheral
    uart_init();

//    USART RX/TX Connections
    InitPins();

//    Ser DIV of uart0.
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);

//    Struct for LED pins
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0};

//    Initialize GPIO Port
    GPIO_PortInit(GPIO, GPIO_PORT);

//    Initialize GPIO pins for LEDs
    GPIO_PinInit(GPIO, GPIO_PORT, RED_LED, &led_config);
    GPIO_PinInit(GPIO, GPIO_PORT, GREEN_LED, &led_config);
    GPIO_PinInit(GPIO, GPIO_PORT, YELLOW_LED, &led_config);


//    Connect PIO_9 as a source to PIN INT 1:
    SYSCON_AttachSignal(SYSCON, kPINT_PinInt1, kSYSCON_GpioPort0Pin9ToPintsel);

    /*
     * !!! CAUTION
     * If PINT_Init is run, access to the GPIO unit is restricted and no output can be obtained from the pins.
     * So, this line is commented.
     */
////    Initialize PIN Interrupts
//    PINT_Init(PINT);

    // Setup Pin Interrupt 1:
    //  falling edge triggers the INT
    //  register the name of the callback function (as the last argument).
    PINT_PinInterruptConfig(PINT,                // Pin INT base register address
                            kPINT_PinInt1,       // Use Pin INT 1
                            kPINT_PinIntEnableFallEdge, // At falling edge.
                            pint_intr_callback); // Name of the callback function

    // Enable callbacks for PINT0 by Index:
    // This clears the pending INT flags
    //  and enables the interrupts for this PIN INT.
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt1);


    PRINTF("PINT Pin Interrupt events are configured\r\n");
    PRINTF("Press Button to generate events\r\n");

    while (1)
    {
        switch (state)
        {
            case 0:
//                PRINTF("\f\r\n\t\tState %u: Red LED ON", state);
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 1);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 0);
                break;
            case 1:
//                PRINTF("\f\r\n\t\tState %u: Green LED ON", state);
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 1);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 0);
                break;
            case 2:
//                PRINTF("\f\r\n\t\tState %u: Yellow LED ON", state);
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 1);
                break;
        }
        delay_ms(1);
//        If delay command is not included, GPIO_PinWrite commands don't run!
    }
}


status_t uart_init(void)
{
    uint32_t uart_clock_freq;
    status_t result;

    uart_clock_freq=CLOCK_GetMainClkFreq(); // Read UART clock frequency.

    CLOCK_EnableClock(kCLOCK_Uart0);               // Enable clock of UART0.
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);   // Set prescaler of UART0.
    RESET_PeripheralReset(kUART0_RST_N_SHIFT_RSTn);// Reset UART0

    // See:
    // Xpresso_SDK/devices/LPC824/utilities/debug_console_lite/fsl_debug_console.c
    result = DbgConsole_Init(USART_INSTANCE,
                             USART_BAUDRATE,
                             kSerialPort_Uart,
                             uart_clock_freq);
    // assert(kStatus_Success == result);
    return result;
}

///////////  This is the ISR callback for PIN INTERRUPT: ////////////
// The actual ISR is in file fsl_pint.c and has no arguments.

// The arguments are defined in fsl_pint.c file around line 794
// within the declaration:
//  void PIN_INT0_DriverIRQHandler(void)
// Check that file for more details.
void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    PRINTF("\f\r\nPINT Pin Interrupt %d event detected.", pintr);
    state++;

    if (state == 3)
    {
        state = 0;
    }

    PRINTF("\f\r\n\tState is incremented by one, state = %u", state);
}

// Setup clock source: Internal RC clock with the PLL set to 30MHz frequency:
void clock_init(void) {

  // Set up using Internal RC clock (IRC) oscillator:
  POWER_DisablePD(kPDRUNCFG_PD_IRC_OUT);        // Turn ON IRC OUT
  POWER_DisablePD(kPDRUNCFG_PD_IRC);            // Turn ON IRC

  CLOCK_Select(kSYSPLL_From_Irc);               // Connect IRC to PLL input.

  clock_sys_pll_t config;
  config.src = kCLOCK_SysPllSrcIrc;             // Select PLL source as IRC. 
  config.targetFreq = CORE_CLOCK*2;             // set pll target freq

  CLOCK_InitSystemPll(&config);                 // set parameters

  CLOCK_SetMainClkSrc(kCLOCK_MainClkSrcSysPll); // Select PLL as main clock source.
  CLOCK_Select(kCLKOUT_From_Irc);               // select IRC for CLKOUT
  CLOCK_SetCoreSysClkDiv(1U);

  // Check processor registers and calculate the
  // Actual clock speed. This is stored in the
  // global variable SystemCoreClock
  SystemCoreClockUpdate();

  // The following is for convenience and not necessary. AO.
  // It outputs the system clock on Pin 27
  //    so that we can check using an oscilloscope:

  /*
  // First activate the clock out function:
  SYSCON->CLKOUTSEL = (uint32_t)3; //set CLKOUT source to main clock.
  SYSCON->CLKOUTUEN = 0UL;
  SYSCON->CLKOUTUEN = 1UL;
  // Divide by a reasonable constant so that it is easy to view on an oscilloscope:
  //SYSCON->CLKOUTDIV = 100;
  SYSCON->CLKOUTDIV = 2000; 

  // Using the switch matrix, connect clock out to Pin 27:
  CLOCK_EnableClock(kCLOCK_Swm);     // Enables clock for switch matrix.
  SWM_SetMovablePinSelect(SWM0, kSWM_CLKOUT, kSWM_PortPin_P0_27);
  CLOCK_DisableClock(kCLOCK_Swm); // Disable clock for switch matrix.
  */
}

void SysTick_Handler(void) {
    if (g_systickCounter != 0U) {
        g_systickCounter--;
    }
}

// This program sets and monitors the global variable g_systickCounter.
// This variable is decremented by SysTickHandler ISR each time it is called.
void delay_ms(uint32_t ms){
    g_systickCounter = ms;
    while (g_systickCounter != 0U)
    {}
}
