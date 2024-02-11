// Experiment 5 - Part 1

// Demonstration of temperature sensing using the LM75 module.
// The method shown in Fig.11 of LM75 datasheet is implemented.

// The connections to LM75 module are:
// PIO0_10 <-> SCL
// PIO0_11 <-> SDA
// GND <-> GND
// 3V3 <-> 3V3

// System Clock is initialized to 30MHz, Main Clock is 60MHz.
// USART is initialized to 15200 Baud 8N1.
// I2C baud rate is 12MHz
// xprintf from ChaN is used for serial terminal functions.
// Changes were made mainly in Ahmet Onat's Example.

#include <stdio.h>
#include <string.h>
#include "pin_mux.h"
#include "fsl_i2c.h"
#include "fsl_power.h"
#include "fsl_swm.h"
#include "fsl_swm_connections.h"
#include "xprintf.h"
#include "lm75.h"

#define GPIO_PORT 0
#define BLUE_LED_PIN 16

// Set CPU Core clock frequency (Hz)
#define CORE_CLOCK   30000000U

#define I2C_MASTER_CLOCK_FREQUENCY (12000000)
#define I2C0_BASE_ADDR ((I2C_Type *)(I2C0_BASE))

// Set I2C_BAUDRATE 100K
#define I2C_BAUDRATE  (100000)


void clock_init(void);
void SysTick_DelayTicks(uint32_t n);
static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData);
void I2C_Write(uint8_t data, uint8_t address, i2c_master_handle_t* i2c_handle);
void config_uart0 (void);
void uart_putch (uint8_t character);


volatile bool i2c_TX_complete = false;
volatile uint32_t SystickCounter;


int main(void)
{

    i2c_master_handle_t i2c_handle;

//    Create RX and TX Buffer
    uint8_t i2c_rxbuf[LM75_READ_LEN];
    uint8_t i2c_txbuf[LM75_WRITE_LEN];

//    Enable clock of uart0.
    CLOCK_EnableClock(kCLOCK_Uart0);
//    Enable clock of i2c0.
    CLOCK_EnableClock(kCLOCK_I2c0);

//    Set configuration of USART and I2C pins
    InitPins();
//    Set clock speed to 30MHz
    clock_init();
//    Configure UART0 for correct speed and byte format.
    config_uart0();

//    Set the hardware interface function for xprintf
    xdev_out(uart_putch);

    xprintf("\r\n\r\n\n\nEXPERIMENT 5 - PART 1\r\n");
    xprintf("~~~~~~~~~~~~~~~~~~~~~~\n\n\n");


//    Set PIO0_16 as an output:
    GPIO->DIR[GPIO_PORT] |= (1<< BLUE_LED_PIN);


//    Set systick reload value to generate 1ms interrupt
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        while (1) { } // This would be an error condition.
    }


    ////////// I2C configuration:  ////////////

    i2c_master_config_t masterConfig;

    I2C_MasterGetDefaultConfig(&masterConfig);

//     Set I2C transmission speed.
    masterConfig.baudRate_Bps = I2C_BAUDRATE;

//    Initialize the I2C peripheral as master:
    I2C_MasterInit(I2C0_BASE_ADDR, &masterConfig, I2C_MASTER_CLOCK_FREQUENCY);

//    Create the I2C handle for non-blocking transfers
    I2C_MasterTransferCreateHandle(I2C0_BASE_ADDR, &i2c_handle, i2c_master_callback, NULL);

//    Set the configuration register on temperature sensor
//    binary -> 00010000
//    hexadecimal -> 0x10
//    decimal -> 16
    i2c_txbuf[0]=16;
    i2c_txbuf[1]=0;
    LM75_Write_Reg(LM75_REG_CONF, i2c_txbuf, LM75_READ_LEN, &i2c_handle);

    // Check what has been written:
    LM75_Read_Reg(LM75_REG_CONF, i2c_rxbuf, LM75_READ_LEN,  &i2c_handle);
    xprintf("\nLM75 Configuration\n");
    xprintf("\n\t* ");
    print_os_fault(i2c_rxbuf);

//    Set the alarm ON temperature
    i2c_txbuf[0]=LM75_T_HIGH;
    i2c_txbuf[1]=0;
    LM75_Write_Reg(LM75_REG_TOS, i2c_txbuf, LM75_READ_LEN, &i2c_handle);

//    Check what has been written:
    LM75_Read_Reg(LM75_REG_TOS, i2c_rxbuf, LM75_READ_LEN,  &i2c_handle);
    xprintf("\n\nAlarm Configuration\n");
    xprintf("\n\t* Upper Alarm Temperature = ");
    print_temp(i2c_rxbuf);

//    Set the alarm clear temperature.
    i2c_txbuf[0]=LM75_T_LOW;
    i2c_txbuf[1]=0;
    LM75_Write_Reg(LM75_REG_HYST, i2c_txbuf, LM75_READ_LEN, &i2c_handle);

//    Check what has been written:
    LM75_Read_Reg(LM75_REG_HYST, i2c_rxbuf, LM75_READ_LEN,  &i2c_handle);
    xprintf(" Celcius Degree");
    xprintf("\n\t* Lower Alarm Temperature = ");
    print_temp(i2c_rxbuf);
    xprintf(" Celcius Degree\n\r\n\r");

    xprintf ("\n[INFO]: I2C initialization complete.\r\n");
    xprintf("\n\r\n\r");

    while (1)
    {
        LM75_Read_Reg(LM75_REG_TEMP, i2c_rxbuf, LM75_READ_LEN,  &i2c_handle);
        xprintf("T=");
        print_temp(i2c_rxbuf);
        xprintf(" deg C\n\r");

        SysTick_DelayTicks(500U);
    }
}

static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_Success)
    {
        i2c_TX_complete = true;
    }
}

void I2C_Write(uint8_t data, uint8_t address, i2c_master_handle_t* i2c_handle)
{
    i2c_master_transfer_t masterXfer = {0};
    status_t retVal = kStatus_Fail;
    uint8_t txBuff[1];

    txBuff[0]=(int)(data);

    masterXfer.slaveAddress   = address;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = 0;  //Dummy...
    masterXfer.subaddressSize = 0;  // There is no register to configure.
    masterXfer.data           = txBuff;
    masterXfer.dataSize       = 1;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

//    Send master data to slave, non-blocking
//    Clear transfer complete flag.
    i2c_TX_complete = false;

    retVal = I2C_MasterTransferNonBlocking(I2C0_BASE_ADDR, i2c_handle, &masterXfer);

    if (retVal != kStatus_Success)
    {
        //return -1;
    }

    while (i2c_TX_complete==false) { }  //  Wait for transfer complete

    i2c_TX_complete = false;
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
