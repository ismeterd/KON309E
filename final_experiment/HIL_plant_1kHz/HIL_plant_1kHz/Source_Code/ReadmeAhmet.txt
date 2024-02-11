

This project implements a dynamic system given by a difference equation.
It has analog input and a PWM output.

Input-Output relationship
The analog input can range between (0,3V3). Internally it is mapped to a range that is suitable for the dynamic system. Typically 3.3/2 should be taken as '0'.

The output is PWM which must be filtered to get an analog voltage. The dynamic system implemented here runs at 1kHz sampling rate and the PWM rate is 100kHz. A simple 1st order RC filter is enough to get decent analog output signal.

PWM duty cycle range is (0,300). The output should be scaled to fit in this range, and typically '0' output corresponds to '150' PWM duty.


The plant implementation


Connection between a controller and a plant


How to obtain the dynamic models of the controller and the plant


