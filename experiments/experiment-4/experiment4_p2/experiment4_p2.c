// Experiment 4 - Part 2

// # Connections
// - Potentiometer   -> P07 (ADC Channel 0)
// - Voltage Divider -> P06 (ADC Channel 1)
// - Red LED         -> P17


#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_adc.h"
#include "fsl_sctimer.h"
#include "fsl_clock.h"
#include "fsl_power.h"
#include "fsl_gpio.h"

#include <stdbool.h>
#include <stdint.h>

#define GPIO_PORT 0U
#define RED_LED 17U

#define ADC_CHANNEL_0 0U
#define ADC_CHANNEL_1 1U

#define ADC_CLOCK_DIVIDER 1U

// The pointer and flag are global so that ISR can manipulate them:
adc_result_info_t *volatile ADCResultPtr; 
volatile bool ADCConvCompleteFlag;
uint8_t mainCounter = 0;
uint32_t channel_1_result = 0;
uint32_t channel_2_result = 0;

void ADC_Configuration(adc_result_info_t * ADCResultStruct);
void SCT_Configuration(void);
int16_t convert12BitToMilliVolt(uint32_t adcData);


int main(void)
{
    uint32_t frequency = 0U;
    adc_result_info_t ADCResultStruct;

//    The global pointer points to this variable
    ADCResultPtr = &ADCResultStruct;

    BOARD_InitPins();
    BOARD_InitBootClocks();

//    struct for the LED pins
    gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 0};
//    GPIO Initialization
    GPIO_PortInit(GPIO, GPIO_PORT);
    GPIO_PinInit(GPIO, GPIO_PORT, RED_LED, &led_config);


    CLOCK_EnableClock(kCLOCK_Uart0);
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);
    BOARD_InitDebugConsole();

    PRINTF("Experiment 4 - Part 2\r\n\n");

//     Enable clock of SCT
    CLOCK_EnableClock(kCLOCK_Sct);
    SCT_Configuration();

//    Enable ADC clock
    CLOCK_EnableClock(kCLOCK_Adc);

//    Power on ADC0
    POWER_DisablePD(kPDRUNCFG_PD_ADC0);

//    Hardware calibration is required after each chip reset.
//    See: Sec. 21.3.4 Hardware self-calibration
    frequency = CLOCK_GetFreq(kCLOCK_Irc);

    if (true == ADC_DoSelfCalibration(ADC0, frequency))
    {
        PRINTF("ADC Calibration Done.\r\n");
    }
    else
    {
        PRINTF("ADC Calibration Failed.\r\n");
    }

//    Configure ADC and operation mode.
    ADC_Configuration(&ADCResultStruct);

//    Enable the interrupt the for Sequence A Conversion Complete:
    ADC_EnableInterrupts(ADC0, kADC_ConvSeqAInterruptEnable); // Within ADC0
    NVIC_EnableIRQ(ADC0_SEQA_IRQn);                           // Within NVIC

    PRINTF("Configuration Done.\r\n\n");


    /*
     * All ADC conversion is handled by the hardware.
     *
     * ADC0 conversion is triggered by the hardware: SCT OUTPUT 3 event
     * When SCT OUTPUT3 changes, the conversion of Sequence A starts.
     *
     * When the conversion is complete,
     *   SEQA_INT (Sequence A conversion complete INT) is triggered.
     * This calls ADC0_SEQA_IRQHandler function which finally prints out
     *  the conversion result to the serial port (and to the terminal screen.)
    */

    while (1)
    {

        if (ADCConvCompleteFlag == true)
        {
            PRINTF("ADC%d = %4d mV, ", ADC_CHANNEL_0,
                   convert12BitToMilliVolt(channel_1_result));
            PRINTF("ADC%d = %4d mV\n", ADC_CHANNEL_1,
                convert12BitToMilliVolt(channel_2_result));
            mainCounter += 1;
            ADCConvCompleteFlag = false;
        }
        if (mainCounter == 4)
        {
            mainCounter = 0;
        }
        switch (mainCounter)
        {
            case 0:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                break;
            case 1:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 1);
                break;
            case 2:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 1);
                break;
            case 3:
                GPIO_PinWrite(GPIO, GPIO_PORT, RED_LED, 0);
                break;
        }
    }
}



//ISR for ADC conversion sequence A done.
void ADC0_SEQA_IRQHandler(void)
{
    if (kADC_ConvSeqAInterruptFlag ==
	(kADC_ConvSeqAInterruptFlag & ADC_GetStatusFlags(ADC0)))
    {
        ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL_0, ADCResultPtr);
        ADC_ClearStatusFlags(ADC0, kADC_ConvSeqAInterruptFlag);

        channel_1_result = ADCResultPtr->result;
//        PRINTF("Ch %d result = %d, ", // See below for PRINTF usage in an ISR.
//               ADCResultPtr->channelNumber,
//               channel_1_result);

        ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL_1, ADCResultPtr);

        ADC_ClearStatusFlags(ADC0, kADC_ConvSeqAInterruptFlag);

        channel_2_result = ADCResultPtr->result;
