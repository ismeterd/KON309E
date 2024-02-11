#include "LPC824.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "xprintf.h"
#include "fsl_adc.h"
#include "fsl_clock.h"
#include "fsl_power.h"
#include "fsl_swm.h"

#include "peripherals.h"

//#define SAMPLING_RATE_100Hz
#define SAMPLING_RATE_1kHz

// Configure and initialize the ADC
void ADC_Configuration(adc_result_info_t * ADCResultStruct)
{
    adc_config_t adcConfigStruct;
    adc_conv_seq_config_t adcConvSeqConfigStruct;

    adcConfigStruct.clockDividerNumber = ADC_CLOCK_DIVIDER; // Defined above.
    adcConfigStruct.enableLowPowerMode = false;
    // See Sec. 21.6.11 A/D trim register (voltage mode):
    adcConfigStruct.voltageRange = kADC_HighVoltageRange;

    ADC_Init(ADC0, &adcConfigStruct); // Initialize ADC0 with this structure.

    // Insert this channel in Sequence A, and set conversion properties:
    // See Sec: 21.6.2 A/D Conversion Sequence A Control Register
    adcConvSeqConfigStruct.channelMask = (1U << ADC_CHANNEL);

    adcConvSeqConfigStruct.triggerMask      = 0U;
    adcConvSeqConfigStruct.triggerPolarity  = kADC_TriggerPolarityPositiveEdge;
    adcConvSeqConfigStruct.enableSingleStep = false;
    adcConvSeqConfigStruct.enableSyncBypass = false;
    adcConvSeqConfigStruct.interruptMode    = kADC_InterruptForEachSequence;

    // Initialize the ADC0 with the sequence defined above:
    ADC_SetConvSeqAConfig(ADC0, &adcConvSeqConfigStruct);

    ADC_EnableConvSeqA(ADC0, true); // Enable the conversion sequence A.

    // Make the first ADC conversion so that
    //  the result register has a sensible initial value.
    ADC_DoSoftwareTriggerConvSeqA(ADC0);

    while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL, ADCResultStruct))
    { }

    ADC_GetConvSeqAGlobalConversionResult(ADC0, ADCResultStruct);
}



//// Configure SCT low side to trigger the ADC:
//void SCT_Config_ADC_Trig(void)
//{
//    //SCTIMER_GetDefaultConfig(&sctimerConfig);
//    //sctimerConfig.enableCounterUnify = false; // Use as two 16 bit timers.
//
//
//    // Input Clock 60MHz.
//    // Set prescaler at 250 so 240kHz results. We wish to get 1kHz sampling rate.
//    // The counter must be set to 240 to have 1kHz.
//
//    SCT0->CTRL_ACCESS16BIT.CTRLL |= (250-1)<<5; // Set prescaler L (only 8 bits!)
//
//    // Configure Event 2 to trigger on count to max:
//#if defined (SAMPLING_RATE_100Hz)
//    SCT0->MATCHREL_ACCESS16BIT[2].MATCHRELL = 2400;   //
//#elif defined(SAMPLING_RATE_1kHz)
//    SCT0->MATCHREL_ACCESS16BIT[2].MATCHRELL = 240;   //
//#else
//#error "Please define the sampling rate in peripherals.c"
//#endif
//
//    SCT0->EV[2].STATE=0xFF;// Event 2 happens in all states.
//    SCT0->EV[2].CTRL = 2+(1 << 12);// Match reg 2 is used and match cond. only
//
//    // Evt 3 will be used to clear the output.
//    SCT0->MATCHREL_ACCESS16BIT[3].MATCHRELL = 120;   //For now generate about 2.5 samples/s.
//    SCT0->EV[3].STATE=0xFFFFFFFF;// Event 3 happens in all states.
//    SCT0->EV[3].CTRL = 3+(1 << 12);// Match reg 3 is used and match cond. only
//
//
//    // Link event 2 to limit of CTRL Low:
//    SCT0->LIMIT_ACCESS16BIT.LIMITL |= (1<<2); //Set BIT 2 of limit reg.
//
//
//    // Toggle output 3 at each ocurrance of Evt2:
//    SCT0->OUT[3].CLR = (1<<3);   //Event 3 will clear SCT0_OUT3 (for the ADC)
//    SCT0->OUT[3].SET = (1<<2);   //Event 2 will set SCT0_OUT3 (for the ADC)
//
//    SCT0->CTRL_ACCESS16BIT.CTRLL &=~(1<<2); // Unhalt SCT0L by clearing bit 2 of CTRL.
//    SCT0->CTRL_ACCESS16BIT.CTRLH &=~(1<<2); // Unhalt SCT0H by clearing bit 2 of CTRL.
//}

