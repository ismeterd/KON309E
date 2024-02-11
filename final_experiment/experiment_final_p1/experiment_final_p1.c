// Final Experiment - Part 1
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


#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_adc.h"
#include "fsl_clock.h"
#include "fsl_power.h"
#include "fsl_sctimer.h"

// Channel 0 will be used in this experiment.
#define ADC_CHANNEL 0U

// See Fig 52. ADC clocking in Ref Manual.
#define ADC_CLOCK_DIVIDER 1U

void SysTick_DelayTicks(uint32_t ticks);
void ADC_Configuration(adc_result_info_t * ADCResultStruct);
int16_t convert12BitToPWMDutyCyclePercent(uint16_t adcData);


volatile uint32_t SysTickCounter;
uint32_t event0 = 0;
uint32_t sctimerClock;


int main(void)
{
    uint8_t PWMDutyPercent = 0;
    uint32_t frequency = 0U;
    adc_result_info_t ADCResultStruct;

    BOARD_InitPins();
    BOARD_InitBootClocks();

//    Enable UART0 clock
    CLOCK_EnableClock(kCLOCK_Uart0);
//    Enable ADC clock
    CLOCK_EnableClock(kCLOCK_Adc);
//    Enable clock of SCTimer peripheral
    CLOCK_EnableClock(kCLOCK_Sct);
//    Power on ADC0
    POWER_DisablePD(kPDRUNCFG_PD_ADC0);

//    Set UART0 clock divider
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);
//    Initialize Debug Console
    BOARD_InitDebugConsole();


//    Set SysTick reload value to generate 1ms interrupt
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


    PRINTF("\n\n\nFinal Experiment - Part 1\r\n");
    PRINTF("~~~~~~~~~~~~~~~~~~~~~~~~~\r\n\n\n");

    PRINTF("# Initialization\r\n\n");

//    Hardware calibration is required after each chip reset.
//    See: Sec. 21.3.4 Hardware self-calibration
    frequency = CLOCK_GetFreq(kCLOCK_Irc);

    if (true == ADC_DoSelfCalibration(ADC0, frequency))
    {
        PRINTF("\t *[INFO]: ADC Calibration Done.\r\n");
    }
    else
    {
        PRINTF("\t *[INFO]: ADC Calibration Failed.\r\n");
    }

//    Configure ADC and operation mode.
    ADC_Configuration(&ADCResultStruct);

    PRINTF("\t *[INFO]: ADC Configuration Done.\r\n\n\n");

    while (1)
    {
        ADC_DoSoftwareTriggerConvSeqA(ADC0); // Start conversion.

//        Keep polling the ADC to check if the conversion is complete:
        while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL, &ADCResultStruct))
        { }

        PRINTF("[INFO]: ADC Ch %d -> %d\r\n", ADCResultStruct.channelNumber, ADCResultStruct.result);

        PWMDutyPercent = convert12BitToPWMDutyCyclePercent(ADCResultStruct.result);
        PRINTF("[INFO]: PWM Duty Percent -> %d\r\n", PWMDutyPercent);

        SCTIMER_UpdatePwmDutycycle(SCT0,kSCTIMER_Out_2, PWMDutyPercent, event0);  //red

//        100 Hz
        SysTick_DelayTicks(10);
    }
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

//SysTick system timer INT functions for precise delay:
void SysTick_Handler(void)
{
    if (SysTickCounter != 0U) {
        SysTickCounter--;
    }
}

void SysTick_DelayTicks(uint32_t ticks)
{
    SysTickCounter = ticks;
    while (SysTickCounter != 0U) { }
}