//        PRINTF("Ch %d result = %d \n", // See below for PRINTF usage in an ISR.
//               ADCResultPtr->channelNumber,
//               channel_2_result );

        ADCConvCompleteFlag = true;
    }
}

// Usage of long functions in an ISR:
// Note that in general an ISR must be written to complete and exit
// as quickly as possible.
// PRINTF is a function that may take a long time to execute.
// So it is not advisable to use PRINTF in an ISR.
// However, PRINTF is used in an ISR here to
//  emphasize that the main loop is not doing anything.


// Configure and initialize the ADC
void ADC_Configuration(adc_result_info_t * ADCResultStruct)
{
    adc_config_t adcConfigStruct;
    adc_conv_seq_config_t adcConvSeqConfigStruct;

    adcConfigStruct.clockDividerNumber = ADC_CLOCK_DIVIDER; // Defined above.
    adcConfigStruct.enableLowPowerMode = false;
//    See Sec. 21.6.11 A/D trim register (voltage mode):
    adcConfigStruct.voltageRange = kADC_HighVoltageRange;

    ADC_Init(ADC0, &adcConfigStruct); // Initialize ADC0 with this structure.

//    Insert this channel in Sequence A, and set conversion properties:
//    See Sec: 21.6.2 A/D Conversion Sequence A Control Register
    adcConvSeqConfigStruct.channelMask = (1U << ADC_CHANNEL_0);
    adcConvSeqConfigStruct.channelMask += (1U << ADC_CHANNEL_1);

//    Triggered by SCT OUT3 event. See Table 277. "ADC hardware trigger inputs":
    adcConvSeqConfigStruct.triggerMask      = 3U;

    adcConvSeqConfigStruct.triggerPolarity  = kADC_TriggerPolarityPositiveEdge;
    adcConvSeqConfigStruct.enableSingleStep = false;
    adcConvSeqConfigStruct.enableSyncBypass = false;
    adcConvSeqConfigStruct.interruptMode    = kADC_InterruptForEachSequence;

//    Initialize the ADC0 with the sequence defined above:
    ADC_SetConvSeqAConfig(ADC0, &adcConvSeqConfigStruct);

    ADC_EnableConvSeqA(ADC0, true); // Enable the conversion sequence A.

//    Make the first ADC conversion so that the result register has a sensible initial value.
    ADC_DoSoftwareTriggerConvSeqA(ADC0);

    while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL_0, ADCResultStruct))
    { }

    while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL_1, ADCResultStruct))
    { }

    ADC_GetConvSeqAGlobalConversionResult(ADC0, ADCResultStruct);
}

// Configure and initialize the SCT
void SCT_Configuration(void)
{
    sctimer_config_t sctimerConfig;
    uint32_t eventCounterL;
    uint16_t matchValueL;

    SCTIMER_GetDefaultConfig(&sctimerConfig);

//    Set the configuration struct for the timer:
//    For more information, see:  Xpresso_SDK/devices/LPC824/drivers/fsl_sctimer.h
    sctimerConfig.enableCounterUnify = false; // Use as two 16 bit timers.

//    Use system clock as SCT input
    sctimerConfig.clockMode = kSCTIMER_System_ClockMode;

    matchValueL= 24000; // This is in: 16.6.20 SCT match registers 0 to 7
    sctimerConfig.enableBidirection_l= false; // Use as single directional register.
    // Prescaler is 8 bit, in: CTRL. See: 16.6.3 SCT control register
    sctimerConfig.prescale_l = 249; // Thi value +1 is used.

//    Initialize SCTimer module
    SCTIMER_Init(SCT0, &sctimerConfig);

//    Configure the low side counter.
//    Schedule a match event for the 16-bit low counter:
    SCTIMER_CreateAndScheduleEvent(SCT0,
                                   kSCTIMER_MatchEventOnly,
                                   matchValueL,
                                   0,    // Not used for "Match Only"
                                   kSCTIMER_Counter_L,
                                   &eventCounterL);

//    TODO: Rather than toggle, it should set the output:
//    Toggle output_3 when the 16-bit low counter event occurs:
    SCTIMER_SetupOutputToggleAction(SCT0, kSCTIMER_Out_3, eventCounterL);

//    Reset Counter L when the 16-bit low counter event occurs
    SCTIMER_SetupCounterLimitAction(SCT0, kSCTIMER_Counter_L, eventCounterL);

//    Setup the 16-bit low counter event active direction (See fsl_sctimer.h)
    SCTIMER_SetupEventActiveDirection(SCT0, kSCTIMER_ActiveIndependent, eventCounterL);

//    Start the 16-bit low counter
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_L);
}

// convert12BitToMilliVolt() function convert 12 bit ADC Data to mV (from 0 to 3300).
int16_t convert12BitToMilliVolt(uint32_t adcData)
{
    double milliVolts = 0;
    double conversionCoefficient = 0.805860805;

    milliVolts = (double)adcData * conversionCoefficient;
    milliVolts = (int16_t)milliVolts;

    return milliVolts;
}
