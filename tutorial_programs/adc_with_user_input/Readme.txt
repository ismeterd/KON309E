Written by Ahmet Onat

The "adc_basic" project demonstrates a simple initialization and use of the ADC.
Conversion is started by the software directly. The software blocks (stops all further execution) until the ADC conversion is complete.

A key press from the terminal is required to initiate one ADC conversion.
Once the conversion is initiated, the DATAVALID bit of the "Global Data Register" (See Sec. 21.6.4 A/D Global Data Register A and B) is polled to check for end of conversion.
If the bit is set, the result is read and printed out through the serial port. The program then waits for another key press. Serial communication speed is set to 115200 Baud.

The ADC is a complex peripheral device which can do several things:
* Trigger sources:
  It can get a trigger from different sources such as:
  - Software
  - Timer
  - etc.

* Conversion complete:
  After the conversion is complete, it can do the following:
  - Set a "conversion complete" flag
  - Trigger an ADC INT
  - Trigger a DMA transaction to automatically write the results to
    a specified location in the memory.
  - etc.

* Convert several specified ADC channels after one trigger.
  - There are two "sequence registers": Sequence A and Sequence B.
  - A channel can be registered in these sequences.
  - After a trigger of either sequence, the ADC converts all the channels registered in the given sequence and then performs the "Conversion Complete" event as described above

* Detect ADC readings that are lower or higher than set thresholds
  If the user sets ADC thresholds to test against, the HW will check the result wht those thresholds, and if enabled, an INT will be triggered.
  This can be considered as an analog voltage alarm.

After each processor reset, the ADC must be calibrated. This is a simple operation which is done by a function call in the PSL or setting a bit in the A/D control register (See: Sec. 21.6.1 ADC Control Register, bit 30)

The ADC conversion of Sequence A is started by setting  SEQA_CTRL_START to '1'. Sequence B is similar.



