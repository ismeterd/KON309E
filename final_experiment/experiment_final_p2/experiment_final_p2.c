// Final Experiment - Part 2
// -------------------------
//
// # Scenario
//    - Read an analog port at 100 Hz.
//        - Sys Tick and trigger ADC conversion
//    - Print the results of ADC conversion to PC terminal
//    - Produce PWM signal from SCT0 (100 kHZ)
//        - Maximum Duty Ratio = 300 counts
//        - PWM Output -> PIO0_27 (the green LED)
//    - At every ADC conversion, update the PWM duty ratio
//    - Repeat

#include <stdio.h>
#include <string.h>

#include "pin_mux.h"
#include "fsl_power.h"
#include "fsl_clock.h"
#include "fsl_swm.h"
#include "fsl_swm_connections.h"
#include "fsl_adc.h"
#include "fsl_sctimer.h"

#include "xprintf.h"

// Set CPU Core clock frequency (Hz)
#define CORE_CLOCK   30000000U

// Channel 0 will be used in this experiment.
#define ADC_CHANNEL 0U

// See Fig 52. ADC clocking in Ref Manual.
#define ADC_CLOCK_DIVIDER 1U


void clock_init(void);
void SysTick_DelayTicks(uint32_t n);
void config_uart0 (void);
void uart_putch (uint8_t character);
int16_t convert12BitToPWMDutyCyclePercent(uint16_t adcData);
int32_t convert12BitToControllerInput(uint32_t adcData);
//int32_t convert12BitToControllerInput_forPrint(uint32_t adcData);


volatile uint32_t SystickCounter;
uint32_t event0 = 0;
uint32_t sctimerClock;


int main(void)
{
    uint8_t PWMDutyPercent = 0;
    int32_t controllerInput_int = 0;
    double controllerInput = 0;

    uint32_t frequency = 0U;
    adc_result_info_t ADCResultStruct;

//    Enable clock of uart0.
    CLOCK_EnableClock(kCLOCK_Uart0);
//    Enable ADC clock
    CLOCK_EnableClock(kCLOCK_Adc);
//    Enable clock of SCTimer peripheral
    CLOCK_EnableClock(kCLOCK_Sct);
//    Power on ADC0
    POWER_DisablePD(kPDRUNCFG_PD_ADC0);

//    Set configuration of USART and I2C pins
    BOARD_InitPins();
//    Set clock speed to 30MHz
    clock_init();
//    Configure UART0 for correct speed and byte format.
    config_uart0();

//    Set the hardware interface function for xprintf
    xdev_out(uart_putch);



//    Set systick reload value to generate 1ms interrupt
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1) { } // This would be an error condition.
    }

    sctimerClock = CLOCK_GetFreq(kCLOCK_Irc);

//    This struct stores several parameters of SCTimer:
    sctimer_config_t sctimerConfig;

//    This struct stores several parameters of the  PWM signal:
    sctimer_pwm_signal_param_t pwmParam;

//    sctimerConfig structure contains the configuration of SCTimer.
//    Set it to a default value. See the library function.
    SCTIMER_GetDefaultConfig(&sctimerConfig);
    SCTIMER_Init(SCT0, &sctimerConfig);  // Initialize SCTimer module:

//    Configure first PWM with frequency 100kHz from first output
    pwmParam.output = kSCTIMER_Out_2; // This is the Green LED.
//    High duty ratio will produce a higher average signal:
    pwmParam.level = kSCTIMER_HighTrue;
//    However here 100 is dim, 0 is bright because LED anode is connected to VDD:
    pwmParam.dutyCyclePercent = PWMDutyPercent;

//    Configure the timer:
    SCTIMER_SetupPwm(SCT0,                      // Timer is SCT0.
                     &pwmParam,                 // Use the pwmParam struct.
                     kSCTIMER_CenterAlignedPwm, // Generate center aligned PWM
                     100000U,                    // Freq is 100kHz
                     sctimerClock,              // Use the clock from sctimer
                     &event0);


    xprintf("\r\n\r\n\n\nEXPERIMENT 5 - PART 2\r\n");
    xprintf("~~~~~~~~~~~~~~~~~~~~~~\n\n\n");

    xprintf("# Initialization\r\n\n");

