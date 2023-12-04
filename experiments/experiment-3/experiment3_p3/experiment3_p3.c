// Experiment 3 - Part 3

// * Configure the external button as a PINT such that when it is pressed, an
//INT is triggered (i.e. correctly configure falling edge or rising edge).
// * In the ISR of the PINT, change the value of a volatile global variable: It must
//default to 5 and at each execution of the PINT ISR, it should cycle through the
//following values: 10, 25, 99, 5, 10, 25...

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
#include "fsl_mrt.h"
#include "pin_mux.h"
#include "fsl_power.h"
#include "fsl_pint.h"
#include "fsl_clock.h"
#include "fsl_syscon.h"
#include "fsl_sctimer.h"

//Set CPU Core clock frequency (Hz)
#define CORE_CLOCK   30000000U

//Set Debug Console Communication Parameters
#define USART_INSTANCE 0U
#define USART_BAUDRATE 115200

//Desired number of Interrupts per second. (Value 100 is equal to interrupt per 10ms.
#define DESIRED_INT_FREQ 100

#define RED_LED 17
#define GREEN_LED 18
#define YELLOW_LED 19


void clock_init(void);
status_t uart_init(void);
void SysTick_Handler(void);
void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status);
void delay_ms(uint32_t ms);

volatile uint32_t g_systickCounter;

uint32_t event0 = 0;
uint32_t event1 = 1;
uint32_t event2 = 2;
uint32_t sctimerClock;

int32_t redDutyCyclePercent = 5;
int32_t  greenDutyCyclePercent = 0;
int32_t  yellowDutyCyclePercent = 0;

//MRT ISR sets this flag to true.
static volatile bool mrtIsrFlag = false;

volatile int brightnessCounter = 0;
int brightness[4] = {5, 10, 25, 99};

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

//    Initialize UART Peripheral
    uart_init();

//    USART Configuration and SCT Out Settings
    InitPins();


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

    PRINTF("\033[0m");
    PRINTF("[INFO]: PINT Pin Interrupt events are configured.\r\n");
    PRINTF("[INFO]: Press Button to change brightness. (Current Brightness: %d)\r\n", brightness[brightnessCounter]);

    while (1)
    {
//        Wait for interrupt
    }
}

///////////  This is the ISR callback for PIN INTERRUPT: ////////////
// The actual ISR is in file fsl_pint.c and has no arguments.

// The arguments are defined in fsl_pint.c file around line 794
// within the declaration:
//  void PIN_INT0_DriverIRQHandler(void)
// Check that file for more details.
void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    PRINTF("\033[0m");
    PRINTF("\n[PIN INTERRUPT]: PINT Pin Interrupt event detected.\r\n");
    brightnessCounter++;
    if (brightnessCounter == 4)
    {
        brightnessCounter = 0;
    }
    PRINTF("[PIN INTERRUPT]:\t LED brightness has changed -> Brightness: %d \r\n", brightness[brightnessCounter]);
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

            switch (state)
            {
                case 0:
                    PRINTF("\033[1;31m");
                    PRINTF("\n[TIMER INTERRUPT]: State is increment by one. (Current LED: RED)\n");
                    break;
                case 1:
                    PRINTF("\033[1;32m");
                    PRINTF("\n[TIMER INTERRUPT]: State is increment by one. (Current LED: GREEN)\n");
                    break;
                case 2:
                    PRINTF("\033[1;33m");
                    PRINTF("\n[TIMER INTERRUPT]: State is increment by one. (Current LED: YELLOW)\n");
                    break;
            }
        }

        switch (state)
        {
            case 0:
                redDutyCyclePercent = brightness[brightnessCounter];
                greenDutyCyclePercent = 0;
                yellowDutyCyclePercent = 0;
                break;
            case 1:
                redDutyCyclePercent = 0;
                greenDutyCyclePercent = brightness[brightnessCounter];
                yellowDutyCyclePercent = 0;
                break;
            case 2:
                redDutyCyclePercent = 0;
                greenDutyCyclePercent = 0;
                yellowDutyCyclePercent = brightness[brightnessCounter];
                break;
        }

        SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_4, redDutyCyclePercent, event0);  //red
        SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_5, greenDutyCyclePercent, event1);  //green
        SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_3, yellowDutyCyclePercent, event2);  //yellow

        mrtIsrFlag = false;
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
