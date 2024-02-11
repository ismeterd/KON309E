// ADC Channel 0 -> PIO_07

#define ADC_CHANNEL 0U  // Channel 1 will be used in this example.
#define ADC_CLOCK_DIVIDER 0U // See Fig 52. ADC clocking in Ref Manual.

#define CORE_CLOCK   30000000U  // Set CPU Core clock frequency (Hz)

#define PWM_MAX_VAL 300

void ADC_Configuration(adc_result_info_t * ADCResultStruct);
//void SCT_Config_ADC_Trig(void);
void SCT_Config_PWM(void);
void SCT_Set_PWM(uint16_t duty);

void clock_init(void);
void config_uart0 (void);
void uart_putch (uint8_t character);