//    Hardware calibration is required after each chip reset.
//    See: Sec. 21.3.4 Hardware self-calibration
    frequency = CLOCK_GetFreq(kCLOCK_Irc);

    if (true == ADC_DoSelfCalibration(ADC0, frequency))
    {
        xprintf("\t *[INFO]: ADC Calibration Done.\r\n");
    }
    else
    {
        xprintf("\t *[INFO]: ADC Calibration Failed.\r\n");
    }

//    Configure ADC and operation mode.
    ADC_Configuration(&ADCResultStruct);

    xprintf("\t *[INFO]: ADC Configuration Done.\r\n\n\n");


    while (1)
    {
        ADC_DoSoftwareTriggerConvSeqA(ADC0); // Start conversion.

//        Keep polling the ADC to check if the conversion is complete:
        while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL, &ADCResultStruct))
        { }

        xprintf("[INFO]: ADC Ch %d -> %d\r\n", ADCResultStruct.channelNumber, ADCResultStruct.result);


        PWMDutyPercent = convert12BitToPWMDutyCyclePercent(ADCResultStruct.result);
        controllerInput_int = convert12BitToControllerInput(ADCResultStruct.result);
        controllerInput = (double) controllerInput_int / (double) 100;


        xprintf("[INFO]: PWM Duty Percent -> %d\r\n", PWMDutyPercent);
        xprintf("[INFO]: Controller Input -> %d\r\n", controllerInput_int);

        SCTIMER_UpdatePwmDutycycle(SCT0, kSCTIMER_Out_2, PWMDutyPercent, event0);  //red

//        100 Hz
        SysTick_DelayTicks(100);
    }
}
int32_t convert12BitToControllerInput_forPrint(uint32_t adcData)
{
    float controllerInput;

    adcData = (int32_t) adcData;
    adcData = adcData - 2047;
    controllerInput = (float) adcData * 0.0977039;
    

    return (int32_t) controllerInput;
}


int32_t convert12BitToControllerInput(uint32_t adcData)
{

    double controllerInput = 0;
    double conversionCoefficient = (double) 400/4096;


    controllerInput = (double)adcData * conversionCoefficient;
    controllerInput = (int32_t)controllerInput;

    controllerInput = (int32_t)controllerInput - 200;

    return controllerInput;
}


// convert12BitToPWMDutyCyclePercent() function convert 12 bit ADC Data to PWM Duty Cycle Percent [0-100]
int16_t convert12BitToPWMDutyCyclePercent(uint16_t adcData)
{
    double milliVolts = 0;
    double conversionCoefficient = (double) 100/4096;

    milliVolts = (double)adcData * conversionCoefficient;
    milliVolts = (int16_t)milliVolts;

    return milliVolts;
}

// Configure and initialize the ADC
void ADC_Configuration(adc_result_info_t * ADCResultStruct)
{
    adc_config_t adcConfigStruct;
    adc_conv_seq_config_t adcConvSeqConfigStruct;

//    Defined above.
    adcConfigStruct.clockDividerNumber = ADC_CLOCK_DIVIDER;
    adcConfigStruct.enableLowPowerMode = false;

//    See Sec. 21.6.11 A/D trim register (voltage mode):
    adcConfigStruct.voltageRange = kADC_HighVoltageRange;

//    Initialize ADC0 with this structure.
    ADC_Init(ADC0, &adcConfigStruct);

//    Insert this channel in Sequence A, and set conversion properties:
//    See Sec: 21.6.2 A/D Conversion Sequence A Control Register
    adcConvSeqConfigStruct.channelMask = (1U << ADC_CHANNEL);

    adcConvSeqConfigStruct.triggerMask      = 0U;
    adcConvSeqConfigStruct.triggerPolarity  = kADC_TriggerPolarityPositiveEdge;
    adcConvSeqConfigStruct.enableSingleStep = false;
    adcConvSeqConfigStruct.enableSyncBypass = false;
    adcConvSeqConfigStruct.interruptMode    = kADC_InterruptForEachSequence;

//    Initialize the ADC0 with the sequence defined above:
    ADC_SetConvSeqAConfig(ADC0, &adcConvSeqConfigStruct);

//    Enable the conversion sequence A.
    ADC_EnableConvSeqA(ADC0, true);

//    Make the first ADC conversion so that the result register has a sensible initial value.
    ADC_DoSoftwareTriggerConvSeqA(ADC0);

    while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL, ADCResultStruct))
    {}

    ADC_GetConvSeqAGlobalConversionResult(ADC0, ADCResultStruct);
}


