
#ifndef DYNAMIC_PARAMS_H
#define DYNAMIC_PARAMS_H

#define A1 -1.998
#define A2 0.998
#define B1 0.001
#define B2 -0.000998

// Input range is (-25,25)
#define ADC_INPUT_CONV_OFFSET 2047
#define ADC_INPUT_CONV_GAIN 0.01220703125 //25/2048

// Output of the plant:
#define Y_MIN -2.0
#define Y_MAX 2.0
#define PWM_DUTY_CONV_OFFSET 150 // Convert to PWM of (0,300) range.
#define PWM_DUTY_CONV_GAIN 75



typedef struct plant_model {

  float y, yz, yz2;  // Current output, 1 delayed, 2 delayed output.
  float u, uz, uz2;  // Current input, 1 delayed, 2 delayed input.

  float a1, a2;      //denom coeff.
  float b1, b2;      // num coeff.

  // The plant transfer function is:

  // Y(z)        b1*z^(-1) + b2*z(-2)
  //----- = --------------------------
  // U(z)     1+ a1*z^(-1) + a2*z(-2)

  // Y(z)= -a1*z^(-1)Y(z)-a2*z(-2)Y(z)+b1*z^(-1)U(z)+b2*z^(-2)U(Z)
  // Written out as:
  // y = -a1*yz-a2*yz2+b1*uz+b2*uz2;
  
}disc_plant_t;


#endif
