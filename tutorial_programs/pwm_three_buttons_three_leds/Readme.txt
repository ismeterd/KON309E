

The SCTimer project is a simple demonstration program of the SDK SCTimer's driver capabiltiy to generate PWM signals.
The PWM output can be observed with an oscilloscope.


In this example, The SCTimer module generates two PWM signals.

The example will initialize clock, pin mux configuration, then configure the SCTimer module to make it work.

The example configures first 24kHZ PWM with 95% duty cycle
    from SCTIMER output 4.  (Blue LED on Alakart)
The second is configured as  24kHZ PWM with 20% negative duty cycle
from SCTIMER output 2.  (Green LED on Alakart)

If you connect Pins PIO0_27 and PIO0_16 to an oscilloscope,
You can observe two 24kHZ PWM signals, one wit duty cycle of (100-95)%,
and the other with a duty cycle of 25%.
