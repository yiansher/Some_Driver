#include "bh1730.h"
#include "mytimer.h"

#ifdef BH1730

static void BH1730_WriteReg(uint8_t addr, uint8_t value)
{
    // uint8_t ret;
    uint8_t buf[2];
    // buf[0] = addr;
    buf[0] = addr | BH1730_CMD;
    buf[1] = value;
    HAL_I2C_Master_Transmit(&hi2c2, BH1730_ADDR, buf, 2, HAL_I2C_STATE_TIMEOUT);
}


static uint8_t BH1730_ReadReg(uint8_t addr)
{
    // uint8_t ret;
    uint8_t value = 0;
    addr = addr | BH1730_CMD;
    HAL_I2C_Master_Transmit(&hi2c2, BH1730_ADDR, &addr, 1, HAL_I2C_STATE_TIMEOUT);

    HAL_I2C_Master_Receive(&hi2c2, BH1730_ADDR, &value, 1, HAL_I2C_STATE_TIMEOUT);
    return value;
}


void BH1730_init(void)
{
    BH1730_WriteReg(BH1730_CMD_SPECIAL | BH1730_CMD_SPECIAL_SOFT_RESET, (1 << 7));
}

/**
 *  Set gain of the internal ADC
 *
 */
void setGain(uint8_t gain)
{
    if (gain == GAIN_X1)
    {
        BH1730_WriteReg(BH1730_REG_GAIN, BH1730_GAIN_X1_MODE);
    }
    else if (gain == GAIN_X2)
    {
        BH1730_WriteReg(BH1730_REG_GAIN, BH1730_GAIN_X2_MODE);
    }
    else if (gain == GAIN_X64)
    {
        BH1730_WriteReg(BH1730_REG_GAIN, BH1730_GAIN_X64_MODE);
    }
    else
    {
        BH1730_WriteReg(BH1730_REG_GAIN, BH1730_GAIN_X128_MODE);
    }
}

/**
 * Read lux level from sensor.
 * Returns -1 if read is timed out
 *
 */
uint32_t BH1730_GetLux()
{

    // set accuracy, default GAIN_X1
    uint8_t gain = GAIN_X1;

    BH1730_WriteReg(BH1730_REG_CONTROL, BH1730_REG_CONTROL_POWER | BH1730_REG_CONTROL_ADC_EN | BH1730_REG_CONTROL_ONE_TIME);

    uint8_t data0_lb, data0_mb, data1_lb, data1_mb;
    data0_lb = BH1730_ReadReg(BH1730_REG_DATA0_LOW);
    data0_mb = BH1730_ReadReg(BH1730_REG_DATA0_HIGH);
    data1_lb = BH1730_ReadReg(BH1730_REG_DATA1_LOW);
    data1_mb = BH1730_ReadReg(BH1730_REG_DATA1_HIGH);

    uint16_t data0, data1;
    data0 = (data0_mb << 8) + data0_lb;
    data1 = (data1_mb << 8) + data1_lb;

    if (data0 == 0)return 0;

    uint32_t lx = 0;
    uint32_t div = data1 / data0;


    if (div < 0.26)
    {
        lx = ((1.29 * data0) - (2.733 * data1)) / gain * 102.6 / BH1730_ITIME_MS;
    }
    else if (div < 0.55)
    {
        lx = ((0.795 * data0) - (0.859 * data1)) / gain * 102.6 / BH1730_ITIME_MS;
    }
    else if (div < 1.09)
    {
        lx = ((0.51 * data0) - (0.345 * data1)) / gain * 102.6 / BH1730_ITIME_MS;
    }
    else if (div < 2.13)
    {
        lx = ((0.276 * data0) - (0.13 * data1)) / gain * 102.6 / BH1730_ITIME_MS;
    }

    return lx;
}
#endif


#ifdef LTR303

void LTR303_Write(uint8_t RegAddress, uint8_t Cmd)
{
    uint8_t buf[1] = {0};
    buf[0] = Cmd;
    HAL_I2C_Mem_Write(&hi2c2, LTR303_ADDR, RegAddress, I2C_MEMADD_SIZE_8BIT, buf, 1, HAL_I2C_STATE_TIMEOUT);
}

uint8_t LTR303_Read(uint8_t RegAddress)
{
    uint8_t RegData[1] = {0};
    HAL_I2C_Mem_Read(&hi2c2, LTR303_ADDR, RegAddress, I2C_MEMADD_SIZE_8BIT, RegData, 1, HAL_I2C_STATE_TIMEOUT);
    return RegData[0];
}

void LTR303_Init(void)
{
    uint8_t buf = 0;
    /* Enable ALS */
    buf = 0x01;
    LTR303_Write(0x80, buf);
    Delay_1ms(10);
    /* ALS Measurement Rate Set */
    buf = 0x12;
    LTR303_Write(0x85, buf);
    Delay_1ms(10);
}

uint16_t LTR303_ReadADC_CH1(void)
{
    uint16_t ADC_Data = 0;
    uint8_t ADC_bufL = 0;
    uint8_t ADC_bufH = 0;
    ADC_bufL = LTR303_Read(0x88);
    Delay_1ms(10);
    ADC_bufH = LTR303_Read(0x89);
    Delay_1ms(10);
    ADC_Data = (ADC_bufH << 8) | ADC_bufL;
    return ADC_Data;

}

uint16_t LTR303_ReadADC_CH0(void)
{
    uint16_t ADC_Data = 0;
    uint8_t ADC_bufL = 0;
    uint8_t ADC_bufH = 0;
    ADC_bufL = LTR303_Read(0x8A);
    Delay_1ms(10);
    ADC_bufH = LTR303_Read(0x8B);
    Delay_1ms(10);
    ADC_Data = (ADC_bufH << 8) | ADC_bufL;
    return ADC_Data;
}


#endif



