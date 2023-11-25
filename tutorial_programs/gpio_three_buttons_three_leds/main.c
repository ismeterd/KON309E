// This program is an example program for using the GPIO peripheral.
// * Reads 3 button states.
// * Runs state machine.
// * LEDs turn on depending on the state.

// # State Machine (Output)
// State 0: Red LED -> ON  - Green LED -> OFF  - Yellow LED -> OFF
// State 1: Red LED -> OFF - Green LED -> ON   - Yellow LED -> OFF
// State 2: Red LED -> OFF - Green LED -> OFF  - Yellow LED -> ON

// # State Machine (Input)
// Button1: Increase 1 state
// Button2: Increase 2 state
// Button3: Increase 3 state

// # Connections
// - Button1    -> P09
// - Button2    -> P08
// - Button3    -> P07
// - Red LED    -> P17
// - Green LED  -> P18
// - Yellow LED -> P19

#include <stdint.h>

#include "fsl_power.h"
#include "fsl_swm.h"
#include "fsl_swm_connections.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"


//Set CPU Core clock frequency (Hz)
#define CORE_CLOCK 30000000U

#define GPIO_PORT 0

#define BUTTON_1 9
#define BUTTON_2 8
#define BUTTON_3 7

#define RED_LED 17
#define GREEN_LED 18
#define YELLOW_LED 19


void clock_init(void);
void SysTick_Handler(void);
void delay_ms(uint32_t ms);


volatile uint32_t g_systickCounter;

uint8_t button1Pressed = 0;
uint8_t button2Pressed = 0;
uint8_t button3Pressed = 0;
uint8_t state = 0;

int main(void)
{
//    Initialize processor clock.
    clock_init();

//    Initialize tick period
    SysTick_Config(SystemCoreClock / 1000U);

//    struct for the LED pins
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0};
//    struct for the Button pins
    gpio_pin_config_t button_config = {kGPIO_DigitalInput, 1};

//    Initialize GPIO Port
    GPIO_PortInit(GPIO, GPIO_PORT);

//    Initialize GPIO pins for Buttons
    GPIO_PinInit(GPIO, GPIO_PORT, BUTTON_1, &button_config);
    GPIO_PinInit(GPIO, GPIO_PORT, BUTTON_2, &button_config);
    GPIO_PinInit(GPIO, GPIO_PORT, BUTTON_3, &button_config);

//    Initialize GPIO pins for LEDs
    GPIO_PinInit(GPIO, GPIO_PORT, RED_LED, &led_config);
    GPIO_PinInit(GPIO, GPIO_PORT, GREEN_LED, &led_config);
    GPIO_PinInit(GPIO, GPIO_PORT, YELLOW_LED, &led_config);

    while (1)
    {
        if ((GPIO_PinRead(GPIO, GPIO_PORT, BUTTON_1) == 1) && !button2Pressed && !button3Pressed)
        {
            if (!button1Pressed)
            {
                // debounce delay
                delay_ms(50);
                if (GPIO_PinRead(GPIO, GPIO_PORT, BUTTON_1) == 1)
                {
                    state++;

                    if (state >= 3)
                    {
                        state -= 3;
                    }
                }
                button1Pressed = 1;
            }
        }

        else if ((GPIO_PinRead(GPIO, GPIO_PORT, BUTTON_2) == 1) && !button1Pressed && !button3Pressed)
        {
            if (!button2Pressed)
            {
                // debounce delay
                delay_ms(50);
                if (GPIO_PinRead(GPIO, GPIO_PORT, BUTTON_2) == 1)
                {
                    state += 2;

                    if (state >= 3)
                    {
                        state -= 3;
                    }
                }
                button2Pressed = 1;
            }
        }

        else if ((GPIO_PinRead(GPIO, GPIO_PORT, BUTTON_3) == 1) && !button1Pressed && !button2Pressed)
        {
            if (!button2Pressed)
            {
                // debounce delay
                delay_ms(50);
                if (GPIO_PinRead(GPIO, GPIO_PORT, BUTTON_3) == 1)
                {
                    state += 3;

                    if (state >= 3)
                    {
                        state -= 3;
                    }
                }
                button3Pressed = 1;
            }
        }

        else
        {
            button1Pressed = 0;
            button2Pressed = 0;
            button3Pressed = 0;
        }

        switch (state)
        {
            case 0:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 1);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 0);
                break;
            case 1:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 1);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 0);
                break;
            case 2:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 1);
                break;
        }
    }
}

//setup clock source
void clock_init(void)
{    // Set up the clock source
    // Set up IRC
    POWER_DisablePD(kPDRUNCFG_PD_IRC_OUT);        // Turn ON IRC OUT
    POWER_DisablePD(kPDRUNCFG_PD_IRC);            // Turn ON IRC
    //POWER_DisablePD(kPDRUNCFG_PD_SYSOSC);       // In Alakart SYSOSC is not used.
    CLOCK_Select(kSYSPLL_From_Irc);               // Connect IRC to PLL input.
    clock_sys_pll_t config;
    config.src = kCLOCK_SysPllSrcIrc;             // Select PLL source as IRC.
    config.targetFreq = CORE_CLOCK*2;             // set pll target freq
    CLOCK_InitSystemPll(&config);                 // set parameters
    //CLOCK_SetMainClkSrc(kCLOCK_MainClkSrcIrc);  // Select IRC as main clock source.
    CLOCK_SetMainClkSrc(kCLOCK_MainClkSrcSysPll); // Select PLL as main clock source.
    CLOCK_Select(kCLKOUT_From_Irc);               // select IRC for CLKOUT
    CLOCK_SetCoreSysClkDiv(1U);
    //CLOCK_UpdateClkOUTsrc();
    // Set SystemCoreClock variable.
    //    SystemCoreClock = CORE_CLOCK;

    // Check processor registers and calculate the
    // Actual clock speed. This is stored in the
    // global variable SystemCoreClock
    SystemCoreClockUpdate ();

//    // The following is for convenience and not necessary. AO.
//    // It outputs the system clock on Pin 26
//    //    so that we can check using an oscilloscope:
//    // First activate the clock out function:
//    SYSCON->CLKOUTSEL = (uint32_t)3; //set CLKOUT source to main clock.
//    SYSCON->CLKOUTUEN = 0UL;
//    SYSCON->CLKOUTUEN = 1UL;
//    // Divide by a reasonable constant so that it is easy to view on an oscilloscope:
//    //SYSCON->CLKOUTDIV = 100;
//    SYSCON->CLKOUTDIV = 2000;
//
//    // Using the switch matrix, connect clock out to Pin 26:
//    CLOCK_EnableClock(kCLOCK_Swm);     // Enables clock for switch matrix.
//    SWM_SetMovablePinSelect(SWM0, kSWM_CLKOUT, kSWM_PortPin_P0_26);
//    CLOCK_DisableClock(kCLOCK_Swm); // Disable clock for switch matrix.
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