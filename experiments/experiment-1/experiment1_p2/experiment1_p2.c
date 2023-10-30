/*
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                  Experiment 1 - Part 2
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 *      # Connections
 *      ---------------------------------------------
 *      * Button 1 -> PIO0_21
 *      * Button 2 -> PIO0_20
 *      * Red Led -> PIO0_17
 *      * Yellow Led -> PIO0_18
 *      * Green Led -> PIO0_19
 *
 *      # Scenario
 *      ---------------------------------------------
 *      * Start red LED will be ON (initial state).
 *      *  When B1 is pressed, the LEDs will cycle as:
 *          - Red ON, Yellow OFF, Green OFF
 *          - Red OFF, Yellow ON, Green OFF
 *          - Red OFF, Yellow OFF, Green ON
 *          - Red ON, Yellow ON, Green ON
 *          - Red OFF, Yellow OFF, Green OFF
 *      * When B2 is pressed, the LEDs will cycle in the
 *      same sequence but will skip two places:
 *          - Red ON, Yellow OFF, Green OFF
 *          - Red OFF, Yellow OFF, Green ON
 *          - Red OFF, Yellow OFF, Green OFF
 *          - Red OFF, Yellow ON, Green OFF
 *          - Red ON, Yellow ON, Green ON
 *
 *      ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

#include <stdint.h> // Declarations of uint32_t etc.
#include "lpc824.h" // Declarations of LPC824 register names.

#define SYSTEM_CORE_CLOCK 30000000UL   //Declare system clock as 30MHz
// (The clock speed has been set in "init.c" file to 30MHz.)

static inline uint32_t SysTickConfig(uint32_t ticks);
void SysTick_Handler(void);  //our systick interrupt handler
void delay_ms(uint32_t ms);//delay (ms)

volatile uint32_t delaytime; // This is decremented by SysTick_Handler.
volatile int state = 0;
volatile int button1Pressed = 0;
volatile int button2Pressed = 0;

int main(void)
{
    delaytime=0;

    // System Clock Control Register
    // This register enables the clocks to individual system and peripheral blocks.
    // Bit 6 -> GPIO
    // Bit 7 -> SWM (Switch Matrix)
    // Bit 18 -> IOCON
    // 0100 0000 0000 1100 0000 -> 0x400C0
    SYSCON_SYSAHBCLKCTRL |= 0x400C0; // Enable IOCON, SWM & GPIO clocks.

    // Peripheral Reset Control Register
    // Bit 10 -> GPIO_RST_N
    // 0100 0000 0000 -> 0x400
    SYSCON_PRESETCTRL &= ~(0x400);  // Peripheral reset control to gpio/gpio int
    SYSCON_PRESETCTRL |=   0x400;   // AO: Check.

    // GPIO Direction Port Register
    // 0 input - 1 output
    GPIO_DIR0 &= (!(1<<21)); // Pin 21 input (B1)
    GPIO_DIR0 &= (!(1<<20)); // Pin 20 input (B2)

    GPIO_DIR0 |= (1<<17); // Pin 17 output (Red LED)
    GPIO_DIR0 |= (1<<18); // Pin 17 output (Yellow LED)
    GPIO_DIR0 |= (1<<19); // Pin 17 output (Green LED)

    SysTickConfig(SYSTEM_CORE_CLOCK/1000);  //setup systick clock interrupt @1ms

    GPIO_B17 = 0; // red on
    GPIO_B18 = 1; // yellow off
    GPIO_B19 = 1; // green off

    while (1)
    {
        if (GPIO_B21 == 1) // when button 1 is pressed
        {
            if(!button1Pressed)
            {
                button1Pressed = 1;
                state++;
            }

            if (state > 4)
            {
                state = 0;
            }
            delay_ms(50);
        }
        else
        {
            button1Pressed = 0;
        }

        if (GPIO_B20 == 1) // when button 2 is pressed
        {
            if(!button2Pressed)
            {
                button2Pressed = 1;
                state += 2;
            }

            if (state == 5)
            {
                state = 0;
            }
            else if (state == 6)
            {
                state = 1;
            }
            delay_ms(50);
        }
        else
        {
            button2Pressed = 0;
        }

        switch(state)
        {
            case 0:
                GPIO_B17 = 0; // red on
                GPIO_B18 = 1; // yellow off
                GPIO_B19 = 1; // green off
                break;
            case 1:
                GPIO_B17 = 1; // red off
                GPIO_B18 = 0; // yellow on
                GPIO_B19 = 1; // green off
                break;
            case 2:
                GPIO_B17 = 1; // red off
                GPIO_B18 = 1; // yellow off
                GPIO_B19 = 0; // green on
                break;
            case 3:
                GPIO_B17 = 0; // red on
                GPIO_B18 = 0; // yellow on
                GPIO_B19 = 0; // green on
                break;
            case 4:
                GPIO_B17 = 1; // red off
                GPIO_B18 = 1; // yellow off
                GPIO_B19 = 1; // green off
                break;
            default:
                break;
        }
    }
}

//The interrupt handler for SysTick system time-base timer.
void SysTick_Handler(void)
{
    if (delaytime!=0)
    { // If delaytime has been set somewhere in the program,
        --delaytime;     //  decrement it every time SysTick event occurs (1ms).
    }
}

void delay_ms(uint32_t ms)
{
    delaytime=ms;        // Set the delay time to the number of millisecs of wait
    while(delaytime!=0){}// Wait here until the delay time expires.
}

// System Tick Configuration:
// Initializes the System Timer and its interrupt, and
// Starts the System Tick Timer.
// ticks = Number of ticks between two interrupts.

static inline uint32_t SysTickConfig(uint32_t ticks)
{
    if (ticks > 0xFFFFFFUL) // Timer is only 24 bits wide.
    {
        return (1); //Reload value impossible
    }

    SYST_RVR = (ticks & 0xFFFFFFUL) - 1;  //Set reload register
    SYST_CVR = 0;   //Load the initial count value.
    SYST_CSR = 0x07;  // Counter ENABLE, INT ENABLE, CLK source=system clock.
    return (0);
}         // AO!: Check OK.
