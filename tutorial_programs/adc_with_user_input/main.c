// Developed based on Ahmet Onat's ADC Basic Example.

// # Connections
// - Potentiometer -> P07 (ADC Channel 0)

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_adc.h"
#include "fsl_clock.h"
#include "fsl_power.h"

#include <stdbool.h>

// Channel 0 will be used in this example.
#define ADC_CHANNEL_0 0U

// See Fig 52. ADC clocking in Ref Manual.
#define ADC_CLOCK_DIVIDER 1U

void ADC_Configuration(adc_result_info_t * ADCResultStruct);

int main(void)
{
    uint32_t frequency = 0U;
    adc_result_info_t ADCResultStruct;

    BOARD_InitPins();
    BOARD_InitBootClocks();

//    Enable UART0 clock
    CLOCK_EnableClock(kCLOCK_Uart0);
//    Set UART0 clock divider
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);
    BOARD_InitDebugConsole();

    PRINTF("ADC Basic Example\r\n");
    PRINTF("~~~~~~~~~~~~~~~~~\r\n");

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

    PRINTF("Configuration Done.\r\n\n");


    while (1)
    {
        PRINTF("Enter the char to start ADC conversion:\r\n\n");
//        Wait for an ADC conversion request from the user terminal.
        GETCHAR();

//        Start conversion.
        ADC_DoSoftwareTriggerConvSeqA(ADC0);

//         Keep polling the ADC to check if the conversion is complete:
        while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL_0, &ADCResultStruct))
        { }

        PRINTF("Ch %d result = %d\r\n\n", ADCResultStruct.channelNumber, ADCResultStruct.result);
    }
}

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

    while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL_0, ADCResultStruct))
    { }

    ADC_GetConvSeqAGlobalConversionResult(ADC0, ADCResultStruct);
}
