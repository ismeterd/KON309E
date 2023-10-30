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


void delay(int counts);//delay (ms)

volatile int state = 0;
volatile int button1Pressed = 0;
volatile int button2Pressed = 0;

int main(void)
{
    // System Clock Control Register
    // This register enables the clocks to individual system and peripheral blocks.
    // Bit 6 -> GPIO
    // Bit 7 -> SWM (Switch Matrix)
    // Bit 18 -> IOCON
    // 0100 0000 0000 1100 0000
    // 0040
    // 0x400C0

//    SYSCON_BASE		0x40048000
//    #define SYSCON_SYSAHBCLKCTRL	REGISTER_32(SYSCON_BASE + 0x80)

//    SYSCON_SYSAHBCLKCTRL |= 0x400C0; // Enable IOCON, SWM & GPIO clocks.
    *((volatile unsigned int *)(0x40048080)) |= 0x00000040;


    // Peripheral Reset Control Register
    // Bit 10 -> GPIO_RST_N
    // 0100 0000 0000

//#define SYSCON_PRESETCTRL	REGISTER_32(SYSCON_BASE + 0x04)
//    SYSCON_BASE		0x40048000
//    SYSCON_PRESETCTRL &= ~(0x400);  // Peripheral reset control to gpio/gpio int
//    SYSCON_PRESETCTRL |= 0x400;   // AO: Check.

    *((volatile unsigned int *)(0x40048004)) &= ~(0x400);
    *((volatile unsigned int *)(0x40048004)) |= 0x400;

    // GPIO Direction Port Register
    // 0 input - 1 output
    //Make Pin 9 an output. On Alakart, Pin #9 is the blue LED:
    // 0000 0000 0000
    // 0000 0000 0001
    // 0010 0000 0000

//#define GPIO_DIR0			REGISTER_32(GPIO_BASE + 0x2000)
//#define GPIO_BASE		0xa0000000

    *((volatile unsigned int *)(0xA0002000)) &= ~(0x00300000);
//    GPIO_DIR0 &= (!(1<<21)); //Pin 21 input (B1)
//    *((volatile unsigned int *)(0xa0002015)) = 0;
//    GPIO_DIR0 &= (!(1<<20)); //Pin 20 input (B2)
//    *((volatile unsigned int *)(0xa0002014)) = 0;


    *((volatile unsigned int *)(0xA0002000)) |= (0x000E0000);
//    GPIO_DIR0 |= (1<<17);
//    *((volatile unsigned int *)(0xa0002011)) = 1;
//    *((volatile unsigned int *)(0xA0002000)) |= 0x20000;
//    GPIO_DIR0 |= (1<<18);
//    *((volatile unsigned int *)(0xa0002012)) = 1;
//    GPIO_DIR0 |= (1<<19);
//    *((volatile unsigned int *)(0xa0002013)) = 1;


//#define GPIO_B17			REGISTER_8(GPIO_BASE + 17)
//#define GPIO_BASE		0xa0000000
//    GPIO_B17 = 0; // red on
    *((volatile unsigned char *)(0xA0000011)) = 0;

//    GPIO_B18 = 1; // yellow off
    *((volatile unsigned char *)(0xA0000012)) = 1;

//    GPIO_B19 = 1; // green off
    *((volatile unsigned char *)(0xA0000013)) = 1;


//    GPIO_B20 -> 0xa0000014 -> *((volatile unsigned int *)(0xa0000014))
//    GPIO_B21 -> 0xa0000015 -> *((volatile unsigned int *)(0xa0000015))

    while (1)
    {
        if (*((volatile unsigned char *)(0xA0000015)) == 1) //when button 1 is pressed
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
            delay(100000);
        }
        else
        {
            button1Pressed = 0;
        }

        if (*((volatile unsigned char *)(0xA0000014)) == 1) //when button 2 is pressed
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
            delay(100000);
        }
        else
        {
            button2Pressed = 0;
        }

        //    GPIO_B17 -> 0xa0000011 -> *((volatile unsigned int *)(0xa0000011))
        //    GPIO_B18 -> 0xa0000012 -> *((volatile unsigned int *)(0xa0000012))
        //    GPIO_B19 -> 0xa0000013 -> *((volatile unsigned int *)(0xa0000013))

        if(state == 0) {
            *((volatile unsigned char *)(0xA0000011)) = 0;
            *((volatile unsigned char *)(0xA0000012)) = 1;
            *((volatile unsigned char *)(0xA0000013)) = 1;
        } else if(state == 1) {
            *((volatile unsigned char *)(0xA0000011)) = 1;
            *((volatile unsigned char *)(0xA0000012)) = 0;
            *((volatile unsigned char *)(0xA0000013)) = 1;
        } else if(state == 2) {
            *((volatile unsigned char *)(0xA0000011)) = 1;
            *((volatile unsigned char *)(0xA0000012)) = 1;
            *((volatile unsigned char *)(0xA0000013)) = 0;
        } else if(state == 3) {
            *((volatile unsigned char *)(0xA0000011)) = 0;
            *((volatile unsigned char *)(0xA0000012)) = 0;
            *((volatile unsigned char *)(0xA0000013)) = 0;
        } else if(state == 4) {
            *((volatile unsigned char *)(0xA0000011)) = 1;
            *((volatile unsigned char *)(0xA0000012)) = 1;
            *((volatile unsigned char *)(0xA0000013)) = 1;
        } else {
            // do nothing
        }
    }
}

void delay(int counts)
{
    int wait;
    for (wait=counts; wait>0; --wait){
    }

}
