// Changes were made mainly in Ahmet Onat's lm75.h file.

#ifndef LM75_H
#define LM75_H

#define LM75_ADDR      0x48 // This is the 7 bit address.
#define LM75_REG_TEMP  0x00 // Temp readout register This is the default value.
#define LM75_REG_CONF  0x01 // Config Register
#define LM75_REG_HYST  0x02 // Hysteresis register.
#define LM75_REG_TOS   0x03 // Thermostat register

#define LM75_READ_LEN   2 // LM75 returns 2 bytes.
#define LM75_WRITE_LEN   2 // LM75 requires 2 bytes.

#define LM75_T_HIGH   32 // Alarm threshold upper temp
#define LM75_T_LOW   30 // Alarm threshold lower temp

extern volatile bool i2c_TX_complete;

int16_t LM75_Read_Reg(uint8_t pointer,
		      uint8_t * rxbuf,
		      uint8_t read_len,
		      i2c_master_handle_t * i2c_handle);

void LM75_Write_Reg(uint8_t pointer,
		    uint8_t *txbuf,
		    uint8_t write_len,
		    i2c_master_handle_t* i2c_handle);

void print_temp(uint8_t* buf);

void print_os_fault(uint8_t* buf);

#endif
