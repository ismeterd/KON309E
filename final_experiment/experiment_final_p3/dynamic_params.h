#ifndef DYNAMIC_PARAMS_H
#define DYNAMIC_PARAMS_H


// Input of the Controller
#define ADC_TO_U_KT_GAIN 0.0009765 // 2/2048
#define ADC_TO_U_KT_OFFSET 2047

// Output of the controller:
#define U_MIN -25.0
#define U_MAX 25.0

// u(kT) -> Duty Count Conversion
#define U_KT_TO_DUTY_COUNT_GAIN 6 // 2/2048
#define U_KT_TO_DUTY_COUNT_OFFSET 25

// Controller Parameters
#define K_P 15.0
#define K_I 60.0
#define K_D 0.05


#endif
