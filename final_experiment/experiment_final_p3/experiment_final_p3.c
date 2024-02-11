//                     FINAL EXPERIMENT - PART 3
//        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//        Plant Output y(t)
//          - y(t) is varies (-2 -> 2)
//
//        * ADC Peripheral *
//          - y(t) -> y(kT)
//          - y(kT) is varies (0 ->4095)
//
//        * ADC to Plant Output Conversion *
//          - y(kT) -> y(kT)
//          - (0, 4095) -> (-2 -> 2)
//
//        Reference Signal r(kT)
//          - r(kT) is varies (0 -> 1 -> 0 -> 1 ...)
//          - Cycle 4 second -> (2 seconds: 1, 2 seconds: 0)
//
//        * e(kT) Calculation *
//          - e(kT) = r(kT) - y(kT)
//
//        * Controller Implementation *
//          - e(kT) -> u(kT)
//          - Kp: 15, Ki: 60, Kd: 0.05
//
//        Control Signal u(kT)
//          - u(kT) is varies (-25 -> 25)
//
//        * Control Signal to PWM Duty Conversion *
//          - u(kT) -> PWM Duty Count
//          - (-25 -> 25) -> (0 -> 300)
//          - PWM SCT OUT 2
//
//        u(kT) -> u(t)
//          - PWM + filter
//
//        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "LPC824.h"
#include "fsl_device_registers.h"
#include "pin_mux.h"
#include "xprintf.h"

#include "fsl_adc.h"
#include "fsl_power.h"

#include "peripherals.h"
#include "dynamic_params.h"


void SysTick_DelayTicks(uint32_t ticks);


volatile uint32_t SysTickCounter;
uint16_t counter = 0;

float y_kt;
float u_kt = 0;
float r_kt = 0;


float e_kt;
float e_kt_prev = 0.0;
float i;

float i_prev = 0.0;
float kp = 15.0;
float ki = 60.0;

float kd = 0.05;
float proportional = 0.0;
float derivative = 0.0;

float samplingTime = 0.01;

uint16_t pwmDutyCount;


int main(void)
{
    uint32_t frequency = 0U;
    adc_result_info_t ADCResultStruct;


    InitPins();
    clock_init();    // Set clock speed to 30MHz
    config_uart0();  // Configure UART0 for correct speed and byte format.
    xdev_out(uart_putch); // Set the hardware interface function for xprintf

//    Changes

    xprintf("\n\n\n\r");
    xprintf("**********************************\n\r");
    xprintf("Final Experiment - PART 3 .\r\n");
    xprintf("**********************************\n\n\n\n\r");


    CLOCK_EnableClock(kCLOCK_Sct);  // Enable clock of sct.
    SCT_Config_PWM();               // Config the half SCT0 which generates PWM.

    CLOCK_EnableClock(kCLOCK_Adc);      // Enable ADC clock

    POWER_DisablePD(kPDRUNCFG_PD_ADC0); // Power on ADC0


    //    Set SysTick reload value to generate 1ms interrupt
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1) { } // This would be an error condition.
    }


    // Hardware calibration is required after each chip reset.
    // See: Sec. 21.3.4 Hardware self-calibration
    frequency = CLOCK_GetFreq(kCLOCK_Irc);

    if (true == ADC_DoSelfCalibration(ADC0, frequency))
    {
//        xprintf("ADC Calibration Done.\r\n");
    } else
    {
//        xprintf("ADC Calibration Failed.\r\n");
    }


    ADC_Configuration(&ADCResultStruct);    // Configure ADC and operation mode.

//    xprintf("Configuration Done.\r\n\n");


    while (1)
    {


        if (counter >= 200)
        {
            counter = 0;
        }

        if (counter >= 0 && counter < 100)
        {
            r_kt = 0;
        }
        else if (counter >= 100 && counter < 200)
        {
            r_kt = 1;
        }


        ADC_DoSoftwareTriggerConvSeqA(ADC0);
        while (!ADC_GetChannelConversionResult(ADC0, ADC_CHANNEL, &ADCResultStruct)){}

        y_kt = (float)((int16_t)ADCResultStruct.result - ADC_TO_U_KT_OFFSET) * ADC_TO_U_KT_GAIN;
        e_kt = r_kt - y_kt;


        proportional = kp * e_kt;
        derivative = kd * ((e_kt - e_kt_prev) / (float) samplingTime);
        i = i_prev + (ki * samplingTime) * ((e_kt + e_kt_prev) / 2.0);


//        u(kT) <-> [-25, 25]
        u_kt = proportional + derivative + i;

        if (u_kt < U_MIN)
        {
            u_kt = -25.0;
        }
        else if (u_kt > U_MAX)
        {
            u_kt = 25.0;
        }

        pwmDutyCount = (uint16_t)((u_kt + U_KT_TO_DUTY_COUNT_OFFSET) * U_KT_TO_DUTY_COUNT_GAIN);


//        For Command Line Output
//        -----------------------
        xprintf("[INFO]: ADC Ch %d -> %d\r\n", ADCResultStruct.channelNumber, ADCResultStruct.result);
        xprintf("[INFO]: r(kT) -> %d (x100 Scale)\r\n", (int16_t)(r_kt * 100));
        xprintf("[INFO]: y(kT) -> %d (x100 Scale)\r\n", (int16_t)(y_kt * 100));
        xprintf("[INFO]: e(kT) -> %d (x100 Scale)\r\n", (int16_t)(e_kt * 100));
        xprintf("[INFO]: u(kT) -> %d\r\n", (int16_t)(u_kt)); // patlatıyor
        xprintf("[INFO]: PWM Duty Count -> %d\r\n", pwmDutyCount);
        xprintf("\t[INFO]: Proportional -> %d (x10 Scale)\r\n", (int16_t)(proportional * 10));
        xprintf("\t[INFO]: Integral -> %d (x10 Scale)\r\n", (int16_t)(i * 10));
        xprintf("\t[INFO]: Derivative -> %d (x10 Scale)\r\n", (int16_t)(derivative * 10));


//        For Serial Plot Output
//        -----------------------
//        xprintf("%d, %d\r\n", (int16_t)(r_kt * 100), (int16_t)(y_kt * 100));


        SCT_Set_PWM(pwmDutyCount);

        e_kt_prev = e_kt;
        i_prev = i;

        counter++;

//        100 Hz -> 10 ms Sampling Time
        SysTick_DelayTicks(10);
    }
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
