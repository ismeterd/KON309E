// This program is an example program for using the PWM peripheral.
// * Reads 3 button states.
// * Runs state machine.
// * LEDs turn on depending on the state with own PWM duty cycle.
// * Each LED's PWM duty cycle is with memory.

// # State Machine (Output)
// State 0: Red LED -> ON  - Green LED -> OFF  - Yellow LED -> OFF
// State 1: Red LED -> OFF - Green LED -> ON   - Yellow LED -> OFF
// State 2: Red LED -> OFF - Green LED -> OFF  - Yellow LED -> ON

// # State Machine (Input)
// Button1: Increase 1 state
// Button2: Increase PWM Duty Cycle (+%20)
// Button3: Increase PWM Duty Cycle (-%20)

// # Connections
// - Button1    -> P09
// - Button2    -> P08
// - Button3    -> P07
// - Red LED    -> P17
// - Green LED  -> P18
// - Yellow LED -> P19

#include <stdint.h>

#include "pin_mux.h"
#include "fsl_power.h"
#include "fsl_swm.h"
#include "fsl_swm_connections.h"
#include "fsl_sctimer.h"
#include "fsl_gpio.h"


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

uint32_t event0 = 0;
uint32_t event1 = 1;
uint32_t event2 = 2;
uint32_t sctimerClock;

int32_t redDutyCyclePercent = 20;
int32_t  greenDutyCyclePercent = 20;
int32_t  yellowDutyCyclePercent = 20;


int main(void)
{
    InitPins();        // Initialize board pins.
    clock_init();      // Initialize processor clock.

    //    Initialize tick period
    SysTick_Config(SystemCoreClock / 1000U);

    gpio_pin_config_t button_config = {kGPIO_DigitalInput, 1};

    //    Initialize GPIO Port
    GPIO_PortInit(GPIO, GPIO_PORT);

//    Initialize GPIO pins for Buttons
    GPIO_PinInit(GPIO, GPIO_PORT, BUTTON_1, &button_config);
    GPIO_PinInit(GPIO, GPIO_PORT, BUTTON_2, &button_config);
    GPIO_PinInit(GPIO, GPIO_PORT, BUTTON_3, &button_config);

    // This struct stores several parameters of SCTimer:
    sctimer_config_t sctimerConfig;

    // This struct stores several parameters of the  PWM signal:
    sctimer_pwm_signal_param_t red_pwmParam;
    sctimer_pwm_signal_param_t green_pwmParam;
    sctimer_pwm_signal_param_t yellow_pwmParam;

    CLOCK_EnableClock(kCLOCK_Sct); // Enable clock of SCTimer peripheral


    // What frequency is the SCTimer getting?:
    //sctimerClock = CLOCK_GetFreq(kCLOCK_Irc);
    sctimerClock = CORE_CLOCK;

    // sctimerConfig structure contains the configuration of SCTimer.
    // Set it to a default value. See the library function.
    SCTIMER_GetDefaultConfig(&sctimerConfig);
    SCTIMER_Init(SCT0, &sctimerConfig);  // Initialize SCTimer module:

    // Configure first PWM with frequency 24kHz from first output
    red_pwmParam.output = kSCTIMER_Out_4; // This is the Red LED.
    // High duty ratio will produce a higher average signal:
    red_pwmParam.level = kSCTIMER_HighTrue;
    // However here 100 is dim, 0 is bright because LED anode is connected to VDD:
    red_pwmParam.dutyCyclePercent = redDutyCyclePercent;

    // Configure the timer:
    SCTIMER_SetupPwm(SCT0,                      // Timer is SCT0.
                     &red_pwmParam,                 // Use the pwmParam struct.
                     kSCTIMER_CenterAlignedPwm, // Generate center aligned PWM
                     24000U,                    // Freq is 24kHz
                     sctimerClock,              // Use the clock from sctimer
                     &event0);

    green_pwmParam.output = kSCTIMER_Out_5;
    green_pwmParam.level = kSCTIMER_HighTrue;
    green_pwmParam.dutyCyclePercent = greenDutyCyclePercent;
    SCTIMER_SetupPwm(SCT0,
                     &green_pwmParam,
                     kSCTIMER_CenterAlignedPwm,
                     24000U,
                     sctimerClock,
                     &event1);

    yellow_pwmParam.output = kSCTIMER_Out_3;
    yellow_pwmParam.level = kSCTIMER_HighTrue;
    yellow_pwmParam.dutyCyclePercent = yellowDutyCyclePercent;
    SCTIMER_SetupPwm(SCT0,
                     &yellow_pwmParam,
                     kSCTIMER_CenterAlignedPwm,
                     24000U,
                     sctimerClock,
                     &event2);


    // Start the 32-bit unified timer
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_U);

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
                    switch (state)
                    {
                        case 0:
                            redDutyCyclePercent += 20;
                            if (redDutyCyclePercent >= 100)
                            {
                                redDutyCyclePercent = 100;
                                break;
                            }
                            break;
                        case 1:
                            greenDutyCyclePercent += 20;
                            if (greenDutyCyclePercent >= 100)
                            {
                                greenDutyCyclePercent = 100;
                                break;
                            }
                            break;
                        case 2:
                            yellowDutyCyclePercent += 20;
                            if (yellowDutyCyclePercent >= 100)
                            {
                                yellowDutyCyclePercent = 100;
                                break;
                            }
                            break;
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
                    switch (state)
                    {
                        case 0:
                            redDutyCyclePercent -= 20;
                            if (redDutyCyclePercent <= 0)
                            {
                                redDutyCyclePercent = 0;
                                break;
                            }
                            break;
                        case 1:
                            greenDutyCyclePercent -= 20;
                            if (greenDutyCyclePercent <= 0)
                            {
                                greenDutyCyclePercent = 0;
                                break;
                            }
                            break;
                        case 2:
                            yellowDutyCyclePercent -= 20;
                            if (yellowDutyCyclePercent <= 0)
                            {
                                yellowDutyCyclePercent = 0;
                                break;
                            }
                            break;
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
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_4, redDutyCyclePercent, event0);  //red
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_5, 0, event1);  //green
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_3, 0, event2);  //yellow
                break;
            case 1:
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_4, 0, event0);  //red
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_5, greenDutyCyclePercent, event1);  //green
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_3, 0, event2);  //yellow
                break;
            case 2:
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_4, 0, event0);  //red
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_5, 0, event1);  //green
                SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_3, yellowDutyCyclePercent, event2);  //yellow
                break;
        }
    }
}

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