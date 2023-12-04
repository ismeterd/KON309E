// Experiment 3 - Part 2

// * Connect the LEDs to the outputs of the SCT using the SWM peripheral.
// * Configure the SCT to generate a PWM signal such that the brightness of the
//LEDs can be changed.
// * Configure the PWM frequency to be 10kHz.
// * The MRT INT should still be active, and still served by the ISR as in PART I.
// * Every 500ms, change the PWM duty ratio of the currently ON LED to 0%, and
//the next LED to 50%. (You can use more or less PWM duty ratio to get a visually
//pleasing brightness.)

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
#include "pin_mux.h"
#include "fsl_power.h"
#include "fsl_clock.h"
#include "fsl_syscon.h"
#include "fsl_sctimer.h"

//Set CPU Core clock frequency (Hz)
#define CORE_CLOCK   30000000U

//Desired number of Interrupts per second. (Value 100 is equal to interrupt per 10ms.
#define DESIRED_INT_FREQ 100

#define RED_LED 17
#define GREEN_LED 18
#define YELLOW_LED 19


void clock_init(void);
void SysTick_Handler(void);
void delay_ms(uint32_t ms);

volatile uint32_t g_systickCounter;

uint32_t event0 = 0;
uint32_t event1 = 1;
uint32_t event2 = 2;
uint32_t sctimerClock;

int32_t redDutyCyclePercent = 50;
int32_t  greenDutyCyclePercent = 0;
int32_t  yellowDutyCyclePercent = 0;

//MRT ISR sets this flag to true.
static volatile bool mrtIsrFlag = false;

uint32_t counter = 0;
uint8_t state = 0;

int main(void)
{
//    Initialize processor clock
    clock_init();

//    Initialize tick period
    SysTick_Config(SystemCoreClock / 1000U);


//    Enable clock of SCTimer peripheral
    CLOCK_EnableClock(kCLOCK_Sct);

//    Initialize variable for MRT
    uint32_t mrt_clock;
    uint32_t mrt_count_val;

//    Struct for configuring the MRT:
    mrt_config_t mrtConfig;

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


//    SCT Out Settings
    InitPins();

//    Ser DIV of uart0.
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);

//    This struct stores several parameters of SCTimer:
    sctimer_config_t sctimerConfig;

//    This struct stores several parameters of the  PWM signal:
    sctimer_pwm_signal_param_t red_pwmParam;
    sctimer_pwm_signal_param_t green_pwmParam;
    sctimer_pwm_signal_param_t yellow_pwmParam;

//    What frequency is the SCTimer getting?:
//    sctimerClock = CLOCK_GetFreq(kCLOCK_Irc);
    sctimerClock = CORE_CLOCK;

//    sctimerConfig structure contains the configuration of SCTimer.
//    Set it to a default value. See the library function.
    SCTIMER_GetDefaultConfig(&sctimerConfig);
    SCTIMER_Init(SCT0, &sctimerConfig);  // Initialize SCTimer module:

//    Configure first PWM with frequency 24kHz from first output
    red_pwmParam.output = kSCTIMER_Out_4; // This is the Red LED.
//    High duty ratio will produce a higher average signal:
    red_pwmParam.level = kSCTIMER_HighTrue;
//    However here 100 is dim, 0 is bright because LED anode is connected to VDD:
    red_pwmParam.dutyCyclePercent = redDutyCyclePercent;

//    Configure the timer:
    SCTIMER_SetupPwm(SCT0,                      // Timer is SCT0.
                     &red_pwmParam,                 // Use the pwmParam struct.
                     kSCTIMER_CenterAlignedPwm, // Generate center aligned PWM
                     10000U,                    // Freq is 10kHz
                     sctimerClock,              // Use the clock from sctimer
                     &event0);

    green_pwmParam.output = kSCTIMER_Out_5;
    green_pwmParam.level = kSCTIMER_HighTrue;
    green_pwmParam.dutyCyclePercent = greenDutyCyclePercent;
    SCTIMER_SetupPwm(SCT0,
                     &green_pwmParam,
                     kSCTIMER_CenterAlignedPwm,
                     10000U,
                     sctimerClock,
                     &event1);

    yellow_pwmParam.output = kSCTIMER_Out_3;
    yellow_pwmParam.level = kSCTIMER_HighTrue;
    yellow_pwmParam.dutyCyclePercent = yellowDutyCyclePercent;
    SCTIMER_SetupPwm(SCT0,
                     &yellow_pwmParam,
                     kSCTIMER_CenterAlignedPwm,
                     10000U,
                     sctimerClock,
                     &event2);


//    Start the 32-bit unified timer
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_U);

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

            if (state == 3)
            {
                state = 0;
            }
        }

        switch (state)
        {
            case 0:
                redDutyCyclePercent = 50;
                greenDutyCyclePercent = 0;
                yellowDutyCyclePercent = 0;
                break;
            case 1:
                redDutyCyclePercent = 0;
                greenDutyCyclePercent = 50;
                yellowDutyCyclePercent = 0;
                break;
            case 2:
                redDutyCyclePercent = 0;
                greenDutyCyclePercent = 0;
                yellowDutyCyclePercent = 50;
                break;
        }

        SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_4, redDutyCyclePercent, event0);  //red
        SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_5, greenDutyCyclePercent, event1);  //green
        SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_3, yellowDutyCyclePercent, event2);  //yellow

        mrtIsrFlag = false;
    }
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
