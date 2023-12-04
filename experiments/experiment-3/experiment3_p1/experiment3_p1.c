// Experiment 3 - Part 1

// * Configure the MRT timer such that it generates an INT every 10 ms.
// * Write an ISR for the MRT. The ISR must have a counter that counts to 50,
//turns off the current LED and turns on the next LED.

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

#include "fsl_mrt.h"
#include "fsl_power.h"
#include "fsl_gpio.h"
#include "fsl_clock.h"
#include "fsl_syscon.h"

//Set CPU Core clock frequency (Hz)
#define CORE_CLOCK 30000000U

//Desired number of Interrupts per second. (Value 100 is equal to interrupt per 10ms.
#define DESIRED_INT_FREQ 100

#define GPIO_PORT 0

#define RED_LED 17
#define GREEN_LED 18
#define YELLOW_LED 19


void clock_init(void);

//MRT ISR sets this flag to true.
static volatile bool mrtIsrFlag = false;

uint32_t counter = 0;
uint8_t state = 0;


int main(void)
{
//    Initialize processor clock
    clock_init();

//    Enable clock of MRT
    CLOCK_EnableClock(kCLOCK_Mrt);

//    Enable clock of GPIO0
    CLOCK_EnableClock(kCLOCK_Gpio0);

//    Initialize variable for MRT
    uint32_t mrt_clock;
    uint32_t mrt_count_val;

//    Struct for configuring the MRT:
    mrt_config_t mrtConfig;

//    Struct for LED pins
    gpio_pin_config_t led_config ={kGPIO_DigitalOutput, 0};

//    Initialize GPIO Port
    GPIO_PortInit(GPIO, GPIO_PORT);

//    Initialize GPIO pins for LEDs
    GPIO_PinInit(GPIO, GPIO_PORT, RED_LED, &led_config);
    GPIO_PinInit(GPIO, GPIO_PORT, GREEN_LED, &led_config);
    GPIO_PinInit(GPIO, GPIO_PORT, YELLOW_LED, &led_config);


//    Get default configuration of MRT Peripheral
    MRT_GetDefaultConfig(&mrtConfig);

//    Init MRT module to default configuration
    MRT_Init(MRT0, &mrtConfig);

//    Setup Channel 0 to periodic INT mode (See Sec. 19.5.1)
    MRT_SetupChannelMode(MRT0, kMRT_Channel_0, kMRT_RepeatMode);

//    Query the input clock frequency of MRT
    mrt_clock = CLOCK_GetFreq(kCLOCK_CoreSysClk);


//    To get DESIRED_INT_FREQ number of INTs per second, Channel 0 of MRT
//    must count up to: (Input clock frequency)/ (desired int frequency).
//    (Input clock frequency in this case is 30,000,000. For 2 INT per second,
//    the count value calculates as 15,000,000.)
//    CAUTION: MRT counter is only 24 bits wide. See Sec. 19.6.2 Timer register
//    and 19.6.5 Module Configuration register.
//    However, this still means we can write up to about 167,000,000 here.
    mrt_count_val = mrt_clock / DESIRED_INT_FREQ;
    MRT_StartTimer(MRT0, kMRT_Channel_0, mrt_count_val);

//    Finally enable MRT interrupts:
//    Enable MRT interrupts for channel 0 (See Sec. 19.6.3 Control register)
    MRT_EnableInterrupts(MRT0, kMRT_Channel_0, kMRT_TimerInterruptEnable);

//    Enable MRT INTs in NVIC.
    EnableIRQ(MRT0_IRQn);
//    (See Sec. 4.4.1 Interrupt Set Enable Register 0 register, Bit 10)

    while (1)
    {
//        Wait for interrupt
    }
}

///////////////////////////////////////////////////////////////////////
////////// This is the MRT interrupt service routine. /////////////////
///////////////////////////////////////////////////////////////////////

// It was declared in the file startup_LPC824.S
// as the 10th entry of the vector table.
// See Table 5 of Sec. 4.3.1 Interrupt sources

void MRT0_IRQHandler(void)
{
    MRT_ClearStatusFlags(MRT0,      // Clear interrupt flag:
                         kMRT_Channel_0,
                         kMRT_TimerInterruptFlag);
    mrtIsrFlag = true;              // Set global variable to inform main().

    if (mrtIsrFlag == true)
    { // This flag is set by the MRT ISR.
        counter++;
        if (counter == 50)
        {
            state++;
            counter = 0;

            if (state == 4)
            {
                state = 1;
            }
        }

        switch (state)
        {
            case 1:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 1);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 0);
                break;
            case 2:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 1);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 0);
                break;
            case 3:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, GREEN_LED, 0);
                GPIO_PinWrite(GPIO, GPIO_PORT, YELLOW_LED, 1);
                break;
        }
        mrtIsrFlag = false;
    }
}

// Setup processor clock source:
// Internal RC clock with the PLL set to 30MHz frequency.
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
    SystemCoreClockUpdate ();

    /*
    // The following is for convenience and not necessary. AO.
    // It outputs the system clock frequency on Pin 27
    //    so that we can check using an oscilloscope:

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
