// Functions to perform read and write operations on LM75 from Alakart with a LPC824
// Changes were made mainly in Ahmet Onat's lm75.c file.


#include "LPC824.h"
#include "fsl_i2c.h"
#include "lm75.h"
#include "xprintf.h"


#define I2C0_BASE_ADDR ((I2C_Type *)(I2C0_BASE))


void LM75_Write_Reg(uint8_t pointer, uint8_t *txbuf, uint8_t write_len, i2c_master_handle_t* i2c_handle)
{
    //This function implements Fig.10 in LM75 datasheet.
    //The following transaction takes place:
    // 1. Master sends slave address (of LM75) in write mode
    //    Note: Last bit ='0'
    //    Slave acknowledges.
    // 2. Master sends pointer byte to tell LM75 which register it will write to.
    //    This is the argument "pointer"
    //    Slave acknowledges.
    // 3. Master sends out MSB of the data
    //    Slave acknowledges.
    // 4. Master sends out LSB of the data
    //    Master sends Stop sequence.


    i2c_master_transfer_t masterXfer = {0};
    status_t retVal = kStatus_Fail;

    masterXfer.slaveAddress   = LM75_ADDR;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = pointer;  //Where to write to
    masterXfer.subaddressSize = 1;        // What is the value
    masterXfer.data           = txbuf;
    masterXfer.dataSize       = write_len;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    // Send master data to slave, non-blocking
    i2c_TX_complete = false; // Clear transfer complete flag.

    retVal = I2C_MasterTransferNonBlocking(I2C0_BASE_ADDR, i2c_handle, &masterXfer);

    if (retVal != kStatus_Success) {
        //return -1;
    }


    while (i2c_TX_complete==false) { //Wait until transfer complete.
    }

    i2c_TX_complete = false;
}




int16_t LM75_Read_Reg(uint8_t pointer, uint8_t * rxbuf, uint8_t read_len, i2c_master_handle_t * i2c_handle)
{
    //This function implements Fig.11 in LM75 datasheet.
    //The following transaction takes place:
    // 1. Master sends slave address (of LM75) in *write mode*
    //    Note: Last bit ='0'
    //    Slave acknowledges.
    // 2. Master sends pointer byte to tell LM75 which register it will read from.
    //    This is the argument "pointer"
    //    Slave acnowledges.
    // 3. Master sends RE-START sequence after device acknowledge.
    // 4. Master sends slave address (of LM75) in *read mode*
    //    Note: Last bit ='1'
    //    Slave acknowledges.
    // 5. Slave sends out MSB of the data
    //    Master acknowledges.
    // 6. Slave sends out LSB of the data
    //    Master sends Stop sequence.


    uint16_t temperature;

    i2c_master_transfer_t masterXfer = {0};
    status_t retVal                   = kStatus_Fail;

    // Prepare the library I2C struct:
    masterXfer.slaveAddress   = LM75_ADDR;
    masterXfer.direction      = kI2C_Read;
    masterXfer.subaddress     = (uint32_t)pointer;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = rxbuf;
    masterXfer.dataSize       = read_len;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    retVal = I2C_MasterTransferNonBlocking(I2C0_BASE_ADDR, i2c_handle, &masterXfer);

    i2c_TX_complete = false; // Is set by ISR.

    if (retVal != kStatus_Success) {
        return -1;
    }

    while (i2c_TX_complete==false) { //Wait until transfer complete.
    }
    i2c_TX_complete = false;


    // For the formatting of the temperature data, see LM75 datasheet.
    temperature=rxbuf[0];
    temperature=temperature << 8;
    temperature=temperature + rxbuf[1];
    temperature=temperature >> 5;

    return (temperature);
}


void print_temp (uint8_t* buf)
{
    // xprintf cannot print floating point numbers, we mimic it here:

    xprintf("%d.", buf[0]);

    // The digits after the decimal point are shifted 5 places to the right:
    buf[1]=buf[1]>>5;  // Correct it.
    uint16_t i=buf[1]*125;   // Each digit represents 0.125 degC
    xprintf("%d",i );
    if (buf[1]==0){ // They print correctly, except .0
        xprintf("00");      // For that, we need to add a trailing '00' manually.
    }
    //xprintf("\n\r");  // Better keep the format related characters at the calling function.
}

void print_os_fault(uint8_t* buf)
{
    uint8_t significant_value = buf[0];

    uint8_t bit_3 = significant_value & 0b00001000;
    uint8_t bit_4 = significant_value & 0b00010000;

//    queue value 1
    if (bit_4 == 0 && bit_3 == 0)
    {
        xprintf("Queue Value = 1\n");
    }
//    queue value 2
    else if (bit_4 == 0 && bit_3 != 0)
    {
        xprintf("Queue Value = 2\n");
    }
//    queue value 4
    else if (bit_4 != 0 && bit_3 == 0)
    {
        xprintf("Queue Value = 4\n");
    }
//    queue value 6
    else if (bit_4 != 0 && bit_3 != 0)
    {
        xprintf("Queue Value = 6\n");
    }
}