//Set up the clock source
void clock_init(void)
{
//    Set up IRC
    POWER_DisablePD(kPDRUNCFG_PD_IRC_OUT);        // Turn ON IRC OUT
    POWER_DisablePD(kPDRUNCFG_PD_IRC);            // Turn ON IRC
//    POWER_DisablePD(kPDRUNCFG_PD_SYSOSC);       // In Alakart SYSOSC is not used.
    CLOCK_Select(kSYSPLL_From_Irc);               // Connect IRC to PLL input.
    clock_sys_pll_t config;
    config.src = kCLOCK_SysPllSrcIrc;             // Select PLL source as IRC.
    config.targetFreq = CORE_CLOCK*2;             // set pll target freq
    CLOCK_InitSystemPll(&config);                 // set parameters
    CLOCK_SetMainClkSrc(kCLOCK_MainClkSrcSysPll); // Select PLL as main clock source.
    CLOCK_Select(kCLKOUT_From_MainClk);               // select IRC for CLKOUT
    CLOCK_SetCoreSysClkDiv(1U);

//    Check processor registers and calculate the actual clock speed. This is stored in the global variable
//    SystemCoreClock
    SystemCoreClockUpdate();

    // The following is for convenience and not necessary. AO.
    // It outputs the system clock on Pin 26
    //    so that we can check using an oscilloscope:
    // First activate the clock out function:
    SYSCON->CLKOUTSEL = (uint32_t)3; //set CLKOUT source to main clock.
    SYSCON->CLKOUTUEN = 0UL;
    SYSCON->CLKOUTUEN = 1UL;
    // Divide by a reasonable constant so that it is easy to view on an oscilloscope:
    SYSCON->CLKOUTDIV = 200;  // Max possible divisor is 255, 1 divides by 1. MainCLK =2 * SystemCLK

//    Using the switch matrix, connect clock out to Pin 26:
    CLOCK_EnableClock(kCLOCK_Swm);     // Enables clock for switch matrix.
    SWM_SetMovablePinSelect(SWM0, kSWM_CLKOUT, kSWM_PortPin_P0_26);
    CLOCK_DisableClock(kCLOCK_Swm); // Disable clock for switch matrix.
}

void uart_putch (uint8_t character)
{
//    Check if transmission has ended. See: 13.6.3 USART Status register:
    while ((USART0->STAT& 0b0100)==0);
    //  USART0_TXDAT=character;
    USART0->TXDAT = character;
}

void config_uart0 (void)
{
    // The following steps set up the serial port USART0:
    // See User Manual
    // 13.3 Basic Configuration (USART)

    // 1. Turn the peripheral on:
    CLOCK_EnableClock(kCLOCK_Uart0);    // Enable clock for USART0.

    // 2. Set speed (baud rate) to 115200bps:
    // See Sec. 13.7.1.1 and 13.6.9 in User Manual.
    // Obtain a preliminary clock by first dividing the processor main clock
    // Processor main clock is 60MHz. (60000000)
    // Divide by 8 to obtain 7,500,000 Hz. intermediate clock.
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 8); // USART clock div register.

    // Baud rate generator value is calculated from:
    // Intermediate clock /16 (always divided) = 468750Hz
    // To obtain 115200 baud transmission speed, we must divide further:
    // 468750/115200=4.069 ~= 4
    // Baud rate generator should be set to one less than this value.
    USART0->BRG=3;  //(4-1)  Baud rate generator register value.

    // 3. Enable USART & configure byte format for 8 bit, no parity, 1 stop bit:
    // (See 13.6.1 USART Configuration register)
    // (Bit 0) Enable USART
    // (Bit 1) not used.
    // (Bit 2:3) Data Length 00 => 8 bits.
    // (Bit 4:5) Parity 00 => No parity (default)
    // (Bit 6) Stop bit  0 => 1:  (default)
    // (Bit 7) Reserved
    // The remaining bits are left at default values.
    USART0->CFG=0b00000101; // Configuration of USART0
}

//Systick system timer INT functions for precise delay:
void SysTick_Handler(void)
{
    if (SystickCounter != 0U) {
      SystickCounter--;
    }
}

void SysTick_DelayTicks(uint32_t n)
{
    SystickCounter = n;
    while (SystickCounter != 0U) { }
}