void SCT_Set_PWM(uint16_t duty)
{
    if (duty > PWM_MAX_VAL-1){ // Check limit...
        duty=PWM_MAX_VAL-1;
    }

    // Before changing the value stop the counter.
    SCT0->CTRL_ACCESS16BIT.CTRLH |=(1<<2); // Halt SCT0H by setting bit 2 of CTRL.

    SCT0->MATCHREL_ACCESS16BIT[1].MATCHRELH = duty; //Set match reg for evt.1

    SCT0->CTRL_ACCESS16BIT.CTRLH &=~(1<<2); // Start SCT0H
}

void SCT_Config_PWM(void)
{
    SCT0->CONFIG |= (1<<18);  // Two 16-bit timers, auto limit on H

    SCT0->CTRL_ACCESS16BIT.CTRLH |= (2-1)<<5; // Set prescaler (only 8 bits!)

    SCT0->MATCHREL_ACCESS16BIT[0].MATCHRELH = PWM_MAX_VAL-1; //Match 0
    SCT0->MATCHREL_ACCESS16BIT[1].MATCHRELH = 240;   //Match 1 for duty cycle.


    // TODO: Modify the below to 0xFF only:
    //  SCT0->EV[0].STATE=0xFFFFFFFF;// Event 0 happens in all states.
    SCT0->EV[0].STATE=0xFF;// Event 0 happens in all states.
    SCT0->EV[0].CTRL = 0+((1<<4)|(1 << 12));// Match reg 0 is used and match cond. only Bit 4 manipulation is necessary for  H counter.

    SCT0->EV[1].STATE=0xFF;// Event 1 happens in all states.
    SCT0->EV[1].CTRL = 1+((1<<4)|(1<<12));// Match reg 1 is used on match cond only. Bit 4 maipulation is necessary for  H counter.

    SCT0->OUT[2].SET = (1<<0);       // Event 0 will set SCT0_OUT2
    SCT0->OUT[2].CLR = (1<<1);       // Event 1 will clear SCT0_OUT2

    SCT0->CTRL_ACCESS16BIT.CTRLH &=~(1<<2); // Unhalt SCT0H by clearing bit 2 of CTRL.
    // This is a quirk of the SCT:
    // The events can happen only if *both* counters are operating...
    // It is not necessary for the lower counter.
    SCT0->CTRL_ACCESS16BIT.CTRLL &=~(1<<2); // Unhalt SCT0L by clearing bit 2 of CTRL.
}




////////////////////////////////////////////////////////////////////////////////
// Set up the clock source
void clock_init(void)
{
    // Set up IRC
    POWER_DisablePD(kPDRUNCFG_PD_IRC_OUT);        // Turn ON IRC OUT
    POWER_DisablePD(kPDRUNCFG_PD_IRC);            // Turn ON IRC
    //POWER_DisablePD(kPDRUNCFG_PD_SYSOSC);       // In Alakart SYSOSC is not used.
    CLOCK_Select(kSYSPLL_From_Irc);               // Connect IRC to PLL input.
    clock_sys_pll_t config;
    config.src = kCLOCK_SysPllSrcIrc;             // Select PLL source as IRC.
    config.targetFreq = CORE_CLOCK*2;             // set pll target freq
    CLOCK_InitSystemPll(&config);                 // set parameters
    CLOCK_SetMainClkSrc(kCLOCK_MainClkSrcSysPll); // Select PLL as main clock source.
    CLOCK_Select(kCLKOUT_From_MainClk);               // select IRC for CLKOUT
    CLOCK_SetCoreSysClkDiv(1U);

    // Check processor registers and calculate the
    // Actual clock speed. This is stored in the
    // global variable SystemCoreClock
    SystemCoreClockUpdate ();

    // The following is for convenience and not necessary. AO.
    // It outputs the system clock on Pin 26
    //    so that we can check using an oscilloscope:
    // First activate the clock out function:
    SYSCON->CLKOUTSEL = (uint32_t)3; //set CLKOUT source to main clock.
    SYSCON->CLKOUTUEN = 0UL;
    SYSCON->CLKOUTUEN = 1UL;
    // Divide by a reasonable constant so that it is easy to view on an oscilloscope:
    SYSCON->CLKOUTDIV = 200;  // Max possible divisor is 255, 1 divides by 1. MainCLK =2 * SystemCLK

    // Using the switch matrix, connect clock out to Pin 26:
    CLOCK_EnableClock(kCLOCK_Swm);     // Enables clock for switch matrix.
    SWM_SetMovablePinSelect(SWM0, kSWM_CLKOUT, kSWM_PortPin_P0_26);
    CLOCK_DisableClock(kCLOCK_Swm); // Disable clock for switch matrix.
}


////////////////////////////////////////////////////////////////////////////////


void uart_putch (uint8_t character)
{
    // Check if transmission has ended. See: 13.6.3 USART Status register:
    //  while ((USART0_STAT& 0b0100)==0);
    while ((USART0->STAT& 0b0100)==0);
    //  USART0_TXDAT=character;
    USART0->TXDAT=character;
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

